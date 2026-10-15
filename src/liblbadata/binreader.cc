#include "binaryreader.h"
#include <QtEndian>
#include <cstring>

#define ERROR(c) if (!c) { mError = true; return 0; }

//-------------------------------------------------------------------------------------------
BinaryReader::BinaryReader(const QByteArray &buffer)
 : mCursor(NULL), mError(false)
{
    if (!buffer.isEmpty())
        setBuffer(buffer);
}

//-------------------------------------------------------------------------------------------
void BinaryReader::setBuffer(const QByteArray &buffer)
{
    mBuffer = buffer;
    mCursor = reinterpret_cast<const quint8 *>(mBuffer.constData());
    mError = false;
}

//-------------------------------------------------------------------------------------------
void BinaryReader::clear()
{
    mCursor = NULL;
    mBuffer.clear();
    mError = false;
}

//-------------------------------------------------------------------------------------------
int BinaryReader::pos() const
{
    if (!mCursor)
        return -1;
    return mCursor - reinterpret_cast<const quint8 *>(mBuffer.constData());
}

//-------------------------------------------------------------------------------------------
int BinaryReader::size() const
{
    return mBuffer.size();
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::atEnd() const
{
    return !mCursor || (mCursor >= tail());
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::error() const
{
    return mError;
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::read(void *destPtr, int size)
{
    if (!destPtr || !mCursor || size < 0 || size > (tail() - mCursor)) {
        mError = true;
        return false;
    }
    memcpy(destPtr,mCursor,size);
    mCursor += size;
    return true;
}

//-------------------------------------------------------------------------------------------
qint8 BinaryReader::readInt8()
{
    qint8 ret;
    bool done = read(&ret,1);
    ERROR(done);
    return ret;
}

//-------------------------------------------------------------------------------------------
quint8 BinaryReader::readUint8()
{
    quint8 ret;
    bool done = read(&ret,1);
    ERROR(done);
    return ret;
}

//-------------------------------------------------------------------------------------------
qint16 BinaryReader::readInt16()
{
    quint16 raw;
    bool done = read(&raw,2);
    ERROR(done);
    return qint16(qFromLittleEndian(raw));
}

//-------------------------------------------------------------------------------------------
quint16 BinaryReader::readUint16()
{
    quint16 raw;
    bool done = read(&raw,2);
    ERROR(done);
    return qFromLittleEndian(raw);
}

//-------------------------------------------------------------------------------------------
qint32 BinaryReader::getInt32(int pos) const
{
    if (pos < 0 || pos > mBuffer.length() - 4) {
        mError = true;
        return 0;
    }
    return qint32(qFromLittleEndian<quint32>(
                      reinterpret_cast<const uchar *>(mBuffer.constData() + pos)));
}

//-------------------------------------------------------------------------------------------
quint32 BinaryReader::getUint32(int pos) const
{
    if (pos < 0 || pos > mBuffer.length() - 4) {
        mError = true;
        return 0;
    }
    return qFromLittleEndian<quint32>(
                reinterpret_cast<const uchar *>(mBuffer.constData() + pos));
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::skip(int size)
{
    if (!mCursor || size < 0 || size > (tail() - mCursor)) {
        mError = true;
        return false;
    }
    mCursor += size;
    return true;
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::seek(int pos)
{
    if (pos < 0 || pos > mBuffer.length()) {
        mError = true;
        return false;
    }
    mCursor = reinterpret_cast<const quint8 *>(mBuffer.constData()) + pos;

    return true;
}

//-------------------------------------------------------------------------------------------
QByteArray BinaryReader::readBlock(int size)
{
    if (size < 0)
        return QByteArray();
    QByteArray block(size,'\0');
    if (!read(block.data(),size))
        return QByteArray();
    return block;
}

//-------------------------------------------------------------------------------------------
const quint8 *BinaryReader::tail() const
{
    return reinterpret_cast<const quint8 *>(mBuffer.constData() + mBuffer.length());
}
