#ifndef LBW3DWIDGET_H
#define LBW3DWIDGET_H


#include <QList>
#include <lbw3d.h>
#include <lbabody.h>
#include <lbapalette.h>
#include <lbaanimation.h>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

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
    void setupShader();
    void setupModels();

    static GLuint     makeBO( QOpenGLFunctions *f, GLenum type, const void* data, GLsizei size, int accessFlags );
    static LbwVertex *verticesFromBody(const LbaBody &body, const LbaPalette &pal, int &count);
    static LbwVertex *linesFromBody(const LbaBody &body, const LbaPalette &pal, int &count);
    static void       createVertexByBone(const LbaBody &body,LbwVertex *&v, int boneId);
    static LbwVertex *bonesFromBody(const LbaBody &body, const LbaPalette &pal, int &count);
    static LbwVertex *spheresFromBody(const LbaBody &body, const LbaPalette &pal, int &count);

    int   mRenderFlags;
    float mAngle;
    float mTarget;
    float mCamFoV;
    float mCamDist;

    GLuint projectionMatrix;
    GLuint modelviewMatrix;
    GLuint normalviewMatrix;

    // These are variables passed into shaders
    GLuint vertexPosition;
    GLuint vertexNormal;
    GLuint vertexColor;

    struct Shape {
        int   glType; // usualy GL_TRIANGLES

        QMatrix4x4 modelView;

        // Shaders
        GLuint vertexBuffer;
        GLuint elementBuffer;
        int    verticesCount;
        Shape() {  modelView.data()[15] = 1; }
    };

    QList<Shape>           mShapes;
    LbaBody                mBody;
    LbaPalette             mPalette;
    QOpenGLShader         *mVertexShader;
    QOpenGLShader         *mFragmentShader;
    QOpenGLShaderProgram  *mShaderProgram;
};


#endif // LBW3DWIDGET_H
