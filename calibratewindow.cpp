#include "calibratewindow.h"
#include "ui_calibratewindow.h"
#include "calibrationengine.h"
#include <QApplication>
#include <QFileDialog>     //save calib
//#include <windows.h>
//#include <psapi.h>
//#include <QThread>

#include <QDebug>
#include "appmanager.h"

CalibrateWindow::CalibrateWindow(AppManager* appManager, QWidget *parent)
    : QMainWindow(parent),
      calibration_set(0, 0),
      ui(new Ui::CalibrateWindow),
      appManager_(appManager)
{
    ui->setupUi(this);
    //ui->calculate  ->setEnabled(false);
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-100, -100, 200, 200);
    //ui->graphicsView->setScene(scene);
    //ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    //pen.setColor(Qt::blue);
    //pen.setWidth(5);
    //scene->addEllipse(-25,-25, 50, 50,pen);
    //ui->graphicsView->setRenderHints(QPainter::Antialiasing);
    //ui->graphicsView->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    //ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    if (appManager_) {
        connect(appManager_, &AppManager::calibrateAnglesAdded,
                this, &CalibrateWindow::addAngles, Qt::UniqueConnection);
    }
}

CalibrateWindow::~CalibrateWindow()
{
    delete ui;
}



void CalibrateWindow::set_arms(double arm1, double arm2)
{
    ui->doubleSpinBox_arm1->setValue(arm1);
    ui->doubleSpinBox_arm2->setValue(arm2);
    ui->doubleSpinBox_arm1_2->setValue(arm1);
    ui->doubleSpinBox_arm2_2->setValue(arm2);
    ui->doubleSpinBox_arm1_3->setValue(arm1);
    ui->doubleSpinBox_arm2_3->setValue(arm2);
    ui->doubleSpinBox_krok->setValue(10.00);
    ui->doubleSpinBox_prumer->setValue(500);
    ui->label_points->setText("-");
    //ui->label_max_chyba_2->setText("-");
    ui->label_max_chyba->setText("-");


}

double CalibrateWindow::get_arm1()
{
    return ui->doubleSpinBox_arm1->value();

}
double CalibrateWindow::get_arm2()
{
    return ui->doubleSpinBox_arm2->value();

}

void CalibrateWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"zavreno krizkem" << event;

    emit close_calibrate(false);
    deleteLater();
}




void CalibrateWindow::addAngles(double alfa, double beta)
{
    qDebug() << "addAngles alfa=" << alfa << "  Beta:" <<beta;
    Angles angles(alfa, beta);
    angleslist.append(angles);
    //QString countString = QString::number(angleslist.count());
    ui->label_points->setText(QString::number(angleslist.count()));
    check_calculate_enable();

}

