#ifndef LBWCHARACTER_H
#define LBWCHARACTER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QStringList>
#include <libqtr3d/physics/qtr3daaboundingbox.h>
#include <lbaanimation.h>

class Qtr3dModel;

class LbwCharacter : public QObject
{
public:
    LbwCharacter(const QString &name);

    struct AnimationClip {
        QString      name;
        int          logicalIndex;
        int          resourceIndex;
        LbaAnimation animation;

        AnimationClip()
            : logicalIndex(-1)
            , resourceIndex(-1)
        {}
        AnimationClip(const QString &clipName, int logical, int resource,
                      const LbaAnimation &source)
            : name(clipName)
            , logicalIndex(logical)
            , resourceIndex(resource)
            , animation(source)
        {}
    };

    void addCostume(const QString &costumeName, Qtr3dModel *model, const Qtr3dAaBoundingBox &aabb);
    void addAnimation(const QString &animationName, int logicalIndex,
                      int resourceIndex, const LbaAnimation &animation);

    QString                      name() const;
    QStringList                  costumes() const;
    Qtr3dModel                *model(const QString &costumeName);
    const Qtr3dAaBoundingBox  *aabb(const QString &costumeName);
    QStringList                  animations() const;
    const AnimationClip         *animation(const QString &animationName) const;
    const AnimationClip         *animation(int logicalIndex) const;

private:

    struct Costume {
        QString            name;
        Qtr3dModel        *model;
        Qtr3dAaBoundingBox aabb;

        Costume(const QString &n,Qtr3dModel *m, const Qtr3dAaBoundingBox &box)
          : name(n)
          , model(m)
          , aabb(box)
        {}

        Costume()
          : name("")
          , model(nullptr)
        {}
    };


    QString               mName;
    QMap<QString,Costume> mCostumes;
    QMap<QString,AnimationClip> mAnimations;

};

using LbwCharacters = QList<LbwCharacter*>;

#endif // LBWCHARACTER_H
