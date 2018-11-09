#ifndef BINARYREADER_H
#define BINARYREADER_H

#include <QByteArray>

class BinaryReader
{
public:
    BinaryReader(const QByteArray &buffer = QByteArray());

    void setBuffer(const QByteArray &buffer);
    void clear();
    int  pos()  const;
    int  size() const;
    bool atEnd() const;
    bool error() const;

    bool read(void* destPtr, int size);

    qint8  readInt8();
    quint8 readUint8();
    qint16 readInt16();
    quint16 readUint16();

    qint32  getInt32(int pos) const;
    quint32 getUint32(int pos) const;

    bool skip(int bytes);
    bool seek(int pos);
    QByteArray readBlock(int size);


private:
    quint8        *tail() const;

    QByteArray    mBuffer;
    const quint8 *mCursor;
    bool          mError;
};

#endif // BINARYREADER_H
