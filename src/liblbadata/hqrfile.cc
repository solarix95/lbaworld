#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QtEndian>
#include <limits>
#include <iostream>
#include "hqrfile.h"

#define VERBOSE_COUT(s)   if (mVerbose) std::cout << s << std::endl

//-------------------------------------------------------------------------------------------
inline int memcmp2max(const char *str1, const char *str2, int maxLength)
{
    for (int i=0; i<maxLength; i++) {
        if (str1[i] != str2[i])
            return i;
    }

    return maxLength;
}

//-------------------------------------------------------------------------------------------
static quint16 readLittleEndian16(const QByteArray &buffer, int offset)
{
    return qFromLittleEndian<quint16>(
                reinterpret_cast<const uchar *>(buffer.constData() + offset));
}

//-------------------------------------------------------------------------------------------
static quint32 readLittleEndian32(const QByteArray &buffer, int offset)
{
    return qFromLittleEndian<quint32>(
                reinterpret_cast<const uchar *>(buffer.constData() + offset));
}

//-------------------------------------------------------------------------------------------
HqrFile::EntryInfo::EntryInfo()
 : offset(0), decompressedSize(0), compressedSize(0), compressionMode(0),
   available(false), valid(false)
{
}

//-------------------------------------------------------------------------------------------
HqrFile::EntryState::EntryState()
 : loaded(false), sourceBacked(false)
{
}

//-------------------------------------------------------------------------------------------
HqrFile::HqrFile(const QString &filename)
 : mVerbose(false), mOpen(false), mSourceSize(0)
{
    if (!filename.isEmpty())
        fromFile(filename);
}

//-------------------------------------------------------------------------------------------
HqrFile::~HqrFile()
{
}

//-------------------------------------------------------------------------------------------
void HqrFile::setVerbose(bool v)
{
    mVerbose = v;
}

//-------------------------------------------------------------------------------------------
bool HqrFile::fromFile(const QString &filename)
{
    reset();

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        mErrorString = QString("Cannot open '%1': %2").arg(filename, f.errorString());
        return false;
    }

    if (f.size() < 4) {
        mErrorString = QString("'%1' is too small to be an HQR archive").arg(filename);
        return false;
    }

    mFilename = QFileInfo(f).absoluteFilePath();
    mSourceSize = f.size();
    f.close();

    return readIndex(mSourceSize);
}

//-------------------------------------------------------------------------------------------
bool HqrFile::fromBuffer(const QByteArray &buffer)
{
    reset();
    if (buffer.size() < 4) {
        mErrorString = "Buffer is too small to be an HQR archive";
        return false;
    }

    mSourceBuffer = buffer;
    mSourceSize = buffer.size();
    return readIndex(mSourceSize);
}

//-------------------------------------------------------------------------------------------
void HqrFile::reset()
{
    mOpen = false;
    mSourceSize = 0;
    mFilename.clear();
    mSourceBuffer.clear();
    mEntries.clear();
    mValidationErrors.clear();
    mErrorString.clear();
}

