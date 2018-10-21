#include <QApplication>
#include <lbaress.h>
#include "lbaspriteplayer.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    LbaRess ress;
    ress.init();

    LbaSpritePlayer player(ress);
    player.show();

    return app.exec();
}
