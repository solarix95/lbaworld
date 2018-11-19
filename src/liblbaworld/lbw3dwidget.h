#ifndef LBW3DWIDGET_H
#define LBW3DWIDGET_H


#include <QList>

#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#include <lbw3d.h>
#include <lbabody.h>
#include <lbapalette.h>
#include <lbaanimation.h>
#include <lbwgeometrybuffer.h>
#include <lbwshader.h>

class Lbw3dWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit Lbw3dWidget(QWidget *parent = 0);

    void init(const LbaBody &body, LbaAnimation *ani, int keyFrame, const LbaPalette &pal, int flags);

public slots:
    void setCamFov(int fov);
    void setCamDist(double dist);

protected:
    virtual void paintGL();
    virtual void initializeGL();
    virtual void wheelEvent(QWheelEvent * event);

private slots:
    virtual void process();

private:
    void setupModels();

    int   mRenderFlags;
    float mAngle;
    float mTarget;
    float mCamFoV;
    float mCamDist;

    LbaBody                mBody;
    LbaPalette             mPalette;
    LbwShader             *mShader;
    LbwGeometryBuffer      mGeometryBuffer;
};


#endif // LBW3DWIDGET_H
