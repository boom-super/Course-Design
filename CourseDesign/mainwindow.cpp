#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineEdit>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_loginView(nullptr)
    , m_mainView(nullptr)
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
    delete m_loginView;
    delete m_mainView;
}

void MainWindow::onLoginSuccess(const QString& username, const QString& role) {
    // 创建主功能视图
    m_mainView = new formreportview(username, role, this);
    ui->stackedWidget->addWidget(m_mainView);
    ui->stackedWidget->setCurrentWidget(m_mainView);

    ui->stackedWidget->setCurrentWidget(m_mainView);
}

void MainWindow::on_btBack_clicked()
{
    ui->stackedWidget->setCurrentWidget(m_loginView);
    QLineEdit* pwdEdit = m_loginView->findChild<QLineEdit*>("lineEditPwd");
    if (pwdEdit) pwdEdit->clear();
}


void MainWindow::on_btLogout_clicked()
{
    if (QMessageBox::question(this, "确认", "是否退出登录？") == QMessageBox::Yes) {
        // 切回登录页并清空密码
        ui->stackedWidget->setCurrentWidget(m_loginView);
        QLineEdit* pwdEdit = m_loginView->findChild<QLineEdit*>("lineEditPwd");
        if (pwdEdit) pwdEdit->clear();
        // 可选：清空用户名
        QLineEdit* userEdit = m_loginView->findChild<QLineEdit*>("lineEditUser");
        if (userEdit) userEdit->clear();
    }
}

