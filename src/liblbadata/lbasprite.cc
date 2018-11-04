#include <QDebug>
#include <QImage>
#include "lbasprite.h"
#include "binaryreader.h"

//-------------------------------------------------------------------------------------------
LbaSprite::LbaSprite(const LbaPalette &pal, const QByteArray &buffer, bool raw)
    : mPalette(pal)
{
    if (raw)
        fromRawBuffer(buffer);
    else
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
bool LbaSprite::fromRawBuffer(const QByteArray &buffer)
{
    // https://github.com/agrande/lba2remake/blob/master/src/iso/sprites.js

    /*
 const dataView = new DataView(sprites.getEntry(entry));
    const width = dataView.getUint8(8);
    const height = dataView.getUint8(9);
    const buffer = new ArrayBuffer(width * height);
    const pixels = new Uint8Array(buffer);
    let ptr = 12;
    for (let y = 0; y < height; y += 1) {
        let x = 0;
        const offset = () => (y * width) + x;
        for (let run = 0; run < width; run += 1) {
            pixels[offset()] = dataView.getUint8(ptr);
            ptr += 1;
            x += 1;
        }
}
    */
    BinaryReader reader(buffer);
    reader.skip(8);
    const auto width   = reader.readUint8();
    const auto height  = reader.readUint8();
    reader.skip(2);
    Q_ASSERT(reader.pos() == 12);

    QImage image(width, height, QImage::Format_ARGB32);

    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            image.setPixel(x,y,mPalette.palette()[reader.readUint8()]);
        }
    }

    mImage = image;

    return true;
}

//-------------------------------------------------------------------------------------------
const QImage &LbaSprite::image() const
{
    return mImage;
}