void CalibrateWindow::create_new_set(double arm_1, double arm_2)
{
    double x_value,y_value; /// pomocne promene
    double x_value2,y_value2;
    double x_value_oposit,y_value_oposit;
    double x_value2_oposit,y_value2_oposit;
    calibration_set.xy_points_list.clear();

    foreach (const Angles& angle, angleslist)
    {
      //  qDebug() << "Alfa add :" << angle.alfa << "Beta add :" << angle.beta << "arm1 " << arm_1 << "arm2 "<<arm_2;
        x_value = cos(double ( angle.alfa/180*M_PI))*arm_1;
        y_value = sin(double ( angle.alfa/180*M_PI))*arm_1;

      //  qDebug() << "x=" << x_value << " y=" << y_value;
        //calibration_set.current_point.point_xy.setX(25);
        calibration_set.current_point.point_xy.setX(x_value + cos(double ((( angle.alfa/180*M_PI)-(angle.beta/180*M_PI)-M_PI)))*arm_2);
        calibration_set.current_point.point_xy.setY(y_value + sin(double ((( angle.alfa/180*M_PI)-(angle.beta/180*M_PI)-M_PI)))*arm_2);
        //qDebug() << "x2=" << calibration_set.current_point.point_xy.x() << " y2=" << calibration_set.current_point.point_xy.y();
        calibration_set.xy_points_list.append(calibration_set.current_point);
        //qDebug() << "size point" << calibration_set.xy_points_list.size();

//        currentobject = new mypoint;
        //qDebug() << "add point:";
//        currentobject->setPos(calibration_set.current_point.point_xy);
//        scene->addItem(currentobject);
    }
    //qDebug() << "final size point" << calibration_set.xy_points_list.size();

    foreach (const my_set_xy_points& points, calibration_set.xy_points_list)
    {
        Q_UNUSED(points)
        //qDebug() << " point " << points.point_xy;
    }

    // Projdeme všechny body a hledáme nejvzdálenější bod
    for (int i = 0; i < calibration_set.xy_points_list.size(); ++i) {

        QPointF currentPoint = calibration_set.xy_points_list.at(i).point_xy;
        qreal maxDistance = 0.0;
        QPointF farthestPoint;
        int index = 0;
        for (int j = 0; j < calibration_set.xy_points_list.size(); ++j) {
            if (i != j) {
                qreal d = distance(currentPoint, calibration_set.xy_points_list.at(j).point_xy);
                if (d > maxDistance) {
                    maxDistance = d;
                    farthestPoint = calibration_set.xy_points_list.at(j).point_xy;
                    index= j;
                }
            }
        }

    // Výstup nejvzdálenějšího bodu pro aktuální bod
    //qDebug() << "Nejvzdálenější bod pro bod:" << i << "  " << currentPoint << "je" << farthestPoint << "index" << index << "vzdalenost " << maxDistance;

    calibration_set.xy_points_list[i].point_xy_oposit.setX(farthestPoint.x());
    calibration_set.xy_points_list[i].point_xy_oposit.setY(farthestPoint.y());
    calibration_set.xy_points_list[i].index=i;
    calibration_set.xy_points_list[i].oposit_index=index;
    calibration_set.xy_points_list[i].vzdalenost_to_oposit=maxDistance;
    //qDebug() << "distance=" << points->vzdalenost.at(i) ;
    }
    /*foreach (const my_set_xy_points& points, calibration_set.xy_points_list)
    {
        qDebug() << " point " << points.point_xy  << " oposit " << points.point_xy_oposit  << "index " << points.oposit_index << "dist:"  << points.vzdalenost_to_oposit ;
    }*/
    //------------------------------
    qreal temp_max_distance = 0.0;
    qreal temp_min_distance = 1000000.0;
    //qDebug() << "Alfa max min :" << "angleslist.size  " << angleslist.size();
    //foreach (const Angles& angle, CalibrateWindow::angleslist)
    //for (int i = 0; i < CalibrateWindow::angleslist.size(); ++i)
    //qDebug() << "xy_points_list.size :"  << calibration_set.xy_points_list.size();
    for (int i = 0; i < (calibration_set.xy_points_list.size()); ++i)

    {
            x_value = cos(double ( angleslist.at(i).alfa/180*M_PI))*arm_1*(1+100)/100;
            y_value = sin(double ( angleslist.at(i).alfa/180*M_PI))*arm_1*(1+100)/100;

            x_value_oposit = cos(double ( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI))*arm_1*(1+100)/100;
            y_value_oposit = sin(double ( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI))*arm_1*(1+100)/100;

            x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.xy_points_list.at(i).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).index).beta/180*M_PI)-M_PI)))*arm_2;
            y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.xy_points_list.at(i).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).index).beta/180*M_PI)-M_PI)))*arm_2;

            x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).beta/180*M_PI)-M_PI)))*arm_2;
            y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).beta/180*M_PI)-M_PI)))*arm_2;

            if ( abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit)>temp_max_distance)
            {
                temp_max_distance = abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit);
                calibration_set.alfa_max = i;
                qDebug () << " new max dist " << temp_max_distance << " index " << i;
            }
            if ( abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit)<temp_min_distance)
            {
                temp_min_distance = abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit);
                calibration_set.alfa_min = i;
                qDebug () << " new min dist " << temp_min_distance << " index " << i;
            }
            //qDebug() << "iii="<< i<< "alfa="<<angleslist.at(i).alfa << "  diff="<<abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit);
    }
    //xy_poitlist.append(*points);
    qDebug() << "alfa max=" << calibration_set.alfa_max << "hodnota " << temp_max_distance;
    qDebug() << "alfa min=" << calibration_set.alfa_min << "hodnota " << temp_min_distance;
    calibration_set.alfa_max_oposit=calibration_set.xy_points_list[calibration_set.alfa_max].oposit_index;
    calibration_set.alfa_min_oposit=calibration_set.xy_points_list[calibration_set.alfa_min].oposit_index;
    qDebug() << "alfa max_oposit=" << calibration_set.alfa_max_oposit;
    //calibration_set.xy_points_list[i].oposit_index

    //-----------------------------
    //qDebug() << "xy_points_list.size :"  << calibration_set.xy_points_list.size();
    temp_max_distance = 0.0;
    temp_min_distance = 1000000.0;
    for (int i = 0; i < calibration_set.xy_points_list.size(); ++i)
   {
           x_value = cos(double ( angleslist.at(i).alfa/180*M_PI))*arm_1;
           y_value = sin(double ( angleslist.at(i).alfa/180*M_PI))*arm_1;

           x_value_oposit = cos(double ( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI))*arm_1;
           y_value_oposit = sin(double ( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI))*arm_1;

           x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.xy_points_list.at(i).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).index).beta/180*M_PI)-M_PI)))*arm_2*(1+100)/100;
           y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.xy_points_list.at(i).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).index).beta/180*M_PI)-M_PI)))*arm_2*(1+100)/100;

           x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).beta/180*M_PI)-M_PI)))*arm_2*(1+100)/100;
           y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(i).oposit_index).beta/180*M_PI)-M_PI)))*arm_2*(1+100)/100;

           if ( abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit)>temp_max_distance)
           {
               temp_max_distance = abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit);
               calibration_set.beta_max = i;
           }
           if ( abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit)<temp_min_distance)
           {
               temp_min_distance = abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit);
               calibration_set.beta_min = i;
           }
   }
   //xy_poitlist.append(*points);
   qDebug() << "beta max=" << calibration_set.beta_max << "hodnota " << temp_max_distance;
   qDebug() << "beta min=" << calibration_set.beta_min << "hodnota " << temp_min_distance;
   calibration_set.beta_max_oposit=calibration_set.xy_points_list[calibration_set.beta_max].oposit_index;
   calibration_set.beta_min_oposit=calibration_set.xy_points_list[calibration_set.beta_min].oposit_index;
   qDebug() << "beta max_oposit=" << calibration_set.beta_max_oposit;


   //-----------------------------
}



CalibrateWindow::my_calibration_set::my_calibration_set()
{
    arm_1 = 0;
    arm_2 = 0;
    // případně vymaž seznam, nastav výchozí hodnoty atd.
}

CalibrateWindow::my_calibration_set::my_calibration_set(double arm_1, double arm_2)
{
    this->arm_1=arm_1;
    this->arm_2=arm_2;
}

CalibrateWindow::my_calibration_set::~my_calibration_set()
{
 xy_points_list.clear();
}

double CalibrateWindow::distance(const QPointF &p1, const QPointF &p2)
{

        qreal dx = p1.x() - p2.x();
        qreal dy = p1.y() - p2.y();
        return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));
}



void CalibrateWindow::on_buttonBox_accepted()
{
    qDebug() << "accepted from calib" ;
    //Settings settings = appManager->settingsManager()->currentSettings(); // kopie

    foreach (QGraphicsItem* item , scene->items())
    {
    GraphicsItems *fm = dynamic_cast<GraphicsItems*>(item);
    if (fm)
    {
        fm->scene()->removeItem(fm);
    }
    }
    //-----------
    QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, "Save arms", "prepsat nastaveni arms v settings? ",
                                    QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes) {
        qDebug() << "Yes was clicked";
        //setting.arm1_length=ui->doubleSpinBox_arm1->value();
        //setting->arm2_length=ui->doubleSpinBox_arm2->value();
        emit close_calibrate(true);


      } else {
        qDebug() << "Yes was *not* clicked";
        emit close_calibrate(false);
      }
    //-----------
    // close();
deleteLater();



}


void CalibrateWindow::on_buttonBox_rejected()
{
    qDebug() << "decline calib ";
    emit close_calibrate(false);
//    close();
    deleteLater();
}




