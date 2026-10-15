#ifndef LBA_HQR_FILE_H
#define LBA_HQR_FILE_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include "binarywriter.h"

class HqrFile
{
public:
     struct EntryInfo {
         EntryInfo();

         quint32 offset;
         quint32 decompressedSize;
         quint32 compressedSize;
         quint16 compressionMode;
         bool    available;
         bool    valid;
         QString error;
     };

     explicit HqrFile(const QString &filename = QString());
     virtual ~HqrFile();

     void       setVerbose(bool v);
     bool       fromFile(const QString &filename);
     bool       fromBuffer(const QByteArray &buffer);
     QByteArray toByteArray(int mode = 1) const;
     int        count() const;

     bool        isOpen() const;
     bool        isValid() const;
     QString     filename() const;
     QString     errorString() const;
     QStringList validationErrors() const;
     bool        validate(QStringList *errors = 0, bool verifyData = true) const;

     EntryInfo   entryInfo(int index) const;
     bool        hasBlock(int index) const;
     QByteArray  data(int index, bool *ok = 0) const;
     int         cachedBlockCount() const;
     void        clearCache() const;

     const QByteArray &block(int index) const;
     void  appendBlock(const QByteArray &inData);

private:
     struct EntryState {
         EntryState();

         EntryInfo info;
         mutable QByteArray data;
         mutable QString runtimeError;
         mutable bool loaded;
         bool sourceBacked;
     };

     void       reset();
     bool       readIndex(qint64 sourceSize);
     bool       readSource(qint64 offset, qint64 size, QByteArray &result) const;
     bool       loadBlock(int index) const;
     void       addValidationError(int index, const QString &error);

     QByteArray decompressEntry(const QByteArray &inBuffer, qint32 decompsize, qint32 mode,
                                bool *ok, QString *error) const;
     QByteArray compressEntry(const QByteArray &inBuffer, qint32 mode) const;
     qint32     findThisBlockNearPosition(const char *src, const char *pos, int len, qint32 mode, quint16 &distance) const;

     bool              mVerbose;
     bool              mOpen;
     qint64            mSourceSize;
     QString           mFilename;
     QByteArray        mSourceBuffer;
     QList<EntryState> mEntries;
     QStringList       mValidationErrors;
     mutable QString   mErrorString;
};

#endif
