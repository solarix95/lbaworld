#include "lbwcharacter.h"

//-------------------------------------------------------------------------------------------------
LbwCharacter::LbwCharacter(const QString &name)
 : mName(name)
{
}

//-------------------------------------------------------------------------------------------------
void LbwCharacter::addCostume(const QString &costumeName, Qtr3dModel *model, const Qtr3dAaBoundingBox &aabb)
{
    mCostumes[costumeName] = Costume(costumeName,model,aabb);
}

//-------------------------------------------------------------------------------------------------
void LbwCharacter::addAnimation(const QString &animationName, int logicalIndex,
                                int resourceIndex, const LbaAnimation &animation)
{
    if (animationName.isEmpty() || !animation.isValid())
        return;
    mAnimations[animationName] = AnimationClip(animationName, logicalIndex,
                                                resourceIndex, animation);
}

//-------------------------------------------------------------------------------------------------
QString LbwCharacter::name() const
{
    return mName;
}

//-------------------------------------------------------------------------------------------------
QStringList LbwCharacter::costumes() const
{
    return mCostumes.keys();
}

//-------------------------------------------------------------------------------------------------
Qtr3dModel *LbwCharacter::model(const QString &costumeName)
{
    return mCostumes.value(costumeName,Costume()).model;
}

//-------------------------------------------------------------------------------------------------
const Qtr3dAaBoundingBox *LbwCharacter::aabb(const QString &costumeName)
{
    if (!mCostumes.contains(costumeName))
        return nullptr;
    return &mCostumes[costumeName].aabb;
}

//-------------------------------------------------------------------------------------------------
QStringList LbwCharacter::animations() const
{
    return mAnimations.keys();
}

//-------------------------------------------------------------------------------------------------
const LbwCharacter::AnimationClip *LbwCharacter::animation(const QString &animationName) const
{
    QMap<QString,AnimationClip>::const_iterator it = mAnimations.constFind(animationName);
    return it == mAnimations.constEnd() ? nullptr : &it.value();
}

//-------------------------------------------------------------------------------------------------
const LbwCharacter::AnimationClip *LbwCharacter::animation(int logicalIndex) const
{
    for (QMap<QString,AnimationClip>::const_iterator it = mAnimations.constBegin();
         it != mAnimations.constEnd(); ++it) {
        if (it.value().logicalIndex == logicalIndex)
            return &it.value();
    }
    return nullptr;
}
