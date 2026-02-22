# Testing the INEGI Extension

This document explains the testing strategy and how to run tests locally and in CI/CD.

## Test Structure

The extension has three types of tests:

### 1. Unit Tests
**Files:** `test/sql/inegi.test`, `test/sql/inegi_token.test`

**Purpose:** Test basic functionality without making real API calls

**Requirements:** None (no token needed)

**What they test:**
- Extension loads correctly
- Token management (set/get)
- Error handling when token is missing
- Function signatures

**Run locally:**
```bash
make test
```

### 2. Integration Tests
**File:** `test/sql/inegi_integration.test`

**Purpose:** Test real API calls to INEGI

**Requirements:** Valid INEGI API token

**What they test:**
- Real data fetching from INEGI API
- JSON-Stat parsing
- Data structure validation
- Different parameter combinations (language, geography, etc.)

**Run locally:**
```bash
# Set token as environment variable
export INEGI_TOKEN='your-token-here'

# Run tests
make test
```

**Behavior:**
- If `INEGI_TOKEN` is set → Runs real API calls
- If `INEGI_TOKEN` is NOT set → Skips API calls (tests pass without errors)

### 3. API-Specific Tests
**File:** `test/sql/inegi_read.test`

**Purpose:** Test INEGI_Read function behavior

**Requirements:** Token (can be dummy for basic tests)

**What they test:**
- Error handling without token
- Token from environment variable usage
- Function parameter validation

## Running Tests Locally

### Without Token (Unit Tests Only)

```bash
# Build the extension
make release

# Run tests (integration tests will be skipped)
make test
```

### With Token (All Tests)

```bash
# Set your INEGI token
export INEGI_TOKEN='your-actual-token-here'

# Build and test
make release
make test
```

### Run Specific Test File

```bash
# Run only token tests
./build/release/test/unittest "test/sql/inegi_token.test"

# Run only integration tests
./build/release/test/unittest "test/sql/inegi_integration.test"

# Run with token
INEGI_TOKEN='your-token' ./build/release/test/unittest "test/sql/inegi_integration.test"
```

## Running Tests in GitHub Actions

### Setup (One-Time)

1. Get your INEGI API token:
   - Visit: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
   - Register and generate token

2. Add token to GitHub Secrets:
   - Go to repository **Settings**
   - Click **Secrets and variables** → **Actions**
   - Click **New repository secret**
   - Name: `INEGI_TOKEN`
   - Value: Your token
   - Click **Add secret**

**Detailed guide:** See `.github/SETUP_SECRETS.md`

### Automatic Testing

Once the secret is set, tests run automatically on:
- Every push to `main` branch
- Every pull request
- Manual workflow trigger

**View test results:**
1. Go to **Actions** tab
2. Click on a workflow run
3. Expand test steps to see results

## Test Coverage

### Current Coverage

| Component | Unit Tests | Integration Tests |
|-----------|------------|-------------------|
| Extension Loading | ✅ | ✅ |
| Token Management | ✅ | ✅ |
| INEGI_SetToken | ✅ | ✅ |
| INEGI_GetToken | ✅ | ✅ |
| INEGI_Read (basic) | ✅ | ✅ |
| INEGI_Read (language) | ⚠️ | ✅ |
| INEGI_Read (geography) | ⚠️ | ✅ |
| INEGI_Read (recent_only) | ⚠️ | ✅ |
| JSON-Stat parsing | ⚠️ | ✅ |
| Error handling | ✅ | ✅ |

✅ = Covered  
⚠️ = Partially covered

### Future Test Additions

- [ ] INEGI_Indicators() function tests
- [ ] DENUE API tests
- [ ] Filter pushdown tests
- [ ] Performance tests
- [ ] Edge case handling (malformed responses, timeouts)

## Writing New Tests

### Test File Template

```sql
# name: test/sql/your_test.test
# description: Description of what this tests
# group: [inegi]

require inegi

# Test without token (should fail)
statement error
SELECT * FROM INEGI_Read('123');
----
INEGI API token not set

# Set token from environment or use dummy
statement ok
SELECT INEGI_SetToken(COALESCE(getenv('INEGI_TOKEN'), 'test-token'));

# Your test cases here
query I
SELECT COUNT(*) > 0 FROM INEGI_Read('628194', recent_only=true);
----
true
```

