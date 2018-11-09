#ifndef LBAIMAGE_H
#define LBAIMAGE_H

#include <QByteArray>
#include <QImage>
#include <lbapalette.h>

class LbaImage
{
public:
    LbaImage(const QByteArray &buffer = QByteArray());

    bool fromBuffer(const QByteArray &buffer);

    const QImage &image() const;

private:
    QImage mImage;
};

#endif // LBAIMAGE_H
