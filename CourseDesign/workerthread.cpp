#include "workerthread.h"
#include "databasemanager.h"
#include <QVariantMap>
#include <QDebug>

WorkerThread::WorkerThread(QObject *parent) : QThread(parent)
{
    m_taskType = None;
}

WorkerThread::~WorkerThread()
{
    quit();
    wait();
}

void WorkerThread::setTask(TaskType type, const QVariantMap &params)
{
    m_taskType = type;
    m_params = params;
}

void WorkerThread::run()
{
    bool success = false;
    QString msg = "";

    switch (m_taskType)
    {
    case CheckConflict: // 冲突检测任务
    {
        // 1. 获取传入的参数
        QString studentId = m_params["studentId"].toString();
        int activityId = m_params["activityId"].toInt();

        // 2. 调用补全的 getActivityById 接口，获取活动时间
        QVariantMap activity = DatabaseManager::getInstance().getActivityById(activityId);
        if (activity.isEmpty()) {
            success = false;
            msg = "活动信息不存在！";
            break;
        }
        QString activityTime = activity["time"].toString();

        // 3. 调用正确的 checkConflict 接口（无拼写错误）
        success = DatabaseManager::getInstance().checkConflict(studentId, activityTime);

        // 4. 组装提示信息
        msg = success ? "无时间冲突，可正常报名！" : "该时间段已报名其他活动，报名冲突！";
        break;
    }
    case ExportCSV: // CSV导出任务
    {
        int activityId = m_params["activityId"].toInt();
        QString filePath = m_params["filePath"].toString();

        success = DatabaseManager::getInstance().exportCSV(activityId, filePath);
        msg = success ? "CSV导出成功！文件路径：" + filePath : "CSV导出失败！";
        break;
    }
    case None:
    default:
        success = false;
        msg = "未设置任务类型！";
        break;
    }

    // 发射信号返回结果给UI层
    emit taskFinished(success, msg);
}
