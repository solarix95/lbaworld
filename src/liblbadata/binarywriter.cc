#include "binarywriter.h"
#include <QtEndian>

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
    append(quint16(d));
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(quint16 d)
{
    const quint16 value = qToLittleEndian(d);
    mBuffer.append(reinterpret_cast<const char *>(&value), sizeof(value));
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(qint32 d)
{
    append(quint32(d));
}

//-------------------------------------------------------------------------------------------
void BinaryWriter::append(quint32 d)
{
    const quint32 value = qToLittleEndian(d);
    mBuffer.append(reinterpret_cast<const char *>(&value), sizeof(value));
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
