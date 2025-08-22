#include "shapemanager.h"

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
