#include <QApplication>

#include "lbaress.h"
#include "lbwscreen.h"
#include "lbwconsole.h"
#include "lbwaudio.h"
#include "lbwsdlaudio.h"
#include "lbwmissioncontrol.h"

int main(int argc, char *argv[]) {
    QApplication app(argc,argv);

    LbaRess           resources;   // body.hqr, ....
    LbwSdlAudio       audioSystem(resources);
    LbwConsole        console;
    LbwScreen         screen(&console);
    LbwMissionControl control(resources);

    control.registerAudio(&audioSystem);
    control.registerScreen(&screen);

    QObject::connect(&console,     SIGNAL(requestCmd(QString,QStringList)),
                     &control, SLOT(exec(QString,QStringList)));

    QObject::connect(&control,     SIGNAL(log(QString)),
                     &console, SLOT(addOutput(QString)));
    QObject::connect(&resources,   SIGNAL(log(QString)),
                     &console, SLOT(addOutput(QString)));
    QObject::connect(&audioSystem, SIGNAL(log(QString)),
                     &console, SLOT(addOutput(QString)));
    QObject::connect(&screen, SIGNAL(ready2run()),
                     &console, SLOT(init()));

    resources.init();
    audioSystem.init();

    screen.show();


    return app.exec();
}
