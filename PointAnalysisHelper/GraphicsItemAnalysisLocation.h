#ifndef GRAPHICSITEMANALYSISLOCATION_H
#define GRAPHICSITEMANALYSISLOCATION_H

#include <QGraphicsItem>
#include <QString>
#include <QPixmap>
#include <QFont>
#include <QPen>

class GraphicsItemAnalysisLocation : public QGraphicsItem
{
public:
    GraphicsItemAnalysisLocation(QString name, int fontPixelSize, QColor color);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    static void ExpandRect(double expansion, QRectF *rect);

protected:
    QString m_name;
    QPixmap m_pixmap;
    QFont m_font;
    QPen m_pen;
    QColor m_color;
    QBrush m_brush;
    QRectF m_rect;
    QRect m_textRect;
    double m_fontPixelSize;
    double m_markerSize;
    double m_markerHalfSize;
    double m_markerLineWidth;
    double m_markerHalfLineWidth;
};

#endif // GRAPHICSITEMANALYSISLOCATION_H

