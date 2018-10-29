#include <QDebug>
#include <QVector3D>
#include <QMatrix4x4>
#include <QVector4D>
#include "lbabody.h"
#include "binaryreader.h"
#include "lbaanimation.h"

//-------------------------------------------------------------------------------------------
LbaBody::LbaBody()
 : mAnimation(NULL)
{

}

//-------------------------------------------------------------------------------------------
LbaBody::~LbaBody()
{
    if (mAnimation)
        delete mAnimation;
}

//-------------------------------------------------------------------------------------------
bool LbaBody::fromLba1Buffer(const QByteArray &buffer)
{
    BinaryReader reader(buffer);

    qint16 bodyflag;

    reader.read(&bodyflag, 2);
    reader.skip(2*12);

    loadLba1Points(reader);
    loadLba1Bones(reader);
    loadLba1Normals(reader);
    loadLba1Polygones(reader);
    loadLba1Lines(reader);
    loadLba1Spheres(reader);

    return true;

}

//-------------------------------------------------------------------------------------------
bool LbaBody::animationFromBuffer(const QByteArray &buffer)
{
    if (mAnimation)
        delete mAnimation;
    mAnimation = NULL;
    if (buffer.isEmpty())
        return false;
    mAnimation = new LbaAnimation();
    mAnimationKeyFrame = 0;
    return mAnimation->fromBuffer(buffer);
}

//-------------------------------------------------------------------------------------------
void LbaBody::setAnimation(LbaAnimation *ani)
{
    if (mAnimation)
        delete mAnimation;
    mAnimation = ani;
    mAnimationKeyFrame = 0;
}

//-------------------------------------------------------------------------------------------
void LbaBody::translateVertices(int keyFrame)
{
    mAnimationKeyFrame = keyFrame;
    Points tv = mPoints;

   // QVector3D matrix(0,0,0);
   QMatrix4x4 matrix;
   translateVertices(-1,matrix, tv);

   mPointsTranslated = tv;
}

//-------------------------------------------------------------------------------------------
void LbaBody::translateVertices(int parentId, QMatrix4x4 matrix, Points &vertices)
{
    Bones nextBones = childsOfBone(parentId);

    for(int i=0; i<nextBones.count(); i++) {

        QMatrix4x4 subMatrix;
        subMatrix.translate( mPoints[nextBones[i].parentVertex].x,
                             mPoints[nextBones[i].parentVertex].y,
                             mPoints[nextBones[i].parentVertex].z);


        QMatrix4x4 subMatrixAni;
        if (nextBones[i].boneType == 0) {

            if (nextBones[i].rotateZ)
                subMatrixAni.rotate(nextBones[i].rotateZ,0,0,1);
            if (nextBones[i].rotateY)
                subMatrixAni.rotate(nextBones[i].rotateY,0,1,0);
            if (nextBones[i].rotateX)
                subMatrixAni.rotate(nextBones[i].rotateX,1,0,0);

        } else {
            subMatrixAni.translate( nextBones[i].rotateX/800.0,
                                 nextBones[i].rotateY/800.0,
                                 nextBones[i].rotateZ/800.0);
        }


        QMatrix4x4 boneMatrix = matrix * subMatrix * subMatrixAni;

        for (int v=nextBones[i].firstVertex; v < (nextBones[i].firstVertex + nextBones[i].numVertices); v++) {

            QVector4D vec(mPoints[v].x,mPoints[v].y,mPoints[v].z,1);
            vec = boneMatrix * vec;

            vertices[v].x = vec.x();
            vertices[v].y = vec.y();
            vertices[v].z = vec.z();

        }
        translateVertices(nextBones[i].id,boneMatrix, vertices);
    }

}

//-----------------------------------------------------------------------------------------
const LbaBody::Points &LbaBody::points() const
{
    return mPointsTranslated.count() > 0 ? mPointsTranslated : mPoints;
}

//-------------------------------------------------------------------------------------------
const LbaBody::Points &LbaBody::normals() const
{
    return mNormals;
}

