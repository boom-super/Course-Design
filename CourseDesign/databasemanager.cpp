#include "databasemanager.h"
#include <QFile>          // 文件操作类
#include <QTextStream>    // 文本流类（写入CSV需要）
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>
#include <QList>
#include <QString>
#include <QMessageBox>

// 构造函数：私有化，初始化时自动连接数据库
DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    connectDatabase(); // 初始化连接
}

// 析构函数：关闭数据库连接
DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

// 核心：连接SQLite数据库（返回是否连接成功）
bool DatabaseManager::connectDatabase()
{
    // 1. 若已连接，直接返回成功
    if (m_db.isOpen()) {
        return true;
    }

    // 2. 添加SQLite驱动（Qt必须）
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    // 关键：数据库文件路径（和Navicat创建的路径一致！）
    // 替换为你的实际路径，比如 E:/CourseDesign/campus_activity.db
    m_db.setDatabaseName("C:/Users/13045/Desktop/zuoye/qt/campus_activity.db");

    // 3. 打开数据库
    if (!m_db.open()) {
        QMessageBox::critical(nullptr, "数据库错误", "连接失败：" + m_db.lastError().text());
        return false;
    }

    // 4. 开启外键支持（SQLite默认关闭）
    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON;");

    return true;
}

// 接口1：用户登录验证（返回用户信息，空则登录失败）
QVariantMap DatabaseManager::login(const QString& username, const QString& password)
{
    QVariantMap userInfo; // 存储用户信息（id, username, role）
    QSqlQuery query;

    // 准备SQL（防止SQL注入）
    query.prepare("SELECT id, username, role FROM users WHERE username = ? AND password = ?;");
    query.addBindValue(username);
    query.addBindValue(password);

    // 执行查询
    if (query.exec() && query.next()) {
        userInfo["id"] = query.value("id").toInt();
        userInfo["username"] = query.value("username").toString();
        userInfo["role"] = query.value("role").toString();
    }

    return userInfo;
}

// 接口2：发布活动
bool DatabaseManager::addActivity(const QString& title, const QString& time, const QString& organizer, int maxNum)
{
    QSqlQuery query;
    query.prepare("INSERT INTO activities (title, time, organizer, max_num) VALUES (?, ?, ?, ?);");
    query.addBindValue(title);
    query.addBindValue(time);
    query.addBindValue(organizer);
    query.addBindValue(maxNum);

    return query.exec(); // 执行成功返回true
}

// 接口3：按角色获取活动列表
QList<QVariantMap> DatabaseManager::getActivities(const QString& role, const QString& username)
{
    QList<QVariantMap> activityList;
    QSqlQuery query;

    // 不同角色看到不同活动：
    // - 管理员：所有活动
    // - 发起人：自己发布的活动
    // - 学生：已审批的活动
    if (role == "admin") {
        query.exec("SELECT * FROM activities ORDER BY publish_time DESC;");
    } else if (role == "organizer") {
        query.prepare("SELECT * FROM activities WHERE organizer = ? ORDER BY publish_time DESC;");
        query.addBindValue(username);
        query.exec();
    } else if (role == "student") {
        query.exec("SELECT * FROM activities WHERE approved = 1 ORDER BY publish_time DESC;");
    }

    // 遍历结果，存入列表
    while (query.next()) {
        QVariantMap activity;
        activity["id"] = query.value("id").toInt();
        activity["title"] = query.value("title").toString();
        activity["time"] = query.value("time").toString();
        activity["organizer"] = query.value("organizer").toString();
        activity["max_num"] = query.value("max_num").toInt();
        activity["approved"] = query.value("approved").toInt();
        activityList.append(activity);
    }

    return activityList;
}

// 接口4：审批活动（isApproved=true：通过，false：驳回）
bool DatabaseManager::approveActivity(int activityId, bool isApproved)
{
    QSqlQuery query;
    query.prepare("UPDATE activities SET approved = ? WHERE id = ?;");
    query.addBindValue(isApproved ? 1 : 2); // 1=通过，2=驳回
    query.addBindValue(activityId);

    return query.exec();
}

