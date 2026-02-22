#include "inegi_api_client.hpp"
#include "duckdb/common/exception.hpp"
#include <curl/curl.h>
#include <sstream>

namespace duckdb {

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

INEGIAPIClient::INEGIAPIClient(const string &token) : token(token) {
    if (token.empty()) {
        throw InvalidInputException("INEGI API token is required");
    }
}

string INEGIAPIClient::MakeHTTPRequest(const string &url) {
    CURL *curl;
    CURLcode res;
    string response_string;
    
    curl = curl_easy_init();
    if (!curl) {
        throw IOException("Failed to initialize CURL");
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        string error_msg = "CURL request failed: ";
        error_msg += curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        throw IOException(error_msg);
    }
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    
    if (response_code != 200) {
        throw IOException("HTTP request failed with status code: " + std::to_string(response_code));
    }
    
    return response_string;
}

string INEGIAPIClient::BuildIndicatorURL(const string &indicator_id, 
                                         const string &language,
                                         const string &geography,
                                         bool recent_only,
                                         const string &bank) {
    std::ostringstream url;
    url << BASE_URL_INDICATORS << "/" << bank << "/" << indicator_id;
    url << "/" << language;
    url << "/" << geography;
    url << "/" << (recent_only ? "true" : "false");
    url << "?type=json";
    url << "&token=" << token;
    return url.str();
}

nlohmann::json INEGIAPIClient::FetchIndicators(const string &language) {
    throw NotImplementedException("INEGI_Indicators is not yet implemented. This will list available indicators.");
}

nlohmann::json INEGIAPIClient::FetchIndicatorData(const string &indicator_id, 
                                                   const string &language,
                                                   const string &geography,
                                                   bool recent_only,
                                                   const string &bank) {
    string url = BuildIndicatorURL(indicator_id, language, geography, recent_only, bank);
    string response = MakeHTTPRequest(url);
    
    try {
        return nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw IOException("Failed to parse JSON response from INEGI API: " + string(e.what()));
    }
}

nlohmann::json INEGIAPIClient::FetchDENUE(const std::map<string, string> &params) {
    throw NotImplementedException("INEGI_DENUE is not yet implemented");
}

} // namespace duckdb
