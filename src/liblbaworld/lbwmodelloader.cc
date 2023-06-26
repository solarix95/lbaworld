#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dmodelanimation.h>
#include <libqtr3d/qtr3dmesh.h>
#include <libqtr3d/qtr3dfactory.h>

#include "lbwmodelloader.h"
#include "lbabody.h"
#include "lbaanimation.h"

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


namespace LbwModelLoader {

//--------------------------------------------------------------------------------------------------
void loadBody(Qtr3dModel &model, LbaBody &body, const LbaPalette &pal, LbaAnimation *animation)
{
    // body.translateVertices();
    loadBodySkin(model,body); // LBA1 Skin + Qtr3d-Nodes

    loadBodySpheres(model,body, pal);
    // loadBodyLines(*model,body, pal);

    loadBodyAnimation(model,body,animation);
}

//--------------------------------------------------------------------------------------------------
void loadBodySkin(Qtr3dModel &model, const LbaBody &body)
{
    const auto &polygons = body.polygons();
    const auto &vertices = body.vertices();
    const auto &normals  = body.normals();


    auto *mesh = model.context()->createMesh(false);

    mesh->startMesh(Qtr3d::Triangle, Qtr3d::CounterClockWise);

    int vindex = 0;

    for (int i=0; i<polygons.count(); i++) {

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
    }

    mesh->endMesh();
    model.addMesh(mesh,false);
    loadModelNodes(model,mesh,body,-1,nullptr);
    loadSkinBones(mesh,body);
}

//--------------------------------------------------------------------------------------------------
void loadBodySpheres(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal)
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

        auto *mesh = model.context()->createMesh(false);
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

//--------------------------------------------------------------------------------------------------
void loadModelNodes(Qtr3dModel &model, Qtr3dMesh *mesh, const LbaBody &body, int lbaParentId, Qtr3dModel::Node *parentNode)
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

//--------------------------------------------------------------------------------------------------
void loadSkinBones(Qtr3dMesh *skinMesh, const LbaBody &body)
{
    if (body.bones().count() <= 0) {
        qWarning() << "Invalid model. Cant load bones";
        return;
    }

    Q_ASSERT(skinMesh);

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

        skinMesh->addBone(bone);
    }
}

void loadBodyAnimation(Qtr3dModel &model, const LbaBody &body, LbaAnimation *animation)
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


}

