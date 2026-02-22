# Testing Approach for DuckDB-INEGI Extension

## Current Testing Strategy

### What the Tests Actually Do

The current test suite **does NOT make real API calls to INEGI**. Here's what each test file does:

#### 1. `test/sql/inegi.test`
- ✅ Verifies extension loads correctly
- ✅ Tests `INEGI_SetToken()` function exists and accepts a token
- ✅ Tests `INEGI_GetToken()` function exists and returns the stored token
- ❌ Does NOT test real API calls

#### 2. `test/sql/inegi_token.test`
- ✅ Tests token validation (empty token rejection)
- ✅ Tests token persistence in session
- ❌ Does NOT validate against real INEGI API

#### 3. `test/sql/inegi_read.test`
- ✅ Tests that `INEGI_Read()` requires a token
- ✅ Uses a fake test token: `'test-token-12345'`
- ❌ Does NOT make real API requests

#### 4. `test/sql/inegi_integration.test`
- ✅ Tests basic token management
- ❌ Does NOT test real API integration (despite the name)

## Why Real API Tests Don't Work

### The Problem

**DuckDB static builds don't have access to `getenv()`**, which means:

1. The `INEGI_TOKEN` secret you set in GitHub Actions is available as a **shell environment variable**
2. But there's no way to pass it into the SQL test files
3. The `getenv()` SQL function doesn't exist in static DuckDB builds

### What Happens in GitHub Actions

```yaml
# In .github/workflows/CI.yml
env:
  INEGI_TOKEN: ${{ secrets.INEGI_TOKEN }}  # ✅ Available as shell env var
```

```sql
-- In test files
SELECT getenv('INEGI_TOKEN');  -- ❌ FAILS: Function doesn't exist
```

### Current Workaround

Tests use **fake tokens** and only verify:
- Functions are callable
- Token storage/retrieval works
- Error messages are correct
- Extension loads properly

## What Gets Tested in CI/CD

### GitHub Actions Workflow Tests

1. **Build Tests** ✅
   - Extension compiles on Linux, macOS, Windows
   - No compilation errors
   - All dependencies resolve

2. **Unit Tests** ✅
   - Extension loads
   - Functions exist
   - Token management works
   - Error handling works

3. **Integration Tests** ❌
   - **NOT TESTED**: Real API calls to INEGI
   - **NOT TESTED**: JSON-Stat parsing with real data
   - **NOT TESTED**: Network connectivity
   - **NOT TESTED**: API authentication

## Manual Testing Required

To actually test the extension with real INEGI API:

### Local Manual Testing

```bash
# 1. Build the extension
make release

# 2. Start DuckDB with the extension
./build/release/duckdb

# 3. Load extension and set your real token
LOAD 'build/release/extension/inegi/inegi.duckdb_extension';
SELECT INEGI_SetToken('your-real-token-here');

# 4. Test real API call
SELECT * FROM INEGI_Read('628194', recent_only=true);
```

### What to Test Manually

- [ ] Token authentication works with INEGI API
- [ ] Data is fetched successfully
- [ ] JSON-Stat parsing works correctly
- [ ] Different parameters (language, geography, bank) work
- [ ] Error handling for invalid indicators
- [ ] Error handling for network issues
- [ ] Error handling for invalid tokens

## Future Improvements

### Option 1: DuckDB Secrets API (Recommended)

Implement proper DuckDB Secrets API instead of custom token management:

```sql
-- Future approach
CREATE SECRET inegi_token (
    TYPE INEGI,
    TOKEN 'your-token-here'
);

-- Or from environment
CREATE SECRET inegi_token (
    TYPE INEGI,
    TOKEN FROM ENV 'INEGI_TOKEN'
);
```

**Benefits**:
- Standard DuckDB pattern
- Better security
- Environment variable support
- Persistent secrets

### Option 2: Test with Mock Server

Create a mock INEGI API server for testing:

```python
# test/mock_inegi_server.py
# Returns fake but valid JSON-Stat responses
```

**Benefits**:
- Tests run without real API
- No token needed
- Fast and reliable
- Can test error cases

### Option 3: Separate Integration Test Suite

Create a separate test suite that only runs when explicitly enabled:

```bash
# Only run with real token
INEGI_TOKEN=xxx make test-integration
```

## Current Recommendation

**For now, the current approach is acceptable because**:

1. ✅ **Build verification works** - Extension compiles correctly
2. ✅ **Function signatures verified** - All functions exist and are callable
3. ✅ **Token management tested** - Storage/retrieval works
4. ✅ **Error handling tested** - Proper error messages
5. ⚠️ **Manual testing required** - Real API calls must be tested manually

**The extension will work correctly** when users provide a real token, even though automated tests don't verify this.

## Testing Checklist for Releases

Before releasing a new version, manually verify:

- [ ] Extension builds on all platforms (GitHub Actions)
- [ ] All unit tests pass (GitHub Actions)
- [ ] Manual test with real INEGI token works
- [ ] At least one real API call succeeds
- [ ] Error handling works with invalid token
- [ ] Documentation is up to date

## Summary

**What's tested automatically**: Extension structure, function existence, token management  
**What's NOT tested automatically**: Real API calls, actual data fetching  
**Why**: `getenv()` not available in static builds, can't pass secrets to SQL tests  
**Solution**: Manual testing required for real API verification  
**Future**: Implement DuckDB Secrets API or mock server for better testing
