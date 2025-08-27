#include "GraphicsItems.h"
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPathStroker>
#include <QDebug>
#include "3rdparty/dxflib/src/dl_dxf.h"
#include <QTextStream>

GraphicsItems::GraphicsItems()
{
    //constructor
    pen.setColor(Qt::blue);
    pen.setWidth(1);
    setFlag (ItemIsSelectable, true);
    setAcceptHoverEvents(true);
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

void GraphicsItems::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, double scale)
{
    Q_UNUSED(dxf);
    Q_UNUSED(dw);
    Q_UNUSED(scale);
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

    painter->setPen(pen);
    //painter->drawLine (-1,-1,1,1);
    //painter->drawLine (1,-1,-1,1);
    painter->drawPoint(1,1);
}

void mypoint::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, double scale)
{
    qDebug() << "export mypoint" << this->pos();
    dxf.writeComment(dw, QString("alfa: %1").arg(-alfa).toStdString());
    dxf.writeComment(dw, QString("beta: %1").arg(-beta).toStdString());
    DL_PointData data(this->pos().x() * scale, this->pos().y() * (-scale), 0.0);
    dxf.writePoint(dw, data, DL_Attributes("0", 256, -1, "BYLAYER", 1.0));
}

void mypoint::save(QTextStream &out)
{
    out << "POINT " << pos().x() << " " << pos().y() << " " << alfa << " " << beta << "\n";
}





mypolyline::mypolyline()
{
    setFlag (ItemIsSelectable, true);
    setAcceptHoverEvents(true);
//    pen.setColor(Qt::green);
    pen.setCapStyle(Qt::RoundCap);
    mypolygon = new QPolygonF;
    m_boundingRect=QRectF(-20,-20,40,40);
    qDebug() << "konstriuktor polyline" ;
}

void mypolyline::addPointToShape(const QPointF& point)
{
    qDebug() << "pridavam bod do mypolyline " << point ;
    prepareGeometryChange();
    mypolygon->append(point);
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
    //return m_boundingRect;
    if (!mypolygon || mypolygon->isEmpty())
            return QRectF();
    return  mypolygon->boundingRect().adjusted(-3,-3,3,3);
}

void mypolyline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{


    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(pen);
    painter->drawLine (-10,-10,10,10);
    painter->drawLine (10,-10,-10,10);
    painter->drawPolyline(*mypolygon);

    if (mypolygon->count()>1 and isSelected())
    {
    //QPen temppen=QPen(Qt::red) ;
    //temppen.setCapStyle(Qt::RoundCap);
    //temppen.setWidth(6);
    //painter->setPen(temppen);
    painter->drawLine (mypolygon->at(mypolygon->count()-1).x(),mypolygon->at(mypolygon->count()-1).y(),mypolygon->at(mypolygon->count()-2).x(),mypolygon->at(mypolygon->count()-2).y());
    painter->setPen(pen);
    }

/*
    QPen temppen=QPen(Qt::green) ;
    temppen.setCapStyle(Qt::RoundCap);
    temppen.setWidth(6);
    painter->setPen(temppen);
    painter->drawRect(m_boundingRect);
    painter->setPen(pen);
*/
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

void mypolyline::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, double scale)
{
    qDebug() << "export mypolyline" << this->pen;
    for (int i = 0; i < mypolygon->count() - 1; i++) {
        DL_LineData lineData(
            (mypolygon->at(i).x() + pos().x()) * scale,
            (mypolygon->at(i).y() + pos().y()) * (-scale),
            0.0,
            (mypolygon->at(i + 1).x() + pos().x()) * scale,
            (mypolygon->at(i + 1).y() + pos().y()) * (-scale),
            0.0);
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

void mycircle::export_dxf(DL_Dxf& dxf, DL_WriterA& dw, double scale)
{
    qDebug() << "export mycircle" << this->pen;
    DL_CircleData circleData(center.x() * scale, center.y() * (-scale), 0.0, radius * scale);
    dxf.writeCircle(dw, circleData, DL_Attributes("0", 256, -1, "BYLAYER", 1.0));
    DL_PointData pointData(center.x() * scale, center.y() * (-scale), 0.0);
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

