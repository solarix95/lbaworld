#ifndef LBWACTOR_H
#define LBWACTOR_H

#include <QObject>
#include <QList>
#include <libqtr3d/qtr3dmodel.h>
#include <libqtr3d/qtr3dgeometrystate.h>
#include <libqtr3d/physics/qtr3dstandardentity.h>

class LbwCharacter;

class LbwActor : public Qtr3dStandardEntity
{
public:
    LbwActor(LbwCharacter *character);

    void setCostume(const QString &costumeName, const QString &animation = "");
    void setAnimation(const QString &animation);

    inline Qtr3dGeometryState *currentState() const { return mState; }

private:

    LbwCharacter       *mCharacter;
    Qtr3dModel         *mCurrentModel;

    Qtr3dGeometryState *mState;

    QString             mCurrentCostume;
    QString             mCurrentAnimation;
};

using LbwActors = QList<LbwActor*>;

#endif // LBWACTOR_H
