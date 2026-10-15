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
bool LbwMap::init(const LbwMapData &data, QString *error)
{
    if (error)
        error->clear();
    if (!data.isValid()) {
        if (error)
            *error = "Cannot initialize an invalid map description";
        return false;
    }

    destroy();

    mMapMesh = mAssets->createMesh();
    if (!Qtr3d::meshByPlane(*mMapMesh, float(data.size().width()),
                            float(data.size().height()), data.groundColor())) {
        delete mMapMesh;
        mMapMesh = nullptr;
        if (error)
            *error = "Cannot create map ground geometry";
        return false;
    }

    mMapState = mAssets->createState(mMapMesh, Qtr3d::NoLighting);
    mData = data;
    return true;
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
    mMapState = mAssets->createState(mMapMesh, Qtr3d::NoLighting);

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

    if (mMapMesh) {
        // Geometry states are children of their geometry and are deleted with it.
        delete mMapMesh;
        mMapMesh = nullptr;
    }
    mMapState = nullptr;
    mData = LbwMapData();
}

//-------------------------------------------------------------------------------------------------
bool LbwMap::isLoaded() const
{
    return mMapMesh && mMapState;
}

//-------------------------------------------------------------------------------------------------
LbwMapData LbwMap::data() const
{
    return mData;
}

//-------------------------------------------------------------------------------------------------
void LbwMap::placeActor(LbwActor *actor, float x, float z)
{
    placeActor(actor, QVector3D(x, 0.0f, z));
}

//-------------------------------------------------------------------------------------------------
void LbwMap::placeActor(LbwActor *actor, const QVector3D &position)
{
    Q_ASSERT(actor);

    if (!mHighMap && !isLoaded())
        return;

    const float height = mHighMap
            ? mHighMap->yHightAt(position.x(), position.z()) + position.y()
            : position.y();

    actor->setPos(QVector3D(position.x(), height, position.z()));
}
