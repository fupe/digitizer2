#include "3rdparty/dxflib/src/dl_dxf.h"
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog> //save dxf
#include <QGraphicsScene>
#include <QLineF>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPen>
#include <QtMath>

#include "CustomToolButton.h"
#include "GraphicsItems.h"
#include "InfoDialog.h"
#include "MeasureDialog.h"
#include "SettingsDialog.h"
#include "appmanager.h"
#include "calibratewindow.h"
#include "mainwindow.h"
#include "settings.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(AppManager *app, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), appManager_(app) {
  Q_ASSERT(appManager_);
  ui->setupUi(this); // ← a hned setupUi

  // až teď existují všechny prvky z UI → můžeš nastavovat akce, signály,
  // shortcuty…
  settingsManager_ = appManager_->settingsManager();
  Q_ASSERT(settingsManager_);

  qDebug() << "konstruktoru";
  // mapování UI → AppManager
  connect(ui->actionConnect, &QAction::triggered, this, [this](bool) {
    appManager_->openSerial();
  }); // zavolá metodu bez argumentů
  connect(ui->actionDisconnect, &QAction::triggered, this,
          [this](bool) { appManager_->closeSerial(); });
  connect(appManager_, &AppManager::serialOpened, this,
          [this]() { updateSerialActions(true); });
  qDebug() << "konstruktoru 2";
  connect(appManager_, &AppManager::serialClosed, this,
          [this]() { updateSerialActions(false); });
  qDebug() << "konstruktoru 3";
  updateSerialActions(appManager_->isSerialConnected());
  connect(ui->actionAdd_polyline, &QAction::toggled, this, [this](bool on) {
    if (on)
      appManager()->setAddPointMode(AddPointMode::Polyline);
  });
  connect(ui->actionAdd_circle, &QAction::toggled, this, [this](bool on) {
    if (on)
      appManager()->setAddPointMode(AddPointMode::Circle);
  });
  connect(ui->actionCalibrate, &QAction::toggled, this, [this](bool on) {
    if (on)
      appManager()->setAddPointMode(AddPointMode::Calibrate);
    else
      appManager()->setAddPointMode(AddPointMode::None);
  });
  connect(appManager(), &AppManager::modeAddPointChanged, this,
          &MainWindow::onAddPointModeChanged); // reakce na zmenu modu
  connect(appManager(), &AppManager::modeContiChanged, this,
          &MainWindow::onContiModeChanged); // reakce na zmenu modu conti

  qDebug() << "konstruktoru 4";
  // UI → AppManager
  // AppManager → status bab
  connect(appManager_, &AppManager::connectionNotice, this,
          [this](const QString &msg) { statusBar()->showMessage(msg, 5000); });
  connect(appManager_, &AppManager::serialError, this,
          [this](const QString &e) {
            statusBar()->showMessage(e, 8000);
            qWarning() << "STATUSBAR error:" << e;
          });
  const auto &cs = settingsManager_->currentSettings();
  if (cs.main_window_position.isValid()) {
    setGeometry(cs.main_window_position);
  }
  //-----
  connect(settingsManager_, &SettingsManager::settingsChanged, this,
          &MainWindow::onSettingsChanged);
  onLanguageChanged(cs.language);
  connect(ui->actionExit, &QAction::triggered, this, [this]() {
    //qDebug() << "Menu Exit triggered";
    this->close(); // vyvolá closeEvent()
  });
  this->setWindowTitle(tr("Digitizer"));
  // qApp->installEventFilter(this);
  // this->installEventFilter(this);
  initMenu();
  ui->toolBar_2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  ui->toolBar_2->setIconSize(QSize(50, 50));
  /*graphics*/
  scene = new QGraphicsScene(this);
  setup_scene();
  connect(appManager_, &AppManager::armsUpdated, this, &MainWindow::updateArms);
  connect(appManager_, &AppManager::sceneModified, this,
          &MainWindow::sceneModified);

  // nastavit výchozí klávesové zkratky podle aktuálních nastavení
  onSettingsChanged(cs);

  qDebug() << "konec konstruktoru";
  onAddPointModeChanged(AddPointMode::None); // vyplneni zkratek v tooltipu a podobne
  onContiModeChanged(appManager()->getContiMode()); // vyplni zkratky u conti a
  Zoom_All();


}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::updateSerialActions(bool connected)
{
  ui->actionConnect->setEnabled(!connected);
  ui->actionDisconnect->setEnabled(connected);
}

void MainWindow::closeEvent(QCloseEvent *event)

{
  qDebug() << "MainWindow::closeEvent " << &event;
  {
    if (!appManager())
      return QWidget::closeEvent(event);
    auto s = settingsManager_->currentSettings();
    if (s.save_main_window_position_on_exit) {
      s.main_window_position = this->geometry(); // QRect jako dřív
      settingsManager_->updateSettings(
          s); // commit (uloží do QSettings přes SettingsManager)
    }
    QWidget::closeEvent(event);
  }
}

