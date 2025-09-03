#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

class AppManager;

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

private:
    Ui::InfoDialog *ui;
    AppManager* app_ = nullptr;
};

#endif // MYINFO_H
