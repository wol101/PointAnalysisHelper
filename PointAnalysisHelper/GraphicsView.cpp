#include <QDebug>
#include <QMouseEvent>

#include "GraphicsView.h"
#include "MainWindow.h"

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::ShiftModifier)
    {
        QPointF mappedLocation = QGraphicsView::mapToScene(event->pos());
        emit EmitNewMappedLocation(mappedLocation);
    }
}
