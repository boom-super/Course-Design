#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QVariantMap>
#include <QFile>
#include <QTextStream>
#include "databasemanager.h"

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    enum TaskType { CheckConflict, ExportCSV };
    explicit WorkerThread(QObject *parent = nullptr);

    // 设置任务类型和参数
    void setTask(TaskType type, const QVariantMap& params);

signals:
    // 任务完成信号
    void taskFinished(bool success, const QString& msg, const QVariant& data = QVariant());

protected:
    void run() override;

private:
    TaskType m_taskType;
    QVariantMap m_params;

    // 导出CSV实现
    bool exportCSV(const QString& filePath, int activityId);
    // 检测冲突实现
    bool checkConflict(const QString& studentId, int activityId);
};

#endif // WORKERTHREAD_H
