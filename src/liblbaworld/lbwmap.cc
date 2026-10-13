#include "lbwmap.h"
#include "lbwactor.h"

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

    mHighMap = new Qtr3dHighmap();
    mHighMap->fromImage(highmap,{1,200,1});

    // auto *debug = mAssets->createMesh();
    //Qtr3d::normalMeshByMesh(*debug,*mMapMesh,1);
    //mAssets->createState(debug);
}

//-------------------------------------------------------------------------------------------------
void LbwMap::destroy()
{
    if (mHighMap) {
        delete mHighMap;
        mHighMap = nullptr;
    }

    if (mMapState) {
        delete mMapState;
        mMapState = nullptr;
    }

    if (mMapMesh) {
        delete mMapMesh;
        mMapMesh = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
void LbwMap::placeActor(LbwActor *actor, float x, float z)
{
    if (!mHighMap)
        return;

    Q_ASSERT(actor);

    float height = mHighMap->yHightAt(x,z);

    actor->setPos(QVector3D(x,height,z));
}