/*
void MainWindow::readData()
{
    const QByteArray data = serialDevice->readAll();
    //qDebug() << "readAll" << data ;
    if (data.startsWith("#A:"))
    {
        int start;
        int end;
        int length ;
        start=data.indexOf(":")+1;
        end=data.indexOf("|");
        length = end-start;
        //qDebug() << "retezec: " << data.mid(start,length);
        double promena;
        bool ok;
        promena = data.mid(start,length).toDouble(&ok);
        //qDebug() << "cislo: " << promena << "ok: " << ok ;
        alfa=double(promena);   // - setting->alfa_offset;
        //appManager.setAngles(alfa,beta,index);
    }
    if (data.startsWith("#B:"))
    {
        int start;
        int end;
        int length ;
        start=data.indexOf(":")+1;
        end=data.indexOf("|");
        length = end-start;
        //qDebug() << "retezec: " << data.mid(start,length);
        double promena;
        bool ok;
        promena = data.mid(start,length).toDouble(&ok);
        //qDebug() << "cislo: " << promena << "ok: " << ok ;
        beta=double(-promena);   // - setting->alfa_offset;
        //appManager.setAngles(alfa,beta,index);
    }
    if (data.startsWith("#I:"))
    {
        int start;
        int end;
        int length ;
        start=data.indexOf(":")+1;
        end=data.indexOf("|");
        length = end-start;
        //qDebug() << "retezec: " << data.mid(start,length);
        double promena;
        bool ok;
        promena = data.mid(start,length).toDouble(&ok);
        //qDebug() << "cislo: " << promena << "ok: " << ok ;
        index=double(promena);   // - setting->alfa_offset;

        appManager()->setAngles(alfa,beta,index);

    }

}
*/

void MainWindow::Zoom_Dynamic() {
  zoomMode_ = ZoomMode::Dynamic;

  ZoomToolButton->setDefaultAction(actionZoom_Dynamic);

  auto *sc = ui->graphicsView->scene();
  if (!sc)
    return;

  QRectF bounds;
  bool hasItem = false;

  for (QGraphicsItem *item : sc->items()) {
    if (auto *shape = dynamic_cast<GraphicsItems *>(item)) {
        //qDebug()<<"shape" <<  shape->sceneBoundingRect() ;
      if (!hasItem) {
        bounds = shape->sceneBoundingRect();
        //qDebug()<<"!hasItem" <<  bounds ;
        hasItem = true;
      } else {
        bounds = bounds.united(shape->sceneBoundingRect());
      //  qDebug()<<"hasItem" <<  bounds ;
      }
    }

  }

  QRectF endPointRect(lastEndArm2_, QSizeF(1, 1)); // obdélník okolo aktuální pozice
  if (hasItem) {
    bounds = bounds.united(endPointRect);
    //qDebug()<<"hasItem endpoint" <<  bounds ;
  } else {
    bounds = endPointRect;
    //qDebug()<<"!hasItem endpoint" <<  bounds ;
    hasItem = true;
  }

  /*if (arm2) {
    bounds = bounds.united(arm2->sceneBoundingRect());
    qDebug()<<"arm2 " <<  bounds ;
    hasItem = true;
  }*/

  if (hasItem) {
    bounds.adjust(-20, -20, 20, 20); // přidá okraje
    //qDebug()<<"bounds.adjust " <<  bounds ;
    //qDebug()<<"bounds.center " <<  bounds.center() ;
    ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
    //ui->graphicsView->fitInView(-25, -25, 50, 50, Qt::KeepAspectRatio);
    ui->graphicsView->centerOn(bounds.center());
  }
  emit zoomModeChanged(zoomMode_);
}

void MainWindow::Zoom_All() {
    qDebug()<<"-----------------------------";
  zoomMode_ = ZoomMode::All;
  ZoomToolButton->setDefaultAction(actionZoom_All);

  const auto &s = settingsManager_->currentSettings();
  ui->graphicsView->fitInView(QRect(-1.05 * (s.arm1_length + s.arm2_length),
                                    -1.05 * (s.arm1_length + s.arm2_length),
                                    2.1 * (s.arm1_length + s.arm2_length),
                                    2.1 * (s.arm1_length + s.arm2_length)),
                              Qt::KeepAspectRatio);
  qDebug()<<"rect ------------- " << QRect(-1.05 * (s.arm1_length + s.arm2_length),
                                           -1.05 * (s.arm1_length + s.arm2_length),
                                           2.1 * (s.arm1_length + s.arm2_length),
                                           2.1 * (s.arm1_length + s.arm2_length)) ;
  emit zoomModeChanged(zoomMode_);
}

void MainWindow::Zoom_User() {
  zoomMode_ = ZoomMode::User;
  ZoomToolButton->setDefaultAction(actionZoom_User);
  emit zoomModeChanged(zoomMode_);
}

void MainWindow::toggleZoomMode() {
  if (zoomMode_ == ZoomMode::All) {
    Zoom_Dynamic();
  } else {
    Zoom_All();
  }
}

void MainWindow::status_bar_print(QString text, int delay) {
  statusBar()->showMessage(text, delay);
}

void MainWindow::initActions() {
  QString menuStyle("QMenu::item{"
                    "background-color: rgb(200, 170, 0);"
                    "color: rgb(255, 255, 255);"
                    "}"
                    "QMenu::item:selected{"
                    "background-color: rgb(0, 5, 127);"
                    "color: rgb(255, 255, 255);"
                    "}");
}

