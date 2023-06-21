#ifndef LBAMODELVIEWER_H
#define LBAMODELVIEWER_H

#include "ui_viewerform.h"
#include <lbaress.h>
#include <lbapalette.h>

class LbaBody;
class LbaAnimation;

class LbaModelViewer : public QWidget
{
    Q_OBJECT
public:
    LbaModelViewer(const LbaRess &ress);
    virtual ~LbaModelViewer();

private slots:
    void loadModel();

private:
    void loadBody(LbaBody &body, const LbaPalette &pal,  LbaAnimation *animation);
    void loadBodyMeshes(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);
    void loadBodySpheres(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);
    void loadBodyLines(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);
    void loadModelNodes(Qtr3dModel &model, Qtr3dMesh *mesh, const LbaBody &body, int lbaParentId, Qtr3dModel::Node *parentNode);
    void loadBodyBones(Qtr3dModel &model, const LbaBody &body);
    void loadBodyAnimation(Qtr3dModel &model, const LbaBody &body, LbaAnimation *animation);

    // UI
    void setupViewerByUI();

private:

    Ui::LbaModelUi mUi;

    const LbaRess &mRess;
};

#endif // LBAMODELVIEWER_H
