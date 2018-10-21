#ifndef LBAMODELVIEWER_H
#define LBAMODELVIEWER_H

#include "ui_viewerform.h"
#include <lbaress.h>

class LbaModelViewer : public QWidget
{
    Q_OBJECT
public:
    LbaModelViewer(const LbaRess &ress);
    virtual ~LbaModelViewer();

private slots:
    void loadModel();

private:
    Ui::LbaModelUi mUi;

    const LbaRess &mRess;
};

#endif // LBAMODELVIEWER_H
