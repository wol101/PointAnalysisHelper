#ifndef GRAPHICSITEMSCALEBAR_H
#define GRAPHICSITEMSCALEBAR_H

#include <QGraphicsItem>
#include <QString>
#include <QPixmap>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QColor>

enum ScaleBarShape
{
    ScaleBarIBeam,
    ScaleBarRectangle
};

class GraphicsItemScaleBar : public QGraphicsItem
{
public:
    GraphicsItemScaleBar(double size, QString label, ScaleBarShape barShape, int fontPixelSize, QColor color);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    QString m_label;
    QFont m_font;
    QColor m_color;
    QPen m_pen;
    QBrush m_brush;
    QRectF m_rect;
    QRect m_textRect;
    double m_fontPixelSize;
    double m_barWidth;
    double m_barHeight;
    ScaleBarShape m_barShape;
};

#endif // GRAPHICSITEMSCALEBAR_H
