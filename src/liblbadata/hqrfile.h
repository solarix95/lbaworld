#ifndef LBA_HQR_FILE_H
#define LBA_HQR_FILE_H

#include <QByteArray>
#include <QList>
#include "binaryreader.h"

class HqrFile
{
public:
     HqrFile(const QString &filename = "");
     virtual ~HqrFile();

     bool fromFile(const QString &filename);
     bool fromBuffer(const QByteArray &buffer);

     int  count() const;

     const QByteArray &block(int index) const;

private:
     void       readHqrBlock(int index);
     QByteArray decompressEntry(const QByteArray &inBuffer, qint32 decompsize, qint32 mode);

     BinaryReader      mBuffer;
     QList<QByteArray> mBlocks;
};

#endif
