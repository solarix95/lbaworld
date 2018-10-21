#include <QApplication>
#include <lbaress.h>
#include "lbamoviewidget.h"
#include "lbasfmlaudio.h"
#include "lbasdlaudio.h"
#include "lbaflaplayer.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    LbaRess ress;
    ress.init();

    // LbaSfmlAudio audio;
    LbaSdlAudio audio(ress);

    LbaFlaPlayer player(ress);

    QObject::connect(&player, SIGNAL(playSample(int,int)), &audio, SLOT(playFlaVoc(int,int)));

    player.show();
    player.initFromCli();

    return app.exec();
}
