#include <QColor>
#include <QFile>
#include <flamovie.h>
#include <QDebug>

/** Original FLA screen width */
#define FLASCREEN_WIDTH			320
/** Original FLA screen height */
#define FLASCREEN_HEIGHT		200

enum FlaFrameOpcode {
    kLoadPalette	= 0,
    kFade			= 1,
    kPlaySample		= 2,
    kStopSample		= 4,
    kDeltaFrame		= 5,
    kKeyFrame		= 7
};

/** FLA movie sample structure */
typedef struct FLASampleStruct {
    /** Number os samples */
    qint16 sampleNum;
    /** Sample frequency */
    qint16 freq;
    /** Numbers of time to repeat */
    qint16 repeat;
    /** Dummy variable */
    qint8 dummy;
    /** Unknown x */
    quint8 x;
    /** Unknown y */
    quint8 y;
} FLASampleStruct;

//-------------------------------------------------------------------------------------------
FlaMovie::FlaMovie()
 : mCurrentFrame(-1)
{
    connect(&mAnimationTimer, SIGNAL(timeout()), this, SLOT(nextFrame()));
}

//-------------------------------------------------------------------------------------------
bool FlaMovie::fromFile(const QString &filename)
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
bool FlaMovie::fromBuffer(const QByteArray &buffer)
{
    mFrames.clear();
    mColorTable.clear();
    mFade = 0;
    mBinReader.setBuffer(buffer);

    char version[6];
    mBinReader.read(version,6);

    qint32 numOfFrames;
    qint8  speed;
    qint16 width;
    qint16 height;
    qint16 numOfSamples;

    mBinReader.read(&numOfFrames,4);
    mBinReader.read(&speed,1);
    mBinReader.skip(1);
    mBinReader.read(&width,2);
    mBinReader.read(&height,2);
    numOfSamples = mBinReader.readInt16();
    mBinReader.skip(2);

    if (mBinReader.error())
        return false;

    qDebug() << version << numOfFrames << speed << width << height << numOfSamples;

    for (int i = 0; i < numOfSamples; i++) {
        qint16 sampleId = mBinReader.readInt16();
        mBinReader.skip(2);
        qDebug() << "S" << i << sampleId;
    }

    for (int i=0; i<numOfFrames; i++)
        processFrame();

    Q_ASSERT(mFrames.count() > 0);

    mBinReader.clear();
    mColorTable.clear();
    mSpeed = speed;

    emit loaded();
    return true;
}

//-------------------------------------------------------------------------------------------
int FlaMovie::frameCount() const
{
    return mFrames.count();
}

//-------------------------------------------------------------------------------------------
int FlaMovie::fps() const
{
    return mSpeed;
}

//-------------------------------------------------------------------------------------------
QSize FlaMovie::size() const
{
    if (mFrames.isEmpty())
        return QSize(0,0);

    return mFrames.first().frame.size();
}

//-------------------------------------------------------------------------------------------
bool FlaMovie::isPlaying() const
{
    return mAnimationTimer.isActive();
}

//-------------------------------------------------------------------------------------------
bool FlaMovie::atEnd() const
{
    return mCurrentFrame == (mFrames.count()-1);
}

//-------------------------------------------------------------------------------------------
void FlaMovie::start()
{
    mCurrentFrame = -1;

    if (mFrames.isEmpty())
        return;

    mAnimationTimer.start(1000.0/mSpeed);
    setCurrentFrame(0);
}

//-------------------------------------------------------------------------------------------
void FlaMovie::stop()
{
    mAnimationTimer.stop();
}

//-------------------------------------------------------------------------------------------
void FlaMovie::resume()
{
    if (mFrames.isEmpty() || atEnd() || mAnimationTimer.isActive())
        return;
    mAnimationTimer.start(1000.0/mSpeed);
}

//-------------------------------------------------------------------------------------------
void FlaMovie::requestFrame(int i)
{
    if (i >= 0 && i<mFrames.count())
        setCurrentFrame(i);
}

//-------------------------------------------------------------------------------------------
void FlaMovie::requestFps(int f)
{
    if (f <= 0)
        return;

    mSpeed = f;
    if (mAnimationTimer.isActive()) {
        mAnimationTimer.start(1000.0/mSpeed);
    }
}

