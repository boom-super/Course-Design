#include "approvaldialog.h"
#include "ui_approvaldialog.h"
#include "databasemanager.h"
#include <QMessageBox>

approvaldialog::approvaldialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::approvaldialog)
{
    ui->setupUi(this);
    loadPendingActivities(); // 加载待审批活动
}

approvaldialog::~approvaldialog()
{
    delete ui;
}

void approvaldialog::loadPendingActivities() {
    ui->pendingList->clear();
    QList<QVariantMap> pendingList = DatabaseManager::getInstance().getPendingActivities();
    for (const auto& act : pendingList) {
        QString itemText = QString("%1 - %2（发起人：%3）")
                .arg(act["id"].toString())
                .arg(act["title"].toString())
                .arg(act["organizer"].toString());
        ui->pendingList->addItem(itemText);
    }
}

void approvaldialog::on_btnPass_clicked()
{
    if (!ui->pendingList->currentItem()) {
            QMessageBox::warning(this, "提示", "请选择要审批的活动！");
            return;
        }
        int activityId = ui->pendingList->currentItem()->text().split(" - ").first().toInt();
        bool ok = DatabaseManager::getInstance().approveActivity(activityId, true);
        if (ok) {
            QMessageBox::information(this, "成功", "活动审批通过！");
            loadPendingActivities(); // 刷新列表
        } else {
            QMessageBox::critical(this, "失败", "审批失败！");
        }
}


void approvaldialog::on_btnReject_clicked()
{
    if (!ui->pendingList->currentItem()) {
            QMessageBox::warning(this, "提示", "请选择要审批的活动！");
            return;
        }
        int activityId = ui->pendingList->currentItem()->text().split(" - ").first().toInt();
        bool ok = DatabaseManager::getInstance().approveActivity(activityId, false);
        if (ok) {
            QMessageBox::information(this, "成功", "活动审批驳回！");
            loadPendingActivities(); // 刷新列表
        } else {
            QMessageBox::critical(this, "失败", "审批失败！");
        }
}

