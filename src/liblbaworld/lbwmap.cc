#include "lbwmap.h"

#include <libqtr3d/qtr3dmesh.h>
#include <libqtr3d/qtr3dgeometrystate.h>
#include <libqtr3d/qtr3dfactory.h>
#include <libqtr3d/physics/qtr3dhighmap.h>

//-------------------------------------------------------------------------------------------------
LbwMap::LbwMap(Qtr3dAssets *assets)
 : QObject()
 , mAssets(assets)
 , mHighMap(nullptr)
 , mMapMesh(nullptr)
 , mMapState(nullptr)
 , mPlayer(nullptr)
{
    Q_ASSERT(mAssets);
}

//-------------------------------------------------------------------------------------------------
LbwMap::~LbwMap()
{
    destroy();
}

//-------------------------------------------------------------------------------------------------
void LbwMap::init(const QImage &highmap, const QImage &mapTexture)
{
    destroy();

    if (highmap.isNull())
        return;
    if (mapTexture.isNull())
        return;

    mMapMesh = mAssets->createMesh();
    Qtr3d::meshByHighmap(*mMapMesh,highmap,mapTexture, {1,200,1});
    mAssets->createState(mMapMesh, Qtr3d::NoLighting);
}

//-------------------------------------------------------------------------------------------------
void LbwMap::destroy()
{
    if (mHighMap)
        delete mHighMap;

    if (mMapState)
        delete mMapState;

    if (mMapMesh)
        delete mMapMesh;

    if (mPlayer)
        delete mPlayer;
}
