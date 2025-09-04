#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

class AppManager;
enum class ZoomMode;

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(AppManager* app, QWidget *parent = nullptr);
    ~InfoDialog();
    void set_num_of_points(int number);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_close_clicked();
    void updateModes();
    void updateCounts();
    void updateZoomMode(ZoomMode mode);
    void updateConnectionStatus(bool connected);
    void onSerialOpened();
    void onSerialClosed();

private:
    static QString zoomModeToString(ZoomMode mode);
    Ui::InfoDialog *ui;
    AppManager* app_ = nullptr;
};

#endif // MYINFO_H
