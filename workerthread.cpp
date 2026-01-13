#include "workerthread.h"

WorkerThread::WorkerThread(QObject *parent) : QThread(parent) {}

void WorkerThread::setTask(TaskType type, const QVariantMap& params) {
    m_taskType = type;
    m_params = params;
}

void WorkerThread::run() {
    bool success = false;
    QString msg;
    switch (m_taskType) {
    case CheckConflict: {
        QString studentId = m_params["studentId"].toString();
        int activityId = m_params["activityId"].toInt();
        success = !DatabaseManager::getInstance().checkTimeConflict(studentId, activityId);
        msg = success ? "无时间冲突，可以报名！" : "该时间段已报名其他活动，报名冲突！";
        break;
    }
    case ExportCSV: {
        int activityId = m_params["activityId"].toInt();
        QString filePath = m_params["filePath"].toString();
        success = exportCSV(filePath, activityId);
        msg = success ? "CSV导出成功！" : "CSV导出失败！";
        break;
    }
    }
    emit taskFinished(success, msg);
}

bool WorkerThread::exportCSV(const QString& filePath, int activityId) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    // 写入表头
    out << "活动ID,学生ID,报名时间,状态\n";
    // 写入报名数据
    QList<QVariantMap> applyList = DatabaseManager::getInstance().getApplyList(activityId);
    for (const auto& item : applyList) {
        out << item["activity_id"].toString() << ","
            << item["student_id"].toString() << ","
            << item["apply_time"].toString() << ","
            << item["status"].toString() << "\n";
    }
    file.close();
    return true;
}

bool WorkerThread::checkConflict(const QString& studentId, int activityId) {
    return DatabaseManager::getInstance().checkTimeConflict(studentId, activityId);
}
