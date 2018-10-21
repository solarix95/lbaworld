#include <QApplication>
#include <glwidget.h>
#include <hqrfile.h>
#include <lbapalette.h>
#include <lbaress.h>
#include <lbamodelviewer.h>
#include <lbacharacter.h>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    LbaRess ress;
    ress.init();

    LbaModelViewer viewer(ress);
    viewer.show();

    return app.exec();
}
