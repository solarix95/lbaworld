#ifndef LBAMODELVIEWER_H
#define LBAMODELVIEWER_H

#include "ui_viewerform.h"
#include <lbaress.h>
#include <lbapalette.h>

class LbaBody;
class LbaAnimation;
class LbwActor;
class LbwCharacter;

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
    void clearCharacter();
    bool loadCharacter(int entityIndex, int logicalBodyIndex = 0);
    void startAnimationDemo();

private:

    Ui::LbaModelUi mUi;

    const LbaRess &mRess;
    Qtr3dModel         *mModel;
    Qtr3dGeometryState *mState;
    LbwCharacter       *mCharacter;
    LbwActor           *mActor;
};

#endif // LBAMODELVIEWER_H
