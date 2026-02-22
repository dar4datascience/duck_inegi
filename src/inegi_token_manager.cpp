#include "inegi_token_manager.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/common/exception.hpp"

namespace duckdb {

void INEGITokenManager::SetToken(ClientContext &context, const string &token) {
	if (token.empty()) {
		throw InvalidInputException("INEGI API token cannot be empty");
	}
	context.registered_state->Insert(TOKEN_KEY, make_shared_ptr<INEGITokenState>(token));
}

string INEGITokenManager::GetToken(ClientContext &context) {
	auto state = context.registered_state->Get<INEGITokenState>(TOKEN_KEY);
	if (!state) {
		throw InvalidInputException("INEGI API token not set. Please obtain a token from:\n"
		                            "https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify\n"
		                            "Then set it using: SELECT INEGI_SetToken('your-token-here');");
	}
	return state->GetToken();
}

bool INEGITokenManager::HasToken(ClientContext &context) {
	auto state = context.registered_state->Get<INEGITokenState>(TOKEN_KEY);
	return state != nullptr;
}

void INEGITokenManager::ClearToken(ClientContext &context) {
	context.registered_state->Remove(TOKEN_KEY);
}

} // namespace duckdb
