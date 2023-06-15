#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <float.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <limits>

#include <QString>
#include <QList>

#include "MultiSpectralImage.h"
#include "DataFile.h"

MultiSpectralImage::MultiSpectralImage()
{
    m_defaultGamma = 0.5;
    m_defaultPercentileClip = 99.9;
    m_applyLineOffset = false;
    m_lineOffset = 0;
}

MultiSpectralImage::~MultiSpectralImage()
{
    for (int i = 0; i < m_dataList.size(); i++) delete m_dataList[i];
    for (int i = 0; i < m_previewList.size(); i++) delete m_previewList[i];
}


int MultiSpectralImage::ReadRASFile(QString fileName)
{
    struct
    {
      int8_t    version[80];        /* Version number                      */
      int32_t   ver_num;            /* Not used?                           */
      uint32_t  pid;                /* PID of data collection process      */
      int8_t    comment1[80];       /* Misc. comment lines                 */
      int8_t    comment2[80];
      int8_t    comment3[80];
      int8_t    comment4[80];
      int8_t    comment5[80];
      int8_t    comment6[80];
      int8_t    x_motor[40];        /* name of X motor                     */
      int8_t    y_motor[40];        /* Name of Y motor                     */
      int8_t    region[80];         /* Region file name                    */
      int8_t    file_head[80];      /* Data file name info.                */
      int8_t    file_name[80];
      int8_t    start_time[40];
      int8_t    stop_time[40];
      int32_t   num_chans;          /* number of channels to collect       */
                                    /* - sub addresses of the hex scaler   */
      double    count_time;         /* copied from the region              */
      int32_t   num_sweeps;         /* number of sweeps                    */
      int32_t   ascii_out;          /* automatically generate ASCII file?  */
      int32_t   num_points;         /* copied from region - redundant?     */
      int32_t   num_raster;         /* copied from region - redundant?     */
      int32_t   pixel_point;        /* pixel per pt, for real-time display */
      int32_t   scan_dir;           /* = 2 scan in + & -ve X, 1 = +ve only */
      int32_t   scan_type;          /* quick scan or pt to pt, 1 or 2      */
      int32_t   cur_x_dir;          /* cur. dir. of x for real-time plot   */
      int32_t   command1;           /* Point num from collector process    */
      int32_t   command2;           /* Line number, from collector         */
      int32_t   command3;           /* Exit command from collector         */
      int32_t   command4;           /* Commands to collector               */
      double    offsets[6];         /* Not used at present                 */
      int32_t   run_num;            /* Incremented for file name           */
    } header;
    int headerSize = sizeof(header);

    m_RASFileName = fileName;
    DataFile dataFile;

    if (dataFile.ReadFile(fileName.toUtf8().constData())) return __LINE__;
    ptrdiff_t fileSize = dataFile.GetSize();

    if (fileSize < headerSize) return __LINE__;

    char *ptr = dataFile.GetRawData();
    memcpy(&header, ptr, headerSize);
    uint32_t *dataStart = (uint32_t *)(ptr + headerSize);
    //std::cerr << dataStart << "\n";

    int nChannels = header.num_chans;
    int width = header.num_points;
    int height = header.num_raster;

    ptrdiff_t dataSize = (ptrdiff_t)nChannels * (ptrdiff_t)width * (ptrdiff_t)height * 4;
    if (dataSize + headerSize > fileSize) return __LINE__;

    for (int i = 0; i < m_dataList.size(); i++) delete m_dataList[i];
    m_dataList.clear();
    int iChannel;
    ImageData *imageData;
    uint32_t *imageDataPtr;
    int ix, iy;
    for (iChannel = 0; iChannel < nChannels; iChannel++)
    {
        imageData = new ImageData(width, height);
        m_dataList.append(imageData);
        imageDataPtr = imageData->data;
        imageData->min = std::numeric_limits<uint32_t>::max();
        imageData->max = 0;
        imageData->gamma = m_defaultGamma;
        imageData->percentileClip = m_defaultPercentileClip;
        imageData->label = QString("%1").arg(iChannel);

        for (iy = 0; iy < height; iy++)
        {
            for (ix = 0; ix < width; ix++)
            {
                //std::cerr << (dataStart + iy * width * nChannels + ix * nChannels + iChannel) << "\n";
                *imageDataPtr = *(dataStart + iy * width * nChannels + ix * nChannels + iChannel);
                if (*imageDataPtr > imageData->max) imageData->max = *imageDataPtr;
                if (*imageDataPtr < imageData->min) imageData->min = *imageDataPtr;
                imageDataPtr++;
            }
        }
    }
    return 0;
}

