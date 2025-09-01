#ifndef CALIBRATEWINDOW_H
#define CALIBRATEWINDOW_H

#include <QMainWindow>
#include "SettingsDialog.h"
#include "GraphicsItems.h"
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QMessageBox>
#include <calibrationengine.h>
#include "settings.h"
#include <QThread>

class AppManager;  // dopředná deklarace

struct my_set_xy_points {
    QPointF point_xy;
    int index;
    int oposit_index;
    QPointF point_xy_oposit;
    double vzdalenost_to_oposit;
 };

namespace Ui {
class CalibrateWindow;
}

class CalibrateWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit CalibrateWindow(AppManager* appManager, QWidget *parent = nullptr);
    ~CalibrateWindow();
    QPen pen;
    void set_arms(double arm1, double arm2);
    double get_arm1() ;
    double get_arm2() ;
    //Circle circle;
    QGraphicsScene *scene;
    void check_calculate_enable(void);
    QPolygonF *circlepolygon;
    GraphicsItems *currentobject ;
    void closeEvent(QCloseEvent *event);

    void addAngles(double alfa, double beta);
    //QList<Angles> angleslist;
    QVector<Angles> angleslist;


    class my_calibration_set {
    public:
        double arm_1;
        double arm_2;
        my_set_xy_points current_point;
        QList<my_set_xy_points> xy_points_list;
        int alfa_min ;
        int alfa_min_oposit ;
        int beta_min ;
        int beta_min_oposit ;
        int alfa_max ;
        int alfa_max_oposit ;
        int beta_max ;
        int beta_max_oposit ;
        int max_error_index;
        my_calibration_set (double arm_1, double arm_2);
        ~my_calibration_set () ;
        my_calibration_set();  // výchozí konstruktor
        void on_calculate_clicked2(int count);


    };
    my_calibration_set calibration_set;
    void create_new_set(double arm_1, double arm_2);
    static double distance(const QPointF& p1, const QPointF& p2);
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void calculate_arm2();
    void on_clear_clicked();
    void calculate_circle();
    void calculate_clickedstarej(int count);
    void on_actionLoad_calibration_data_triggered();
    void on_actionSave_calibration_data_triggered();
    void on_delete_max_error_point_clicked();
    void on_pushButton_toggled(bool checked);
    void on_pushButton_clicked();
    void on_calculate_clicked();
signals :
    void button_calibrate_clicked(bool checked);
    void close_calibrate(bool status);
protected:

private:
    Ui::CalibrateWindow *ui;
    AppManager* appManager_ = nullptr;
};

#endif // CALIBRATEWINDOW_H
