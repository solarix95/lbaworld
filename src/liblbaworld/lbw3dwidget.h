#ifndef LBW3DWIDGET_H
#define LBW3DWIDGET_H


#include <QList>
#include <QTime>
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

signals:
    void ready2run();

public slots:
    void setCamFov(int fov);
    void setCamDist(double dist);

protected:
    virtual void paintGL() override;
    virtual void paint2dLayers();
    virtual void initializeGL() override;
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void wheelEvent(QWheelEvent * event) override;
    virtual void initWidget();

private slots:
     void process();

protected:
     virtual bool processState(float speed); // normalized game speed (1 ~ normspeed)

private:
    void setupModels();

    int   mRenderFlags;
    float mAngle;
    float mTarget;
    float mCamFoV;
    float mCamDist;

    QTime mStopWatch;
    LbaBody                mBody;
    LbaPalette             mPalette;
    LbwShader             *mShader;
    LbwGeometryBuffer      mGeometryBuffer;
};


#endif // LBW3DWIDGET_H
