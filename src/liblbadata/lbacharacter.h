#ifndef LBACHARACTER_H
#define LBACHARACTER_H

#include <QByteArray>
#include <QVector3D>
#include <QList>

class LbaCharacter
{
public:
    enum CharacterOpCodes {
        OPBODY =  0x01,
        OPANIM =  0x03,
        OPEOF  =  0xff
    };

    enum CollicionBoxFlag {
        NoCollisionBox  = 0x00,
        UseCollisionBox = 0x01
    };

    struct Body {
        int       bodyIndex;  // Index in BODY.HQR
        QVector3D bottomLeft;
        QVector3D topRight;
    };


    LbaCharacter();


    bool fromBuffer(const QByteArray &buffer);

private:
    QList<Body> mBodies;
};

#endif // LBACHARACTER_H
