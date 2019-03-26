#ifndef LBA_HQR_FILE_H
#define LBA_HQR_FILE_H

#include <QByteArray>
#include <QList>
#include "binaryreader.h"
#include "binarywriter.h"

class HqrFile
{
public:
     HqrFile(const QString &filename = "");
     virtual ~HqrFile();

     bool       fromFile(const QString &filename);
     bool       fromBuffer(const QByteArray &buffer);
     QByteArray toByteArray() const;
     int        count() const;

     const QByteArray &block(int index) const;
     void  appendBlock(const QByteArray &inData);

private:
     void       readHqrBlock(int index);

     QByteArray decompressEntry(const QByteArray &inBuffer, qint32 decompsize, qint32 mode);
     QByteArray compressEntry(const QByteArray &inBuffer, qint32 mode) const;
     qint32     compressNextBlock(const char *src, const char *pos, int len, qint32 mode, quint16 &distance) const;

     BinaryReader      mBuffer;
     QList<QByteArray> mBlocks;
};

#endif
