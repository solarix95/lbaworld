#ifndef LBAANIMATION_H
#define LBAANIMATION_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QVector3D>

class LbaAnimation
{
public:
    LbaAnimation();

    struct SimpleBone {
        int boneType;
        qint16 rawX;
        qint16 rawY;
        qint16 rawZ;
        float rotateX;
        float rotateY;
        float rotateZ;
        SimpleBone(int type, qint16 sourceX, qint16 sourceY, qint16 sourceZ,
                   float x, float y, float z)
            : boneType(type)
            , rawX(sourceX)
            , rawY(sourceY)
            , rawZ(sourceZ)
            , rotateX(x)
            , rotateY(y)
            , rotateZ(z)
        {}
    };
    typedef QList<SimpleBone> SimpleBones;

    struct KeyFrame {
        int               keyFrameLength; // Duration in LBA ticks (50 Hz).
        qint16            stepX;          // Root displacement in LBA world units.
        qint16            stepY;
        qint16            stepZ;
        float             velocityX;
        float             velocityY;
        float             velocityZ;
        SimpleBones       bones;

        KeyFrame()
            : keyFrameLength(0)
            , stepX(0)
            , stepY(0)
            , stepZ(0)
            , velocityX(0)
            , velocityY(0)
            , velocityZ(0)
        {}
    };
    typedef QList<KeyFrame> KeyFrames;

    struct TimedFrame {
        int frameIndex;
        float time;
        TimedFrame(int index = -1, float frameTime = 0.0f)
            : frameIndex(index), time(frameTime) {}
    };
    typedef QList<TimedFrame> Timeline;

    struct RootMotion {
        QVector3D translation;
        float rotationY;
        RootMotion(const QVector3D &position = QVector3D(), float angle = 0.0f)
            : translation(position), rotationY(angle) {}
    };

    bool fromBuffer(const QByteArray &buffer);

    bool isValid() const;
    QString errorString() const;

    int keyFrameCount() const;
    int boneCount() const;
    int loopStartIndex() const;
    int durationTicks() const;
    int playbackDurationTicks() const;
    int playbackLoopStartTicks() const;

    const KeyFrame *keyFrame(int keyFrameIndex) const;
    const KeyFrames &keyFrames() const;
    const SimpleBones &bones(int keyFrameIndex) const;
    Timeline playbackTimeline() const;
    RootMotion rootMotionAt(float playbackTime) const;

private:
    int       mBoneCount;
    int       mLoopStartIndex;
    int       mDurationTicks;
    KeyFrames mFrames;
    QString   mErrorString;

};

#endif // LBAANIMATION_H
