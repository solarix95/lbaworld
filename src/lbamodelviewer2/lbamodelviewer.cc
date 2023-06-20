#include <QtGlobal>
#include <QCheckBox>
#include <libqtr3d/qtr3dcamera.h>
#include <libqtr3d/qtr3dmesh.h>
#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dmodelanimation.h>
#include <libqtr3d/qtr3dmodelanimator.h>
#include <libqtr3d/qtr3dfactory.h>
#include <libqtr3d/qtr3dlightsource.h>
#include <libqtr3d/extras/qtr3dfreecameracontroller.h>

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
{
    mUi.setupUi(this);

    connect(mUi.viewer, &Qtr3dWidget::initialized, this, [this]() {
        connect(mUi.spbModel, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));
        connect(mUi.chkAni, SIGNAL(clicked(bool)), this, SLOT(loadModel()));
        connect(mUi.spbAniFrame, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));
        connect(mUi.spbAniIndex, SIGNAL(valueChanged(int)), this, SLOT(loadModel()));

        connect(mUi.spbCamFoV,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this]() {
            mUi.viewer->camera()->setFov(mUi.spbCamFoV->value());
        });

        connect(mUi.chkPolygon, &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
        connect(mUi.chkLines,   &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
        connect(mUi.chkSpheres, &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
        connect(mUi.chkBones,   &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
        connect(mUi.btnLba1,    &QCheckBox::clicked, this, &LbaModelViewer::loadModel);
        connect(mUi.btnLba2,    &QCheckBox::clicked, this, &LbaModelViewer::loadModel);

         connect(new Qtr3dFreeCameraController(mUi.viewer), &Qtr3dFreeCameraController::positionChanged, mUi.viewer->primaryLightSource(), &Qtr3dLightSource::setPos);
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
    if (mUi.chkAni->isChecked()) {
        ani = new LbaAnimation();
        if (!ani->fromBuffer(mRess.data(source,LbaRess::Anim,mUi.spbAniIndex->value()))) {
            qWarning() << "Animation loading failed";
            delete ani;
            return;
        }
    }

    // body.setAnimation(ani);
    // body.translateVertices(mUi.spbAniFrame->value());


    loadBody(body, pal, ani);

    return;



    int flags = 0;

    // TODO: proper QFlags<>..
    flags += mUi.chkPolygon->isChecked() ? 0x01 : 0;
    flags += mUi.chkLines->isChecked() ?   0x02 : 0;
    flags += mUi.chkSpheres->isChecked() ? 0x04 : 0;
    flags += mUi.chkBones->isChecked() ?   0x08 : 0;


    // mUi.openGLWidget->clearGeometryBuffers();
    // mUi.openGLWidget->appendGeometryBuffer(body, ani,keyFrame, pal,flags);
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBody(LbaBody &body, const LbaPalette &pal, LbaAnimation *animation)
{
    mUi.viewer->assets()->reset();

    auto *model = mUi.viewer->createModel();

    // body.translateVertices();
    loadBodyMeshes(*model,body, pal); // LBA1 Skin + Qtr3d-Nodes

    // loadBodySpheres(*model,body, pal);
    // loadBodyLines(*model,body, pal);
    loadBodyBones(*model,body);
    loadBodyAnimation(*model,body,animation);

    auto *state = mUi.viewer->createState(model);

    if (!model->animations().isEmpty())
        state->setAnimator(new Qtr3dModelAnimator(model->animationByName(model->animations().first())));

    mUi.viewer->camera()->lookAt(QVector3D(3*model->radius(),2*model->radius(),3*model->radius()),model->center(),{0,1,0});

    auto debugBones     = body.boneAnimation();
    auto debugVertices1 = body.vertices();
    auto debugVertices2 = body.translatedVertices();
    body.translateVertices();
    auto debugVertices3 = body.vertices();

    qDebug() << (debugVertices1 == debugVertices2);
    qDebug() << (debugVertices2 == debugVertices3);

    qDebug() << debugVertices1.count() << debugVertices2.count() << debugBones.count();

    for (auto &v : debugVertices1) {
        QVector4D v4(v.x,v.y,v.z,1.0);
        v4 = debugBones[v.boneId] * v4;
        v.x = v4.x();
        v.y = v4.y();
        v.z = v4.z();
    }

    qDebug() << (debugVertices1 == debugVertices2);

    // model->meshes().first()
    QVector<QVector3D> translated;
    for (const auto &n: model->nodes().mNodes) {
        for (const auto *m: n->mMeshes) {
            QVector<QMatrix4x4> skeleton;
            Qtr3dModel::setupSkeleton(skeleton,n->rootNode(),m,nullptr,QMatrix4x4(),QMatrix4x4());

            for (int vi=0; vi<10; vi++) {
                qDebug() << vi << m->vertex(vi).p.toQVector() << skeleton[int(m->vertex(vi).bi[0])] * m->vertex(vi).p.toQVector();
            }
        }
    }
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
                vindex++;
                mesh->addVertex(v);

                initVertex(v,1+triangleIndex, vertices, normals, polygons[i]);
                vindex++;
                mesh->addVertex(v);

                initVertex(v,2+triangleIndex, vertices, normals, polygons[i]);
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
            QVector3D cn = QVector3D::crossProduct(c0,c1);

            // Init Verteces
            for (int p=0; p<polygons[i].vertices.count(); p++) {
                int sibling = p < (polygons[i].vertices.count()-1) ? (p + 1) : 0; // sibling of the last is the first

                Qtr3dColoredVertex v;
                initVertex(v,p, vertices, normals, polygons[i]);
                int bone0 = v.bi[0];
                vindex++;
                mesh->addVertex(v);
                if (mesh->vertexCount() == 387)
                    qDebug() << "BLUB";


                initVertex(v,sibling, vertices, normals, polygons[i]);
                int bone1 = v.bi[0];
                vindex++;
                mesh->addVertex(v);
                if (mesh->vertexCount() == 387)
                    qDebug() << "BLUB";

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
                if (mesh->vertexCount() == 402)
                    qDebug() << "BLUB";
                vindex++;
            }
        }
    }

    // count *= 3;
    // Q_ASSERT(vindex == count);

    mesh->endMesh();
    model.addMesh(mesh,false);
    loadModelNodes(model,mesh,body,-1,nullptr);
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBodySpheres(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal)
{
    const auto &spheres = body.spheres();

    for (const auto &sphere: spheres) {
        auto *mesh = mUi.viewer->createMesh();
        Qtr3d::meshBySphere(*mesh, 15, pal.palette()[sphere.colorIndex]);

        QMatrix4x4 transform;
        transform.translate(body.vertices()[sphere.centerPoint].toVector()/800.0);
        transform.scale(sphere.size/800.0);
        model.createNode(mesh,transform,"");
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
    auto nextBones = body.childsOfBone(lbaParentId);

    for(int i=0; i<nextBones.count(); i++) {

        QMatrix4x4 subMatrix;
        subMatrix.translate(
                body.vertices()[nextBones[i].parentVertex].x/800.0,
                body.vertices()[nextBones[i].parentVertex].y/800.0,
                body.vertices()[nextBones[i].parentVertex].z/800.0);


        QMatrix4x4 subMatrixAni;
        if (nextBones[i].boneType == 0) {

            if (nextBones[i].rotateZ)
                subMatrixAni.rotate(nextBones[i].rotateZ,0,0,1);
            if (nextBones[i].rotateY)
                subMatrixAni.rotate(nextBones[i].rotateY,0,1,0);
            if (nextBones[i].rotateX)
                subMatrixAni.rotate(nextBones[i].rotateX,1,0,0);

        } else {
            subMatrixAni.translate( nextBones[i].rotateX/800.0,
                                    nextBones[i].rotateY/800.0,
                                    nextBones[i].rotateZ/800.0);
        }

        QMatrix4x4 nodeMatrix = /*parentMatrix **/ subMatrix * subMatrixAni;

        auto *nextParent = model.createNode(mesh,nodeMatrix,QString::number(nextBones[i].id),parentNode);
        loadModelNodes(model,nullptr,body,nextBones[i].id,nextParent);
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBodyBones(Qtr3dModel &model, const LbaBody &body)
{
    if (model.meshes().count() <= 0 || body.bones().count() <= 0) {
        qWarning() << "Invalid model. Cant load bones";
        return;
    }

    // Create skeletal for the primary LBA-Mesh ("Skin")
    auto *mesh = model.meshes().first();
    Q_ASSERT(mesh);

    const auto &vertices = body.vertices();

    // Step 1: find out how many bones gets adressed by the vertices
    int maxId = -1;
    for (const auto &v: vertices)
        if (v.boneId > maxId)
            maxId = v.boneId;

    if (maxId < 0)
        return; // WTF?

    const auto &bones    = body.bones();
    for (int bi=0; bi<=maxId; bi++) {
        Qtr3dMesh::Bone bone;
        bone.name = QString::number(bi);
        for (int vi=0; vi<vertices.count(); vi++) {
            if (vertices[vi].boneId == bi)
                bone.weights << Qtr3dMesh::BoneWeight(vi,1.0);
        }

        // Offset-Calculation:
        // Find LBA1-Bone:
        /*
        int lbaVertexId = -1;
        for (const auto &b: bones) {
            if (b.id == bi) {
                lbaVertexId = b.parentVertex;
            }
        }
        if (lbaVertexId)
            bone.offset.translate(vertices[lbaVertexId].toVector());
        else */
            bone.offset.setToIdentity();

        mesh->addBone(bone);
    }
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBodyAnimation(Qtr3dModel &model, const LbaBody &body, LbaAnimation *animation)
{
    if (!animation)
        return;

    if (animation->keyFrameCount() <= 0)
        return;

    qDebug() << "Loading LBA1 Animation";
    if (model.meshes().count() <= 0 || body.bones().count() <= 0) {
        qWarning() << "Invalid model. Cant load Animation";
        return;
    }

    // Create skeletal for the primary LBA-Mesh ("Skin")
    Qtr3dModelAnimation *anim = new Qtr3dModelAnimation("demo",1000,1000);

    model.addAnimation(anim);

    auto *mesh = model.meshes().first();
    Q_ASSERT(mesh);

    const auto &bones    = body.bones();

    if (bones.count() != animation->bones(0).count()) {
        qWarning() << "Animation loading: invalid bone count: " << bones.count() << "/" << animation->bones(0).count();
        return;
    }

    qInfo() << "Setup Model Animation";
}

