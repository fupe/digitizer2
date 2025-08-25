#include <QDebug>
#include <QMessageBox>
#include <QAction>
#include <QGraphicsScene>
#include <QFileDialog>     //save dxf
#include <QDir>
#include <QtMath>
#include "3rdparty/dxflib/src/dl_dxf.h"

#include "appmanager.h"
#include "CustomToolButton.h"
#include "MeasureDialog.h"
#include "calibratewindow.h"
#include "InfoDialog.h"
#include "SettingsDialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(AppManager* app, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , appManager_(app)
{
    Q_ASSERT(appManager_);
    ui->setupUi(this);                         // ← a hned setupUi

    // až teď existují všechny prvky z UI → můžeš nastavovat akce, signály, shortcuty…
    settingsManager_ = appManager_->settingsManager();
    Q_ASSERT(settingsManager_);
    auto modes = new QActionGroup(this);
    modes->setExclusive(true);
    ui->actionAdd_polyline->setCheckable(true);
    ui->actionAdd_circle->setCheckable(true);
    ui->actionMeasure->setCheckable(true);
    ui->actionCalibrate->setCheckable(true);

    modes->addAction(ui->actionAdd_polyline);
    modes->addAction(ui->actionAdd_circle);

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);

    // mapování UI → AppManager
    connect(ui->actionConnect,   &QAction::triggered, this, [this](bool){appManager_->openSerial(); });  // zavolá metodu bez argumentů
    connect(ui->actionDisconnect, &QAction::triggered, this, [this](bool){appManager_->closeSerial();});
    connect(appManager_, &AppManager::serialOpened, this, [this](){  //stav ikony z appmanageru.
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
    });
    connect(appManager_, &AppManager::serialClosed, this, [this](){
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
    });
    connect(ui->actionAdd_polyline, &QAction::toggled, this, [this](bool on){ if (on) appManager()->setAddPointMode(AddPointMode::Polyline); });
    connect(ui->actionAdd_circle,   &QAction::toggled, this, [this](bool on){ if (on) appManager()->setAddPointMode(AddPointMode::Circle);   });
    connect(ui->actionCalibrate,    &QAction::toggled, this, [this](bool on){ if (on) appManager()->setAddPointMode(AddPointMode::Calibrate);});
    connect(appManager(), &AppManager::modeAddPointChanged,this, &MainWindow::onAddPointModeChanged); //reakce na zmenu modu
    // UI → AppManager
        // AppManager → status bab
        connect(appManager_, &AppManager::connectionNotice, this, [this](const QString& msg){
            statusBar()->showMessage(msg, 5000);
        });
        connect(appManager_, &AppManager::serialError, this, [this](const QString& e){
            statusBar()->showMessage(e, 8000);
            qWarning() << "STATUSBAR error:" << e;
        });

        currentSettings_ = appManager_->settingsManager()->currentSettings();
        if (currentSettings_.main_window_position.isValid()) {
            setGeometry(currentSettings_.main_window_position);
        }
    //-----
    connect(settingsManager_, &SettingsManager::settingsChanged,
            this, &MainWindow::onSettingsChanged);
    // inicializace z aktuálních Settings (např. zkratky, stavy checkboxů…)
    onSettingsChanged(settingsManager_->currentSettings()); // nebo jak se u tebe jmenuje getter
    connect(ui->actionExit, &QAction::triggered, this, [this]() {
        qDebug() << "Menu Exit triggered";
        this->close();  // vyvolá closeEvent()
    });
    this->setWindowTitle(tr("Digitizer"));
    //qApp->installEventFilter(this);
    //this->installEventFilter(this);
    initMenu();
    ui->toolBar_2->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    ui->toolBar_2->setIconSize(QSize(50, 50));
    /*graphics*/
    // qDebug() << " scene = new QGraphicsScene(this); = " ;
    scene = new QGraphicsScene(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)

