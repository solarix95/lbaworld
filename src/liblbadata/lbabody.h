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
    struct Point {
        float x,y,z;

        Point(float px, float py, float pz) : x(px), y(py), z(pz) {}
        Point(const Point &o) : x(o.x), y(o.y), z(o.z) {}
    };
    typedef QList<Point> Points;

    struct Line {
        int p0;
        int p1;

        Line(int a, int b) : p0(a), p1(b) {}
    };
    typedef QList<Line> Lines;

    struct Vertex {
        int vertexIndex;
        int normalIndex;

        Vertex(int vi, int ni) :  vertexIndex(vi), normalIndex(ni) {}
        Vertex(const Vertex &o) : vertexIndex(o.vertexIndex), normalIndex(o.normalIndex) {}
    };
    typedef QList<Vertex> Vertices;

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

        int        firstVertex;
        int        numVertices;
        int        parentVertex;

        float      rotateX;
        float      rotateY;
        float      rotateZ;
        Bone() : id(-1) {}
    //    int
    };
    typedef QList<Bone>    Bones;

    typedef struct {
        int        centerPoint;
        int        size;
        int        colorIndex;
       // Sphere(int p, int s, int c) : centerPoint(p), size(s), colorIndex(c) {}
    } Sphere;
    typedef QList<Sphere>    Spheres;


    //-----------------------------------------------------------------------------------------
    LbaBody();
    virtual ~LbaBody();

    bool fromLba1Buffer(const QByteArray &buffer);
    bool animationFromBuffer(const QByteArray &buffer);
    void setAnimation(LbaAnimation *ani);

    void translateVertices(int keyFrame = -1);
    void translateVertices(int parentId, QMatrix4x4 matrix, Points &vertices);

    const Points   &points() const;
    const Points   &normals() const;
    const Polygons &polygons() const;
    Polygon         polygonByPoint(int pointIndex) const;
    const Bones    &bones() const;
    const Lines    &lines() const;
    const Spheres  &spheres() const;

    Bones  childsOfBone(int parentId) const;
    Bone   boneById(int id) const;

private:

    void loadLba1Points(BinaryReader &reader);
    void loadLba1Bones(BinaryReader &reader);
    void loadLba1Normals(BinaryReader &reader);
    void loadLba1Polygones(BinaryReader &reader);
    void loadLba1Lines(BinaryReader &reader);
    void loadLba1Spheres(BinaryReader &reader);

    LbaAnimation *mAnimation;
    int           mAnimationKeyFrame;

    Points   mPoints;
    Points   mPointsTranslated;
    Points   mNormals;
    Polygons mPolygones;
    Bones    mBones;
    Lines    mLines;
    Spheres  mSpheres;

};

#endif // LBABODY_H
