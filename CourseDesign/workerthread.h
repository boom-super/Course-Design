#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QString>
#include <QVariantMap>

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    // 任务类型枚举
    enum TaskType {
        None,        // 无任务
        CheckConflict, // 冲突检测
        ExportCSV    // CSV导出
    };

    explicit WorkerThread(QObject *parent = nullptr);
    ~WorkerThread() override;

    // 设置任务类型和参数
    void setTask(TaskType type, const QVariantMap &params);

signals:
    // 任务完成信号：参数1=是否成功，参数2=提示信息
    void taskFinished(bool success, const QString &msg);

protected:
    // 线程执行函数（重写QThread的run方法）
    void run() override;

private:
    TaskType m_taskType;    // 当前任务类型
    QVariantMap m_params;   // 任务参数
};

#endif // WORKERTHREAD_H
