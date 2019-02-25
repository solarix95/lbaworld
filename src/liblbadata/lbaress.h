#ifndef LBARESS_H
#define LBARESS_H

#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QFileInfoList>
#include <QMap>
#include <QObject>

class HqrFile;
class LbaRess : public QObject
{
    Q_OBJECT
public:
    LbaRess();
    virtual ~LbaRess();

    void init();

    enum Source {
        LBA1 = 0,   // Datasource: LBA1
        LBA2 = 1,   // Datasource: LBA2
        LBAW = 2    // Datasource: LBA World: additional open source game data
    };

    enum Content {
        Body,        // Body.hqr
        StaticObjs,  // LBA1: Invobj.hqr, LBA2: Objfix.hqr
        Anim,        // Anim.hqr
        File3d,      // LBA1: File3d.hqr
        Ress,        // Ress.hqr
        Sprites,     // Sprites.hqr
        SpritesRaw,  // LBA2: Spriraw.hqr
        FlaSmpl,     // LBA1: FLASAMP.HQR
        Samples      // LBA1: SAMPLES.HQR
    };

    typedef QMap<Content,HqrFile*> LbaContent;

    int        count(Source source, Content content) const;
    QByteArray data(Source source, Content content, int index) const;

    // Files..
    QStringList tracks() const; // ... from steam (
    QString     pathByTrack(const QString &track) const;

    QStringList flas() const;
    QByteArray  fla(const QString &name) const;

    static bool fromUrl(const QString &url, LbaRess::Source &s, LbaRess::Content &cont, QString &ident);

signals:
    void log(const QString &msg);

private:
    QString findLbaData(const QString &sourcename) const;
    void    processDir(const QString &dirName, Source source);
    void    processFiles(const QFileInfoList &files, Source source);

    LbaContent  mContent[3]; // LBA1, LBA2, LBAWorld

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

    QMap<QString,QString> mLbaFlas;   // LBA Cinematics
    QMap<QString,QString> mLbaTracks;

    HqrFile    *mLbaFlaSamples;
};

#endif // LBARESS_H
