# DuckDB-INEGI Extension

A DuckDB extension for fetching data from INEGI (Instituto Nacional de Estadística y Geografía - Mexico's National Statistics Institute) APIs directly into DuckDB using SQL queries.

## Features

- **Token-based Authentication**: Secure session-based token management
- **Indicator Bank API**: Fetch statistical indicators from BIE/BISE
- **JSON-Stat Support**: Automatic parsing of JSON-Stat format responses
- **Flexible Queries**: Support for multiple languages, geographies, and time periods
- **SQL Integration**: Query INEGI data using familiar SQL syntax

## Installation

### Prerequisites

- CMake 3.5 or higher
- C++14 compatible compiler
- Git with submodules
- VCPKG (for dependency management)

### Clone and Build

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/<your-repo>/duck_inegi.git
cd duck_inegi

# Set up VCPKG (if not already installed)
cd <your-working-dir>
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg && git checkout ce613c41372b23b1f51333815feb3edd87ef8a8b
sh ./scripts/bootstrap.sh -disableMetrics
export VCPKG_TOOLCHAIN_PATH=`pwd`/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build the extension
cd duck_inegi
make release

# Or for faster builds with Ninja
GEN=ninja make release
```

## Quick Start

### 1. Get Your INEGI API Token

Before using the extension, you need to obtain an API token from INEGI:

1. Visit: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
2. Register and generate your token
3. Keep your token secure

### 2. Load the Extension

```sql
-- Start DuckDB with the extension
./build/release/duckdb

-- Load the extension (if not auto-loaded)
LOAD 'build/release/extension/inegi/inegi.duckdb_extension';
```

### 3. Set Your Token

```sql
-- Set your INEGI API token (required before any data queries)
SELECT INEGI_SetToken('your-token-here');
```

### 4. Fetch Data

```sql
-- Fetch indicator data (example: GDP indicator)
SELECT * FROM INEGI_Read('628194') LIMIT 10;

-- Fetch with custom parameters
SELECT * FROM INEGI_Read('628194', 
    language='en',
    geography='00',
    recent_only=true,
    bank='BIE'
);
```

## Available Functions

### INEGI_SetToken(token VARCHAR)

Sets the INEGI API token for the current session.

**Parameters:**
- `token` (VARCHAR): Your INEGI API token

**Returns:** BOOLEAN (true on success)

**Example:**
```sql
SELECT INEGI_SetToken('your-token-here');
```

### INEGI_GetToken()

Retrieves the currently set token (useful for debugging).

**Returns:** VARCHAR (the current token)

**Example:**
```sql
SELECT INEGI_GetToken();
```

### INEGI_Read(indicator_id VARCHAR, [options])

Fetches data for a specific indicator from the INEGI Indicator Bank API.

**Parameters:**
- `indicator_id` (VARCHAR, required): The INEGI indicator ID
- `language` (VARCHAR, optional): Language for responses - 'es' (Spanish) or 'en' (English). Default: 'es'
- `geography` (VARCHAR, optional): Geographic level - '00' (national), '99' (state), '999' (municipality). Default: '00'
- `recent_only` (BOOLEAN, optional): Fetch only recent data vs. full historical series. Default: false
- `bank` (VARCHAR, optional): Indicator bank - 'BIE' or 'BISE'. Default: 'BIE'

**Returns:** Table with columns:
- `time_period` (VARCHAR): Time period of the observation
- `observation_value` (DOUBLE): The indicator value
- `indicator_id` (VARCHAR): The indicator ID
- `indicator_name` (VARCHAR): Human-readable indicator name

**Examples:**
```sql
-- Basic usage with defaults (Spanish, national level, full history, BIE)
SELECT * FROM INEGI_Read('628194');

-- English language
SELECT * FROM INEGI_Read('628194', language='en');

-- State level data
SELECT * FROM INEGI_Read('628194', geography='99');

-- Only recent data
SELECT * FROM INEGI_Read('628194', recent_only=true);

-- Use BISE bank
SELECT * FROM INEGI_Read('628194', bank='BISE');

-- Combine multiple options
SELECT * FROM INEGI_Read('628194', 
    language='en',
    geography='00',
    recent_only=false,
    bank='BIE'
) LIMIT 100;
```

## Example Queries

### Analyze GDP Trends

```sql
-- Set token
SELECT INEGI_SetToken('your-token-here');

-- Fetch GDP data and analyze trends
SELECT 
    time_period,
    observation_value as gdp,
    LAG(observation_value) OVER (ORDER BY time_period) as prev_gdp,
    ((observation_value - LAG(observation_value) OVER (ORDER BY time_period)) / 
     LAG(observation_value) OVER (ORDER BY time_period) * 100) as growth_rate
FROM INEGI_Read('628194')
ORDER BY time_period DESC
LIMIT 20;
```

### Compare Multiple Indicators

```sql
-- Create a view combining multiple indicators
CREATE VIEW economic_indicators AS
SELECT 'GDP' as indicator, time_period, observation_value 
FROM INEGI_Read('628194')
UNION ALL
SELECT 'Inflation' as indicator, time_period, observation_value 
FROM INEGI_Read('628195')
UNION ALL
SELECT 'Unemployment' as indicator, time_period, observation_value 
FROM INEGI_Read('628196');

-- Query the combined view
SELECT * FROM economic_indicators 
WHERE time_period >= '2020-01-01'
ORDER BY time_period, indicator;
```

### Export to CSV

```sql
-- Export indicator data to CSV
COPY (
    SELECT * FROM INEGI_Read('628194')
) TO 'inegi_data.csv' (HEADER, DELIMITER ',');
```

## API Reference

### INEGI Indicator Bank API

The extension uses the INEGI Indicator Bank API:
- **Base URL**: `https://www.inegi.org.mx/app/api/indicadores/desarrolladores/`
- **Format**: JSON-Stat
- **Documentation**: https://www.inegi.org.mx/app/api/indicadores/desarrolladores/

### Finding Indicator IDs

To find indicator IDs:
1. Visit the INEGI Indicator Bank: https://www.inegi.org.mx/app/indicadores/
2. Browse or search for indicators
3. The indicator ID is shown in the URL or indicator details

Common indicators:
- `628194`: GDP (PIB)
- `628273`: Inflation (Inflación)
- `444603`: Unemployment Rate (Tasa de Desempleo)

## Development

### Running Tests

```bash
# Run all tests
make test

# Run specific test
./build/release/test/unittest "test/sql/inegi_token.test"
```

### Project Structure

```
duck_inegi/
├── src/
│   ├── inegi_extension.cpp          # Main extension implementation
│   ├── inegi_token_manager.cpp      # Token management
│   ├── inegi_api_client.cpp         # HTTP API client
│   └── include/
│       ├── inegi_extension.hpp
│       ├── inegi_token_manager.hpp
│       └── inegi_api_client.hpp
├── test/
│   └── sql/
│       ├── inegi.test               # Basic extension tests
│       ├── inegi_token.test         # Token management tests
│       └── inegi_read.test          # Data fetching tests
├── CMakeLists.txt
├── extension_config.cmake
└── vcpkg.json                       # Dependencies
```

### Dependencies

- **OpenSSL**: For secure HTTPS connections
- **CURL**: For HTTP requests
- **nlohmann-json**: For JSON parsing

## Troubleshooting

### Token Errors

**Error**: "INEGI API token not set"

**Solution**: Set your token using `SELECT INEGI_SetToken('your-token-here');`

### Network Errors

**Error**: "CURL request failed" or "HTTP request failed"

**Solutions**:
- Check your internet connection
- Verify the INEGI API is accessible
- Ensure your firewall allows HTTPS connections

### Invalid Indicator ID

**Error**: "HTTP request failed with status code: 404"

**Solution**: Verify the indicator ID exists in the INEGI database

### Build Errors

**Error**: "Could not find CURL" or "Could not find nlohmann_json"

**Solution**: Ensure VCPKG is properly configured and `VCPKG_TOOLCHAIN_PATH` is set

## Security Best Practices

1. **Never hardcode tokens** in your SQL scripts or source code
2. **Use environment variables** to store tokens
3. **Don't commit tokens** to version control
4. **Rotate tokens** periodically
5. **Limit token sharing** to authorized users only

## Roadmap

- [ ] Implement `INEGI_Indicators()` function to list available indicators
- [ ] Add DENUE API support for business directory data
- [ ] Implement caching for frequently accessed data
- [ ] Add filter pushdown optimization
- [ ] Support for Census and Geographic APIs
- [ ] Add data transformation utilities

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## License

See LICENSE file for details.

## Acknowledgments

- Inspired by the [duckdb-eurostat](https://github.com/ahuarte47/duckdb-eurostat) extension
- Built with the [DuckDB Extension Template](https://github.com/duckdb/extension-template)
- Data provided by [INEGI](https://www.inegi.org.mx/)

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/<your-repo>/duck_inegi/issues
- INEGI API Documentation: https://www.inegi.org.mx/app/api/

## References

- INEGI Official Website: https://www.inegi.org.mx/
- INEGI API Portal: https://www.inegi.org.mx/app/desarrolladores/
- DuckDB Documentation: https://duckdb.org/docs/