//-------------------------------------------------------------------------------------------
const LbaBody::Polygons &LbaBody::polygons() const
{
    return mPolygones;
}

//-----------------------------------------------------------------------------------------
LbaBody::Polygon LbaBody::polygonByPoint(int pointIndex) const
{
    for (int i=0; i<mPolygones.count(); i++) {
        if (mPolygones[i].vertices.contains(pointIndex))
            return mPolygones[i];
    }
    return Polygon();
}

//-------------------------------------------------------------------------------------------
const LbaBody::Bones &LbaBody::bones() const
{
    return mBones;
}

//-------------------------------------------------------------------------------------------
const LbaBody::Lines &LbaBody::lines() const
{
    return mLines;
}

//-------------------------------------------------------------------------------------------
const LbaBody::Spheres &LbaBody::spheres() const
{
    return mSpheres;
}

//-----------------------------------------------------------------------------------------
LbaBody::Bones LbaBody::childsOfBone(int parentId) const
{
    LbaBody::Bones childs;
    for (int i=0; i<mBones.count(); i++) {
        if (mBones[i].parentId == parentId) {
            childs << mBones[i];
            if (mAnimation) {
                childs.last().boneType = mAnimation->bones(mAnimationKeyFrame)[i].boneType;
                childs.last().rotateX  = mAnimation->bones(mAnimationKeyFrame)[i].rotateX;
                childs.last().rotateY  = mAnimation->bones(mAnimationKeyFrame)[i].rotateY;
                childs.last().rotateZ  = mAnimation->bones(mAnimationKeyFrame)[i].rotateZ;
            }
        }
    }

    return childs;
}

