#include <QtGlobal>
#include <libqtr3d/qtr3dcamera.h>
#include <libqtr3d/qtr3dmesh.h>
#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dfactory.h>

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
    v.w   = 1.0;                                   \
    v.n.x = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dx : 0;
    v.n.y = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dy : 0;
    v.n.z = polygon.normals.count() > 0 ? normals[polygon.normals[vi]].dz : 0;
    v.n.normalize();

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

    loadBody(body, pal);

    return;

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


    // mUi.openGLWidget->clearGeometryBuffers();
    // mUi.openGLWidget->appendGeometryBuffer(body, ani,keyFrame, pal,flags);
}

//-------------------------------------------------------------------------------------------------
void LbaModelViewer::loadBody(LbaBody &body, const LbaPalette &pal)
{
    body.translateVertices(0);
    mUi.viewer->assets()->reset();

    auto *model = mUi.viewer->createModel();
    loadBodyMeshes(*model,body, pal);
    loadBodySpheres(*model,body, pal);
    loadBodyLines(*model,body, pal);

    mUi.viewer->createState(model);
    mUi.viewer->camera()->lookAt(QVector3D(3*model->radius(),2*model->radius(),3*model->radius()),model->center(),{0,1,0});
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
        if (polygons[i].vertices.count() == 3) {

            Qtr3dColoredVertex v;
            initVertex(v,0, vertices, normals, polygons[i]);
            vindex++;
            mesh->addVertex(v);

            initVertex(v,1, vertices, normals, polygons[i]);
            vindex++;
            mesh->addVertex(v);

            initVertex(v,2, vertices, normals, polygons[i]);
            vindex++;
            mesh->addVertex(v);
        }

        if (polygons[i].vertices.count() >= 4) {

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
                vindex++;
                mesh->addVertex(v);

                initVertex(v,sibling, vertices, normals, polygons[i]);
                vindex++;
                mesh->addVertex(v);

                v.p.x = cx/f;
                v.p.y = cy/f;
                v.p.z = cz/f;
                v.w   = 1.0;
                v.n.x = cn.x();
                v.n.y = cn.y();
                v.n.z = cn.z();
                v.n.normalize();

                v.c.x = 3*qRed(  polygons[i].color)/255.0;
                v.c.y = 3*qGreen(polygons[i].color)/255.0;
                v.c.z = 3*qBlue( polygons[i].color)/255.0;
                mesh->addVertex(v);
                vindex++;
            }
        }
    }

    count *= 3;
    Q_ASSERT(vindex == count);

    mesh->endMesh();
    model.addMesh(mesh,true);
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
        model.createNode(mesh,transform);
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

