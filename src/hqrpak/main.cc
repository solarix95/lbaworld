#include <iostream>
#include <iomanip>
#include <QCoreApplication>
#include <QFile>
#include <QSaveFile>
#include <QString>
#include <QStringList>
#include <hqrfile.h>
#include <QDir>

//-------------------------------------------------------------------------------------------------
void printUsage();
int  hqrCompress(const QString &hqrFile, const QStringList &inputFiles);
int  hqrExtract(const QString &hqrFile, const QString &outDir);
int  hqrList(const QString &hqrFile);
int  hqrValidate(const QString &hqrFile);

//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);

    enum Mode { NoMode, Extract, Compress, List, Validate };
    Mode mode = NoMode;
    QString      target;
    QStringList  vars;

    QStringList args = app.arguments();
    args.takeFirst(); // remove appname
    foreach(QString arg, args) {
        if (arg == "-e" || arg == "--extract")
            mode = Extract;
        else if (arg == "-c" || arg == "--create")
            mode = Compress;
        else if (arg == "-l" || arg == "--list")
            mode = List;
        else if (arg == "-t" || arg == "--validate")
            mode = Validate;
        else if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        }
        else if (target.isEmpty())
            target = arg;
        else
            vars << arg;
    }

    if (target.isEmpty()) {
        printUsage();
        return 1;
    }

    switch (mode) {
    case Extract:
        if (!vars.isEmpty())
            return hqrExtract(target, vars.first());
        break;
    case Compress:
        if (!vars.isEmpty())
            return hqrCompress(target, vars);
        break;
    case List:     return hqrList(target);
    case Validate: return hqrValidate(target);
    case NoMode:   break;
    }

    printUsage();
    return 1;
}


//-------------------------------------------------------------------------------------------------
void printUsage()
{
    std::cout
        << "Usage:\n"
        << "  hqrpak --list <archive.hqr>\n"
        << "  hqrpak --validate <archive.hqr>\n"
        << "  hqrpak --extract <archive.hqr> <output-dir>\n"
        << "  hqrpak --create <archive.hqr> <entry-0> [entry-1 ...]\n"
        << "\nShort options: -l, -t, -e, -c\n";
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

    QSaveFile outHqr(hqrFileName);
    if (!outHqr.open(QIODevice::WriteOnly)) {
        std::cerr << "Error opening/writing: " << hqrFileName.toUtf8().data() << std::endl;
        return 3;
    }

    std::cout << "Compressing..." << std::flush;
    f.setVerbose(true);
    const QByteArray archiveData = f.toByteArray(2);
    if (archiveData.isEmpty() || outHqr.write(archiveData) != archiveData.size()
            || !outHqr.commit()) {
        std::cerr << "Error writing: " << hqrFileName.toUtf8().data() << std::endl;
        return 3;
    }
    std::cout << "done" << std::endl;
    std::cout << "Compressed to: " << hqrFileName.toUtf8().data() << std::endl;

    return 0;
}

//-------------------------------------------------------------------------------------------------
int  hqrExtract(const QString &hqrFileName, const QString &outDir)
{
    HqrFile f;

    if (!f.fromFile(hqrFileName)) {
        std::cerr << "Invalid HQR file: " << hqrFileName.toUtf8().data()
                  << ": " << f.errorString().toUtf8().data() << std::endl;
        return 2;
    }

    QDir dir;
    if (!dir.mkpath(outDir)) {
        std::cerr << "Invalid output dir: " << outDir.toUtf8().data() << std::endl;
        return 3;
    }

    for (int i=0; i<f.count(); i++) {
        bool ok = false;
        const QByteArray entry = f.data(i, &ok);
        if (!ok) {
            std::cerr << "Cannot decode entry " << i << ": "
                      << f.entryInfo(i).error.toUtf8().data() << std::endl;
            return 5;
        }
        QFile outFile(outDir + QDir::separator() + QString("%1.bin").arg(i,4,10,QChar('0')));
        if (!outFile.open(QIODevice::WriteOnly)) {
            std::cerr << "Cannot create file: " << outFile.fileName().toUtf8().data() << std::endl;
            return 4;
        }
        std::cout << "Extract to: " << outFile.fileName().toUtf8().data() << std::endl;
        if (outFile.write(entry) != entry.size()) {
            std::cerr << "Cannot write file: " << outFile.fileName().toUtf8().data() << std::endl;
            return 4;
        }
        outFile.close();
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------
int hqrList(const QString &hqrFileName)
{
    HqrFile archive;
    if (!archive.fromFile(hqrFileName)) {
        std::cerr << "Invalid HQR file: " << archive.errorString().toUtf8().data() << std::endl;
        return 2;
    }

    std::cout << "index  offset      stored      decoded  mode  state\n";
    for (int index = 0; index < archive.count(); ++index) {
        const HqrFile::EntryInfo info = archive.entryInfo(index);
        const char *state = !info.valid ? "invalid" : info.available ? "available" : "absent";
        std::cout << std::setw(5) << index
                  << std::setw(9) << info.offset
                  << std::setw(12) << info.compressedSize
                  << std::setw(13) << info.decompressedSize
                  << std::setw(6) << info.compressionMode
                  << "  " << state;
        if (!info.error.isEmpty())
            std::cout << " (" << info.error.toUtf8().constData() << ")";
        std::cout << '\n';
    }
    return archive.isValid() ? 0 : 2;
}

//-------------------------------------------------------------------------------------------------
int hqrValidate(const QString &hqrFileName)
{
    HqrFile archive;
    if (!archive.fromFile(hqrFileName)) {
        std::cerr << "Invalid HQR file: " << archive.errorString().toUtf8().data() << std::endl;
        return 2;
    }

    QStringList errors;
    if (!archive.validate(&errors, true)) {
        foreach (const QString &error, errors)
            std::cerr << error.toUtf8().constData() << '\n';
        return 2;
    }

    std::cout << "Valid HQR archive with " << archive.count() << " entries" << std::endl;
    return 0;
}
