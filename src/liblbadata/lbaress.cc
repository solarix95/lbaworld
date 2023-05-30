#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QFileInfoList>
#include <QProcessEnvironment>
#include "hqrfile.h"
#include "lbaress.h"

//-------------------------------------------------------------------------------------------------
LbaRess::LbaRess()
 : mLbaRess(NULL), mLbaBodys(NULL), mLbaInventoryObjects(NULL), mLbaAnims(NULL), mLba3d(NULL), mLbaFlaSamples(NULL), mLbaSprites(NULL)
{    
}

//-------------------------------------------------------------------------------------------------
LbaRess::~LbaRess()
{
    if (mLbaFlaSamples)
        delete mLbaFlaSamples;

    if (mLbaRess)
        delete mLbaRess;

    if (mLbaBodys)
        delete mLbaBodys;

    if (mLbaInventoryObjects)
        delete mLbaInventoryObjects;

    if (mLbaAnims)
        delete mLbaAnims;

    if (mLbaSprites)
        delete mLbaSprites;

    if (mLba3d)
        delete mLba3d;
}

//-------------------------------------------------------------------------------------------------
void LbaRess::init()
{
    processDir(findLbaData("lba1base"), LBA1);
    processDir(findLbaData("lba2base"), LBA2);
    processDir(findLbaData("lbawbase"), LBAW);
}

//-------------------------------------------------------------------------------------------------
int LbaRess::count(LbaRess::Source source, LbaRess::Content content) const
{
    if (mContent[source][content])
        return mContent[source][content]->count();
    return 0;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::data(LbaRess::Source source, LbaRess::Content content, int index) const
{
    if (index >= count(source,content))
        return QByteArray();
    return mContent[source][content]->block(index);
}

//-------------------------------------------------------------------------------------------------
QStringList LbaRess::tracks() const
{
    return mLbaTracks.keys();
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::pathByTrack(const QString &track) const
{
    return mLbaTracks.value(track,"");
}

//-------------------------------------------------------------------------------------------------
QStringList LbaRess::flas() const
{
    return mLbaFlas.keys();
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::fla(const QString &name) const
{
    QFile f(mLbaFlas[name]);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();

    return f.readAll();
}

//-------------------------------------------------------------------------------------------------
bool LbaRess::fromUrl(const QString &url, LbaRess::Source &s, LbaRess::Content &cont, QString &ident)
{
    QStringList parts = url.split("/");
    if (parts.count() != 3) return false;

    if (parts[0] == "1")
        s = LbaRess::LBA1;
    else if (parts[0] == "2")
        s = LbaRess::LBA2;
    else if (parts[0] == "w")
        s = LbaRess::LBAW;
    else
        return false;

    if (parts[1] == "spl")
        cont = LbaRess::Samples;
    else if (parts[1] == "flaspl")
        cont = LbaRess::FlaSmpl;
    else if (parts[1] == "ress")
        cont = LbaRess::Ress;
    else if (parts[1] == "spr")
        cont = LbaRess::Sprites;
    else
        return false;

    ident = parts[2];
    return true;
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::findLbaData(const QString &sourcename) const
{
    QString dir;

    // First priority: program args
    foreach(QString nextArg, qApp->arguments()) {
        if (nextArg.startsWith(QString("--%1=").arg(sourcename.toLower()))) {
            dir = nextArg.split("=")[1];
            if (QFile::exists(dir))
                return dir;
        }
    }

    // 2nd prio: ENV
    QProcessEnvironment env;
    dir = env.value(sourcename.toUpper(),sourcename.toLower()); // 3rd prio: relative folder e.g. "lbabase"
    if (QFile::exists(dir))
        return dir;

    return "";
}

//-------------------------------------------------------------------------------------------------
void LbaRess::processDir(const QString &dirName, Source source)
{
    if (dirName.isEmpty())
        return;

    QDir dir(dirName);

    dir.setFilter(QDir::Files);

    QStringList filters;
    filters << "*.hqr" << "*.fla" << "*.mp3" << "*.ogg" << "*.wav";
    dir.setNameFilters(filters);

    QFileInfoList list = dir.entryInfoList();

    processFiles(list, source);

    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    list = dir.entryInfoList();
    foreach(QFileInfo nextDir, list) {
        processDir(nextDir.absoluteFilePath(), source);
    }
}

//-------------------------------------------------------------------------------------------------
void LbaRess::processFiles(const QFileInfoList &files, Source source)
{
    for (int i=0; i<files.count(); i++) {
        QString next = files[i].fileName().toLower();
        if (next.endsWith(".fla"))
            if (!mLbaFlas.contains(next))
                mLbaFlas[next] = files[i].absoluteFilePath();

        if (next == "flasamp.hqr" && !mContent[source][FlaSmpl]) {
            emit log(tr("process '%1'").arg(files[i].absoluteFilePath()));
            mContent[source][FlaSmpl] = new HqrFile(files[i].absoluteFilePath());
        }

        if (next == "samples.hqr" && !mContent[source][Samples]) {
            emit log(tr("process '%1'").arg(files[i].absoluteFilePath()));
            mContent[source][Samples] = new HqrFile(files[i].absoluteFilePath());
        }

        if (next == "ress.hqr" && !mContent[source][Ress]) {
            emit log(tr("process '%1'").arg(files[i].absoluteFilePath()));
            mContent[source][Ress] = new HqrFile(files[i].absoluteFilePath());
        }

        if (next == "body.hqr" && !mContent[source][Body]) {
            emit log(tr("process '%1'").arg(files[i].absoluteFilePath()));
            mContent[source][Body] = new HqrFile(files[i].absoluteFilePath());
        }

        if (next == "invobj.hqr" && !mContent[source][StaticObjs]) {                        // LBA1 only
            emit log(tr("process '%1'").arg(files[i].absoluteFilePath()));
            mContent[source][StaticObjs] = new HqrFile(files[i].absoluteFilePath());
        }

        if (next == "objfix.hqr" && !mContent[source][StaticObjs])                        // LBA2 only
            mContent[source][StaticObjs] = new HqrFile(files[i].absoluteFilePath());

        if (next == "anim.hqr" && !mContent[source][Anim])
            mContent[source][Anim] = new HqrFile(files[i].absoluteFilePath());

        if (next == "sprites.hqr" && !mContent[source][Sprites])
            mContent[source][Sprites] = new HqrFile(files[i].absoluteFilePath());

        if (next == "spriraw.hqr" && !mContent[source][SpritesRaw])
            mContent[source][SpritesRaw] = new HqrFile(files[i].absoluteFilePath());

        if (next == "file3d.hqr" && !mContent[source][File3d])
            mContent[source][File3d] = new HqrFile(files[i].absoluteFilePath());

        if (next.endsWith(".ogg"))
            mLbaTracks[next.mid(0,next.length()-4)] = files[i].absoluteFilePath();
    }
}