void CalibrateWindow::calculate_arm2()
{


    double temp_arm2=ui->doubleSpinBox_arm2->value()-5*ui->doubleSpinBox_krok->value();
    double minimum=10000;
    double min_arm2=0;
    double x_value,y_value,x_value_oposit,y_value_oposit,x_value2,y_value2,x_value2_oposit,y_value2_oposit;
    double dist1,dist2,dist3;
    //qDebug()  << "angle list at alfa " << angleslist.at(calibration_set.alfa_max).alfa;
    //qDebug()  << "angle list at beta " << angleslist.at(calibration_set.alfa_max).beta;
    //qDebug() << "calibration_set.alfa_max" << calibration_set.xy_points_list.at(calibration_set.alfa_max).point_xy ;
    //qDebug() << "index" << calibration_set.xy_points_list.at(calibration_set.alfa_max).index;
    //qDebug() << "oposit index" << calibration_set.xy_points_list.at(calibration_set.alfa_max).oposit_index;
    //qDebug()  << "angle list at alfa oposit" << angleslist.at(calibration_set.alfa_max_oposit).alfa;
    //qDebug()  << "angle list at beta oposit" << angleslist.at(calibration_set.alfa_max_oposit).beta;

    for (int i=0;i<=10;i++)

    {
        //qDebug() << "i" <<i << "arm2 upravena" << temp_arm2+i*ui->doubleSpinBox_krok->value();
        //qDebug() << "calibration_set.alfa_max" << calibration_set.alfa_max;
        //qDebug() << "calibration_set.alfa_max_oposit" << calibration_set.alfa_max_oposit;
        //qDebug() << "alfa" << angleslist.at(calibration_set.alfa_max).alfa;

        //--------------------------
        x_value = cos(double ( angleslist.at(calibration_set.alfa_min).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        y_value = sin(double ( angleslist.at(calibration_set.alfa_min).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        x_value_oposit = cos(double ( angleslist.at(calibration_set.alfa_min_oposit).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        y_value_oposit = sin(double ( angleslist.at(calibration_set.alfa_min_oposit).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.alfa_min).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.alfa_min).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.alfa_min_oposit).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min_oposit).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.alfa_min_oposit).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min_oposit).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        //qDebug() << "x_value2" << x_value2 << "y_value2" << y_value2;
        //qDebug() << "x_value2_oposit" << x_value2_oposit << "y_value2_oposit" << y_value2_oposit;
        //qDebug() << "vzdalenost1 beta =" << distance(QPointF(x_value2,y_value2),QPointF(x_value2_oposit,y_value2_oposit));
        dist1=distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2));

        x_value = cos(double ( angleslist.at(calibration_set.alfa_min-1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        y_value = sin(double ( angleslist.at(calibration_set.alfa_min-1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        x_value_oposit = cos(double ( angleslist.at(calibration_set.alfa_min_oposit-1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        y_value_oposit = sin(double ( angleslist.at(calibration_set.alfa_min_oposit-1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.alfa_min-1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min-1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.alfa_min-1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min-1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.alfa_min_oposit-1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min_oposit-1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.alfa_min_oposit-1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min_oposit-1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        //qDebug() << "x_value2" << x_value2 << "y_value2" << y_value2;
        //qDebug() << "x_value2_oposit" << x_value2_oposit << "y_value2_oposit" << y_value2_oposit;
        //qDebug() << "vzdalenost2 beta  =" << distance(QPointF(x_value2,y_value2),QPointF(x_value2_oposit,y_value2_oposit));
        dist2=distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2));


        x_value = cos(double ( angleslist.at(calibration_set.alfa_min+1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        y_value = sin(double ( angleslist.at(calibration_set.alfa_min+1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        x_value_oposit = cos(double ( angleslist.at(calibration_set.alfa_min_oposit+1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        y_value_oposit = sin(double ( angleslist.at(calibration_set.alfa_min_oposit+1).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
        x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.alfa_min+1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min+1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.alfa_min+1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min+1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.alfa_min_oposit+1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min_oposit+1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.alfa_min_oposit+1).alfa/180*M_PI)-angleslist.at(calibration_set.alfa_min_oposit+1).beta/180*M_PI)-M_PI))*(temp_arm2+i*ui->doubleSpinBox_krok->value());
        //qDebug() << "x_value2" << x_value2 << "y_value2" << y_value2;
        //qDebug() << "x_value2_oposit" << x_value2_oposit << "y_value2_oposit" << y_value2_oposit;
        //qDebug() << "vzdalenost3 beta  =" << distance(QPointF(x_value2,y_value2),QPointF(x_value2_oposit,y_value2_oposit));
        dist3=distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2));


        //qDebug() << "prumerna hodnota  pro beta= "  << (dist1+dist2+dist3)/3;
        //qDebug() << "jedina   hodnota  pro beta= "  << dist1;

        if (abs(dist1-ui->doubleSpinBox_prumer->value())<minimum) // vypocet pro jednu hodnotu
        //if (abs((dist1+dist2+dist3)/3-ui->doubleSpinBox_prumer->value())<minimum)   //vzpocet pro prumer 3
        {
            // minimum = abs((dist1+dist2+dist3)/3-ui->doubleSpinBox_prumer->value()); /vzpocet pro prumer 3
            minimum = abs(dist1-ui->doubleSpinBox_prumer->value());
            //qDebug() << "minimum =" << minimum;
            min_arm2 = temp_arm2+i*ui->doubleSpinBox_krok->value();
            //qDebug() << "min_arm1 prumer "  << temp_arm2+i*ui->doubleSpinBox_krok->value();
        }

        /*if ( abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-ui->doubleSpinBox_prumer->value())<minimum)
        {
            minimum = abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-ui->doubleSpinBox_prumer->value());
            qDebug() << "minimum =" << minimum;
            min_arm2 = temp_arm2+i*ui->doubleSpinBox_krok->value();
            qDebug() << "min_arm2 "  << temp_arm2+i*ui->doubleSpinBox_krok->value();
        }*/
        //{
            //temp_max_distance = abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-calibration_set.xy_points_list.at(i).vzdalenost_to_oposit);
            //calibration_set.beta_max = i;
        //}
        //qDebug() << " vzdalenost" << abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2)));
        //-------------------------


    }
    qDebug() << "best arm2 " << min_arm2  << " minimum = " << minimum;
    ui->doubleSpinBox_arm2->setValue(min_arm2) ;
    ui->doubleSpinBox_krok->setValue(ui->doubleSpinBox_krok->value()*0.9);


    //----------------------------
    double min=100000, max=0, prumer=0;
    for (int i=0; i<calibration_set.xy_points_list.count(); i++)
    {
     //qDebug() << " bod i"   << i << " vzdalenost bodu od protejsiho  "  << abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)) ;
    if (abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value())>max)
       {
          max=abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value()) ;
          //qDebug() << "  novy max"
        }
    if (abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value())<min)
       {
          min=abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value()) ;
       }
    prumer=prumer+abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value());
    }
    qDebug() << "min error protejsi = " << min << "max error =" << max << "prumerna chyba click 4" << prumer/calibration_set.xy_points_list.count() ;

    QString errString = QString::number(max);
    ui->label_max_chyba->setText(errString+" mm");
    calculate_circle();

}


void CalibrateWindow::calculate_circle()
{
    if (calibration_set.xy_points_list.count() > 4)
  {
   // Circle circle;
    //qDebug() << " pocet prvku"  << calibration_set.xy_points_list.count();
    //circlepolygon->append()
    /*for (int i = 0; i < calibration_set.xy_points_list.size(); ++i)
    {
        qDebug()<< "points " << calibration_set.xy_points_list.at(i).point_xy;
        circlepolygon->append(calibration_set.xy_points_list.at(i).point_xy);
    }*/

        /*CircleData data(calibration_set.xy_points_list.count());

    for (int i=0; i<calibration_set.xy_points_list.count(); i++)
    {
       data.X[i]=calibration_set.xy_points_list.at(i).point_xy.x();
       data.Y[i]=calibration_set.xy_points_list.at(i).point_xy.y();
       //qDebug () << "X=" << data.X[i] ;
       //qDebug () << "Y=" << data.Y[i] ;

    }
    circle = CircleFitByPratt (data);  //default funguje
    //circle = CircleFitByKasa (data);  //stejny vysledek jako Pratt
    //circle = CircleFitByTaubin (data); //stejny vysledek jako Pratt
    //circle = CircleFitByHyper (data);




    //qDebug () << "kruznice r=" <<circle.r << "stred x="  << circle.a  << " y=" << circle.b ;
    double min=100000, max=0, prumer=0;
    //int index;

    for (int i=0; i<calibration_set.xy_points_list.count(); i++)
    {
     //qDebug() << " bod i"   << i << " vzdalenost bodu od kruznice  "  << distance(calibration_set.xy_points_list.at(i).point_xy,QPoint(circle.a,circle.b))-ui->doubleSpinBox_prumer->value()/2 ;
     if (abs(distance(calibration_set.xy_points_list.at(i).point_xy,QPoint(circle.a,circle.b))-ui->doubleSpinBox_prumer->value()/2)>max)
        {
           max=abs(distance(calibration_set.xy_points_list.at(i).point_xy,QPoint(circle.a,circle.b))-ui->doubleSpinBox_prumer->value()/2) ;
           calibration_set.max_error_index = i;
           //qDebug() <<"novy nejvzdalenejsi bod" << calibration_set.max_error_index;
        }
     if (abs(distance(calibration_set.xy_points_list.at(i).point_xy,QPoint(circle.a,circle.b))-ui->doubleSpinBox_prumer->value()/2)<min)
        {
           min=abs(distance(calibration_set.xy_points_list.at(i).point_xy,QPoint(circle.a,circle.b))-ui->doubleSpinBox_prumer->value()/2) ;
        }

    }
    qDebug() <<"nejvzdalenejsi bod" << calibration_set.max_error_index;
    QString circString = QString::number(max);
   // ui->label_max_chyba_2->setText(circString+ " mm");

    qDebug() << "min error od kruznice = " << min << "max error =" << max ;
    min=100000, max=0 ;
    for (int i=0; i<calibration_set.xy_points_list.count(); i++)
    {
     // --mk-- qDebug() << " bod i"   << i << " vzdalenost bodu od protejsiho  "  << distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit) ;
    if (abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value())>max)
       {
          max=abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value()) ;

    }
    if (abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value())<min)
       {
          min=abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value()) ;
       }
    prumer=prumer+abs(distance(calibration_set.xy_points_list.at(i).point_xy,calibration_set.xy_points_list.at(i).point_xy_oposit)-ui->doubleSpinBox_prumer->value());
    }
    qDebug() << "max error =" << max << "index "<< calibration_set.max_error_index << "max chyba " << max ; //prumer/calibration_set.xy_points_list.count() ;
    QString indexString = QString::number(calibration_set.max_error_index);
    ui->max_err_index->setText(indexString);
//    currentobject = new mycircle;
    //currentobject->setPos(QPoint(0,0));
    for (int i=0; i<calibration_set.xy_points_list.count(); i++)
    {

//    currentobject->mypolygon->append(calibration_set.xy_points_list.at(i).point_xy);
    //qDebug () << " poly "  <<calibration_set.xy_points_list.at(i).point_xy ;
    //qDebug() << "circle " ;
    }

    qDebug() << "kružnice" ;
//    currentobject->finished=true;
//    currentobject->pen.setColor(Qt::blue) ;
//    scene->addItem(currentobject) ;

    //GraphicsItems *newitem_calib = new mypoint;
//    newitem_calib->pen.setColor(Qt::black) ;
    double x_value,y_value,x_value2,y_value2;
    x_value = cos(double ( angleslist.at(calibration_set.alfa_max).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
    y_value = sin(double ( angleslist.at(calibration_set.alfa_max).alfa/180*M_PI))*ui->doubleSpinBox_arm2->value();
    x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.xy_points_list.at(calibration_set.alfa_max).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(calibration_set.alfa_max).index).beta/180*M_PI)-M_PI)))*ui->doubleSpinBox_arm2->value();
    y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.xy_points_list.at(calibration_set.alfa_max).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(calibration_set.alfa_max).index).beta/180*M_PI)-M_PI)))*ui->doubleSpinBox_arm2->value();
    x_value2 = x_value2+(x_value2 - circle.a)*0.15 ;
    y_value2 = y_value2+(y_value2 - circle.b)*0.15 ;
//    newitem_calib->setPos(QPoint(x_value2,y_value2));
//    scene->addItem(newitem_calib);

//    newitem_calib = new mypoint;
//    newitem_calib->pen.setColor(Qt::black) ;

    x_value = cos(double ( angleslist.at(calibration_set.beta_max).alfa/180*M_PI))*ui->doubleSpinBox_arm1->value();
    y_value = sin(double ( angleslist.at(calibration_set.beta_max).alfa/180*M_PI))*ui->doubleSpinBox_arm2->value();
    x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.xy_points_list.at(calibration_set.beta_max).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(calibration_set.beta_max).index).beta/180*M_PI)-M_PI)))*ui->doubleSpinBox_arm2->value();
    y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.xy_points_list.at(calibration_set.beta_max).index).alfa/180*M_PI)-(angleslist.at(calibration_set.xy_points_list.at(calibration_set.beta_max).index).beta/180*M_PI)-M_PI)))*ui->doubleSpinBox_arm2->value();
    x_value2 = x_value2+(x_value2 - circle.a)*0.15 ;
    y_value2 = y_value2+(y_value2 - circle.b)*0.15 ;
//    newitem_calib->setPos(QPoint(x_value2,y_value2));
//    scene->addItem(newitem_calib);

*/
  }
  else
    {
        qDebug() << "malo bodu na kruznici" ;
    }

}


void CalibrateWindow::on_clear_clicked()
{



  angleslist.clear();
  QString anglesString = QString::number(angleslist.count());
  ui->label_points->setText(anglesString);
  ui->doubleSpinBox_krok->setValue(10);
  foreach (QGraphicsItem* item , scene->items())
  {
  GraphicsItems *fm = dynamic_cast<GraphicsItems*>(item);
  if (fm)
  {
      fm->scene()->removeItem(fm);
  }
  }


  //ui->label_max_chyba_2->setText("-");
  ui->label_max_chyba->setText("-");
    check_calculate_enable();
}


void CalibrateWindow::calculate_clickedstarej(int count)   //calculate arm1
{

    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (angleslist.count()>100)  //alespon 100 bodu
    {

        for (int cyklus=0;cyklus<count;cyklus++)   //pocet iteraci
            {
            create_new_set(ui->doubleSpinBox_arm1->value(),ui->doubleSpinBox_arm2->value());

                double temp_arm1=ui->doubleSpinBox_arm1->value()-5*ui->doubleSpinBox_krok->value();
                qDebug()<<"arm1_temp1="<<temp_arm1;
                double minimum=10000;
                double min_arm1=0;
                double x_value,y_value,x_value_oposit,y_value_oposit,x_value2,y_value2,x_value2_oposit,y_value2_oposit;
                double dist1 ,dist2,dist3;
    //qDebug()  << "angle list at alfa " << angleslist.at(calibration_set.alfa_max).alfa;
    //qDebug()  << "angle list at beta " << angleslist.at(calibration_set.alfa_max).beta;
    //qDebug() << "calibration_set.alfa_max" << calibration_set.xy_points_list.at(calibration_set.alfa_max).point_xy ;
    //qDebug() << "index" << calibration_set.xy_points_list.at(calibration_set.alfa_max).index;
    //qDebug() << "oposit index" << calibration_set.xy_points_list.at(calibration_set.alfa_max).oposit_index;
    //qDebug()  << "angle list at alfa oposit" << angleslist.at(calibration_set.alfa_max_oposit).alfa;
    //qDebug()  << "angle list at beta oposit" << angleslist.at(calibration_set.alfa_max_oposit).beta;

    for (int i=0;i<=10;i++)

    {
        //qDebug() <<i << "arm1 upravena" << temp_arm1+i*ui->doubleSpinBox_krok->value();
        //qDebug() << "calibration_set.alfa_max" << calibration_set.alfa_max;
        //qDebug() << "calibration_set.alfa_max_oposit" << calibration_set.alfa_max_oposit;
        //qDebug() << "alfa" << angleslist.at(calibration_set.alfa_max).alfa;

        //--------------------------
        x_value = cos(double ( angleslist.at(calibration_set.beta_min).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        y_value = sin(double ( angleslist.at(calibration_set.beta_min).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        x_value_oposit = cos(double ( angleslist.at(calibration_set.beta_min_oposit).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        y_value_oposit = sin(double ( angleslist.at(calibration_set.beta_min_oposit).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.beta_min).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.beta_min).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.beta_min_oposit).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min_oposit).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.beta_min_oposit).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min_oposit).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        //qDebug() << "x_value2" << x_value2 << "y_value2" << y_value2;
        //qDebug() << "x_value2_oposit" << x_value2_oposit << "y_value2_oposit" << y_value2_oposit;
//        qDebug() << "vzdalenost alfa =" << distance(QPointF(x_value2,y_value2),QPointF(x_value2_oposit,y_value2_oposit));
        dist1=distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2));

        x_value = cos(double ( angleslist.at(calibration_set.beta_min-1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        y_value = sin(double ( angleslist.at(calibration_set.beta_min-1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        x_value_oposit = cos(double ( angleslist.at(calibration_set.beta_min_oposit-1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        y_value_oposit = sin(double ( angleslist.at(calibration_set.beta_min_oposit-1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.beta_min-1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min-1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.beta_min-1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min-1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.beta_min_oposit-1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min_oposit-1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.beta_min_oposit-1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min_oposit-1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        //qDebug() << "x_value2-1" << x_value2 << "y_value2" << y_value2;
        //qDebug() << "x_value2_oposit-1" << x_value2_oposit << "y_value2_oposit" << y_value2_oposit;
//        qDebug() << "vzdalenost 2 alfa=" << distance(QPointF(x_value2,y_value2),QPointF(x_value2_oposit,y_value2_oposit));
        dist2=distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2));


        x_value = cos(double ( angleslist.at(calibration_set.beta_min+1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        y_value = sin(double ( angleslist.at(calibration_set.beta_min+1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        x_value_oposit = cos(double ( angleslist.at(calibration_set.beta_min_oposit+1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        y_value_oposit = sin(double ( angleslist.at(calibration_set.beta_min_oposit+1).alfa/180*M_PI))*(temp_arm1+i*ui->doubleSpinBox_krok->value());
        x_value2  = x_value + cos(double ((( angleslist.at(calibration_set.beta_min+1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min+1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        y_value2  = y_value + sin(double ((( angleslist.at(calibration_set.beta_min+1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min+1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        x_value2_oposit = x_value_oposit + cos(double ((( angleslist.at(calibration_set.beta_min_oposit+1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min_oposit+1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        y_value2_oposit = y_value_oposit + sin(double ((( angleslist.at(calibration_set.beta_min_oposit+1).alfa/180*M_PI)-angleslist.at(calibration_set.beta_min_oposit+1).beta/180*M_PI)-M_PI))*ui->doubleSpinBox_arm2->value();
        //qDebug() << "x_value2+1" << x_value2 << "y_value2" << y_value2;
        //qDebug() << "x_value2_oposit+1" << x_value2_oposit << "y_value2_oposit" << y_value2_oposit;
//        qDebug() << "vzdalenost3 alfa =" << distance(QPointF(x_value2,y_value2),QPointF(x_value2_oposit,y_value2_oposit));
        dist3=distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2));

//        qDebug() << "prumerna hodnota = "  <<(dist1+dist2+dist3)/3;


       // if (abs((dist1+dist2+dist3)/3-ui->doubleSpinBox_prumer->value())<minimum)
        if (abs(dist1-ui->doubleSpinBox_prumer->value())<minimum)
        {
            //minimum = abs((dist1+dist2+dist3)/3-ui->doubleSpinBox_prumer->value());
            minimum = abs(dist1-ui->doubleSpinBox_prumer->value());
            //qDebug() << "minimum =" << minimum;
            min_arm1 = temp_arm1+i*ui->doubleSpinBox_krok->value();
            //qDebug() << "min_arm1 prumer "  << temp_arm1+i*ui->doubleSpinBox_krok->value();
        }


      /*  if ( abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-ui->doubleSpinBox_prumer->value())<minimum)
        {
            minimum = abs (distance (QPointF(x_value2_oposit,y_value2_oposit),QPointF(x_value2,y_value2))-ui->doubleSpinBox_prumer->value());
            qDebug() << "minimum =" << minimum;
            min_arm1 = temp_arm1+i*ui->doubleSpinBox_krok->value();
            qDebug() << "min_arm1 "  << temp_arm1+i*ui->doubleSpinBox_krok->value();
        }*/
    }
    qDebug() << "best arm1 " << min_arm1  << " minimum = " << minimum;
    ui->doubleSpinBox_arm1->setValue(min_arm1) ;

    calculate_arm2();

  }

}

}

void CalibrateWindow::on_actionLoad_calibration_data_triggered()
{
    on_clear_clicked();

    QString calib_file = QFileDialog::getOpenFileName (this,tr("Load calib data"), "./data" ,"CALIB files (*.cal)");
    //QDir d = QFileInfo(calib_file).absoluteDir();

    QFile file(calib_file);

    QDataStream in(&file);



    if (file.open(QIODevice::ReadOnly))
    {
        int listSize;
        in >> listSize; // Načtěte počet položek v seznamu
        //qDebug() << " pocet polozek "  << listSize;

        for (int i = 0; i < listSize; ++i) {
            double alfa, beta;
            //qDebug() << " ....:" ;
            in >> alfa;
            in >> beta;
            angleslist.append(Angles(alfa, beta));
            QString countString = QString::number(angleslist.count());
            ui->label_points->setText(countString);
        }
        check_calculate_enable();
        file.close();
    }
//create_new_set(ui->doubleSpinBox_arm1->value(),ui->doubleSpinBox_arm2->value());
}

void CalibrateWindow::on_actionSave_calibration_data_triggered()
{
    QString calib_file = QFileDialog::getSaveFileName (this,tr("Save calib data"), "./data" ,"CALIB files (*.cal)");
    //QString export_file = QFileDialog::getSaveFileName (this,tr("Export DXF"),QDir::dirName() + formattedTime,"DXF files (*.dxf)");

    QDir d = QFileInfo(calib_file).absoluteDir();
    //setting->directory_save_dxf=d.absolutePath();

    qDebug() << "filename " << calib_file ;



    QFile file(calib_file);


    if (!file.open(QFile::WriteOnly))
    {
        qDebug()<< "error";
       // setting->document_saved = false;
    }
    else {
        // setting->document_modified=false;
        //ui->actionSave_file->setEnabled(false);
        // setting->document_saved = true;
        //qDebug() << "modified"   << setting->document_modified ;

    QDataStream out(&file);

    out << angleslist.size();
    for (const Angles &angles : angleslist) {
            out << angles.alfa;
            out << angles.beta;
        }
    file.close();


    // Vytvoření nového souboru s jinou příponou
    QFileInfo fileInfo(calib_file);
    QString calib_file_txt = fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName() + ".txt";
    QFile file2(calib_file_txt);
    //qDebug() << "file 2 " << file2;
    QTextStream out2(&file2);

    if (file2.open(QIODevice::WriteOnly)) {
        foreach (const Angles& angle, angleslist)
              {
              out2 << angle.alfa ;
              out2 << "  " << angle.beta ;
              out2 << "0\n";

              }
        file2.close(); // Uzavření souboru, když je zápis dokončen.
    } else {
        // Nastala chyba při otevírání nového souboru pro zápis.
        qDebug() << "Chyba při otevírání nového souboru pro zápis.";
    }


    }

}



void CalibrateWindow::on_delete_max_error_point_clicked()
{

    qDebug()<< "mazu na index " << calibration_set.max_error_index;
    angleslist.removeAt(calibration_set.max_error_index);
    QString countString = QString::number(angleslist.count());
    ui->label_points->setText(countString);
    ui->doubleSpinBox_krok->setValue(5);
    check_calculate_enable();
    //create_new_set(ui->doubleSpinBox_arm1->value(),ui->doubleSpinBox_arm2->value());
    calculate_clickedstarej(1);
   // on_calculate_clicked2(5);
}

void CalibrateWindow::check_calculate_enable(void)
{
    //ui->calculate->setEnabled(angleslist.count() > 100);
}




void CalibrateWindow::on_pushButton_toggled(bool checked)
{
    qDebug()<< "calibrate" << checked ;
    emit button_calibrate_clicked(checked); // <-- předáš stav
}

void CalibrateWindow::on_pushButton_clicked()
{

}

void CalibrateWindow::on_calculate_clicked()
{

    QApplication::setOverrideCursor(Qt::WaitCursor);
    double stepArm = ui->doubleSpinBox_krok->value();
    double tempstep = stepArm;
    for (int i=1;i<100;i++){
     CalibrationEngine engine(ui->doubleSpinBox_arm1_2->value(),ui->doubleSpinBox_arm2_2->value());
     engine.setAngles(angleslist);
     engine.computeOpositPoints();
    //qDebug() << "vysledek z noveho engine arm1" << engine.getArm1() << "  arm2=" << engine.getArm2();
    qDebug() << " circlefit " << engine.getCircleRadius();
    double percent = 1;
    DeviationResult DeviationResult = computeMaxDeviation(angleslist,
                                             engine.points(),
                                             engine.getArm1(),
                                             engine.getArm2(),
                                             percent);
    qDebug() << " minIndexArm1 " << DeviationResult.minIndexArm1 << " mixDeviationArm1" << DeviationResult.minDeviationArm1 << " maxIndexArm1 " << DeviationResult.maxIndexArm1 << " maxDeviationArm1 " << DeviationResult.maxDeviationArm1;
    qDebug() << " minIndexArm2 " << DeviationResult.minIndexArm2 << " mixDeviationArm2" << DeviationResult.minDeviationArm2 << " maxIndexArm2 " << DeviationResult.maxIndexArm2 << " maxDeviationArm2 " << DeviationResult.maxDeviationArm2;

    CalibrationResult result = engine.optimizeArms(
                ui->doubleSpinBox_prumer->value(),       // reference distance
                stepArm,         // step arm1
                stepArm,         // step arm2
                10,                                      // steps
                engine.getBetaMinIndex(),
                engine.getBetaMinOpositIndex(),
                engine.getAlfaMinIndex(),
                engine.getAlfaMinOpositIndex()
                );
               qDebug() << " optimalizovane rameno 1 " << result.adjustedArm1 << " optimalizovane rameno 2 " << result.adjustedArm2 ;
    qDebug() << "Součet všech odchylek diff:"  <<  engine.totalDeviationFromReference(ui->doubleSpinBox_prumer->value());

    qDebug() << " prumerna hodnota "  <<engine.getaverage_dist();
    ui->doubleSpinBox_arm1_2->setValue(result.adjustedArm1);
    ui->doubleSpinBox_arm2_2->setValue(result.adjustedArm2);
    stepArm=stepArm*0.8;


    QString circlerad = QString::number(engine.getCircleRadius()*2);
    ui->refcircle_deriv->setText(circlerad);
    QString circlerr = QString::number(engine.getmaxErrorCircle());
    ui->maxcirclerrorderiv->setText(circlerr);
    QString circlaverageerr = QString::number(engine.getAverageCircleFitError());
    ui->avgaerrderiv->setText(circlaverageerr);
    QString sumeerr = QString::number(engine.totalDeviationFromReference(ui->doubleSpinBox_prumer->value()));
    ui->sumaerrderiv->setText(sumeerr);

    //QThread::sleep(1); // pozastaví běh na 1 sekundu
    ui->doubleSpinBox_krok->setValue(stepArm);
    QApplication::processEvents();
    QThread::msleep(100); //0.5sec

    }
//--------------konec deriv ---- zacatek grid
    ui->doubleSpinBox_krok->setValue(tempstep);
    double stepArm1 = ui->doubleSpinBox_krok->value();
    double stepArm2 = ui->doubleSpinBox_krok->value();
    //double percent = 0.0;
    double percent = 1;
    int steps = 10;
    for (int i=0 ; stepArm1>0.0001 ;i++)
    {
    CalibrationEngine engine2(ui->doubleSpinBox_arm1->value(),ui->doubleSpinBox_arm2->value());
    engine2.setAngles(angleslist);
    engine2.computeOpositPoints();
    DeviationResult DeviationResult = computeMaxDeviation(angleslist,
                                             engine2.points(),
                                             engine2.getArm1(),
                                             engine2.getArm2(),
                                             percent);
    qDebug() << " minIndexArm1 " << DeviationResult.minIndexArm1 << " mixDeviationArm1" << DeviationResult.minDeviationArm1 << " maxIndexArm1 " << DeviationResult.maxIndexArm1 << " maxDeviationArm1 " << DeviationResult.maxDeviationArm1;
    qDebug() << " minIndexArm2 " << DeviationResult.minIndexArm2 << " mixDeviationArm2" << DeviationResult.minDeviationArm2 << " maxIndexArm2 " << DeviationResult.maxIndexArm2 << " maxDeviationArm2 " << DeviationResult.maxDeviationArm2;
    double refdist=ui->doubleSpinBox_prumer->value();
    CalibrationResult result = engine2.optimizeArmsGrid(refdist,stepArm1, stepArm2, steps);

    qDebug() << "Optimal arm1 from grid:" << result.adjustedArm1;
    qDebug() << "Optimal arm2 from grid:" << result.adjustedArm2;
    qDebug() << "Součet všech odchylek diff:"  <<  engine2.totalDeviationFromReference(ui->doubleSpinBox_prumer->value());


    ui->doubleSpinBox_arm1->setValue(result.adjustedArm1);
    ui->doubleSpinBox_arm2->setValue(result.adjustedArm2);
    QApplication::processEvents();
    stepArm1=stepArm1*0.9;
    stepArm2=stepArm2*0.9;
    ui->doubleSpinBox_krok->setValue(stepArm1);
    QString circlerad = QString::number(engine2.getCircleRadius()*2);
    ui->refcircle_grid->setText(circlerad);
    QString circlerr = QString::number(engine2.getmaxErrorCircle());
    ui->maxcirclerrorgrid->setText(circlerr);
    QString circlaverageerr = QString::number(engine2.getAverageCircleFitError());
    ui->avgaerrgrid->setText(circlaverageerr);
    QString sumeerr = QString::number(engine2.totalDeviationFromReference(ui->doubleSpinBox_prumer->value()));
    ui->sumaerrgrid->setText(sumeerr);


    //QThread::sleep(1); // pozastaví běh na 1 sekundu
    QThread::msleep(100); //0.5sec
    QApplication::processEvents();
    }
    QApplication::restoreOverrideCursor();
    ui->doubleSpinBox_krok->setValue(tempstep);
}

void CalibrateWindow::runDerivativeCalculation()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    double stepArm = ui->doubleSpinBox_krok->value();
    double tempstep = stepArm;
    for (int i = 1; i < 100; i++) {
        CalibrationEngine engine(ui->doubleSpinBox_arm1_2->value(), ui->doubleSpinBox_arm2_2->value());
        engine.setAngles(angleslist);
        engine.computeOpositPoints();
        qDebug() << " circlefit " << engine.getCircleRadius();
        double percent = 1;
        DeviationResult DeviationResult = computeMaxDeviation(angleslist,
                                                              engine.points(),
                                                              engine.getArm1(),
                                                              engine.getArm2(),
                                                              percent);
        qDebug() << " minIndexArm1 " << DeviationResult.minIndexArm1 << " mixDeviationArm1" << DeviationResult.minDeviationArm1 << " maxIndexArm1 " << DeviationResult.maxIndexArm1 << " maxDeviationArm1 " << DeviationResult.maxDeviationArm1;
        qDebug() << " minIndexArm2 " << DeviationResult.minIndexArm2 << " mixDeviationArm2" << DeviationResult.minDeviationArm2 << " maxIndexArm2 " << DeviationResult.maxIndexArm2 << " maxDeviationArm2 " << DeviationResult.maxDeviationArm2;

        CalibrationResult result = engine.optimizeArms(
                    ui->doubleSpinBox_prumer->value(),
                    stepArm,
                    stepArm,
                    10,
                    engine.getBetaMinIndex(),
                    engine.getBetaMinOpositIndex(),
                    engine.getAlfaMinIndex(),
                    engine.getAlfaMinOpositIndex());
        qDebug() << " optimalizovane rameno 1 " << result.adjustedArm1 << " optimalizovane rameno 2 " << result.adjustedArm2;
        qDebug() << "Součet všech odchylek diff:"  <<  engine.totalDeviationFromReference(ui->doubleSpinBox_prumer->value());

        qDebug() << " prumerna hodnota "  <<engine.getaverage_dist();
        ui->doubleSpinBox_arm1_2->setValue(result.adjustedArm1);
        ui->doubleSpinBox_arm2_2->setValue(result.adjustedArm2);
        stepArm=stepArm*0.8;


        QString circlerad = QString::number(engine.getCircleRadius()*2);
        ui->refcircle_deriv->setText(circlerad);
        QString circlerr = QString::number(engine.getmaxErrorCircle());
        ui->maxcirclerrorderiv->setText(circlerr);
        QString circlaverageerr = QString::number(engine.getAverageCircleFitError());
        ui->avgaerrderiv->setText(circlaverageerr);
        QString sumeerr = QString::number(engine.totalDeviationFromReference(ui->doubleSpinBox_prumer->value()));
        ui->sumaerrderiv->setText(sumeerr);

        ui->doubleSpinBox_krok->setValue(stepArm);
        QApplication::processEvents();
        QThread::msleep(100); //0.5sec

    }
    ui->doubleSpinBox_krok->setValue(tempstep);
    QApplication::restoreOverrideCursor();
}

void CalibrateWindow::runGridCalculation()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    double tempstep = ui->doubleSpinBox_krok->value();
    double stepArm1 = ui->doubleSpinBox_krok->value();
    double stepArm2 = ui->doubleSpinBox_krok->value();
    double percent = 1;
    int steps = 10;
    for (int i=0 ; stepArm1>0.0001 ;i++)
    {
        CalibrationEngine engine2(ui->doubleSpinBox_arm1->value(),ui->doubleSpinBox_arm2->value());
        engine2.setAngles(angleslist);
        engine2.computeOpositPoints();
        DeviationResult DeviationResult = computeMaxDeviation(angleslist,
                                             engine2.points(),
                                             engine2.getArm1(),
                                             engine2.getArm2(),
                                             percent);
        qDebug() << " minIndexArm1 " << DeviationResult.minIndexArm1 << " mixDeviationArm1" << DeviationResult.minDeviationArm1 << " maxIndexArm1 " << DeviationResult.maxIndexArm1 << " maxDeviationArm1 " << DeviationResult.maxDeviationArm1;
        qDebug() << " minIndexArm2 " << DeviationResult.minIndexArm2 << " mixDeviationArm2" << DeviationResult.minDeviationArm2 << " maxIndexArm2 " << DeviationResult.maxIndexArm2 << " maxDeviationArm2 " << DeviationResult.maxDeviationArm2;
        double refdist=ui->doubleSpinBox_prumer->value();
        CalibrationResult result = engine2.optimizeArmsGrid(refdist,stepArm1, stepArm2, steps);

        qDebug() << "Optimal arm1 from grid:" << result.adjustedArm1;
        qDebug() << "Optimal arm2 from grid:" << result.adjustedArm2;
        qDebug() << "Součet všech odchylek diff:"  <<  engine2.totalDeviationFromReference(ui->doubleSpinBox_prumer->value());


        ui->doubleSpinBox_arm1->setValue(result.adjustedArm1);
        ui->doubleSpinBox_arm2->setValue(result.adjustedArm2);
        QApplication::processEvents();
        stepArm1=stepArm1*0.9;
        stepArm2=stepArm2*0.9;
        ui->doubleSpinBox_krok->setValue(stepArm1);
        QString circlerad = QString::number(engine2.getCircleRadius()*2);
        ui->refcircle_grid->setText(circlerad);
        QString circlerr = QString::number(engine2.getmaxErrorCircle());
        ui->maxcirclerrorgrid->setText(circlerr);
        QString circlaverageerr = QString::number(engine2.getAverageCircleFitError());
        ui->avgaerrgrid->setText(circlaverageerr);
        QString sumeerr = QString::number(engine2.totalDeviationFromReference(ui->doubleSpinBox_prumer->value()));
        ui->sumaerrgrid->setText(sumeerr);


        QThread::msleep(100); //0.5sec
        QApplication::processEvents();
    }
    QApplication::restoreOverrideCursor();
    ui->doubleSpinBox_krok->setValue(tempstep);
}

void CalibrateWindow::runLeastSquaresCalculation()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    CalibrationEngine engine(ui->doubleSpinBox_arm1_3->value(), ui->doubleSpinBox_arm2_3->value());
    engine.setAngles(angleslist);
    engine.computeOpositPoints();
    CalibrationResult result = engine.optimizeArmsLeastSquares(ui->doubleSpinBox_prumer->value());
    ui->doubleSpinBox_arm1_3->setValue(result.adjustedArm1);
    ui->doubleSpinBox_arm2_3->setValue(result.adjustedArm2);
    QString circlerad = QString::number(engine.getCircleRadius()*2);
    ui->refcircle_ls->setText(circlerad);
    QString circlerr = QString::number(engine.getmaxErrorCircle());
    ui->maxcirclerrorls->setText(circlerr);
    QString circlaverageerr = QString::number(engine.getAverageCircleFitError());
    ui->avgaerrls->setText(circlaverageerr);
    QString sumeerr = QString::number(engine.totalDeviationFromReference(ui->doubleSpinBox_prumer->value()));
    ui->sumaerrls->setText(sumeerr);
    QApplication::restoreOverrideCursor();
}

void CalibrateWindow::on_calculate_deriv_clicked()
{
    runDerivativeCalculation();
}

void CalibrateWindow::on_calculate_grid_clicked()
{
    runGridCalculation();
}

void CalibrateWindow::on_calculate_ls_clicked()
{
    runLeastSquaresCalculation();
}