### Integration Test Template

```sql
# For tests that require real API calls
statement ok
CREATE TABLE env_check AS SELECT getenv('INEGI_TOKEN') as token;

statement ok
CREATE TEMP MACRO has_token() AS (SELECT token IS NOT NULL AND token != '' FROM env_check);

# Only run if token is available
query I
SELECT CASE 
    WHEN has_token() THEN (
        -- Your real API test here
        SELECT COUNT(*) > 0 FROM INEGI_Read('628194')
    )
    ELSE true
END;
----
true
```

## Continuous Integration Workflows

### CI Workflow (`.github/workflows/CI.yml`)

**Triggers:**
- Push to main/master
- Pull requests
- Manual dispatch

**Platforms:**
- Linux (Ubuntu)
- macOS
- Windows

**Environment:**
- `INEGI_TOKEN` from GitHub Secrets
- All dependencies via vcpkg

**Steps:**
1. Install dependencies
2. Checkout code
3. Setup vcpkg
4. Build extension
5. Run tests (with token if available)
6. Upload artifacts

### Distribution Pipeline (`.github/workflows/MainDistributionPipeline.yml`)

**Triggers:**
- Push to any branch
- Pull requests
- Manual dispatch

**What it does:**
- Builds for all platforms
- Runs code quality checks
- Creates release binaries
- Runs tests with token

## Debugging Failed Tests

### Local Debugging

```bash
# Run with verbose output
./build/release/test/unittest --verbose "test/sql/inegi_integration.test"

# Run DuckDB CLI for manual testing
./build/release/duckdb

# In DuckDB:
LOAD 'build/release/extension/inegi/inegi.duckdb_extension';
SELECT INEGI_SetToken(getenv('INEGI_TOKEN'));
SELECT * FROM INEGI_Read('628194', recent_only=true);
```

### GitHub Actions Debugging

1. Go to **Actions** tab
2. Click on failed workflow
3. Click on failed job
4. Expand the "Run tests" step
5. Look for error messages

**Common issues:**
- Token not set → Add `INEGI_TOKEN` secret
- Network timeout → INEGI API may be down
- Invalid token → Regenerate token on INEGI website

## Test Best Practices

### DO:
- ✅ Use `COALESCE(getenv('INEGI_TOKEN'), 'test-token')` for flexibility
- ✅ Make integration tests conditional on token availability
- ✅ Test error cases (missing token, invalid IDs, etc.)
- ✅ Use `recent_only=true` for faster tests
- ✅ Clean up temporary tables/macros

### DON'T:
- ❌ Hardcode tokens in test files
- ❌ Make tests fail if token is not available
- ❌ Fetch large datasets in tests (use LIMIT)
- ❌ Commit tokens to repository
- ❌ Log sensitive information

## Performance Considerations

### Test Speed

- **Unit tests**: < 1 second
- **Integration tests (with token)**: 5-10 seconds
- **Full test suite**: 10-15 seconds

### Optimization Tips

1. Use `recent_only=true` to fetch less data
2. Use `LIMIT` in queries
3. Cache frequently used test data (future enhancement)
4. Run integration tests only when needed

## Security

### Token Security

- Tokens are stored as GitHub Secrets (encrypted)
- Tokens are masked in workflow logs
- Tokens are never committed to repository
- Environment variables are cleared after tests

### Best Practices

1. Use separate tokens for testing and production
2. Rotate tokens periodically
3. Limit token permissions if possible
4. Monitor token usage

## Summary

- **Unit tests**: Always run, no token needed
- **Integration tests**: Run if `INEGI_TOKEN` is set
- **GitHub Actions**: Add `INEGI_TOKEN` secret for full testing
- **Local testing**: Export `INEGI_TOKEN` environment variable

For more information:
- GitHub Secrets setup: `.github/SETUP_SECRETS.md`
- Detailed secrets guide: `docs/GITHUB_SECRETS.md`
- Usage examples: `docs/EXAMPLES.md`
