#ifndef GRAPHICSVIEWLINECORRELATIONCHART_H
#define GRAPHICSVIEWLINECORRELATIONCHART_H

#include <QGraphicsView>
#include <QWidget>
#include <QPointF>

#include "MultiSpectralImage.h"

class MainWindow;

class GraphicsViewLineCorrelationChart : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsViewLineCorrelationChart(QWidget *parent = 0);
    // explicit GraphicsViewLineCorrelationChart(QGraphicsScene * scene, QWidget * parent = 0);

signals:
    void EmitClicked();

public slots:

protected:
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // GRAPHICSVIEWLINECORRELATIONCHART_H
