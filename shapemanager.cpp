#include "shapemanager.h"
#include <QDebug>
#include <QRectF>

ShapeManager::ShapeManager(QObject* parent)
    : QObject(parent)
{
}

void ShapeManager::addShape(GraphicsItems* shape)
{
    shapes_.push_back(shape);
    emit shapesChanged();
}

const QVector<GraphicsItems*>& ShapeManager::getShapes() const
{
    qDebug() <<"ShapeManager::getShapes";
    return shapes_;
}

void ShapeManager::clear()
{
    shapes_.clear();
    currentShape_ = nullptr;
    emit shapesChanged();
}

void ShapeManager::deleteLastShape()
{
    if (!shapes_.isEmpty())
    {
        shapes_.removeLast();
        emit shapesChanged();
    }
}

void ShapeManager::printlistitems()
{
    qDebug() << "printlistitems";
    int index = 0;
    for (GraphicsItems* item : this->getShapes()) {
        qDebug().noquote() << index++ << ": " << shapeInfo(item);
    }
}

void ShapeManager::printShapesInfo() const
{
    int index = 0;
    for (const GraphicsItems* item : shapes_) {
        qDebug().noquote() << index++ << ":::: " << shapeInfo(item);
    }
}

QString ShapeManager::shapeInfo(const GraphicsItems* item) const
{
    if (!item)
        return QString("<null>");

    QRectF rect = item->boundingRect();
    QPointF pos = item->pos();
       return QString("%1 [posx:%2 posy:%3 x:%4 y:%5 w:%6 h:%7 finished:%8]")
               .arg(item->typeName())
               .arg(pos.x())
               .arg(pos.y())
               .arg(rect.x())
               .arg(rect.y())
               .arg(rect.width())
               .arg(rect.height())
               .arg(item->finished ? "true" : "false");
}

void ShapeManager::startShape(GraphicsItems* shape)
{
    if (!shape) return;
    currentShape_ = shape;
    addShape(shape);
}

void ShapeManager::appendToCurrent(const QPointF& point)
{
    if (currentShape_) {
        currentShape_->addPointToShape(point);
    }
}

void ShapeManager::finishCurrent()
{
    if (currentShape_) {
        if (auto pl = dynamic_cast<mypolyline*>(currentShape_)) {
            if (pl->mypolygon && pl->mypolygon->size() > 1) {
                pl->addPointToShape(pl->mypolygon->first());
            }
        }

        currentShape_->finished = true;
        currentShape_ = nullptr;
        emit shapesChanged();
    }
}

bool ShapeManager::hasCurrent() const
{
    return currentShape_ != nullptr;
}
