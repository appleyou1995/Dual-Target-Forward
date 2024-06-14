# 【財務演算法】期末團體作業 Dual Target Forward

## 作業說明

1. **組員**
   - D11723002 張詠瑄
   - D11723003 尤柔蘋
   - B09703009 許靖

2. **作業檔案**
   1. 利用 Julia 評價 Dual Target Forward
   2. 利用 C++ 評價 Dual Target Forward
   3. 利用 C++ 與 Qt 建立評價 Dual Target Forward 合約的 GUI 檔案

3. **相關說明**
   - 商品合約內容（[Dual Target Forward Contract Details](https://github.com/appleyou1995/Dual-Target-Forward/edit/main/README.md#dual-target-forward-contract-details)）
   - GUI 操作說明（[README.md](https://github.com/appleyou1995/Dual-Target-Forward/blob/main/Qt/README.md)）

4. **商品性質概要**  <br>
Dual Target Forward 產品屬於比較具有風險性質的商品，因為它讓投資人賺錢的時候只能賺一倍，賠錢的時候卻會賠兩倍。當市場風向對發行商不利時會讓投資人趕快將獲利了結、提早出場；而對發行商有利時會讓投資人無法出場拿比較少的錢、甚至賠錢。

<br>

## Dual Target Forward Contract Details

The Dual Target Forward is a product which offers the client the opportunity of obtaining a conversion rate which is more favourable than the average forward rate for comparable series of standard forward exchange transactions. 

At each expiration date, depending on the fixing rate of the underlying currency pair, the client has to convert the original notional amount, the leveraged amount or none at all (if the product was traded with a Knock-In barrier on the leverage). 

If a pre-defined advantage (Target) or a pre-defined number of favourable fixings (Target Count) is reached or breached, the product will be early terminated.


### Dual Target Forward Details

| **Parameter**             | **Detail**                                                                                       |
|---------------------------|--------------------------------------------------------------------------------------------------|
| **Spot Reference**        | 1.3750 USD per EUR                                                                               |
| **Party A**               | Commerzbank AG Frankfurt                                                                         |
| **Party B**               | t.b.d.                                                                                           |
| **Currency Pair**         | EUR/USD                                                                                          |
| **Currency and Notional** | EUR 1,000,000 in respect of each FX Forward Transaction (Settlement Amounts may differ from the Notional Amount, for details please refer to section "Settlement Amounts") |
| **Trade Date**            | t.b.d.                                                                                           |
| **Expiration Dates**      | Monthly, 1 month to 12 months (subject to Target Event)                                          |
| **Settlement Dates**      | Expiration Dates + 2 business days (subject to Target Event)                                     |
| **Upfront Premium**       | Commerzbank pays USD 50,000 value Trade Date + 2 business days                                   |
| **Strike**                | 1.33500 USD per EUR                                                                              |
| **Target**                | 0.10000 USD per EUR                                                                              |
| **Target Count**          | 4                                                                                                |
| **Leverage**              | 200.00%                                                                                          |


### Knock-Out Event 1

The Knock-Out Event 1 is deemed to have occurred when the "**_Cumulative Intrinsic Value_**" is greater than or equal to the "**_Target_**".


### Knock-Out Event 2

The Knock-Out Event 2 is deemed to have occurred when the "**_Cumulative Digital Value_**" is equal to the "**_Target Count_**".


### Cumulative Intrinsic Value

The Cumulative Intrinsic Value, $CIVP(n)$, on any $n$-th Expiration Date is defined as the sum of positive Intrinsic Values, $P(i)$, up to and including that Expiration Date. Thus for the $n$-th Expiration Date:

$$
CIVP(n) = \sum_{i=1}^n P(i)
$$


Where

- If Fixing is at or below the Strike, then the relevant Fixing has positive Intrinsic Value equal to:

$$
P(i) = \max(0, \text{Strike} - \text{Fixing Rate})
$$

- If Fixing is strictly above the Strike, then the Intrinsic Value shall be zero.

### Cumulative Digital Value

The Cumulative Digital Value, $CVP(n)$, on any $n$-th Expiration Date is defined as the sum of positive Digital Values. Thus for the $n$-th Expiration Date:

$$
CVP(n) = \sum_{i=1}^{n} P(i)
$$

Where

- If Fixing is at or below the Strike, then the relevant Fixing has a positive Digital Value equal to

$$ P(i)=1 $$

- If Fixing is strictly above the Strike, then the Digital Value shall be zero.

### Settlement Amounts

In respect of each FX Forward Transaction on its corresponding Settlement Date:

1. **If neither Knock-Out Event 1 nor Knock-Out Event 2 have occurred on any Expiration Date**:
   - **If the Fixing Rate is at or above the Strike**:
     - Party A shall pay an amount in EUR equal to the Notional Amount to Party B on the Settlement Date.
     - Party B shall pay an amount in USD equal to Notional Amount × Strike to Party A on the Settlement Date.
   - **If the Fixing Rate is strictly below the Strike**:
     - Party A shall pay an amount in EUR equal to Notional Amount × Leverage to Party B on the Settlement Date.
     - Party B shall pay an amount in USD equal to Notional Amount × Leverage × Strike to Party A on the Settlement Date.

2. **If either Knock-Out Event 1 or Knock-Out Event 2 have occurred, on the Settlement Date immediately following the occurrence of the Knock-Out Events**:
   - **If the Fixing Rate is at or above the Strike**:
     - Party A shall pay an amount in EUR equal to the Notional Amount to Party B on the Settlement Date.
     - Party B shall pay an amount in USD equal to Notional Amount × Strike to Party A on the Settlement Date.
     
     Neither Party will have any further obligations to the other Party in respect of all remaining Settlement Dates falling after the occurrence of the Knock-Out Event.

### Spot Rate

The Spot Rate is expressed as the amount of USD per one EUR as determined by the Calculation Agent in its sole discretion, acting in a commercially reasonable manner.

### Fixing Rate

t.b.d.

### Calculation Agent

Commerzbank AG

---

### Additional Risk Disclosures

Before investing in this product, investors should carefully consider its appropriateness, suitability, and the following additional risks:
1. **Issuer Risk**: Any failure by the issuer to perform its contractual obligations, when due, may result in loss of all or part of the invested capital.
2. **Counterparty Risk**: Any failure by Commerzbank AG to perform contractual obligations, when due, may result in the loss of all or part of the invested capital.
3. **Market Risk**: Various market factors may affect the value of the investment or the underlying assets, including but not limited to the impact of volatility, interest rates, dividends (if any), foreign exchange.
4. **Liquidity/Secondary Market Risk**: Under normal market conditions, Commerzbank will endeavor to provide a secondary market price. However, Commerzbank has no obligation to make a secondary market in the instruments concerned. Accordingly, under some circumstances, the secondary market for the investment may be limited and subject to wide bid/offer spreads.
5. **Reinvestment Risk**: The risk that the investment redeems prior to maturity at a time when reinvestment opportunities are not favorable to the investor.
6. **Redemption Risk**: The risk that the investor may receive substantially less than the amount invested if he/she liquidates the investment prior to maturity.
7. **Tax Risk**: There may be tax implications based on where the investor resides. Please consult a tax professional before investing.
8. **Legal Risk**: There may be legal restrictions depending on where the investor is domiciled. It is advised to seek legal guidance prior to investing. For additional information on the product features and risks, please contact your sales advisor or refer to the contacts page.
