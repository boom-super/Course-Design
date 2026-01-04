#ifndef APPROVALDIALOG_H
#define APPROVALDIALOG_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class approvaldialog;
}

class approvaldialog : public QWidget
{
    Q_OBJECT

public:
    explicit approvaldialog(QWidget *parent = nullptr);
    ~approvaldialog();

private slots:
    void on_btnPass_clicked();

    void on_btnReject_clicked();

    void loadPendingActivities();

private:
    Ui::approvaldialog *ui;
};

#endif // APPROVALDIALOG_H
