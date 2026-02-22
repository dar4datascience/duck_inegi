# Quick Setup: GitHub Secrets for INEGI Extension

## Required Secret

To run integration tests with real INEGI API data, you need to add one secret:

**Secret Name:** `INEGI_TOKEN`  
**Secret Value:** Your INEGI API token

## How to Add the Secret

### Step 1: Get Your Token
Visit: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify

### Step 2: Add to GitHub
1. Go to your repository on GitHub
2. Click **Settings** → **Secrets and variables** → **Actions**
3. Click **New repository secret**
4. Name: `INEGI_TOKEN`
5. Value: Paste your token
6. Click **Add secret**

### Step 3: Done!
Push code to trigger workflows. Integration tests will now fetch real data from INEGI.

## What Happens Without the Secret?

- ✅ Extension still builds successfully
- ✅ Unit tests still pass
- ⚠️ Integration tests are skipped (no real API calls)

This is by design - the extension works without the secret, but you won't test real API functionality.

## More Information

See `docs/GITHUB_SECRETS.md` for detailed documentation.
