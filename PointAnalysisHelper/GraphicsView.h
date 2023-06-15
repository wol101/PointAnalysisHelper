#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QPointF>

class MainWindow;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget *parent = 0);
    // explicit GraphicsView(QGraphicsScene * scene, QWidget * parent = 0);

signals:
    void EmitNewMappedLocation(const QPointF &mappedLocation);

public slots:

protected:
    void mousePressEvent(QMouseEvent *event);

};

#endif // GRAPHICSVIEW_H
