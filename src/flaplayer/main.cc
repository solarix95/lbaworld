#include <QApplication>
#include <lbaress.h>
#include "lbamoviewidget.h"
#include "lbwsdlaudio.h"
#include "lbaflaplayer.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    LbaRess ress;
    ress.init();

    LbwSdlAudio audio(ress);
    LbaFlaPlayer player(ress);

    QObject::connect(&player, SIGNAL(playSample(int,int)), &audio, SLOT(playFlaVoc(int,int)));

    QObject::connect(&player, SIGNAL(stopAudio()), &audio, SLOT(stop()));

    player.show();
    player.initFromCli();

    return app.exec();
}
