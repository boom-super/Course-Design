#include "loginview.h"
#include "ui_loginview.h"

loginview::loginview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::loginview)
{
    ui->setupUi(this);
    // 初始化数据库
    if (!DatabaseManager::getInstance().initDatabase()) {
        QMessageBox::critical(this, "错误", "数据库初始化失败！");
    }
}

loginview::~loginview()
{
    delete ui;
}

void loginview::on_btnRegister_2_clicked()
{
    QString username = ui->lineEditUser->text().trimmed();
    QString password = ui->lineEditPwd->text().trimmed();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名/密码不能为空！");
        return;
    }
    // 登录验证
    QString role = DatabaseManager::getInstance().login(username, password);
    if (role.isEmpty()) {
        QMessageBox::critical(this, "错误", "用户名或密码错误！");
        return;
    }
    // 发送登录成功信号
    emit loginSuccess(username, role);
}


void loginview::on_btnlogin_2_clicked()
{
    QMessageBox::information(this, "提示", "简易版暂不支持注册，测试账号：\n管理员：admin/admin\n发起人：org1/org1\n学生：stu1/stu1");
}

