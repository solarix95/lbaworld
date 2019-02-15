#include <math.h>
#include <QDebug>
#include "rendering/lbwplasmaeffect.h"

#include "lbwplasmaeffect.h"

//-------------------------------------------------------------------------------------------------
LbwPlasmaEffect::LbwPlasmaEffect()
{
    mTime = 0;
    mSintable = (unsigned char *) malloc (1024);
    mWave     = (int *) malloc (4* sizeof (int));

    for (int i=0; i<1024; i++)
        mSintable[i] = 128+127*sin((float)i*3.14/512.0);

    for (int i=0; i<4; i++)
        mWave[i]=0;
}

//-------------------------------------------------------------------------------------------------
LbwPlasmaEffect::~LbwPlasmaEffect()
{
    free(mSintable);
}

//-------------------------------------------------------------------------------------------------
void LbwPlasmaEffect::process(float speed)
{
    mTime += speed*0.03;
}

//-------------------------------------------------------------------------------------------------
void LbwPlasmaEffect::render (const QRect &rect, QPainter &p)
{
    int speed_x[4];
    int speed_y[4];
    int workwave_x[4];
    int workwave_y[4];

    // Speed-Werte der Wellen vorberechnen
    speed_x[0] = 3000 * sin (mTime*1.0+1);
    speed_y[0] = 3000 * cos (mTime*1.1+2);
    speed_x[1] =  700 * sin (-mTime*1.2+3);
    speed_y[1] =  700 * cos (-mTime*1.3+4);
    speed_x[2] = 2300 * sin (mTime*1.4+5);
    speed_y[2] = 2300 * cos (mTime*1.5+6);
    speed_x[3] = 1700 * sin (mTime*1.6+7);
    speed_y[3] = 1700 * cos (mTime*1.7+8);

    for (int i=0; i<4; i++) {
        mWave[i]      += speed_x[i];
        workwave_y[i] = mWave[i];
    }

    QColor c(0,0,0);

    int fieldSize = 16;
    p.setPen(QPen(Qt::black,1));
    p.scale(rect.width()/(float)fieldSize,rect.height()/(float)fieldSize);
    int gbOffset;
    for (int y=0; y<fieldSize+1; y++) {
        for (int i=0; i<4; i++) workwave_x[i] = workwave_y[i];

        // Scanline zeichnen
        for (int x=0; x<fieldSize+1; x++) {
           gbOffset = ((mSintable[(workwave_x[0]>>8)&1023] +
                                  mSintable[(workwave_x[1]>>8)&1023] +
                                  mSintable[(workwave_x[2]>>8)&1023] +
                                  mSintable[(workwave_x[3]>>8)&1023])>>2)/2;

           c.setBlue(gbOffset);
           c.setGreen(gbOffset);
           // p.fillRect(x,y,1,1,c);
           p.setPen(c);
           p.drawPoint(x,y);
          for (int i=0; i<4; i++) workwave_x[i] += speed_x[i];
        }
        for (int i=0; i<4; i++) workwave_y[i] += speed_y[i];
    }
}
