#include "GraphicsItems.h"
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPathStroker>
#include <QDebug>
#include <QGraphicsScene>
#include "3rdparty/dxflib/src/dl_dxf.h"
#include <QTextStream>
#include "settings.h"
#include "appmanager.h"

GraphicsItems::GraphicsItems()
{
    //constructor
    pen.setColor(Qt::blue);
    pen.setWidth(3);
    pen.setCosmetic(true);
    setFlag (ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    finished = false;

    // KLÍČOVÉ: vizuální velikost bodu nezávislá na zoomu

}

QRectF GraphicsItems::boundingRect() const
{
    return QRectF (-20,-20,40,40);
}

void GraphicsItems::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void GraphicsItems::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units)
{
    Q_UNUSED(dxf);
    Q_UNUSED(dw);
    Q_UNUSED(units);
    qDebug() << "export mygraphicsitem" << this->pen;
}







void GraphicsItems::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    setCursor(QCursor(Qt::PointingHandCursor)); // opraveno
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphicsItems::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    unsetCursor();
    QGraphicsItem::hoverLeaveEvent(event);
}


mypoint::mypoint()
{
    setFlag (ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    pen.setColor(Qt::red);

}


void mypoint::addPointToShape(const QPointF& point)
{
    qDebug() << "pridavam bod do point " << point ;
}

QRectF mypoint::boundingRect() const
{
    return QRectF (-10,-10,20,20);
}

void mypoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    Q_UNUSED(option);
    Q_UNUSED(widget);
    pen.setWidth(3);
    painter->setPen(pen);
    painter->drawLine (-10,-10,10,10);
    painter->drawLine (10,-10,-10,10);
    //painter->drawPoint(1,1);
    //qDebug()<<"point vykreslen";
}

void mypoint::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units)
{
    qDebug() << "export mypoint" << this->pos();
    dxf.writeComment(dw, QString("alfa: %1").arg(-alfa).toStdString());
    dxf.writeComment(dw, QString("beta: %1").arg(-beta).toStdString());
    double x = mmToUnits(this->pos().x(), units);
    double y = mmToUnits(this->pos().y(), units) * (-1);
    DL_PointData data(x, y, 0.0);
    dxf.writePoint(dw, data, DL_Attributes("0", 256, -1, "BYLAYER", 1.0));
}

void mypoint::save(QTextStream &out)
{
    out << "POINT " << pos().x() << " " << pos().y() << " " << alfa << " " << beta << "\n";
}





mypolyline::mypolyline(AppManager* app)
    : appManager_(app)
{
    setFlag(ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    pen.setCapStyle(Qt::RoundCap);
    mypolygon = new QPolygonF;
    m_boundingRect = QRectF(-20,-20,40,40);
    qDebug() << "konstriuktor polyline";
}

void mypolyline::addPointToShape(const QPointF& point)
{
    qDebug() << "pridavam bod do mypolyline " << point ;
    prepareGeometryChange();
    mypolygon->append(point);
    update();
    //m_boundingRect=QRectF(bounding_min_max());
}

QPainterPath mypolyline::shape() const
{
    QPainterPath path;
    if (mypolygon && mypolygon->size() >1)
    {
        path.moveTo((mypolygon->at(0)));
        for (int i=1;i<mypolygon->size();i++)
        {
            path.lineTo(mypolygon->at(i));
        }
    }
    QPainterPathStroker stroker;
    stroker.setWidth(6);
    return stroker.createStroke(path);
}

QRectF mypolyline::boundingRect() const
{
    if (!mypolygon || mypolygon->isEmpty())
        return QRectF();

    QPolygonF poly = *mypolygon;
    if (!finished && appManager_) {
        poly << appManager_->arm2EndPoint();
    }
    return poly.boundingRect().adjusted(-3, -3, 3, 3);
}

void mypolyline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //qDebug() << "-------------------poly paint------------------------";

    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(pen);
    painter->drawLine (-10,-10,10,10);
    painter->drawLine (10,-10,-10,10);
    painter->drawPolyline(*mypolygon);
    // spojnice posledního a prvního bodu během kreslení
    if (mypolygon->count() > 1 && finished) {
        painter->drawLine(mypolygon->last(), mypolygon->first());
    }

    // pomocná čárkovaná spojnice od prvního bodu k rameni při kreslení
    if (appManager_ && !finished && !mypolygon->isEmpty()) {
        //qDebug() << "-------------------------------------------";
        QPen tempPen = pen;
        tempPen.setStyle(Qt::DashLine);
        tempPen.setWidth(qMax(1, pen.width() - 3));
        painter->setPen(tempPen);
        const QPointF armEnd = appManager_->arm2EndPoint();
        painter->drawLine(mypolygon->first(), armEnd);
        if (mypolygon->count() > 1) {
            tempPen.setWidth(qMax(1, pen.width() - 1));
            painter->setPen(tempPen);
            painter->drawLine(mypolygon->last(), armEnd);
        }
        painter->setPen(pen);
    }

    // zvýraznění posledního segmentu při výběru
  /*  if (mypolygon->count()>1 and isSelected())
    {
        painter->drawLine(mypolygon->at(mypolygon->count()-1),
                          mypolygon->at(mypolygon->count()-2));
        painter->setPen(pen);
    }*/

/*
    QPen temppen=QPen(Qt::green) ;
    temppen.setCapStyle(Qt::RoundCap);
    temppen.setWidth(6);
    painter->setPen(temppen);
    painter->drawRect(m_boundingRect);
    painter->setPen(pen);
*/
    // označení začátku polyline (malý kruh)
    if (!mypolygon->isEmpty()) {
        painter->drawEllipse(mypolygon->first(), 5, 5);
    }

    if (hoveredSegmentIndex >= 0 && isSelected() ) {
            painter->setPen(QPen(Qt::red, 30));
            painter->drawLine(mypolygon->at(hoveredSegmentIndex),
                              mypolygon->at(hoveredSegmentIndex + 1));
        }
    if (selectedSegmentIndex >= 0 && isSelected() ) {
            painter->setPen(QPen(Qt::blue, 50));
            painter->drawLine(mypolygon->at(selectedSegmentIndex),
                              mypolygon->at(selectedSegmentIndex + 1));
        }

}

