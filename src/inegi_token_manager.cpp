#include "inegi_token_manager.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/common/exception.hpp"

namespace duckdb {

void INEGITokenManager::SetToken(ClientContext &context, const string &token) {
	if (token.empty()) {
		throw InvalidInputException("INEGI API token cannot be empty");
	}
	context.registered_state->Set(TOKEN_KEY, make_shared_ptr<Value>(Value(token)));
}

string INEGITokenManager::GetToken(ClientContext &context) {
	if (!context.registered_state->Contains(TOKEN_KEY)) {
		throw InvalidInputException("INEGI API token not set. Please obtain a token from:\n"
		                            "https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify\n"
		                            "Then set it using: SELECT INEGI_SetToken('your-token-here');");
	}
	auto value_ptr = context.registered_state->Get(TOKEN_KEY);
	return value_ptr->template GetValue<string>();
}

bool INEGITokenManager::HasToken(ClientContext &context) {
	return context.registered_state->Contains(TOKEN_KEY);
}

void INEGITokenManager::ClearToken(ClientContext &context) {
	context.registered_state->Remove(TOKEN_KEY);
}

} // namespace duckdb
