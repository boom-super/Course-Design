#ifndef FORMREPORTVIEW_H
#define FORMREPORTVIEW_H

#include <QWidget>
#include <QMessageBox>
#include "databasemanager.h"
#include "approvaldialog.h"
#include "exportdialog.h"
#include "publishview.h"
#include "workerthread.h"

namespace Ui {
class formreportview;
}

class formreportview : public QWidget
{
    Q_OBJECT

public:
    explicit formreportview(QWidget *parent = nullptr);
    ~formreportview();

private slots:
    void on_btnPublish_clicked();

    void on_btnApprove_clicked();

    void on_btnApply_clicked();

    void on_btnCancel_clicked();

    void on_btnCheckConflict_clicked();

    void on_activityList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous); // 选中活动变化
    void on_btnExport_clicked();     // 导出CSV
    void onConflictCheckFinished(bool success, const QString& msg); // 冲突检测回调

private:
    Ui::formreportview *ui;
    QString m_username; // 当前登录用户名
    QString m_role;     // 当前登录角色（admin/organizer/student）
    int m_currentActivityId; // 当前选中的活动ID
    WorkerThread *m_conflictThread; // 冲突检测线程

    // 加载活动列表（按角色过滤）
    void loadActivities();
    // 加载报名列表
    void loadApplyList(int activityId);
};

#endif // FORMREPORTVIEW_H
