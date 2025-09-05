#include <QDebug>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#endif
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include "InfoDialog.h"
#include "ui_InfoDialog.h"
#include "appmanager.h"
#include "mainwindow.h"

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
        connect(app_, &AppManager::serialOpened,
                this, &InfoDialog::onSerialOpened);
        connect(app_, &AppManager::serialClosed,
                this, &InfoDialog::onSerialClosed);
        updateModes();
        updateCounts();
        updateConnectionStatus(app_->isSerialConnected());
    }

    if (auto mw = qobject_cast<MainWindow*>(parent)) {
        connect(mw, &MainWindow::zoomModeChanged,
                this, &InfoDialog::updateZoomMode);
        updateZoomMode(mw->zoomMode());
    }

    connect(&memoryTimer_, &QTimer::timeout,
            this, &InfoDialog::updateMemoryUsage);
    memoryTimer_.start(1000);
    updateMemoryUsage();
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
}

void InfoDialog::updateZoomMode(ZoomMode mode)
{
    ui->zoommode_value->setText(zoomModeToString(mode));
}

QString InfoDialog::zoomModeToString(ZoomMode mode)
{
    switch (mode) {
    case ZoomMode::All:
        return tr("All");
    case ZoomMode::Dynamic:
        return tr("Dynamic");
    case ZoomMode::User:
        return tr("User");
    default:
        return tr("Unknown");
    }
}

void InfoDialog::updateConnectionStatus(bool connected)
{
    ui->connection_value->setText(connected ? tr("Connected")
                                           : tr("Disconnected"));
}

void InfoDialog::updateMemoryUsage()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                              (PROCESS_MEMORY_COUNTERS*)&pmc,
                              sizeof(pmc))) {
        SIZE_T mem = pmc.WorkingSetSize;
        ui->memory_value->setText(QString::number(mem / (1024 * 1024)) + " MB");
    } else {
        ui->memory_value->setText("N/A");
    }
#else
    QFile f("/proc/self/statm");
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        long pages; in >> pages;
        long pageSize = sysconf(_SC_PAGE_SIZE);
        long mem = pages * pageSize;
        ui->memory_value->setText(QString::number(mem / (1024 * 1024)) + " MB");
    } else {
        ui->memory_value->setText("N/A");
    }
#endif
}

void InfoDialog::onSerialOpened()
{
    updateConnectionStatus(true);
}

void InfoDialog::onSerialClosed()
{
    updateConnectionStatus(false);
}
