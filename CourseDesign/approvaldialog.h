#ifndef APPROVALDIALOG_H
#define APPROVALDIALOG_H

#include <QDialog>
#include <QString>
#include <QListWidgetItem>

namespace Ui {
class approvaldialog;
}

class approvaldialog : public QDialog
{
    Q_OBJECT

public:
    explicit approvaldialog(QWidget *parent = nullptr);
    ~approvaldialog();

private slots:
    void on_btnPass_clicked();

    void on_btnReject_clicked();

    void loadPendingActivities();

    void on_pendingList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
private:
    Ui::approvaldialog *ui;
    int m_selectedActivityId;
};

#endif // APPROVALDIALOG_H
