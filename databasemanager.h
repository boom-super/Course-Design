#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QList>
#include <QVariantMap>

class DatabaseManager {
public:
    static DatabaseManager& getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    // 初始化数据库
    bool initDatabase();

    // 用户登录验证（返回角色：admin/organizer/student，空字符串表示失败）
    QString login(const QString& username, const QString& password);

    // 活动相关接口
    bool addActivity(const QString& title, const QString& time, const QString& organizer, int maxNum);
    bool approveActivity(int activityId, bool isApproved);
    QList<QVariantMap> getActivities(const QString& role, const QString& userId);
    QList<QVariantMap> getPendingActivities(); // 获取待审批活动
    QVariantMap getActivityInfo(int activityId); // 获取活动详情

    // 报名相关接口
    bool applyActivity(int activityId, const QString& studentId, QString& errMsg);
    bool cancelApply(int activityId, const QString& studentId);
    QList<QVariantMap> getApplyList(int activityId); // 获取报名列表
    bool checkTimeConflict(const QString& studentId, int activityId); // 检测时间冲突

private:
    DatabaseManager() {}
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
