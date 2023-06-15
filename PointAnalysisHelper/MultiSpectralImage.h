#ifndef MULTISPECTRALIMAGE_H
#define MULTISPECTRALIMAGE_H

#include <QString>
#include <QList>
#include <QImage>

#include <stdint.h>

struct ImageData
{
    ImageData() { width = 0; height = 0; data = 0; }
    ImageData(int newWidth, int newHeight) { width = newWidth; height = newHeight; data = new uint32_t[width * height]; }
    ~ImageData() { if (data) delete [] data; }

    int width;
    int height;
    uint32_t min;
    uint32_t max;
    double gamma;
    double percentileClip;
    QString label;
    uint32_t *data;
};

class MultiSpectralImage
{
public:
    MultiSpectralImage();
    ~MultiSpectralImage();

    int ReadRASFile(QString fileName);
    void CreatePreviews();
    void RefreshPreview(int channel, double gamma, double percentileClip);

    void SetDefaultGamma(double gamma) { m_defaultGamma = gamma; }
    void SetDefaultPercentileClip(double defaultPercentileClip) { m_defaultPercentileClip = defaultPercentileClip; }
    void SetImageLabel(int channel, const QString &label) { m_dataList[channel]->label = label; }
    void SetApplyLineOffset(bool applyLineOffset) { m_applyLineOffset = applyLineOffset; }
    void SetLineOffset(int lineOffset) { m_lineOffset = lineOffset; }

    QImage *GetPreview(int i) { if (i < 0 || i > m_previewList.size()) return 0; else return m_previewList[i]; }
    ImageData *GetImageData(int i) { if (i < 0 || i > m_dataList.size()) return 0; else return m_dataList[i]; }
    QString GetRASFileName() { return m_RASFileName; }
    int GetChannels() { return m_dataList.size(); }
    bool GetApplyLineOffset() { return m_applyLineOffset; }
    int GetLineOffset() { return m_lineOffset; }
    QString GetImageLabel(int channel) { return m_dataList[channel]->label; }

    double OptimalGamma(int channel);
    uint32_t NthElement(const uint32_t *input_data, int data_len, int nth);

protected:
    QList<ImageData *> m_dataList;
    QList<QImage *> m_previewList;
    QString m_RASFileName;
    double m_defaultGamma;
    double m_defaultPercentileClip;

    bool m_applyLineOffset;
    int m_lineOffset;
};

#endif // MULTISPECTRALIMAGE_H
