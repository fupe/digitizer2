#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include <QObject>
#include <QVector>
#include <QGraphicsItem>
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

signals:
    void shapesChanged();

private:
    QVector<GraphicsItems*> shapes_;
};

#endif // SHAPEMANAGER_H
