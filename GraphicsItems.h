#ifndef GRAPHICSITEMS_H
#define GRAPHICSITEMS_H
#include <QGraphicsItem>
#include <QPainter>
#include <QList>
#include <QtGlobal>
#include "settings.h"

class QTextStream;
class DL_Dxf;
class DL_WriterA;


class GraphicsItems : public QGraphicsItem

{
public:
    GraphicsItems();
    enum { Type = UserType + 1 }  ;
        int type() const override
           {
               // Enable the use of qgraphicsitem_cast with this item.
               return Type;
           }
    virtual void addPointToShape(const QPointF&) {/* defaultně nic nedělá*/ }
    QRectF boundingRect()const override;
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget) override ;

    QPoint position;
    QPen pen;
    bool finished = false;
    QRectF m_boundingRect;

    virtual void export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units);  //export to dxf file
    virtual void save(QTextStream &out) = 0;  // čistě virtuální metoda v základní třídě


    virtual QString typeName() const { return "GraphicsItems"; }

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

class mypoint : public GraphicsItems

{
public:
    mypoint();
    float alfa=0,beta=0;
    enum { Type = UserType + 2 }  ;
        int type() const override
           {
               // Enable the use of qgraphicsitem_cast with this item.
               return Type;
           }
    void addPointToShape(const QPointF&) override ;
    QRectF boundingRect()const override;
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget) override ;
    virtual void export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units) override;  //export to dxf file
    void save(QTextStream &out) override;

    QString typeName() const override { return "point"; }

};

class AppManager;  // forward declaration

class mypolyline : public GraphicsItems
{
public:
    explicit mypolyline(AppManager* app = nullptr);
    enum { Type = UserType + 3 }  ;
        int type() const override
           {
               // Enable the use of qgraphicsitem_cast with this item.
               return Type;
           }
    QPolygonF *mypolygon;
    AppManager* appManager_ = nullptr;
    QPainterPath shape() const override ;
    QRectF boundingRect()const override;
    void addPointToShape(const QPointF&) override ;
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget) override ;
    virtual void export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units) override;  //export to dxf file
    void save(QTextStream &out) override;

    void updatePreview();


    QString typeName() const override { return "polyline"; }
    int hoveredSegmentIndex = -1;
    int selectedSegmentIndex = -1; // index kliknutého segmentu
    int get_selectedSegmentIndex(void);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;



};

class mycircle : public GraphicsItems

{
public:
    mycircle();
    enum { Type = UserType + 4 }  ;   int type() const override
           {
               // Enable the use of qgraphicsitem_cast with this item.
               return Type;
           }
    QPolygonF *mypolygon;
    void addPointToShape(const QPointF&) override ;
    QRectF boundingRect()const override;
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget) override ;
    virtual void export_dxf(DL_Dxf& dxf, DL_WriterA& dw, Units units) override;  //export to dxf file
    void save(QTextStream &out) override;


    QString typeName() const override { return "circle"; }
    QPointF center;
    double radius;




};







#endif // MYGRAPHICSITEMS_H
