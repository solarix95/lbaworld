

#include <QApplication>

#include "lbwscreen.h"
#include "lbwconsole.h"

int main(int argc, char *argv[]) {
    QApplication app(argc,argv);


    LbwConsole console;
    LbwScreen  screen(&console);

    screen.show();

    return app.exec();
}
