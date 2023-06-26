#ifndef LBWMODELLOADER_H
#define LBWMODELLOADER_H

#include "libqtr3d/qtr3dmodel.h"

class LbaBody;
class LbaPalette;
class LbaAnimation;

namespace LbwModelLoader
{

void loadBody(Qtr3dModel &model, LbaBody &body, const LbaPalette &pal,  LbaAnimation *animation);
void loadBodySkin(Qtr3dModel &model, const LbaBody &body);
void loadBodySpheres(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);
void loadBodyLines(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);
void loadModelNodes(Qtr3dModel &model, Qtr3dMesh *mesh, const LbaBody &body, int lbaParentId, Qtr3dModel::Node *parentNode);
void loadSkinBones(Qtr3dMesh *skinMesh, const LbaBody &body);
void loadBodyAnimation(Qtr3dModel &model, const LbaBody &body, LbaAnimation *animation);

};

#endif // LBWMODELLOADER_H
