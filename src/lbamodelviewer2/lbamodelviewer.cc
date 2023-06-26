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

#include "lbamodelviewer.h"
#include <lbabody.h>
#include <lbasprite.h>
#include <lbaanimation.h>
#include <lbwmodelloader.h>


//-------------------------------------------------------------------------------------------------
LbaModelViewer::LbaModelViewer(const LbaRess &ress)
 : mRess(ress)
 , mState(nullptr)
{
    mUi.setupUi(this);

    connect(mUi.viewer, &Qtr3dWidget::initialized, this, [this]() {

        connect(mUi.spbModel, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));
        connect(mUi.spbAniIndex, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));

        connect(mUi.spbCamFoV,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this]() {
            mUi.viewer->camera()->setFov(mUi.spbCamFoV->value());
        });

        connect(mUi.chkPlay,&QCheckBox::clicked, this, [this]() {
            if (mState && mState->animator())
                mState->animator()->setState(mUi.chkPlay->isChecked());
        });
        connect(mUi.chkAni,&QCheckBox::clicked, this, [this]() {
            if (mState && mState->animator())
                mState->animator()->setEnabled(mUi.chkAni->isChecked());
        });

        connect(mUi.btnLba1,    &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
        connect(mUi.btnLba2,    &QCheckBox::clicked, this, &LbaModelViewer::loadModel);

        connect(mUi.btnBlack,   &QRadioButton::clicked, this, &LbaModelViewer::setupViewerByUI);
        connect(mUi.btnGrey,    &QRadioButton::clicked, this, &LbaModelViewer::setupViewerByUI);
        connect(mUi.btnWhite,   &QRadioButton::clicked, this, &LbaModelViewer::setupViewerByUI);
        connect(mUi.spbOffset,  static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &LbaModelViewer::setupViewerByUI);

        mUi.viewer->camera()->lookAt({3,3,3},{0,0,0},{0,1,0});

        connect(mUi.viewer->camera(), &Qtr3dCamera::positionChanged, mUi.viewer->primaryLightSource(), &Qtr3dLightSource::setPos);

        new Qtr3dFreeCameraController(mUi.viewer),
        mUi.viewer->setDefaultLighting(Qtr3d::FlatLighting);
        mUi.viewer->primaryLightSource()->setAmbientStrength(0.5);

        setupViewerByUI();

        connect(&mUi.viewer->assets()->loop(),&Qtr3dFpsLoop::stepDone, mUi.viewer, [this]() { mUi.viewer->update(); });
        mUi.viewer->assets()->loop().setFps(25);
    });
}

//-------------------------------------------------------------------------------------------------
LbaModelViewer::~LbaModelViewer()
{
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadModel()
{
    int modelId = mUi.spbModel->value();

    LbaRess::Source source       = mUi.btnLba1->isChecked() ? LbaRess::LBA1 : LbaRess::LBA2;
    LbaRess::Content bodyContent = mUi.btnBody->isChecked() ? LbaRess::Body : LbaRess::StaticObjs;

    if (modelId >= mRess.count(source,bodyContent))
        return;

    QByteArray modelData =  mRess.data(source,bodyContent,modelId);
    if (modelData.isEmpty())
        return;

    LbaPalette pal(mRess.data(source,LbaRess::Ress,0));
    LbaBody body;
    if (source == LbaRess::LBA1)
        body.fromLba1Buffer(modelData, pal);
    else {
        LbaSprite uvTexture(pal,mRess.data(source,LbaRess::Ress,6),LbaSprite::Image);
        body.fromLba2Buffer(modelData, pal, uvTexture.image());
    }

    LbaAnimation *ani = NULL;
    ani = new LbaAnimation();
    if (!ani->fromBuffer(mRess.data(source,LbaRess::Anim,mUi.spbAniIndex->value()))) {
        qWarning() << "Animation loading failed";
        delete ani;
        ani= nullptr;
    }

    // body.setAnimation(ani);
    // body.translateVertices(mUi.spbAniFrame->value());
    // loadBody(body, pal, ani);

    mUi.viewer->assets()->reset();
    mModel = mUi.viewer->createModel();
    LbwModelLoader::loadBody(*mModel,body,pal,ani);
    mState = mUi.viewer->createState(mModel);

    if (mState && !mModel->animations().isEmpty()) {
        mState->setAnimator(new Qtr3dModelAnimator(mModel->animationByName(mModel->animations().first())));
        mState->animator()->setLoop(true);
        mState->animator()->setState(mUi.chkPlay->isChecked());
        mState->animator()->setEnabled(mUi.chkAni->isChecked());
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::setupViewerByUI()
{
    if (mUi.btnBlack->isChecked())
        mUi.viewer->assets()->environment().setClearColor(Qt::black);
    if (mUi.btnGrey->isChecked())
        mUi.viewer->assets()->environment().setClearColor(QColor(210,210,210));
    if (mUi.btnWhite->isChecked())
        mUi.viewer->assets()->environment().setClearColor(Qt::white);

    mUi.viewer->camera()->lookAt(mUi.viewer->camera()->pos(),{0.0f,(float)mUi.spbOffset->value(),0.0f},{0,1,0});
}

