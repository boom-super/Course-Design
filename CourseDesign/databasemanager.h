#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

// 引入Qt核心头文件
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>
#include <QList>
#include <QString>
#include <QMessageBox>

// 单例模式：全局唯一数据库管理类
class DatabaseManager : public QObject
{
    Q_OBJECT // Qt信号槽必须加此宏

public:
    // 1. 获取单例实例（全局唯一）
    static DatabaseManager& getInstance() {
        static DatabaseManager instance; // 局部静态变量，保证唯一
        return instance;
    }

    // 禁止拷贝和赋值（单例模式核心）
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // 2. 核心数据库操作接口（对外暴露）
    bool connectDatabase();                  // 连接数据库
    QVariantMap login(const QString& username, const QString& password); // 用户登录
    bool addActivity(const QString& title, const QString& time, const QString& organizer, int maxNum); // 发布活动
    QList<QVariantMap> getActivities(const QString& role, const QString& username); // 获取活动列表（按角色过滤）
    bool approveActivity(int activityId, bool isApproved); // 审批活动
    bool applyActivity(int activityId, const QString& studentId, QString& errMsg); // 报名活动
    bool cancelApply(int activityId, const QString& studentId); // 取消报名
    QList<QVariantMap> getApplyList(int activityId); // 获取活动报名列表
    bool checkConflict(const QString& studentId, const QString& activityTime); // 检测报名冲突
    QVariantMap getActivityById(int activityId);
    bool exportCSV(int activityId, const QString& filePath);

private:
    // 私有化构造/析构函数（禁止外部创建对象）
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    QSqlDatabase m_db; // 数据库连接对象（核心）
};

#endif // DATABASEMANAGER_H
