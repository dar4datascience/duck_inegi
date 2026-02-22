#define DUCKDB_EXTENSION_MAIN

#include "inegi_extension.hpp"
#include "inegi_token_manager.hpp"
#include "inegi_api_client.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>
#include <nlohmann/json.hpp>

namespace duckdb {

// INEGI_SetToken function - stores API token in session
static void INEGISetTokenFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &token_vector = args.data[0];
	auto &context = state.GetContext();
	
	UnaryExecutor::Execute<string_t, bool>(token_vector, result, args.size(), [&](string_t token) {
		string token_str = token.GetString();
		INEGITokenManager::SetToken(context, token_str);
		return true;
	});
}

// INEGI_GetToken function - retrieves current token (for debugging)
static void INEGIGetTokenFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &context = state.GetContext();
	
	if (!INEGITokenManager::HasToken(context)) {
		throw InvalidInputException(
			"INEGI API token not set. Please obtain a token from:\n"
			"https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify\n"
			"Then set it using: SELECT INEGI_SetToken('your-token-here');"
		);
	}
	
	string token = INEGITokenManager::GetToken(context);
	result.SetValue(0, Value(token));
}

// INEGI_Read table function - fetches indicator data
struct INEGIReadBindData : public TableFunctionData {
	string indicator_id;
	string language;
	string geography;
	bool recent_only;
	string bank;
	string token;
	nlohmann::json data;
	bool data_fetched = false;
};

struct INEGIReadGlobalState : public GlobalTableFunctionState {
	idx_t current_row = 0;
};

static unique_ptr<FunctionData> INEGIReadBind(ClientContext &context, TableFunctionBindInput &input,
                                               vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<INEGIReadBindData>();
	
	// Get token from session
	if (!INEGITokenManager::HasToken(context)) {
		throw InvalidInputException(
			"INEGI API token not set. Please obtain a token from:\n"
			"https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify\n"
			"Then set it using: SELECT INEGI_SetToken('your-token-here');"
		);
	}
	result->token = INEGITokenManager::GetToken(context);
	
	// Get indicator ID (required)
	result->indicator_id = input.inputs[0].ToString();
	
	// Get optional parameters from named parameters
	for (auto &kv : input.named_parameters) {
		if (kv.first == "language") {
			result->language = kv.second.ToString();
		} else if (kv.first == "geography") {
			result->geography = kv.second.ToString();
		} else if (kv.first == "recent_only") {
			result->recent_only = kv.second.GetValue<bool>();
		} else if (kv.first == "bank") {
			result->bank = kv.second.ToString();
		}
	}
	
	// Set defaults
	if (result->language.empty()) result->language = "es";
	if (result->geography.empty()) result->geography = "00";
	if (result->bank.empty()) result->bank = "BIE";
	
	// Define output schema
	names.emplace_back("time_period");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("observation_value");
	return_types.emplace_back(LogicalType::DOUBLE);
	names.emplace_back("indicator_id");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("indicator_name");
	return_types.emplace_back(LogicalType::VARCHAR);
	
	return std::move(result);
}

static unique_ptr<GlobalTableFunctionState> INEGIReadInit(ClientContext &context, TableFunctionInitInput &input) {
	auto result = make_uniq<INEGIReadGlobalState>();
	auto &bind_data = input.bind_data->Cast<INEGIReadBindData>();
	
	// Fetch data from API
	if (!bind_data.data_fetched) {
		INEGIAPIClient client(bind_data.token);
		bind_data.data = client.FetchIndicatorData(
			bind_data.indicator_id,
			bind_data.language,
			bind_data.geography,
			bind_data.recent_only,
			bind_data.bank
		);
		bind_data.data_fetched = true;
	}
	
	return std::move(result);
}

