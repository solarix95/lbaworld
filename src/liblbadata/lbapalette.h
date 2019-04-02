#ifndef LBAPALETTE_H
#define LBAPALETTE_H

#include <QVector>
#include <QByteArray>
#include <QColor>

class LbaPalette
{
public:
    LbaPalette(const QByteArray &buffer = QByteArray());
    LbaPalette(const QVector<QRgb> &pal);

    bool fromBuffer(const QByteArray &buffer);
    bool fromFile(const QString &filename);
    QByteArray toBuffer() const;

    void setPalette(const QVector<QRgb> &pal);
    const QVector<QRgb> &palette() const;

    int  nearestIndexOf(QRgb c) const;

    bool       toFile(const QString &filename) const;
    QByteArray toGimpPalette(const QString &name) const;


private:
    bool fromLbaPalette(const QByteArray &buffer);
    bool fromGimpPalette(const QByteArray &buffer);

    QVector<QRgb> mPalette;
};

#endif // LBAPALETTE_H
