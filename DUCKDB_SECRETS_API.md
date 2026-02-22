# DuckDB Secrets API - Explanation and Evaluation

## What is the DuckDB Secrets API?

The **Secrets API** is DuckDB's built-in system for managing credentials and authentication tokens across extensions. It was introduced to provide a standardized, secure way to handle sensitive information like API keys, database passwords, and cloud storage credentials.

### Key Features

1. **Typed Secrets** - Each secret has a type (e.g., `s3`, `azure`, `postgres`) that identifies which service it's for
2. **Secret Providers** - Different methods to create secrets (manual `CONFIG` or automatic `credential_chain`)
3. **Scoped Secrets** - Secrets can apply to specific file paths/prefixes
4. **Persistent Storage** - Secrets can be saved between DuckDB sessions
5. **Environment Variable Support** - Can read from environment variables (in some providers)

### How It Works

```sql
-- Create a temporary secret (in-memory only)
CREATE SECRET my_secret (
    TYPE s3,
    KEY_ID 'my_access_key',
    SECRET 'my_secret_key',
    REGION 'us-east-1'
);

-- Create a persistent secret (saved to disk)
CREATE PERSISTENT SECRET my_secret (
    TYPE s3,
    KEY_ID 'my_access_key',
    SECRET 'my_secret_key'
);

-- Drop a secret
DROP SECRET my_secret;

-- List all secrets
SELECT * FROM duckdb_secrets();
```

### Built-in Secret Types (DuckDB v1.4.4)

- `s3` - AWS S3 (httpfs extension)
- `gcs` - Google Cloud Storage (httpfs extension)
- `r2` - Cloudflare R2 (httpfs extension)
- `azure` - Azure Blob Storage (azure extension)
- `postgres` - PostgreSQL (postgres extension)
- `mysql` - MySQL (mysql extension)
- `http` - HTTP authentication (httpfs extension)
- `huggingface` - Hugging Face (httpfs extension)

## Could We Use It for INEGI Extension?

### Yes, We Can! Here's How:

Extensions can **register custom secret types** by implementing:

1. **Secret Type Registration** - Define a new secret type (e.g., `inegi`)
2. **Secret Provider** - Implement how secrets are created/validated
3. **Secret Access** - Use the secret in your extension functions

### Example Implementation

```cpp
// Register INEGI secret type
void RegisterINEGISecretType(DatabaseInstance &db) {
    SecretType secret_type;
    secret_type.name = "inegi";
    secret_type.deserializer = INEGISecretDeserialize;
    secret_type.default_provider = "config";
    
    ExtensionUtil::RegisterSecretType(db, secret_type);
}

// Users would then use it like:
```

```sql
-- Create INEGI secret
CREATE SECRET my_inegi (
    TYPE inegi,
    TOKEN 'your-inegi-api-token-here'
);

-- Or from environment variable (if provider supports it)
CREATE SECRET my_inegi (
    TYPE inegi,
    PROVIDER credential_chain  -- Would read from INEGI_TOKEN env var
);

-- Then use it automatically
SELECT * FROM INEGI_Read('628194');  -- Automatically uses the secret
```

## Benefits of Using Secrets API

### ✅ Advantages

1. **Standard DuckDB Pattern**
   - Users familiar with S3/Azure secrets will understand immediately
   - Consistent with other DuckDB extensions

2. **Environment Variable Support**
   - Could implement a provider that reads `INEGI_TOKEN` from environment
   - Would solve the testing problem!

3. **Persistent Secrets**
   - Users don't need to set token every session
   - Stored in `~/.duckdb/stored_secrets/`

4. **Better Security**
   - Secrets are managed by DuckDB's secret manager
   - Can be scoped to specific databases/connections

5. **Automatic Secret Selection**
   - Functions can automatically use the appropriate secret
   - No need to manually call `INEGI_SetToken()`

6. **Multiple Secrets**
   - Users could have different tokens for different projects
   - Scoped secrets for different data sources

### ❌ Disadvantages

1. **More Complex Implementation**
   - Need to implement secret type registration
   - Need to implement secret providers
   - More C++ code to write and maintain

2. **Breaking Change**
   - Would need to change from `INEGI_SetToken()` to `CREATE SECRET`
   - Current approach is simpler for users

3. **Learning Curve**
   - Users need to understand DuckDB secrets
   - More documentation needed

4. **Testing Still Complex**
   - Would still need to implement credential_chain provider
   - More code to test

## Recommendation

### For Now: **Keep Current Approach** ✅

**Reasons:**
1. **Current approach works** - Token management via `INEGI_SetToken()` is functional
2. **Simpler to understand** - Users just call a function, no secrets to manage
3. **Already implemented** - Working code that's been tested
4. **Time to market** - Get the extension working first, optimize later

### Future: **Implement Secrets API** (v2.0)

**When to implement:**
- After initial release and user feedback
- When you have time for a more complex implementation
- If users request it for better integration

**Implementation Plan for Future:**

```cpp
// Phase 1: Add secret type registration
void InegiExtension::Load(ExtensionLoader &loader) {
    // Register secret type
    SecretType inegi_secret;
    inegi_secret.name = "inegi";
    inegi_secret.deserializer = INEGISecretDeserialize;
    inegi_secret.default_provider = "config";
    loader.RegisterSecretType(inegi_secret);
    
    // Keep existing functions for backward compatibility
    LoadInternal(loader);
}

// Phase 2: Implement credential_chain provider
// This would read from INEGI_TOKEN environment variable

// Phase 3: Update INEGI_Read to use secrets
// Check for secrets first, fall back to INEGI_SetToken
```

## Current vs Future Comparison

### Current Approach (v1.0)

```sql
-- User workflow
SELECT INEGI_SetToken('my-token');
SELECT * FROM INEGI_Read('628194');
```

**Pros:** Simple, works now  
**Cons:** No persistence, no env var support in tests

### Future with Secrets API (v2.0)

```sql
-- User workflow
CREATE PERSISTENT SECRET inegi_token (
    TYPE inegi,
    TOKEN 'my-token'
);

-- Or from environment
CREATE SECRET inegi_token (
    TYPE inegi,
    PROVIDER credential_chain  -- Reads INEGI_TOKEN env var
);

-- Then just use it
SELECT * FROM INEGI_Read('628194');  -- Auto-uses secret
```

**Pros:** Standard pattern, persistent, env var support  
**Cons:** More complex, more code

## Testing Benefits with Secrets API

If we implement the Secrets API with a `credential_chain` provider:

```sql
-- In tests, this would work:
CREATE SECRET inegi_test (
    TYPE inegi,
    PROVIDER credential_chain  -- Reads from INEGI_TOKEN env var
);

-- Then tests could actually make real API calls
SELECT * FROM INEGI_Read('628194');
```

This would solve the testing problem because:
1. ✅ Environment variables would be accessible through the provider
2. ✅ Tests could use real tokens in CI/CD
3. ✅ Local tests could use developer's own tokens
4. ✅ Standard DuckDB pattern for handling credentials

## Conclusion

**Answer to your question:** Yes, we can use the DuckDB Secrets API, and it would be a better long-term solution!

**Current recommendation:** 
- **Ship v1.0 with current approach** (simpler, works now)
- **Plan v2.0 with Secrets API** (better, more DuckDB-native)

**Why wait:**
- Current approach is functional
- Secrets API requires significant additional work
- Better to get user feedback first
- Can add Secrets API as enhancement later

**If you want to implement it now:**
- I can help you implement the Secrets API
- Would take additional development time
- Would be more "DuckDB-native" from the start
- Would solve the testing problem

**Your choice:** Ship simple now, or implement full Secrets API?
