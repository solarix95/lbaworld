#ifndef LBW2DCONSOLELAYER_H
#define LBW2DCONSOLELAYER_H

#include "lbw2dlayer.h"
#include "rendering/lbwplasmaeffect.h"

class Lbw2dConsoleLayer : public Lbw2dLayer
{
    Q_OBJECT
public:
    Lbw2dConsoleLayer();

    virtual bool processState(float speed) override;
    virtual void render(const QRect &window, QPainter &p) override;
    virtual bool keyEvent(QKeyEvent *ke) override;

private:
    void handleKeyEvent(QKeyEvent *ke);
    bool inPosition() const;

    bool     mIsActive;
    QString  mCurrentInput;
    float    mPosTarget;
    float    mPosCurrent;
    LbwPlasmaEffect mPlasma;
};

#endif // LBW2DCONSOLELAYER_H
