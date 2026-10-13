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
