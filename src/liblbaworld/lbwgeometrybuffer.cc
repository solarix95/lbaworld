#include "lbwgeometrybuffer.h"
#include <math.h>
#include <lbabody.h>
#include <lbaanimation.h>
#include <lbapalette.h>
#include <lbw3dsphere.h>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#define PI 3.1415f

//-------------------------------------------------------------------------------------------------
inline void normalize(LbwVector &v)
{
    float sum = sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
    if (sum > 0) {
        v.x /= sum;
        v.y /= sum;
        v.z /= sum;
    }
}

//-------------------------------------------------------------------------------------------------
inline void initVertex(LbwVertex &v, int vi, const LbaBody::Vertices &points, const LbaBody::Normals  &normals, const LbaBody::Polygon &polygon)
{
    float f = 800;
    v.p.x = points[polygon.vertices[vi]].x/f;
    v.p.y = points[polygon.vertices[vi]].y/f; \
    v.p.z = points[polygon.vertices[vi]].z/f; \
    v.w   = 1.0;                                   \
    v.n.x = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dx : 0;
    v.n.y = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dy : 0;
    v.n.z = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dz : 0;
    normalize(v.n);                   \
    {
        QRgb color = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].color : polygon.color;
        v.c.x = qRed(  color)/255.0;
        v.c.y = qGreen(color)/255.0;
        v.c.z = qBlue( color)/255.0;
    }
}
#define INITV(V) initVertex(v[vindex],V,points,normals,polygons[i]);

//-------------------------------------------------------------------------------------------------
LbwGeometryBuffer::LbwGeometryBuffer()
{
}

