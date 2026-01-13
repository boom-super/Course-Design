#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWidget>
#include <QMessageBox>
#include "databasemanager.h"
#include "formreportview.h"

namespace Ui {
class loginview;
}

class loginview : public QWidget
{
    Q_OBJECT

public:
    explicit loginview(QWidget *parent = nullptr);
    ~loginview();

signals:
    // 登录成功信号（传递用户名和角色）
    void loginSuccess(const QString& username, const QString& role);

private slots:
    void on_btnRegister_2_clicked();

    void on_btnlogin_2_clicked();

private:
    Ui::loginview *ui;
};

#endif // LOGINVIEW_H
