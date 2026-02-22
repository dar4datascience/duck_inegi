# DuckDB-INEGI Extension - Quick Start Guide

## What You've Built

A DuckDB extension that allows you to query INEGI (Mexico's National Statistics Institute) data directly using SQL!

## Two Ways to Build

### Option 1: Use GitHub Actions (Recommended - No Local Compilation!)

**Advantages:**
- ✅ No need to compile on your machine
- ✅ Builds for Linux, macOS, and Windows automatically
- ✅ Just push to GitHub and download pre-built binaries
- ✅ Takes 10-15 minutes on GitHub's servers (not your computer!)

**Steps:**
1. Push your code to GitHub:
   ```bash
   git add .
   git commit -m "INEGI extension ready"
   git push origin main
   ```

2. Go to your GitHub repository → **Actions** tab

3. Wait for the build to complete (~10-15 minutes)

4. Download the artifact for your platform:
   - `linux-extension` - For Linux
   - `macos-extension` - For macOS  
   - `windows-extension` - For Windows

5. Extract and use:
   ```bash
   unzip linux-extension.zip
   duckdb -unsigned
   ```
   ```sql
   LOAD './inegi.duckdb_extension';
   SELECT INEGI_SetToken('your-token-here');
   SELECT * FROM INEGI_Read('628194') LIMIT 10;
   ```

**See full details:** `docs/GITHUB_ACTIONS.md`

### Option 2: Build Locally

**Requirements:**
- CMake 3.5+
- C++14 compiler
- vcpkg (for dependencies)

**Steps:**
```bash
# Set up vcpkg
export VCPKG_TOOLCHAIN_PATH=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build (first time takes 10-15 minutes)
make release

# Or faster with Ninja
GEN=ninja make release

# Run tests
make test

# Use the extension
./build/release/duckdb
```

## Get Your INEGI API Token

Before using the extension, get a free API token:

1. Visit: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
2. Register and generate your token
3. Keep it secure!

## Basic Usage

```sql
-- Load the extension
LOAD 'build/release/extension/inegi/inegi.duckdb_extension';

-- Set your token (required!)
SELECT INEGI_SetToken('your-token-here');

-- Fetch GDP data
SELECT * FROM INEGI_Read('628194') LIMIT 10;

-- Fetch inflation data in English
SELECT * FROM INEGI_Read('628273', language='en') LIMIT 10;

-- Get only recent data (faster)
SELECT * FROM INEGI_Read('628194', recent_only=true);

-- State-level data
SELECT * FROM INEGI_Read('444603', geography='99') LIMIT 10;
```

## Available Functions

### INEGI_SetToken(token)
Set your API token for the session.

### INEGI_GetToken()
Get the current token (for debugging).

### INEGI_Read(indicator_id, [options])
Fetch indicator data from INEGI.

**Options:**
- `language`: 'es' or 'en' (default: 'es')
- `geography`: '00' (national), '99' (state), '999' (municipality) (default: '00')
- `recent_only`: true/false (default: false)
- `bank`: 'BIE' or 'BISE' (default: 'BIE')

## Example Queries

See `docs/EXAMPLES.md` for 50+ example queries including:
- Economic indicators (GDP, inflation, unemployment)
- Time series analysis
- Multi-indicator dashboards
- Data exports (CSV, Parquet, JSON)
- Statistical analysis
- And much more!

## Documentation

- **README.md** - Full documentation and installation
- **docs/EXAMPLES.md** - 50+ usage examples and queries
- **docs/GITHUB_ACTIONS.md** - How to build with GitHub Actions
- **docs/README.md** - DuckDB extension template documentation

## Common Indicator IDs

```sql
-- GDP (PIB)
SELECT * FROM INEGI_Read('628194');

-- Inflation
SELECT * FROM INEGI_Read('628273');

-- Unemployment Rate
SELECT * FROM INEGI_Read('444603');

-- Exchange Rate
SELECT * FROM INEGI_Read('628168');
```

Find more indicators at: https://www.inegi.org.mx/app/indicadores/

## Troubleshooting

**"Token not set" error:**
```sql
SELECT INEGI_SetToken('your-token-here');
```

**Build fails locally:**
- Use GitHub Actions instead! (See Option 1 above)
- Or check that vcpkg is properly configured

**Network errors:**
- Check internet connection
- Verify INEGI API is accessible
- Try with `recent_only=true` for faster response

## Project Structure

```
duck_inegi/
├── src/
│   ├── inegi_extension.cpp       # Main extension
│   ├── inegi_token_manager.cpp   # Token management
│   ├── inegi_api_client.cpp      # HTTP API client
│   └── include/                  # Header files
├── test/sql/                     # SQL tests
├── docs/
│   ├── EXAMPLES.md              # Usage examples
│   └── GITHUB_ACTIONS.md        # CI/CD guide
├── .github/workflows/
│   ├── CI.yml                   # Continuous integration
│   └── MainDistributionPipeline.yml  # Release builds
└── README.md                     # Full documentation
```

## Next Steps

1. **Get your INEGI token** (if you haven't already)
2. **Choose your build method:**
   - GitHub Actions (recommended) - See `docs/GITHUB_ACTIONS.md`
   - Local build - Run `make release`
3. **Try the examples** in `docs/EXAMPLES.md`
4. **Start querying INEGI data!**

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## Support

- **GitHub Issues**: Report bugs or request features
- **INEGI API Docs**: https://www.inegi.org.mx/servicios/api_indicadores.html
- **DuckDB Docs**: https://duckdb.org/docs/

---

**Happy querying! 🦆📊🇲🇽**