{
    qDebug()<<"MainWindow::closeEvent " << &event  ;
    {
        if (!appManager()) return QWidget::closeEvent(event);
        if (currentSettings_.save_main_window_position_on_exit) {
            currentSettings_.main_window_position = this->geometry();  // QRect jako dřív
            appManager()->settingsManager()->updateSettings(currentSettings_);               // commit (uloží do QSettings přes SettingsManager)
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

void MainWindow::Zoom_Dynamic()
{
    //qDebug() << "dynamic";
    if (actionZoom_Dynamic->isChecked())
    {
        QGraphicsScene* aaa = ui->graphicsView->scene();
            if (!aaa) return;

            QRectF bounds;
            bool hasItem = false;

            for (QGraphicsItem* item : scene->items()) {
                auto* shape = dynamic_cast<GraphicsItems*>(item);
                if (shape) {
                    if (!hasItem) {
                        bounds = shape->sceneBoundingRect();
                        hasItem = true;
                    } else {
                        bounds = bounds.united(shape->sceneBoundingRect());
                    }
                }
            }
            if (hasItem) {
                QRectF endpoitArm(lastEndArm2_, QSizeF(1, 1));  // malý obdélník okolo bodu
                        bounds = bounds.united(endpoitArm);          // přidat do oblasti
                bounds.adjust(-20, -20, 20, 20);  // přidá okraje
                ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
                ui->graphicsView->centerOn(bounds.center());
            } else {
                qDebug() << "Žádné GraphicsItems k zobrazení.";
            }
    }
}

void MainWindow::Zoom_All()
{
    ui->graphicsView->fitInView(QRect(-1.05*(currentSettings_.arm1_length+currentSettings_.arm2_length),-1.05*(currentSettings_.arm1_length+currentSettings_.arm2_length),2.1*(currentSettings_.arm1_length+currentSettings_.arm2_length),2.1*(currentSettings_.arm1_length+currentSettings_.arm2_length)),Qt::KeepAspectRatio);
}

void MainWindow::Zoom_User()
{
    qDebug() << "User defined zoom";
    //actionZoom_Dynamic->setChecked(false);
    //actionZoom_All->setChecked(false);
}

void MainWindow::status_bar_print(QString text,int delay)
{
    statusBar()->showMessage(text,delay);
}

void MainWindow::initActions()
{
      QString  menuStyle(
               "QMenu::item{"
               "background-color: rgb(200, 170, 0);"
               "color: rgb(255, 255, 255);"
               "}"
               "QMenu::item:selected{"
               "background-color: rgb(0, 5, 127);"
               "color: rgb(255, 255, 255);"
               "}"
            );
}

void MainWindow::initMenu()
{
    /*--------pridani cudliku zoom-------*/
    actionZoom_All = new QAction(tr("Zoom All orig"), this);
    actionZoom_Dynamic = new QAction(tr("Zoom Dynamic orig"), this);
    actionZoom_All->setIcon(QIcon(":/pic/zoom_all.png"));
    actionZoom_Dynamic->setIcon(QIcon(":/pic/zoom_select.png"));
    actionZoom_Dynamic->setCheckable(true);
    connect(actionZoom_All,     &QAction::triggered, this, &MainWindow::Zoom_All);
    connect(actionZoom_Dynamic, &QAction::triggered, this, &MainWindow::Zoom_Dynamic);
    ZoomMenu = new QMenu;
    ZoomMenu->addAction(actionZoom_All);
    ZoomMenu->addAction(actionZoom_Dynamic);
    ZoomToolButton = new CustomToolButton;
    ZoomToolButton->setMenu(ZoomMenu);
    ZoomToolButton->setDefaultAction(actionZoom_All);
    ZoomToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ZoomToolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ZoomToolButton->setCheckable(true);
    ui->toolBar_2->addWidget(ZoomToolButton);
    /*-----------konec zoom-------*/
    ZoomMenu->setTitle("zoommenu");
}

void MainWindow::setup_scene()
{
    qDebug() << "scene = " ;
    scene->setSceneRect(QRect(-1.05*(currentSettings_.arm1_length+currentSettings_.arm2_length),-1.05*(currentSettings_.arm1_length+currentSettings_.arm2_length),2.1*(currentSettings_.arm1_length+currentSettings_.arm2_length),2.1*(currentSettings_.arm1_length+currentSettings_.arm2_length)));
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
    ui->graphicsView->setDragMode(GraphicsView::RubberBandDrag);
    ui->graphicsView->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
//    ui->graphicsView->setDragMode(selectModeButton->isChecked() ? QGraphicsView::RubberBandDrag : QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    arm1  = scene->addLine(0,0,currentSettings_.arm1_length,0,*currentSettings_.arms_pen);
    arm2  = scene->addLine(0,0,currentSettings_.arm2_length,0,*currentSettings_.arms_pen);
    base = scene->addEllipse(-25,-25, 50, 50,*currentSettings_.arms_pen);
    arm2->setPos(currentSettings_.arm1_length,0);
    // arm1->hide();
    // arm2->hide();
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(),Qt::KeepAspectRatio);
    //ui->graphicsView->show();
//    qDebug() << " konecscene = " ;
}

void MainWindow::on_actionDelete_last_point_triggered()
{

}

void MainWindow::sceneModified(const QList<QRectF> & region)
{
    Q_UNUSED(region);
}

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent)

{
    zoom_pen.setColor(Qt::black);
    zoom_pen.setWidth(2);
}


void GraphicsView::wheelEvent(QWheelEvent *e)
{
    qDebug() << "wheel" << e ;
   // if (e->modifiers() ) //& Qt::ControlModifier)
    //{
         const ViewportAnchor anchor = transformationAnchor();
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        int angle = e->angleDelta().y();
        qDebug() << "wheel + control event " << e << "delta " << e->angleDelta().y();
        qreal factor;
        if (angle > 0)
        {
            factor = 1.1;
        } else {
            factor = 0.9;
        }
        scale(factor, factor);
        setTransformationAnchor(anchor);
        qDebug() << "anchor " << anchor ;
        qDebug() << "transform " << transform() ;
    //}
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
        qDebug() << "right button press" << pan_moving <<  " pos " << event->x() << event->y();
        emit text_to_status_bar("Pan window",5000);
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
        qDebug() << "middle buton press  - zoom_originalX " << zoom_originalX << "zoom_originalY" <<zoom_originalY ;
        //QGraphicsView::mousePressEvent (event);
        setCursor(Qt::CrossCursor);
        zoom_rect = this->scene()->addRect(zoom_originalX,zoom_originalY, 5 ,5,zoom_pen);

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
        qDebug() << "right buton release" << pan_moving <<  " pos " << event->x() << event->y();

    }
    else if  (event->button() == Qt::MiddleButton)
    {
        qDebug() << "middle buton release";
        //qDebug() << "mapovani release "  << mapToScene(event->pos());
        this->scene()->removeItem(zoom_rect);
        zoom_moving = false;
        //this->fitInView(QRect(zoom_originalX,zoom_originalY,mapToScene(event->pos()).x()-zoom_originalX ,mapToScene(event->pos()).y()-zoom_originalY),Qt::KeepAspectRatio);
        int startx,starty,wide,height;
        if (zoom_originalX >  mapToScene(event->pos()).x()) startx = mapToScene(event->pos()).x() ;
        else startx = zoom_originalX ;
        wide   = abs(mapToScene(event->pos()).x()-zoom_originalX);
        if (zoom_originalY >  mapToScene(event->pos()).y())   starty = mapToScene(event->pos()).y() ;
        else    starty = zoom_originalY ;
        height   = abs(mapToScene(event->pos()).y()-zoom_originalY);
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
            qDebug() << "mouse move" << pan_moving <<  " pos " << translation.x() << translation.y();
            setTransformationAnchor(anchor);
        }
    else if (zoom_moving)
        {
            qDebug() << "mouse move zooming" << zoom_moving << "pos: " << mapToScene(event->pos()) ;
            this->scene()->removeItem(zoom_rect);
            int startx,starty,wide,height;
            if (zoom_originalX >  mapToScene(event->pos()).x()) startx = mapToScene(event->pos()).x() ;
            else startx = zoom_originalX ;
            wide   = abs(mapToScene(event->pos()).x()-zoom_originalX);
            if (zoom_originalY >  mapToScene(event->pos()).y())   starty = mapToScene(event->pos()).y() ;
            else    starty = zoom_originalY ;
            height   = abs(mapToScene(event->pos()).y()-zoom_originalY);
            zoom_rect = this->scene()->addRect(startx,starty, wide ,height,zoom_pen);
            update();
        }
    else
        {

            QGraphicsView::mouseMoveEvent(event);

        }
}*/


