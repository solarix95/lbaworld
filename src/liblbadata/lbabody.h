#ifndef LBABODY_H
#define LBABODY_H

#include <QByteArray>
#include <QList>
#include <QVector3D>

class LbaAnimation;
class BinaryReader;
class LbaBody
{
public:

    //-----------------------------------------------------------------------------------------
    // typedef QVector3D Point;
    struct Vertex {
        float x,y,z;
        int   boneId;

        Vertex(float px, float py, float pz, int b = -1) : x(px), y(py), z(pz), boneId(b) {}
        Vertex(const Vertex &o) : x(o.x), y(o.y), z(o.z), boneId(o.boneId)  {}
    };
    typedef QList<Vertex>      Vertices;

    struct Normal {
        float dx,dy,dz;
        int   colorIndex;

        Normal(float x, float y, float z, int c = -1) : dx(x), dy(y), dz(z), colorIndex(c) {}
        Normal(const Normal &o) : dx(o.dx), dy(o.dy), dz(o.dz), colorIndex(o.colorIndex)  {}
    };
    typedef QList<Normal>      Normals;


    struct Line {
        int p0;
        int p1;

        Line(int a, int b) : p0(a), p1(b) {}
    };
    typedef QList<Line> Lines;

    typedef struct {
        float r,g,b;
    } VertexColor;

    struct Polygon {
        QList<int> vertices;
        QList<int> normals;
        int        lbaColorIndex;
        Polygon() : lbaColorIndex(-1) {}
    };
    typedef QList<Polygon> Polygons;

    struct Bone {
        int        id;
        int        parentId;
        int        boneType;

        // int        firstVertex;
        // int        numVertices;
        int        parentVertex;

        float      rotateX;
        float      rotateY;
        float      rotateZ;
        Bone(int i=-1, int p=-1, int t=0, int pv=-1) : id(i), parentId(p), boneType(t), parentVertex(pv), rotateX(0), rotateY(0), rotateZ(0) {}
    //    int
    };
    typedef QList<Bone>    Bones;

    struct Sphere {
        int        centerPoint;
        int        size;
        int        colorIndex;
        Sphere(int p=-1, int s=0, int c=0) : centerPoint(p), size(s), colorIndex(c) {}
    };
    typedef QList<Sphere>    Spheres;

    //-----------------------------------------------------------------------------------------
    LbaBody();
    virtual ~LbaBody();

    bool fromLba1Buffer(const QByteArray &buffer);
    bool fromLba2Buffer(const QByteArray &buffer);
    bool animationFromBuffer(const QByteArray &buffer);
    void setAnimation(LbaAnimation *ani);

    void translateVertices(int keyFrame = -1);
    void translateVertices(int parentId, QMatrix4x4 matrix, Vertices &vertices);

    const Vertices &vertices() const;
    const Normals  &normals() const;
    const Polygons &polygons() const;
    Polygon         polygonByPoint(int pointIndex) const;
    const Bones    &bones() const;
    const Lines    &lines() const;
    const Spheres  &spheres() const;

    Bones  childsOfBone(int parentId) const;
    Bone   boneById(int id) const;
    QList<int> verticesByBone(int id) const;

private:

    void loadLba1Points(BinaryReader &reader);
    void loadLba1Bones(BinaryReader &reader);
    void loadLba1Normals(BinaryReader &reader);
    void loadLba1Polygones(BinaryReader &reader);
    void loadLba1Lines(BinaryReader &reader);
    void loadLba1Spheres(BinaryReader &reader);
    int  updateNormal(int normalIndex, int colorIndex);

    void loadLba2Vertices(BinaryReader &reader, quint32 count);
    void loadLba2Bones(BinaryReader &reader, quint32 count);
    void loadLba2Normals(BinaryReader &reader, quint32 count);
    void loadLba2Polygones(BinaryReader &reader, quint32 count);
    Polygon loadLba2Polygon(BinaryReader &reader, quint16 renderType, quint16 blockSize);
    void loadLba2Spheres(BinaryReader &reader, quint32 count);
    void loadLba2Lines(BinaryReader &reader, quint32 count);

    void validate();

    LbaAnimation *mAnimation;
    int           mAnimationKeyFrame;

    Vertices  mVertices;
    Vertices  mVerticesTranslated;
    Normals   mNormals;
    Polygons  mPolygones;
    Bones     mBones;
    Lines     mLines;
    Spheres   mSpheres;

};

#endif // LBABODY_H
