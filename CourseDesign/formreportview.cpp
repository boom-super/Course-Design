#include "formreportview.h"
#include "ui_formreportview.h"

formreportview::formreportview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::formreportview),
    m_username(username),
    m_role(role),
    m_currentActivityId(-1),
    m_conflictThread(new WorkerThread(this))
{
    ui->setupUi(this);
    // 角色适配：隐藏非当前角色的按钮
    ui->btnPublish->setVisible(role == "organizer");
    ui->btnApprove->setVisible(role == "admin");
    ui->btnApply->setVisible(role == "student");
    ui->btnCancel->setVisible(role == "student");
    ui->btnCheckConflict->setVisible(role == "student");
    ui->btnExport->setVisible(role != "student");

    // 连接冲突检测线程信号
    connect(m_conflictThread, &WorkerThread::taskFinished, this, &formreportview::onConflictCheckFinished);

    // 加载活动列表
    loadActivities();
}

formreportview::~formreportview()
{
    delete ui;
}

void formreportview::loadActivities() {
    ui->activityList->clear();
    QList<QVariantMap> activities = DatabaseManager::getInstance().getActivities(m_role, m_username);
    for (const auto& act : activities) {
        QString status = act["approved"].toInt() ? "[已审批]" : "[待审批]";
        QString itemText = QString("%1 %2 - %3（时间：%4 | 最大人数：%5）")
                .arg(status)
                .arg(act["id"].toString())
                .arg(act["title"].toString())
                .arg(act["time"].toString())
                .arg(act["max_num"].toString());
        ui->activityList->addItem(itemText);
    }
}

void formreportview::loadApplyList(int activityId) {
    ui->applyTable->setRowCount(0); // 清空表格
    QList<QVariantMap> applyList = DatabaseManager::getInstance().getApplyList(activityId);
    for (int i = 0; i < applyList.size(); ++i) {
        const auto& item = applyList[i];
        ui->applyTable->insertRow(i);
        ui->applyTable->setItem(i, 0, new QTableWidgetItem(item["activity_id"].toString()));
        ui->applyTable->setItem(i, 1, new QTableWidgetItem(item["student_id"].toString()));
        ui->applyTable->setItem(i, 2, new QTableWidgetItem(item["apply_time"].toString()));
        ui->applyTable->setItem(i, 3, new QTableWidgetItem(item["status"].toString()));
    }
    // 统计候补/已报名人数
    int normal = 0, backup = 0;
    for (const auto& item : applyList) {
        item["status"] == "normal" ? normal++ : backup++;
    }
    ui->labelStatus->setText(QString("已报名：%1 | 候补：%2").arg(normal).arg(backup));
}

void formreportview::on_activityList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);
    if (current) {
        // 提取活动ID
        QString itemText = current->text();
        m_currentActivityId = itemText.split(" ")[1].toInt();
        loadApplyList(m_currentActivityId);
    } else {
        m_currentActivityId = -1;
    }
}

void formreportview::on_btnPublish_clicked()
{
    publishview *publishDlg = new publishview(m_username, this);
    publishDlg->setWindowModality(Qt::ApplicationModal);
    publishDlg->show();
    connect(publishDlg, &publishview::destroyed, this, &formreportview::loadActivities);
}


void formreportview::on_btnApprove_clicked()
{
    approvaldialog *approveDlg = new approvaldialog(this);
    approveDlg->setWindowModality(Qt::ApplicationModal);
    approveDlg->show();
    connect(approveDlg, &approvaldialog::destroyed, this, &formreportview::loadActivities);
}


void formreportview::on_btnApply_clicked()
{
    if (m_currentActivityId == -1) {
        QMessageBox::warning(this, "提示", "请选择要报名的活动！");
        return;
    }
    QString errMsg;
    bool ok = DatabaseManager::getInstance().applyActivity(m_currentActivityId, m_username, errMsg);
    QMessageBox::information(this, ok ? "成功" : "失败", errMsg);
    loadApplyList(m_currentActivityId);
}


void formreportview::on_btnCancel_clicked()
{
    if (m_currentActivityId == -1) {
        QMessageBox::warning(this, "提示", "请选择要检测的活动！");
        return;
    }
    // 设置冲突检测任务
    QVariantMap params;
    params["studentId"] = m_username;
    params["activityId"] = m_currentActivityId;
    m_conflictThread->setTask(WorkerThread::CheckConflict, params);
    m_conflictThread->start();
}


void formreportview::on_btnCheckConflict_clicked()
{
    QMessageBox::information(this, "冲突检测结果", msg);
}

void formreportview::on_btnExport_clicked() {
    if (m_currentActivityId == -1) {
        QMessageBox::warning(this, "提示", "请选择要导出的活动！");
        return;
    }
    ExportDialog *exportDlg = new ExportDialog(m_currentActivityId, this);
    exportDlg->setWindowModality(Qt::ApplicationModal);
    exportDlg->show();
}
