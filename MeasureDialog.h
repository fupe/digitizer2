#ifndef MEASUREDIALOG_H
#define MEASUREDIALOG_H

#include <QDialog>
#include <QRect>
#include <QColor>

class QShowEvent;
class QCloseEvent;

class SettingsManager;
struct Settings;

namespace Ui {
class MeasureDialog;
}

class MeasureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MeasureDialog(SettingsManager* sm, QWidget* parent = nullptr);
    ~MeasureDialog();
    int mode;  // 0-nic 1-zacatek mereni 2-zmrazeni
    QPointF start_position;

public slots:
    void set_value(double value);      // units are read from SettingsManager
    void set_color(QColor color);

protected:
    void showEvent(QShowEvent* e) override;
    void closeEvent(QCloseEvent* e) override;

private:
    Ui::MeasureDialog* ui = nullptr;
    SettingsManager* settingsManager_ = nullptr;

    void applySavedGeometry_();
    QRect clampToScreen_(const QRect& r) const;
};

#endif // MEASUREDIALOG_H
