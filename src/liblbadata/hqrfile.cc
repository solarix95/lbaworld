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
QByteArray HqrFile::toByteArray() const
{
    if (mBlocks.isEmpty())
        return QByteArray();

    QList<QByteArray> compressedBlocks;
    const int mode = 1;
    for (int i=0; i<mBlocks.count(); i++) {
        compressedBlocks << compressEntry(mBlocks[i], mode);
    }

    Q_ASSERT(compressedBlocks.size() == mBlocks.size());

    // calculate start offset for each block:
    QList<qint32> offsets;
    qint32        startOfFirstBlock = mBlocks.size() * 4; // 4 bytes per block description
    offsets << startOfFirstBlock;

    for (int i=1; i<mBlocks.count(); i++) {
        offsets << (offsets[i-1] + compressedBlocks[i-1].size() + sizeof(quint32) + sizeof(quint32) + sizeof(quint16));
        //                                                        \  ................    HEADER ..................../
    }
    Q_ASSERT(offsets.count() == compressedBlocks.count());

    // Now serialize everything:
    BinaryWriter outBuffer;

    foreach(qint32 offset, offsets)
        outBuffer.append(offset);

    for (int i=0; i<mBlocks.count(); i++) {
        // HEADER
        outBuffer.append((quint32)mBlocks[i].size());          // real size
        outBuffer.append((quint32)compressedBlocks[i].size()); // compressed size
        outBuffer.append((quint16)mode);                       // mode
        // Compressed resource
        outBuffer.append(compressedBlocks[i]);
    }
    return outBuffer.buffer();
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
void HqrFile::appendBlock(const QByteArray &inData)
{
    mBlocks << inData;
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

    if (realSize == 0) {
        mBlocks << QByteArray();
        return;
    }

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

                //     8Bits +   8Bits
                //  0000 0000  0000 0000
                //|  Distance      | length
                //     3x4bits     |  1x4Bits
                offset = *(quint16*)(src);
                src += 2;
                lenght = (offset & 0x0F) + (mode + 1);
                // qDebug() << "Distance" << (offset >> 4);
                ptr    = dst - (offset >> 4) - 1;
                for (i = 0; i < lenght; i++)
                    *(dst++) = *(ptr++);
            } else {
                lenght = 1;
                *(dst++) = *(src++);
            }
            decompsize -= lenght;
            if (decompsize < 0)
                Q_ASSERT(0);
            Q_ASSERT(decompsize >= 0); // negative: ptr/dst are invalide pointers!
            if (decompsize == 0)
                return outBuffer;
        }
    } while (decompsize);

    return outBuffer;
}

//-------------------------------------------------------------------------------------------
qint32 HqrFile::compressNextBlock(const char *src, const char *pos, int len, qint32 mode, quint16 &distance) const
{
    // Lets find a position between "src" and "pos":
    //  * string "pos" + len
    //  * maximum distance to "pos": 0xFFF (we only have 3 bytes for distance)
    //  * maximum length: 0xF + mode + 1
    //  * minimum length: 3 (2 bytes for compression metadata, with 3 bytes we win..)

    if ((pos - src) < 8) // no compression if we have less than 8 bytes "dictionary"
        return 1;

    if (len < 3)         // needle to small.. better no compression
        return 1;

    const char *bestPos    = NULL;
    int         bestLength = 0;
    const char *nextPos    = pos - 3;
    int         maxCompareLength;
    int         maxCompressionLength = 0xF + mode + 1;
    int         cl; // "current length
    while (nextPos >= src) {

        maxCompareLength = pos - nextPos;
        if (len < maxCompareLength)
            maxCompareLength = len;
        if (len < maxCompareLength)
            maxCompareLength = len;
        if (maxCompressionLength < maxCompareLength)
            maxCompareLength = maxCompressionLength;
        for (cl = 3; cl <= maxCompareLength; cl++) {
            if (!memcmp(nextPos,pos,cl)) {
                if (cl > bestLength) {
                    bestLength = cl;
                    bestPos    = nextPos;
                }
            }
        }

        if (bestLength == maxCompressionLength) // found biggest possible block!
            break;

        nextPos--;

        if ((nextPos - src) > 0xFFF) // reach max distance
            break;
    }

    if (!bestLength)
        return 1;

    distance = pos - bestPos;

    /*
    qDebug() << "Compressed String";
    for (int i=0; i< (qMin(bestLength,5)); i++) {
        qDebug() << (int)(*(bestPos+i));
    }
    */

    return bestLength;
}

//-------------------------------------------------------------------------------------------
QByteArray HqrFile::compressEntry(const QByteArray &inBuffer, qint32 mode) const
{
    BinaryWriter outBuffer;
    BinaryWriter subBuffer;

    const char *src      = inBuffer.constData();
    const char *srcStart = inBuffer.constData();
    int compressLength = inBuffer.length();

    quint8  nextBlock  = 0x01;
    quint8  nextBlocks = 0x00;
    quint16 offset;
    quint16 distance;

    while (compressLength > 0) {
        qint32 nextLen = compressNextBlock(srcStart,src,compressLength,mode, distance);
        if (nextLen == 1) { // handle uncompressed byte
            nextBlocks |= nextBlock;
            subBuffer.append(*src);
        } else {            // handle compressed block
            offset = nextLen - mode - 1;
            Q_ASSERT(offset <= 0x0F);
            offset |= ((distance-1) << 4);
            subBuffer.append(offset);
        }
        compressLength -= nextLen;
        src += nextLen;
        if (nextBlock == 0x80) { // bit 8: 10000000
            nextBlocks |= nextBlock;
            outBuffer.append(nextBlocks);
            outBuffer.append(subBuffer.buffer());

            // Reset:
            nextBlock  = 0x01;
            nextBlocks = 0x00;
            subBuffer.clear();
        } else {
            nextBlock = nextBlock << 1;
        }
    }

    // Handle tail:
    if (subBuffer.size() > 0) {
        nextBlocks |= nextBlock; // last unwritten block
        while (nextBlock != 0x80) {
            nextBlock = nextBlock << 1;
            nextBlocks |= nextBlock;
        }
        outBuffer.append(nextBlocks);
        outBuffer.append(subBuffer.buffer());
    }

    return outBuffer.buffer();
}

