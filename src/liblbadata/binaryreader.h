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

    bool read(void* destPtr, int size);
    quint8 readUint8();
    qint16 readInt16();

    bool skip(int bytes);
    bool seek(int pos);
    QByteArray readBlock(int size);


private:
    quint8        *tail() const;

    QByteArray    mBuffer;
    const quint8 *mCursor;
};

#endif // BINARYREADER_H