void MainWindow::initMenu() {
  /*--------pridani cudliku zoom-------*/
  actionZoom_All = new QAction(tr("Zoom All"), this);
  actionZoom_Dynamic = new QAction(tr("Zoom Dynamic"), this);
  actionZoom_User = new QAction(tr("Zoom User"), this);
  actionZoom_All->setIcon(QIcon(":/pic/zoom_all.png"));
  actionZoom_Dynamic->setIcon(QIcon(":/pic/zoom_select.png"));
  actionZoom_User->setIcon(QIcon(":/pic/zoom_user.png"));
  connect(actionZoom_All, &QAction::triggered, this, &MainWindow::Zoom_All);
  connect(actionZoom_Dynamic, &QAction::triggered, this,
          &MainWindow::Zoom_Dynamic);
  connect(actionZoom_User, &QAction::triggered, this, &MainWindow::Zoom_User);
  ZoomMenu = new QMenu;
  ZoomMenu->addAction(actionZoom_All);
  ZoomMenu->addAction(actionZoom_Dynamic);
  ZoomMenu->addAction(actionZoom_User);
  ZoomToolButton = new CustomToolButton;
  ZoomToolButton->setMenu(ZoomMenu);
  ZoomToolButton->setDefaultAction(actionZoom_All);
  ZoomToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  ZoomToolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ui->toolBar_2->addWidget(ZoomToolButton);
  zoomShortcut_ = new QShortcut(this);
  zoomShortcut_->setKey(settingsManager_->currentSettings().shortcuts.map.value(
      QStringLiteral("action.zoom")));
  zoomShortcut_->setContext(Qt::ApplicationShortcut);
  connect(zoomShortcut_, &QShortcut::activated, this,
          &MainWindow::toggleZoomMode);
  /*-----------konec zoom-------*/
  ZoomMenu->setTitle("zoommenu");
}

void MainWindow::setup_scene() {
  // qDebug() << "scene = " << scene ;
  const auto &s = settingsManager_->currentSettings();
  //scene->setSceneRect(-100, -100, 200, 200);
  scene->setSceneRect(QRect(-1.05*(s.arm1_length+s.arm2_length),-1.05*(s.arm1_length+s.arm2_length),2.1*(s.arm1_length+s.arm2_length),2.1*(s.arm1_length+s.arm2_length)));
  ui->graphicsView->setScene(scene);
  ui->graphicsView->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
  //qDebug()<<"scener eckt"<< scene->sceneRect();
  appManager_->setScene(scene);
  // qDebug() << "scene = 1";
  ui->graphicsView->setRenderHints(QPainter::Antialiasing);
  ui->graphicsView->setDragMode(GraphicsView::RubberBandDrag);
  ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
  // qDebug() << "scene =5 " << s.arm1_length;
  //    ui->graphicsView->setDragMode(selectModeButton->isChecked() ?
  //    QGraphicsView::RubberBandDrag : QGraphicsView::ScrollHandDrag);
  ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
  // qDebug() << "scene = 6";
  arm1 = scene->addLine(0, 0, s.arm1_length, 0, *s.arms_pen);
  // scene->addLine(0,0,200,200,*s.arms_pen);
  // qDebug() << "scene = 7";
  arm2 = scene->addLine(0, 0, s.arm2_length, 0, *s.arms_pen);
  arm1->setZValue(100); // cary v popredi
  arm2->setZValue(101);
  base = scene->addEllipse(-25, -25, 50, 50, *s.arms_pen);
  arm2->setPos(s.arm1_length, 0);
  arm1->show();
  arm2->show();

  //ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
  // ui->graphicsView->show();
  // qDebug() << " konecscene = " ;

}

void MainWindow::on_actionDelete_last_point_triggered()
{
    if (appManager()) {
        appManager()->deleteLastPoint();
    }
}

void MainWindow::sceneModified(QPointF) {

  if (zoomMode_ == ZoomMode::Dynamic)
  {
      //qDebug()<<" zoom dynamic" ;
    Zoom_Dynamic();
  }

}


GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)

{
  zoom_pen.setColor(Qt::black);
  zoom_pen.setWidth(2);
}

void GraphicsView::wheelEvent(QWheelEvent *e) {
  qDebug() << "wheel" << e;
  // if (e->modifiers() ) //& Qt::ControlModifier)
  //{
  const ViewportAnchor anchor = transformationAnchor();
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  int angle = e->angleDelta().y();
  //qDebug() << "wheel + control event " << e << "delta " << e->angleDelta().y();
  qreal factor;
  if (angle > 0) {
    factor = 1.1;
  } else {
    factor = 0.9;
  }
  scale(factor, factor);
  setTransformationAnchor(anchor);
  MainWindow *mw = qobject_cast<MainWindow *>(this->window());
  if (mw) // && mw->zoomMode() != ZoomMode::Dynamic)
    mw->Zoom_User();
  //qDebug() << "anchor " << anchor;
  //qDebug() << "transform " << transform();
  //}
}

void GraphicsView::mousePressEvent(QMouseEvent *event) {
  MainWindow *mw = qobject_cast<MainWindow *>(this->window());
  if (mw && mw->appManager()->getAddPointMode() == AddPointMode::Polyline) {
    if (event->button() == Qt::LeftButton) {
      mw->appManager()->addpointfrommainwindow();
      event->accept();
      return;
    } else if (event->button() == Qt::RightButton) {
      mw->appManager()->cancelCurrentAction();
      event->accept();
      return;
    }
  }
  QGraphicsView::mousePressEvent(event);
}

