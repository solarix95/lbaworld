#ifndef LBWACTOR_H
#define LBWACTOR_H

#include <QObject>
#include <QList>
#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dgeometrystate.h>
#include <libqtr3d/physics/qtr3dstandardentity.h>
#include "lbwcharacter.h"

class LbwActor : public Qtr3dStandardEntity
{
public:
    LbwActor(LbwCharacter *character);

    bool setCostume(const QString &costumeName, const QString &animation = "");
    bool setAnimation(const QString &animation);

    void setRootMotionEnabled(bool enabled);
    bool rootMotionEnabled() const;

    QString currentCostume() const;
    QString currentAnimation() const;
    float animationTime() const;
    int animationIntroDurationTicks() const;
    bool animationIntroActive() const;

    bool update(float dtSeconds) override;

    inline Qtr3dGeometryState *currentState() const { return mState; }

private:

    LbwCharacter       *mCharacter;
    Qtr3dModel         *mCurrentModel;

    Qtr3dGeometryState *mState;

    QString             mCurrentCostume;
    QString             mCurrentAnimation;
    const LbwCharacter::AnimationClip *mCurrentClip;
    float               mLastAnimationTime;
    LbaAnimation::RootMotion mLastRootMotion;
    bool                mRootMotionEnabled;
};

using LbwActors = QList<LbwActor*>;

#endif // LBWACTOR_H
