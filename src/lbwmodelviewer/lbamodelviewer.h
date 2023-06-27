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
    void setupViewerByUI();

private:

    Ui::LbaModelUi mUi;

    const LbaRess &mRess;
    Qtr3dModel         *mModel;
    Qtr3dGeometryState *mState;
};

#endif // LBAMODELVIEWER_H
