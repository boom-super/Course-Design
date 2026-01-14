#ifndef PUBLISHVIEW_H
#define PUBLISHVIEW_H

#include <QDialog>
#include <QString>
#include <QMessageBox>
#include "databasemanager.h"

namespace Ui {
class publishview;
}

class publishview : public QDialog
{
    Q_OBJECT
public:
    // 声明：必须带 const QString& organizer 参数
    explicit publishview(const QString& organizer, QWidget *parent = nullptr);
    ~publishview();

private slots:
    void on_btnSubmit_clicked();
    void on_btnCancel_clicked();

private:
    Ui::publishview *ui;
    QString m_organizer; // 存储发起人名称
};

#endif // PUBLISHVIEW_H