static void INEGIReadFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &bind_data = data_p.bind_data->Cast<INEGIReadBindData>();
	auto &state = data_p.global_state->Cast<INEGIReadGlobalState>();
	
	// Parse JSON-Stat format
	try {
		if (!bind_data.data.contains("value") || !bind_data.data.contains("dimension")) {
			throw IOException("Invalid JSON-Stat format received from INEGI API");
		}
		
		auto values = bind_data.data["value"];
		auto dimensions = bind_data.data["dimension"];
		
		// Get indicator metadata
		string indicator_name = "";
		if (dimensions.contains("INDICADOR") && dimensions["INDICADOR"].contains("category")) {
			auto categories = dimensions["INDICADOR"]["category"];
			if (categories.contains("label")) {
				auto labels = categories["label"];
				if (!labels.empty() && labels.is_object()) {
					indicator_name = labels.begin().value().get<string>();
				}
			}
		}
		
		// Get time periods
		std::vector<string> time_periods;
		if (dimensions.contains("TIME_PERIOD") && dimensions["TIME_PERIOD"].contains("category")) {
			auto time_cat = dimensions["TIME_PERIOD"]["category"];
			if (time_cat.contains("index")) {
				auto time_index = time_cat["index"];
				for (auto &el : time_index.items()) {
					time_periods.push_back(el.key());
				}
			}
		}
		
		idx_t total_rows = values.size();
		idx_t rows_to_output = std::min((idx_t)STANDARD_VECTOR_SIZE, total_rows - state.current_row);
		
		if (rows_to_output == 0) {
			output.SetCardinality(0);
			return;
		}
		
		// Fill output chunk
		for (idx_t i = 0; i < rows_to_output; i++) {
			idx_t row_idx = state.current_row + i;
			
			// Time period
			if (row_idx < time_periods.size()) {
				output.SetValue(0, i, Value(time_periods[row_idx]));
			} else {
				output.SetValue(0, i, Value());
			}
			
			// Observation value
			if (row_idx < values.size() && !values[row_idx].is_null()) {
				double val = values[row_idx].get<double>();
				output.SetValue(1, i, Value::DOUBLE(val));
			} else {
				output.SetValue(1, i, Value());
			}
			
			// Indicator ID
			output.SetValue(2, i, Value(bind_data.indicator_id));
			
			// Indicator name
			output.SetValue(3, i, Value(indicator_name));
		}
		
		state.current_row += rows_to_output;
		output.SetCardinality(rows_to_output);
		
	} catch (const nlohmann::json::exception &e) {
		throw IOException("Error parsing JSON response: " + string(e.what()));
	}
}

static void LoadInternal(ExtensionLoader &loader) {
	// Register INEGI_SetToken scalar function
	auto set_token_function = ScalarFunction(
		"INEGI_SetToken",
		{LogicalType::VARCHAR},
		LogicalType::BOOLEAN,
		INEGISetTokenFunction
	);
	ExtensionUtil::RegisterFunction(loader.GetDatabase(), set_token_function);
	
	// Register INEGI_GetToken scalar function
	auto get_token_function = ScalarFunction(
		"INEGI_GetToken",
		{},
		LogicalType::VARCHAR,
		INEGIGetTokenFunction
	);
	ExtensionUtil::RegisterFunction(loader.GetDatabase(), get_token_function);
	
	// Register INEGI_Read table function
	TableFunction read_function(
		"INEGI_Read",
		{LogicalType::VARCHAR},
		INEGIReadFunction,
		INEGIReadBind,
		INEGIReadInit
	);
	
	// Add named parameters
	read_function.named_parameters["language"] = LogicalType::VARCHAR;
	read_function.named_parameters["geography"] = LogicalType::VARCHAR;
	read_function.named_parameters["recent_only"] = LogicalType::BOOLEAN;
	read_function.named_parameters["bank"] = LogicalType::VARCHAR;
	
	ExtensionUtil::RegisterFunction(loader.GetDatabase(), read_function);
}

void INEGIExtension::Load(ExtensionLoader &loader) {
	LoadInternal(loader);
}

std::string INEGIExtension::Name() {
	return "inegi";
}

std::string INEGIExtension::Version() const {
#ifdef EXT_VERSION_INEGI
	return EXT_VERSION_INEGI;
#else
	return "0.1.0";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_CPP_EXTENSION_ENTRY(inegi, loader) {
	duckdb::LoadInternal(loader);
}
}
