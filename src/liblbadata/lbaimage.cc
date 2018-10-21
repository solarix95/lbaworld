#include "lbaimage.h"
#include <math.h>

//-------------------------------------------------------------------------------------------
LbaImage::LbaImage(const QByteArray &buffer)
{
    fromBuffer(buffer);
}

//-------------------------------------------------------------------------------------------
bool LbaImage::fromBuffer(const QByteArray &buffer)
{
    mImage = QImage();

    int width;
    int height;
    if (buffer.length() == (640*480)) {
        width  = 640;
        height = 480;
    } else {
        width = height = sqrt(buffer.length());
    }

    if (buffer.length() != (width*height))
        return false;

    quint8 *src = (quint8*)buffer.constData();
    mImage = QImage(QSize(width,height), QImage::Format_Indexed8);
    for (int y=0; y<height; y++) {
        quint8 *line = mImage.scanLine(y);
        for (int x=0; x<width; x++) {
                 line[x] = src[y*width + x];
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------
const QImage &LbaImage::image() const
{
    return mImage;
}

