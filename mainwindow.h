#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "appmanager.h" //musi byt
#include "settings.h"
#include <QGraphicsView>
#include <QMainWindow>
#include <QObject>
#include <QSerialPort>
#include <QShortcut>
#include <QtCore>
#include <QtGui>

QT_BEGIN_NAMESPACE
// class QModbusClient;
// class QModbusReply;
class QGraphicsView;
class QGraphicsItem;
class QGraphicsLineItem;
class QGraphicsEllipseItem;
class AppManager;
class SettingsManager;
class CustomToolButton;
class MeasureDialog;
class CalibrateWindow;
class InfoDialog;
class GraphicsItems;
class QKeyEvent;
class QMouseEvent;
enum class AddPointMode;
enum class ZoomMode { All, Dynamic, User };

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// class WriteRegisterModel;

class MainWindow : public QMainWindow {
  Q_OBJECT
  Ui::MainWindow *ui = nullptr;

public:
  explicit MainWindow(AppManager *app, QWidget *parent = nullptr);
  ~MainWindow();
  AppManager *appManager() const { return appManager_; }
  SettingsManager *settingsManager() const { return settingsManager_; }
  void onAddPointModeChanged(AddPointMode mode);
  void onContiModeChanged(ContiMode mode);

  void updateUiForMode(AddPointMode mode);
  MeasureDialog *measure = nullptr;
  CalibrateWindow *calibrate = nullptr;
  // QPointer<CalibrateWindow> calibrate;
  InfoDialog *info = nullptr;
  GraphicsItems *lastitem;
  GraphicsItems *currentobject;
  QPointF position1;
  QPointF endpoint;
  QPointF lastpoint;
  double distance;
  double alfa, beta;
  int index;
  // int index_of_slave;
  // float number[3] ; // pole pro oba encodery
  double_t number[3];
  QElapsedTimer elapsedtimer; // mereni casu
  bool shortcutPressed(const QString &shortcutStr, QKeyEvent *keyEvent);
  void handleCalibrateButtonClicked(bool checked);
  QGraphicsLineItem *arm1 = nullptr; // rameno 1
  QGraphicsLineItem *arm2 = nullptr; // rameno 2
  QGraphicsEllipseItem *base;
  double x_value, y_value;   // koncova pozice prvniho ramene
  double x_value2, y_value2; // koncova pozice druheho ramene
  QTranslator translator;
  QTranslator guitranslator;
  QStringList languages;

private:
  // std::unique_ptr<Ui::MainWindow> ui;  // vlastnictví UI
  QPointer<AppManager> appManager_;
  QPointer<SettingsManager> settingsManager_;
  QPointF lastEndArm2_{};
  // void closeEvent(QCloseEvent* event) override;

signals:
  void dataReady(double alfa, double beta);

private slots:
  void updateArms(double Arm1Angle, double Arm2Angle, QPointF endPointArm1,
                  QPointF endPointArm2);
  void handleError(QSerialPort::SerialPortError error);
  void onSettingsChanged(const Settings &s);
  // void itemSelected();
  void sceneModified(QPointF);
  // void calibrateWindowClosed(int result);
  // void calibrateWindowClosed();
  void on_actionDelete_last_point_triggered();

  void on_actionSave_dxf_triggered();
  void on_actionset_zero_triggered();
  void on_actionAuto_triggered();
  void on_actionNew_file_triggered();
  void on_actionInfo_triggered();
  void on_actionSave_file_data_triggered();
  void on_actionLoad_file_data_triggered();

  void on_actionAdd_polyline_toggled(bool arg1);

  void on_actionAdd_circle_toggled(bool arg1);

  void on_actionMeasure_toggled(bool arg1);

  void on_actionadd_point_triggered();

  void on_actionSetup_triggered(bool checked);

  void on_actionConnect_triggered();

public slots:
  void Zoom_Dynamic();
  void Zoom_All();
  void Zoom_User();
  void toggleZoomMode();
  void status_bar_print(QString, int);
  void setup_scene();
  // void retranslate();
  // void handleDocumentModifiedChanged(bool newValue);
  void onLanguageChanged(const QString &language);

private:
  void initActions();
  void initMenu();

  QMenu *ZoomMenu;
  CustomToolButton *ZoomToolButton;
  ZoomMode zoomMode_ = ZoomMode::All;
  QShortcut *zoomShortcut_ = nullptr;

  /*modbus*/
  // QModbusDataUnit readRequest() const;
  // QModbusReply *lastRequest;
  // QModbusClient *modbusDevice, *mymodbus_Device;
  QSerialPort *serialDevice = nullptr;

public:
  // WriteRegisterModel *writeModel;
  QAction *actionZoom_All;
  QAction *actionZoom_Dynamic;
  QAction *actionZoom_User;
  QGraphicsScene *scene;

  QGraphicsLineItem *line;
  QGraphicsRectItem *rectangle;

protected:
  void closeEvent(QCloseEvent *event) override;
};

class GraphicsView : public QGraphicsView {
  Q_OBJECT
public:
  GraphicsView(QWidget *parent = 0);

protected:
  void wheelEvent(QWheelEvent *) override;
  void mousePressEvent(QMouseEvent *event) override;
  // void mouseDoubleClickEvent(QMouseEvent *event) override;
  // void mouseReleaseEvent(QMouseEvent* event) override;
  // void mouseMoveEvent(QMouseEvent* event) override;

signals:

  void text_to_status_bar(QString, int);

private:
  int pan_originalX = 0;
  int pan_originalY = 0;
  bool pan_moving = false;
  int zoom_originalX = 0;
  int zoom_originalY = 0;
  bool zoom_moving = false;
  QPen zoom_pen;
  QGraphicsItem *zoom_rect;
};

#endif // MAINWINDOW_H
