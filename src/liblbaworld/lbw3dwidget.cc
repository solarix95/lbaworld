
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


static inline void MatrixAsUniform(QOpenGLFunctions *f, GLuint location, QMatrix4x4 m) { f->glUniformMatrix4fv(location,1,GL_TRUE,m.data()); }
#define PI 3.1415f

#define INITVX(V) \
    v[vindex].p.x = points[polygons[i].vertices[V]].x/f; \
    v[vindex].p.y = points[polygons[i].vertices[V]].y/f; \
    v[vindex].p.z = points[polygons[i].vertices[V]].z/f; \
    v[vindex].w   = 1.0;                                   \
    v[vindex].n.x = polygons[i].normals.count() > 0 ? normals[polygons[i].normals[V]].dx : 0;  \
    v[vindex].n.y = polygons[i].normals.count() > 0 ? normals[polygons[i].normals[V]].dy : 0;  \
    v[vindex].n.z = polygons[i].normals.count() > 0 ? normals[polygons[i].normals[V]].dz : 0;  \
    normalize(v[vindex].n);                   \
    { \
       int colorIndex = polygons[i].normals.count() > 0 ? normals[polygons[i].normals[V]].colorIndex : polygons[i].lbaColorIndex; \
       v[vindex].c.x = 3*qRed(  colorTable[colorIndex])/255.0; \
       v[vindex].c.y = 3*qGreen(colorTable[colorIndex])/255.0; \
       v[vindex].c.z = 3*qBlue( colorTable[colorIndex])/255.0;\
    }

    // v[vindex].c.x = 3*qRed(  colorTable[polygons[i].lbaColorIndex])/255.0;
    // v[vindex].c.y = 3*qGreen(colorTable[polygons[i].lbaColorIndex])/255.0;
    // v[vindex].c.z = 3*qBlue( colorTable[polygons[i].lbaColorIndex])/255.0;

void normalize(LbwVector &v)
{
    float sum = sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
    if (sum > 0) {
        v.x /= sum;
        v.y /= sum;
        v.z /= sum;
    }
}

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
Lbw3dWidget::Lbw3dWidget(QWidget *parent)
 : QOpenGLWidget(parent)
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

    // QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    for(int i=0; i<mShapes.count(); i++) {
        // TODO: Clean up?
        // f->glDelete.. what?
    }
    mShapes.clear();
    setupModels();
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

    setupShader();
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
void Lbw3dWidget::setupShader()
{
    mVertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    mVertexShader->compileSourceFile(":/phong.vert");
    Q_ASSERT(mVertexShader->isCompiled());

    mFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    mFragmentShader->compileSourceFile(":/phong.frag");
    Q_ASSERT(mFragmentShader->isCompiled());


    mShaderProgram = new QOpenGLShaderProgram();
    mShaderProgram->addShader(mVertexShader);
    mShaderProgram->addShader(mFragmentShader);

    bool done = mShaderProgram->link();
    Q_ASSERT_X(done,"Shader Linker", mShaderProgram->log().toUtf8().data());

    // Get locations of attributes and uniforms used inside.
    vertexPosition = mShaderProgram->attributeLocation("vertex" );
    vertexNormal = mShaderProgram->attributeLocation("vnormal" );
    vertexColor  = mShaderProgram->attributeLocation("vcolor" );

    modelviewMatrix  = mShaderProgram->uniformLocation("modelview" );
    normalviewMatrix = mShaderProgram->uniformLocation("normalview" );
    projectionMatrix = mShaderProgram->uniformLocation("projection" );

}

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::setupModels()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    // Setup LBA Polygon Model
    if (mRenderFlags & 0x01)
    {
        Shape lbaModel;
        LbwVertex *v = verticesFromBody(mBody,mPalette, lbaModel.verticesCount);
        lbaModel.vertexBuffer  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
        delete [] v;

        GLushort *elementOrder = new GLushort[lbaModel.verticesCount];
        for (int i=0; i<lbaModel.verticesCount; i++) {
            elementOrder[i] = i;
        }

        lbaModel.elementBuffer = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

        delete [] elementOrder;
        lbaModel.glType = GL_TRIANGLES;
        mShapes << lbaModel;
    }

    // Setup LBA Model Lines
    if (mRenderFlags & 0x02)
    {
        Shape lbaModel;
        LbwVertex *v = linesFromBody(mBody,mPalette, lbaModel.verticesCount);
        lbaModel.vertexBuffer  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
        delete [] v;

        GLushort *elementOrder = new GLushort[lbaModel.verticesCount];
        for (int i=0; i<lbaModel.verticesCount; i++) {
            elementOrder[i] = i;
        }

        lbaModel.elementBuffer = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

        delete [] elementOrder;
        lbaModel.glType = GL_LINES;
        mShapes << lbaModel;
    }

    if (mRenderFlags & 0x04) {
        LbaBody::Spheres spheres = mBody.spheres();

        for (int i=0; i< spheres.count(); i++) {
            Lbw3dSphere s(mBody.vertices()[spheres[i].centerPoint],spheres[i].size/800.0,5);
            Shape lbaModel;
            LbwVertex *v = s.vertices(lbaModel.verticesCount, mPalette.palette()[spheres[i].colorIndex]); // QColor(Qt::green).rgb());
            lbaModel.vertexBuffer  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
            delete [] v;

            GLushort *elementOrder = s.indices(lbaModel.verticesCount);

            lbaModel.elementBuffer = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

            delete [] elementOrder;
            lbaModel.glType = GL_TRIANGLES;
            // lbaModel.modelView.scale(0.1,0.1,0.1);
            mShapes << lbaModel;
        }
    }

    // Setup LBA Model Bones
    if (mRenderFlags & 0x08) {
        Shape lbaModel;
        LbwVertex *v = bonesFromBody(mBody,mPalette, lbaModel.verticesCount);
        lbaModel.vertexBuffer  = makeBO(f,GL_ARRAY_BUFFER,v,lbaModel.verticesCount * sizeof(LbwVertex),GL_STATIC_DRAW);
        delete [] v;

        GLushort *elementOrder = new GLushort[lbaModel.verticesCount];
        for (int i=0; i<lbaModel.verticesCount; i++) {
            elementOrder[i] = i;
        }

        lbaModel.elementBuffer = makeBO(f, GL_ELEMENT_ARRAY_BUFFER, elementOrder, lbaModel.verticesCount*sizeof( GLushort ), GL_STATIC_DRAW);

        delete [] elementOrder;
        lbaModel.glType = GL_LINES;
        mShapes << lbaModel;
    }
}