//-------------------------------------------------------------------------------------------
int FlaMovie::currentFrameIndex() const
{
    return mCurrentFrame;
}

//-------------------------------------------------------------------------------------------
const QImage &FlaMovie::currentFrame() const
{
    Q_ASSERT(mCurrentFrame >= 0);
    Q_ASSERT(mCurrentFrame < mFrames.count());

    return mFrames.at(mCurrentFrame).frame;
}

//-------------------------------------------------------------------------------------------
void FlaMovie::nextFrame()
{
    if (mCurrentFrame < (mFrames.count()-1))
        setCurrentFrame(mCurrentFrame+1);
    else {
        mAnimationTimer.stop();
        emit finished();
    }
}


//-------------------------------------------------------------------------------------------
bool FlaMovie::processFrame()
{
    qint8  videoSize;
    qint32 windowSize;

    if (!mBinReader.read(&videoSize,1))
        return false;

    if (!mBinReader.skip(1))
        return false;

    if (!mBinReader.read(&windowSize,4))
        return false;

   if (((qint32)videoSize) <= 0)
        return false;


    Q_ASSERT(windowSize > 0);
    Q_ASSERT(windowSize < mBinReader.size());


    QByteArray window = mBinReader.readBlock(windowSize);

    char *ptr = window.data();
    quint8  opcode;
    quint32 opcodeBlockSize;
    qint32  aux = 0;

    QList<QPair<int,int> > samples;
    do {
        opcode = *((quint8*)ptr);
        ptr += 2;
        opcodeBlockSize = *((quint16*)ptr);
        ptr += 2;

        switch (opcode - 1) {
        case kLoadPalette: {
            qint16 numOfColor = *((qint16*)ptr);
            qint16 startColor = *((qint16*)(ptr + 2));
            Q_ASSERT(numOfColor >  0);
            Q_ASSERT(startColor >= 0);

            mColorTable.resize(startColor + numOfColor);
            for (int i=0; i<numOfColor; i++) {
                int r = *(((quint8*)ptr) + 4 + i*3 + 0);
                int g = *(((quint8*)ptr) + 4 + i*3 + 1);
                int b = *(((quint8*)ptr) + 4 + i*3 + 2);

                mColorTable[i+startColor] = QColor(r,g,b).rgb();
            }
            break;
        }
        case kFade: {
            mFade++;
            break;
        }
        case kPlaySample: {
            FLASampleStruct sample;
            memcpy(&sample, ptr, sizeof(FLASampleStruct));
            samples << qMakePair<int,int>(sample.sampleNum, sample.repeat-1);
            break;
        }
        case kStopSample: {
            qDebug() << "Stop SAMPLE";
            // stopSample(sample.sampleNum);
            break;
        }
        case kDeltaFrame: {
            appendDeltaFrame((quint8*)ptr);
            break;
        }
        case kKeyFrame: {
            appendKeyFrame((quint8*)ptr);
            break;
        }
        default: {
            qDebug() << "Unknown OP Code" << ((int)(opcode - 1)) << "of size" << opcodeBlockSize;
            break;
        }
        }

        aux++;
        ptr += opcodeBlockSize;

    } while (aux < (qint32)videoSize);

    mFrames.last().samples = samples;

    /*
    if (mFade == 1) {
        fadeToBlack();
    }

    if (mFade == 2) {
        fadeFromBlack();
        mFade = 0;
    }
    */

    return true;
}

//-------------------------------------------------------------------------------------------
void FlaMovie::appendKeyFrame(quint8 *inBuffer)
{
    QImage img(FLASCREEN_WIDTH,FLASCREEN_HEIGHT,QImage::Format_Indexed8);
    img.setColorTable(mColorTable);

    qint8 flag1;
    qint8 flag2;

    for (int y=0; y<FLASCREEN_HEIGHT; y++) {
        quint8 *destPtr = (quint8*)img.scanLine(y);
        flag1 = *(inBuffer++);

        for (int a = 0; a < flag1; a++) {
            flag2 = *(inBuffer++);

            if (flag2 < 0) {
                flag2 = - flag2;
                for (int b = 0; b < flag2; b++) {
                    quint8 colorIndex = *(inBuffer++);
                    *(destPtr++) = colorIndex;
                }
            } else {
                quint8 colorIndex = *(inBuffer++);

                for (int b = 0; b < flag2; b++) {
                    *(destPtr++) = colorIndex;
                }
            }
        }
    }
    Frame f;
    f.frame         = img;
    mFrames << f;
}

