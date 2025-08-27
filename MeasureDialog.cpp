#include "MeasureDialog.h"
#include "settingsmanager.h"
#include "settings.h"
#include "ui_MeasureDialog.h"

#include <QShowEvent>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QSignalBlocker>
#include <QAction>
#include <QScreen>
#include <QLCDNumber>
#include <QPalette>
#include <QDebug>
#include <cmath>

MeasureDialog::MeasureDialog(SettingsManager* sm, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MeasureDialog)
    , settingsManager_(sm)
{
    ui->setupUi(this);
    Q_ASSERT(settingsManager_);
}

MeasureDialog::~MeasureDialog()
{
    delete ui;
}

void MeasureDialog::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);
    applySavedGeometry_();

}

void MeasureDialog::closeEvent(QCloseEvent* e)
{
    if (settingsManager_) {
        auto s = settingsManager_->currentSettings();
        if (s.save_measure_window_position_on_exit && !isMaximized() && !isFullScreen()) {
            s.measure_window_position = this->geometry();
            settingsManager_->updateSettings(s);
            qDebug() << "[MeasureDialog] saved geometry:" << s.measure_window_position;
        }
    }
    QDialog::closeEvent(e);
}

void MeasureDialog::applySavedGeometry_()
{
    if (!settingsManager_) return;

    const auto& s = settingsManager_->currentSettings();
    if (!s.save_measure_window_position_on_exit) return;
    if (!s.measure_window_position.isValid())    return;
    if (isMaximized() || isFullScreen())         return;

    const QRect clamped = clampToScreen_(s.measure_window_position);
    setGeometry(clamped);
    qDebug() << "[MeasureDialog] restored geometry:" << clamped;

}

QRect MeasureDialog::clampToScreen_(const QRect& r) const
{
    const auto screens = QGuiApplication::screens();
    for (auto* scr : screens) {
        if (scr && scr->availableGeometry().intersects(r)) {
            return r;
        }
    }

    const QScreen* primary = QGuiApplication::primaryScreen();
    const QRect ag = primary ? primary->availableGeometry()
                             : QRect(0, 0, 1280, 800);

    QSize size = r.size();
    if (size.width() <= 0 || size.height() <= 0) {
        size = QSize(800, 600);
    }

    QRect out(QPoint(0,0), size);
    out.moveCenter(ag.center());
    return out;
}

void MeasureDialog::set_value(double value)
{
    // get units from settings
    int decimals = 2; // default
    QString unitText = QStringLiteral("mm");
    if (settingsManager_) {
        const auto& s = settingsManager_->currentSettings();
        if (s.units == Units::Millimeters) {
            decimals = 1;
        } else {
            decimals = 2;
        }
        unitText = unitsToString(s.units);
    }
    ui->lcdNumber->display(QString::number(value, 'f', decimals));
    ui->unitLabel->setText(unitText);
}

void MeasureDialog::set_color(QColor color)
{
    auto palette = ui->lcdNumber->palette();
    palette.setColor(QPalette::WindowText, color);
    ui->lcdNumber->setPalette(palette);
}
