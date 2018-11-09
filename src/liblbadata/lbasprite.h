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
        Image
    };

    LbaSprite(const LbaPalette &pal, const QByteArray &buffer = QByteArray(), Type spriteType = Sprite);

    bool fromSpriteBuffer(const QByteArray &buffer);
    bool fromRawSpriteBuffer(const QByteArray &buffer);
    bool fromImageBuffer(const QByteArray &buffer);

    const QImage &image() const;

private:
    LbaPalette mPalette;
    QImage     mImage;
};




#endif // LBASPRITE_H
