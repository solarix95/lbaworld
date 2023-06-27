#include <QtGlobal>
#include <QCheckBox>
#include <QQuaternion>
#include <QTimer>
#include <libqtr3d/qtr3dcamera.h>
#include <libqtr3d/qtr3dmesh.h>
#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dmodelanimation.h>
#include <libqtr3d/qtr3dmodelanimator.h>
#include <libqtr3d/qtr3dfactory.h>
#include <libqtr3d/qtr3dlightsource.h>
#include <libqtr3d/extras/qtr3dfreecameracontroller.h>
#include <libqtr3d/physics/qtr3dfpsloop.h>

#include "mapviewer.h"
#include <lbabody.h>
#include <lbasprite.h>
#include <lbaanimation.h>
#include <lbwmodelloader.h>
#include <lbwmap.h>

//-------------------------------------------------------------------------------------------------
MapViewer::MapViewer(const LbaRess &ress)
 : mRess(ress)
 , mMap(nullptr)
{
    mUi.setupUi(this);

    connect(mUi.viewer, &Qtr3dWidget::initialized, this, [this]() {

        connect(mUi.spbCamFoV,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this]() {
            mUi.viewer->camera()->setFov(mUi.spbCamFoV->value());
        });

        mUi.viewer->camera()->lookAt({3,3,3},{0,0,0},{0,1,0});

        connect(mUi.viewer->camera(), &Qtr3dCamera::positionChanged, mUi.viewer->primaryLightSource(), &Qtr3dLightSource::setPos);

        new Qtr3dFreeCameraController(mUi.viewer),
        mUi.viewer->setDefaultLighting(Qtr3d::FlatLighting);
        mUi.viewer->primaryLightSource()->setAmbientStrength(0.5);

        setupViewerByUI();

        connect(&mUi.viewer->assets()->loop(),&Qtr3dFpsLoop::stepDone, mUi.viewer, [this]() { mUi.viewer->update(); });
        mUi.viewer->assets()->loop().setFps(25);

        loadMap();
    });
}

//-------------------------------------------------------------------------------------------------
MapViewer::~MapViewer()
{
}

//-------------------------------------------------------------------------------------------------
void MapViewer::loadMap()
{
    // mUi.viewer->assets()->reset();

    mMap = new LbwMap(mUi.viewer->assets());
    mMap->init(QImage(":/map/Rolling Hills Height Map.png"),QImage(":/map/Rolling Hills Bitmap 1025.png"));

    QByteArray modelData =  mRess.data(LbaRess::LBA1,LbaRess::Body,3 /* Twisen + Tunica */);
    if (modelData.isEmpty())
        return;

    LbaPalette pal(mRess.data(LbaRess::LBA1,LbaRess::Ress,0));
    LbaBody body;

    body.fromLba1Buffer(modelData, pal);

    LbaAnimation *ani = NULL;
    ani = new LbaAnimation();
    if (!ani->fromBuffer(mRess.data(LbaRess::LBA1,LbaRess::Anim,1 /* walking */))) {
        qWarning() << "Animation loading failed";
        delete ani;
        ani= nullptr;
    }

    auto *model = mUi.viewer->createModel();
    LbwModelLoader::loadBody(*model,body,pal,ani);
    auto *state = mUi.viewer->createState(model);

    if (state && !model->animations().isEmpty()) {
        state->setAnimator(new Qtr3dModelAnimator(model->animationByName(model->animations().first())));
        state->animator()->setLoop(true);
        state->animator()->setState(true);
        state->animator()->setEnabled(true);
    }
}

//-------------------------------------------------------------------------------------------------
void MapViewer::setupViewerByUI()
{
    if (mUi.btnBlack->isChecked())
        mUi.viewer->assets()->environment().setClearColor(Qt::black);
    if (mUi.btnGrey->isChecked())
        mUi.viewer->assets()->environment().setClearColor(QColor(210,210,210));
    if (mUi.btnWhite->isChecked())
        mUi.viewer->assets()->environment().setClearColor(Qt::white);

    mUi.viewer->camera()->lookAt(mUi.viewer->camera()->pos(),{0.0f,(float)mUi.spbOffset->value(),0.0f},{0,1,0});
}

