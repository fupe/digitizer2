#include <QDebug>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif
#include "InfoDialog.h"
#include "ui_InfoDialog.h"
#include "appmanager.h"

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

    memoryTimer_ = new QTimer(this);
    connect(memoryTimer_, &QTimer::timeout, this, &InfoDialog::updateMemory);
    memoryTimer_->start(1000);
    updateMemory();
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

void InfoDialog::updateMemory()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
        ui->memory_value->setText(QString::number(counters.WorkingSetSize / 1024));
    }
#else
    ui->memory_value->setText("N/A");
#endif
}
