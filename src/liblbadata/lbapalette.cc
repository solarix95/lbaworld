#include "lbapalette.h"

//-------------------------------------------------------------------------------------------
LbaPalette::LbaPalette(const QByteArray &buffer)
{
    fromBuffer(buffer);
}

//-------------------------------------------------------------------------------------------
bool LbaPalette::fromBuffer(const QByteArray &buffer)
{
    mPalette.clear();
    if (buffer.length()/3 != 256)
        return false;

    quint8 *ptr = (quint8*)buffer.constData();
    mPalette.resize(256);
    for (int i=0; i<256; i++) {
        int r = *(((quint8*)ptr) + i*3 + 0);
        int g = *(((quint8*)ptr) + i*3 + 1);
        int b = *(((quint8*)ptr) + i*3 + 2);

        mPalette[i] = QColor(r,g,b).rgb();
    }

    return true;
}

//-------------------------------------------------------------------------------------------
const QVector<QRgb> &LbaPalette::palette() const
{
    return mPalette;
}

