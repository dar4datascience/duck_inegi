---
description: Complete development guide for DuckDB-INEGI extension
auto_execution_mode: 2
---

# DuckDB-INEGI Extension Development Workflow

This workflow guides you through developing a DuckDB extension that fetches data from the INEGI (Instituto Nacional de Estadística y Geografía) API, similar to the duckdb-eurostat extension.

## Project Overview

**Goal**: Create a DuckDB extension that allows users to query INEGI data using SQL.

**Reference Projects**:
- Structure: https://github.com/ahuarte47/duckdb-eurostat
- Python API Example: https://github.com/dar4datascience/DatosMex/tree/master/INEGIpy

**Key Requirements**:
- Token-based authentication (required for INEGI API)
- Token can be obtained at: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
- Token should be cached per session
- Support for multiple INEGI APIs (Indicator Bank, DENUE, etc.)

## Development Steps

### 1. Set Up Development Environment

Ensure you have the required dependencies:
- CMake 3.5 or higher
- C++14 compatible compiler
- Ninja (recommended): `GEN=ninja`
- Git with submodules

Clone and initialize submodules:
```bash
git submodule update --init --recursive
```

### 2. Plan Core Functions

Based on the EUROSTAT extension pattern, implement these core functions:

**a. INEGI_SetToken(token VARCHAR)**
- Store the API token for the session
- Validate token format
- Cache token in session state

**b. INEGI_Indicators()**
- List available indicators from the Indicator Bank API
- Return: indicator_id, name, description, unit, source

**c. INEGI_Read(indicator_id VARCHAR, [options])**
- Fetch data for a specific indicator
- Support optional parameters:
  - language: 'es' or 'en' (default: 'es')
  - geography: '00' (national), '99' (state), '999' (municipality)
  - recent_only: true/false (latest vs. full historical series)
  - bank: 'BIE' or 'BISE'
- Return data as a table with time_period and observation_value columns

**d. INEGI_DENUE(query_params)**
- Query the DENUE (business directory) API
- Support location-based queries

### 3. Implement Token Management

Create a token manager class in C++:

```cpp
// In src/include/inegi_token_manager.hpp
class INEGITokenManager {
private:
    static string session_token;
    static bool token_validated;
    
public:
    static void SetToken(const string &token);
    static string GetToken();
    static bool HasToken();
    static void ValidateToken();
};
```

Key considerations:
- Check for token before any API call
- Throw informative error if token is missing
- Provide clear instructions on how to obtain a token

### 4. Implement HTTP Client for API Calls

Add HTTP library dependency to vcpkg.json:
- Consider: libcurl, cpp-httplib, or cpr
- Update CMakeLists.txt to link the HTTP library

Create API client class:
```cpp
// In src/include/inegi_api_client.hpp
class INEGIAPIClient {
private:
    string base_url = "https://www.inegi.org.mx/app/api/";
    string token;
    
public:
    INEGIAPIClient(const string &token);
    string FetchIndicators();
    string FetchIndicatorData(const string &indicator_id, const map<string, string> &params);
    string FetchDENUE(const map<string, string> &params);
};
```

### 5. Implement Data Parsing

INEGI API returns data in JSON-Stat or Pc-Axis format:
- Add JSON parsing library (nlohmann/json is already common in DuckDB extensions)
- Parse JSON-Stat format responses
- Convert to DuckDB table format

### 6. Register Functions in Extension

Update `src/quack_extension.cpp` (rename to `inegi_extension.cpp`):

```cpp
static void LoadInternal(ExtensionLoader &loader) {
    // Token management
    auto set_token_function = ScalarFunction("INEGI_SetToken", 
        {LogicalType::VARCHAR}, LogicalType::BOOLEAN, INEGISetTokenFun);
    loader.RegisterFunction(set_token_function);
    
    // Table functions for data retrieval
    auto indicators_function = TableFunction("INEGI_Indicators", 
        {}, INEGIIndicatorsFun);
    loader.RegisterFunction(indicators_function);
    
    auto read_function = TableFunction("INEGI_Read", 
        {LogicalType::VARCHAR}, INEGIReadFun);
    loader.RegisterFunction(read_function);
}
```

