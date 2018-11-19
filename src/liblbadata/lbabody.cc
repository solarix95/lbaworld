#include <QDebug>
#include <QVector3D>
#include <QMatrix4x4>
#include <QVector4D>
#include <QImage>
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
bool LbaBody::fromLba1Buffer(const QByteArray &buffer, const LbaPalette &pal)
{
    BinaryReader reader(buffer);

    qint16 bodyflag;

    reader.read(&bodyflag, 2);
    reader.skip(2*12);

    loadLba1Points(reader);
    loadLba1Bones(reader);
    loadLba1Normals(reader);
    loadLba1Polygones(reader,pal);
    loadLba1Lines(reader);
    loadLba1Spheres(reader);

    return true;

}

//-------------------------------------------------------------------------------------------
bool LbaBody::fromLba2Buffer(const QByteArray &buffer, const LbaPalette &pal, const QImage &uvTexture)
{
    BinaryReader reader(buffer);

    // https://github.com/agrande/lba2remake/blob/master/src/model/body.js
    qint32 bodyFlag = reader.getInt32(0x00);


    qint32 xMin = reader.getInt32(0x08);
    qint32 xMax = reader.getInt32(0x0C);
    qint32 yMin = reader.getInt32(0x10);
    qint32 yMax = reader.getInt32(0x14);
    qint32 zMin = reader.getInt32(0x18);
    qint32 zMax = reader.getInt32(0x1C);
    quint32 bonesSize = reader.getUint32(0x20);
    quint32 bonesOffset = reader.getUint32(0x24);
    quint32 verticesSize = reader.getUint32(0x28);
    quint32 verticesOffset = reader.getUint32(0x2C);
    quint32 normalsSize = reader.getUint32(0x30);
    quint32 normalsOffset = reader.getUint32(0x34);
    quint32 unk1Size = reader.getUint32(0x38);
    quint32 unk1Offset = reader.getUint32(0x3C);
    quint32 polygonsSize = reader.getUint32(0x40);
    quint32 polygonsOffset = reader.getUint32(0x44);
    quint32 linesSize = reader.getUint32(0x48);
    quint32 linesOffset = reader.getUint32(0x4C);
    quint32 spheresSize = reader.getUint32(0x50);
    quint32 spheresOffset = reader.getUint32(0x54);
    quint32 uvGroupsSize = reader.getUint32(0x58);
    quint32 uvGroupsOffset = reader.getUint32(0x5C);

    qDebug() << "LbaBody::fromLba2Buffer" << bonesSize  << verticesSize << normalsSize << polygonsSize << linesSize << spheresSize;

    reader.seek(bonesOffset);
    loadLba2Bones(reader,bonesSize);

    reader.seek(verticesOffset);
    loadLba2Vertices(reader,verticesSize);

    reader.seek(normalsOffset);
    loadLba2Normals(reader,normalsSize);

    reader.seek(uvGroupsOffset);
    loadLba2UvGroups(reader,uvGroupsSize);

    reader.seek(polygonsOffset);
    loadLba2Polygones(reader,linesOffset - polygonsOffset, pal, uvTexture);

    reader.seek(spheresOffset);
    loadLba2Spheres(reader,spheresSize);

    reader.seek(linesOffset);
    loadLba2Lines(reader,linesSize);

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
    Vertices tv = mVertices;

    // QVector3D matrix(0,0,0);
    QMatrix4x4 matrix;
    translateVertices(-1,matrix, tv);

    mVerticesTranslated = tv;
}

//-------------------------------------------------------------------------------------------
void LbaBody::translateVertices(int parentId, QMatrix4x4 matrix, Vertices &vertices)
{
    Bones nextBones = childsOfBone(parentId);

    for(int i=0; i<nextBones.count(); i++) {

        QMatrix4x4 subMatrix;
        subMatrix.translate( mVertices[nextBones[i].parentVertex].x,
                mVertices[nextBones[i].parentVertex].y,
                mVertices[nextBones[i].parentVertex].z);


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

        QList<int> vertexIndexes = verticesByBone(nextBones[i].id);
        for (int v=0; v < vertexIndexes.count(); v++) {

            QVector4D vec(mVertices[vertexIndexes[v]].x,mVertices[vertexIndexes[v]].y,mVertices[vertexIndexes[v]].z,1);
            vec = boneMatrix * vec;

            vertices[vertexIndexes[v]].x = vec.x();
            vertices[vertexIndexes[v]].y = vec.y();
            vertices[vertexIndexes[v]].z = vec.z();

        }
        translateVertices(nextBones[i].id,boneMatrix, vertices);
    }

}

