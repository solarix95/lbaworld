#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dmodelanimator.h>
#include <QMatrix4x4>
#include "lbwactor.h"
#include "lbwcharacter.h"

//-------------------------------------------------------------------------------------------------
LbwActor::LbwActor(LbwCharacter *character)
    : Qtr3dStandardEntity(nullptr,{0,0,0})
    , mCharacter(character)
    , mCurrentModel(nullptr)
    , mState(nullptr)
    , mCurrentClip(nullptr)
    , mLastAnimationTime(0.0f)
    , mRootMotionEnabled(true)
{
    Q_ASSERT(mCharacter);
    setLookAt({0,0,1});
}

//-------------------------------------------------------------------------------------------------
bool LbwActor::setCostume(const QString &costumeName, const QString &animation)
{
    if (costumeName.isEmpty())
        return false;

    Qtr3dModel *nextModel = mCharacter->model(costumeName);
    if (!nextModel)
        return false;

    QString nextAnimation = animation;
    if (nextAnimation.isEmpty()) {
        if (mCharacter->animation("idle") && nextModel->animationByName("idle"))
            nextAnimation = "idle";
        else if (!nextModel->animations().isEmpty())
            nextAnimation = nextModel->animations().first();
    }
    if (nextAnimation.isEmpty()
            || !mCharacter->animation(nextAnimation)
            || !nextModel->animationByName(nextAnimation))
        return false;

    if (mState) {
        Qtr3dGeometryState *oldState = mState;
        mState = nullptr;
        setState(nullptr);
        oldState->deleteLater();
    }

    mCurrentCostume = costumeName;
    mCurrentModel = nextModel;
    setState(mState = mCurrentModel->context()->createState(mCurrentModel));
    return setAnimation(nextAnimation);
}

//-------------------------------------------------------------------------------------------------
bool LbwActor::setAnimation(const QString &animation)
{
    if (!mState || !mCurrentModel || animation.isEmpty())
        return false;

    Qtr3dModelAnimation *modelAnimation = mCurrentModel->animationByName(animation);
    const LbwCharacter::AnimationClip *clip = mCharacter->animation(animation);
    if (!modelAnimation || !clip)
        return false;

    mCurrentAnimation = animation;
    mCurrentClip = clip;
    mState->setAnimator(new Qtr3dModelAnimator(modelAnimation));
    mState->animator()->setLoop(true);
    mState->animator()->start();
    mState->animator()->setEnabled(true);
    mLastAnimationTime = 0.0f;
    mLastRootMotion = mCurrentClip->animation.rootMotionAt(0.0f);
    return true;
}

//-------------------------------------------------------------------------------------------------
void LbwActor::setRootMotionEnabled(bool enabled)
{
    mRootMotionEnabled = enabled;
    if (mCurrentClip && mState && mState->animator()) {
        mLastAnimationTime = mState->animator()->animationTime();
        mLastRootMotion = mCurrentClip->animation.rootMotionAt(mLastAnimationTime);
    }
}

//-------------------------------------------------------------------------------------------------
bool LbwActor::rootMotionEnabled() const
{
    return mRootMotionEnabled;
}

//-------------------------------------------------------------------------------------------------
QString LbwActor::currentCostume() const
{
    return mCurrentCostume;
}

//-------------------------------------------------------------------------------------------------
QString LbwActor::currentAnimation() const
{
    return mCurrentAnimation;
}

//-------------------------------------------------------------------------------------------------
bool LbwActor::update(float dtSeconds)
{
    bool changed = Qtr3dStandardEntity::update(dtSeconds);
    if (!mRootMotionEnabled || !mCurrentClip || !mState || !mState->animator())
        return changed;

    const LbaAnimation &animation = mCurrentClip->animation;
    const float animationTime = mState->animator()->animationTime();
    const LbaAnimation::RootMotion rootMotion = animation.rootMotionAt(animationTime);

    QVector3D translationDelta;
    float rotationDelta = 0.0f;
    if (animationTime + 0.0001f < mLastAnimationTime) {
        const LbaAnimation::RootMotion endMotion =
                animation.rootMotionAt(animation.playbackDurationTicks());
        const LbaAnimation::RootMotion loopMotion =
                animation.rootMotionAt(animation.playbackLoopStartTicks());
        translationDelta = (endMotion.translation-mLastRootMotion.translation)
                + (rootMotion.translation-loopMotion.translation);
        rotationDelta = (endMotion.rotationY-mLastRootMotion.rotationY)
                + (rootMotion.rotationY-loopMotion.rotationY);
    } else {
        translationDelta = rootMotion.translation-mLastRootMotion.translation;
        rotationDelta = rootMotion.rotationY-mLastRootMotion.rotationY;
    }

    if (rotationDelta != 0.0f) {
        QVector3D actorRotation = rotation();
        actorRotation.setY(actorRotation.y()+rotationDelta);
        changed |= setRotation(actorRotation);
    }
    if (!translationDelta.isNull()) {
        QMatrix4x4 actorRotation;
        actorRotation.rotate(rotation().y(), 0.0f, 1.0f, 0.0f);
        changed |= setPos(pos()+actorRotation.mapVector(translationDelta));
    }

    mLastAnimationTime = animationTime;
    mLastRootMotion = rootMotion;
    return changed;
}