void MainWindow::on_actionSave_dxf_triggered()
{
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("yyyy-MM-dd-hh-mm-ss");
    QString export_file = QFileDialog::getSaveFileName(this, tr("Export DXF"), currentSettings_.directory_save_dxf + "/" + formattedTime, "DXF files (*.dxf)");
    if (export_file.isEmpty()) {
        return;
    }
    QDir d = QFileInfo(export_file).absoluteDir();
    currentSettings_.directory_save_dxf = d.absolutePath();
    appManager()->settingsManager()->updateSettings(currentSettings_);

    DL_Dxf dxf;
    QByteArray filenameArray = export_file.toLocal8Bit();
    DL_WriterA* dw = dxf.out(filenameArray.constData(), DL_Codes::AC1009);
    if (dw == nullptr) {
        qDebug() << "error";
        return;
    }

    dxf.writeHeader(*dw);
    dw->sectionEnd();
    dw->sectionEntities();

    dxf.writeComment(*dw, QString("ARM1: %1").arg(currentSettings_.arm1_length).toStdString());
    dxf.writeComment(*dw, QString("ARM2: %1").arg(currentSettings_.arm2_length).toStdString());
    dxf.writePoint(*dw, DL_PointData(0.0, 0.0, 0.0), DL_Attributes("0", 256, -1, "BYLAYER", 1.0));

    for (QGraphicsItem* item : scene->items()) {
        if (auto fm = dynamic_cast<GraphicsItems*>(item)) {
            fm->export_dxf(dxf, *dw);
        }
    }

    dw->sectionEnd();
    dxf.writeObjects(*dw);
    dw->dxfEOF();
    dw->close();
    delete dw;
}

