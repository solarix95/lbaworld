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

//-------------------------------------------------------------------------------------------------
inline void initVertex(Qtr3dColoredVertex &v, int vi, const LbaBody::Vertices &points, const LbaBody::Normals  &normals, const LbaBody::Polygon &polygon)
{
    float f = 800;
    v.p.x = points[polygon.vertices[vi]].x/f;
    v.p.y = points[polygon.vertices[vi]].y/f; \
    v.p.z = points[polygon.vertices[vi]].z/f; \
                                 \
    v.n.x = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dx : 0;
    v.n.y = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dy : 0;
    v.n.z = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dz : 0;
    v.n.normalize();

    v.bi[0] = points[polygon.vertices[vi]].boneId;
    v.bi[1] = -1;
    v.bi[2] = -1;

    v.bw[0] = 1.0;
    v.bw[1] = 0.0;
    v.bw[2] = 0.0;

    {
        QRgb color = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].color : polygon.color;
        v.c.x = qRed(  color)/255.0;
        v.c.y = qGreen(color)/255.0;
        v.c.z = qBlue( color)/255.0;
    }
}

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
        QTimer *t = new QTimer();
        connect(t, &QTimer::timeout, mUi.viewer, [this]() { mUi.viewer->update();});
        t->start(1000/25);

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

    loadBody(body, pal, ani);

    if (mState && !mModel->animations().isEmpty()) {
        mState->setAnimator(new Qtr3dModelAnimator(mModel->animationByName(mModel->animations().first())));
        mState->animator()->setLoop(true);
        mState->animator()->setState(mUi.chkPlay->isChecked());
        mState->animator()->setEnabled(mUi.chkAni->isChecked());
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBody(LbaBody &body, const LbaPalette &pal, LbaAnimation *animation)
{
    mUi.viewer->assets()->reset();

    mModel = mUi.viewer->createModel();

    // body.translateVertices();
    loadBodyMeshes(*mModel,body, pal); // LBA1 Skin + Qtr3d-Nodes

    loadBodySpheres(*mModel,body, pal);
    // loadBodyLines(*model,body, pal);

    loadBodyAnimation(*mModel,body,animation);

    mState = mUi.viewer->createState(mModel);
    // mUi.viewer->camera()->lookAt(QVector3D(3*model->radius(),2*model->radius(),3*model->radius()),model->center(),{0,1,0});
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBodyMeshes(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal)
{
    const auto &polygons = body.polygons();
    const auto &vertices = body.vertices();
    const auto &normals  = body.normals();

    // Schritt1: wiviele Triangles haben wir?
    int count = 0;
    for (int i=0; i<polygons.count(); i++) {
        if (polygons[i].vertices.count() == 3)
            count++;
        if (polygons[i].vertices.count() >= 4)
            count += polygons[i].vertices.count();
    }

    if (!count)
        return;

    auto *mesh = mUi.viewer->createMesh();
    mesh->startMesh(Qtr3d::Triangle, Qtr3d::CounterClockWise);

    int vindex = 0;

    for (int i=0; i<polygons.count(); i++) {
        float f = 800;
        if (polygons[i].vertices.count() >= 3) {

            for (int triangleIndex = 0; triangleIndex <= polygons[i].vertices.count()-3; triangleIndex++)
            {
                Qtr3dColoredVertex v;
                initVertex(v,0, vertices, normals, polygons[i]);

                if (v.n.isNull()) {
                    qDebug() << "Argh?";
                }
                vindex++;
                mesh->addVertex(v);

                initVertex(v,1+triangleIndex, vertices, normals, polygons[i]);
                vindex++;
                if (v.n.isNull())
                    qDebug() << "Argh?";
                mesh->addVertex(v);

                initVertex(v,2+triangleIndex, vertices, normals, polygons[i]);
                if (v.n.isNull())
                    qDebug() << "Argh?";
                vindex++;
                mesh->addVertex(v);
            }
        }


        if (0 && (polygons[i].vertices.count() >= 4)) {

            // Calculate center vertex

            float cx = 0;
            float cy = 0;
            float cz = 0;
            for (int p=0; p<polygons[i].vertices.count(); p++) {
                cx += vertices[polygons[i].vertices[p]].x;
                cy += vertices[polygons[i].vertices[p]].y;
                cz += vertices[polygons[i].vertices[p]].z;
            }
            cx /= polygons[i].vertices.count();
            cy /= polygons[i].vertices.count();
            cz /= polygons[i].vertices.count();

            // Normal Calculation: from center to polygon vertex 0 + 1:
            QVector3D c0(cx - vertices[polygons[i].vertices[0]].x,cy - vertices[polygons[i].vertices[0]].y,cz - vertices[polygons[i].vertices[0]].z);
            QVector3D c1(cx - vertices[polygons[i].vertices[1]].x,cy - vertices[polygons[i].vertices[1]].y,cz - vertices[polygons[i].vertices[1]].z);
            QVector3D cn = QVector3D::crossProduct(c1,c0);

            // Init Verteces
            for (int p=0; p<polygons[i].vertices.count(); p++) {
                int sibling = p < (polygons[i].vertices.count()-1) ? (p + 1) : 0; // sibling of the last is the first

                Qtr3dColoredVertex v;
                initVertex(v,p, vertices, normals, polygons[i]);
                int bone0 = v.bi[0];
                vindex++;
                mesh->addVertex(v);

                initVertex(v,sibling, vertices, normals, polygons[i]);
                int bone1 = v.bi[0];
                vindex++;
                mesh->addVertex(v);

                v.p.x = cx/f;
                v.p.y = cy/f;
                v.p.z = cz/f;

                v.n.x = cn.x();
                v.n.y = cn.y();
                v.n.z = cn.z();
                v.n.normalize();

                // v.c.x = // 3*qRed(  polygons[i].color)/255.0;
                // v.c.y = // 3*qGreen(polygons[i].color)/255.0;
                // v.c.z = // 3*qBlue( polygons[i].color)/255.0;

                v.bi[0] = bone0; // mesh->vertex(mesh->vertexCount()-1).bi[0]; // vertices[polygons[i].vertices[0]].boneId;
                v.bi[1] = -1;
                v.bi[2] = -1;

                v.bw[0] = 1.0;
                v.bw[1] = 0;
                v.bw[2] = 0;

                if (bone1 != bone0) {
                       v.bi[1] = bone1;
                       v.bw[0] = 0.5;
                       v.bw[1] = 0.5;
                }
                mesh->addVertex(v);

                vindex++;
            }
        }
    }

    // count *= 3;
    // Q_ASSERT(vindex == count);

    mesh->endMesh();
    model.addMesh(mesh,false);
    loadModelNodes(model,mesh,body,-1,nullptr);
    loadMeshBones(mesh,body);
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBodySpheres(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal)
{
    const auto &spheres = body.spheres();
    auto translatedBones    = body.boneAnimation();
    auto translatedVertices = body.translatedVertices(0);

    for (const auto &sphere: spheres) {

        const auto &sphereCenter = translatedVertices[sphere.centerPoint];
        int sphereBoneId  = sphereCenter.boneId;
        int lba1BoneIndex = body.boneIndexById(sphereBoneId);
        if (lba1BoneIndex < 0)
            continue;

        QMatrix4x4 boneTranslation = translatedBones[lba1BoneIndex];
        QVector3D boneStartPos(0,0,0);
        boneStartPos = boneTranslation * boneStartPos;

        QVector3D boneToCenterDistance = (sphereCenter.toVector() - boneStartPos)/800.0;


        const auto *sphereNode = model.nodes().nodeByName(QString::number(sphereBoneId));
        if (!sphereNode) {
            qWarning() << "LbaModelViewer::loadBodySpheres: invalid model sphere";
            continue;
        }

        auto *mesh = mUi.viewer->createMesh();
        QMatrix4x4 transform;
        transform.translate(boneToCenterDistance);
        transform.scale(sphere.size/800.0);

        Qtr3d::appendSphere2Mesh(*mesh, 4, pal.palette()[sphere.colorIndex], transform);

        for (int vi=0; vi<mesh->verticesCount(); vi++) {
            mesh->updateBone(vi,Qtr3dVector(0 /*sphereBoneId*/,-1,-1),Qtr3dVector(1,0,0));
        }

        Qtr3dMesh::Bone bone;
        bone.name = QString::number(sphereBoneId);

        for (int vi=0; vi<mesh->verticesCount(); vi++) {
            bone.weights << Qtr3dMesh::BoneWeight(vi,1.0);
        }

        mesh->addBone(bone);


        // Step 1: find out how many bones gets adressed by the vertices
        /*
        int maxId = -1;
        for (const auto &v: body.vertices())
            if (v.boneId > maxId)
                maxId = v.boneId;

        if (maxId < 0)
            return; // WTF?

        for (int bi=0; bi<=maxId; bi++) {
            Qtr3dMesh::Bone bone;
            bone.name = QString::number(bi);
            if (bi == sphereBoneId) {
                for (int vi=0; vi<mesh->vertexCount(); vi++) {
                    bone.weights << Qtr3dMesh::BoneWeight(vi,1.0);
                }
            }
            bone.offset.setToIdentity();
            mesh->addBone(bone);
        }
        */

        mesh->endMesh();
        model.addMesh(mesh,sphereNode->mName);
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBodyLines(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal)
{
    const auto  &points   = body.vertices();
    const auto  &lines    = body.lines();
    int count = lines.count();

    if (count <= 0)
        return;

    int vindex = 0;

    auto *mesh = mUi.viewer->createMesh();
    mesh->startMesh(Qtr3d::Line);

    for (int i=0; i<lines.count(); i++) {
        float f = 800;
        QRgb colorP0 = body.polygonByPoint(lines[i].p0).color;

        Qtr3dColoredVertex v;
        v.p.x = points[lines[i].p0].x/f;
        v.p.y = points[lines[i].p0].y/f;
        v.p.z = points[lines[i].p0].z/f;

        v.c.x = qRed(colorP0)/255.0;
        v.c.y = qGreen(colorP0)/255.0;
        v.c.z = qBlue(colorP0)/255.0;
        mesh->addVertex(v);
        vindex++;

        QRgb colorP1 = body.polygonByPoint(lines[i].p1).color;

        v.p.x = points[lines[i].p1].x/f;
        v.p.y = points[lines[i].p1].y/f;
        v.p.z = points[lines[i].p1].z/f;

        v.c.x = qRed(colorP1)/255.0;
        v.c.y = qGreen(colorP1)/255.0;
        v.c.z = qBlue(colorP1)/255.0;
        mesh->addVertex(v);
        vindex++;
    }

    count *= 2;
    Q_ASSERT(vindex == count);
    mesh->endMesh();
    model.addMesh(mesh,true);
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadModelNodes(Qtr3dModel &model, Qtr3dMesh *mesh, const LbaBody &body, int lbaParentId, Qtr3dModel::Node *parentNode)
{
    auto nextBones = body.childsOfBone(lbaParentId, -1);

    for(int i=0; i<nextBones.count(); i++) {

        QMatrix4x4 subMatrix;
        subMatrix.translate(
                body.vertices()[nextBones[i].parentVertex].x/800.0,
                body.vertices()[nextBones[i].parentVertex].y/800.0,
                body.vertices()[nextBones[i].parentVertex].z/800.0);

        QMatrix4x4 subMatrixAni = LbaBody::transformByBone(nextBones[i].boneType,
                                                           nextBones[i].rotateX,
                                                           nextBones[i].rotateY,
                                                           nextBones[i].rotateZ);

        QMatrix4x4 nodeMatrix = /*parentMatrix **/ subMatrix * subMatrixAni;

        auto *nextParent = model.createNode(mesh,nodeMatrix,QString::number(nextBones[i].id),parentNode);
        loadModelNodes(model,nullptr,body,nextBones[i].id,nextParent);
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadMeshBones(Qtr3dMesh *mesh, const LbaBody &body)
{
    if (body.bones().count() <= 0) {
        qWarning() << "Invalid model. Cant load bones";
        return;
    }

    Q_ASSERT(mesh);

    const auto &vertices = body.vertices();

    // Step 1: find out how many bones gets adressed by the vertices
    int maxId = -1;
    for (const auto &v: vertices)
        if (v.boneId > maxId)
            maxId = v.boneId;

    if (maxId < 0)
        return; // WTF?

    for (int bi=0; bi<=maxId; bi++) {
        Qtr3dMesh::Bone bone;
        bone.name = QString::number(bi);
        for (int vi=0; vi<vertices.count(); vi++) {
            if (vertices[vi].boneId == bi)
                bone.weights << Qtr3dMesh::BoneWeight(vi,1.0);
        }

        bone.offset.setToIdentity();

        mesh->addBone(bone);
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBodyAnimation(Qtr3dModel &model, const LbaBody &body, LbaAnimation *animation)
{
    if (model.meshes().count() <= 0)
        return;

    if (!animation)
        return;

    if (animation->keyFrameCount() <= 0 || animation->bones(0).count() <= 0)
        return;

    // Create skeletal for the primary LBA-Mesh ("Skin")
    Qtr3dModelAnimation *anim = new Qtr3dModelAnimation("demo",3000,1000);

    /*
        LBA  has synchronous keyframes:
           - keyframe n [ node 0, node 1, node n ]

        QT3D has ASSIMP-Channels
           - node n [ keyframe 0, keyframe 1, keyframe n ]

        ... so we have to "invert" the datastructure here
    */

    for (int i=0; i<animation->bones(0).count(); i++) {

        Qtr3dModelAnimation::Channel ch;
        ch.nodeName = QString::number(i);

        // first loop: create rotation keys:
        for (int k=0; k<animation->keyFrameCount(); k++) {
            float t = k * 3000/animation->keyFrameCount();
            const auto &lbaBone = animation->bones(k).at(i);
            if (lbaBone.boneType == 0) { // Rotation

                Qtr3dModelAnimation::RotationKey rotation(t, QQuaternion::fromEulerAngles(
                                                              lbaBone.rotateX,
                                                              lbaBone.rotateY,
                                                              lbaBone.rotateZ));
                ch.mRotationKeys << rotation;
            } else {
                Qtr3dModelAnimation::PositionKey position(t, QVector3D(lbaBone.rotateX/800.0,lbaBone.rotateY/800.0,lbaBone.rotateZ/800.0));
            }
        }

        anim->addChannel(ch);
    }
    model.addAnimation(anim);

    const auto &bones    = body.bones();
    if (bones.count() != animation->bones(0).count()) {
        qWarning() << "Animation loading: invalid bone count: " << bones.count() << "/" << animation->bones(0).count();
        return;
    }

}

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

