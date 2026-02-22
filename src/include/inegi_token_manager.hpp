#pragma once

#include "duckdb.hpp"
#include "duckdb/main/client_context.hpp"

namespace duckdb {

class INEGITokenManager {
public:
    static void SetToken(ClientContext &context, const string &token);
    static string GetToken(ClientContext &context);
    static bool HasToken(ClientContext &context);
    static void ClearToken(ClientContext &context);
    
private:
    static constexpr const char* TOKEN_KEY = "inegi_api_token";
};

} // namespace duckdb
