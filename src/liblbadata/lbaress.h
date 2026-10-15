#ifndef LBARESS_H
#define LBARESS_H

#include <QByteArray>
#include <QFileInfoList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include "hqrfile.h"

class LbaResourceArchive;

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

    // Kept as a source-compatibility alias for users of the former public API.
    typedef QMap<Content,HqrFile*> LbaContent;
    typedef QMap<QString,LbaResourceArchive*> ArchiveRegistry;

    // Discover data through --lba1base/--lba2base/--lbawbase, matching
    // environment variables, or relative directories with these names.
    void init();
    void clear();

    // Explicit runtime configuration. Directories are scanned recursively;
    // packed archives remain on disk and their blocks are decompressed lazily.
    // Direct child directories containing indexed files are loose development
    // archives and override a packed archive with the same name.
    bool loadFromDirectory(Source source, const QString &dirName);
    bool loadArchive(Source source, Content content, const QString &filename);
    bool loadArchive(Source source, const QString &archiveName, const QString &filename);
    bool loadDirectoryArchive(Source source, const QString &archiveName,
                              const QString &directory);

    bool           hasArchive(Source source, Content content) const;
    bool           hasArchive(Source source, const QString &archiveName) const;
    const HqrFile *archive(Source source, Content content) const;
    const LbaResourceArchive *resourceArchive(Source source,
                                               const QString &archiveName) const;
    QString        archivePath(Source source, Content content) const;
    QString        archivePath(Source source, const QString &archiveName) const;
    QStringList    archiveNames(Source source) const;
    QString        dataPath(Source source) const;
    QStringList    validationErrors(Source source) const;

    int        count(Source source, Content content) const;
    int        count(Source source, const QString &archiveName) const;
    bool       hasData(Source source, const QString &archiveName, int index) const;
    QByteArray data(Source source, Content content, int index, bool *ok = 0) const;
    QByteArray data(Source source, const QString &archiveName, int index,
                    bool *ok = 0) const;
    QByteArray data(Source source, const QString &archiveName,
                    const QString &identifier, bool *ok = 0) const;
    QByteArray data(const QString &url, bool *ok = 0) const;
    int        entryIndex(Source source, const QString &archiveName,
                          const QString &identifier, bool *ok = 0) const;
    QString    errorString() const;

    QStringList tracks() const;
    QString     pathByTrack(const QString &track) const;

    QStringList flas() const;
    QByteArray  fla(const QString &name) const;

    // Accepts both the legacy form "1/spl/42" and canonical runtime URLs
    // such as "lba1://samples/42" or "lba1://SAMPLES.HQR/42".
    static bool    fromUrl(const QString &url, Source &source, Content &content, QString &ident);
    static bool    fromUrl(const QString &url, Source &source, QString &archiveName,
                           QString &ident);
    static QString toUrl(Source source, Content content, int index);
    static QString toUrl(Source source, const QString &archiveName,
                         const QString &identifier);
    static QString sourceName(Source source);
    static QString contentName(Content content);
    static QString canonicalArchiveName(const QString &name);

signals:
    void log(const QString &msg);

private:
    static bool validSource(Source source);
    static bool contentFromName(const QString &name, Content &content);

    QString findLbaData(const QString &sourceName) const;
    void    processDir(const QString &dirName, Source source);
    void    processFiles(const QFileInfoList &files, Source source);
    void    processLooseArchives(const QString &dirName, Source source);
    bool    installArchive(Source source, const QString &archiveName,
                           LbaResourceArchive *archive, bool replace);
    void    addError(Source source, const QString &error);
    void    setError(const QString &error) const;

    ArchiveRegistry mArchives[3];
    QString     mDataPaths[3];
    QStringList mErrors[3];
    QMap<QString,QString> mLbaFlas;
    QMap<QString,QString> mLbaTracks;
    mutable QString mErrorString;
};

#endif // LBARESS_H
