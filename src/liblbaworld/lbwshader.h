#ifndef LBWSHADER_H
#define LBWSHADER_H

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

class LbwGeometryBuffer;
class LbwShader
{
public:
    LbwShader();

    void bind(const QMatrix4x4 &perspectiveMatrix);
    void drawBuffer(const LbwGeometryBuffer &buffer, const QMatrix4x4 &modelView, int animationState = -1);

private:

    // These are variables passed into shaders
    GLuint projectionMatrix;
    GLuint modelviewMatrix;
    GLuint normalviewMatrix;

    GLuint vertexPosition;
    GLuint vertexNormal;
    GLuint vertexColor;

    QOpenGLShader         *mVertexShader;
    QOpenGLShader         *mFragmentShader;
    QOpenGLShaderProgram  *mShaderProgram;
};

#endif // LBWSHADER_H
