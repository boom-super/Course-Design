#include "approvaldialog.h"
#include "ui_approvaldialog.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QDebug> // 可选：用于调试

approvaldialog::approvaldialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::approvaldialog),
    m_selectedActivityId(-1)
{
    ui->setupUi(this);
    this->setWindowTitle("活动审批");
    this->setFixedSize(600, 400);
    loadPendingActivities(); // 加载待审批活动
}

approvaldialog::~approvaldialog()
{
    delete ui;
}

// 核心修复：补充列表项选中变化的槽函数（解决未定义引用错误）
void approvaldialog::on_pendingList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous); // 抑制未使用参数警告
    // 选中项变化时，更新当前活动ID
    if (current) {
        m_selectedActivityId = current->data(Qt::UserRole).toInt();
        qDebug() << "选中活动ID：" << m_selectedActivityId; // 调试用，可选
    } else {
        m_selectedActivityId = -1; // 未选中时置为无效ID
    }
}

void approvaldialog::loadPendingActivities() {
    // 清空列表（UI中控件objectName必须是pendingList）
    ui->pendingList->clear();
    m_selectedActivityId = -1; // 重置选中ID

    // 从数据库获取管理员可见的所有活动
    QList<QVariantMap> activities = DatabaseManager::getInstance().getActivities("admin", "");
    for (const auto& act : activities) {
        // 仅筛选待审批活动（approved=0）
        if (act["approved"].toInt() == 0) {
            // 拼接显示文本
            QString itemText = QString("%1 - %2（发起人：%3 | 时间：%4 | 最大人数：%5）")
                    .arg(act["id"].toString())
                    .arg(act["title"].toString())
                    .arg(act["organizer"].toString())
                    .arg(act["time"].toString())
                    .arg(act["max_num"].toString());
            // 创建列表项，存储活动ID（隐藏数据）
            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setData(Qt::UserRole, act["id"].toInt());
            ui->pendingList->addItem(item);
        }
    }

    // 默认选中第一个活动
    if (ui->pendingList->count() > 0) {
        ui->pendingList->setCurrentRow(0);
        // 直接从隐藏数据获取ID，避免字符串分割
        m_selectedActivityId = ui->pendingList->currentItem()->data(Qt::UserRole).toInt();
    } else {
        QMessageBox::information(this, "提示", "暂无待审批的活动！");
    }
}

void approvaldialog::on_btnPass_clicked()
{
    // 检查是否选中活动（用成员变量，更简洁）
    if (m_selectedActivityId == -1) {
        QMessageBox::warning(this, "提示", "请选择要审批的活动！");
        return;
    }

    // 调用审批接口（通过：true）
    bool ok = DatabaseManager::getInstance().approveActivity(m_selectedActivityId, true);
    if (ok) {
        QMessageBox::information(this, "成功", "活动审批通过！");
        loadPendingActivities(); // 刷新列表
    } else {
        QMessageBox::critical(this, "失败", "审批失败！");
    }
}

void approvaldialog::on_btnReject_clicked()
{
    // 统一用成员变量检查选中状态
    if (m_selectedActivityId == -1) {
        QMessageBox::warning(this, "提示", "请选择要审批的活动！");
        return;
    }

    // 调用审批接口（驳回：false）
    bool ok = DatabaseManager::getInstance().approveActivity(m_selectedActivityId, false);
    if (ok) {
        QMessageBox::information(this, "成功", "活动已驳回！");
        loadPendingActivities(); // 刷新列表
    } else {
        QMessageBox::critical(this, "失败", "审批失败！");
    }
}
