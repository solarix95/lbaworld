#include <iostream>
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <hqrfile.h>
#include <QDir>

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

    QStringList args = app.arguments();
    args.takeFirst(); // remove appname
    foreach(QString arg, args) {
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
int  hqrCompress(const QString &hqrFileName, const QStringList &inputFiles)
{
    if (QFile::exists(hqrFileName)) {
        std::cerr << "HQR file already exists: " << hqrFileName.toUtf8().data() << std::endl;
        return 2;
    }

    HqrFile f;
    foreach(QString nextFile, inputFiles) {
        QFile inputFile(nextFile);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            std::cerr << "Error opening/reading: " << nextFile.toUtf8().data() << std::endl;
            continue;
        }

        std::cout << "Append " << nextFile.toUtf8().data() << std::endl;
        f.appendBlock(inputFile.readAll());
    }

    QFile outHqr(hqrFileName);
    if (!outHqr.open(QIODevice::WriteOnly)) {
        std::cerr << "Error opening/writing: " << hqrFileName.toUtf8().data() << std::endl;
        return 3;
    }

    std::cout << "Compressing..." << std::flush;
    f.setVerbose(true);
    outHqr.write(f.toByteArray(2));
    std::cout << "done" << std::endl;
    std::cout << "Compressed to: " << hqrFileName.toUtf8().data() << std::endl;

    return 0;
}

//-------------------------------------------------------------------------------------------------
int  hqrExtract(const QString &hqrFileName, const QString &outDir)
{
    HqrFile f;

    if (!f.fromFile(hqrFileName)) {
        std::cerr << "Invalid HQR file: " << hqrFileName.toUtf8().data() << std::endl;
        return 2;
    }

    QDir dir;
    if (!dir.mkpath(outDir)) {
        std::cerr << "Invalid output dir: " << outDir.toUtf8().data() << std::endl;
        return 3;
    }

    for (int i=0; i<f.count(); i++) {
        QFile outFile(outDir + QDir::separator() + QString("%1.bin").arg(i,4,10,QChar('0')));
        if (!outFile.open(QIODevice::WriteOnly)) {
            std::cerr << "Cannot create file: " << outFile.fileName().toUtf8().data() << std::endl;
            return 4;
        }
        std::cout << "Extract to: " << outFile.fileName().toUtf8().data() << std::endl;
        outFile.write(f.block(i));
        outFile.close();
    }
    return 0;
}
