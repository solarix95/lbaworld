#ifndef LBWABSTRACTGEOMETRYBUFFER_H
#define LBWABSTRACTGEOMETRYBUFFER_H

#include <QString>
#include <QOpenGLFunctions>
#include <lbw3d.h>
#include <lbwabstractgeometrystate.h>

class LbwAbstractGeometryState;

class LbwAbstractGeometryBuffer
{
    friend class LbwShader;
public:
    LbwAbstractGeometryBuffer(const QString &url);
    virtual ~LbwAbstractGeometryBuffer();

    QString url() const;

    void registerState(LbwAbstractGeometryState *state);
    void unregisterState(LbwAbstractGeometryState *state);


protected:

    struct Shape {
        LbwShapeType  shType; // Polygone, Line, Spheres,..
        int           glType; // usualy GL_TRIANGLES

        // Shaders
        GLuint vertexBufferId;
        GLuint elementBufferId;
        int    verticesCount;
        Shape(LbwShapeType t): shType(t) { }
    };

    typedef QList<Shape> Shapes;

    // Shader-Interface
    /*
      Foreach state
        render all shapes
    */
    virtual const Shapes              &shapes() const = 0;
    virtual const LbwAbstractGeometryStates &states() const;

private:
    LbwAbstractGeometryStates mStates;
    QString                   mUrl;

};

#endif // LBWABSTRACTGEOMETRYBUFFER_H