void mypolyline::updatePreview()
{
    prepareGeometryChange();
    update();
}

void mypolyline::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units)
{
    qDebug() << "export mypolyline" << this->pen;
    for (int i = 0; i < mypolygon->count() - 1; i++) {
        double x1 = mmToUnits(mypolygon->at(i).x() + pos().x(), units);
        double y1 = mmToUnits(mypolygon->at(i).y() + pos().y(), units) * (-1);
        double x2 = mmToUnits(mypolygon->at(i + 1).x() + pos().x(), units);
        double y2 = mmToUnits(mypolygon->at(i + 1).y() + pos().y(), units) * (-1);
        DL_LineData lineData(x1, y1, 0.0, x2, y2, 0.0);
        dxf.writeLine(dw, lineData, DL_Attributes("0", 256, -1, "BYLAYER", 1.0));
    }
}

void mypolyline::save(QTextStream &out)
{
    out << "POLYLINE " << mypolygon->count();
    for (int i = 0; i < mypolygon->count(); i++)
    {
        out << " " << mypolygon->at(i).x() << " " << mypolygon->at(i).y();
    }
    out << "\n";
}




int mypolyline::get_selectedSegmentIndex()
{
    return selectedSegmentIndex;
}
void mypolyline::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QPointF pos = event->pos();  // souřadnice myši v rámci itemu

    selectedSegmentIndex = -1;  // reset

    for (int i = 0; i < mypolygon->size() - 1; ++i) {
        QLineF segment(mypolygon->at(i), mypolygon->at(i + 1));

        QPainterPath path;
        path.moveTo(segment.p1());
        path.lineTo(segment.p2());

        QPainterPathStroker stroker;
        stroker.setWidth(6);
        QPainterPath clickable = stroker.createStroke(path);

        if (clickable.contains(pos)) {
            selectedSegmentIndex = i;
            update();  // překresli pro zvýraznění
            break;
        }
    }

    QGraphicsItem::mousePressEvent(event);
}

void mypolyline::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    setCursor(QCursor(Qt::PointingHandCursor)); // opraveno
    QGraphicsItem::hoverEnterEvent(event);
}

void mypolyline::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    unsetCursor();
    hoveredSegmentIndex = -1;
    QGraphicsItem::hoverLeaveEvent(event);
}



void mypolyline::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF pos = event->pos();  // pozice myši v souřadnicích itemu

    for (int i = 0; i < mypolygon->size() - 1; ++i) {
        QLineF segment(mypolygon->at(i), mypolygon->at(i + 1));

        QPainterPath path;
        path.moveTo(segment.p1());
        path.lineTo(segment.p2());

        QPainterPathStroker stroker;
        stroker.setWidth(6);  // šířka klikací oblasti
        QPainterPath clickable = stroker.createStroke(path);

        if (clickable.contains(pos)) {
            hoveredSegmentIndex = i;  // např. pro zvýraznění nebo výběr
            setCursor(Qt::PointingHandCursor);
            update();  // překreslit
            return;
        }
    }

    hoveredSegmentIndex = -1;
    unsetCursor();
    update();
}

