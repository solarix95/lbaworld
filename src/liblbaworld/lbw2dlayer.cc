#include "lbw2dlayer.h"

//-------------------------------------------------------------------------------------------------
Lbw2dLayer::Lbw2dLayer()
 : mIsVisible(false)
{
}

//-------------------------------------------------------------------------------------------------
bool Lbw2dLayer::keyEvent(QKeyEvent *ke)
{
    return false;
}

//-------------------------------------------------------------------------------------------------
bool Lbw2dLayer::processState(float /*speed*/)
{
    return false;
}

//-------------------------------------------------------------------------------------------------
void Lbw2dLayer::setVisible(bool visible)
{
    mIsVisible = visible;
    emit updateRequest();
}
