#include <QApplication>
#include <hqrfile.h>
#include <lbapalette.h>
#include <lbaress.h>
#include <mapviewer.h>
#include <lbacharacter.h>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    LbaRess ress;
    ress.init();

    MapViewer viewer(ress);
    viewer.show();

    return app.exec();
}
