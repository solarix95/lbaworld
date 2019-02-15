#ifndef LBWPLASMAEFFECT_H
#define LBWPLASMAEFFECT_H

#include <QPainter>
#include <QSize>

class LbwPlasmaEffect
{
public:
    LbwPlasmaEffect();
    ~LbwPlasmaEffect();

    void process(float speed);
    void render(const QRect &rect, QPainter &p);

private:
    float          mTime;
    unsigned char *mSintable;
    int           *mWave;
};

#endif // LBWPLASMAEFFECT_H