void MultiSpectralImage::CreatePreviews()
{
    ImageData *imageData;
    QImage *image;
    int i;
    for (i = 0; i < m_previewList.size(); i++) delete m_previewList[i];
    m_previewList.clear();
    for (i = 0; i < m_dataList.size(); i++)
    {
        imageData = m_dataList[i];
        image = new QImage(imageData->width, imageData->height, QImage::Format_RGB32);
        m_previewList.append(image);
        RefreshPreview(i, imageData->gamma, imageData->percentileClip);
    }
}

void MultiSpectralImage::RefreshPreview(int channel, double gamma, double percentileClip)
{
    double norm, gnorm;
    int ix, iy, t;
    int r, g, b;
    uint32_t percentilePixel;
    int numPixels, nthWanted;
    ImageData *imageData = m_dataList[channel];
    QImage *image;
    uint32_t *imageDataPtr = imageData->data;
    uint32_t imageDataRange = imageData->max - imageData->min;
    if (imageDataRange == 0) imageDataRange = 1;

    // do percentile clip first
    imageData->percentileClip = percentileClip;
    if (percentileClip < 100)
    {
        numPixels = imageData->height * imageData->width;
        nthWanted = int(numPixels * (percentileClip / 100));
        if (nthWanted < numPixels - 1)
        {
            percentilePixel = NthElement(imageData->data, numPixels, numPixels * (percentileClip / 100));
            imageDataRange = percentilePixel - imageData->min;
            if (imageDataRange == 0) imageDataRange = 1;
        }
    }

    imageData->gamma = gamma;
    image = m_previewList[channel];
    if (m_applyLineOffset == false || m_lineOffset == 0)
    {
        for (iy = 0; iy < imageData->height; iy++)
        {
            for (ix = 0; ix < imageData->width; ix++)
            {
                norm = double(*imageDataPtr - imageData->min) / double(imageDataRange);
                gnorm = pow(norm, imageData->gamma);
                r = int(256 * gnorm);
                if (r > 255) r = 255;
                g = b = r;
                image->setPixel(ix, iy, qRgb(r, g, b));
                imageDataPtr++;
            }
        }
    }
    else
    {
        for (iy = 0; iy < imageData->height; iy++)
        {
            if (((iy) & 1) == 0) // iy is eve (0, 2, 4 ...)
            {
                for (ix = 0; ix < imageData->width; ix++)
                {
                    norm = double(*imageDataPtr - imageData->min) / double(imageDataRange);
                    gnorm = pow(norm, imageData->gamma);
                    r = int(256 * gnorm);
                    if (r > 255) r = 255;
                    g = b = r;
                    image->setPixel(ix, iy, qRgb(r, g, b));
                    imageDataPtr++;
                }
            }
            else
            {
                for (ix = 0; ix < imageData->width; ix++)
                {
                    norm = double(*imageDataPtr - imageData->min) / double(imageDataRange);
                    gnorm = pow(norm, imageData->gamma);
                    r = int(256 * gnorm);
                    if (r > 255) r = 255;
                    g = b = r;
                    t = ix - m_lineOffset;
                    if (t >= 0 && t < imageData->width) image->setPixel(t, iy, qRgb(r, g, b));
                    else
                    {
                        if (m_lineOffset > 0) image->setPixel(imageData->width - m_lineOffset + ix, iy, qRgb(0, 0, 0));
                        else image->setPixel(ix - imageData->width - m_lineOffset, iy, qRgb(0, 0, 0));
                    }
                    imageDataPtr++;
                }
            }
        }
    }
}


double MultiSpectralImage::OptimalGamma(int channel)
{
    // we can calculate the gamma that leads to a value of 0.5 when applied to the median intensity pixel
    ImageData *imageData = m_dataList[channel];
    uint32_t imageRange = imageData->max - imageData->min;
    if (imageRange == 0) return 1; // this is just a sanity check
    int numPixels = imageData->height * imageData->width;
    // do percentile clip first
    if (imageData->percentileClip < 100)
    {
        int nthWanted = int(numPixels * (imageData->percentileClip / 100));
        if (nthWanted < numPixels - 1)
        {
            uint32_t percentilePixel = NthElement(imageData->data, numPixels, numPixels * (imageData->percentileClip / 100));
            imageRange = percentilePixel - imageData->min;
            if (imageRange == 0) return 1; // this is just a sanity check
        }
    }
    uint32_t medianPixel = NthElement(imageData->data, numPixels, numPixels / 2);
    if (medianPixel == 0) medianPixel = 1; // otherwise we end up with a gamma of zero which is not helpful
    double normMedianPixel = double(medianPixel - imageData->min) / double(imageRange);
    double optimalGamma = log(0.5) / log(normMedianPixel);
    return optimalGamma;
}

uint32_t MultiSpectralImage::NthElement(const uint32_t *input_data, int data_len, int nth)
{
    uint32_t *data = new uint32_t[data_len];
    memcpy(data, input_data, data_len * sizeof(uint32_t));
    std::nth_element(data, data + nth, data + data_len);
    uint32_t nthElement = *(data + nth);
    delete [] data;
    return nthElement;
}

