#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <string>
#include <iomanip>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

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
    Contract(const vector<vector<double>>& S, double Strike, double Notional, double UpfrontPremium, double Leverage, double r, int monthDay, int month_max, int delayDay, int TargetCount, double Target)
        : S(S), Strike(Strike), Notional(Notional), UpfrontPremium(UpfrontPremium), Leverage(Leverage), r(r), monthDay(monthDay), month_max(month_max), delayDay(delayDay), TargetCount(TargetCount), Target(Target) {
        NumPath = S.size();
        month_max = min(month_max, int(S[0].size() / monthDay) - 1);
        calculatePayoffs();
    }

    void displayResults(Ui::MainWindow* ui) const {
        ui->label_mean->setText(QString::number(mean(pay), 'f', 4));
        ui->label_std->setText(QString::number(standardError(pay), 'f', 4));
        ui->label_num->setText(QString::number(accumulate(earlyExit.begin(), earlyExit.end(), 0.0), 'f', 4));
        ui->label_prob->setText(QString::number(accumulate(earlyExit.begin(), earlyExit.end(), 0.0) / NumPath, 'f', 4));
        ui->label_bigloss->setText(QString::number(quantile(pay, 0.05), 'f', 4));
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    int T = 1;

    double S0      = ui->lineEdit_S0->text().toDouble();
    double sigma   = ui->lineEdit_Sigma->text().toDouble();
    double r1      = ui->lineEdit_r1->text().toDouble();
    double r2      = ui->lineEdit_r2->text().toDouble();
    int NumPath    = ui->lineEdit_NumPath->text().toInt();

    int n = 240; // 12 months * 20 trading days per month

    // Simulation
    Simulation sim(S0, sigma, r1, r2, T, NumPath, n);

    // Contract Parameters
    int monthDay = 20;
    int delayDay = 0;
    int month_max = 12;

    double Strike         = ui->lineEdit_Strike->text().toDouble();
    double Notional       = ui->lineEdit_Notional->text().toDouble();
    double UpfrontPremium = ui->lineEdit_UpfrontPremium->text().toDouble();
    double Leverage       = ui->lineEdit_Leverage->text().toDouble();
    double Target         = ui->lineEdit_Target->text().toDouble();
    int TargetCount       = ui->lineEdit_TargetCount->text().toInt();

    // Contract
    Contract contract(sim.getPaths(), Strike, Notional, UpfrontPremium, Leverage, r1, monthDay, month_max, delayDay, TargetCount, Target);

    // Display results on UI
    contract.displayResults(ui);
}
