#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dmodelanimator.h>
#include "lbwactor.h"
#include "lbwcharacter.h"

//-------------------------------------------------------------------------------------------------
LbwActor::LbwActor(LbwCharacter *character)
    : Qtr3dStandardEntity(nullptr,{0,0,0})
    , mCharacter(character)
    , mCurrentModel(nullptr)
    , mState(nullptr)
{
    Q_ASSERT(mCharacter);
    setLookAt({0,0,1});
}

//-------------------------------------------------------------------------------------------------
void LbwActor::setCostume(const QString &costumeName, const QString &animation)
{
    if (mState) {
        mState->deleteLater();
        mState = nullptr;
    }

    if (costumeName.isEmpty())
        return;

    mCurrentCostume = costumeName;

    mCurrentModel = mCharacter->model(mCurrentCostume);
    if (!mCurrentModel)
        return;

    setState(mState = mCurrentModel->context()->createState(mCurrentModel));

    setAnimation(animation);
}

//-------------------------------------------------------------------------------------------------
void LbwActor::setAnimation(const QString &animation)
{
    mCurrentAnimation = animation;
    if (mState && !mCurrentModel->animations().isEmpty()) {
        mState->setAnimator(new Qtr3dModelAnimator(mCurrentModel->animationByName(mCurrentModel->animations().first())));
        mState->animator()->setLoop(true);
        mState->animator()->setState(true);
        mState->animator()->setEnabled(true);
    }
}