/*void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "button press zoom pen" << zoom_pen;
    if (event->buttons() == Qt::RightButton){
        //setDragMode(QGraphicsView::NoDrag);
        setDragMode(QGraphicsView::ScrollHandDrag);
        qDebug() << "right buton press" << event;
        pan_originalX = event->x();
        pan_originalY = event->y();
        // set the "moving" state
        pan_moving = true;
        qDebug() << "right button press" << pan_moving <<  " pos " << event->x()
<< event->y(); emit text_to_status_bar("Pan window",5000);
    }
    else if (event->buttons() == Qt::LeftButton){

        qDebug() << "left button press";
        if (event->modifiers() & Qt::ControlModifier)
        {
            //GraphicsItems *currentobject ;
            //currentobject = new mypoint ;
            //currentobject->setPos(mapToScene(event->pos()));
            //lastpoint mapToScene(event->pos());
            //this->scene()->addItem(currentobject);
            qDebug() << "emit";
            qDebug() << "po emit";
        }
        else
        {
            setDragMode(QGraphicsView::RubberBandDrag);
            qDebug() << "left button press";
            emit text_to_status_bar("Select by window",5000);
            QGraphicsView::mousePressEvent (event);
        }
    }
    else if (event->buttons() == Qt::MiddleButton){
        //const DragMode	mode = dragMode();
        emit text_to_status_bar("Zoom by window",5000);
        setDragMode(QGraphicsView::NoDrag);
        zoom_originalX = mapToScene(event->pos()).x();
        zoom_originalY = mapToScene(event->pos()).y();
        // set the "moving" state
        zoom_moving = true;
        qDebug() << "middle buton press  - zoom_originalX " << zoom_originalX <<
"zoom_originalY" <<zoom_originalY ;
        //QGraphicsView::mousePressEvent (event);
        setCursor(Qt::CrossCursor);
        zoom_rect = this->scene()->addRect(zoom_originalX,zoom_originalY, 5
,5,zoom_pen);

    }
    else
    {
       QGraphicsView::mousePressEvent (event);
    }
}*/

/*void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
        {
            qDebug() << "double click";
        }

}*/

/*void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton){
        qDebug() << "right buton release";
        //setDragMode(QGraphicsView::RubberBandDrag);
        setDragMode(QGraphicsView::NoDrag);
        pan_moving = false;
        qDebug() << "right buton release" << pan_moving <<  " pos " <<
event->x() << event->y();

    }
    else if  (event->button() == Qt::MiddleButton)
    {
        qDebug() << "middle buton release";
        //qDebug() << "mapovani release "  << mapToScene(event->pos());
        this->scene()->removeItem(zoom_rect);
        zoom_moving = false;
        //this->fitInView(QRect(zoom_originalX,zoom_originalY,mapToScene(event->pos()).x()-zoom_originalX
,mapToScene(event->pos()).y()-zoom_originalY),Qt::KeepAspectRatio); int
startx,starty,wide,height; if (zoom_originalX >  mapToScene(event->pos()).x())
startx = mapToScene(event->pos()).x() ; else startx = zoom_originalX ; wide   =
abs(mapToScene(event->pos()).x()-zoom_originalX); if (zoom_originalY >
mapToScene(event->pos()).y())   starty = mapToScene(event->pos()).y() ; else
starty = zoom_originalY ; height   =
abs(mapToScene(event->pos()).y()-zoom_originalY);
        this->fitInView(QRect(startx,starty,wide,height),Qt::KeepAspectRatio);

    }
    else
    {
        QGraphicsView::mouseReleaseEvent(event);
    }
    setCursor(Qt::ArrowCursor);
}*/

/*void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "mouse move  " << event;
    if (pan_moving)
        {
        const ViewportAnchor anchor = transformationAnchor();
            // panning operates in the scene coordinates using x,y
            qDebug() << "mouse move pan  " << event;
            QPointF oldp = mapToScene(pan_originalX, pan_originalY);
            QPointF newp = mapToScene(event->pos());
            QPointF translation = newp - oldp;
            setTransformationAnchor(QGraphicsView::NoAnchor);
            translate(translation.x(), translation.y());

            pan_originalX = event->x();
            pan_originalY = event->y();
            qDebug() << "mouse move" << pan_moving <<  " pos " <<
translation.x() << translation.y(); setTransformationAnchor(anchor);
        }
    else if (zoom_moving)
        {
            qDebug() << "mouse move zooming" << zoom_moving << "pos: " <<
mapToScene(event->pos()) ; this->scene()->removeItem(zoom_rect); int
startx,starty,wide,height; if (zoom_originalX >  mapToScene(event->pos()).x())
startx = mapToScene(event->pos()).x() ; else startx = zoom_originalX ; wide   =
abs(mapToScene(event->pos()).x()-zoom_originalX); if (zoom_originalY >
mapToScene(event->pos()).y())   starty = mapToScene(event->pos()).y() ; else
starty = zoom_originalY ; height   =
abs(mapToScene(event->pos()).y()-zoom_originalY); zoom_rect =
this->scene()->addRect(startx,starty, wide ,height,zoom_pen); update();
        }
    else
        {

            QGraphicsView::mouseMoveEvent(event);

        }
}*/

void MainWindow::on_actionSave_dxf_triggered() {
  QDateTime date = QDateTime::currentDateTime();
  QString formattedTime = date.toString("yyyy-MM-dd-hh-mm-ss");
  auto settings = settingsManager_->currentSettings();
  QString export_file = QFileDialog::getSaveFileName(
      this, tr("Export DXF"), settings.directory_save_dxf + "/" + formattedTime,
      "DXF files (*.dxf)");
  if (export_file.isEmpty()) {
    return;
  }
  QDir d = QFileInfo(export_file).absoluteDir();
  settings.directory_save_dxf = d.absolutePath();
  settingsManager_->updateSettings(settings);

  DL_Dxf dxf;
  QByteArray filenameArray = export_file.toLocal8Bit();
  DL_WriterA *dw = dxf.out(filenameArray.constData(), DL_Codes::AC1009);
  if (dw == nullptr) {
    qDebug() << "error";
    return;
  }

  dxf.writeHeader(*dw);
  dw->dxfString(9, "$INSUNITS");
  dw->dxfInt(70, settings.units == Units::Millimeters ? 4 : 1);
  dw->sectionEnd();
  dw->sectionEntities();

  dxf.writeComment(*dw,
                   QString("ARM1: %1 %2")
                       .arg(mmToUnits(settings.arm1_length, settings.units))
                       .arg(unitsToString(settings.units))
                       .toStdString());
  dxf.writeComment(*dw,
                   QString("ARM2: %1 %2")
                       .arg(mmToUnits(settings.arm2_length, settings.units))
                       .arg(unitsToString(settings.units))
                       .toStdString());
  dxf.writePoint(*dw, DL_PointData(0.0, 0.0, 0.0),
                 DL_Attributes("0", 256, -1, "BYLAYER", 1.0));

  for (QGraphicsItem *item : scene->items()) {
    if (auto fm = dynamic_cast<GraphicsItems *>(item)) {
      fm->export_dxf(dxf, *dw, settings.units);
    }
  }

  dw->sectionEnd();
  dxf.writeObjects(*dw);
  dw->dxfEOF();
  dw->close();
  delete dw;
}

