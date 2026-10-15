#ifndef LBWMAP_H
#define LBWMAP_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QVector3D>

#include "lbwmapdata.h"

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

    bool init(const LbwMapData &data, QString *error = 0);
    void init(const QImage &highmap, const QImage &mapTexture);
    void destroy();

    bool       isLoaded() const;
    LbwMapData data() const;
    void placeActor(LbwActor *actor, float x, float z);
    void placeActor(LbwActor *actor, const QVector3D &position);

private:
    Qtr3dAssets  *mAssets;

    // Map
    Qtr3dHighmap       *mHighMap;
    Qtr3dMesh          *mMapMesh;
    Qtr3dGeometryState *mMapState;
    LbwMapData          mData;

};

using LbwMaps = QList<LbwMap*>;

#endif // LBWMAP_H
