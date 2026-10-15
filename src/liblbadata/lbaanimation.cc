#include "lbaanimation.h"
#include "binaryreader.h"

//-------------------------------------------------------------------------------------------
LbaAnimation::LbaAnimation()
    : mBoneCount(0)
    , mLoopStartIndex(0)
    , mDurationTicks(0)
{
}

//-------------------------------------------------------------------------------------------
bool LbaAnimation::fromBuffer(const QByteArray &buffer)
{
    mFrames.clear();
    mBoneCount = 0;
    mLoopStartIndex = 0;
    mDurationTicks = 0;
    mErrorString.clear();

    if (buffer.size() < 8) {
        mErrorString = "Animation header is truncated";
        return false;
    }

    BinaryReader reader(buffer);

    const quint16 numOfKeyframes = reader.readUint16();
    const quint16 numOfBones = reader.readUint16();
    const quint16 loopStart = reader.readUint16();
    reader.readUint16(); // reserved

    if (reader.error() || numOfKeyframes == 0 || numOfBones == 0) {
        mErrorString = "Animation has an invalid header";
        return false;
    }

    const qint64 frameSize = 8 + qint64(numOfBones) * 8;
    const qint64 requiredSize = 8 + qint64(numOfKeyframes) * frameSize;
    if (requiredSize > buffer.size()) {
        mErrorString = QString("Animation is truncated: expected at least %1 bytes, got %2")
                .arg(requiredSize).arg(buffer.size());
        return false;
    }
    KeyFrames frames;
    frames.reserve(numOfKeyframes);
    int durationTicks = 0;
    for (int i=0; i<numOfKeyframes; i++) {
        KeyFrame f;
        f.keyFrameLength = reader.readUint16();
        durationTicks += f.keyFrameLength;

        // TODO: Support velocity http://lbafileinfo.kazekr.net/index.php?title=LBA1:Animation
        const qint16 velocityX = reader.readInt16();
        const qint16 velocityY = reader.readInt16();
        const qint16 velocityZ = reader.readInt16();
        f.stepX = velocityX;
        f.stepY = velocityY;
        f.stepZ = velocityZ;
        f.velocityX = velocityX/(float)0x400;
        f.velocityY = velocityY/(float)0x400;
        f.velocityZ = velocityZ/(float)0x400;

        for (int b=0; b<numOfBones; b++) {
            const quint16 flags = reader.readUint16();
            const qint16 valX = reader.readInt16();
            const qint16 valY = reader.readInt16();
            const qint16 valZ = reader.readInt16();

            if (flags == 0) {
                float x,y,z;
                x = 360.0 * valX / 0x400;
                y = 360.0 * valY / 0x400;
                z = 360.0 * valZ / 0x400;

                f.bones << SimpleBone(flags, valX, valY, valZ, x, y, z);
            } else {
                f.bones << SimpleBone(flags, valX, valY, valZ, valX, valY, valZ);
            }
        }

        if (reader.error()) {
            mErrorString = QString("Animation data is truncated in keyframe %1").arg(i);
            return false;
        }
        frames << f;
    }

    mFrames = frames;
    mBoneCount = numOfBones;
    mLoopStartIndex = loopStart;
    mDurationTicks = durationTicks;
    return true;
}

//-------------------------------------------------------------------------------------------
bool LbaAnimation::isValid() const
{
    return !mFrames.isEmpty() && mBoneCount > 0 && mErrorString.isEmpty();
}

