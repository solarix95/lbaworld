#include <iostream>
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <lbapalette.h>
#include <QDir>

//-------------------------------------------------------------------------------------------------
void printUsage();
int  palConvert(const QString &fromFile, const QString &toFile);

//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);


    QStringList args = app.arguments();
    args.takeFirst(); // remove appname

    if (args.count() != 2) {
        printUsage();
        return 1;
    }

    int ret = palConvert(args[0], args[1]);
    if (!ret)
        std::cout << "done" << std::endl;

    return 0;
}

//-------------------------------------------------------------------------------------------------
void printUsage()
{
    std::cout << "pal2gimp [lbaworld]: converts LBA-Palettes to GIMP and vice versa" << std::endl;
    std::cout << "usage: pal2gimp <from-file> <to-file>" << std::endl << std::endl;

    std::cout << "Examples" << std::endl;
    std::cout << "$ pal2gimp 0001.bin LBA1_palette.gpl"  << std::endl;
    std::cout << "$ pal2gimp LBA1_palette.gpl  0001.bin" << std::endl;
}

//-------------------------------------------------------------------------------------------------
int  palConvert(const QString &fromFile, const QString &toFile)
{
   LbaPalette pal;
   if (!pal.fromFile(fromFile)) {
       std::cerr << "Error opening/loading " << fromFile.toUtf8().data() << std::endl;
       return 2;
   }

   if (!pal.toFile(toFile)) {
       std::cerr << "Error writing " << toFile.toUtf8().data() << std::endl;
       return 3;
   }

   return 0;
}