//-------------------------------------------------------------------------------------------
bool HqrFile::readIndex(qint64 sourceSize)
{
    QByteArray firstWord;
    if (!readSource(0, 4, firstWord)) {
        mErrorString = "Cannot read the HQR header";
        return false;
    }

    const quint32 headerSize = readLittleEndian32(firstWord, 0);
    if (headerSize < 4 || (headerSize % 4) != 0 || headerSize > quint64(sourceSize)
            || headerSize > quint32(std::numeric_limits<int>::max())) {
        mErrorString = QString("Invalid HQR index size %1 for a %2 byte source")
                .arg(headerSize).arg(sourceSize);
        return false;
    }

    QByteArray indexBuffer;
    if (!readSource(0, headerSize, indexBuffer)) {
        mErrorString = "Cannot read the complete HQR index";
        return false;
    }

    const int offsetCount = int(headerSize / 4);
    // Original HQR files store one final offset pointing at EOF. Older
    // lbaworld-generated archives omitted it, so accept both layouts.
    const bool hasEndOffset = offsetCount > 1
            && readLittleEndian32(indexBuffer, (offsetCount - 1) * 4) == quint64(sourceSize);
    const int entryCount = offsetCount - (hasEndOffset ? 1 : 0);
    for (int index = 0; index < entryCount; ++index) {
        EntryState state;
        state.sourceBacked = true;
        state.info.offset = readLittleEndian32(indexBuffer, index * 4);

        // A zero offset is used by original archives for an intentionally
        // absent entry. It still occupies an index and is not an archive error.
        if (state.info.offset == 0) {
            state.info.valid = true;
            mEntries << state;
            continue;
        }

        state.info.available = true;
        if (state.info.offset < headerSize || quint64(state.info.offset) + 10 > quint64(sourceSize)) {
            state.info.error = QString("entry header offset %1 is outside the archive")
                    .arg(state.info.offset);
            mEntries << state;
            addValidationError(index, state.info.error);
            continue;
        }

        QByteArray entryHeader;
        if (!readSource(state.info.offset, 10, entryHeader)) {
            state.info.error = "cannot read entry header";
            mEntries << state;
            addValidationError(index, state.info.error);
            continue;
        }

        state.info.decompressedSize = readLittleEndian32(entryHeader, 0);
        state.info.compressedSize = readLittleEndian32(entryHeader, 4);
        state.info.compressionMode = readLittleEndian16(entryHeader, 8);

        if (state.info.compressionMode > 5) {
            state.info.error = QString("unsupported compression mode %1")
                    .arg(state.info.compressionMode);
        } else if (state.info.decompressedSize > quint32(std::numeric_limits<int>::max())
                   || state.info.compressedSize > quint32(std::numeric_limits<int>::max())) {
            state.info.error = "entry is too large for QByteArray";
        } else if (quint64(state.info.offset) + 10 + state.info.compressedSize > quint64(sourceSize)) {
            state.info.error = "compressed entry data extends beyond the archive";
        } else if (state.info.compressionMode == 0
                   && state.info.compressedSize != state.info.decompressedSize) {
            state.info.error = "uncompressed entry size does not match its stored size";
        } else {
            state.info.valid = true;
        }

        mEntries << state;
        if (!state.info.valid)
            addValidationError(index, state.info.error);
    }

    mOpen = true;
    return true;
}

//-------------------------------------------------------------------------------------------
bool HqrFile::readSource(qint64 offset, qint64 size, QByteArray &result) const
{
    result.clear();
    if (offset < 0 || size < 0 || offset > mSourceSize || size > mSourceSize - offset
            || size > std::numeric_limits<int>::max())
        return false;

    if (!mFilename.isEmpty()) {
        QFile file(mFilename);
        if (!file.open(QIODevice::ReadOnly) || !file.seek(offset))
            return false;
        result = file.read(size);
        return result.size() == size;
    }

    if (offset + size > mSourceBuffer.size())
        return false;
    result = mSourceBuffer.mid(int(offset), int(size));
    return result.size() == size;
}

//-------------------------------------------------------------------------------------------
QByteArray HqrFile::toByteArray(int mode) const
{
    if (mEntries.isEmpty())
        return QByteArray();

    if (mode <= 0 || mode > 5) // invalid modes are set to 1
        mode = 1;

    QList<QByteArray> compressedBlocks;
    QList<QByteArray> blocks;
    for (int i=0; i<mEntries.count(); i++) {
        VERBOSE_COUT("Compress entry " << i);
        bool ok = false;
        const QByteArray nextBlock = data(i, &ok);
        if (!ok)
            return QByteArray();
        blocks << nextBlock;
        compressedBlocks << compressEntry(nextBlock, mode);
    }

    Q_ASSERT(compressedBlocks.size() == blocks.size());

    // calculate start offset for each block:
    QList<quint32> offsets;
    quint64 nextOffset = quint64(blocks.size() + 1) * 4; // entries plus EOF sentinel
    for (int i = 0; i < blocks.count(); ++i) {
        if (nextOffset > std::numeric_limits<quint32>::max())
            return QByteArray();
        offsets << quint32(nextOffset);
        nextOffset += quint64(compressedBlocks[i].size())
                + sizeof(quint32) + sizeof(quint32) + sizeof(quint16);
    }
    if (nextOffset > std::numeric_limits<quint32>::max())
        return QByteArray();
    offsets << quint32(nextOffset);

    // Now serialize everything:
    BinaryWriter outBuffer;

    foreach(quint32 offset, offsets)
        outBuffer.append(offset);

    for (int i=0; i<blocks.count(); i++) {
        // HEADER
        outBuffer.append((quint32)blocks[i].size());           // real size
        outBuffer.append((quint32)compressedBlocks[i].size()); // compressed size
        outBuffer.append((quint16)mode);                       // mode
        // Compressed resource
        outBuffer.append(compressedBlocks[i]);
    }

    int totalSizeRaw = 0;
    int totalSizeCompressed = 0;
    for (int i=0; i<blocks.count(); i++) {
        totalSizeRaw += blocks[i].size();
        totalSizeCompressed += compressedBlocks[i].size();
    }
    if (totalSizeRaw > 0) { // DbZ
        VERBOSE_COUT(blocks.size() << " entries compressed to " << (100*totalSizeCompressed/totalSizeRaw) << "%");
    }

    return outBuffer.buffer();
}