void MainWindow::on_actionset_zero_triggered() {

  auto s = settingsManager_->currentSettings();
  s.alfa_offset = appManager()->getAlfa();
  s.beta_offset = appManager()->getBeta();
  settingsManager_->updateSettings(s);
}

void MainWindow::on_actionAuto_triggered() {
  if (appManager()->getContiMode() == ContiMode::SinglePoint) {
    appManager()->setContiMode(ContiMode::Continous);
  } else {
    appManager()->setContiMode(ContiMode::SinglePoint);
  }
}

void MainWindow::handleError(QSerialPort::SerialPortError error) {
  if (error == QSerialPort::ResourceError) {
    QMessageBox::critical(this, tr("Critical Error"),
                          serialDevice->errorString());
    // closeSerialPort();
  }
}

void MainWindow::on_actionNew_file_triggered() {
  Settings settings =
      appManager()->settingsManager()->currentSettings(); // kopie

  qDebug() << "modified" << settings.document_modified;
  if (settings.document_modified == true) {
    int ret = QMessageBox::warning(this, tr("Digitizer"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard |
                                       QMessageBox::Cancel,
                                   QMessageBox::Save);
    qDebug() << "return " << ret;
    if (ret == QMessageBox::Save) {
      on_actionSave_dxf_triggered();
    }
    if (ret == QMessageBox::Discard or
        (ret == QMessageBox::Save and settings.document_saved == true)) {
      appManager()->clearShapeManager();
      settings.document_modified = false;
      ui->actionSave_dxf->setEnabled(false);
    }
  } else {
    appManager()->clearShapeManager();
    settings.document_modified = false;
    ui->actionSave_dxf->setEnabled(false);
  }
}

void MainWindow::on_actionInfo_triggered() {
  info = new InfoDialog(appManager(), this);
  //    recalculate_info();  pocitala pocet prvku
  info->show();
}

void saveScene(QGraphicsScene *scene, const QString &filename) {
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  QTextStream out(&file);
  for (QGraphicsItem *item : scene->items()) {
    GraphicsItems *myItem = dynamic_cast<GraphicsItems *>(item);
    if (myItem) {
      myItem->save(out);
    }
  }
  file.close();
}

void MainWindow::on_actionSave_file_data_triggered() {
  QDateTime date = QDateTime::currentDateTime();
  QString formattedTime = date.toString("yyyy-MM-dd-hh-mm-ss");
  Settings settings =
      appManager()->settingsManager()->currentSettings(); // kopie
  QString save_file = QFileDialog::getSaveFileName(
      this, tr("Save data"),
      settings.directory_save_data + "/" + formattedTime + ".dig",
      "DIGITIZER files (*.dig)");
  if (save_file.isEmpty())
    return;
  QDir d = QFileInfo(save_file).absoluteDir();
  settings.directory_save_data = d.absolutePath();
  appManager()->settingsManager()->updateSettings(settings); // ulozit cestu
  // qDebug() << "Vybraná cesta:" << save_file;
  //  Volání funkce pro uložení dat ze scény
  saveScene(scene,
            save_file); // nebo ui->graphicsView->scene(), podle tvého projektu
}

static void loadScene(QGraphicsScene *scene, const QString &filename,
                      AppManager *app) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    if (line.isEmpty())
      continue;
    QStringList parts = line.split(' ', Qt::SkipEmptyParts);
    if (parts.size() == 0)
      continue;
    QString type = parts[0];
    if (type == "POINT" && parts.size() >= 5) {
      mypoint *p = new mypoint();
      p->setPos(parts[1].toDouble(), parts[2].toDouble());
      p->alfa = parts[3].toDouble();
      p->beta = parts[4].toDouble();
      scene->addItem(p);
    } else if (type == "POLYLINE" && parts.size() >= 2) {
      mypolyline *pl = new mypolyline(app);
      ;
      int n = parts[1].toInt();
      if (parts.size() >= 2 + n * 2) {
        for (int i = 0; i < n; i++) {
          double x = parts[2 + i * 2].toDouble();
          double y = parts[3 + i * 2].toDouble();
          pl->mypolygon->append(QPointF(x, y));
        }
        scene->addItem(pl);
      } else {
        delete pl; // ochrana před chybou formátu
      }
    } else if (type == "CIRCLE" && parts.size() >= 4) {
      double x = parts[1].toDouble();
      double y = parts[2].toDouble();
      double r = parts[3].toDouble();
      qDebug() << "Loading CIRCLE with center:" << x << y << "and radius:" << r;
      mycircle *c = new mycircle();
      c->center = QPointF(parts[1].toDouble(), parts[2].toDouble());
      c->radius = parts[3].toDouble();
      c->finished = 1;
      scene->addItem(c);
    }
  }
  file.close();
}

