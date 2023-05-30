#ifndef LBAMODELVIEWER_H
#define LBAMODELVIEWER_H

#include "ui_viewerform.h"
#include <lbaress.h>
#include <lbapalette.h>

class LbaBody;
class LbaModelViewer : public QWidget
{
    Q_OBJECT
public:
    LbaModelViewer(const LbaRess &ress);
    virtual ~LbaModelViewer();

private slots:
    void loadModel();
    void loadBody(LbaBody &body, const LbaPalette &pal);
    void loadBodyMeshes(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);
    void loadBodySpheres(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);
    void loadBodyLines(Qtr3dModel &model, const LbaBody &body, const LbaPalette &pal);

private:
    Ui::LbaModelUi mUi;

    const LbaRess &mRess;
};

#endif // LBAMODELVIEWER_H
