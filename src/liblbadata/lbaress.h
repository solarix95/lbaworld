#ifndef LBARESS_H
#define LBARESS_H

#include <QByteArray>
#include <QFileInfoList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include "hqrfile.h"

class LbaRess : public QObject
{
    Q_OBJECT
public:
    explicit LbaRess(QObject *parent = 0);
    virtual ~LbaRess();

    enum Source {
        LBA1 = 0,
        LBA2 = 1,
        LBAW = 2
    };

    enum Content {
        Body,
        StaticObjs,
        Anim,
        File3d,
        Ress,
        Sprites,
        SpritesRaw,
        FlaSmpl,
        Samples
    };

    typedef QMap<Content,HqrFile*> LbaContent;

    // Discover data through --lba1base/--lba2base/--lbawbase, matching
    // environment variables, or relative directories with these names.
    void init();
    void clear();

    // Explicit runtime configuration. Directories are scanned recursively;
    // archives remain on disk and their blocks are decompressed lazily.
    bool loadFromDirectory(Source source, const QString &dirName);
    bool loadArchive(Source source, Content content, const QString &filename);

    bool           hasArchive(Source source, Content content) const;
    const HqrFile *archive(Source source, Content content) const;
    QString        archivePath(Source source, Content content) const;
    QString        dataPath(Source source) const;
    QStringList    validationErrors(Source source) const;

    int        count(Source source, Content content) const;
    QByteArray data(Source source, Content content, int index, bool *ok = 0) const;

    QStringList tracks() const;
    QString     pathByTrack(const QString &track) const;

    QStringList flas() const;
    QByteArray  fla(const QString &name) const;

    // Accepts both the legacy form "1/spl/42" and canonical runtime URLs
    // such as "lba1://samples/42" or "lba1://SAMPLES.HQR/42".
    static bool    fromUrl(const QString &url, Source &source, Content &content, QString &ident);
    static QString toUrl(Source source, Content content, int index);
    static QString sourceName(Source source);
    static QString contentName(Content content);

signals:
    void log(const QString &msg);

private:
    static bool validSource(Source source);
    static bool contentFromName(const QString &name, Content &content);

    QString findLbaData(const QString &sourceName) const;
    void    processDir(const QString &dirName, Source source);
    void    processFiles(const QFileInfoList &files, Source source);
    void    addError(Source source, const QString &error);

    LbaContent  mContent[3];
    QString     mDataPaths[3];
    QStringList mErrors[3];
    QMap<QString,QString> mLbaFlas;
    QMap<QString,QString> mLbaTracks;
};

#endif // LBARESS_H