// 接口5：报名活动（含冲突检测+候补队列）
bool DatabaseManager::applyActivity(int activityId, const QString& studentId, QString& errMsg)
{
    // 步骤1：检查活动是否存在且已审批
    QSqlQuery query;
    query.prepare("SELECT title, time, max_num FROM activities WHERE id = ? AND approved = 1;");
    query.addBindValue(activityId);
    if (!query.exec() || !query.next()) {
        errMsg = "活动不存在或未审批！";
        return false;
    }
    QString activityTime = query.value("time").toString();
    int maxNum = query.value("max_num").toInt();

    // 步骤2：检测报名冲突
    if (!checkConflict(studentId, activityTime)) {
        errMsg = "该时间段已报名其他活动，冲突！";
        return false;
    }

    // 步骤3：检查当前报名人数，判断是否候补
    query.prepare("SELECT COUNT(*) FROM applies WHERE activity_id = ? AND status = 'normal';");
    query.addBindValue(activityId);
    query.exec();
    query.next();
    int currentNum = query.value(0).toInt();
    QString status = (currentNum < maxNum) ? "normal" : "backup";

    // 步骤4：插入报名记录
    query.prepare("INSERT INTO applies (activity_id, student_id, status) VALUES (?, ?, ?);");
    query.addBindValue(activityId);
    query.addBindValue(studentId);
    query.addBindValue(status);
    if (!query.exec()) {
        errMsg = "报名失败：" + query.lastError().text();
        return false;
    }

    errMsg = status == "normal" ? "报名成功！" : "人数已满，已加入候补！";
    return true;
}

// 接口6：取消报名
bool DatabaseManager::cancelApply(int activityId, const QString& studentId)
{
    QSqlQuery query;
    query.prepare("UPDATE applies SET status = 'canceled' WHERE activity_id = ? AND student_id = ?;");
    query.addBindValue(activityId);
    query.addBindValue(studentId);

    return query.exec();
}

// 接口7：获取活动报名列表
QList<QVariantMap> DatabaseManager::getApplyList(int activityId)
{
    QList<QVariantMap> applyList;
    QSqlQuery query;

    query.prepare("SELECT * FROM applies WHERE activity_id = ?;");
    query.addBindValue(activityId);
    query.exec();

    while (query.next()) {
        QVariantMap apply;
        apply["id"] = query.value("id").toInt();
        apply["activity_id"] = query.value("activity_id").toInt();
        apply["student_id"] = query.value("student_id").toString();
        apply["apply_time"] = query.value("apply_time").toString();
        apply["status"] = query.value("status").toString();
        applyList.append(apply);
    }

    return applyList;
}

// 接口8：检测报名时间冲突
bool DatabaseManager::checkConflict(const QString& studentId, const QString& activityTime)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT a.time
        FROM applies ap
        JOIN activities a ON ap.activity_id = a.id
        WHERE ap.student_id = ? AND ap.status = 'normal';
    )");
    query.addBindValue(studentId);
    query.exec();

    // 遍历已报名活动，判断时间是否重叠（简化：完全相同则冲突）
    while (query.next()) {
        QString existTime = query.value("time").toString();
        if (existTime == activityTime) {
            return false; // 冲突
        }
    }

    return true; // 无冲突
}

// ========== 新增：根据活动ID获取活动信息 ==========
QVariantMap DatabaseManager::getActivityById(int activityId) {
    QVariantMap activity; // 存储单条活动信息
    QSqlQuery query;

    // 准备SQL：根据ID查询活动详情
    query.prepare("SELECT * FROM activities WHERE id = ?;");
    query.addBindValue(activityId); // 绑定活动ID，防止SQL注入

    // 执行查询并解析结果
    if (query.exec() && query.next()) {
        activity["id"] = query.value("id").toInt();
        activity["title"] = query.value("title").toString();
        activity["time"] = query.value("time").toString(); // 关键：获取活动时间
        activity["organizer"] = query.value("organizer").toString();
        activity["max_num"] = query.value("max_num").toInt();
        activity["approved"] = query.value("approved").toInt();
        activity["publish_time"] = query.value("publish_time").toString();
    }

    return activity; // 无结果则返回空的QVariantMap
}

// ========== 可选：导出CSV接口实现（适配WorkerThread的ExportCSV任务） ==========
bool DatabaseManager::exportCSV(int activityId, const QString& filePath) {
    // 1. 获取该活动的报名数据
    QList<QVariantMap> applyList = getApplyList(activityId);
    if (applyList.isEmpty()) {
        return false;
    }

    // 2. 写入CSV文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    // 写入CSV表头
    out << "报名ID,活动ID,学生ID,报名时间,报名状态\n";
    // 写入报名数据
    for (const auto& apply : applyList) {
        out << apply["id"].toString() << ","
            << apply["activity_id"].toString() << ","
            << apply["student_id"].toString() << ","
            << apply["apply_time"].toString() << ","
            << apply["status"].toString() << "\n";
    }

    file.close();
    return true;
}
