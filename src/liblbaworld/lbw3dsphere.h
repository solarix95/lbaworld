#ifndef LBW3DSPHERE_H
#define LBW3DSPHERE_H

#include <vector>
#include <QOpenGLWidget>

#include "lbw3d.h"
#include "lbabody.h"

class Lbw3dSphere
{
public:
    Lbw3dSphere(LbaBody::Vertex center, float radius, unsigned sectors);

    GLushort *indices(int &count) const;
    LbwVertex *vertices(int &count, QRgb c) const;

protected:
    std::vector<GLfloat>  mVertices;
    std::vector<GLfloat>  mNormals;
    std::vector<GLushort> mIndices;
};

#endif // LBW3DSPHERE_H
