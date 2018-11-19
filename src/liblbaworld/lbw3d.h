#ifndef LBW3D
#define LBW3D

//-------------------------------------------------------------------------------------------------
#define LbwScalar float

//-------------------------------------------------------------------------------------------------
struct LbwVector {
    LbwScalar x,y,z;
    void normalize();
};

//-------------------------------------------------------------------------------------------------
struct LbwVertex {
    LbwVector p;   // Point
    float  w;      // Unknown.. Weight?
    LbwVector n;   // Normal-Vector
    LbwVector c;   // Color (x -> red, y -> green, z -> blue)
};

//-------------------------------------------------------------------------------------------------
enum LbwShapeType {
    PolygonMesh,
    Sphere,
    Line,
    Bone
};

#endif // LBW3D

