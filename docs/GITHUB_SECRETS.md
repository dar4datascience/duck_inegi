# Setting Up GitHub Secrets for INEGI Token

This guide explains how to add your INEGI API token as a GitHub secret so that automated tests can fetch real data from the INEGI API.

## Why Use GitHub Secrets?

GitHub Secrets allow you to:
- ✅ Store sensitive information (like API tokens) securely
- ✅ Use tokens in GitHub Actions without exposing them in code
- ✅ Run integration tests that require authentication
- ✅ Keep tokens encrypted and hidden from repository viewers

## Step-by-Step Guide

### 1. Get Your INEGI API Token

If you don't have a token yet:

1. Visit: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
2. Register for a free account
3. Generate your API token
4. Copy the token (you'll need it in the next step)

### 2. Add Secret to GitHub Repository

#### Option A: Via GitHub Web Interface

1. Go to your repository on GitHub
2. Click **Settings** (top menu)
3. In the left sidebar, click **Secrets and variables** → **Actions**
4. Click **New repository secret**
5. Fill in the form:
   - **Name**: `INEGI_TOKEN`
   - **Value**: Paste your INEGI API token
6. Click **Add secret**

#### Option B: Via GitHub CLI

```bash
# Install GitHub CLI if you haven't already
# https://cli.github.com/

# Set the secret
gh secret set INEGI_TOKEN

# When prompted, paste your token and press Enter
```

### 3. Verify Secret is Set

1. Go to **Settings** → **Secrets and variables** → **Actions**
2. You should see `INEGI_TOKEN` in the list
3. The value will be hidden (shown as `***`)

### 4. Test the Integration

Push a commit to trigger GitHub Actions:

```bash
git add .
git commit -m "Test INEGI integration with token"
git push origin main
```

Then:
1. Go to **Actions** tab
2. Click on the running workflow
3. Check the test results
4. Integration tests should now pass with real API data!

## How It Works

### In GitHub Actions Workflows

The workflows are configured to use the secret:

```yaml
env:
  INEGI_TOKEN: ${{ secrets.INEGI_TOKEN }}
```

This makes the token available as an environment variable during the build and test process.

### In Tests

The integration tests check for the token:

```sql
-- Use token from environment if available
SELECT INEGI_SetToken(COALESCE(getenv('INEGI_TOKEN'), 'test-token-12345'));
```

**Integration tests** (`test/sql/inegi_integration.test`):
- Only run real API calls if `INEGI_TOKEN` is set
- Skip API tests if token is not available
- Validate actual data from INEGI

**Unit tests** (`test/sql/inegi.test`, `test/sql/inegi_token.test`):
- Run without real API calls
- Use dummy tokens for testing token management
- Always pass regardless of token availability

## Security Best Practices

### ✅ DO:
- Store tokens as GitHub Secrets
- Use different tokens for development and production
- Rotate tokens periodically
- Limit token permissions if possible

### ❌ DON'T:
- Commit tokens to the repository
- Share tokens in pull request comments
- Log tokens in test output
- Use production tokens for testing

## Troubleshooting

### Secret Not Working

**Problem**: Tests still fail even after adding secret

**Solutions**:
1. Verify secret name is exactly `INEGI_TOKEN` (case-sensitive)
2. Check that the token value is correct (no extra spaces)
3. Re-run the workflow (sometimes takes a moment to propagate)
4. Check workflow logs for token-related errors

### Token Expired

**Problem**: Tests fail with authentication errors

**Solution**:
1. Generate a new token from INEGI
2. Update the secret in GitHub Settings
3. Re-run the workflow

### Tests Skipped

**Problem**: Integration tests are being skipped

**Cause**: This is expected if `INEGI_TOKEN` is not set

**Solution**: 
- Add the secret as described above
- Or run tests locally with: `INEGI_TOKEN=your-token make test`

## Local Testing with Token

You can also test locally with the token:

```bash
# Set token as environment variable
export INEGI_TOKEN='your-token-here'

# Run tests
make test

# Or run DuckDB with token
./build/release/duckdb
```

```sql
-- Token will be automatically loaded from environment
SELECT INEGI_SetToken(getenv('INEGI_TOKEN'));
SELECT * FROM INEGI_Read('628194') LIMIT 10;
```

## Multiple Environments

If you need different tokens for different branches:

### Environment Secrets (Advanced)

1. Go to **Settings** → **Environments**
2. Create environments (e.g., `development`, `production`)
3. Add secrets to each environment
4. Update workflow to use environment:

```yaml
jobs:
  test:
    runs-on: ubuntu-latest
    environment: development  # or production
    steps:
      # ... your steps
```

## Viewing Secret Usage

You can see when secrets are used:

1. Go to **Settings** → **Secrets and variables** → **Actions**
2. Click on the secret name
3. View "Used by" section to see which workflows use it

## Removing a Secret

If you need to remove or rotate a token:

1. Go to **Settings** → **Secrets and variables** → **Actions**
2. Click on `INEGI_TOKEN`
3. Click **Remove secret**
4. Confirm removal
5. Add new secret if needed

## FAQ

**Q: Can collaborators see my token?**  
A: No, secrets are encrypted and only accessible to GitHub Actions. Even repository admins cannot view secret values.

**Q: Will the token appear in logs?**  
A: No, GitHub automatically masks secret values in workflow logs.

**Q: Can I use the same token for multiple repositories?**  
A: Yes, but you'll need to add it as a secret to each repository separately.

**Q: What if I accidentally commit a token?**  
A: 
1. Immediately revoke the token in INEGI
2. Generate a new token
3. Update the GitHub secret
4. Remove the token from git history (use `git filter-branch` or BFG Repo-Cleaner)

**Q: Do secrets work in pull requests from forks?**  
A: No, for security reasons. Secrets are not available to workflows triggered by pull requests from forks.

## Summary

1. ✅ Get your INEGI token from https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
2. ✅ Add it as `INEGI_TOKEN` secret in GitHub Settings
3. ✅ Push code to trigger workflows
4. ✅ Integration tests will now fetch real data from INEGI API

Your token is secure, encrypted, and ready to use! 🔒✨
