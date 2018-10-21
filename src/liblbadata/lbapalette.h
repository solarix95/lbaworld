#ifndef LBAPALETTE_H
#define LBAPALETTE_H

#include <QVector>
#include <QByteArray>
#include <QColor>

class LbaPalette
{
public:
    LbaPalette(const QByteArray &buffer = QByteArray());

    bool fromBuffer(const QByteArray &buffer);
    const QVector<QRgb> &palette() const;

private:
    QVector<QRgb> mPalette;
};

#endif // LBAPALETTE_H