void MainWindow::on_actionset_zero_triggered()
{


    currentSettings_.alfa_offset = appManager()->getAlfa();
    currentSettings_.beta_offset = appManager()->getBeta();
    //currentSettings_.save_Settings();
}

void MainWindow::on_actionAuto_triggered()
{
    if (ui->actionAuto->isChecked())
        {
        ui->actionAuto->setIcon(QIcon(QStringLiteral(":/pic/auto.png")));
        appManager()->setContiMode(ContiMode::Continous);
        }
    else
        {
        ui->actionAuto->setIcon(QIcon(QStringLiteral(":/pic/manual.png")));
        appManager()->setContiMode(ContiMode::SinglePoint);
        }

}


void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serialDevice->errorString());
        //closeSerialPort();
    }
}

void MainWindow::on_actionNew_file_triggered()
{
    Settings settings = appManager()->settingsManager()->currentSettings(); // kopie

    qDebug() << "modified"   << settings.document_modified ;
    if (settings.document_modified==true)
    {
        int ret = QMessageBox::warning(this, tr("Digitizer"),tr("The document has been modified.\n""Do you want to save your changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,QMessageBox::Save);
        qDebug() << "return " << ret;
        if (ret==QMessageBox::Save)
        {
            on_actionSave_dxf_triggered();
        }
        if (ret==QMessageBox::Discard or (ret==QMessageBox::Save and settings.document_saved==true))
        {
            for (QGraphicsItem* item : scene->items()) {
                    if (auto fm = dynamic_cast<GraphicsItems*>(item)) {
                        if (fm->scene()) {
                            fm->scene()->removeItem(fm);
                        }
                    }
                }
            settings.document_modified=false;
            ui->actionSave_dxf->setEnabled(false);
        }
    }
    else
    {
        for (QGraphicsItem* item : scene->items()) {
                if (auto fm = dynamic_cast<GraphicsItems*>(item)) {
                    if (fm->scene()) {
                        fm->scene()->removeItem(fm);
                    }
                }
            }
    settings.document_modified=false;
    ui->actionSave_dxf->setEnabled(false);
    }

}


void MainWindow::on_actionInfo_triggered()
{
    info = new InfoDialog(this);
//    recalculate_info();  pocitala pocet prvku
    info->show();
}

void saveScene(QGraphicsScene *scene, const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    for (QGraphicsItem *item : scene->items())
    {
        GraphicsItems *myItem = dynamic_cast<GraphicsItems*>(item);
        if (myItem)
        {
            myItem->save(out);
        }
    }
    file.close();
}

void MainWindow::on_actionSave_file_data_triggered()
{
        QDateTime date = QDateTime::currentDateTime();
        QString formattedTime = date.toString("yyyy-MM-dd-hh-mm-ss");
        Settings settings = appManager()->settingsManager()->currentSettings(); // kopie
        QString save_file = QFileDialog::getSaveFileName(
            this,
            tr("Save data"),
            settings.directory_save_data + "/" + formattedTime + ".dig",
            "DIGITIZER files (*.dig)"
        );
        if (save_file.isEmpty())
            return;
        QDir d = QFileInfo(save_file).absoluteDir();
        settings.directory_save_data = d.absolutePath();
        appManager()->settingsManager()->updateSettings(settings);   // ulozit cestu
        //qDebug() << "Vybraná cesta:" << save_file;
        // Volání funkce pro uložení dat ze scény
        saveScene(scene, save_file);  // nebo ui->graphicsView->scene(), podle tvého projektu
}

void loadScene(QGraphicsScene *scene, const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() == 0)
            continue;
        QString type = parts[0];
        if (type == "POINT" && parts.size() >= 5)
        {
            mypoint *p = new mypoint();
            p->setPos(parts[1].toDouble(), parts[2].toDouble());
            p->alfa = parts[3].toDouble();
            p->beta = parts[4].toDouble();
            scene->addItem(p);
        }
        else if (type == "POLYLINE" && parts.size() >= 2)
        {
            mypolyline *pl = new mypolyline();
            int n = parts[1].toInt();
            if (parts.size() >= 2 + n*2)
            {
                for (int i = 0; i < n; i++)
                {
                    double x = parts[2 + i*2].toDouble();
                    double y = parts[3 + i*2].toDouble();
                    pl->mypolygon->append(QPointF(x, y));
                }
                scene->addItem(pl);
            }
            else
            {
                delete pl;  // ochrana před chybou formátu
            }
        }
        else if (type == "CIRCLE" && parts.size() >= 4)
        {
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

void MainWindow::on_actionLoad_file_data_triggered()
{
    qDebug() << "load add " << currentSettings_.directory_save_data;
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Načíst data"),
        currentSettings_.directory_save_data,
        "DIG files (*.dig)"
    );
    if (filename.isEmpty())
        return;
    QDir d = QFileInfo(filename).absoluteDir();
    qDebug() << "dir=" << d.absolutePath();
    currentSettings_.directory_save_data = d.absolutePath();
    appManager()->settingsManager()->updateSettings(currentSettings_); // ulozit do reg
    qDebug() << "Načítám soubor:" << filename;
    // Vymaž staré položky, pokud je třeba
    scene->clear();  // nebo ui->graphicsView->scene()->clear();
    // Načtení dat
    loadScene(scene, filename);
    Zoom_All();
    qDebug() << "prekresluji" ;
}



void MainWindow::handleCalibrateButtonClicked(bool checked)
{
    qDebug() << "Stav tlačítka z CalibrateWindow: " << checked;
    if (checked) {
        qDebug() << "hura" << checked;
        ui->actionAuto->setChecked(true);
    } else {
        qDebug() << "nehura" << checked;
        ui->actionAuto->setChecked(false);

    }

 }

void MainWindow::updateArms(double Arm1Angle, double Arm2Angle,QPointF endPointArm1,QPointF endPointArm2)
{
    // Sem napiš logiku, která aktualizuje vykreslení ramen
    // např. aktualizace scénického prvku, repaint(), atd.



    arm1->setRotation(Arm1Angle); //*180/M_PI);
    arm2->setPos(endPointArm1);
    arm2->setRotation(Arm2Angle);//*180/M_PI);

    QString s = "alfa " +QString::number(Arm1Angle, 'f', 4) +" deg";
    ui->enc1value->setText(s);
    s = "beta " +QString::number(Arm2Angle, 'f', 4) + " deg";
    ui->enc2value->setText(s);
    s = QString::number(endPointArm1.x()/currentSettings_.units_scale, 'f', 8);
    ui->position1x->setText(s);
    s = QString::number(endPointArm1.y()/currentSettings_.units_scale, 'f', 8);
    ui->position1y->setText(s);
    s = QString::number(endPointArm2.x()/currentSettings_.units_scale, 'f', 8);
    ui->position2x->setText(s);
    s = QString::number(endPointArm2.y()/currentSettings_.units_scale, 'f', 8);
    ui->position2y->setText(s);
    lastEndArm2_ = endPointArm2;   // ⬅ uložit pro Zoom_Dynamic()

}

void MainWindow::updateUiForMode(AddPointMode mode) {
    qDebug() << "update Ui for mode " <<appManager()->modeAddPointToString(mode); ;
    // Nejdřív vše zakázat (výchozí stav)

        ui->actionAdd_circle->setEnabled(false);
        ui->actionAdd_polyline->setEnabled(false);
        ui->actionMeasure->setEnabled(false);

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
        case AddPointMode::None:
            ui->actionAdd_polyline->setChecked(false);
            ui->actionAdd_polyline->setEnabled(true);
            ui->actionAdd_circle->setChecked(false);
            ui->actionAdd_circle->setEnabled(true);
            ui->actionMeasure->setChecked(false);
            ui->actionMeasure->setEnabled(true);
            ui->actionCalibrate->setChecked(false);
            //ui->actionCalibrate->setEnabled(true);

            break;
        default:
            // vše zůstává zakázané
            break;
        }

}




void MainWindow::on_actionAdd_polyline_toggled(bool mode)
{
    qDebug() << "on_actionAdd_polyline_toggled " << mode ;
    if (mode)
    {
        appManager()->setAddPointMode(AddPointMode::Polyline);
    }
    else
    {
        appManager()->setAddPointMode(AddPointMode::None);
    }


        ui->actionAdd_polyline->setText(mode ? tr("End polyline") : tr("Add polyline"));
        QKeySequence seq = appManager()->settingsManager()->currentSettings().shortcuts.map.value(QStringLiteral("action.polyline"));
        QString shortcut = seq.toString(QKeySequence::PortableText);
        ui->actionAdd_polyline->setToolTip(mode ? tr("konec polyline (%1)").arg(shortcut) : tr("Přidat polyline (%1)").arg(shortcut));

}

void MainWindow::on_actionAdd_circle_toggled(bool arg1)
{
    qDebug() << "on_actionAdd_circle_toggled " << arg1 ;
    if (arg1)
    {
        appManager()->setAddPointMode(AddPointMode::Circle);
    }
    else
    {
        appManager()->setAddPointMode(AddPointMode::None);
    }
    ui->actionAdd_circle->setText(arg1 ? tr("End circle") : tr("Add circle"));
}

void MainWindow::on_actionMeasure_toggled(bool on)
{

     appManager()->setAddPointMode(on ? AddPointMode::Measure : AddPointMode::None);
}

void MainWindow::on_actionCalibrate_toggled(bool arg1)
{


    qDebug() << "on_actionCalibrate_toggled " << arg1 ;
    if ( arg1)
    {
    appManager()->setAddPointMode(AddPointMode::Calibrate);
    calibrate = new CalibrateWindow(this);
    connect(calibrate, &CalibrateWindow::button_calibrate_clicked,this, &MainWindow::handleCalibrateButtonClicked);
    //connect(calibrate, &CalibrateWindow::close_calibrate ,this, &MainWindow::calibrateWindowClosed);
    calibrate->show();
    calibrate->set_arms(currentSettings_.arm1_length,currentSettings_.arm2_length);

    }
    else
    {
        //qDebug() << " none from on_actionCalibrate_toggled" ;
        //appManager()->setAddPointMode(AddPointMode::None);
    }
}

void MainWindow::on_actionadd_point_triggered()
{
    qDebug() << "add point trigered ui";
    appManager()->addpointfrommainwindow();
}

void MainWindow::show_measure_value(double Arm1Angle, double Arm2Angle,QPointF endPointArm1,QPointF endPointArm2)
{


    Q_UNUSED(Arm1Angle);
    Q_UNUSED(Arm2Angle);
    Q_UNUSED(endPointArm1);
    if (!measure) return;
    if  (measure->mode==1)
    {
        double distance;
        distance= double(qSqrt(double(qPow(measure->start_position.x()-endPointArm2.x(),2))+double(qPow(measure->start_position.y()-endPointArm2.y(),2))))/currentSettings_.units_scale;
        measure->set_value(distance);
    }
    else if (measure->mode==0)
    {
        measure->set_value(0);
    }
}



void MainWindow::on_actionSetup_triggered(bool /*checked*/)
{
    qDebug()<<"on_actionSetup_triggered";
    if (!appManager()) return;

    auto* sm = appManager()->settingsManager();   // <- přístup přes AppManager
        if (!sm) return;

    qDebug()<<"on_actionSetup_triggered";
    SettingsDialog dlg(this);
        dlg.setSettings(sm->currentSettings());      // KOPIE do dialogu

        if (dlg.exec() == QDialog::Accepted) {
            qDebug()<<"ACCEPT";
            sm->updateSettings(dlg.result());        // commit uvnitř SettingsManageru
        }
}


void MainWindow::on_actionexport_settings_triggered()
{
        if (!appManager()->settingsManager()) return;
        const QString defDir =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString path = QFileDialog::getSaveFileName(
            this,
            tr("Export settings to JSON"),
            QDir(defDir).filePath("settings.json"),
            tr("JSON files (*.json)")
        );
        if (path.isEmpty()) return;
        QString err;
        if (!appManager()->settingsManager()->exportJson(path, &err)) {
            QMessageBox::warning(this, tr("Export failed"),
                                 tr("Cannot export settings:\n%1").arg(err));
            return;
        }
        statusBar()->showMessage(tr("Settings exported to %1").arg(QDir::toNativeSeparators(path)), 4000);
}

void MainWindow::on_actionConnect_triggered()
{

}


void MainWindow::onSettingsChanged(const Settings& s)
{
currentSettings_ = s ;
}

void MainWindow::onAddPointModeChanged(AddPointMode mode)
{
    qDebug() << " onAddPointModeChanged" << appManager()->modeAddPointToString(mode);
    const bool on_measure = (mode == AddPointMode::Measure);

    // UI: update akce
    {
        QSignalBlocker block_action_measure(ui->actionMeasure);
        ui->actionMeasure->setChecked(on_measure);
        ui->actionMeasure->setText(on_measure ? tr("Konec měření") : tr("Měření"));

        const QKeySequence seq =
            appManager()->settingsManager()->currentSettings().shortcuts.map
                .value(QStringLiteral("action.measure"));
        ui->actionMeasure->setToolTip(on_measure
            ? tr("Konec měření (%1)").arg(seq.toString(QKeySequence::PortableText))
            : tr("Měření (%1)").arg(seq.toString(QKeySequence::PortableText)));
    }

    // Otevřít/zavřít MeasureDialog
    // Otevřít/zavřít MeasureDialog
    if (on_measure) {
        if (!measure) {
            measure = new MeasureDialog(appManager()->settingsManager(), this);
            measure->setAttribute(Qt::WA_DeleteOnClose, true);

            // Jakmile se dialog zavře (OK/Cancel/křížek), vrať režim do None
            connect(measure, &QDialog::finished, this, [this](int){
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
            auto* dlg = measure;
            measure = nullptr;          // odpoj ukazatel hned
            dlg->close();               // vyvolá finished() → zbytek proběhne v lambda výše
            // žádné deleteLater(); dialog se smaže sám (WA_DeleteOnClose)
        }
    }

}


void MainWindow::on_actionimport_settings_triggered()
{
    if (!appManager()->settingsManager()) return;

    const QString defDir =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Import settings from JSON"),
        QDir(defDir).filePath("settings.json"),
        tr("JSON files (*.json)")
    );
    if (path.isEmpty()) return;

    QString err;
    if (!appManager()->settingsManager()->importJson(path, &err)) {
        QMessageBox::warning(this, tr("Import failed"),
                             tr("Cannot import settings:\n%1").arg(err));
        return;
    }

    statusBar()->showMessage(
        tr("Settings imported from %1")
            .arg(QDir::toNativeSeparators(path)),
        4000
    );
}
