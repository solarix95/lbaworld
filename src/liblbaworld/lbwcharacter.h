#ifndef LBWCHARACTER_H
#define LBWCHARACTER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <libqtr3d/physics/qtr3daaboundingbox.h>

class Qtr3dModel;

class LbwCharacter : public QObject
{
public:
    LbwCharacter(const QString &name);

    void addCostume(const QString &costumeName, Qtr3dModel *model, const Qtr3dAaBoundingBox &aabb);

    Qtr3dModel                *model(const QString &costumeName);
    const Qtr3dAaBoundingBox  *aabb(const QString &costumeName);

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

};

using LbwCharacters = QList<LbwCharacter*>;

#endif // LBWCHARACTER_H
