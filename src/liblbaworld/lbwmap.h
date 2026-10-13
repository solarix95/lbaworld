#ifndef LBWMAP_H
#define LBWMAP_H

#include <QObject>
#include <QImage>
#include <QList>

class Qtr3dAssets;
class Qtr3dMesh;
class Qtr3dGeometryState;
class Qtr3dHighmap;
class LbwActor;

class LbwMap : public QObject
{
public:
    LbwMap(Qtr3dAssets *assets);
    virtual ~LbwMap();

    void init(const QImage &highmap, const QImage &mapTexture);
    void destroy();

    void placeActor(LbwActor *actor, float x, float z);

private:
    Qtr3dAssets  *mAssets;

    // Map
    Qtr3dHighmap       *mHighMap;
    Qtr3dMesh          *mMapMesh;
    Qtr3dGeometryState *mMapState;

};

using LbwMaps = QList<LbwMap*>;

#endif // LBWMAP_H
