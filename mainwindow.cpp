#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineEdit>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("校园活动报名与签到管理系统");
    // 初始化登录视图
    m_loginView = new loginview(this);
    ui->stackedWidget->addWidget(m_loginView);
    ui->stackedWidget->setCurrentWidget(m_loginView);
    // 连接登录成功信号
    connect(m_loginView, &loginview::loginSuccess, this, &MainWindow::onLoginSuccess);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLoginSuccess(const QString& username, const QString& role) {
    // 创建主功能视图
    m_mainView = new formreportview(username, role, this);
    ui->stackedWidget->addWidget(m_mainView);
    ui->stackedWidget->setCurrentWidget(m_mainView);
}

void MainWindow::on_btBack_clicked()
{
    ui->stackedWidget->setCurrentWidget(m_loginView);
}


void MainWindow::on_btLogout_clicked()
{
    if (QMessageBox::question(this, "确认", "是否退出登录？") == QMessageBox::Yes) {
        ui->stackedWidget->setCurrentWidget(m_loginView);
        // 清空密码框
        m_loginView->findChild<QLineEdit*>("lineEditPwd")->clear();
    }
}

