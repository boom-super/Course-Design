#include "databasemanager.h"

bool DatabaseManager::initDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("campus.db");
    if (!db.open()) {
        qDebug() << "数据库打开失败：" << db.lastError().text();
        return false;
    }

    // 创建用户表
    QSqlQuery userQuery;
    userQuery.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            role TEXT NOT NULL CHECK(role IN ('admin', 'organizer', 'student'))
        )
    )");

    // 创建活动表
    QSqlQuery activityQuery;
    activityQuery.exec(R"(
        CREATE TABLE IF NOT EXISTS activities (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            time TEXT NOT NULL,
            organizer TEXT NOT NULL,
            max_num INTEGER NOT NULL,
            approved INTEGER DEFAULT 0,
            FOREIGN KEY(organizer) REFERENCES users(username)
        )
    )");

    // 创建报名表
    QSqlQuery applyQuery;
    applyQuery.exec(R"(
        CREATE TABLE IF NOT EXISTS applies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            activity_id INTEGER NOT NULL,
            student_id TEXT NOT NULL,
            apply_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            status TEXT DEFAULT 'normal' CHECK(status IN ('normal', 'backup')),
            FOREIGN KEY(activity_id) REFERENCES activities(id),
            FOREIGN KEY(student_id) REFERENCES users(username),
            UNIQUE(activity_id, student_id)
        )
    )");

    // 初始化测试数据
    QSqlQuery checkUser;
    checkUser.exec("SELECT COUNT(*) FROM users");
    checkUser.next();
    if (checkUser.value(0).toInt() == 0) {
        QSqlQuery initUser;
        initUser.exec(R"(
            INSERT INTO users (username, password, role) VALUES
            ('admin', 'admin', 'admin'),
            ('org1', 'org1', 'organizer'),
            ('stu1', 'stu1', 'student')
        )");
    }
    return true;
}

QString DatabaseManager::login(const QString& username, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT role FROM users WHERE username=? AND password=?");
    query.addBindValue(username);
    query.addBindValue(password);
    query.exec();
    if (query.next()) {
        return query.value(0).toString();
    }
    return "";
}

bool DatabaseManager::addActivity(const QString& title, const QString& time, const QString& organizer, int maxNum) {
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO activities (title, time, organizer, max_num)
        VALUES (?, ?, ?, ?)
    )");
    query.addBindValue(title);
    query.addBindValue(time);
    query.addBindValue(organizer);
    query.addBindValue(maxNum);
    return query.exec();
}

bool DatabaseManager::approveActivity(int activityId, bool isApproved) {
    QSqlQuery query;
    query.prepare("UPDATE activities SET approved=? WHERE id=?");
    query.addBindValue(isApproved ? 1 : 0);
    query.addBindValue(activityId);
    return query.exec();
}

QList<QVariantMap> DatabaseManager::getActivities(const QString& role, const QString& userId) {
    QList<QVariantMap> list;
    QSqlQuery query;
    if (role == "admin") {
        // 管理员看所有活动
        query.exec("SELECT * FROM activities");
    } else if (role == "organizer") {
        // 发起人看自己发布的活动
        query.prepare("SELECT * FROM activities WHERE organizer=?");
        query.addBindValue(userId);
        query.exec();
    } else if (role == "student") {
        // 学生看已审批的活动
        query.exec("SELECT * FROM activities WHERE approved=1");
    }
    while (query.next()) {
        QVariantMap map;
        map["id"] = query.value("id");
        map["title"] = query.value("title");
        map["time"] = query.value("time");
        map["organizer"] = query.value("organizer");
        map["max_num"] = query.value("max_num");
        map["approved"] = query.value("approved");
        list.append(map);
    }
    return list;
}

QList<QVariantMap> DatabaseManager::getPendingActivities() {
    QList<QVariantMap> list;
    QSqlQuery query;
    query.exec("SELECT * FROM activities WHERE approved=0");
    while (query.next()) {
        QVariantMap map;
        map["id"] = query.value("id");
        map["title"] = query.value("title");
        map["organizer"] = query.value("organizer");
        list.append(map);
    }
    return list;
}

QVariantMap DatabaseManager::getActivityInfo(int activityId) {
    QVariantMap map;
    QSqlQuery query;
    query.prepare("SELECT * FROM activities WHERE id=?");
    query.addBindValue(activityId);
    query.exec();
    if (query.next()) {
        map["time"] = query.value("time");
        map["max_num"] = query.value("max_num");
    }
    return map;
}

bool DatabaseManager::applyActivity(int activityId, const QString& studentId, QString& errMsg) {
    // 检测活动是否存在
    QVariantMap actInfo = getActivityInfo(activityId);
    if (actInfo.isEmpty()) {
        errMsg = "活动不存在！";
        return false;
    }
    // 检测时间冲突
    if (checkTimeConflict(studentId, activityId)) {
        errMsg = "报名时间冲突（同一时段已报名其他活动）！";
        return false;
    }
    // 检测当前报名人数
    QSqlQuery countQuery;
    countQuery.prepare("SELECT COUNT(*) FROM applies WHERE activity_id=? AND status='normal'");
    countQuery.addBindValue(activityId);
    countQuery.exec();
    countQuery.next();
    int currentCount = countQuery.value(0).toInt();
    int maxNum = actInfo["max_num"].toInt();

    // 插入报名记录
    QSqlQuery insertQuery;
    insertQuery.prepare(R"(
        INSERT OR REPLACE INTO applies (activity_id, student_id, status)
        VALUES (?, ?, ?)
    )");
    insertQuery.addBindValue(activityId);
    insertQuery.addBindValue(studentId);
    insertQuery.addBindValue(currentCount >= maxNum ? "backup" : "normal");
    if (!insertQuery.exec()) {
        errMsg = "报名失败：" + insertQuery.lastError().text();
        return false;
    }
    errMsg = currentCount >= maxNum ? "报名成功（已加入候补队列）" : "报名成功！";
    return true;
}

bool DatabaseManager::cancelApply(int activityId, const QString& studentId) {
    QSqlQuery query;
    query.prepare("DELETE FROM applies WHERE activity_id=? AND student_id=?");
    query.addBindValue(activityId);
    query.addBindValue(studentId);
    return query.exec();
}

QList<QVariantMap> DatabaseManager::getApplyList(int activityId) {
    QList<QVariantMap> list;
    QSqlQuery query;
    query.prepare("SELECT * FROM applies WHERE activity_id=?");
    query.addBindValue(activityId);
    query.exec();
    while (query.next()) {
        QVariantMap map;
        map["activity_id"] = query.value("activity_id");
        map["student_id"] = query.value("student_id");
        map["apply_time"] = query.value("apply_time");
        map["status"] = query.value("status");
        list.append(map);
    }
    return list;
}

bool DatabaseManager::checkTimeConflict(const QString& studentId, int activityId) {
    // 获取当前活动时间
    QVariantMap actInfo = getActivityInfo(activityId);
    if (actInfo.isEmpty()) return false;
    QString actTime = actInfo["time"].toString();

    // 检测该学生同一时间是否有其他报名
    QSqlQuery query;
    query.prepare(R"(
        SELECT a.id FROM activities a
        JOIN applies ap ON a.id = ap.activity_id
        WHERE ap.student_id=? AND a.time=? AND ap.status='normal'
    )");
    query.addBindValue(studentId);
    query.addBindValue(actTime);
    query.exec();
    return query.next();
}
