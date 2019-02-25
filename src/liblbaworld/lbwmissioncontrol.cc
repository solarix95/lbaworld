#include <QDebug>
#include <QStringList>
#include <QString>
#include <QApplication>
#include "lbapalette.h"
#include "lbaress.h"
#include "lbasprite.h"
#include "lbwaudio.h"
#include "lbwscreen.h"
#include "lbwmissioncontrol.h"

//-------------------------------------------------------------------------------------------------
LbwMissionControl::LbwMissionControl(LbaRess &ress)
 : mRess(ress), mAudio(NULL), mScreen(NULL)
{
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
    QMetaObject::invokeMethod(this, cmd.toUtf8().data(), Qt::DirectConnection,
                              Q_ARG(QStringList,args));
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
    if (!LbaRess::fromUrl(args.first(),source, cont, ident))
        return;

    LbaPalette pal;
    if (!pal.fromBuffer(mRess.data(source,cont,ident.toInt())))
        return;

    if (!LbaRess::fromUrl(args.last(),source, cont, ident))
        return;

    LbaSprite sprite(pal,mRess.data(source,cont,ident.toInt()),LbaSprite::AutoSprite);

    QImage img = sprite.image();
    if (img.isNull())
        return;

    mScreen->fadeTo(img);
}