mycircle::mycircle()
{
    setFlag (ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    pen.setColor(Qt::blue);
    mypolygon = new QPolygonF;
    finished=0;
    center=QPointF(0.0,0.0);
    radius=0;
}

void mycircle::addPointToShape(const QPointF& point)
{
    qDebug() << "pridavam bod do mycircle " << point ;
    //m_boundingRect=QRectF(bounding_min_max());
}

QRectF mycircle::boundingRect() const
{
   return QRectF(-20,-20,40,40);
    //qDebug() << "boungingrect" <<m_boundingRect;
    //return m_boundingRect;

}

void mycircle::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units)
{
    qDebug() << "export mycircle" << this->pen;
    double cx = mmToUnits(center.x(), units);
    double cy = mmToUnits(center.y(), units) * (-1);
    double r  = mmToUnits(radius, units);
    DL_CircleData circleData(cx, cy, 0.0, r);
    dxf.writeCircle(dw, circleData, DL_Attributes("0", 256, -1, "BYLAYER", 1.0));
    DL_PointData pointData(cx, cy, 0.0);
    dxf.writePoint(dw, pointData, DL_Attributes("0", 256, -1, "BYLAYER", 1.0));
}

void mycircle::save(QTextStream &out)
{
    out << "CIRCLE " << center.x() << " " << center.y() << " " << radius << "\n";
}



void mycircle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    Q_UNUSED(option);
    Q_UNUSED(widget);

    qDebug() << "paint count " << mypolygon->count();

    painter->setPen(pen);
    //qDebug() << "mycircle" << mypolygon ;
   /* Circle circle;
    if (mypolygon->count()>= 5 )
    {
       // qDebug() << "uz staci  count " << mypolygon->count();
        //Circle circle;

        CircleData data(mypolygon->count());
        data.n=mypolygon->count();
        for (int i=0; i<mypolygon->count(); i++)
        {
           data.X[i]=mypolygon->at(i).x();
           data.Y[i]=mypolygon->at(i).y();
           //qDebug () << "X=" << data.X[i] ;
           //qDebug () << "Y=" << data.Y[i] ;

        }

        circle = CircleFitByPratt (data);
        //qDebug () << "a=" <<circle.a ;
        //qDebug () << "b=" <<circle.b ;
        //qDebug () << "r=" <<circle.r ;
        painter->setPen(pen);
        painter->drawEllipse (circle.a-circle.r,circle.b-circle.r,2*circle.r,2*circle.r);
        painter->drawLine (circle.a-10,circle.b-10,circle.a+10,circle.b+10);
        painter->drawLine (circle.a+10,circle.b-10,circle.a-10,circle.b+10);
        //center=mapToScene(circle.a,circle.b);
        center=QPointF(circle.a,circle.b);
        radius=circle.r;
        m_boundingRect=QRectF(center.x()-radius,center.y()-radius,2*radius,2*radius  );
        //qDebug () << "mapfromscene" << mapFromScene(circle.a,circle.b);
        //qDebug () << "maptoscene" << mapToScene(circle.a,circle.b);


    }*/
    if (!finished)
    {
        for (int i=0; i<mypolygon->count(); i++)
        {
            //qDebug() << "circle point " << mypolygon->at(i) << " pos x " <<mypolygon->at(i).x() << " pos y " <<mypolygon->at(i).y() ;
            painter->drawEllipse(mypolygon->at(i).x()-10,mypolygon->at(i).y()-10,20,20);

/* kresleni zeleneho ctverce boundingrect
            QPen temppen=QPen(Qt::green) ;
            temppen.setCapStyle(Qt::RoundCap);
            temppen.setWidth(6);
            painter->setPen(temppen);
            painter->drawRect(m_boundingRect);
            painter->setPen(pen);
*/



        }
    }
    else
    {

       // qDebug () << "mapfromscene" << mapFromScene(circle.a,circle.b);
        //qDebug () << "maptoscene" << mapToScene(circle.a,circle.b);
      //  qDebug() << "finished je nastaven radius je " << -circle.r;
//        painter->drawEllipse (-circle.r,-circle.r,2*circle.r,2*circle.r);
//        painter->drawLine (-10,-10,+10,+10);
//        painter->drawLine (+10,-10,-10,+10);
        // Kruh byl načten ze souboru – použij uložené hodnoty
                painter->drawEllipse(center.x() - radius, center.y() - radius, 2 * radius, 2 * radius);
                painter->drawLine(center.x() - 10, center.y() - 10, center.x() + 10, center.y() + 10);
                painter->drawLine(center.x() + 10, center.y() - 10, center.x() - 10, center.y() + 10);

    }
}

