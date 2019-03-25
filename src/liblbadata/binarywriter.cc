#include "binarywriter.h"

//-------------------------------------------------------------------------------------------
BinaryWriter::BinaryWriter()
{
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::clear()
{
    mBuffer.clear();
}

//-------------------------------------------------------------------------------------------
int BinaryWriter::size() const
{
    return mBuffer.size();
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(char d)
{
    mBuffer.append(d);
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(qint8 d)
{
    mBuffer.append((char)d);
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(quint8 d)
{
    mBuffer.append((char)d);
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(qint16 d)
{
    mBuffer.append((const char *)&d,2); // TODO: Write little endian
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(quint16 d)
{
    mBuffer.append((const char *)&d,2); // TODO: Write little endian
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(qint32 d)
{
    mBuffer.append((const char *)&d,4); // TODO: Write little endian
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(quint32 d)
{
    mBuffer.append((const char *)&d,4); // TODO: Write little endian
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(const QByteArray &d)
{
    mBuffer.append(d);
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::skip(int bytes)
{
    while (bytes > 0) {
        mBuffer.append('\0');
        bytes--;
    }
}

//-------------------------------------------------------------------------------------------
const QByteArray &BinaryWriter::buffer() const
{
    return mBuffer;
}
