#include <QDebug>
#include <QStringList>
#include <QString>
#include <QApplication>
#include <QDesktopWidget>
#include "lbapalette.h"
#include "lbaress.h"
#include "flamovie.h"
#include "lbasprite.h"
#include "lbwaudio.h"
#include "lbwscreen.h"
#include "lbwmissioncontrol.h"

//-------------------------------------------------------------------------------------------------
LbwMissionControl::LbwMissionControl(LbaRess &ress)
 : mRess(ress), mAudio(NULL), mScreen(NULL), mCmdState(Execute)
{
    connect(&mSleepTimer, SIGNAL(timeout()), this, SLOT(sleepEnd()));
    setupHelp();
}

//-------------------------------------------------------------------------------------------------
LbwMissionControl::~LbwMissionControl()
{
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::registerAudio(LbwAudio *audioSystem)
{
    mAudio = audioSystem;
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::registerScreen(LbwScreen *screen)
{
    mScreen = screen;
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::init()
{
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::exec(const QString &cmd, const QStringList &args)
{
    mCmdQueue << QPair<QString, QStringList>(cmd, args);
    processCmdQueue();
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::sleepEnd()
{
    mCmdState = Execute;
    processCmdQueue();
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::processCmdQueue()
{
    while (mCmdState == Execute && !mCmdQueue.isEmpty()) {
        QPair<QString, QStringList> next = mCmdQueue.takeFirst();
        processCmd(next.first, next.second);
    }
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::processCmd(const QString &cmd, const QStringList &args)
{
    if (!QMetaObject::invokeMethod(this, cmd.toLower().toUtf8().data(), Qt::DirectConnection,
                              Q_ARG(QStringList,args)))
    {
        emit log(QString("#ff0000 Unknown command '%1'").arg(cmd));
    }
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::help(const QStringList &args)
{
    QString out;
    if (args.isEmpty()) {
        QMapIterator<QString, QString> i(mCommandHelp);
         while (i.hasNext()) {
             i.next();
             if (!out.isEmpty())
                 out += " | ";
             out += i.key();
         }
    } else {
        out = mCommandHelp.value(args.first(),"");
    }

    if (out.isEmpty()) {
        emit log("#FF0000 Nobody can help you :(");
        return;
    }

    emit log("#00FF00 " + out);
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::sleep(const QStringList &args)
{
    if (args.isEmpty())
        return;
    mSleepTimer.start(args.last().toInt());
    mCmdState = Sleeping;
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::playspl(const QStringList &args)
{
    Q_ASSERT(mAudio);
    if (args.count() == 0) return;

    LbaRess::Source  source;
    LbaRess::Content cont;
    QString          ident;
    if (!LbaRess::fromUrl(args.first(),source, cont, ident))
        return ;

    mAudio->playVoc(source,cont,ident.toInt());
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::playmus(const QStringList &args)
{
    Q_ASSERT(mAudio);
    if (args.count() == 0) return;

    mAudio->playMusic(args.last());
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::playfla(const QStringList &args)
{
    if (!mScreen || args.isEmpty())
        return;
    QString flaName = args.last();


    FlaMovie *movie = new FlaMovie();
    if (!movie->fromBuffer(mRess.fla(flaName))) {
        emit log("Invalid/unknown fla");
        delete movie;
        return;
    }

    if (mAudio)
        connect(movie, SIGNAL(playFlaSample(int,int)), mAudio, SLOT(playFlaVoc(int,int)));

    mScreen->fadeTo(movie);
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::quit(const QStringList &args)
{
    qApp->quit();
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::showposter(const QStringList &args)
{
    if (args.count() != 2)
        return;

    LbaRess::Source  source;
    LbaRess::Content cont;
    QString          ident;
    if (!LbaRess::fromUrl(args.first(),source, cont, ident)) {

        return;
    }

    LbaPalette pal;
    if (!pal.fromBuffer(mRess.data(source,cont,ident.toInt()))) {
        emit log(QString("Invalid/unknown palette '%1'").arg(args.first()));
        return;
    }

    if (!LbaRess::fromUrl(args.last(),source, cont, ident))
        return;

    LbaSprite sprite(pal,mRess.data(source,cont,ident.toInt()),LbaSprite::AutoSprite);

    QImage img = sprite.image();
    if (img.isNull()) {
        emit log(QString("Invalid/unknown image '%1'").arg(args.last()));
        return;
    }

    mScreen->fadeTo(img);
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::showstatus(const QStringList &args)
{
    if (!mScreen || args.isEmpty())
        return;
    mScreen->setStatusText(args.last());
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::setwindowrect(const QStringList &args)
{
    if (!mScreen)
        return;

    if (args.count() == 2) { // Size only -> center
        int deskW = qApp->desktop()->geometry().width();
        int deskH = qApp->desktop()->geometry().height();
        int scrW  = args[0].toInt();
        int scrH  = args[1].toInt();
        mScreen->setGeometry(deskW/2 - scrW/2,deskH/2-scrH/2,scrW,scrH);
    }

    if (args.count() == 4) {
        mScreen->setGeometry(args[0].toInt(), args[1].toInt(),args[2].toInt(), args[3].toInt());
    }
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::printfla(const QStringList &)
{
    emit log(QString("FLAs: %1").arg(mRess.flas().join(",")));
}

//-------------------------------------------------------------------------------------------------
void LbwMissionControl::setupHelp()
{
    mCommandHelp["URL"]           = "LBA Resource URL: <version>/<hqr-source>/<index>";
    mCommandHelp["playspl"]       = "playspl <URL>";
    mCommandHelp["showstatus"]    = "showstatus <string>";
    mCommandHelp["setwindowrect"] = "setwindowrect [ x y ] width height";
}

