#ifndef PUBLISHVIEW_H
#define PUBLISHVIEW_H

#include <QWidget>
#include <QMessageBox>
#include "databasemanager.h"

namespace Ui {
class publishview;
}

class publishview : public QWidget
{
    Q_OBJECT

public:
    explicit publishview(QWidget *parent = nullptr);
    ~publishview();

private slots:
    void on_btnSubmit_clicked(); // 提交发布活动

private:
    Ui::publishview *ui;
    QString m_organizer; // 发起人用户名
};

#endif // PUBLISHVIEW_H
