
#include <math.h>

#include <QDebug>
#include <QWheelEvent>
#include <QTimer>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMatrix4x4>

#include "lbw3d.h"
#include "lbw3dsphere.h"
#include "lbw3dwidget.h"

//-------------------------------------------------------------------------------------------------
Lbw3dWidget::Lbw3dWidget(QWidget *parent)
 : QOpenGLWidget(parent), mShader(NULL)
{
    mAngle = mTarget = 45;
    mCamFoV = 60;
    mCamDist = 7;
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(process()));
    t->start(1000/25.0);
}

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::init(const LbaBody &body, LbaAnimation *ani, int keyFrame, const LbaPalette &pal, int flags)
{
    makeCurrent();
    mRenderFlags = flags;
    mBody = body;
    mBody.setAnimation(ani);
    mBody.translateVertices(keyFrame);
    mPalette = pal;

    mGeometryBuffer.init(mBody,mPalette);
    update();
}

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::setCamFov(int fov)
{
    mCamFoV = fov;
    update();
}

void Lbw3dWidget::setCamDist(double dist)
{
    mCamDist = dist;
    update();
}

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::initializeGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->initializeOpenGLFunctions(); // obvious

    f->glEnable(GL_DEPTH_TEST);
    f->glDepthFunc(GL_LEQUAL) ;
    f->glClearColor(0.5f ,  0.5f ,  0.5f ,  1.0f ) ;
    f->glEnable(GL_CULL_FACE) ;
    f->glCullFace(GL_BACK);

     mShader = new LbwShader();
}

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::wheelEvent(QWheelEvent *event)
{
    mTarget += event->delta()/300.0;
}

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::process()
{
    float newA = mAngle + (mTarget - mAngle)/10;
    if (mAngle != newA) {
        mAngle = newA;
        update();
    }
}

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup perspective
    QMatrix4x4 perspectiveMatrix;
    perspectiveMatrix.perspective(mCamFoV,(float)width()/(float)height(),1,-1);

    // setup modelView (Rotation + Position)
    QMatrix4x4 rot;
    rot.rotate(mAngle*50,QVector3D(1,0,0));
    rot.rotate(mAngle*50/2.0,QVector3D(0,0,1));

    QMatrix4x4 pos;
    pos.translate(0,-.5,-mCamDist);

    QMatrix4x4 modelview = pos * rot;

    mShader->bind(perspectiveMatrix);
    mShader->drawBuffer(mGeometryBuffer,modelview);
}
