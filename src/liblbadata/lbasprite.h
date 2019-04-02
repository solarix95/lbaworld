#ifndef LBASPRITE_H
#define LBASPRITE_H

#include <QByteArray>
#include <QImage>
#include <lbapalette.h>

class LbaSprite
{
public:
    enum Type {
        Sprite,
        RawSprite,
        Image,
        AutoSprite
    };

    LbaSprite();
    LbaSprite(const LbaPalette &pal, const QByteArray &buffer = QByteArray(), Type spriteType = Sprite);

    bool fromSpriteBuffer(const QByteArray &buffer);
    bool fromRawSpriteBuffer(const QByteArray &buffer);
    bool fromImageBuffer(const QByteArray &buffer);

    bool fromFile(const QString &filename, const LbaPalette &pal);
    bool toFile(const QString &filename, const LbaPalette &pal) const;

    const QImage &image() const;

private:
    bool       toLbaImageFile(const QString &filename, const LbaPalette &pal) const;
    QByteArray img2Buffer(const LbaPalette &pal) const;

    LbaPalette mPalette;
    QImage     mImage;
};




#endif // LBASPRITE_H
