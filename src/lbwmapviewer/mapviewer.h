#ifndef LBAMODELVIEWER_H
#define LBAMODELVIEWER_H

#include "ui_viewerform.h"
#include <lbaress.h>
#include <lbapalette.h>


class LbwMap;

class MapViewer : public QWidget
{
    Q_OBJECT
public:
    MapViewer(const LbaRess &ress);
    virtual ~MapViewer();

private slots:
    void loadMap();

private:
    void setupViewerByUI();

private:

    Ui::LbaModelUi mUi;

    const LbaRess &mRess;

    LbwMap        *mMap;
};

#endif // LBAMODELVIEWER_H
