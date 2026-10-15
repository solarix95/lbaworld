#ifndef LBACHARACTER_H
#define LBACHARACTER_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QVector3D>

class LbaCharacter
{
public:
    enum CharacterOpCode {
        OPBODY = 0x01,
        OPANIM = 0x03,
        OPEOF  = 0xff
    };

    struct Body {
        int       index;      // Logical body index used by scripts/actors.
        int       bodyIndex;  // Entry in BODY.HQR.
        bool      hasCollisionBox;
        QVector3D bottomLeft;
        QVector3D topRight;

        Body()
            : index(-1)
            , bodyIndex(-1)
            , hasCollisionBox(false)
        {}
    };

    struct Animation {
        int        index;          // Logical animation index used by scripts.
        int        animationIndex; // Entry in ANIM.HQR.
        QByteArray actions;        // FILE3D animation actions, kept for later use.

        Animation() : index(-1), animationIndex(-1) {}
    };

    typedef QList<Body> Bodies;
    typedef QList<Animation> Animations;

    LbaCharacter();

    bool fromBuffer(const QByteArray &buffer);
    bool isValid() const;
    QString errorString() const;

    const Bodies &bodies() const;
    const Animations &animations() const;
    const Body *body(int logicalIndex) const;
    const Animation *animation(int logicalIndex) const;

    static QString bodyName(int logicalIndex);
    static QString animationName(int logicalIndex);

private:
    Bodies     mBodies;
    Animations mAnimations;
    QString    mErrorString;
    bool       mValid;
};

#endif // LBACHARACTER_H
