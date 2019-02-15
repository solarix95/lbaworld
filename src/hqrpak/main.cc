#include <QCoreApplication>

//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);

    int mode = -1;

    foreach(QString arg, app.arguments()) {
        if (arg == "e" || arg == "-e")
            mode = 1;
    }
    return 0;
}
