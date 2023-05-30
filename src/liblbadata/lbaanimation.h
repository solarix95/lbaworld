#ifndef LBAANIMATION_H
#define LBAANIMATION_H

#include <QByteArray>
#include <QList>

class LbaAnimation
{
public:
    LbaAnimation();

    struct SimpleBone {
        int boneType;
        float rotateX;
        float rotateY;
        float rotateZ;
        SimpleBone(int type, float x, float y, float z) : boneType(type), rotateX(x), rotateY(y), rotateZ(z) {}
    };
    typedef QList<SimpleBone> SimpleBones;

    struct KeyFrame {
        int               keyFrameLength; // Time? in.. ? [ms] ?
        float             velocityX;
        float             velocityY;
        float             velocityZ;
        SimpleBones       bones;
    };
    typedef QList<KeyFrame> KeyFrames;

    bool fromBuffer(const QByteArray &buffer);

    int  keyFrameCount() const;
    int  loopStartIndex() const;
    const SimpleBones &bones(int keyFrameIndex) const;

private:
    int       mLoopStartIndex;
    KeyFrames mFrames;

};

#endif // LBAANIMATION_H
