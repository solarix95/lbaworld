#ifndef LBARESS_H
#define LBARESS_H

#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QFileInfoList>
#include <QMap>

class HqrFile;
class LbaRess
{
public:
    LbaRess();
    virtual ~LbaRess();

    void init();

    /*
       TODO: replace methods with

    int        count(type) const;
    QByteArray data(type, int index) const;

    */

    // Body.HQR
    int        bodyCount() const;
    QByteArray bodyData(int index) const;

    // Invobj.hqr
    int        invCount() const;
    QByteArray invData(int index) const;

    // ANIM.HQR
    int        animCount() const;
    QByteArray animData(int index) const;

    // File3D.HQR
    int        dddCount() const;
    QByteArray dddData(int index) const;

    // RESS.HQR
    int        ressCount() const;
    QByteArray ressData(int index) const;

    // Sprites.hqr -> http://lbafileinfo.kazekr.net/index.php?title=LBA1:Sprites.hqr
    int        spritesCount() const;
    QByteArray spriteData(int index) const;

    // Files..
    QStringList tracks() const; // ... from steam (

    QStringList flas() const;
    QByteArray  fla(const QString &name) const;
    QByteArray  flaSample(int index) const; // from FLASAMP.HQR

private:
    QString findLbaData() const;
    void    processDir(const QString &dirName);
    void    processFiles(const QFileInfoList &files);


    QString     mLbaRessFile;        // Ress.hqr, Palette,
    HqrFile    *mLbaRess;

    QString     mLbaBodyFile;        // Body.hqr, Twinsen, Mega Penguin, ....
    HqrFile    *mLbaBodys;

    QString     mLbaInvobjFile;       // Invobj.hqr
    HqrFile    *mLbaInventoryObjects;

    QString     mLbaAnimFile;        // ANIM.HQR, Movement of Twinsen, Mega Penguin, ....
    HqrFile    *mLbaAnims;

    QString     mLba3dFile;          // FILE3D.HQR, Character Metadata
    HqrFile    *mLba3d;

    HqrFile    *mLbaSprites;        // Sprites.hqr

    QString     mLbaSamplesFile;    // Samples.hqr, VOC-Sounds

    QStringList mLbaTracks;         // Steam Tracks
    QMap<QString,QString> mLbaFlas;  // LBA Cinematics
    HqrFile    *mLbaFlaSamples;
};

#endif // LBARESS_H
