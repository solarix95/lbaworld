#include "binaryreader.h"

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
    mCursor = (quint8*)buffer.constData();
}

//-------------------------------------------------------------------------------------------
void BinaryReader::clear()
{
    mCursor = NULL;
    mBuffer.clear();
}

//-------------------------------------------------------------------------------------------
int BinaryReader::pos() const
{
    if (!mCursor)
        return -1;
    return mCursor - (quint8*)mBuffer.constData();
}

//-------------------------------------------------------------------------------------------
int BinaryReader::size() const
{
    return mBuffer.size();
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::atEnd() const
{
    return !mCursor || (mCursor > tail());
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::error() const
{
    return mError;
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::read(void *destPtr, int size)
{
    if ((mCursor + size) > tail())
        return false;
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
    qint16 ret;
    bool done = read(&ret,2);
    ERROR(done);
    return ret;
}

//-------------------------------------------------------------------------------------------
quint16 BinaryReader::readUint16()
{
    quint16 ret;
    bool done = read(&ret,2);
    ERROR(done);
    return ret;
}

//-------------------------------------------------------------------------------------------
qint32 BinaryReader::getInt32(int pos) const
{
    Q_ASSERT(pos >= 0 && pos < mBuffer.length());
    return *(qint32*)(mBuffer.constData() + pos);
}

//-------------------------------------------------------------------------------------------
quint32 BinaryReader::getUint32(int pos) const
{
    Q_ASSERT(pos >= 0 && pos < mBuffer.length());
    return *(quint32*)(mBuffer.constData() + pos);
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::skip(int size)
{
    if ((mCursor + size) > tail())
        return false;
    mCursor += size;
    return true;
}

//-------------------------------------------------------------------------------------------
bool BinaryReader::seek(int pos)
{
    if (pos >= mBuffer.length())
        return false;
    mCursor = (quint8*)mBuffer.constData() + pos;

    return true;
}

//-------------------------------------------------------------------------------------------
QByteArray BinaryReader::readBlock(int size)
{
    QByteArray block(size,'\0');
    read(block.data(),size);
    return block;
}

//-------------------------------------------------------------------------------------------
quint8 *BinaryReader::tail() const
{
    Q_ASSERT(mCursor);
    return (quint8*)(mBuffer.constData() + mBuffer.length());
}

