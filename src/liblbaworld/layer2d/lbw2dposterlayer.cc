#include "layer2d/lbw2dposterlayer.h"

//-------------------------------------------------------------------------------------------------
Lbw2dPosterLayer::Lbw2dPosterLayer()
{
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::render(const QRect &window, QPainter &p)
{
    if (mCurrentPoster.isNull())
        return;

    p.fillRect(window,Qt::black);
    p.drawImage(window.width()/2-mCurrentPoster.width()/2,window.height()/2-mCurrentPoster.height()/2, mCurrentPoster);
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::showImage(const QImage &image)
{
    mCurrentPoster = image;
}
