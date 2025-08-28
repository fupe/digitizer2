#include "shapemanager.h"
#include <QDebug>

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
    return shapes_;
}

void ShapeManager::clear()
{
    shapes_.clear();
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
    for (GraphicsItems* item : this->getShapes()) {
        qDebug() << item->typeName();
    }
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
        currentShape_->finished = true;
        currentShape_ = nullptr;
        emit shapesChanged();
    }
}