void MainWindow::on_actionLoad_file_data_triggered() {
  auto settings = settingsManager_->currentSettings();
  qDebug() << "load add " << settings.directory_save_data;
  QString filename = QFileDialog::getOpenFileName(this, tr("Načíst data"),
                                                  settings.directory_save_data,
                                                  "DIG files (*.dig)");
  if (filename.isEmpty())
    return;
  QDir d = QFileInfo(filename).absoluteDir();
  qDebug() << "dir=" << d.absolutePath();
  settings.directory_save_data = d.absolutePath();
  settingsManager_->updateSettings(settings); // ulozit do reg
  qDebug() << "Načítám soubor:" << filename;
  // Vymaž staré položky, pokud je třeba
  scene->clear(); // nebo ui->graphicsView->scene()->clear();
  // Načtení dat
  loadScene(scene, filename, appManager());
  Zoom_All();
  qDebug() << "prekresluji";
}

void MainWindow::handleCalibrateButtonClicked(bool checked) {
  qDebug() << "Stav tlačítka z CalibrateWindow: " << checked;
  if (checked) {
    qDebug() << "hura" << checked;
    ui->actionAuto->setChecked(true);
  } else {
    qDebug() << "nehura" << checked;
    ui->actionAuto->setChecked(false);
  }
}

void MainWindow::updateArms(double Arm1Angle, double Arm2Angle,
                            QPointF endPointArm1, QPointF endPointArm2) {
  // Sem napiš logiku, která aktualizuje vykreslení ramen
  // např. aktualizace scénického prvku, repaint(), atd.

  arm1->setRotation(Arm1Angle); //*180/M_PI);
  arm2->setPos(endPointArm1);
  arm2->setRotation(Arm2Angle); //*180/M_PI);

  QString s = "alfa " + QString::number(Arm1Angle, 'f', 4) + " deg";
  ui->enc1value->setText(s);
  s = "beta " + QString::number(Arm2Angle, 'f', 4) + " deg";
  ui->enc2value->setText(s);
  const auto &st = settingsManager_->currentSettings();
  s = QString::number(mmToUnits(endPointArm1.x(), st.units), 'f', 8);
  ui->position1x->setText(s);
  s = QString::number(mmToUnits(endPointArm1.y(), st.units), 'f', 8);
  ui->position1y->setText(s);
  s = QString::number(mmToUnits(endPointArm2.x(), st.units), 'f', 8);
  ui->position2x->setText(s);
  s = QString::number(mmToUnits(endPointArm2.y(), st.units), 'f', 8);
  ui->position2y->setText(s);
  lastEndArm2_ = endPointArm2; // ⬅ uložit pro Zoom_Dynamic()

  if (auto *pl = dynamic_cast<mypolyline *>(appManager()->currentShape())) {
    pl->updatePreview();
  }
}

void MainWindow::updateUiForMode(AddPointMode mode) {
  qDebug() << "update Ui for mode " << appManager()->modeAddPointToString(mode);
  ;
  // Nejdřív vše zakázat (výchozí stav)
  /*
          ui->actionAdd_circle->setEnabled(false);
          ui->actionAdd_polyline->setEnabled(false);
          ui->actionMeasure->setEnabled(false);
          ui->actionCalibrate->setEnabled(false);

          //-----vypnout triggered


          // Aktivuj jen odpovídající tlačítko
          switch (mode) {

          case AddPointMode::Circle:
              ui->actionAdd_circle->setEnabled(true);
              break;
          case AddPointMode::Polyline:
              ui->actionAdd_polyline->setEnabled(true);
              break;
          case AddPointMode::Measure:
              ui->actionMeasure->setEnabled(true);
              break;
          case AddPointMode::Calibrate:
              ui->actionCalibrate->setEnabled(true);
              ui->actionCalibrate->setChecked(true);
              break;
          case AddPointMode::None:
              ui->actionAdd_polyline->setChecked(false);
              ui->actionAdd_polyline->setEnabled(true);
              ui->actionAdd_circle->setChecked(false);
              ui->actionAdd_circle->setEnabled(true);
              ui->actionMeasure->setChecked(false);
              ui->actionMeasure->setEnabled(true);
              ui->actionCalibrate->setChecked(false);
              ui->actionCalibrate->setEnabled(true);

              break;
          default:
              // vše zůstává zakázané
              break;
          }
  */
}

void MainWindow::on_actionAdd_polyline_toggled(bool mode) {
  qDebug() << "on_actionAdd_polyline_toggled " << mode;
  if (mode) {
    appManager()->setAddPointMode(AddPointMode::Polyline);
  } else {
    appManager()->setAddPointMode(AddPointMode::None);
  }

  ui->actionAdd_polyline->setText(mode ? tr("End polyline")
                                       : tr("Add polyline"));
  QKeySequence seq =
      appManager()->settingsManager()->currentSettings().shortcuts.map.value(
          QStringLiteral("action.polyline"));
  QString shortcut = seq.toString(QKeySequence::PortableText);
  ui->actionAdd_polyline->setToolTip(
      mode ? tr("konec polyline (%1)").arg(shortcut)
           : tr("Přidat polyline (%1)").arg(shortcut));
}

