#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_clear_clicked();   // 清除所有數值的槽函數
    void on_pushButton_default_clicked(); // 填入預設值的槽函數

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