//-----------------------------------------------------------------------------------------
const LbaBody::Vertices &LbaBody::vertices() const
{
    return mVerticesTranslated.count() > 0 ? mVerticesTranslated : mVertices;
}

//-------------------------------------------------------------------------------------------
const LbaBody::Normals &LbaBody::normals() const
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
QList<int> LbaBody::verticesByBone(int id) const
{
    QList<int> ret;
    for (int i=0; i<mVertices.count(); i++) {
        if (mVertices[i].boneId == id)
            ret << i;
    }

    return ret;
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Points(BinaryReader &reader)
{
    quint16 verticesCount;
    reader.read(&verticesCount, 2);

    mVertices.clear();

    for (int i=0; i<verticesCount; i++) {
        qint16 x,y,z;
        reader.read(&x, 2);
        reader.read(&y, 2);
        reader.read(&z, 2);
        // qDebug() << "Point" << i << x << y << z;
        mVertices << Vertex(x,y,z);
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

        // LBA1-Format
        // b.firstVertex  = firstPoint;
        // b.numVertices  = numPoints;

        // LBA2-Format:
        for (int v=0; v<numPoints; v++)
            mVertices[firstPoint + v].boneId = i;

        b.parentVertex = parentPoint;
        b.rotateX      = x;
        b.rotateY      = y;
        b.rotateZ      = z;

        mBones << b;
        // qDebug() << "BONE" << i << firstPoint << numPoints << parentPoint  << parentBone << boneType << x << y << z << nrNormals;
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
        mNormals << Normal(x,y,z);
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba1Polygones(BinaryReader &reader, const LbaPalette &pal)
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
        p.color = pal.palette()[colorIndex];
        if (renderType >= 9) {             // each vertex has a normal
            for (int j=0; j<nrVertices; j++) {
                quint16 normal, vertex;
                reader.read(&normal,2);
                reader.read(&vertex,2);
                vertex /= 6;
                // qDebug() << "  Vertex" << vertex << normal;
                p.vertices << vertex;
                p.normals  << updateNormal(normal,p.color);;

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
                p.normals  << updateNormal(normal,p.color);;

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

//-----------------------------------------------------------------------------------------
int LbaBody::updateNormal(int normalIndex, QRgb color)
/*
  LBA1: Color by Polygon.
  LBA2: Color by Normal.

  To make it compatible: Map Polygon-Color to Normal-Color. If Normal is already in
  is (with another color), than clone it and set the LBA1-Polygon-Color.
*/
{
    Q_ASSERT(normalIndex >= 0 && normalIndex < mNormals.count());

    if (mNormals[normalIndex].color != color) {
        qWarning() << "Cannot update color of normal" << normalIndex << mNormals[normalIndex].color << color;
        mNormals << mNormals[normalIndex];
        mNormals.last().color = color;
        return mNormals.count() - 1;
    }
    return normalIndex;
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba2Vertices(BinaryReader &reader, quint32 count)
{
    while (count > 0) {
        /*
            x: data.getInt16(index, true) / 0x4000,
            y: data.getInt16(index + 2, true) / 0x4000,
            z: data.getInt16(index + 4, true) / 0x4000,
            bone: data.getUint16(index + 6, true)
        */

        const auto x = reader.readInt16();
        const auto y = reader.readInt16();
        const auto z = reader.readInt16();
        const auto boneIndex = reader.readUint16();
        // qDebug() << "LBA2 VERTEX" << x << y << z << boneIndex;
        Q_ASSERT(boneIndex >= 0);
        Q_ASSERT(boneIndex < mBones.count());
        mVertices << Vertex(x,y,z,boneIndex);
        count--;
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba2Bones(BinaryReader &reader, quint32 count)
{
    for (int i=0; i<count; i++) {
        /*
            parent: rawBones[index],
            vertex: rawBones[index + 1],
            unk1: rawBones[index + 2],
            unk2: rawBones[index + 3]
        */

        const auto parent = reader.readInt16();
        const auto vertex = reader.readInt16();
        const auto unk1   = reader.readInt16();
        const auto unk2   = reader.readInt16();
        // qDebug() << "LBA2 BONE" << i << parent << vertex;
        mBones << Bone(i,parent,0,vertex);
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba2Normals(BinaryReader &reader, quint32 count)
{
    /*
            x: rawNormals[index] / 0x4000,
            y: rawNormals[index + 1] / 0x4000,
            z: rawNormals[index + 2] / 0x4000,
            colour: Math.floor((rawNormals[index + 3] & 0x00FF) / 16)
    */

    while (count > 0) {
        const auto x = reader.readInt16();
        const auto y = reader.readInt16();
        const auto z   = reader.readInt16();
        const auto c   = (reader.readInt16() & 0x00FF)/16;
        qDebug() << "LBA2 Normal" << x << y << z << "Color" << c;
        mNormals << Normal(x,y,z,c);
        count--;
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba2Polygones(BinaryReader &reader, quint32 count, const LbaPalette &pal, const QImage &uvTexture)
{
    /*
         const renderType = data.getUint16(offset, true);
        const numPolygons = data.getUint16(offset + 2, true);
        const sectionSize = data.getUint16(offset + 4, true);
        // const shade = data.getUint16(offset + 6, true);
        offset += 8;

        if (sectionSize === 0)
            break;

        const blockSize = ((sectionSize - 8) / numPolygons);

        for (let j = 0; j < numPolygons; j += 1) {
            const poly = loadPolygon(data, offset, renderType, blockSize);
            object.polygons.push(poly);
            offset += blockSize;
        }
    */

    qint32 offset = reader.pos();
    while ((reader.pos() - offset) < count) {
        const auto  currentPos = reader.pos();
        const auto  renderType  = reader.readUint16();
        const auto  numPolygons = reader.readUint16();
        const auto  sectionSize = reader.readUint16();
        // const shade = data.getUint16(offset + 6, true);
        reader.skip(2);
        if (sectionSize == 0) {
            qWarning() << "LbaBody::loadLba2Polygones" << "Invalid section size";
            break;
        }

        const auto blockSize = ((sectionSize - 8) / numPolygons);
        for (auto j = 0; j < numPolygons; j += 1) {
            const auto p = loadLba2Polygon(reader, renderType, blockSize, pal, uvTexture);
            mPolygones << p;
        }

        Q_ASSERT((reader.pos() - currentPos) == sectionSize);
    }

    validate();
}

//-----------------------------------------------------------------------------------------
LbaBody::Polygon LbaBody::loadLba2Polygon(BinaryReader &reader, quint16 renderType, quint16 blockSize, const LbaPalette &pal, const QImage &uvTexture)
{
    const auto currentPos      = reader.pos();
    const auto numVertex       = (renderType & 0x8000) ? 4 : 3;
    const bool hasExtra        = (renderType & 0x4000);
    const bool hasTex          = ((renderType & 0x8) && (blockSize > 16));
    const bool hasTransparency = (renderType == 2);

    // Blocksizes:
    // Quad and Extra = 16
    // Quad and Tex = 32
    // Quad and Color = 12
    // Tri and Extra = 16
    // Tri and Tex = 24
    // Tri and Color = 12

    Polygon p;
    // vertex block
    for (auto k = 0; k < numVertex; k++) {
        p.vertices <<  reader.readUint16();
        p.normals  << p.vertices.last();
    }

    // special case for trianguled textures
    int textureId = -1;
    if (hasTex && numVertex == 3) {
        textureId = reader.readUint8();
        // qDebug() << "TEXTURE" << tex;
         reader.skip(1);
        // p.tex = tex
    }

    auto color = reader.readUint16();
    color = (color & 0x00FF) / 16;
    // p.lbaColorIndex = color;
    p.color = pal.palette()[color];

    // polygon color intensity
    const auto intensity = reader.readInt16();
    // poly.intensity = intensity;

    if (hasTex) {
        for (auto k = 0; k < numVertex; k++) {
            /*
                poly.unkX[k] = data.getInt8(offset + 12 + (k * 4), true);
                poly.texX[k] = data.getInt8(offset + 13 + (k * 4), true);
                poly.unkY[k] = data.getInt8(offset + 14 + (k * 4), true);
                poly.texY[k] = data.getInt8(offset + 15 + (k * 4), true);
                */
            auto unkX = reader.readInt8();
            auto texX = reader.readInt8();
            auto unkY = reader.readInt8();
            auto texY = reader.readInt8();

            const auto x = (texX + unkX)/256;
            const auto y = (texY + unkY)/256;
            qDebug() << x << y << textureId;
            if (textureId >= 0 && textureId < mUvGroups.count()) {
                // p.color = uvTexture.pixel(mUvGroups[textureId].x + x, mUvGroups[textureId].y+y);
            }
        }
        // for blocksize 32 with quad texture
        if (numVertex == 4) {
            // poly.tex = data.getUint8(offset + 28, true);
            auto tex = reader.readUint8();
        }
    }

    for (int i=0; i<p.normals.count(); i++) {
        int newIndex = updateNormal(p.normals[i], p.color);
        if (p.normals[i] != newIndex)
            p.normals[i] = newIndex;
    }

    while ((reader.pos() - currentPos) < blockSize)
        reader.skip(1);

    Q_ASSERT((reader.pos() - currentPos) == blockSize);
    return p;
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba2Spheres(BinaryReader &reader, quint32 count)
{
    /*
            unk1: rawSpheres[index],
            colour: Math.floor((rawSpheres[index + 1] & 0x00FF) / 16),
            vertex: rawSpheres[index + 2],
            size: rawSpheres[index + 3] / 0x4000
*/

    while (count > 0) {
        const auto unk1   = reader.readUint16();
        const auto color  = (reader.readUint16() & 0x00FF)/16;
        const auto vertex = reader.readUint16();
        const auto size   = reader.readUint16();
        // qDebug() << "LBA2 Sphere" << vertex << size << color;
        mSpheres << Sphere(vertex,size,color);
        count--;
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba2Lines(BinaryReader &reader, quint32 count)
{
    /*
            unk1: rawLines[index],
            colour: Math.floor((rawLines[index + 1] & 0x00FF) / 16),
            vertex1: rawLines[index + 2],
            vertex2: rawLines[index + 3]
*/
    while (count > 0) {
        const auto unk1   = reader.readUint16();
        const auto color  = (reader.readUint16() & 0x00FF)/16;
        const auto vertex1 = reader.readUint16();
        const auto vertex2 = reader.readUint16();
        // qDebug() << "LBA2 Line" << vertex1 << vertex2 << color;
        mLines << Line(vertex1,vertex2);
        count--;
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::loadLba2UvGroups(BinaryReader &reader, quint32 count)
{
    while (count > 0) {
        const auto x   = reader.readUint8();
        const auto y   = reader.readUint8();
        const auto w   = reader.readUint8();
        const auto h   = reader.readUint8();
        qDebug() << "LBA2 UvGroup" << x << y << w << h;
        mUvGroups << UvGroup(x,y,w,h);
        count--;
    }
}

//-----------------------------------------------------------------------------------------
void LbaBody::validate()
{
    for (int i=0; i<mPolygones.count(); i++) {
        for (int vi=0; vi<mPolygones[i].vertices.count(); vi++) {
            if (mPolygones[i].vertices[vi] >= mVertices.count()) {
                mPolygones.removeAt(i);
                qDebug() << "REMOVED" << i;
                i--;
            }
        }
    }
}

