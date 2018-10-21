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
 : mLbaRess(NULL), mLbaBodys(NULL), mLbaAnims(NULL), mLba3d(NULL), mLbaFlaSamples(NULL), mLbaSprites(NULL)
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
    QString lbadatadir = findLbaData();

    QDir dir(lbadatadir); // empty "lbadatadir" -> current working dir
    dir.setFilter(QDir::Files);

    QFileInfoList list = dir.entryInfoList();

    processFiles(list);
}

//-------------------------------------------------------------------------------------------------
int LbaRess::bodyCount() const
{
    return mLbaBodys ? mLbaBodys->count() : 0;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::bodyData(int index) const
{
    if (!mLbaBodys)
        return QByteArray();

    if (index < 0 || index >= mLbaBodys->count())
        return QByteArray();

    return mLbaBodys->block(index);
}

//-------------------------------------------------------------------------------------------------
int LbaRess::animCount() const
{
    return mLbaAnims ? mLbaAnims->count() : 0;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::animData(int index) const
{
    if (!mLbaAnims)
        return QByteArray();

    if (index < 0 || index >= mLbaAnims->count())
        return QByteArray();

    return mLbaAnims->block(index);
}

//-------------------------------------------------------------------------------------------------
int LbaRess::dddCount() const
{
    return mLba3d ? mLba3d->count() : 0;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::dddData(int index) const
{
    if (!mLba3d)
        return QByteArray();

    if (index < 0 || index >= mLba3d->count())
        return QByteArray();

    return mLba3d->block(index);
}

//-------------------------------------------------------------------------------------------------
int LbaRess::ressCount() const
{
    return mLbaRess ? mLbaRess->count() : 0;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::ressData(int index) const
{
    if (!mLbaRess)
        return QByteArray();

    if (index < 0 || index >= mLbaRess->count())
        return QByteArray();

    return mLbaRess->block(index);
}

//-------------------------------------------------------------------------------------------------
int LbaRess::spritesCount() const
{
    return mLbaSprites ? mLbaSprites->count() : 0;
}

//-------------------------------------------------------------------------------------------------
QByteArray LbaRess::spriteData(int index) const
{
    if (!mLbaSprites)
        return QByteArray();

    if (index < 0 || index >= mLbaSprites->count())
        return QByteArray();

    return mLbaSprites->block(index);
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
QByteArray LbaRess::flaSample(int index) const
{
    Q_ASSERT(index >= 0);
    if (!mLbaFlaSamples || index >= mLbaFlaSamples->count())
        return QByteArray();

    return mLbaFlaSamples->block(index);
}

//-------------------------------------------------------------------------------------------------
QString LbaRess::findLbaData() const
{
    QString dir;

    // First priority: program args
    foreach(QString nextArg, qApp->arguments()) {
        if (nextArg.startsWith("--lbabase=")) {
            dir = nextArg.split("=")[1];
            if (QFile::exists(dir))
                return dir;
        }
    }

    // 2nd prio: ENV
    QProcessEnvironment env;
    dir = env.value("LBABASE","lbabase"); // 3rd prio: relative folder "lbabase"
    if (QFile::exists(dir))
        return dir;

    return "";
}

//-------------------------------------------------------------------------------------------------
void LbaRess::processFiles(const QFileInfoList &files)
{
    for (int i=0; i<files.count(); i++) {
        QString next = files[i].fileName().toLower();
        if (next.endsWith(".fla"))
            if (!mLbaFlas.contains(next))
                mLbaFlas[next] = files[i].absoluteFilePath();

        if (next == "flasamp.hqr")
            mLbaFlaSamples = new HqrFile(files[i].absoluteFilePath());

        if (next == "ress.hqr" && !mLbaRess)
            mLbaRess = new HqrFile(files[i].absoluteFilePath());

        if (next == "body.hqr" && !mLbaBodys)
            mLbaBodys = new HqrFile(files[i].absoluteFilePath());

        if (next == "anim.hqr" && !mLbaAnims)
            mLbaAnims = new HqrFile(files[i].absoluteFilePath());

        if (next == "sprites.hqr" && !mLbaSprites)
            mLbaSprites = new HqrFile(files[i].absoluteFilePath());

        if (next == "file3d.hqr" && !mLba3d)
            mLba3d = new HqrFile(files[i].absoluteFilePath());
    }
}

