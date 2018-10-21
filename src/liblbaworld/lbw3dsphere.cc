#include <math.h>
#include "lbw3dsphere.h"

//-------------------------------------------------------------------------------------------------
#ifndef PI
#define PI 3.1415
#endif

//-------------------------------------------------------------------------------------------------
Lbw3dSphere::Lbw3dSphere(LbaBody::Point center, float radius, unsigned int sectors)
{
    int nVert = (1 + sectors) + (sectors * (sectors-3) + 1);

    mVertices.resize(nVert*3);
    mNormals.resize(nVert*3);
    mIndices.resize(sectors*3 + 2*(sectors-3)*(sectors*3) + sectors*3);

    std::vector<GLfloat>::iterator  v = mVertices.begin();
    std::vector<GLfloat>::iterator  n = mNormals.begin();
    std::vector<GLushort>::iterator i = mIndices.begin();

    float f = 800;
    float cx = center.x/f;
    float cy = center.y/f;
    float cz = center.z/f;

    /* top */
    *v++ = cx;
    *v++ = cy;
    *v++ = cz + radius;

    *n++ =  0.f;
    *n++ = 0.f;
    *n++ = 1;

    float stepXY = 2*PI/sectors;
    float angleZ  = PI/(sectors-1);

    // Top (hat)
    {
        float r = sin(angleZ) * radius;
        float z = cos(angleZ);

        for (int s=0; s<sectors; s++) {

            float x = sin(stepXY*s);
            float y = cos(stepXY*s);

            *n++ =   x;
            *v++ = r*x + cx;

            *n++ =   y;
            *v++ = r*y + cy;

            *n++ =   z;
            *v++ = radius*z + cz;

            *i++ = s+1;
            *i++ = 0; // all polygons to the top center
            *i++ = s == (sectors-1) ? 1 : s+2;
        }
    }


    for (int ring = 0; ring<sectors-3; ring++) {
        // r = sin(PI * ((ring+2)*stepZ)/2) * radius;
        float r = sin((ring+2)*angleZ) * radius;
        float z = cos((ring+2)*angleZ);

        for (int s=0; s<sectors; s++) {

            float x = sin(stepXY*s);
            float y = cos(stepXY*s);

            *n++ =   x;
            *v++ = r*x + cx;

            *n++ =   y;
            *v++ = r*y + cy;

            *n++ =   z;
            *v++ =  radius*z + cz;
        }
    }

    /* Button */
    *v++ = cx;
    *v++ = cy;
    *v++ = cz - radius;

    *n++ =  0.f;
    *n++ = 0.f;
    *n++ = -1;

    for (int ring = 0; ring<sectors-3; ring++) {
        for (int s=0; s<sectors; s++) {

            int first = 1 +(ring*sectors);
            // Oberes Dreieck
            *i++ = first + s;
            *i++ = s == sectors - 1 ? (first) : (first + s + 1);
            *i++ = first + s + sectors;// 1 + s + ((ring+1)*sectors);

            // Unteres Dreieck
            *i++ = first + s;
            *i++ = first + s + sectors;
            *i++ = s == 0 ? (first + 2*sectors - 1) : (first + s + sectors - 1);
        }
    }

    // Button
    for (int s=0; s<sectors; s++) {
        int first = 1+ ((sectors-3)*sectors);
        *i++ = first + s;
        *i++ = s == sectors - 1 ? (first) : (first + s + 1);
        *i++ = 1 + (sectors-2)*sectors;
    }
}

//-------------------------------------------------------------------------------------------------
LbwVertex *Lbw3dSphere::vertices(int &count, QRgb c) const
{
    count = mVertices.size();
    LbwVertex *v = new LbwVertex[count];
    for (int i=0; i<count; i++) {
        v[i].p.x = mVertices[i*3 + 0];
        v[i].p.y = mVertices[i*3 + 1];
        v[i].p.z = mVertices[i*3 + 2];

        v[i].w   = 1.0;

        v[i].n.x = mNormals[i*3 + 0];
        v[i].n.y = mNormals[i*3 + 1];
        v[i].n.z = mNormals[i*3 + 2];
        v[i].n.normalize();

        v[i].c.x = qRed(c)/255.0;
        v[i].c.y = qGreen(c)/255.0;
        v[i].c.z = qBlue(c)/255.0;
    }

    return v;
}

//-------------------------------------------------------------------------------------------------
GLushort *Lbw3dSphere::indices(int &count) const
{
    count = mIndices.size();

    GLushort *elementOrder = new GLushort[count];

    for (int i=0; i<count; i++) {
        elementOrder[i] = mIndices[i];
    }

    return elementOrder;
}
