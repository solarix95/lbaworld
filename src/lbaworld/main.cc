

#include <QApplication>

#include "lbaress.h"
#include "lbwscreen.h"
#include "lbwconsole.h"
#include "lbwaudio.h"
#include "lbwsdlaudio.h"

int main(int argc, char *argv[]) {
    QApplication app(argc,argv);


    LbaRess     resources;   // body.hqr, ....
    LbwSdlAudio audioSystem(resources);
    LbwConsole  console;
    LbwScreen   screen(&console);

    QObject::connect(&resources, SIGNAL(log(QString)), &console, SLOT(addOutput(QString)));
    QObject::connect(&audioSystem, SIGNAL(log(QString)), &console, SLOT(addOutput(QString)));

    resources.init();
    audioSystem.init();

    screen.show();

    return app.exec();
}
