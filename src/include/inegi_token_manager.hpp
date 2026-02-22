#pragma once

#include "duckdb.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/client_context_state.hpp"

namespace duckdb {

// State class to store INEGI API token in ClientContext
class INEGITokenState : public ClientContextState {
public:
	explicit INEGITokenState(string token_p) : token(std::move(token_p)) {
	}

	string GetToken() const {
		return token;
	}

private:
	string token;
};

class INEGITokenManager {
public:
	static void SetToken(ClientContext &context, const string &token);
	static string GetToken(ClientContext &context);
	static bool HasToken(ClientContext &context);
	static void ClearToken(ClientContext &context);

private:
	static constexpr const char *TOKEN_KEY = "inegi_api_token";
};

} // namespace duckdb
