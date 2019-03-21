#include "lbamodelviewer.h"
#include <lbabody.h>
#include <lbasprite.h>

//-------------------------------------------------------------------------------------------------
LbaModelViewer::LbaModelViewer(const LbaRess &ress)
 : mRess(ress)
{
    mUi.setupUi(this);
    connect(mUi.spbModel, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));
    connect(mUi.chkAni, SIGNAL(clicked(bool)), this, SLOT(loadModel()));
    connect(mUi.spbAniFrame, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));
    connect(mUi.spbAniIndex, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));
    connect(mUi.spbCamFoV, SIGNAL(valueChanged(int)), mUi.openGLWidget, SLOT(setCamFov(int)));
    connect(mUi.spbCamDist, SIGNAL(valueChanged(double)), mUi.openGLWidget, SLOT(setCamDist(double)));
    connect(mUi.chkPolygon, &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
    connect(mUi.chkLines,   &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
    connect(mUi.chkSpheres, &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
    connect(mUi.chkBones,   &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
    connect(mUi.btnLba1,    &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
    connect(mUi.btnLba2,    &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
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
    int keyFrame = -1;
    if (mUi.chkAni->isChecked()) {
        ani = new LbaAnimation();
        ani->fromBuffer(mRess.data(source,LbaRess::Anim,mUi.spbAniIndex->value()));
        keyFrame = qMin(mUi.spbAniFrame->value(), ani->keyFrameCount()-1);
    }

    int flags = 0;

    // TODO: proper QFlags<>..
    flags += mUi.chkPolygon->isChecked() ? 0x01 : 0;
    flags += mUi.chkLines->isChecked() ?   0x02 : 0;
    flags += mUi.chkSpheres->isChecked() ? 0x04 : 0;
    flags += mUi.chkBones->isChecked() ?   0x08 : 0;

    mUi.openGLWidget->clearGeometryBuffers();
    mUi.openGLWidget->appendGeometryBuffer(body, ani,keyFrame, pal,flags);
}

