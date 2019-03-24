
#include "flamovie.h"
#include "layer2d/lbw2dposterlayer.h"

//-------------------------------------------------------------------------------------------------
Lbw2dPosterLayer::Lbw2dPosterLayer()
    : mState(Stopped), mOpacity(0)
{
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::render(const QRect &window, QPainter &p)
{
    p.fillRect(window,Qt::black);
    p.setOpacity(mOpacity);

    if (mImageQueue.isEmpty())
        return;

    QImage img = nextFrame();
    while (img.height() < window.height()/2)
        img = img.scaled(img.width()*2, img.height()*2);

    p.drawImage(window.width()/2-img.width()/2,window.height()/2-img.height()/2, img);
}

//-------------------------------------------------------------------------------------------------
bool Lbw2dPosterLayer::processState(float)
{
    bool stateChanged = false;
    switch (mState) {
    case FadeIn:
        mOpacity += mTimeInState.elapsed()/1500.0;
        if (mOpacity >= 1) {
            setState(OnAir);
            mOpacity = 1.0;
        }
        stateChanged = true;
        break;
    case FadeOut:
        mOpacity -= mTimeInState.elapsed()/1500.0;
        if (mOpacity <= 0) {
            // Stop
            mOpacity = 0;
            setState(Stopped);
            deQueue();

            // Start?
            if (!mImageQueue.isEmpty())
                setState(FadeIn);
        }
        stateChanged = true;
        break;
    case OnAir:
        if (mImageQueue.first().movie && !mImageQueue.first().movie->isPlaying())
            setState(FadeOut);
        break;
    }
    return stateChanged;
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::showImage(const QImage &image)
{
    mImageQueue << ImageSource(image);
    onNewSource();
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::showFla(FlaMovie *movie)
{
    Q_ASSERT(movie);
    mImageQueue << ImageSource(movie);
    mImageQueue.last().movie->requestFrame(0);
    connect(movie, SIGNAL(playFrame(int)), this, SIGNAL(updateRequest()));
    onNewSource();
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::setState(Lbw2dPosterLayer::State s)
{
    if (mState == s)
        return;
    mState = s;
    mTimeInState.start();

    switch(mState) {
    case OnAir:
        if (mImageQueue.first().movie)
            mImageQueue.first().movie->resume();
        break;
    }
}

//-------------------------------------------------------------------------------------------------
QImage Lbw2dPosterLayer::nextFrame()
{
    if (mImageQueue.isEmpty())
        return QImage();
    if (mImageQueue.first().movie)
        return mImageQueue.first().movie->currentFrame();
    return mImageQueue.first().img;
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::deQueue()
{
    Q_ASSERT(!mImageQueue.isEmpty());
    ImageSource old = mImageQueue.takeFirst();
    if (old.movie)
        old.movie->deleteLater();
}

//-------------------------------------------------------------------------------------------------
void Lbw2dPosterLayer::onNewSource()
{
    switch (mState) {
    case Stopped:
        setState(FadeIn);
        break;
    case OnAir:
    case FadeIn:
        setState(FadeOut);
        break;
    case FadeOut:
        break;
    }

    setVisible(true);
}