//-----------------------------------------------------------------------------------------
LbaBody::Bone LbaBody::boneById(int id) const
{
    for (int i=0; i<mBones.count(); i++) {
        if (mBones[i].id == id)
            return mBones[i];
    }

    return Bone();
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Points(BinaryReader &reader)
{
    quint16 verticesCount;
    reader.read(&verticesCount, 2);

    mPoints.clear();

    for (int i=0; i<verticesCount; i++) {
        qint16 x,y,z;
        reader.read(&x, 2);
        reader.read(&y, 2);
        reader.read(&z, 2);
        qDebug() << "Point" << i << x << y << z;
        mPoints << Point(x,y,z);
    }
}

//-----------------------------------------------------------------------------------------
//                                     PRIVATE
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Bones(BinaryReader &reader)
{
    quint16 bonesCount;
    reader.read(&bonesCount, 2);
    for (int i=0; i<bonesCount; i++) {
        /*
    int16 firstPoint;		// data1
    int16 numOfPoints;		// data2
    int16 basePoint;		// data3
    int16 baseElement;		// param
    int16 flag;
    int16 rotateZ;
    int16 rotateY;
    int16 rotateX;
    int32 numOfShades;			// field_10
    int32 field_14;
    int32 field_18;
    int32 Y;
    int32 field_20;
    int16 field_24;

         */
        quint16 firstPoint;
        reader.read(&firstPoint, 2);
        Q_ASSERT(!(firstPoint % 6));
        firstPoint /= 6;

        quint16 numPoints;
        reader.read(&numPoints,2);

        quint16 parentPoint;
        reader.read(&parentPoint, 2);
        Q_ASSERT(!(parentPoint % 6));
        parentPoint /= 6;

        qint16 parentBone;
        reader.read(&parentBone, 2);
        if (parentBone != -1) {
            Q_ASSERT(!(parentBone % 38));
            parentBone /= 38;
        }

        qint16 boneType;
        qint16 x,y,z;

        reader.read(&boneType, 2);
        reader.read(&x, 2);
        reader.read(&y, 2);
        reader.read(&z, 2);

        qint16 nrNormals;
        reader.read(&nrNormals, 2);
        reader.skip(2);
        reader.skip(4);
        reader.skip(4);
        reader.skip(4);
        reader.skip(4);
        reader.skip(2);

        Bone b;
        b.id           = i;
        b.parentId     = parentBone;
        b.boneType     = boneType;

        b.firstVertex  = firstPoint;
        b.numVertices  = numPoints;
        b.parentVertex = parentPoint;

        b.rotateX      = x;
        b.rotateY      = y;
        b.rotateZ      = z;

        mBones << b;
        qDebug() << "BONE" << i << firstPoint << numPoints << parentPoint  << parentBone << boneType << x << y << z << nrNormals;
    }

}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Normals(BinaryReader &reader)
{
    quint16 normalsCount;
    reader.read(&normalsCount, 2);
    mNormals.clear();
    for (int i=0; i<normalsCount; i++) {
        qint16 x,y,z,w;
        reader.read(&x, 2);
        reader.read(&y, 2);
        reader.read(&z, 2);
        reader.read(&w, 2); // Unknown Word..
        // qDebug() << "Normal" << i << x << y << z;
        mNormals << Point(x,y,z);
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Polygones(BinaryReader &reader)
{
    quint16 polygonCount;
    mPolygones.clear();
    reader.read(&polygonCount, 2);
    for (int i=0; i<polygonCount; i++) {

        quint8 renderType, nrVertices, colorIndex;
        reader.read(&renderType, 1);
        reader.read(&nrVertices, 1);
        reader.read(&colorIndex, 1);
        reader.skip(1); // Unknown
        qDebug() << "Polygon" << i << renderType << nrVertices << colorIndex;
        Polygon p;
        p.lbaColorIndex = colorIndex;
        if (renderType >= 9) {             // each vertex has a normal
            for (int j=0; j<nrVertices; j++) {
                quint16 normal, vertex;
                reader.read(&normal,2);
                reader.read(&vertex,2);
                vertex /= 6;
                // qDebug() << "  Vertex" << vertex << normal;
                p.vertices << vertex;
                p.normals  << normal;
                // polygon_vertices.append((v, normal))
            }
        } else if (renderType >= 7) {      // one normal for the whole polygon
            quint16 normal, vertex;
            reader.read(&normal, 2);
            for (int j=0; j<nrVertices; j++) {
                reader.read(&vertex,2);
                vertex /= 6;
                // qDebug() << "  Vertex" << vertex << normal;
                p.vertices << vertex;
                p.normals  << normal;
                // polygon_vertices.append((v, normal))
            }
        } else {                           // no normal
            for (int j=0; j<nrVertices; j++) {
                quint16 vertex;
                reader.read(&vertex,2);
                vertex /= 6;
               p.vertices << vertex;
                // qDebug() << "  Vertex" << vertex;
                // polygon_vertices.append((v, ))
            }
        }

        if (!p.vertices.isEmpty())
            mPolygones << p;
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Lines(BinaryReader &reader)
{
    quint16 lineCount;
    reader.read(&lineCount, 2);
    mLines.clear();
    for (int i=0; i<lineCount; i++) {
        reader.skip(sizeof(quint32)); // data = u32().. data?!?
        quint16 vertex1;
        quint16 vertex2;
        reader.read(&vertex1,2);
        reader.read(&vertex2,2);
        vertex1 /= 6;
        vertex2 /= 6;
        // qDebug() << "Line" << i << vertex1 << vertex2;
        mLines << Line(vertex1, vertex2);
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Spheres(BinaryReader &reader)
{
    quint16 sphereCount;
    reader.read(&sphereCount, 2);
    for (int i=0; i<sphereCount; i++) {
        reader.skip(1);
        quint8  color;
        reader.read(&color,1);
        reader.skip(2);

        quint16 size;
        reader.read(&size,2);

        quint16 vertex;
        reader.read(&vertex,2);
        vertex /= 6;
        // qDebug() << "Sphere" << i << vertex << size << color;
        Sphere s;
        s.centerPoint = vertex;
        s.size        = size;
        s.colorIndex  = color;
        mSpheres << s;
    }
}

