#include <QGraphicsItem>
#include <QString>
#include <QPixmap>
#include <QFont>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QColor>

#include "GraphicsItemAnalysisLocation.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))

GraphicsItemAnalysisLocation::GraphicsItemAnalysisLocation(QString name, int fontPixelSize, QColor color)
{
    m_name = name;
    m_fontPixelSize = fontPixelSize;
    m_font = QFont("Helvetica", 12, QFont::Bold, false);
    m_font.setPixelSize(m_fontPixelSize);
    m_color = color;
    m_pen = QPen(m_color);
    m_brush = QBrush(m_color, Qt::SolidPattern);

    m_markerSize = fontPixelSize;
    m_markerHalfSize = m_markerSize / 2;
    if (m_markerSize <= 30) m_markerLineWidth = 1;
    else m_markerLineWidth = 3;
    m_markerHalfLineWidth = m_markerLineWidth / 2;

    m_textRect = QFontMetrics(m_font).boundingRect(m_name);

    m_rect.setLeft(-m_markerHalfSize);
    m_rect.setRight(m_markerHalfSize + 1 + m_textRect.width());
    m_rect.setTop(-MAX(m_textRect.height() / 2, m_markerHalfSize));
    m_rect.setBottom(MAX(m_textRect.height() / 2, m_markerHalfSize));

    ExpandRect(fontPixelSize, &m_rect); // makes sure this is big enough
}

void GraphicsItemAnalysisLocation::ExpandRect(double expansion, QRectF *rect)
{
    rect->setLeft(rect->left() - expansion);
    rect->setRight(rect->right() + expansion);
    rect->setTop(rect->top() - expansion);
    rect->setBottom(rect->bottom() + expansion);
}

QRectF GraphicsItemAnalysisLocation::boundingRect() const
{
    return m_rect;
}

void GraphicsItemAnalysisLocation::paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */)
{
    painter->setPen(m_pen);
    painter->setFont(m_font);
    painter->drawText(QPointF(m_markerHalfSize + 1, m_fontPixelSize / 2), m_name);

    // draw a cross marker
    painter->setPen(Qt::NoPen);
    painter->fillRect(QRectF(-m_markerHalfSize, -m_markerHalfLineWidth, m_markerSize, m_markerLineWidth), m_brush);
    painter->fillRect(QRectF(-m_markerHalfLineWidth, -m_markerHalfSize, m_markerLineWidth, m_markerSize), m_brush);
}