//-------------------------------------------------------------------------------------------
QString LbaAnimation::errorString() const
{
    return mErrorString;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::keyFrameCount() const
{
    return mFrames.count();
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::boneCount() const
{
    return mBoneCount;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::loopStartIndex() const
{
    return mLoopStartIndex;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::durationTicks() const
{
    return mDurationTicks;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::introDurationTicks() const
{
    if (!isValid())
        return 0;

    const int loopIndex = qBound(0, mLoopStartIndex, mFrames.count()-1);
    int duration = 0;
    for (int frameIndex = 0; frameIndex < loopIndex; ++frameIndex)
        duration += mFrames[frameIndex].keyFrameLength;
    return duration;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::transitionDurationTicks() const
{
    return isValid() ? mFrames.first().keyFrameLength : 0;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::playbackDurationTicks() const
{
    if (!isValid())
        return 0;

    int duration = mDurationTicks;
    const int loopIndex = qBound(0, mLoopStartIndex, mFrames.count()-1);
    for (int frameIndex = loopIndex; frameIndex < mFrames.count(); ++frameIndex)
        duration += mFrames[frameIndex].keyFrameLength;
    return duration;
}

//-------------------------------------------------------------------------------------------
int LbaAnimation::playbackLoopStartTicks() const
{
    return isValid() ? mDurationTicks : 0;
}

//-------------------------------------------------------------------------------------------
const LbaAnimation::KeyFrame *LbaAnimation::keyFrame(int keyFrameIndex) const
{
    if (keyFrameIndex < 0 || keyFrameIndex >= mFrames.count())
        return NULL;
    return &mFrames[keyFrameIndex];
}

//-------------------------------------------------------------------------------------------
const LbaAnimation::KeyFrames &LbaAnimation::keyFrames() const
{
    return mFrames;
}

//-------------------------------------------------------------------------------------------
const LbaAnimation::SimpleBones &LbaAnimation::bones(int keyFrameIndex) const
{
    static const SimpleBones emptyBones;
    const KeyFrame *frame = keyFrame(keyFrameIndex);
    if (!frame)
        return emptyBones;

    return frame->bones;
}

//-------------------------------------------------------------------------------------------
LbaAnimation::Timeline LbaAnimation::playbackTimeline() const
{
    Timeline timeline;
    if (!isValid())
        return timeline;

    float time = 0.0f;
    timeline << TimedFrame(0, time);
    for (int frameIndex = 0; frameIndex < mFrames.count(); ++frameIndex) {
        time += mFrames[frameIndex].keyFrameLength;
        timeline << TimedFrame(frameIndex, time);
    }

    const int loopIndex = qBound(0, mLoopStartIndex, mFrames.count()-1);
    for (int frameIndex = loopIndex; frameIndex < mFrames.count(); ++frameIndex) {
        time += mFrames[frameIndex].keyFrameLength;
        timeline << TimedFrame(frameIndex, time);
    }
    return timeline;
}

//-------------------------------------------------------------------------------------------
LbaAnimation::RootMotion LbaAnimation::rootMotionAt(float playbackTime) const
{
    RootMotion result;
    if (!isValid())
        return result;

    playbackTime = qBound(0.0f, playbackTime, float(playbackDurationTicks()));

    QList<int> sequence;
    for (int frameIndex = 0; frameIndex < mFrames.count(); ++frameIndex)
        sequence << frameIndex;
    const int loopIndex = qBound(0, mLoopStartIndex, mFrames.count()-1);
    for (int frameIndex = loopIndex; frameIndex < mFrames.count(); ++frameIndex)
        sequence << frameIndex;

    float segmentStart = 0.0f;
    for (int frameIndex: sequence) {
        const KeyFrame &frame = mFrames[frameIndex];
        const float segmentDuration = frame.keyFrameLength;
        const QVector3D translation(frame.stepX/800.0f,
                                    frame.stepY/800.0f,
                                    frame.stepZ/800.0f);
        const float rotationY = frame.bones.isEmpty()
                ? 0.0f : 360.0f*frame.bones.first().rawY/1024.0f;
        const float segmentEnd = segmentStart + segmentDuration;

        if (segmentDuration > 0.0f && playbackTime < segmentEnd) {
            const float factor = qBound(0.0f,
                                        (playbackTime-segmentStart)/segmentDuration,
                                        1.0f);
            result.translation += factor*translation;
            result.rotationY += factor*rotationY;
            return result;
        }

        result.translation += translation;
        result.rotationY += rotationY;
        segmentStart = segmentEnd;
        if (playbackTime <= segmentStart)
            return result;
    }
    return result;
}