void MainWindow::on_actionAdd_circle_toggled(bool arg1) {
  qDebug() << "on_actionAdd_circle_toggled " << arg1;
  if (arg1) {
    appManager()->setAddPointMode(AddPointMode::Circle);
  } else {
    appManager()->setAddPointMode(AddPointMode::None);
  }
  ui->actionAdd_circle->setText(arg1 ? tr("End circle") : tr("Add circle"));
}

void MainWindow::on_actionMeasure_toggled(bool on) {

  appManager()->setAddPointMode(on ? AddPointMode::Measure
                                   : AddPointMode::None);
}

void MainWindow::on_actionadd_point_triggered() {
  //qDebug() << "add point trigered ui";
  appManager()->addpointfrommainwindow();
}

void MainWindow::on_actionSetup_triggered(bool /*checked*/) {
  qDebug() << "on_actionSetup_triggered";
  if (!appManager())
    return;

  auto *sm = appManager()->settingsManager(); // <- přístup přes AppManager
  if (!sm)
    return;

  qDebug() << "on_actionSetup_triggered";
  SettingsDialog dlg(this, sm, appManager()->serialManager());
  connect(&dlg, &SettingsDialog::languageChanged, this,
          &MainWindow::onLanguageChanged);
  dlg.setSettings(sm->currentSettings()); // KOPIE do dialogu

  if (dlg.exec() == QDialog::Accepted) {
    qDebug() << "ACCEPT";
    sm->updateSettings(dlg.result()); // commit uvnitř SettingsManageru
  }
}

void MainWindow::onLanguageChanged(const QString &language) {
  qDebug() << "aaaaaaaaa";
  qApp->removeTranslator(&translator);
  qApp->removeTranslator(&guitranslator);

  const QString app =
      QStringLiteral(":/translations/%1.qm").arg(language.toLower());

  const QString qt =
      QStringLiteral(":/translations/qtbase_%1.qm").arg(language.toLower());
  qDebug() << " app= " << app + "         qt= " + qt;
  if (translator.load(app)) {
    qDebug() << "load app";
    qApp->installTranslator(&translator);
  }
  if (guitranslator.load(qt))
    qApp->installTranslator(&guitranslator);

  ui->retranslateUi(this);
  setWindowTitle(tr("Digitizer"));
}

void MainWindow::on_actionConnect_triggered() {}

void MainWindow::onSettingsChanged(const Settings &s) {
    qDebug()<<"settings changed color " << s.arms_color;
  // aktualizuj délky ramen podle uloženého nastavení
  QLineF l = arm1->line();    // aktuální čára (uchová P1 i úhel)
  l.setLength(s.arm1_length); // změní jen délku
  arm1->setLine(l);

  if (arm2) {
    QLineF l2 = arm2->line();    // aktuální čára (uchová P1 i úhel)
    l2.setLength(s.arm2_length); // změní jen délku
    arm2->setLine(l2);
  }

  // přiřaď klávesové zkratky k akcím
  ui->actionNew_file->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.newfile")));
  ui->actionadd_point->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.addPoint")));
  ui->actionAdd_polyline->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.polyline")));
  ui->actionAdd_circle->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.circle")));
  ui->actionMeasure->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.measure")));
  ui->actionAuto->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.continous")));
  ui->actionDelete_last_point->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.back")));
  if (zoomShortcut_)
    zoomShortcut_->setKey(s.shortcuts.map.value(QStringLiteral("action.zoom")));
  ui->actionSave_dxf->setShortcut(
      s.shortcuts.map.value(QStringLiteral("action.exportdxf")));
    // zmena velikosti sceny
  scene->setSceneRect(QRect(-1.05*(s.arm1_length+s.arm2_length),-1.05*(s.arm1_length+s.arm2_length),2.1*(s.arm1_length+s.arm2_length),2.1*(s.arm1_length+s.arm2_length)));
    //zmen barvu ramen
  s.arms_pen->setColor(s.arms_color);
  arm1->setPen(*s.arms_pen);
  arm2->setPen(*s.arms_pen);
  base->setPen(*s.arms_pen);

  onAddPointModeChanged(appManager()->getAddPointMode());
}

void MainWindow::onContiModeChanged(ContiMode mode) {
  const bool on_contimode = (mode == ContiMode::Continous);
  // mode continous
  ui->actionAuto->setIcon(QIcon(on_contimode
                                    ? QStringLiteral(":/pic/auto.png")
                                    : QStringLiteral(":/pic/manual.png")));
  QKeySequence seq =
      appManager()->settingsManager()->currentSettings().shortcuts.map.value(
          QStringLiteral("action.continous"));
  ui->actionAuto->setToolTip(
      on_contimode
          ? tr("End continous (%1)")
                .arg(seq.toString(QKeySequence::PortableText))
          : tr("Continous (%1)").arg(seq.toString(QKeySequence::PortableText)));
}