//-------------------------------------------------------------------------------------------------
GLuint Lbw3dWidget::makeBO(QOpenGLFunctions *f, GLenum type, const void *data, GLsizei size, int accessFlags)
{
    GLuint bo;
    f->glGenBuffers( 1, &bo );
    f->glBindBuffer( type, bo );
    f->glBufferData( type, size, data, accessFlags );
    return( bo );
}

//-------------------------------------------------------------------------------------------------
LbwVertex *Lbw3dWidget::verticesFromBody(const LbaBody &body, const LbaPalette &pal, int &count)
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
LbwVertex *Lbw3dWidget::linesFromBody(const LbaBody &body, const LbaPalette &pal, int &count)
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
void Lbw3dWidget::createVertexByBone(const LbaBody &body, LbwVertex *&v, int boneId)
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
LbwVertex *Lbw3dWidget::bonesFromBody(const LbaBody &body, const LbaPalette & /*pal*/, int &count)
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

LbwVertex *Lbw3dWidget::spheresFromBody(const LbaBody &body, const LbaPalette &pal, int &count)
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

//-------------------------------------------------------------------------------------------------
void Lbw3dWidget::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    QMatrix4x4 perspectiveMatrix;
    perspectiveMatrix.perspective(mCamFoV,(float)width()/(float)height(),1,-1);


    // Activate shader.
    f->glUseProgram( mShaderProgram->programId() );
    // mShaderProgram->bind();

    for (int i=0; i<mShapes.count(); i++) {

        Shape &s = mShapes[i];

        // MatrixAsUniform(f,projectionMatrix,perspectiveMatrix * s.modelView);
        MatrixAsUniform(f,projectionMatrix,perspectiveMatrix);

        // Vertices
        f->glBindBuffer( GL_ARRAY_BUFFER, s.vertexBuffer );
        f->glVertexAttribPointer(
                    vertexPosition,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(GLfloat) * 10,
                    (void*)0
                    );
        f->glEnableVertexAttribArray( vertexPosition );

        // Normals
        f->glVertexAttribPointer(
                    vertexNormal,
                    3,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(GLfloat) * 10,
                    (void*)(sizeof(GLfloat) * 4)
                    );
        f->glEnableVertexAttribArray( vertexNormal );

        // Colors
        f->glVertexAttribPointer(
                    vertexColor,
                    3,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(GLfloat) * 10,
                    (void*)(sizeof(GLfloat) * 7)
                    );
        f->glEnableVertexAttribArray( vertexColor );


        // Set modelview - the objects rotation and translation.
        QMatrix4x4 rot;
        rot.rotate(mAngle*50,QVector3D(1,0,0));
        rot.rotate(mAngle*50/2.0,QVector3D(0,0,1));

        QMatrix4x4 pos;
        pos.translate(0,-.5,-mCamDist);

        QMatrix4x4 modelview = pos * rot;
        MatrixAsUniform( f, modelviewMatrix, modelview );

        // Normal view matrix - inverse transpose of modelview.
        QMatrix4x4 normalView = modelview.inverted().transposed();
        MatrixAsUniform(f, normalviewMatrix, normalView );


        // Send element buffer to GPU and draw.
        f->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, s.elementBuffer );
        f->glDrawElements(
                    s.glType,
                    s.verticesCount,
                    GL_UNSIGNED_SHORT,
                    (void*)0
                    );

        // Clean up
        f->glDisableVertexAttribArray( vertexPosition );
        f->glDisableVertexAttribArray( vertexNormal );
        f->glDisableVertexAttribArray( vertexColor );
    }
}