### 7. Update Build Configuration

**a. Update CMakeLists.txt**:
- Change TARGET_NAME from "quack" to "inegi"
- Add HTTP library dependencies
- Add JSON library if needed

**b. Update extension_config.cmake**:
- Update extension name and description

**c. Update vcpkg.json**:
- Add required dependencies (HTTP client, JSON parser)

### 8. Write Tests

Create SQL tests in `test/sql/`:

**a. test/sql/inegi_token.test**:
```sql
# Test token setting
statement ok
SELECT INEGI_SetToken('your-test-token-here');

# Test missing token error
statement error
SELECT * FROM INEGI_Indicators();
```

**b. test/sql/inegi_indicators.test**:
```sql
# Test fetching indicators
query IIIII
SELECT * FROM INEGI_Indicators() LIMIT 5;
```

**c. test/sql/inegi_read.test**:
```sql
# Test reading indicator data
query II
SELECT * FROM INEGI_Read('1234567') LIMIT 10;
```

### 9. Build and Test

Build the extension:
```bash
make clean
make release
```

Run tests:
```bash
make test
```

Test manually:
```bash
./build/release/duckdb
```

Then in DuckDB:
```sql
LOAD 'build/release/extension/inegi/inegi.duckdb_extension';
SELECT INEGI_SetToken('your-token-here');
SELECT * FROM INEGI_Indicators() LIMIT 5;
```

### 10. Documentation

Create comprehensive documentation in `docs/`:

**a. docs/README.md**:
- Overview of the extension
- Installation instructions
- Quick start guide

**b. docs/API.md**:
- Detailed function reference
- Parameter descriptions
- Example queries

**c. docs/INEGI_APIS.md**:
- Information about INEGI APIs
- How to obtain a token
- API limitations and rate limits

### 11. Implement Filter Pushdown (Advanced)

Similar to EUROSTAT extension, implement filter pushdown:
- Detect WHERE clauses in SQL
- Convert to INEGI API query parameters
- Reduce data transfer and improve performance

### 12. Error Handling

Implement robust error handling:
- Invalid token errors
- Network errors
- API rate limit errors
- Invalid indicator IDs
- Malformed responses

### 13. Prepare for Distribution

**a. Update README.md**:
- Clear installation instructions
- Usage examples
- Token acquisition guide

**b. Set up CI/CD**:
- GitHub Actions workflows are in `.github/workflows/`
- Ensure tests pass on multiple platforms

**c. Version management**:
- Update version in extension_config.cmake
- Tag releases appropriately

## Common Commands Reference

```bash
# Build
make release

# Build with Ninja (faster)
GEN=ninja make release

# Run tests
make test

# Clean build
make clean

# Format code
make format

# Run specific test
./build/release/test/unittest "test/sql/inegi_token.test"
```

## API Endpoints Reference

**Indicator Bank API**:
- Base URL: `https://www.inegi.org.mx/app/api/indicadores/desarrolladores/`
- Format: `jsonStat` or `pcAxis`
- Requires: token parameter

**DENUE API**:
- Base URL: `https://www.inegi.org.mx/app/api/denue/v1/`
- Requires: token parameter

## Token Management Best Practices

1. **Never hardcode tokens** in source code
2. **Session-based storage**: Store token in DuckDB session state
3. **Clear error messages**: Guide users to token registration page
4. **Validation**: Check token format before API calls
5. **Security**: Don't log tokens in debug output

## Troubleshooting

**Build errors**:
- Ensure all submodules are initialized
- Check CMake version (>= 3.5)
- Verify C++14 compiler support

**API errors**:
- Verify token is valid
- Check network connectivity
- Review INEGI API documentation for changes

**Test failures**:
- Ensure test token is set
- Check API availability
- Review test expectations vs. actual API responses

## Next Steps After Initial Implementation

1. Add support for more INEGI APIs (Census, Geographic data)
2. Implement caching for frequently accessed data
3. Add data transformation functions
4. Create example notebooks/tutorials
5. Submit to DuckDB Community Extensions
