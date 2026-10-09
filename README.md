## NumbertoWord

### ⚠️ Current Architectural Constraints & Assumptions

1. ** Pure Integer Parsing Limits:** The framework assumes zero fractional components. Decimal scaling and fractional sub-units are not integrated within the base data pipeline.

2. **Modern Decimal Currency Bias:** The operational architecture scales strictly on base-10 shifts. Historic duodecimal/quaternary currencies (e.g., British pre-decimal £sd or pre-1957 Subcontinental Rupee/Anna/Pie systems) remain incompatible without an integrated fraction mapper.

3. ** Left-to-Right Scale Linearity:** The buffer rendering system presumes linguistic structures read from highest scale descending straight down to individual baseline digits.