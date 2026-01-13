#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "workerthread.h"
#include <QMessageBox>
#include <QFileDialog>

ExportDialog::ExportDialog(int activityId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExportDialog),
    m_activityId(activityId),
    m_workerThread(new WorkerThread(this))
{
    ui->setupUi(this);
    connect(m_workerThread, &WorkerThread::taskFinished, this, &ExportDialog::onTaskFinished);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::on_btnBrowse_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "保存CSV文件", "", "CSV文件 (*.csv)");
        if (!filePath.isEmpty()) {
            ui->lineEditPath->setText(filePath);
        }
}

void ExportDialog::on_btnExport_clicked() {
    QString filePath = ui->lineEditPath->text().trimmed();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择保存路径！");
        return;
    }
    // 设置导出任务
    QVariantMap params;
    params["activityId"] = m_activityId;
    params["filePath"] = filePath;
    m_workerThread->setTask(WorkerThread::ExportCSV, params);
    m_workerThread->start();
}

void ExportDialog::onTaskFinished(bool success, const QString& msg) {
    QMessageBox::information(this, success ? "成功" : "失败", msg);
    if (success) this->close();
}
