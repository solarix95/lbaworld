#ifndef LBW2DPOSTERLAYER_H
#define LBW2DPOSTERLAYER_H

#include <QImage>
#include <QTime>
#include "lbw2dlayer.h"

class FlaMovie;
class Lbw2dPosterLayer : public Lbw2dLayer
{
    Q_OBJECT
public:
    Lbw2dPosterLayer();

    virtual void render(const QRect &window, QPainter &p) override;
    virtual bool processState(float speed) override;

public slots:
    void showImage(const QImage &image);
    void showFla(FlaMovie *movie);

private:

    struct ImageSource {
        QImage   img;
        FlaMovie *movie;
        ImageSource(QImage i)   : img(i), movie(NULL) {}
        ImageSource(FlaMovie *m): movie(m) {}
    };

    QList<ImageSource> mImageQueue;

    enum State {
        FadeIn,
        OnAir,
        FadeOut,
        Stopped
    };

    void   setState(State s);
    QImage nextFrame();
    void   deQueue();
    void   onNewSource();

    double mOpacity;
    QTime mTimeInState;
    State mState;
};

#endif // LBW2DPOSTERLAYER_H
