
using Formatting
using Statistics
using StatsBase: percentile
using Random
using Distributions
using LinearAlgebra
using PyCall: pyimport
using PyPlot: pygui, plt
using Dates: today
using DataFrames, PrettyTables
pygui(true)

# import packages from python
datareader = pyimport("pandas_datareader")
pd = pyimport("pandas")

## 抓美元兌歐元即期匯率資料

ticker = "DEXUSEU"

start_date = "2013-05-15"
end_date = "2014-05-15"
#start_date = "2019-10-18"
#end_date = "2020-10-20"

FX = datareader.DataReader(ticker, "fred", start_date, end_date)[ticker].dropna().values
日報酬率 = diff(log.(FX),dims=1)

## 參數估計

Mu = vec(mean(日報酬率,dims=1)*252)
Sigma = std(日報酬率,dims=1)*√252
S0 = FX[end]

d = length(S0)                # 維度
r = [0.009, 0.012]            # 假設 12 個月美元 LIBOR Rate 與 12 個月歐元 LIBOR Rate
monthDay = 20                 # 每月模擬天數
yearDay = 12 * monthDay       # 一年模擬天數
delayDay = 0                  # 延遲天數
n = yearDay + delayDay        # 總模擬天數
T = n/yearDay
Δt = T/n
t = [j*Δt for j=0:n]
NumPath = 100000              # 模擬 path 數

function Simulation(S0, sigma, r, T, NumPath, n, seed=123457; seed_fixed=true)
    if seed_fixed
        Random.seed!(seed)
    end
	normal = Distributions.Normal(0, 1)
    z = Random.rand(normal, (NumPath, n))

    dt = T/n
    mu = r[1]-r[2]-sigma[1]^2/2
    # mu = r-sigma^2/2
    R = fill(NaN, (NumPath, n+1))
    R[:,2:end] = mu*dt .+ sigma[1]*sqrt(dt)*z

	S = S0*ones(NumPath, n+1)
    for j=2:n
        S[:,j+1] = S[:,j] .* exp.(R[:,j+1])
	end
    return S
end

S = Simulation(S0, Sigma, r, T, NumPath, n; seed_fixed=true)

# 畫出模擬路徑圖檢查
fontsize = 10

function plot_simulation(S::Matrix)
    NumPath = size(S, 1)
    n = size(S, 2)-1
    [plt.plot(t, S[i,:], linewidth=0.5) for i=1:NumPath]
    plt.tick_params(labelsize=fontsize)
    plt.xlabel("t", fontsize=fontsize)
    plt.ylabel("FX", fontsize=fontsize)
    title = "Simulation Paths of GBM (Num of Path=$NumPath, n=$n)"
    plt.title(title, fontsize=fontsize)
end

plt.figure()
plot_simulation(S)
plt.show()

## 合約訂定

Notional = 1_000_000     # EUR
UpfrontPremium = 50_000  # USD
Strike = 1.335           # USD per EUR
Target = 0.1             # USD per EUR
TargetCount = 4
Leverage = 2
month_max = 12           # 共配息 12 個月

# target
現金流量折現 = zeros(NumPath, month_max)
提前出場 = zeros(NumPath,month_max)
點數 = zeros(NumPath,month_max)
次數 = zeros(NumPath,month_max)

## 計算 payoff

for i in 1:NumPath
	for month=1:month_max
		點數[i,month] = max(0,Strike-S[i,monthDay*month+delayDay])
        if S[i,monthDay*month+delayDay] >= Strike
            次數[i,month] = 1
        end
        if sum(點數[i,:])>=Target || sum(次數[i,:])==TargetCount
            提前出場[i,month] = 1
            if S[i,monthDay*month+delayDay] >= Strike
            現金流量折現[i,month] = ( Notional * S[i,monthDay*month+delayDay] - Notional * Strike ) * exp(-r[1]*t[monthDay*month+delayDay])
            end
            break
        else
            if S[i,monthDay*month+delayDay] >= Strike
                現金流量折現[i,month] = ( Notional * S[i,monthDay*month+delayDay] - Notional * Strike ) * exp(-r[1]*t[monthDay*month+delayDay])
            else
                現金流量折現[i,month] = ( Notional * Leverage * S[i,monthDay*month+delayDay] - Notional * Leverage * Strike ) * exp(-r[1]*t[monthDay*month+delayDay])
            end
        end
	end
end

現金流量折現 = 現金流量折現 + hcat(fill(UpfrontPremium*exp(-r[1]*t[2]),NumPath, 1),fill(0,NumPath, month_max-1))

α = 0.05
pay = sum(現金流量折現, dims=2)
每月出場機率 = sum(提前出場,dims=1)/sum(sum(提前出場,dims=1))
vec(每月出場機率)
各期出場損益 = sum(現金流量折現, dims=1)/NumPath
vec(各期出場損益)

# 次數
plt.figure()
plt.hist(pay, 35, facecolor="royalblue", edgecolor="black", alpha=0.6)
approx = percentile(pay[:], α)
plt.xlabel("payoff",fontsize=15)
plt.ylabel("frequency",fontsize=15)
plt.title("PAYOFF HISTOGRAM - COUNT",fontweight="bold",fontsize=20)
plt.show()

# (a) 於交易日 (期初), 利用模擬方法探討此結構型商品之評價。
printfmt("商品之評價為 {:.5f}", mean(pay))

# (b) 於交易日 (期初), 利用模擬方法探討此結構型商品之評價的標準差。
# 用樣本標準差去猜母體標準差
printfmt("商品之評價的標準差為 {:.5f}", std(pay)/√NumPath)

# (c) 於交易日 (期初), 利用模擬方法探討此結構型商品之提前出場機率。
提前出場次數 = sum(sum(提前出場, dims=2))
printfmt("提前出場次數為 {:.5f}", sum(sum(提前出場, dims=2)))
提前出場機率 = 提前出場次數/NumPath
printfmt("提前出場機率為 {:.5f}", 提前出場機率)

# (e) 於交易日 (期初), 利用模擬方法探討此結構型商品之風險衡量 (VaR or ExpectedShortfall or …)。
#     大約是何情況會發生極端損失?
printfmt("當商品之評價低於 {:.5f}", -quantile(vec(pay),α))
println(" 時會發生極端損失")
