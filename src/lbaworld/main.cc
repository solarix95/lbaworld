

#include <QApplication>

#include "lbwscreen.h"
#include "lbwconsole.h"

int main(int argc, char *argv[]) {
    QApplication app(argc,argv);

    LbwScreen  screen;
    LbwConsole console;

    screen.show();

    return app.exec();
}
