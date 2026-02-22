# DuckDB-INEGI Extension - Usage Examples

This document provides practical examples of using the DuckDB-INEGI extension to query data from INEGI's APIs.

## Table of Contents

- [Getting Started](#getting-started)
- [Banco de Indicadores (Indicator Bank) Examples](#banco-de-indicadores-indicator-bank-examples)
- [DENUE (Business Directory) Examples](#denue-business-directory-examples)
- [Advanced Queries](#advanced-queries)
- [Data Analysis Examples](#data-analysis-examples)

## Getting Started

### 1. Load the Extension

```sql
-- Load the INEGI extension
LOAD 'build/release/extension/inegi/inegi.duckdb_extension';
```

### 2. Set Your API Token

Before making any API calls, you must set your INEGI API token. Get your token from:
https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify

```sql
-- Set your token (replace with your actual token)
SELECT INEGI_SetToken('your-token-here');
```

### 3. Verify Token is Set

```sql
-- Check if token is set (for debugging)
SELECT INEGI_GetToken();
```

---

## Banco de Indicadores (Indicator Bank) Examples

The Indicator Bank API provides access to statistical indicators at national, state, and municipal levels.

### API Endpoint Structure

```
https://www.inegi.org.mx/app/api/indicadores/desarrolladores/jsonStat/{BANK}/{INDICATOR_ID}/{LANGUAGE}/{GEOGRAPHY}/{RECENT_ONLY}?type=json&token={TOKEN}
```

**Parameters:**
- `BANK`: BIE or BISE
- `INDICATOR_ID`: Unique indicator identifier
- `LANGUAGE`: es (Spanish) or en (English)
- `GEOGRAPHY`: 00 (national), 99 (state), 999 (municipality)
- `RECENT_ONLY`: true (latest value) or false (full time series)

### Example 1: GDP (PIB) - National Level

```sql
-- Fetch GDP data (Indicator: 628194)
-- National level, Spanish, full time series
SELECT * 
FROM INEGI_Read('628194')
ORDER BY time_period DESC
LIMIT 10;
```

**Output:**
```
┌─────────────┬──────────────────┬──────────────┬────────────────────────┐
│ time_period │ observation_value│ indicator_id │ indicator_name         │
│   varchar   │      double      │   varchar    │        varchar         │
├─────────────┼──────────────────┼──────────────┼────────────────────────┤
│ 2024/01     │      23456789.50 │ 628194       │ Producto Interno Bruto │
│ 2023/12     │      23234567.80 │ 628194       │ Producto Interno Bruto │
│ ...         │              ... │ ...          │ ...                    │
└─────────────┴──────────────────┴──────────────┴────────────────────────┘
```

### Example 2: Inflation Rate - English Language

```sql
-- Fetch inflation data in English
SELECT 
    time_period,
    observation_value as inflation_rate,
    indicator_name
FROM INEGI_Read('628273', language='en')
WHERE time_period >= '2020-01-01'
ORDER BY time_period DESC;
```

### Example 3: Unemployment Rate - State Level

```sql
-- Fetch unemployment rate by state
SELECT * 
FROM INEGI_Read('444603', 
    language='es',
    geography='99',  -- State level
    recent_only=false
)
LIMIT 20;
```

### Example 4: Recent Data Only

```sql
-- Get only the most recent value
SELECT 
    time_period,
    observation_value,
    indicator_name
FROM INEGI_Read('628194', recent_only=true);
```

### Example 5: Using BISE Bank

```sql
-- Query from BISE bank instead of BIE
SELECT * 
FROM INEGI_Read('some_indicator_id', 
    bank='BISE',
    language='es',
    geography='00'
)
LIMIT 10;
```

### Common Economic Indicators

Here are some frequently used indicator IDs:

```sql
-- GDP (Producto Interno Bruto)
SELECT * FROM INEGI_Read('628194') LIMIT 5;

-- Inflation (Inflación)
SELECT * FROM INEGI_Read('628273') LIMIT 5;

-- Unemployment Rate (Tasa de Desempleo)
SELECT * FROM INEGI_Read('444603') LIMIT 5;

-- Exchange Rate (Tipo de Cambio)
SELECT * FROM INEGI_Read('628168') LIMIT 5;

-- Industrial Production (Producción Industrial)
SELECT * FROM INEGI_Read('383152') LIMIT 5;
```

---

## DENUE (Business Directory) Examples

**Note:** DENUE support is planned for future releases. Below are examples of how it will work once implemented.

### API Endpoint Structure

The DENUE API provides information about business establishments in Mexico.

```
https://www.inegi.org.mx/app/api/denue/v1/{METHOD}?token={TOKEN}&{PARAMETERS}
```

### Available Methods (Planned)

#### 1. Buscar (Search)
Search for establishments by various criteria.

```sql
-- Example (Future implementation)
-- Search for restaurants in Mexico City
SELECT * 
FROM INEGI_DENUE_Buscar(
    nombre='restaurant',
    entidad='09',  -- Mexico City
    actividad='722'  -- Food services
)
LIMIT 10;
```

#### 2. Ficha (Details)
Get detailed information about a specific establishment.

```sql
-- Example (Future implementation)
-- Get details for a specific establishment
SELECT * 
FROM INEGI_DENUE_Ficha(id='12345678');
```

#### 3. BuscarAreaAct (Search by Area and Activity)
Search establishments by geographic area and economic activity.

```sql
-- Example (Future implementation)
-- Search for manufacturing companies in a specific area
SELECT * 
FROM INEGI_DENUE_BuscarAreaAct(
    latitud='19.4326',
    longitud='-99.1332',
    distancia='5000',  -- 5km radius
    actividad='31'     -- Manufacturing
)
LIMIT 20;
```

#### 4. Cuantificar (Count)
Count establishments matching criteria.

```sql
-- Example (Future implementation)
-- Count retail stores in Jalisco
SELECT * 
FROM INEGI_DENUE_Cuantificar(
    entidad='14',  -- Jalisco
    actividad='46'  -- Retail trade
);
```

---

## Advanced Queries

### Time Series Analysis

```sql
-- Calculate year-over-year GDP growth
WITH gdp_data AS (
    SELECT 
        time_period,
        observation_value as gdp
    FROM INEGI_Read('628194')
    WHERE time_period IS NOT NULL
)
SELECT 
    time_period,
    gdp,
    LAG(gdp, 12) OVER (ORDER BY time_period) as gdp_year_ago,
    ((gdp - LAG(gdp, 12) OVER (ORDER BY time_period)) / 
     LAG(gdp, 12) OVER (ORDER BY time_period) * 100) as yoy_growth_pct
FROM gdp_data
ORDER BY time_period DESC
LIMIT 24;
```

### Multi-Indicator Dashboard

```sql
-- Create a comprehensive economic dashboard
CREATE VIEW economic_dashboard AS
SELECT 
    'GDP' as indicator,
    time_period,
    observation_value,
    'Billions of pesos' as unit
FROM INEGI_Read('628194')
WHERE time_period >= '2020-01-01'

UNION ALL

SELECT 
    'Inflation' as indicator,
    time_period,
    observation_value,
    'Percentage' as unit
FROM INEGI_Read('628273')
WHERE time_period >= '2020-01-01'

UNION ALL

SELECT 
    'Unemployment' as indicator,
    time_period,
    observation_value,
    'Percentage' as unit
FROM INEGI_Read('444603')
WHERE time_period >= '2020-01-01';

-- Query the dashboard
SELECT * FROM economic_dashboard
ORDER BY indicator, time_period DESC;
```

### Export to Different Formats

```sql
-- Export to CSV
COPY (
    SELECT * FROM INEGI_Read('628194')
) TO 'gdp_data.csv' (HEADER, DELIMITER ',');

-- Export to Parquet
COPY (
    SELECT * FROM INEGI_Read('628194')
) TO 'gdp_data.parquet' (FORMAT PARQUET);

-- Export to JSON
COPY (
    SELECT * FROM INEGI_Read('628194')
) TO 'gdp_data.json' (FORMAT JSON);
```

### Join with Local Data

```sql
-- Create a local table with state information
CREATE TABLE states AS 
SELECT * FROM VALUES
    ('01', 'Aguascalientes'),
    ('02', 'Baja California'),
    ('09', 'Ciudad de México'),
    ('14', 'Jalisco')
AS t(state_code, state_name);

-- Join INEGI data with local data (when using state-level indicators)
SELECT 
    s.state_name,
    i.time_period,
    i.observation_value
FROM INEGI_Read('some_state_indicator', geography='99') i
JOIN states s ON SUBSTRING(i.indicator_id, -2) = s.state_code
ORDER BY s.state_name, i.time_period DESC;
```

---

## Data Analysis Examples

### Statistical Summary

```sql
-- Get statistical summary of GDP over time
SELECT 
    COUNT(*) as observations,
    MIN(observation_value) as min_gdp,
    MAX(observation_value) as max_gdp,
    AVG(observation_value) as avg_gdp,
    STDDEV(observation_value) as stddev_gdp,
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY observation_value) as median_gdp
FROM INEGI_Read('628194');
```

### Moving Averages

```sql
-- Calculate 3-month and 12-month moving averages
SELECT 
    time_period,
    observation_value as actual_value,
    AVG(observation_value) OVER (
        ORDER BY time_period 
        ROWS BETWEEN 2 PRECEDING AND CURRENT ROW
    ) as ma_3month,
    AVG(observation_value) OVER (
        ORDER BY time_period 
        ROWS BETWEEN 11 PRECEDING AND CURRENT ROW
    ) as ma_12month
FROM INEGI_Read('628194')
ORDER BY time_period DESC
LIMIT 50;
```

### Correlation Analysis

```sql
-- Analyze correlation between GDP and unemployment
WITH gdp AS (
    SELECT 
        time_period,
        observation_value as gdp_value
    FROM INEGI_Read('628194')
),
unemployment AS (
    SELECT 
        time_period,
        observation_value as unemployment_value
    FROM INEGI_Read('444603')
)
SELECT 
    CORR(g.gdp_value, u.unemployment_value) as correlation_coefficient
FROM gdp g
JOIN unemployment u ON g.time_period = u.time_period;
```

### Seasonal Decomposition

```sql
-- Extract seasonal patterns from monthly data
WITH monthly_data AS (
    SELECT 
        time_period,
        observation_value,
        EXTRACT(MONTH FROM CAST(time_period AS DATE)) as month,
        EXTRACT(YEAR FROM CAST(time_period AS DATE)) as year
    FROM INEGI_Read('628194')
    WHERE time_period IS NOT NULL
)
SELECT 
    month,
    AVG(observation_value) as avg_value,
    STDDEV(observation_value) as stddev_value,
    COUNT(*) as observations
FROM monthly_data
GROUP BY month
ORDER BY month;
```

### Pivot Table

```sql
-- Create a pivot table showing indicators by year
PIVOT (
    SELECT 
        EXTRACT(YEAR FROM CAST(time_period AS DATE)) as year,
        'GDP' as indicator,
        AVG(observation_value) as avg_value
    FROM INEGI_Read('628194')
    WHERE time_period IS NOT NULL
    GROUP BY year
    
    UNION ALL
    
    SELECT 
        EXTRACT(YEAR FROM CAST(time_period AS DATE)) as year,
        'Inflation' as indicator,
        AVG(observation_value) as avg_value
    FROM INEGI_Read('628273')
    WHERE time_period IS NOT NULL
    GROUP BY year
)
ON indicator
USING SUM(avg_value)
ORDER BY year DESC;
```

---

## Performance Tips

### 1. Use `recent_only=true` for Latest Data

```sql
-- Faster - only fetches recent data
SELECT * FROM INEGI_Read('628194', recent_only=true);

-- Slower - fetches entire time series
SELECT * FROM INEGI_Read('628194', recent_only=false);
```

### 2. Filter After Fetching

```sql
-- Fetch data and filter in DuckDB
SELECT * 
FROM INEGI_Read('628194')
WHERE time_period >= '2020-01-01';
```

### 3. Create Views for Frequently Used Queries

```sql
-- Create a view for recent economic indicators
CREATE VIEW recent_economic_data AS
SELECT * FROM INEGI_Read('628194', recent_only=true)
UNION ALL
SELECT * FROM INEGI_Read('628273', recent_only=true)
UNION ALL
SELECT * FROM INEGI_Read('444603', recent_only=true);

-- Query the view
SELECT * FROM recent_economic_data;
```

---

## Troubleshooting

### Token Not Set Error

```sql
-- Error: INEGI API token not set
-- Solution: Set your token first
SELECT INEGI_SetToken('your-token-here');
```

### Invalid Indicator ID

```sql
-- Error: HTTP request failed with status code: 404
-- Solution: Verify the indicator ID exists
-- Check the indicator bank: https://www.inegi.org.mx/app/indicadores/
```

### Network Timeout

```sql
-- If requests timeout, try:
-- 1. Check your internet connection
-- 2. Verify INEGI API is accessible
-- 3. Try with recent_only=true for faster response
SELECT * FROM INEGI_Read('628194', recent_only=true);
```

---

## Additional Resources

- **INEGI Indicator Bank**: https://www.inegi.org.mx/app/indicadores/
- **API Documentation**: https://www.inegi.org.mx/servicios/api_indicadores.html
- **DENUE API Documentation**: https://www.inegi.org.mx/servicios/api_denue.html
- **Get API Token**: https://www.inegi.org.mx/app/desarrolladores/generatoken/Usuarios/token_Verify
- **DuckDB Documentation**: https://duckdb.org/docs/

---

## Contributing Examples

Have a useful query or analysis pattern? Contribute to this documentation by submitting a pull request!
