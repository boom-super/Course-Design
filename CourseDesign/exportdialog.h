#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include "workerthread.h" // 后台线程类

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

private slots:
    void on_btnBrowse_clicked();
    void on_btnExport_clicked(); // 执行导出
    void onTaskFinished(bool success, const QString& msg); // 线程回调

private:
    Ui::ExportDialog *ui;
    int m_activityId;           // 要导出的活动ID
    WorkerThread *m_workerThread; // 后台导出线程
};

#endif // EXPORTDIALOG_H
