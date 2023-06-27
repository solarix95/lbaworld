#ifndef LBWMAP_H
#define LBWMAP_H

#include <QObject>
#include <QImage>

class Qtr3dAssets;
class Qtr3dMesh;
class Qtr3dGeometryState;
class Qtr3dHighmap;
class LbwPlayer;

class LbwMap : public QObject
{
public:
    LbwMap(Qtr3dAssets *assets);
    virtual ~LbwMap();

    void init(const QImage &highmap, const QImage &mapTexture);
    void destroy();

private:
    Qtr3dAssets  *mAssets;

    // Map
    Qtr3dHighmap       *mHighMap;
    Qtr3dMesh          *mMapMesh;
    Qtr3dGeometryState *mMapState;


    // Player/Character
    LbwPlayer    *mPlayer;
};

#endif // LBWMAP_H
