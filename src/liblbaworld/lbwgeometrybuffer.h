#ifndef LBWGEOMETRYBUFFER_H
#define LBWGEOMETRYBUFFER_H

#include <QList>
#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "lbw3d.h"

class LbaBody;
class LbaAnimation;
class LbaPalette;

class LbwGeometryBuffer
{
public:
    LbwGeometryBuffer();
    friend class LbwShader;

    void init(const LbaBody &body, const LbaPalette &pal,  const LbaAnimation *animation = NULL);

private:
    static GLuint     makeBO( QOpenGLFunctions *f, GLenum type, const void* data, GLsizei size, int accessFlags );
    static LbwVertex *verticesFromBody(const LbaBody &body, const LbaPalette &pal, int &count);
    static LbwVertex *linesFromBody(const LbaBody &body, const LbaPalette &pal, int &count);
    static void       createVertexByBone(const LbaBody &body,LbwVertex *&v, int boneId);
    static LbwVertex *bonesFromBody(const LbaBody &body, const LbaPalette &pal, int &count);
    static LbwVertex *spheresFromBody(const LbaBody &body, const LbaPalette &pal, int &count);

    struct Shape {
        LbwShapeType  shType; // Polygone, Line, Spheres,..
        int           glType; // usualy GL_TRIANGLES

        QMatrix4x4 modelView;

        // Shaders
        GLuint vertexBufferId;
        GLuint elementBufferId;
        int    verticesCount;
        Shape(LbwShapeType t): shType(t) {  modelView.data()[15] = 1; }
    };

    typedef QList<Shape>          BodyShapes;
    typedef QList<BodyShapes>     BodyAnimation;

    BodyAnimation mAnimatedBody;
};

#endif // LBWGEOMETRYBUFFER_H
