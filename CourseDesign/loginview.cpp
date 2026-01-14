#include "loginview.h"
#include "ui_loginview.h"
#include "databasemanager.h"
#include "formreportview.h"
#include <QMessageBox>
#include <QScreen> // 核心：引入QScreen头文件

loginview::loginview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::loginview)
{
    ui->setupUi(this);
    // 窗口基础设置
    ui->lineEditPwd->setEchoMode(QLineEdit::Password);
}

loginview::~loginview()
{
    delete ui;
}

void loginview::on_btnlogin_2_clicked() {
    QString username = ui->lineEditUser->text().trimmed();
    QString password = ui->lineEditPwd->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名/密码不能为空！");
        return;
    }

    QVariantMap userInfo = DatabaseManager::getInstance().login(username, password);
    if (userInfo.isEmpty()) {
        QMessageBox::critical(this, "失败", "用户名或密码错误！");
        ui->lineEditPwd->clear();
        return;
    }

    QString role = userInfo["role"].toString();
        emit loginSuccess(username, role);
}

void loginview::on_btnRegister_2_clicked()
{
    // 临时实现：弹出提示（后续可扩展真实注册逻辑）
    QMessageBox::information(this, "提示", "注册功能暂未实现！");

    // （可选）如果需要实现真实注册逻辑，示例如下：
    // QString username = ui->lineEditRegUsername->text().trimmed();
    // QString password = ui->lineEditRegPassword->text().trimmed();
    // bool ok = DatabaseManager::getInstance().registerUser(username, password);
    // if (ok) {
    //     QMessageBox::information(this, "成功", "注册成功！");
    // } else {
    //     QMessageBox::critical(this, "失败", "注册失败，用户名已存在！");
    // }
}
