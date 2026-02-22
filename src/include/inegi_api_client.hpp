#pragma once

#include "duckdb.hpp"
#include <string>
#include <map>
#include <nlohmann/json.hpp>

namespace duckdb {

class INEGIAPIClient {
public:
    explicit INEGIAPIClient(const string &token);
    
    nlohmann::json FetchIndicators(const string &language = "es");
    nlohmann::json FetchIndicatorData(const string &indicator_id, 
                                      const string &language = "es",
                                      const string &geography = "00",
                                      bool recent_only = false,
                                      const string &bank = "BIE");
    
    nlohmann::json FetchDENUE(const std::map<string, string> &params);
    
private:
    string token;
    static constexpr const char* BASE_URL_INDICATORS = "https://www.inegi.org.mx/app/api/indicadores/desarrolladores/jsonStat";
    static constexpr const char* BASE_URL_DENUE = "https://www.inegi.org.mx/app/api/denue/v1";
    
    string MakeHTTPRequest(const string &url);
    string BuildIndicatorURL(const string &indicator_id, 
                            const string &language,
                            const string &geography,
                            bool recent_only,
                            const string &bank);
};

} // namespace duckdb
