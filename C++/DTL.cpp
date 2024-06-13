#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <string>
#include <iomanip>

using namespace std;

class Simulation {
public:
    Simulation(double S0, double sigma, double r1, double r2, int T, int NumPath, int n, unsigned int seed = 123457, bool seed_fixed = true)
        : S0(S0), sigma(sigma), r1(r1), r2(r2), T(T), NumPath(NumPath), n(n), seed(seed), seed_fixed(seed_fixed) {
        dt = T / double(n);
        mu = r1 - r2 - pow(sigma, 2) / 2;
        generatePaths();
    }

    const vector<vector<double>>& getPaths() const {
        return S;
    }

private:
    double S0, sigma, r1, r2, dt, mu;
    int T, NumPath, n;
    unsigned int seed;
    bool seed_fixed;
    vector<vector<double>> S;

    void generatePaths() {
        mt19937 generator(seed_fixed ? seed : random_device{}());
        normal_distribution<double> dist(0, 1);

        S.resize(NumPath, vector<double>(n + 1, S0));
        for (int i = 1; i <= n; ++i) {
            for (int j = 0; j < NumPath; ++j) {
                double z = dist(generator);
                S[j][i] = S[j][i - 1] * exp(mu * dt + sigma * sqrt(dt) * z);
            }
        }
    }
};

class Contract {
public:
    Contract(const vector<vector<double>>& S, double Strike, double Notional, double UpfrontPremium, double Leverage, double r, int monthDay, int T, int delayDay, int TargetCount, double Target)
        : S(S), Strike(Strike), Notional(Notional), UpfrontPremium(UpfrontPremium), Leverage(Leverage), r(r), monthDay(monthDay), delayDay(delayDay), TargetCount(TargetCount), Target(Target) {
        NumPath = S.size();

        // Set month_max based on T
        if (T == 1) {
            month_max = 12;
        }
        else {
            month_max = 24;
        }

        // Adjust month_max based on the path size
        month_max = min(month_max, int(S[0].size() / monthDay) - 1);

        calculatePayoffs();
    }

    void displayResults() const {
        cout << fixed << setprecision(5);

        cout << "商品之評價為 " << mean(pay) << endl;
        cout << "商品之評價的標準差為 " << standardError(pay) << endl;
        cout << "提前出場次數為 " << accumulate(earlyExit.begin(), earlyExit.end(), 0.0) << endl;
        cout << "提前出場機率為 " << accumulate(earlyExit.begin(), earlyExit.end(), 0.0) / NumPath << endl;
        cout << "當商品之評價低於 " << quantile(pay, 0.05) << " 時會發生極端損失" << endl;
    }

private:
    const vector<vector<double>>& S;
    double Strike, Notional, UpfrontPremium, Leverage, r;
    int monthDay, month_max, delayDay, TargetCount, NumPath;
    double Target;
    vector<double> pay;
    vector<double> earlyExit;

    void calculatePayoffs() {
        pay.resize(NumPath, 0.0);
        earlyExit.resize(NumPath, 0.0);

        for (int i = 0; i < NumPath; ++i) {
            double cashFlow = 0.0;
            for (int month = 1; month <= month_max; ++month) {
                int day = monthDay * month + delayDay;
                if (day >= S[i].size()) break;

                double spot = S[i][day];
                double payoff = max(0.0, Strike - spot);

                if (spot >= Strike) {
                    cashFlow += (Notional * spot - Notional * Strike) * exp(-r * day / 252.0);
                    earlyExit[i] = 1;
                    break;
                }
                else {
                    cashFlow += (Notional * Leverage * spot - Notional * Leverage * Strike) * exp(-r * day / 252.0);
                }

                if (payoff <= Target || accumulate(earlyExit.begin(), earlyExit.end(), 0) >= TargetCount) {
                    earlyExit[i] = 1;
                    break;
                }
            }
            pay[i] = cashFlow + UpfrontPremium * exp(-r / 252.0);
        }
    }

    double mean(const vector<double>& v) const {
        return accumulate(v.begin(), v.end(), 0.0) / v.size();
    }

    double standardError(const vector<double>& v) const {
        double m = mean(v);
        double variance = accumulate(v.begin(), v.end(), 0.0, [&](double a, double b) { return a + (b - m) * (b - m); }) / (v.size() - 1);
        return sqrt(variance) / sqrt(v.size());
    }

    double quantile(const vector<double>& v, double q) const {
        vector<double> sorted(v);
        sort(sorted.begin(), sorted.end());
        size_t index = static_cast<size_t>(q * sorted.size());
        return sorted[index];
    }
};

int main() {
    // Parameters
    double S0 = 1.1824;
    double sigma = 0.07134707139575819;
    double r1 = 0.009;
    double r2 = 0.012;
    int T = 1; // Change this to 2 for T=2
    int NumPath = 100000;
    int monthDay = 20;
    int n = monthDay * 12 * T; // Dynamic n based on T

    // Simulation
    Simulation sim(S0, sigma, r1, r2, T, NumPath, n);

    // Contract Parameters
    double Strike = 1.335;
    double Notional = 1000000; // EUR
    double UpfrontPremium = 50000; // USD
    double Leverage = 2;
    double Target = 0.1; // USD per EUR
    int TargetCount = 4;
    int delayDay = 0;

    // Contract
    Contract contract(sim.getPaths(), Strike, Notional, UpfrontPremium, Leverage, r1, monthDay, T, delayDay, TargetCount, Target);

    // Results
    contract.displayResults();

    return 0;
}