//-------------------------------------------------------------------------------------------------
void LbwGeometryBuffer::init(const LbaBody &body, const LbaPalette &pal, const LbaAnimation *animation)
{
    mAnimatedBody.clear(); // FIXME: cleanup registered buffer-Ids?

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    BodyShapes staticBody;

    {
        Shape lbaModel(PolygonMesh);

        LbwVertex *v = verticesFromBody(body,pal, lbaModel.verticesCount);
        lbaModel.vertexBufferId  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
        delete [] v;

        GLushort *elementOrder = new GLushort[lbaModel.verticesCount];
        for (int i=0; i<lbaModel.verticesCount; i++) {
            elementOrder[i] = i;
        }

        lbaModel.elementBufferId = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

        delete [] elementOrder;
        lbaModel.glType = GL_TRIANGLES;
        staticBody << lbaModel;
    }

    //----------------------------------------------------------------------------------------------------------------------
    {
        Shape lbaModel(Line);
        LbwVertex *v = linesFromBody(body,pal, lbaModel.verticesCount);
        lbaModel.vertexBufferId  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
        delete [] v;

        GLushort *elementOrder = new GLushort[lbaModel.verticesCount];
        for (int i=0; i<lbaModel.verticesCount; i++) {
            elementOrder[i] = i;
        }

        lbaModel.elementBufferId = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

        delete [] elementOrder;
        lbaModel.glType = GL_LINES;
        staticBody << lbaModel;
    }

    //----------------------------------------------------------------------------------------------------------------------

    {
        LbaBody::Spheres spheres = body.spheres();

        for (int i=0; i< spheres.count(); i++) {
            Lbw3dSphere s(body.vertices()[spheres[i].centerPoint],spheres[i].size/800.0,5);
            Shape lbaModel(Sphere);
            LbwVertex *v = s.vertices(lbaModel.verticesCount, pal.palette()[spheres[i].colorIndex]); // QColor(Qt::green).rgb());
            lbaModel.vertexBufferId  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
            delete [] v;

            GLushort *elementOrder = s.indices(lbaModel.verticesCount);

            lbaModel.elementBufferId = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

            delete [] elementOrder;
            lbaModel.glType = GL_TRIANGLES;
            staticBody << lbaModel;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------
    {
        Shape lbaModel(Bone);
        LbwVertex *v = bonesFromBody(body,pal, lbaModel.verticesCount);
        lbaModel.vertexBufferId  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
        delete [] v;

        GLushort *elementOrder = new GLushort[lbaModel.verticesCount];
        for (int i=0; i<lbaModel.verticesCount; i++) {
            elementOrder[i] = i;
        }

        lbaModel.elementBufferId = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

        delete [] elementOrder;
        lbaModel.glType = GL_LINES;
        staticBody << lbaModel;
    }
    mAnimatedBody << staticBody;
}

//-------------------------------------------------------------------------------------------------
GLuint LbwGeometryBuffer::makeBO(QOpenGLFunctions *f, GLenum type, const void *data, GLsizei size, int accessFlags)
{
    GLuint bo;
    f->glGenBuffers( 1, &bo );
    f->glBindBuffer( type, bo );
    f->glBufferData( type, size, data, accessFlags );
    return( bo );
}

//-------------------------------------------------------------------------------------------------
LbwVertex *LbwGeometryBuffer::verticesFromBody(const LbaBody &body, const LbaPalette &pal, int &count)
{
    LbwVertex *v = NULL;

    const LbaBody::Vertices   &points   = body.vertices();
    const LbaBody::Normals    &normals  = body.normals();
    const LbaBody::Polygons   &polygons = body.polygons();

    // Array von Triangles zusammenstellen:

    // Schritt1: wiviele Triangles haben wir?
    count = 0;
    for (int i=0; i<polygons.count(); i++) {
        if (polygons[i].vertices.count() == 3)
            count++;
        if (polygons[i].vertices.count() >= 4)
            count += polygons[i].vertices.count();
    }

    if (!count)
        return v;

    v = new LbwVertex[count*3];
    int vindex = 0;
    const QVector<QRgb> &colorTable = pal.palette();

    for (int i=0; i<polygons.count(); i++) {
        float f = 800;
        if (polygons[i].vertices.count() == 3) {
            INITV(0)
                    vindex++;

            INITV(1)
                    vindex++;

            INITV(2)
                    vindex++;
        }
        if (polygons[i].vertices.count() >= 4) {

            // Calculate center vertex

            float cx = 0;
            float cy = 0;
            float cz = 0;
            for (int p=0; p<polygons[i].vertices.count(); p++) {
                cx += points[polygons[i].vertices[p]].x;
                cy += points[polygons[i].vertices[p]].y;
                cz += points[polygons[i].vertices[p]].z;
            }
            cx /= polygons[i].vertices.count();
            cy /= polygons[i].vertices.count();
            cz /= polygons[i].vertices.count();

            // Normal Calculation: from center to polygon vertex 0 + 1:
            QVector3D c0(cx - points[polygons[i].vertices[0]].x,cy - points[polygons[i].vertices[0]].y,cz - points[polygons[i].vertices[0]].z);
            QVector3D c1(cx - points[polygons[i].vertices[1]].x,cy - points[polygons[i].vertices[1]].y,cz - points[polygons[i].vertices[1]].z);
            QVector3D cn = QVector3D::crossProduct(c0,c1);

            // Init Verteces
            for (int p=0; p<polygons[i].vertices.count(); p++) {
                int sibling = p < (polygons[i].vertices.count()-1) ? (p + 1) : 0; // sibling of the last is the first
                INITV(p);
                vindex++;
                INITV(sibling);
                vindex++;

                v[vindex].p.x = cx/f;
                v[vindex].p.y = cy/f;
                v[vindex].p.z = cz/f;
                v[vindex].w   = 1.0;
                v[vindex].n.x = cn.x();
                v[vindex].n.y = cn.y();
                v[vindex].n.z = cn.z();
                normalize(v[vindex].n);
                v[vindex].c.x = 3*qRed(  polygons[i].color)/255.0;
                v[vindex].c.y = 3*qGreen(polygons[i].color)/255.0;
                v[vindex].c.z = 3*qBlue( polygons[i].color)/255.0;

                vindex++;
            }
        }
    }

    count *= 3;
    Q_ASSERT(vindex == count);
    return v;
}

//-------------------------------------------------------------------------------------------------
LbwVertex *LbwGeometryBuffer::linesFromBody(const LbaBody &body, const LbaPalette &pal, int &count)
{
    LbwVertex *v = NULL;

    const LbaBody::Vertices   &points   = body.vertices();
    const LbaBody::Lines      &lines    = body.lines();
    count = lines.count();

    v = new LbwVertex[count*2];
    int vindex = 0;
    const QVector<QRgb> &colorTable = pal.palette();

    for (int i=0; i<lines.count(); i++) {
        float f = 800;
        QRgb colorP0 = body.polygonByPoint(lines[i].p0).color;

        v[vindex].p.x = points[lines[i].p0].x/f;
        v[vindex].p.y = points[lines[i].p0].y/f;
        v[vindex].p.z = points[lines[i].p0].z/f;
        // qDebug() << v[vindex].p.x << v[vindex].p.y*800 << v[vindex].p.z;
        v[vindex].w   = 1.0;

        v[vindex].n.x = 0;
        v[vindex].n.y = 0;
        v[vindex].n.z = 0;
        normalize(v[vindex].n);

        v[vindex].c.x = qRed(colorP0)/255.0;
        v[vindex].c.y = qGreen(colorP0)/255.0;
        v[vindex].c.z = qBlue(colorP0)/255.0;
        vindex++;

        QRgb colorP1 = body.polygonByPoint(lines[i].p1).color;

        v[vindex].p.x = points[lines[i].p1].x/f;
        v[vindex].p.y = points[lines[i].p1].y/f;
        v[vindex].p.z = points[lines[i].p1].z/f;
        v[vindex].w   = 1.0;

        v[vindex].n.x = 0;
        v[vindex].n.y = 0;
        v[vindex].n.z = 0;
        normalize(v[vindex].n);

        v[vindex].c.x = qRed(colorP1)/255.0;
        v[vindex].c.y = qGreen(colorP1)/255.0;
        v[vindex].c.z = qBlue(colorP1)/255.0;
        vindex++;
    }

    count *= 2;
    Q_ASSERT(vindex == count);
    return v;
}

//-------------------------------------------------------------------------------------------------
void LbwGeometryBuffer::createVertexByBone(const LbaBody &body, LbwVertex *&v, int boneId)
{
    LbaBody::Bone bone   = body.boneById(boneId);
    Q_ASSERT(bone.id >= 0);
    LbaBody::Bone parent = body.boneById(bone.parentId);
    if (parent.id >= 0) {
        LbaBody::Vertex p2 = body.vertices()[parent.parentVertex];
        LbaBody::Vertex p1 = body.vertices()[bone.parentVertex];

        float f = 800;
        v[0].p.x = p1.x/f;
        v[0].p.y = p1.y/f;
        v[0].p.z = p1.z/f;
        // qDebug() << v[vindex].p.x << v[vindex].p.y*800 << v[vindex].p.z;
        v[0].w   = 1.0;

        v[0].n.x = 0;
        v[0].n.y = 0;
        v[0].n.z = 0;
        normalize(v[0].n);

        v[0].c.x = 0;
        v[0].c.y = 0;
        v[0].c.z = 0;

        v[1].p.x = p2.x/f;
        v[1].p.y = p2.y/f;
        v[1].p.z = p2.z/f;
        // qDebug() << v[vindex].p.x << v[vindex].p.y*800 << v[vindex].p.z;
        v[1].w   = 1.0;

        v[1].n.x = 0;
        v[1].n.y = 0;
        v[1].n.z = 0;
        normalize(v[1].n);

        v[1].c.x = 0;
        v[1].c.y = 0;
        v[1].c.z = 0;

        v += 2;
    }

    LbaBody::Bones childs = body.childsOfBone(bone.id);
    for(int c=0; c < childs.count(); c++) {
        createVertexByBone(body,v,childs[c].id);
    }
}

//-------------------------------------------------------------------------------------------------
LbwVertex *LbwGeometryBuffer::bonesFromBody(const LbaBody &body, const LbaPalette & /*pal*/, int &count)
{
    LbwVertex *v = NULL;

    const LbaBody::Bones    &bones  = body.bones();

    count = bones.count();
    count = (count-1)*2; // pro bone 2 verteces, ohne Parent.

    if (count <= 0)
        return NULL;

    v = new LbwVertex[count];

    LbwVertex *vNext = v;
    createVertexByBone(body,vNext,0);

    return v;
}

LbwVertex *LbwGeometryBuffer::spheresFromBody(const LbaBody &body, const LbaPalette &pal, int &count)
{
    LbwVertex *v = NULL;

    const LbaBody::Vertices   &points   = body.vertices();
    const LbaBody::Spheres    &spheres  = body.spheres();
    count = spheres.count();

    v = new LbwVertex[count*2*210];
    int vindex = 0;
    const QVector<QRgb> &colorTable = pal.palette();

    float f = 800;
    for (int s=0; s<spheres.count(); s++) {
        // const float PI = 3.141592f;
        GLfloat x, y, z, alpha, beta; // Storage for coordinates and angles
        GLfloat radius = spheres[s].size/f;
        int gradation = 10;

        qDebug() << (gradation * (2.01*PI/(PI/gradation)));
        for (alpha = 0.0; alpha < PI; alpha += PI/gradation)
        {
            // glBegin(GL_TRIANGLE_STRIP);
            for (beta = 0.0; beta < 2.01*PI; beta += PI/gradation)
            {

                x = radius*cos(beta)*sin(alpha);
                y = radius*sin(beta)*sin(alpha);
                z = radius*cos(alpha);

                v[vindex].p.x = points[spheres[s].centerPoint].x/f + x;
                v[vindex].p.y = points[spheres[s].centerPoint].y/f + y;
                v[vindex].p.z = points[spheres[s].centerPoint].z/f + z;

                v[vindex].w   = 1.0;

                v[vindex].n.x = 0;
                v[vindex].n.y = 0;
                v[vindex].n.z = 0;
                normalize(v[vindex].n);

                v[vindex].c.x = qRed(colorTable[spheres[s].colorIndex])/255.0;
                v[vindex].c.y = qGreen(colorTable[spheres[s].colorIndex])/255.0;
                v[vindex].c.z = qBlue(colorTable[spheres[s].colorIndex])/255.0;

                vindex++;

                // glVertex3f(x, y, z);
                x = radius*cos(beta)*sin(alpha + PI/gradation);
                y = radius*sin(beta)*sin(alpha + PI/gradation);
                z = radius*cos(alpha + PI/gradation);
                // glVertex3f(x, y, z);
                v[vindex].p.x = points[spheres[s].centerPoint].x/f + x;
                v[vindex].p.y = points[spheres[s].centerPoint].y/f + y;
                v[vindex].p.z = points[spheres[s].centerPoint].z/f + z;

                v[vindex].w   = 1.0;

                v[vindex].n.x = 0;
                v[vindex].n.y = 0;
                v[vindex].n.z = 0;
                normalize(v[vindex].n);

                v[vindex].c.x = qRed(colorTable[spheres[s].colorIndex])/255.0;
                v[vindex].c.y = qGreen(colorTable[spheres[s].colorIndex])/255.0;
                v[vindex].c.z = qBlue(colorTable[spheres[s].colorIndex])/255.0;

                vindex++;
            }
            // glEnd();
        }
    }

    return v;
}
