#ifndef POINTLOCATIONS_H
#define POINTLOCATIONS_H

#include <QString>
#include <QMap>
#include <QPointF>
#include <QPoint>
#include <QColor>

class MultiSpectralImage;

struct NamedPoint
{
    NamedPoint(const QString &n, const QPoint &p) { name = n; point = p; }

    QString name;
    QPoint point;
};

class PointLocations
{
public:
    PointLocations();
    ~PointLocations();

    int Write(QString fileName);
    int Read(QString fileName);

    void SetMultiSpectralImage(MultiSpectralImage *multiSpectralImage);
    void SetCalibrations(const QPointF &topLeft, const QPointF &bottomRight) { m_topLeft = topLeft; m_bottomRight = bottomRight; }
    void SetOrnaments(double scaleBarLength, const QPoint &scaleBarLocation, const QString &scaleBarUnits, int scaleBarFontSize, int markerFontSize);
    void SetMarkerFontSize(int markerFontSize) { m_pointMapFontSize = markerFontSize; };
    void SetMarkerColor(QColor color) { m_scaleBarColor = color; }
    void SetScaleBarColor(QColor color) { m_pointMapColor = color; }

    void AddPoint(const QString &name, const QPoint &point);
    void DeletePoint(const QString &name);
    bool PointLocationExists(const QString &name);
    QPoint GetPointLocation(const QString &name);
    QList<NamedPoint> *GetPointList() { return &m_pointList; }
    MultiSpectralImage *GetMultiSpectralImage() { return m_multiSpectralImage; }
    void GetCalibrations(QPointF *topLeft, QPointF *bottomRight) { *topLeft = m_topLeft; *bottomRight = m_bottomRight; }
    void GetOrnaments(double *scaleBarLength, QPoint *scaleBarLocation, QString *scaleBarUnits, int *scaleBarFontSize, int *markerFontSize);
    int GetMarkerFontSize() { return m_pointMapFontSize; }
    QColor GetMarkerColor() { return m_scaleBarColor; }
    QColor GetScaleBarColor() { return m_pointMapColor; }

    QPointF CalculateScanPosition(const QPoint &point);
    QPoint CalculateImagePosition(const QPointF &point);

protected:

    MultiSpectralImage *m_multiSpectralImage;
    QPointF m_topLeft;
    QPointF m_bottomRight;

    double m_scaleBarLength;
    QPoint m_scaleBarLocation;
    QString m_scaleBarUnits;    
    int m_scaleBarFontSize;
    QColor m_scaleBarColor;

    QList<NamedPoint> m_pointList;
    int m_pointMapFontSize;
    QColor m_pointMapColor;
};

#endif // POINTLOCATIONS_H
