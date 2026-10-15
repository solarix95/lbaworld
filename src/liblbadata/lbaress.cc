#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QUrl>

#include "lbaress.h"

//-------------------------------------------------------------------------------------------------
LbaRess::LbaRess(QObject *parent)
 : QObject(parent)
{
}

//-------------------------------------------------------------------------------------------------
LbaRess::~LbaRess()
{
    clear();
}

//-------------------------------------------------------------------------------------------------
void LbaRess::clear()
{
    for (int source = LBA1; source <= LBAW; ++source) {
        qDeleteAll(mContent[source]);
        mContent[source].clear();
        mDataPaths[source].clear();
        mErrors[source].clear();
    }
    mLbaFlas.clear();
    mLbaTracks.clear();
}

//-------------------------------------------------------------------------------------------------
void LbaRess::init()
{
    clear();
    loadFromDirectory(LBA1, findLbaData("lba1base"));
    loadFromDirectory(LBA2, findLbaData("lba2base"));
    loadFromDirectory(LBAW, findLbaData("lbawbase"));
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::loadFromDirectory(Source source, const QString &dirName)
{
    if (!validSource(source) || dirName.isEmpty())
        return false;

    const QFileInfo info(dirName);
    if (!info.exists() || !info.isDir()) {
        addError(source, tr("Data directory does not exist: '%1'").arg(dirName));
        return false;
    }

    mDataPaths[source] = info.absoluteFilePath();
    const int archiveCountBefore = mContent[source].count();
    processDir(mDataPaths[source], source);
    return mContent[source].count() > archiveCountBefore;
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::loadArchive(Source source, Content content, const QString &filename)
{
    if (!validSource(source))
        return false;

    HqrFile *nextArchive = new HqrFile();
    if (!nextArchive->fromFile(filename)) {
        addError(source, tr("Cannot open HQR archive '%1': %2")
                 .arg(filename, nextArchive->errorString()));
        delete nextArchive;
        return false;
    }

    if (mContent[source].contains(content))
        delete mContent[source].take(content);
    mContent[source].insert(content, nextArchive);

    const QStringList archiveErrors = nextArchive->validationErrors();
    foreach (const QString &error, archiveErrors)
        emit log(tr("%1: %2").arg(filename, error));

    emit log(tr("Indexed '%1' (%2 entries)").arg(filename).arg(nextArchive->count()));
    return true;
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::hasArchive(Source source, Content content) const
{
    return archive(source, content) != 0;
}

//-------------------------------------------------------------------------------------------------
const HqrFile *LbaRess::archive(Source source, Content content) const
{
    if (!validSource(source))
        return 0;
    return mContent[source].value(content, 0);
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::archivePath(Source source, Content content) const
{
    const HqrFile *value = archive(source, content);
    return value ? value->filename() : QString();
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::dataPath(Source source) const
{
    return validSource(source) ? mDataPaths[source] : QString();
}

//-------------------------------------------------------------------------------------------------
QStringList LbaRess::validationErrors(Source source) const
{
    if (!validSource(source))
        return QStringList() << tr("Invalid LBA data source");

    QStringList result = mErrors[source];
    for (LbaContent::const_iterator it = mContent[source].constBegin();
         it != mContent[source].constEnd(); ++it) {
        foreach (const QString &error, it.value()->validationErrors()) {
            const QString decorated = QString("%1: %2").arg(contentName(it.key()), error);
            if (!result.contains(decorated))
                result << decorated;
        }
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
int LbaRess::count(Source source, Content content) const
{
    const HqrFile *value = archive(source, content);
    return value ? value->count() : 0;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::data(Source source, Content content, int index, bool *ok) const
{
    const HqrFile *value = archive(source, content);
    if (!value) {
        if (ok)
            *ok = false;
        return QByteArray();
    }
    return value->data(index, ok);
}

//-------------------------------------------------------------------------------------------------
QStringList LbaRess::tracks() const
{
    return mLbaTracks.keys();
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::pathByTrack(const QString &track) const
{
    return mLbaTracks.value(track.toLower(), QString());
}

//-------------------------------------------------------------------------------------------------
QStringList LbaRess::flas() const
{
    return mLbaFlas.keys();
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::fla(const QString &name) const
{
    QFile file(mLbaFlas.value(name.toLower()));
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    return file.readAll();
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::fromUrl(const QString &url, Source &source, Content &content, QString &ident)
{
    const QString trimmed = url.trimmed();
    if (trimmed.contains("://")) {
        const QUrl parsed(trimmed);
        const QString scheme = parsed.scheme().toLower();
        if (scheme == "lba1")
            source = LBA1;
        else if (scheme == "lba2")
            source = LBA2;
        else if (scheme == "lbaw")
            source = LBAW;
        else
            return false;

        if (!contentFromName(parsed.host(), content))
            return false;
        ident = parsed.path();
        while (ident.startsWith('/'))
            ident.remove(0, 1);
        return !ident.isEmpty();
    }

    const QStringList parts = trimmed.split('/', Qt::SkipEmptyParts);
    if (parts.count() != 3)
        return false;

    const QString sourcePart = parts[0].toLower();
    if (sourcePart == "1" || sourcePart == "lba1")
        source = LBA1;
    else if (sourcePart == "2" || sourcePart == "lba2")
        source = LBA2;
    else if (sourcePart == "w" || sourcePart == "lbaw")
        source = LBAW;
    else
        return false;

    if (!contentFromName(parts[1], content))
        return false;
    ident = parts[2];
    return !ident.isEmpty();
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::toUrl(Source source, Content content, int index)
{
    return QString("%1://%2/%3").arg(sourceName(source), contentName(content)).arg(index);
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::sourceName(Source source)
{
    switch (source) {
    case LBA1: return "lba1";
    case LBA2: return "lba2";
    case LBAW: return "lbaw";
    }
    return QString();
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::contentName(Content content)
{
    switch (content) {
    case Body:       return "body";
    case StaticObjs: return "objects";
    case Anim:       return "animations";
    case File3d:     return "file3d";
    case Ress:       return "resources";
    case Sprites:    return "sprites";
    case SpritesRaw: return "raw-sprites";
    case FlaSmpl:    return "fla-samples";
    case Samples:    return "samples";
    }
    return QString();
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::validSource(Source source)
{
    return source >= LBA1 && source <= LBAW;
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::contentFromName(const QString &name, Content &content)
{
    QString normalized = QFileInfo(name).completeBaseName().toLower();
    normalized.remove('-');
    normalized.remove('_');

    if (normalized == "body")
        content = Body;
    else if (normalized == "objects" || normalized == "staticobjs"
             || normalized == "invobj" || normalized == "objfix")
        content = StaticObjs;
    else if (normalized == "anim" || normalized == "animations")
        content = Anim;
    else if (normalized == "file3d" || normalized == "3d")
        content = File3d;
    else if (normalized == "ress" || normalized == "resources")
        content = Ress;
    else if (normalized == "spr" || normalized == "sprites")
        content = Sprites;
    else if (normalized == "spriraw" || normalized == "rawsprites")
        content = SpritesRaw;
    else if (normalized == "flaspl" || normalized == "flasamp" || normalized == "flasamples")
        content = FlaSmpl;
    else if (normalized == "spl" || normalized == "samples")
        content = Samples;
    else
        return false;
    return true;
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::findLbaData(const QString &sourceName) const
{
    const QCoreApplication *application = QCoreApplication::instance();
    if (application) {
        foreach (const QString &argument, application->arguments()) {
            const QString prefix = QString("--%1=").arg(sourceName.toLower());
            if (argument.startsWith(prefix)) {
                const QString directory = argument.mid(prefix.length());
                if (QFileInfo(directory).isDir())
                    return directory;
            }
        }
    }

    const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString directory = environment.value(sourceName.toUpper(), sourceName.toLower());
    return QFileInfo(directory).isDir() ? directory : QString();
}

//-------------------------------------------------------------------------------------------------
void LbaRess::processDir(const QString &dirName, Source source)
{
    QDir directory(dirName);
    directory.setFilter(QDir::Files | QDir::Readable);
    processFiles(directory.entryInfoList(), source);

    directory.setFilter(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    const QFileInfoList directories = directory.entryInfoList();
    foreach (const QFileInfo &nextDirectory, directories)
        processDir(nextDirectory.absoluteFilePath(), source);
}

//-------------------------------------------------------------------------------------------------
void LbaRess::processFiles(const QFileInfoList &files, Source source)
{
    foreach (const QFileInfo &fileInfo, files) {
        const QString filename = fileInfo.fileName().toLower();

        if (filename.endsWith(".fla") && !mLbaFlas.contains(filename))
            mLbaFlas.insert(filename, fileInfo.absoluteFilePath());

        if (filename.endsWith(".ogg") || filename.endsWith(".mp3") || filename.endsWith(".wav")) {
            const QString track = fileInfo.completeBaseName().toLower();
            if (!mLbaTracks.contains(track))
                mLbaTracks.insert(track, fileInfo.absoluteFilePath());
        }

        if (!filename.endsWith(".hqr"))
            continue;

        Content content;
        if (!contentFromName(filename, content) || mContent[source].contains(content))
            continue;

        loadArchive(source, content, fileInfo.absoluteFilePath());
    }
}

//-------------------------------------------------------------------------------------------------
void LbaRess::addError(Source source, const QString &error)
{
    if (validSource(source) && !mErrors[source].contains(error))
        mErrors[source] << error;
    emit log(error);
}