//-------------------------------------------------------------------------------------------
void FlaMovie::appendDeltaFrame(quint8 *inBuffer) {
    QImage img(FLASCREEN_WIDTH,FLASCREEN_HEIGHT,QImage::Format_Indexed8);

    img = mFrames.last().frame;
    img.setColorTable(mColorTable);

    quint16 skip;
    qint32  height;

    qint8 flag1;
    qint8 flag2;

    skip = *((quint16*)inBuffer);    // number of lines to skip
    inBuffer += 2;
    height = *((qint16*)inBuffer);
    inBuffer += 2;

    for (int y=0; y<height; y++) {
        quint8 *destPtr = (quint8*)img.scanLine(y+skip);
        flag1 = *(inBuffer++);

        for (int a = 0; a < flag1; a++) {
            destPtr += (unsigned char) * (inBuffer++);
            flag2 = *(inBuffer++);

            if (flag2 > 0) {
                for (int b = 0; b < flag2; b++) {
                    *(destPtr++) = *(inBuffer++);
                }
            } else {
                char colorFill;
                flag2 = - flag2;

                colorFill = *(inBuffer++);

                for (int b = 0; b < flag2; b++) {
                    *(destPtr++) = colorFill;
                }
            }
        }
    }
    Frame f;
    f.frame       = img;
    mFrames << f;
}

//-------------------------------------------------------------------------------------------
void FlaMovie::fadeToBlack()
{
    if (mFrames.isEmpty())
        return;
    QImage fromImage = mFrames.last().frame;
    QVector<QRgb> fromTable = fromImage.colorTable();
    QVector<QRgb> toTable;
    toTable.resize(fromTable.count());

    const int frames = 10;

    for (int f=0; f<frames; f++) {
        for (int i=0; i<fromTable.count(); i++) {
            QColor next(fromTable[i]);
            next.setRed(next.red()     - f*(next.red()/(double)frames));
            next.setGreen(next.green() - f*(next.green()/(double)frames));
            next.setBlue(next.blue()   - f*(next.blue()/(double)frames));
            toTable[i] = next.rgb();
        }
        QImage nextImage = fromImage;
        nextImage.setColorTable(toTable);
        mFrames << Frame(nextImage);
    }
}

//-------------------------------------------------------------------------------------------
void FlaMovie::fadeFromBlack()
{
    if (mFrames.isEmpty())
        return;

    Frame toFrame = mFrames.takeLast();

    QImage fromImage = mFrames.last().frame;
    QVector<QRgb> toTable = toFrame.frame.colorTable();
    QVector<QRgb> fromTable;
    fromTable.resize(toTable.count());

    const int frames = 10;

    for (int f=0; f<frames; f++) {
        for (int i=0; i<fromTable.count(); i++) {
            QColor next(toTable[i]);
            qDebug() << next.red() << next.green() << next.blue();
            next.setRed(f*(next.red()/(double)frames));
            next.setGreen(f*(next.green()/(double)frames));
            next.setBlue(f*(next.blue()/(double)frames));
            fromTable[i] = next.rgb();
            qDebug() << next.red() << next.green() << next.blue();
        }
        QImage nextImage = fromImage;
        nextImage.setColorTable(fromTable);
        mFrames << Frame(nextImage);
    }
    mFrames << toFrame;
}

//-------------------------------------------------------------------------------------------
void FlaMovie::setCurrentFrame(int i)
{
    if (mCurrentFrame == i)
        return;
    mCurrentFrame = i;
    emit playFrame(mCurrentFrame);
    for (int i=0; i<mFrames[mCurrentFrame].samples.count(); i++)
        emit playFlaSample(mFrames[mCurrentFrame].samples[i].first, mFrames[mCurrentFrame].samples[i].second);
}
