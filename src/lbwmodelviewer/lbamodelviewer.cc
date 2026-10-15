#include <QtGlobal>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDebug>
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
#include <lbacharacter.h>
#include <lbwactor.h>
#include <lbwcharacter.h>
#include <lbwcharacterloader.h>
#include <lbwmodelloader.h>

namespace {

int integerArgument(const QString &name, int defaultValue, bool *found = nullptr)
{
    const QString prefix = "--"+name+"=";
    for (const QString &argument: QCoreApplication::arguments()) {
        if (!argument.startsWith(prefix))
            continue;
        bool ok = false;
        const int value = argument.mid(prefix.length()).toInt(&ok);
        if (found)
            *found = ok;
        return ok ? value : defaultValue;
    }
    if (found)
        *found = false;
    return defaultValue;
}

bool hasArgument(const QString &name)
{
    return QCoreApplication::arguments().contains("--"+name);
}

}


//-------------------------------------------------------------------------------------------------
LbaModelViewer::LbaModelViewer(const LbaRess &ress)
 : mRess(ress)
 , mModel(nullptr)
 , mState(nullptr)
 , mCharacter(nullptr)
 , mActor(nullptr)
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
        mUi.viewer->primaryLightSource()->setAmbientStrength(0.8);

        setupViewerByUI();

        connect(&mUi.viewer->assets()->loop(),&Qtr3dFpsLoop::stepDone, mUi.viewer, [this]() { mUi.viewer->update(); });
        mUi.viewer->assets()->loop().setFps(25);

        bool hasEntity = false;
        const int entityIndex = integerArgument("entity", 0, &hasEntity);
        if (hasEntity) {
            const int bodyIndex = integerArgument("logical-body", 0);
            if (loadCharacter(entityIndex, bodyIndex) && hasArgument("animation-demo"))
                startAnimationDemo();
        }
    });
}

//-------------------------------------------------------------------------------------------------
LbaModelViewer::~LbaModelViewer()
{
    clearCharacter();
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
        qWarning() << "Animation loading failed:" << ani->errorString();
        delete ani;
        ani= nullptr;
    }

    // body.setAnimation(ani);
    // body.translateVertices(mUi.spbAniFrame->value());
    // loadBody(body, pal, ani);

    clearCharacter();
    mUi.viewer->assets()->reset();
    mModel = mUi.viewer->createModel();
    LbwModelLoader::loadBody(*mModel,body,pal,ani);
    delete ani;
    ani = nullptr;
    mState = mUi.viewer->createState(mModel);

    if (mState && !mModel->animations().isEmpty()) {
        mState->setAnimator(new Qtr3dModelAnimator(mModel->animationByName(mModel->animations().first())));
        mState->animator()->setLoop(true);
        mState->animator()->setState(mUi.chkPlay->isChecked());
        mState->animator()->setEnabled(mUi.chkAni->isChecked());
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::clearCharacter()
{
    if (mActor) {
        if (mActor->world())
            mActor->world()->removeActor(mActor);
        delete mActor;
        mActor = nullptr;
    }
    delete mCharacter;
    mCharacter = nullptr;
    mState = nullptr;
    mModel = nullptr;
}

//-------------------------------------------------------------------------------------------------
bool LbaModelViewer::loadCharacter(int entityIndex, int logicalBodyIndex)
{
    clearCharacter();
    mUi.viewer->assets()->reset();

    mCharacter = new LbwCharacter(QString("entity-%1").arg(entityIndex));
    mCharacter->setParent(this);

    QString errorString;
    if (!LbwCharacterLoader::load(*mCharacter, *mUi.viewer->assets(), mRess,
                                  LbaRess::LBA1, entityIndex, &errorString)) {
        qCritical() << "Character loading failed:" << errorString;
        delete mCharacter;
        mCharacter = nullptr;
        mUi.viewer->assets()->reset();
        return false;
    }

    const QString costumeName = LbaCharacter::bodyName(logicalBodyIndex);
    mActor = new LbwActor(mCharacter);
    mActor->setParent(this);
    if (!mActor->setCostume(costumeName, "idle")) {
        qCritical() << "Cannot select costume" << costumeName << "with animation idle";
        delete mActor;
        mActor = nullptr;
        delete mCharacter;
        mCharacter = nullptr;
        mUi.viewer->assets()->reset();
        return false;
    }

    mUi.viewer->assets()->space().addActor(mActor);
    mModel = mCharacter->model(costumeName);
    mState = mActor->currentState();
    mUi.chkPlay->setChecked(true);
    mUi.chkAni->setChecked(true);

    qInfo() << "Loaded LBA1 entity" << entityIndex
            << "costume" << costumeName
            << "animations" << mCharacter->animations().join(", ");
    qInfo() << "Character animation:" << mActor->currentAnimation();
    return true;
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::startAnimationDemo()
{
    if (!mActor)
        return;

    LbwActor *demoActor = mActor;
    QTimer::singleShot(1000, demoActor, [demoActor]() {
        if (demoActor->setAnimation("walk"))
            qInfo() << "Character animation:" << demoActor->currentAnimation();
        else
            qWarning() << "Character has no walk animation";
    });
    QTimer::singleShot(3500, demoActor, [demoActor]() {
        if (demoActor->setAnimation("idle"))
            qInfo() << "Character animation:" << demoActor->currentAnimation()
                    << "position:" << demoActor->pos();
        else
            qWarning() << "Character has no idle animation";
    });
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
