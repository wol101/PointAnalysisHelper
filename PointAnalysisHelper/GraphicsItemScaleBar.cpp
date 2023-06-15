#include <QGraphicsItem>
#include <QString>
#include <QPixmap>
#include <QFont>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QColor>
#include <QFontMetrics>

#include "GraphicsItemScaleBar.h"
#include "GraphicsItemAnalysisLocation.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))

GraphicsItemScaleBar::GraphicsItemScaleBar(double size, QString label, ScaleBarShape barShape, int fontPixelSize, QColor color)
{
    m_barShape = barShape;
    m_barWidth = size;
    m_barHeight = size / 10;
    m_label = label;
    m_color = color;
    m_fontPixelSize = fontPixelSize;
    m_font = QFont("Helvetica", 12, QFont::Bold, false);
    m_font.setPixelSize(m_fontPixelSize);
    m_pen = QPen(m_color);
    m_brush = QBrush(m_color, Qt::SolidPattern);
    m_textRect = QFontMetrics(m_font).boundingRect(m_label);

    double width = MAX(m_textRect.width(), m_barWidth);
    m_rect.setLeft(0);
    m_rect.setRight(width);
    m_rect.setTop(0);
    m_rect.setBottom(m_fontPixelSize + 2 + m_barHeight);

    GraphicsItemAnalysisLocation::ExpandRect(fontPixelSize, &m_rect); // makes sure this is big enough
}


QRectF GraphicsItemScaleBar::boundingRect() const
{
    return m_rect;
}

void GraphicsItemScaleBar::paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /*widget*/ )
{
    if (m_barWidth > 0)
    {
        painter->setPen(m_pen);
        painter->setFont(m_font);
        double barCentreX = m_barWidth / 2;
        double barCentreY = m_fontPixelSize + 2 + m_barHeight / 2;
        switch (m_barShape)
        {
        case ScaleBarIBeam:
            m_pen.setWidth(1);
            painter->drawText(QPointF(barCentreX - (double(m_textRect.width()) / 2), m_fontPixelSize), m_label);
            painter->drawLine(barCentreX - m_barWidth / 2, barCentreY, barCentreX + m_barWidth / 2, barCentreY);
            painter->drawLine(barCentreX - m_barWidth / 2, barCentreY - m_barHeight / 2, barCentreX - m_barWidth / 2, barCentreY + m_barHeight / 2);
            painter->drawLine(barCentreX + m_barWidth / 2, barCentreY - m_barHeight / 2, barCentreX + m_barWidth / 2, barCentreY + m_barHeight / 2);
            break;

        case ScaleBarRectangle:
            painter->drawText(QPointF(barCentreX - (double(m_textRect.width()) / 2), m_fontPixelSize), m_label);
            painter->setPen(Qt::NoPen);
            painter->fillRect(QRectF(barCentreX - m_barWidth / 2, barCentreY - m_barHeight / 2, m_barWidth, m_barHeight), m_brush);
            break;
        }
    }
}
