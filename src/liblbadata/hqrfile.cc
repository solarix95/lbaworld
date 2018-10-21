#include <QDebug>
#include <QFile>
#include "hqrfile.h"

//-------------------------------------------------------------------------------------------
HqrFile::HqrFile(const QString &filename)
{
    if (!filename.isEmpty())
        fromFile(filename);
}

//-------------------------------------------------------------------------------------------
HqrFile::~HqrFile()
{

}

//-------------------------------------------------------------------------------------------
bool HqrFile::fromFile(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QByteArray buffer = f.readAll();
    if (buffer.length() <= 0)
        return false;

    return fromBuffer(buffer);
}

//-------------------------------------------------------------------------------------------
bool HqrFile::fromBuffer(const QByteArray &buffer)
{
    quint32 headerSize;

    mBuffer.setBuffer(buffer);
    mBuffer.read(&headerSize,4);

    mBlocks.clear();
    for (int i=0; i<((int)headerSize/4); i++)
        readHqrBlock(i);

    return true;
}

//-------------------------------------------------------------------------------------------
int HqrFile::count() const
{
    return mBlocks.count();
}

//-------------------------------------------------------------------------------------------
const QByteArray &HqrFile::block(int index) const
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < mBlocks.count());
    return mBlocks[index];
}

//-------------------------------------------------------------------------------------------
void HqrFile::readHqrBlock(int index)
{
    quint32 offsetToData;
    quint32 realSize;
    quint32 compSize;
    quint16 mode;

    mBuffer.seek(index*4);
    mBuffer.read(&offsetToData, 4);

    mBuffer.seek(offsetToData);
    mBuffer.read(&realSize, 4);
    mBuffer.read(&compSize, 4);
    mBuffer.read(&mode, 2);

    QByteArray block = mBuffer.readBlock(compSize);
    switch (mode) {
      case 0 : break;
      case 1 : block = decompressEntry(block,realSize,mode); break;
      case 2 : block = decompressEntry(block,realSize,mode); break;
      default:
        qWarning() << "HqrFile::readHqrBlock" << index << mode;
    }

    mBlocks << block;
}

//-------------------------------------------------------------------------------------------
QByteArray HqrFile::decompressEntry(const QByteArray &inBuffer, qint32 decompsize, qint32 mode)
{
    quint8 b;
    qint32 lenght, d, i;
    quint16 offset;
    quint8 *ptr;

    QByteArray outBuffer(decompsize,'\0');
    quint8 *dst = (quint8*)outBuffer.data();
    quint8 *src = (quint8*)inBuffer.constData();

    // Decompress entry based in Yaz0r and Zink decompression code
    do {
        b = *(src++);
        for (d = 0; d < 8; d++) {
            if (!(b & (1 << d))) {
                offset = *(quint16*)(src);
                src += 2;
                lenght = (offset & 0x0F) + (mode + 1);
                ptr = dst - (offset >> 4) - 1;
                for (i = 0; i < lenght; i++)
                    *(dst++) = *(ptr++);
            } else {
                lenght = 1;
                *(dst++) = *(src++);
            }
            decompsize -= lenght;
            if (decompsize <= 0)
                return outBuffer;
        }
    } while (decompsize);

    return outBuffer;
}