void MainWindow::onAddPointModeChanged(AddPointMode mode) {
  qDebug() << " onAddPointModeChanged"
           << appManager()->modeAddPointToString(mode);
  const bool on_measure = (mode == AddPointMode::Measure);
  const bool on_polyline = (mode == AddPointMode::Polyline);
  const bool on_circle = (mode == AddPointMode::Circle);
  const bool on_calibrate = (mode == AddPointMode::Calibrate);
  const bool on_none = (mode == AddPointMode::None);

  // UI: update akce
  ui->actionAdd_circle->setEnabled(false);
  ui->actionAdd_polyline->setEnabled(false);
  ui->actionMeasure->setEnabled(false);
  ui->actionCalibrate->setEnabled(false);

  {
    // polyline
    QSignalBlocker block_action_polyline(ui->actionAdd_polyline);
    // ui->actionAdd_polyline->setChecked(on_polyline);
    ui->actionAdd_polyline->setEnabled(on_polyline);
    ui->actionAdd_polyline->setText(on_polyline ? tr("End polyline")
                                                : tr("Add polyline"));
    QKeySequence seq =
        appManager()->settingsManager()->currentSettings().shortcuts.map.value(
            QStringLiteral("action.polyline"));
    QString shortcut = seq.toString(QKeySequence::PortableText);
    ui->actionAdd_polyline->setToolTip(
        on_polyline ? tr("konec polyline (%1)").arg(shortcut)
                    : tr("Přidat polyline (%1)").arg(shortcut));

    // measure
    QSignalBlocker block_action_measure(ui->actionMeasure);
    ui->actionMeasure->setChecked(on_measure);
    ui->actionMeasure->setText(on_measure ? tr("Konec měření") : tr("Měření"));
    seq =
        appManager()->settingsManager()->currentSettings().shortcuts.map.value(
            QStringLiteral("action.measure"));
    ui->actionMeasure->setToolTip(
        on_measure
            ? tr("Konec měření (%1)")
                  .arg(seq.toString(QKeySequence::PortableText))
            : tr("Měření (%1)").arg(seq.toString(QKeySequence::PortableText)));

    // circle
    ui->actionAdd_circle->setEnabled(on_circle);
    ui->actionAdd_circle->setText(on_circle ? tr("End circle")
                                            : tr("Add circle"));
    seq = appManager()->settingsManager()->currentSettings().shortcuts.map.value(
            QStringLiteral("action.circle"));
    ui->actionAdd_circle->setToolTip(
        on_measure
            ? tr("Konec circle (%1)")
                  .arg(seq.toString(QKeySequence::PortableText))
            : tr("Circle (%1)").arg(seq.toString(QKeySequence::PortableText)));

    // Point
    seq =
        appManager()->settingsManager()->currentSettings().shortcuts.map.value(
            QStringLiteral("action.addPoint"));
    ui->actionadd_point->setToolTip(
        tr("Point (%1)").arg(seq.toString(QKeySequence::PortableText)));

    // new file
    seq =  appManager()->settingsManager()->currentSettings().shortcuts.map.value(
            QStringLiteral("action.newfile"));
    ui->actionNew_file->setToolTip(
        tr("New file (%1)").arg(seq.toString(QKeySequence::PortableText)));

    //zoom
    seq =  appManager()->settingsManager()->currentSettings().shortcuts.map.value(
            QStringLiteral("action.zoom"));
    actionZoom_All->setToolTip(
        tr("Zoom All (%1)").arg(seq.toString(QKeySequence::PortableText)));
    actionZoom_Dynamic->setToolTip(
        tr("Zoom All (%1)").arg(seq.toString(QKeySequence::PortableText)));
    actionZoom_User->setToolTip(
        tr("Zoom All (%1)").arg(seq.toString(QKeySequence::PortableText)));

    //BAck
    seq =  appManager()->settingsManager()->currentSettings().shortcuts.map.value(
            QStringLiteral("action.back"));
    ui->actionDelete_last_point->setToolTip(
        tr("Delete (%1)").arg(seq.toString(QKeySequence::PortableText)));

  }

  if (on_measure) {
    if (!measure) {
      measure = new MeasureDialog(appManager()->settingsManager(), this);
      measure->setAttribute(Qt::WA_DeleteOnClose, true);
      connect(appManager(), &AppManager::positionChanged, measure,
              &MeasureDialog::updatePosition);
      connect(appManager(), &AppManager::measureToggled, measure,
              &MeasureDialog::toggleMode);
      // Jakmile se dialog zavře (OK/Cancel/křížek), vrať režim do None
      connect(measure, &QDialog::finished, this, [this](int) {
        // Sync režimu (idempotentní)
        if (appManager()->getAddPointMode() == AddPointMode::Measure) {
          appManager()->setAddPointMode(AddPointMode::None);
        }
        // UI – odškrtnout a vrátit text; nevyvolá rekurzivně toggled
        QSignalBlocker b(ui->actionMeasure);
        ui->actionMeasure->setChecked(false);
        ui->actionMeasure->setText(tr("Měření"));
        // ukazatel už neplatí (dialog se smaže díky WA_DeleteOnClose)
        measure = nullptr;
      });
    }
    measure->show();
    measure->raise();
    measure->activateWindow();
  } else {
    if (measure) {
      auto *dlg = measure;
      measure = nullptr; // odpoj ukazatel hned
      dlg->close();      // vyvolá finished() → zbytek proběhne v lambda výše
      // žádné deleteLater(); dialog se smaže sám (WA_DeleteOnClose)
    }
  }

  if (on_calibrate) {
    if (!calibrate) {
      calibrate = new CalibrateWindow(appManager(), this);
      connect(calibrate, &CalibrateWindow::button_calibrate_clicked, this,
              &MainWindow::handleCalibrateButtonClicked);
      connect(calibrate, &QObject::destroyed, this, [this](QObject *) {
        calibrate = nullptr;
        appManager()->setAddPointMode(AddPointMode::None);
      });
      const auto &s = settingsManager_->currentSettings();
      calibrate->set_arms(s.arm1_length, s.arm2_length);
    }
    calibrate->show();
  } else if (calibrate) {
    calibrate->close();
    calibrate = nullptr;
  }
  if (on_polyline) {
  }

  if (on_circle) {
  }

  if (on_none) {
    ui->actionAdd_polyline->setEnabled(true);
    ui->actionAdd_circle->setEnabled(true);
    ui->actionMeasure->setEnabled(true);
  }
}
