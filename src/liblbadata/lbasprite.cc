#include <QDebug>
#include <QImage>
#include <QFile>
#include <math.h>
#include "lbasprite.h"
#include "binaryreader.h"
#include "binarywriter.h"

//-------------------------------------------------------------------------------------------
LbaSprite::LbaSprite()
{
}

//-------------------------------------------------------------------------------------------
LbaSprite::LbaSprite(const LbaPalette &pal, const QByteArray &buffer, LbaSprite::Type spriteType )
    : mPalette(pal)
{
    if (buffer.isEmpty())
        return;

    switch(spriteType) {
    case Sprite:
        fromSpriteBuffer(buffer); break;
    case RawSprite:
        fromRawSpriteBuffer(buffer); break;
    case Image:
        fromImageBuffer(buffer); break;
    case AutoSprite: {
        if (fromImageBuffer(buffer))
            break;
        if (fromRawSpriteBuffer(buffer))
            break;
        fromSpriteBuffer(buffer);
    } break;
    }
}

//-------------------------------------------------------------------------------------------
bool LbaSprite::fromImageBuffer(const QByteArray &buffer)
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
    mImage.setColorTable(mPalette.palette());
    for (int y=0; y<height; y++) {
        quint8 *line = mImage.scanLine(y);
        for (int x=0; x<width; x++) {
            line[x] = src[y*width + x];
        }
    }

    return true;
}

//-------------------------------------------------------------------------------------------
bool LbaSprite::fromFile(const QString &filename, const LbaPalette &pal)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QByteArray imageData = f.readAll();
    mImage = QImage::fromData(imageData); // png? jpg? gif? bmp?
    if (!mImage.isNull())
        return true;

    LbaSprite importer(pal,imageData,LbaSprite::AutoSprite);
    mImage = importer.image();

    return !mImage.isNull();
}

//-------------------------------------------------------------------------------------------
bool LbaSprite::toFile(const QString &filename, const LbaPalette &pal) const
{
    if (mImage.isNull())
        return false;

    if (pal.palette().isEmpty())
        return false;

    if (filename.toLower().endsWith(".png") || filename.toLower().endsWith(".jpg"))
        return mImage.save(filename,0,100);

    return toLbaImageFile(filename,pal);
}

//-------------------------------------------------------------------------------------------
bool LbaSprite::fromSpriteBuffer(const QByteArray &buffer)
{
    mImage = QImage();
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

    if (width * height <= 0)
        return false;

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

    if (!reader.error())
        mImage = image;
    return true;
}

//-------------------------------------------------------------------------------------------
bool LbaSprite::fromRawSpriteBuffer(const QByteArray &buffer)
{
    // https://github.com/agrande/lba2remake/blob/master/src/iso/sprites.js
    mImage = QImage();
    BinaryReader reader(buffer);
    reader.skip(8);
    const auto width   = reader.readUint8();
    const auto height  = reader.readUint8();
    reader.skip(2);
    if (reader.pos() != 12)
        return false;

    if (height*width <= 0)
        return false;

    if (height*width > (buffer.size() - 12))
        return false;

    QImage image(width, height, QImage::Format_ARGB32);

    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            image.setPixel(x,y,mPalette.palette()[reader.readUint8()]);
        }
    }

    if (reader.error())
        return false;

    mImage = image;

    return true;
}

//-------------------------------------------------------------------------------------------
const QImage &LbaSprite::image() const
{
    return mImage;
}

//-------------------------------------------------------------------------------------------
bool LbaSprite::toLbaImageFile(const QString &filename, const LbaPalette &pal) const
{
    if (!(((mImage.width() == 640) && (mImage.height() == 480)) || (mImage.width() == mImage.height())))
        return false;

    QByteArray imgData = img2Buffer(pal);
    if (imgData.isEmpty())
        return false;

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }

    f.write(imgData);
    return true;
}

//-------------------------------------------------------------------------------------------
QByteArray LbaSprite::img2Buffer(const LbaPalette &pal) const
{
    BinaryWriter writer;
    for (int y=0; y<mImage.height(); y++) {
        for (int x=0; x<mImage.width(); x++) {
            int colorIndex = pal.nearestIndexOf(mImage.pixel(x,y));
            Q_ASSERT(colorIndex >= 0);
            Q_ASSERT(colorIndex <  256);
            writer.append((quint8)colorIndex);
        }
    }

    return writer.buffer();
}

