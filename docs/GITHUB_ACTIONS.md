# Building with GitHub Actions

This document explains how to use GitHub Actions to build the DuckDB-INEGI extension automatically, so you don't need to compile it on your local machine.

## Overview

The extension includes two GitHub Actions workflows:

1. **CI Workflow** (`CI.yml`) - Builds and tests on every push/PR
2. **Distribution Pipeline** (`MainDistributionPipeline.yml`) - Creates release binaries for all platforms

## Benefits of GitHub Actions

✅ **No local compilation needed** - GitHub's servers do the heavy lifting  
✅ **Multi-platform builds** - Automatically builds for Linux, macOS, and Windows  
✅ **Consistent environment** - Same build environment every time  
✅ **Artifact downloads** - Download pre-built binaries directly  
✅ **Automated testing** - Tests run automatically on every change  

## How to Use

### 1. Push Your Code to GitHub

```bash
# Initialize git repository (if not already done)
git init
git add .
git commit -m "Initial commit of INEGI extension"

# Add your GitHub repository as remote
git remote add origin https://github.com/YOUR_USERNAME/duck_inegi.git

# Push to GitHub
git push -u origin main
```

### 2. GitHub Actions Runs Automatically

Once you push, GitHub Actions will automatically:
- Build the extension for Linux, macOS, and Windows
- Run all tests
- Create downloadable artifacts

### 3. Monitor the Build

1. Go to your repository on GitHub
2. Click on the **"Actions"** tab
3. You'll see the running workflows
4. Click on a workflow run to see detailed logs

### 4. Download Built Extension

After the build completes:

1. Go to the **Actions** tab
2. Click on the completed workflow run
3. Scroll down to **Artifacts**
4. Download the artifact for your platform:
   - `linux-extension` - For Linux systems
   - `macos-extension` - For macOS systems
   - `windows-extension` - For Windows systems

### 5. Use the Downloaded Extension

```bash
# Extract the downloaded artifact
unzip linux-extension.zip  # or macos-extension.zip, windows-extension.zip

# Start DuckDB with unsigned extensions allowed
duckdb -unsigned

# Load the extension
LOAD './inegi.duckdb_extension';

# Set your token and start using it
SELECT INEGI_SetToken('your-token-here');
SELECT * FROM INEGI_Read('628194') LIMIT 10;
```

## Workflow Details

### CI Workflow

**Triggers:**
- Push to `main` or `master` branch
- Pull requests
- Manual trigger via workflow_dispatch

**What it does:**
- Builds extension on Linux, macOS, and Windows
- Runs all SQL tests
- Uploads build artifacts (valid for 90 days)

**Build time:** ~10-15 minutes per platform

### Distribution Pipeline

**Triggers:**
- Push to any branch
- Pull requests
- Manual trigger

**What it does:**
- Creates production-ready binaries
- Runs code quality checks (format, tidy)
- Prepares for distribution

**Build time:** ~15-20 minutes

## Manual Workflow Trigger

You can manually trigger a build without pushing code:

1. Go to **Actions** tab on GitHub
2. Select the workflow (CI or Main Distribution Pipeline)
3. Click **"Run workflow"**
4. Select the branch
5. Click **"Run workflow"** button

## Viewing Build Logs

To see detailed build logs:

1. Click on a workflow run
2. Click on a job (e.g., "Linux Build & Test")
3. Expand the steps to see detailed output
4. Look for errors in red text

## Common Issues

### Build Fails on Dependency Installation

**Problem:** vcpkg fails to install dependencies

**Solution:** 
- Check the vcpkg commit hash is correct
- Verify `vcpkg.json` has correct dependency names
- Check the workflow logs for specific errors

### Tests Fail

**Problem:** SQL tests fail during CI

**Solution:**
- Run tests locally first: `make test`
- Check test expectations match actual output
- Some tests may require valid INEGI tokens (comment out for CI)

### Artifact Not Available

**Problem:** Can't find build artifacts

**Solution:**
- Ensure the workflow completed successfully (green checkmark)
- Artifacts expire after 90 days
- Re-run the workflow if needed

## Customizing Workflows

### Change DuckDB Version

Edit `.github/workflows/MainDistributionPipeline.yml`:

```yaml
with:
  duckdb_version: v1.4.4  # Change to desired version
  ci_tools_version: v1.4.4
  extension_name: inegi
```

### Add More Platforms

The distribution pipeline automatically builds for:
- Linux (x64, ARM64)
- macOS (x64, ARM64)
- Windows (x64)
- WebAssembly

No additional configuration needed!

### Skip Tests on CI

Edit `.github/workflows/CI.yml` and comment out:

```yaml
# - name: Run tests
#   run: |
#     make test
```

## Cost and Limits

GitHub Actions is **free** for public repositories with generous limits:

- **2,000 minutes/month** for private repos (free tier)
- **Unlimited minutes** for public repos
- **500 MB** artifact storage (free tier)

Each build takes ~10-15 minutes, so you can run many builds per month.

## Best Practices

### 1. Test Locally First

Before pushing, test locally:
```bash
make clean
make release
make test
```

### 2. Use Draft PRs

Create draft pull requests to test changes without triggering full CI:
```bash
gh pr create --draft
```

### 3. Cache Dependencies

The workflows already use vcpkg caching to speed up builds.

### 4. Monitor Build Times

Check the Actions tab to see which steps take longest and optimize if needed.

## Troubleshooting

### Build is Slow

**Cause:** First build compiles DuckDB from scratch

**Solution:** Subsequent builds are faster due to caching

### Out of Disk Space

**Cause:** Build artifacts too large

**Solution:** Clean up old artifacts in Settings → Actions → Artifacts

### Permission Denied

**Cause:** GitHub Actions doesn't have write permissions

**Solution:** 
1. Go to Settings → Actions → General
2. Set "Workflow permissions" to "Read and write permissions"

## Advanced: Release Automation

To automatically create releases when you tag:

1. Create a new workflow `.github/workflows/release.yml`
2. Trigger on tag push:
```yaml
on:
  push:
    tags:
      - 'v*'
```
3. Use the distribution pipeline to create release binaries
4. Upload to GitHub Releases

## Getting Help

- **GitHub Actions Docs**: https://docs.github.com/en/actions
- **DuckDB Extension CI Tools**: https://github.com/duckdb/extension-ci-tools
- **This Project's Issues**: https://github.com/YOUR_USERNAME/duck_inegi/issues

## Summary

With GitHub Actions, you can:
1. ✅ Push code to GitHub
2. ✅ Wait 10-15 minutes for build
3. ✅ Download pre-built extension
4. ✅ Use immediately without local compilation

No need to install build tools, manage dependencies, or wait for long local builds!
