#include <QDebug>
#include <QImage>
#include "lbasprite.h"
#include "binaryreader.h"

//-------------------------------------------------------------------------------------------
LbaSprite::LbaSprite(const LbaPalette &pal, const QByteArray &buffer)
    : mPalette(pal)
{
    fromBuffer(buffer);
}

//-------------------------------------------------------------------------------------------
bool LbaSprite::fromBuffer(const QByteArray &buffer)
{
    BinaryReader reader(buffer);

    quint32 offsetSprite0;

    reader.read(&offsetSprite0,4);
    reader.skip(offsetSprite0-4);

    // qDebug() << "Sprites" << offsetSprite0/4 - 1; // WTF... never more than 1 sprite..

    quint8 left;
    quint8 top;

    quint8 width;
    quint8 height;

    reader.read(&width, 1);
    reader.read(&height, 1);

    reader.read(&left, 1);
    reader.read(&top, 1);

    qint32 x = 0;
    qint32 y = 0;

    Q_ASSERT(width * height > 0);

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // twin-e: grid.c drawBrickSprite()
    for (int c1 = 0; c1 < height; c1++) {
        int vc3 = reader.readUint8();
        for (int c2 = 0; c2 < vc3; c2++) {
            int temp      = reader.readUint8();
            int iteration = temp & 0x3F;
            if (temp & 0xC0) {
                iteration++;
                if (!(temp & 0x40)) {
                    temp = reader.readUint8();
                    for (int i = 0; i < iteration; i++) {
                        image.setPixel(x,y,mPalette.palette()[temp]);
                        x++;
                    }
                } else {
                    for (int i = 0; i < iteration; i++) {
                        image.setPixel(x,y,mPalette.palette()[reader.readUint8()]);
                        x++;
                    }
                }
            } else {
                x += iteration + 1;
            }
        }
        x = 0;
        y++;
    }

    mImage = image;
    return true;
}

//-------------------------------------------------------------------------------------------
const QImage &LbaSprite::image() const
{
    return mImage;
}

