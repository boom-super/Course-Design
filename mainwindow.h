#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "loginview.h"
#include "formreportview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btBack_clicked();

    void on_btLogout_clicked();

    void onLoginSuccess(const QString& username, const QString& role);

private:
    Ui::MainWindow *ui;
    loginview *m_loginView;      // 登录视图
    formreportview *m_mainView;  // 主功能视图
};
#endif // MAINWINDOW_H
