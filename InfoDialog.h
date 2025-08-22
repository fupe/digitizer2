#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = nullptr);
    ~InfoDialog();
    void set_num_of_points(int number);
    void closeEvent(QCloseEvent *event);


private slots:
    void on_close_clicked();

private:
    Ui::InfoDialog *ui;
};

#endif // MYINFO_H
