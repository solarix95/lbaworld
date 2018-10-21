#ifndef LBASPRITE_H
#define LBASPRITE_H

#include <QByteArray>
#include <QImage>
#include <lbapalette.h>

class LbaSprite
{
public:
    LbaSprite(const LbaPalette &pal, const QByteArray &buffer = QByteArray());

    bool fromBuffer(const QByteArray &buffer);

    const QImage &image() const;

private:
    LbaPalette mPalette;
    QImage     mImage;
};




#endif // LBASPRITE_H
