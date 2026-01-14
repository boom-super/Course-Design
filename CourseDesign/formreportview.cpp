#include "formreportview.h"
#include "ui_formreportview.h"
#include "databasemanager.h"
#include "approvaldialog.h"
#include "exportdialog.h"
#include "publishview.h"
#include "workerthread.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QDateTime>

formreportview::formreportview(const QString& username, const QString& role, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::formreportview),
    m_username(username),
    m_role(role),
    m_currentActivityId(-1),
    m_conflictThread(new WorkerThread(this))
{
    ui->setupUi(this);
    QLabel *testLabel = new QLabel(QString("当前登录：%1（角色：%2）").arg(username).arg(role), this);
    testLabel->setGeometry(10, 10, 300, 30); // 固定位置显示
    testLabel->setStyleSheet("color: red; font-size: 14px;"); // 红色字体，易识别

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
        QString status = act["approved"].toInt() == 1 ? "[已审批]" : (act["approved"].toInt() == 2 ? "[已驳回]" : "[待审批]");
        QString itemText = QString("%1 %2 - %3（时间：%4 | 最大人数：%5）")
                .arg(status)
                .arg(act["id"].toString())
                .arg(act["title"].toString())
                .arg(act["time"].toString())
                .arg(act["max_num"].toString());
        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, act["id"].toInt());
        ui->activityList->addItem(item);
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
    // 1. 创建发布窗口，指定父窗口为当前主窗口
    publishview *publishDlg = new publishview(m_username, this);
    // 2. 设置为模态窗口（阻塞主窗口，无法操作其他内容）
    publishDlg->setModal(true);
    // 3. 固定弹窗尺寸，居中显示
    publishDlg->setFixedSize(500, 350);
    publishDlg->move(this->rect().center() - publishDlg->rect().center());
    // 4. 显示弹窗（exec() 会阻塞，直到弹窗关闭）
    publishDlg->exec();
    // 5. 弹窗关闭后，刷新活动列表
    loadActivities();
    // 6. 自动销毁弹窗，避免内存泄漏
    publishDlg->deleteLater();
}


void formreportview::on_btnApprove_clicked()
{
    approvaldialog *approveDlg = new approvaldialog(this);
    approveDlg->setModal(true);
    approveDlg->setFixedSize(600, 400);
    approveDlg->move(this->rect().center() - approveDlg->rect().center());
    approveDlg->exec();
    loadActivities();
    approveDlg->deleteLater();
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
        QMessageBox::warning(this, "提示", "请选择要取消报名的活动！");
        return;
    }
    bool ok = DatabaseManager::getInstance().cancelApply(m_currentActivityId, m_username);
    if (ok) {
        QMessageBox::information(this, "成功", "取消报名成功！");
        loadApplyList(m_currentActivityId);
    } else {
        QMessageBox::critical(this, "失败", "取消报名失败！");
    }
}


void formreportview::on_btnCheckConflict_clicked()
{
    // 第一步：检查是否选中活动
    if (m_currentActivityId == -1) {
        QMessageBox::warning(this, "提示", "请选择要检测的活动！");
        return;
    }

    // 第二步：准备线程参数，启动冲突检测线程（核心逻辑）
    QVariantMap params;
    params["studentId"] = m_username;    // 当前登录学生ID
    params["activityId"] = m_currentActivityId; // 选中的活动ID
    m_conflictThread->setTask(WorkerThread::CheckConflict, params); // 设置任务类型
    m_conflictThread->start();           // 启动后台线程

    // 可选：给用户友好提示（非必须，仅提升体验）
    QMessageBox::information(this, "提示", "正在检测报名冲突，请稍候...");
}

void formreportview::on_btnExport_clicked() {
    if (m_currentActivityId == -1) {
        QMessageBox::warning(this, "提示", "请先选择要导出的活动！");
        return;
    }

    // 弹出文件保存对话框（系统模态弹窗）
    QString filePath = QFileDialog::getSaveFileName(this, "导出CSV文件",
                                                    QString("%1活动报名数据.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd")),
                                                    "CSV文件 (*.csv)");
    if (filePath.isEmpty()) return;

    // 调用导出接口
    bool ok = DatabaseManager::getInstance().exportCSV(m_currentActivityId, filePath);
    QMessageBox::information(this, "结果", ok ? "导出成功！" : "导出失败！");
}

void formreportview::onConflictCheckFinished(bool success, const QString& msg) {
    // 此时msg是线程返回的检测结果（如「无时间冲突」「报名冲突」）
    QMessageBox::information(this, "冲突检测结果", msg);
}


void formreportview::on_btnLogout_clicked()
{
    this->close();
}

