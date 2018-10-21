#include <QDebug>
#include "lbaanimation.h"
#include "binaryreader.h"

//-------------------------------------------------------------------------------------------
LbaAnimation::LbaAnimation()
{    
}

//-------------------------------------------------------------------------------------------
bool LbaAnimation::fromBuffer(const QByteArray &buffer)
{
    mFrames.clear();

    BinaryReader reader(buffer);

    quint16 numOfKeyframes;
    reader.read(&numOfKeyframes,2);

    quint16 numOfBones;
    reader.read(&numOfBones,2);

    quint16 loopStart;
    reader.read(&loopStart,2);
    mLoopStartIndex= loopStart;

    reader.skip(2);

    qDebug() << "LbaAnimation::fromBuffer" << numOfKeyframes << numOfBones << loopStart;
    for (int i=0; i<numOfKeyframes; i++) {
        KeyFrame f;
        quint16  keyFrameLength;
        reader.read(&keyFrameLength,2);
        f.keyFrameLength = keyFrameLength;

        // TODO: Support velocity http://lbafileinfo.kazekr.net/index.php?title=LBA1:Animation
        qint16 velocityX, velocityY, velocityZ;
        reader.read(&velocityX,2);
        reader.read(&velocityY,2);
        reader.read(&velocityZ,2);

        for (int b=0; b<numOfBones; b++) {
            quint16 flags;
            reader.read(&flags,2);

            qint16 valX, valY, valZ;
            reader.read(&valX,2);
            reader.read(&valY,2);
            reader.read(&valZ,2);

            if (flags == 0) {
                float x,y,z;
                x = 360.0 * valX / 0x400;
                y = 360.0 * valY / 0x400;
                z = 360.0 * valZ / 0x400;

                qDebug() << i << b << flags << x << y << z;
                f.bones <<  SimpleBone(flags, x,y,z);
            } else {
                f.bones <<  SimpleBone(flags, valX,valY,valZ);
            }
        }

        mFrames << f;
    }

    return true;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::keyFrameCount() const
{
    return mFrames.count();
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::loopStartIndex() const
{
    return mLoopStartIndex;
}

//-------------------------------------------------------------------------------------------
const LbaAnimation::SimpleBones &LbaAnimation::bones(int keyFrameIndex) const
{
    Q_ASSERT(keyFrameIndex >= 0);
    Q_ASSERT(keyFrameIndex < mFrames.count());

    return mFrames[keyFrameIndex].bones;
}

