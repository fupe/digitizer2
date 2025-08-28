#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include <QObject>
#include <QVector>
#include <QGraphicsItem>
#include <QPointF>
#include <QString>
#include "GraphicsItems.h"
#include <memory>

class ShapeManager : public QObject
{
    Q_OBJECT

public:
    explicit ShapeManager(QObject* parent = nullptr);

    void addShape(GraphicsItems* shape);
    const QVector<GraphicsItems*>& getShapes() const;
    void clear();
    void deleteLastShape();
    void printlistitems(void);
    void printShapesInfo() const;

    // nový API pro tvorbu složitějších tvarů
    void startShape(GraphicsItems* shape);
    void appendToCurrent(const QPointF& point);
    void finishCurrent();

signals:
    void shapesChanged();

private:
    QVector<GraphicsItems*> shapes_;
    GraphicsItems* currentShape_ = nullptr;

    QString shapeInfo(const GraphicsItems* item) const;
};

#endif // SHAPEMANAGER_H
