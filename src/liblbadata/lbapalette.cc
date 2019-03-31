#include "lbapalette.h"
#include "binarywriter.h"
#include <QTextStream>
#include <QStringList>

#define GIMP_HEADER "GIMP Palette"

//-------------------------------------------------------------------------------------------
LbaPalette::LbaPalette(const QByteArray &buffer)
{
    fromBuffer(buffer);
}

//-------------------------------------------------------------------------------------------
LbaPalette::LbaPalette(const QVector<QRgb> &pal)
{
    setPalette(pal);
}

//-------------------------------------------------------------------------------------------
bool LbaPalette::fromBuffer(const QByteArray &buffer)
{
    mPalette.clear();

    if (buffer.length()/3 == 256)          // Binary LBA Palette
        return fromLbaPalette(buffer);

    if (buffer.startsWith(GIMP_HEADER)) // GIMP *.gpl-Files
        return fromGimpPalette(buffer);

    return false;
}

//-------------------------------------------------------------------------------------------
QByteArray LbaPalette::toBuffer() const
{
    BinaryWriter ret;

    for (int i=0; i<mPalette.count(); i++) {
        int r = qRed(mPalette[i]);
        int g = qGreen(mPalette[i]);
        int b = qBlue(mPalette[i]);

        ret.append((quint8)r);
        ret.append((quint8)g);
        ret.append((quint8)b);
    }

    return ret.buffer();
}

//-------------------------------------------------------------------------------------------
void LbaPalette::setPalette(const QVector<QRgb> &pal)
{
    mPalette = pal;
}

//-------------------------------------------------------------------------------------------
const QVector<QRgb> &LbaPalette::palette() const
{
    return mPalette;
}

//-------------------------------------------------------------------------------------------
QByteArray LbaPalette::toGimpPalette(const QString &name) const
{
    QByteArray ret;
    ret += GIMP_HEADER; ret += "\n";
    ret += QString("Name: %1\n#\n").arg(name).toUtf8();
    for (int i=0; i<mPalette.count(); i++) {
        int r = qRed(mPalette[i]);
        int g = qGreen(mPalette[i]);
        int b = qBlue(mPalette[i]);

        ret += QString("%1 %2 %3\n")
                .arg(r,3,10,QChar(' '))
                .arg(g,3,10,QChar(' '))
                .arg(b,3,10,QChar(' '));
    }
    return ret;
}

//-------------------------------------------------------------------------------------------
bool LbaPalette::fromLbaPalette(const QByteArray &buffer)
{
    Q_ASSERT(buffer.length() == 3*256);

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
bool LbaPalette::fromGimpPalette(const QByteArray &buffer)
{

    /*
GIMP Palette
Name: ....
#
236 216  20
116 132 192
108 128 188
104 128 188
100 124 184
 92 124 180
 88 120 180
 80 120 176
 76 116 172
    */

    QTextStream stream(buffer);
    QString line;
    bool inColorTable = false;
    QList<QRgb> palette;
    while (stream.readLineInto(&line)) {
        if (line.trimmed() == "#") {
            inColorTable = true;
            continue;
        }
        if (!inColorTable)
            continue;
        QStringList rgb = line.split(" ",QString::SkipEmptyParts);
        if (rgb.count() != 3)
            return false;
        bool ok;
        int r = rgb[0].trimmed().toInt(&ok); if (!ok || (r < 0) || (r > 255)) return false;
        int g = rgb[1].trimmed().toInt(&ok); if (!ok || (r < 0) || (r > 255)) return false;
        int b = rgb[2].trimmed().toInt(&ok); if (!ok || (r < 0) || (r > 255)) return false;
        palette << QColor(r,g,b).rgb();
    }

    if (palette.isEmpty())
        return false;

    mPalette = palette.toVector();
    return true;
}

