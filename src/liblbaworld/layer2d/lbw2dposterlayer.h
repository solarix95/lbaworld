#ifndef LBW2DPOSTERLAYER_H
#define LBW2DPOSTERLAYER_H

#include <QImage>
#include "lbw2dlayer.h"

class Lbw2dPosterLayer : public Lbw2dLayer
{
    Q_OBJECT
public:
    Lbw2dPosterLayer();

    virtual void render(const QRect &window, QPainter &p) override;
public slots:
    void showImage(const QImage &image);

private:
    QImage mCurrentPoster;
};

#endif // LBW2DPOSTERLAYER_H
