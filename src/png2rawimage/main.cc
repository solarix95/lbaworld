#include <iostream>
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <lbapalette.h>
#include <lbasprite.h>
#include <QDir>

//-------------------------------------------------------------------------------------------------
void printUsage();
int  pngConvert(const QString &fromFile, const QString &palette, const QString &toFile);

//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);


    QStringList args = app.arguments();
    args.takeFirst(); // remove appname

    if (args.count() != 3) {
        printUsage();
        return 1;
    }

    int ret = pngConvert(args[0], args[1], args[2]);
    if (!ret)
        std::cout << "done" << std::endl;

    return 0;
}

//-------------------------------------------------------------------------------------------------
void printUsage()
{
    std::cout << "png2rawimage [lbaworld]: convert png-Images to LBA1-Rawimages and vice versa" << std::endl;
    std::cout << "usage1: png2rawimage <from-png> <to-palette> <to-raw-file>" << std::endl;
    std::cout << "usage2: png2rawimage <from-raw-image> <from-palette> <to-png-file>" << std::endl << std::endl;

    std::cout << "Examples" << std::endl;
    std::cout << "$ png2rawimage 0001.png LBA1_palette.gpl 0001.bin"  << std::endl;
    std::cout << "$ png2rawimage 0001.bin LBA1_palette.gpl 0001.png"  << std::endl;
}

//-------------------------------------------------------------------------------------------------
int  pngConvert(const QString &fromFile, const QString &palette, const QString &toFile)
{
   LbaPalette pal;
   if (!pal.fromFile(palette)) {
       std::cerr << "Error reading palette: " << palette.toUtf8().data() << std::endl;
       return 2;
   }

   LbaSprite sprite;
   if (!sprite.fromFile(fromFile,pal)) {
       std::cerr << "Error reading " << fromFile.toUtf8().data() << std::endl;
       return 3;
   }

   if (!sprite.toFile(toFile, pal)) {
       std::cerr << "Error writing " << toFile.toUtf8().data() << std::endl;
       return 4;
   }

   return 0;
}