//-------------------------------------------------------------------------------------------
int HqrFile::count() const
{
    return mEntries.count();
}

//-------------------------------------------------------------------------------------------
bool HqrFile::isOpen() const
{
    return mOpen;
}

//-------------------------------------------------------------------------------------------
bool HqrFile::isValid() const
{
    if (!mOpen || !mValidationErrors.isEmpty())
        return false;
    for (int index = 0; index < mEntries.count(); ++index) {
        if (!mEntries[index].runtimeError.isEmpty())
            return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------
QString HqrFile::filename() const
{
    return mFilename;
}

//-------------------------------------------------------------------------------------------
QString HqrFile::errorString() const
{
    return mErrorString;
}

//-------------------------------------------------------------------------------------------
QStringList HqrFile::validationErrors() const
{
    QStringList errors = mValidationErrors;
    for (int i = 0; i < mEntries.count(); ++i) {
        if (!mEntries[i].runtimeError.isEmpty())
            errors << QString("Entry %1: %2").arg(i).arg(mEntries[i].runtimeError);
    }
    return errors;
}

//-------------------------------------------------------------------------------------------
bool HqrFile::validate(QStringList *errors, bool verifyData) const
{
    if (verifyData) {
        for (int index = 0; index < mEntries.count(); ++index) {
            if (mEntries[index].info.available && mEntries[index].info.valid)
                loadBlock(index);
        }
    }

    QStringList result = validationErrors();
    if (!mOpen && !mErrorString.isEmpty())
        result.prepend(mErrorString);
    if (errors)
        *errors = result;
    return mOpen && result.isEmpty();
}

//-------------------------------------------------------------------------------------------
HqrFile::EntryInfo HqrFile::entryInfo(int index) const
{
    if (index < 0 || index >= mEntries.count())
        return EntryInfo();
    EntryInfo info = mEntries[index].info;
    if (!mEntries[index].runtimeError.isEmpty()) {
        info.valid = false;
        info.error = mEntries[index].runtimeError;
    }
    return info;
}

//-------------------------------------------------------------------------------------------
bool HqrFile::hasBlock(int index) const
{
    return index >= 0 && index < mEntries.count()
            && mEntries[index].info.available && mEntries[index].info.valid;
}

//-------------------------------------------------------------------------------------------
QByteArray HqrFile::data(int index, bool *ok) const
{
    const bool loaded = loadBlock(index);
    if (ok)
        *ok = loaded;
    if (!loaded || index < 0 || index >= mEntries.count())
        return QByteArray();
    return mEntries[index].data;
}

//-------------------------------------------------------------------------------------------
int HqrFile::cachedBlockCount() const
{
    int result = 0;
    for (int index = 0; index < mEntries.count(); ++index) {
        if (mEntries[index].loaded)
            ++result;
    }
    return result;
}

//-------------------------------------------------------------------------------------------
void HqrFile::clearCache() const
{
    for (int index = 0; index < mEntries.count(); ++index) {
        const EntryState &state = mEntries[index];
        if (state.sourceBacked) {
            state.data.clear();
            state.runtimeError.clear();
            state.loaded = false;
        }
    }
}

//-------------------------------------------------------------------------------------------
const QByteArray &HqrFile::block(int index) const
{
    static const QByteArray emptyBlock;
    if (!loadBlock(index))
        return emptyBlock;
    return mEntries[index].data;
}

//-------------------------------------------------------------------------------------------
void HqrFile::appendBlock(const QByteArray &inData)
{
    EntryState state;
    state.info.available = true;
    state.info.valid = true;
    state.info.decompressedSize = inData.size();
    state.info.compressedSize = inData.size();
    state.data = inData;
    state.loaded = true;
    mEntries << state;
    mOpen = true;
}

//-------------------------------------------------------------------------------------------
bool HqrFile::loadBlock(int index) const
{
    if (index < 0 || index >= mEntries.count()) {
        mErrorString = QString("HQR entry index %1 is out of range").arg(index);
        return false;
    }

    const EntryState &state = mEntries[index];
    if (state.loaded)
        return state.runtimeError.isEmpty();
    state.loaded = true;

    if (!state.info.valid) {
        state.runtimeError = state.info.error;
        return false;
    }

    if (!state.info.available || state.info.decompressedSize == 0)
        return true;

    QByteArray storedData;
    if (!readSource(qint64(state.info.offset) + 10, state.info.compressedSize, storedData)) {
        state.runtimeError = "cannot read compressed entry data";
        return false;
    }

    if (state.info.compressionMode == 0) {
        state.data = storedData;
        if (state.data.size() != int(state.info.decompressedSize)) {
            state.runtimeError = "decoded size does not match the entry header";
            state.data.clear();
            return false;
        }
        return true;
    }

    bool ok = false;
    state.data = decompressEntry(storedData, state.info.decompressedSize,
                                 state.info.compressionMode, &ok, &state.runtimeError);
    if (!ok)
        state.data.clear();
    return ok;
}

//-------------------------------------------------------------------------------------------
void HqrFile::addValidationError(int index, const QString &error)
{
    mValidationErrors << QString("Entry %1: %2").arg(index).arg(error);
}

//-------------------------------------------------------------------------------------------
QByteArray HqrFile::decompressEntry(const QByteArray &inBuffer, qint32 decompsize, qint32 mode,
                                    bool *ok, QString *error) const
{
    if (ok)
        *ok = false;
    if (error)
        error->clear();
    if (decompsize < 0 || mode < 1 || mode > 5) {
        if (error)
            *error = "invalid decompression parameters";
        return QByteArray();
    }

    QByteArray outBuffer(decompsize, '\0');
    int sourcePos = 0;
    int targetPos = 0;

    while (targetPos < decompsize) {
        if (sourcePos >= inBuffer.size()) {
            if (error)
                *error = "compressed stream ends before its control byte";
            return QByteArray();
        }

        const quint8 control = quint8(inBuffer.at(sourcePos++));
        for (int bit = 0; bit < 8 && targetPos < decompsize; ++bit) {
            if (control & (1 << bit)) {
                if (sourcePos >= inBuffer.size()) {
                    if (error)
                        *error = "compressed stream ends before a literal byte";
                    return QByteArray();
                }
                outBuffer[targetPos++] = inBuffer.at(sourcePos++);
                continue;
            }

            if (sourcePos + 2 > inBuffer.size()) {
                if (error)
                    *error = "compressed stream ends inside a back-reference";
                return QByteArray();
            }
            const quint16 encoded = readLittleEndian16(inBuffer, sourcePos);
            sourcePos += 2;
            const int length = (encoded & 0x0f) + mode + 1;
            const int distance = (encoded >> 4) + 1;
            if (distance > targetPos) {
                if (error)
                    *error = QString("invalid back-reference distance %1 at output offset %2")
                            .arg(distance).arg(targetPos);
                return QByteArray();
            }
            if (length > decompsize - targetPos) {
                if (error)
                    *error = "back-reference exceeds the declared decompressed size";
                return QByteArray();
            }
            for (int copy = 0; copy < length; ++copy) {
                outBuffer[targetPos] = outBuffer.at(targetPos - distance);
                ++targetPos;
            }
        }
    }

    if (ok)
        *ok = true;
    return outBuffer;
}

//-------------------------------------------------------------------------------------------
qint32 HqrFile::findThisBlockNearPosition(const char *src, const char *pos, int len, qint32 mode, quint16 &distance) const
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
    int         minCompressionLength = mode + 1;
    int         maxCompressionLength = 0xF + mode + 1;
    int         cl; // "current length

    if (len < minCompressionLength)
        return 1;

    while (nextPos >= src) {

        maxCompareLength = pos - nextPos;
        if (len < maxCompareLength)
            maxCompareLength = len;
        if (len < maxCompareLength)
            maxCompareLength = len;
        if (maxCompressionLength < maxCompareLength)
            maxCompareLength = maxCompressionLength;

        /*
        for (cl = minCompressionLength; cl <= maxCompareLength; cl++) {
            if (!memcmp(nextPos,pos,cl)) {
                if (cl > bestLength) {
                    bestLength = cl;
                    bestPos    = nextPos;
                }
            }
        }
        */


        cl = memcmp2max(nextPos,pos,maxCompareLength);
        if (cl >= minCompressionLength && cl > bestLength) {
            bestLength = cl;
            bestPos    = nextPos;
        }


        if (bestLength == maxCompressionLength) // found biggest possible block!
            break;

        nextPos -= 1;

        if ((pos - nextPos) > 0xFFF) // reach max distance
            break;
    }

    if (!bestLength)
        return 1;

    distance = pos - bestPos;
    Q_ASSERT(distance <= 0xFFF);

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
        qint32 nextLen = findThisBlockNearPosition(srcStart,src,compressLength,mode, distance);
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

    QByteArray ret = outBuffer.buffer();
    if (ret.size() > 0) {
        int ratio = (100*ret.size()/inBuffer.size());
        VERBOSE_COUT(" ...done. New compressed size: " <<  ratio << "%" << (ratio > 100 ? " (I'm sorry...)":""));
    }
    return outBuffer.buffer();
}
