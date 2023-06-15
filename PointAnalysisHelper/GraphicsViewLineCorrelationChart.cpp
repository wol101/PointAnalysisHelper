#include <QDebug>
#include <QMouseEvent>

#include "GraphicsViewLineCorrelationChart.h"
#include "MainWindow.h"

GraphicsViewLineCorrelationChart::GraphicsViewLineCorrelationChart(QWidget *parent) :
    QGraphicsView(parent)
{
}

void GraphicsViewLineCorrelationChart::mousePressEvent(QMouseEvent * /* event */)
{
    emit EmitClicked();
}

void GraphicsViewLineCorrelationChart::resizeEvent(QResizeEvent * /* event */)
{
    emit EmitClicked();
}

