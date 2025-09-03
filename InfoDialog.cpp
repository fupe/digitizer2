#include <QDebug>
#include <QProcess>
#include <QCoreApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif
#include "InfoDialog.h"
#include "ui_InfoDialog.h"
#include "appmanager.h"

static qint64 getMemoryUsageKB()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024;
    }
    return 0;
#else
    QProcess process;
    process.start("ps", QStringList() << "-o" << "rss=" << "-p"
                   << QString::number(QCoreApplication::applicationPid()));
    if (!process.waitForFinished())
        return 0;
    bool ok = false;
    qint64 rss = process.readAllStandardOutput().trimmed().toLongLong(&ok);
    return ok ? rss : 0;
#endif
}

InfoDialog::InfoDialog(AppManager* app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog),
    app_(app)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    if (app_) {
        connect(app_, &AppManager::modeAddPointChanged,
                this, &InfoDialog::updateModes);
        connect(app_, &AppManager::modeContiChanged,
                this, &InfoDialog::updateModes);
        connect(app_, &AppManager::shapesChanged,
                this, &InfoDialog::updateCounts);
        updateModes();
        updateCounts();
    }
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::set_num_of_points(int number)
{
    QString num_of_point_String = QString::number(number);
    ui->num_points->setText(num_of_point_String);

}

void InfoDialog::closeEvent(QCloseEvent *event)
{
    qDebug()<<"info::closeEvent " << &event;
    this->deleteLater();



}

void InfoDialog::on_close_clicked()
{
    //this->hide();
    qDebug()<<"Info on_close_clicked" ;
    this->close(); // Vyvolá closeEvent automaticky

}

void InfoDialog::updateModes()
{
    if (!app_) return;
    ui->addmode_value->setText(app_->modeAddPointToString(app_->getAddPointMode()));
    ui->contimode_value->setText(app_->modeContiToString(app_->getContiMode()));
}

void InfoDialog::updateCounts()
{
    if (!app_) return;
    ui->num_points->setText(QString::number(app_->pointCount()));
    ui->num_polylines->setText(QString::number(app_->polylineCount()));
    ui->num_circles->setText(QString::number(app_->circleCount()));
    const qint64 memKB = getMemoryUsageKB();
    const double memMB = static_cast<double>(memKB) / 1024.0;
    ui->memory_usage->setText(QString::number(memMB, 'f', 2) + " MB");
}
