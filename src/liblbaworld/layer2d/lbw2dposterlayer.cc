#include "layer2d/lbw2dposterlayer.h"

//-------------------------------------------------------------------------------------------------
Lbw2dPosterLayer::Lbw2dPosterLayer()
{
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::render(const QRect &window, QPainter &p)
{
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::showImage(const QImage &image)
{
    mCurrentPoster = image;
}
