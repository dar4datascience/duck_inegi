# Local Testing Setup Guide

This guide walks you through setting up your local environment to test the DuckDB-INEGI extension.

## Prerequisites

Before you start, ensure you have:

- **CMake 3.5+**: Build system
- **C++14 compatible compiler**: GCC 7+, Clang 5+, or MSVC 2017+
- **Git**: With submodules support
- **Python 3.7+**: For build scripts
- **INEGI API Token**: Get from https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify

### Optional (Recommended)
- **Ninja**: Faster builds (`sudo apt-get install ninja-build` on Ubuntu)
- **ccache**: Faster rebuilds (`sudo apt-get install ccache` on Ubuntu)
- **vcpkg**: Dependency management (see below)

## Step-by-Step Setup

### 1. Install System Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    ccache \
    libcurl4-openssl-dev \
    libssl-dev \
    python3
```

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake ninja ccache curl openssl python3
```

#### Windows
```powershell
# Install Chocolatey if not already installed
# Then install dependencies
choco install cmake ninja git python3
```

### 2. Clone the Repository

```bash
# Clone with submodules (IMPORTANT!)
git clone --recurse-submodules https://github.com/YOUR_USERNAME/duck_inegi.git
cd duck_inegi

# If you already cloned without --recurse-submodules, run:
git submodule update --init --recursive
```

### 3. Set Up vcpkg (Dependency Manager)

vcpkg manages dependencies like CURL and nlohmann-json.

```bash
# Navigate to a directory outside your project (e.g., home directory)
cd ~

# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Checkout specific version (matches what CI uses)
git checkout ce613c41372b23b1f51333815feb3edd87ef8a8b

# Bootstrap vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
# OR
.\bootstrap-vcpkg.bat  # Windows

# Set environment variable (add to ~/.bashrc or ~/.zshrc for persistence)
export VCPKG_TOOLCHAIN_PATH=~/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**Make it permanent** (Linux/macOS):
```bash
echo 'export VCPKG_TOOLCHAIN_PATH=~/vcpkg/scripts/buildsystems/vcpkg.cmake' >> ~/.bashrc
source ~/.bashrc
```

### 4. Get Your INEGI API Token

1. Visit: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
2. Register for a free account
3. Generate your API token
4. Copy the token (you'll need it for testing)

### 5. Set Environment Variables

```bash
# Set your INEGI token (required for integration tests)
export INEGI_TOKEN='your-actual-token-here'

# Optional: Use Ninja for faster builds
export GEN=ninja
```

**Make it permanent** (Linux/macOS):
```bash
# Add to ~/.bashrc or ~/.zshrc
echo 'export INEGI_TOKEN="your-actual-token-here"' >> ~/.bashrc
echo 'export GEN=ninja' >> ~/.bashrc
source ~/.bashrc
```

**For Windows** (PowerShell):
```powershell
$env:INEGI_TOKEN = "your-actual-token-here"
$env:GEN = "ninja"

# Make permanent
[System.Environment]::SetEnvironmentVariable('INEGI_TOKEN', 'your-actual-token-here', 'User')
```

## Building the Extension

### Standard Build

```bash
# Navigate to project directory
cd ~/path/to/duck_inegi

# Clean previous builds (optional)
make clean

# Build the extension
make release
```

### Faster Build with Ninja

```bash
# Set Ninja as generator
export GEN=ninja

# Build
make release
```

### Build Output

After successful build, you'll find:
- **Extension binary**: `build/release/extension/inegi/inegi.duckdb_extension`
- **DuckDB CLI**: `build/release/duckdb`
- **Test runner**: `build/release/test/unittest`

## Running Tests

### Run All Tests

```bash
# From project root
make test
```

This runs:
- Unit tests (no API calls)
- Integration tests (real API calls if `INEGI_TOKEN` is set)

### Run Specific Test File

```bash
# Run only token tests
./build/release/test/unittest "test/sql/inegi_token.test"

# Run only integration tests
./build/release/test/unittest "test/sql/inegi_integration.test"

# Run basic extension tests
./build/release/test/unittest "test/sql/inegi.test"
```

### Run Tests with Verbose Output

```bash
./build/release/test/unittest --verbose "test/sql/inegi_integration.test"
```

### Run Tests Without Token

If you don't have a token or want to skip integration tests:

```bash
# Unset the token
unset INEGI_TOKEN

# Run tests (integration tests will be skipped)
make test
```

## Manual Testing with DuckDB CLI

### Start DuckDB with Extension

```bash
# Start DuckDB CLI (extension is pre-loaded)
./build/release/duckdb
```

### In DuckDB Shell

```sql
-- Extension is already loaded, just set your token
SELECT INEGI_SetToken(getenv('INEGI_TOKEN'));

-- Or set it manually
SELECT INEGI_SetToken('your-token-here');

-- Verify token is set
SELECT INEGI_GetToken();

-- Test fetching data
SELECT * FROM INEGI_Read('628194', recent_only=true);

