#include "publishview.h"
#include "ui_publishview.h"
#include "databasemanager.h"
#include <QMessageBox>

publishview::publishview(const QString& organizer, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::publishview),
    m_organizer(organizer)

{
    ui->setupUi(this);
    setWindowTitle("发布新活动");
}

publishview::~publishview()
{
    delete ui;
}

void publishview::on_btnSubmit_clicked() {
    QString title = ui->lineEditTitle->text().trimmed();
    QString time = ui->lineEditTime->text().trimmed();
    int maxNum = ui->spinBoxMax->value();

    if (title.isEmpty() || time.isEmpty()) {
        QMessageBox::warning(this, "提示", "活动标题/时间不能为空！");
        return;
    }
    // 发布活动
    bool ok = DatabaseManager::getInstance().addActivity(title, time, m_organizer, maxNum);
    if (ok) {
        QMessageBox::information(this, "成功", "活动发布成功，等待管理员审批！");
        this->close();
    } else {
        QMessageBox::critical(this, "失败", "活动发布失败！");
    }
}
