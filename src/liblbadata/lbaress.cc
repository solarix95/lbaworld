#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QUrl>

#include <limits>

#include "lbaress.h"
#include "lbaresourcearchive.h"

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
        qDeleteAll(mArchives[source]);
        mArchives[source].clear();
        mDataPaths[source].clear();
        mErrors[source].clear();
    }
    mLbaFlas.clear();
    mLbaTracks.clear();
    mErrorString.clear();
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
    const int archiveCountBefore = mArchives[source].count();
    processDir(mDataPaths[source], source);
    processLooseArchives(mDataPaths[source], source);
    return mArchives[source].count() > archiveCountBefore;
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::loadArchive(Source source, Content content, const QString &filename)
{
    return loadArchive(source, contentName(content), filename);
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::loadArchive(Source source, const QString &archiveName, const QString &filename)
{
    if (!validSource(source))
        return false;

    LbaHqrArchive *nextArchive = new LbaHqrArchive();
    if (!nextArchive->open(filename)) {
        addError(source, tr("Cannot open HQR archive '%1': %2")
                 .arg(filename, nextArchive->errorString()));
        delete nextArchive;
        return false;
    }

    return installArchive(source, archiveName, nextArchive, true);
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::loadDirectoryArchive(Source source, const QString &archiveName,
                                   const QString &directory)
{
    if (!validSource(source))
        return false;

    LbaDirectoryArchive *nextArchive = new LbaDirectoryArchive();
    if (!nextArchive->open(directory)) {
        addError(source, tr("Cannot open resource directory '%1': %2")
                 .arg(directory, nextArchive->errorString()));
        delete nextArchive;
        return false;
    }

    return installArchive(source, archiveName, nextArchive, true);
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::hasArchive(Source source, Content content) const
{
    return hasArchive(source, contentName(content));
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::hasArchive(Source source, const QString &archiveName) const
{
    return resourceArchive(source, archiveName) != 0;
}

//-------------------------------------------------------------------------------------------------
const HqrFile *LbaRess::archive(Source source, Content content) const
{
    const LbaResourceArchive *value = resourceArchive(source, contentName(content));
    return value ? value->hqrFile() : 0;
}

//-------------------------------------------------------------------------------------------------
const LbaResourceArchive *LbaRess::resourceArchive(Source source,
                                                    const QString &archiveName) const
{
    if (!validSource(source))
        return 0;
    return mArchives[source].value(canonicalArchiveName(archiveName), 0);
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::archivePath(Source source, Content content) const
{
    return archivePath(source, contentName(content));
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::archivePath(Source source, const QString &archiveName) const
{
    const LbaResourceArchive *value = resourceArchive(source, archiveName);
    return value ? value->sourcePath() : QString();
}

//-------------------------------------------------------------------------------------------------
QStringList LbaRess::archiveNames(Source source) const
{
    return validSource(source) ? mArchives[source].keys() : QStringList();
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
    for (ArchiveRegistry::const_iterator it = mArchives[source].constBegin();
         it != mArchives[source].constEnd(); ++it) {
        foreach (const QString &error, it.value()->validationErrors()) {
            const QString decorated = QString("%1: %2").arg(it.key(), error);
            if (!result.contains(decorated))
                result << decorated;
        }
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
int LbaRess::count(Source source, Content content) const
{
    return count(source, contentName(content));
}

//-------------------------------------------------------------------------------------------------
int LbaRess::count(Source source, const QString &archiveName) const
{
    const LbaResourceArchive *value = resourceArchive(source, archiveName);
    return value ? value->count() : 0;
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::hasData(Source source, const QString &archiveName, int index) const
{
    const LbaResourceArchive *value = resourceArchive(source, archiveName);
    return value && value->hasEntry(index);
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::data(Source source, Content content, int index, bool *ok) const
{
    return data(source, contentName(content), index, ok);
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::data(Source source, const QString &archiveName, int index, bool *ok) const
{
    if (ok)
        *ok = false;
    const QString canonicalName = canonicalArchiveName(archiveName);
    const LbaResourceArchive *value = resourceArchive(source, canonicalName);
    if (!value) {
        setError(tr("Resource archive '%1' is not loaded").arg(canonicalName));
        return QByteArray();
    }

    bool dataOk = false;
    const QByteArray result = value->data(index, &dataOk);
    if (!dataOk)
        setError(tr("Cannot read %1 entry %2: %3")
                 .arg(canonicalName).arg(index).arg(value->errorString()));
    else
        mErrorString.clear();
    if (ok)
        *ok = dataOk;
    return result;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::data(Source source, const QString &archiveName,
                         const QString &identifier, bool *ok) const
{
    bool indexOk = false;
    const int index = entryIndex(source, archiveName, identifier, &indexOk);
    if (!indexOk) {
        if (ok)
            *ok = false;
        return QByteArray();
    }
    return data(source, archiveName, index, ok);
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::data(const QString &url, bool *ok) const
{
    Source source;
    QString archiveName;
    QString identifier;
    if (!fromUrl(url, source, archiveName, identifier)) {
        if (ok)
            *ok = false;
        setError(tr("Invalid resource URL: '%1'").arg(url));
        return QByteArray();
    }
    return data(source, archiveName, identifier, ok);
}

//-------------------------------------------------------------------------------------------------
int LbaRess::entryIndex(Source source, const QString &archiveName,
                        const QString &identifier, bool *ok) const
{
    if (ok)
        *ok = false;

    bool numeric = false;
    const int numericIndex = identifier.toInt(&numeric);
    if (numeric && numericIndex >= 0) {
        if (!hasData(source, archiveName, numericIndex)) {
            setError(tr("Resource entry %1 does not exist in '%2'")
                     .arg(numericIndex).arg(canonicalArchiveName(archiveName)));
            return -1;
        }
        if (ok)
            *ok = true;
        mErrorString.clear();
        return numericIndex;
    }

    bool manifestOk = false;
    const QByteArray manifestData = data(source, archiveName, 0, &manifestOk);
    if (!manifestOk)
        return -1;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(manifestData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setError(tr("Invalid manifest in '%1': %2")
                 .arg(canonicalArchiveName(archiveName), parseError.errorString()));
        return -1;
    }

    const QJsonObject manifest = document.object();
    QJsonObject entries;
    if (manifest.value("entries").isObject())
        entries = manifest.value("entries").toObject();
    else
        entries = manifest; // Also accept the compact { "name": index } form.

    QJsonValue mappedValue = entries.value(identifier);
    if (mappedValue.isUndefined()) {
        for (QJsonObject::const_iterator it = entries.constBegin(); it != entries.constEnd(); ++it) {
            if (it.key().compare(identifier, Qt::CaseInsensitive) == 0) {
                mappedValue = it.value();
                break;
            }
        }
    }

    int index = -1;
    if (mappedValue.isDouble()) {
        const double number = mappedValue.toDouble(-1);
        if (number >= 0 && number <= std::numeric_limits<int>::max()
                && number == int(number))
            index = int(number);
    } else if (mappedValue.isString()) {
        bool mappedOk = false;
        const int mappedIndex = mappedValue.toString().toInt(&mappedOk);
        if (mappedOk && mappedIndex >= 0)
            index = mappedIndex;
    }

    if (index < 0 || !hasData(source, archiveName, index)) {
        setError(tr("Resource name '%1' is not defined in '%2'")
                 .arg(identifier, canonicalArchiveName(archiveName)));
        return -1;
    }

    if (ok)
        *ok = true;
    mErrorString.clear();
    return index;
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::errorString() const
{
    return mErrorString;
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
    QString archiveName;
    if (!fromUrl(url, source, archiveName, ident))
        return false;
    return contentFromName(archiveName, content);
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::fromUrl(const QString &url, Source &source, QString &archiveName, QString &ident)
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

        archiveName = canonicalArchiveName(parsed.host());
        if (archiveName.isEmpty())
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

    archiveName = canonicalArchiveName(parts[1]);
    if (archiveName.isEmpty())
        return false;
    ident = parts[2];
    return !ident.isEmpty();
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::toUrl(Source source, Content content, int index)
{
    return toUrl(source, contentName(content), QString::number(index));
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::toUrl(Source source, const QString &archiveName,
                       const QString &identifier)
{
    return QString("%1://%2/%3").arg(sourceName(source),
                                     canonicalArchiveName(archiveName), identifier);
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
QString LbaRess::canonicalArchiveName(const QString &name)
{
    QString baseName = QFileInfo(name.trimmed()).fileName();
    if (baseName.endsWith(".hqr", Qt::CaseInsensitive))
        baseName.chop(4);
    baseName = baseName.trimmed().toLower();
    if (baseName.isEmpty())
        return QString();

    Content content;
    if (contentFromName(baseName, content))
        return contentName(content);

    // Archive names are URL host names as well as registry keys. Keep their
    // spelling predictable and reject path-like or otherwise ambiguous names.
    static const QRegularExpression validName("^[a-z0-9][a-z0-9_-]*$");
    return validName.match(baseName).hasMatch() ? baseName : QString();
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::validSource(Source source)
{
    return source >= LBA1 && source <= LBAW;
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::contentFromName(const QString &name, Content &content)
{
    QString normalized = QFileInfo(name).fileName().toLower();
    if (normalized.endsWith(".hqr"))
        normalized.chop(4);
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

        const QString archiveName = canonicalArchiveName(filename);
        if (archiveName.isEmpty() || mArchives[source].contains(archiveName))
            continue;

        LbaHqrArchive *archive = new LbaHqrArchive();
        if (!archive->open(fileInfo.absoluteFilePath())) {
            addError(source, tr("Cannot open HQR archive '%1': %2")
                     .arg(fileInfo.absoluteFilePath(), archive->errorString()));
            delete archive;
            continue;
        }
        installArchive(source, archiveName, archive, false);
    }
}

//-------------------------------------------------------------------------------------------------
void LbaRess::processLooseArchives(const QString &dirName, Source source)
{
    const QRegularExpression entryPattern("^\\d+(?:[-_.].*)?$");
    QDir root(dirName);
    const QFileInfoList directories = root.entryInfoList(
                QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                QDir::Name);

    foreach (const QFileInfo &directory, directories) {
        const QString archiveName = canonicalArchiveName(directory.fileName());
        if (archiveName.isEmpty())
            continue;

        bool containsIndexedEntry = false;
        const QFileInfoList files = QDir(directory.absoluteFilePath()).entryInfoList(
                    QDir::Files | QDir::Readable, QDir::Name);
        foreach (const QFileInfo &file, files) {
            if (entryPattern.match(file.fileName()).hasMatch()) {
                containsIndexedEntry = true;
                break;
            }
        }
        if (!containsIndexedEntry)
            continue;

        LbaDirectoryArchive *archive = new LbaDirectoryArchive();
        if (!archive->open(directory.absoluteFilePath())) {
            addError(source, tr("Cannot open resource directory '%1': %2")
                     .arg(directory.absoluteFilePath(), archive->errorString()));
            delete archive;
            continue;
        }
        installArchive(source, archiveName, archive, true);
    }
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::installArchive(Source source, const QString &archiveName,
                             LbaResourceArchive *archive, bool replace)
{
    if (!archive || !validSource(source)) {
        delete archive;
        return false;
    }

    const QString canonicalName = canonicalArchiveName(archiveName);
    if (canonicalName.isEmpty()) {
        addError(source, tr("Invalid resource archive name: '%1'").arg(archiveName));
        delete archive;
        return false;
    }

    if (mArchives[source].contains(canonicalName)) {
        if (!replace) {
            delete archive;
            return false;
        }
        delete mArchives[source].take(canonicalName);
    }
    mArchives[source].insert(canonicalName, archive);

    const QStringList archiveErrors = archive->validationErrors();
    foreach (const QString &error, archiveErrors)
        emit log(tr("%1: %2").arg(archive->sourcePath(), error));

    emit log(tr("Indexed '%1' as %2 (%3 entries, %4 backend)")
             .arg(archive->sourcePath(), canonicalName)
             .arg(archive->count()).arg(archive->backendName()));
    return true;
}

//-------------------------------------------------------------------------------------------------
void LbaRess::addError(Source source, const QString &error)
{
    if (validSource(source) && !mErrors[source].contains(error))
        mErrors[source] << error;
    emit log(error);
}

//-------------------------------------------------------------------------------------------------
void LbaRess::setError(const QString &error) const
{
    mErrorString = error;
}