-- Test with different parameters
SELECT * FROM INEGI_Read('628194', 
    language='en',
    geography='00',
    recent_only=true
) LIMIT 10;

-- Exit DuckDB
.quit
```

## Common Build Issues & Solutions

### Issue: "Could not find vcpkg"

**Solution**:
```bash
# Ensure VCPKG_TOOLCHAIN_PATH is set
echo $VCPKG_TOOLCHAIN_PATH

# If empty, set it:
export VCPKG_TOOLCHAIN_PATH=~/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Issue: "Submodule 'duckdb' not found"

**Solution**:
```bash
git submodule update --init --recursive
```

### Issue: "CURL not found" or "nlohmann-json not found"

**Solution**:
```bash
# vcpkg should install these automatically
# If not, install manually:
cd ~/vcpkg
./vcpkg install curl nlohmann-json

# On Linux, you can also use system packages:
sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev
```

### Issue: Build is very slow

**Solutions**:
1. Use Ninja: `export GEN=ninja && make release`
2. Install ccache: `sudo apt-get install ccache`
3. Use multiple cores: `make release -j$(nproc)`

### Issue: "Token not set" error in tests

**Solution**:
```bash
# Set the token environment variable
export INEGI_TOKEN='your-token-here'

# Verify it's set
echo $INEGI_TOKEN

# Run tests again
make test
```

### Issue: Tests timeout or fail with network errors

**Possible causes**:
- INEGI API is down (check https://www.inegi.org.mx/)
- Network connectivity issues
- Firewall blocking HTTPS requests
- Invalid or expired token

**Solution**:
```bash
# Test network connectivity
curl -I https://www.inegi.org.mx/

# Regenerate token if needed
# Visit: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
```

## Development Workflow

### Typical Development Cycle

```bash
# 1. Make code changes
vim src/inegi_extension.cpp

# 2. Rebuild (incremental build is fast with ccache)
make release

# 3. Run tests
make test

# 4. Manual testing if needed
./build/release/duckdb
```

### Quick Iteration

```bash
# For quick testing during development
make release && make test
```

### Clean Rebuild

```bash
# When you need a fresh build
make clean
make release
```

## Testing Checklist

Before committing code, ensure:

- [ ] Code compiles without warnings: `make release`
- [ ] All unit tests pass: `make test`
- [ ] Integration tests pass (with token): `INEGI_TOKEN='...' make test`
- [ ] Manual testing works: `./build/release/duckdb`
- [ ] Code is formatted (if you have clang-format): `make format`

## Environment Variables Reference

| Variable | Purpose | Required | Example |
|----------|---------|----------|---------|
| `INEGI_TOKEN` | INEGI API token for testing | For integration tests | `abc123xyz789` |
| `VCPKG_TOOLCHAIN_PATH` | Path to vcpkg toolchain | Yes | `~/vcpkg/scripts/buildsystems/vcpkg.cmake` |
| `GEN` | Build generator | No (default: make) | `ninja` |

## Directory Structure After Build

```
duck_inegi/
├── build/
│   └── release/
│       ├── duckdb                    # DuckDB CLI with extension
│       ├── extension/
│       │   └── inegi/
│       │       └── inegi.duckdb_extension  # Extension binary
│       └── test/
│           └── unittest              # Test runner
├── src/                              # Source code
├── test/sql/                         # SQL test files
└── ...
```

## Performance Tips

### Faster Builds

1. **Use Ninja**: 2-3x faster than Make
   ```bash
   export GEN=ninja
   ```

2. **Use ccache**: Caches compilation results
   ```bash
   sudo apt-get install ccache
   ```

3. **Parallel builds**: Use multiple CPU cores
   ```bash
   make release -j$(nproc)
   ```

### Faster Tests

1. **Run specific tests**: Don't run all tests every time
   ```bash
   ./build/release/test/unittest "test/sql/inegi_token.test"
   ```

2. **Skip integration tests**: When you don't need API calls
   ```bash
   unset INEGI_TOKEN
   make test
   ```

## Next Steps

After setup:

1. **Read the examples**: `docs/EXAMPLES.md`
2. **Understand the code**: Start with `src/inegi_extension.cpp`
3. **Try manual queries**: Use `./build/release/duckdb`
4. **Write new tests**: See `TESTING.md`
5. **Contribute**: Make improvements and submit PRs!

## Quick Reference Card

```bash
# Setup (one-time)
git clone --recurse-submodules https://github.com/YOUR_USERNAME/duck_inegi.git
cd duck_inegi
export VCPKG_TOOLCHAIN_PATH=~/vcpkg/scripts/buildsystems/vcpkg.cmake
export INEGI_TOKEN='your-token-here'

# Build
make release

# Test
make test

# Manual testing
./build/release/duckdb
```

## Getting Help

- **Build issues**: Check `docs/README.md`
- **Test issues**: Check `TESTING.md`
- **API questions**: Check `docs/EXAMPLES.md`
- **GitHub Actions**: Check `docs/GITHUB_ACTIONS.md`

---

**You're all set! Happy testing! 🚀**
