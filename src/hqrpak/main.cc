#include <QCoreApplication>
#include <QString>
#include <QStringList>

//-------------------------------------------------------------------------------------------------
void printUsage();
int  hqrCompress(const QString &hqrFile, const QStringList &inputFiles);
int  hqrExtract(const QString &hqrFile, const QString &outDir);

//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);

    int mode = -1;
    QString      target;
    QStringList  vars;

    foreach(QString arg, app.arguments()) {
        if (arg == "-e")
            mode = 1; // Extract
        else if (arg == "-c")
            mode = 2; // Compress
        else if (target.isEmpty())
            target = arg;
        else
            vars << arg;
    }

    if (vars.isEmpty()) {
        printUsage();
        return 1;
    }

    switch (mode) {
    case 1: return hqrExtract(target,vars.first());
    case 2: return hqrCompress(target,vars);
    }

    printUsage();
    return 1;
}


//-------------------------------------------------------------------------------------------------
void printUsage()
{

}

//-------------------------------------------------------------------------------------------------
int  hqrCompress(const QString &hqrFile, const QStringList &inputFiles)
{
    return 0;
}

//-------------------------------------------------------------------------------------------------
int  hqrExtract(const QString &hqrFile, const QString &outDir)
{
    return 0;
}
