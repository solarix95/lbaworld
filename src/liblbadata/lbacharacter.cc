#include "lbacharacter.h"
#include "binaryreader.h"

//-------------------------------------------------------------------------------------------
LbaCharacter::LbaCharacter()
    : mValid(false)
{
}

//-------------------------------------------------------------------------------------------
bool LbaCharacter::fromBuffer(const QByteArray &buffer)
{
    mBodies.clear();
    mAnimations.clear();
    mErrorString.clear();
    mValid = false;

    if (buffer.isEmpty()) {
        mErrorString = "FILE3D entity is empty";
        return false;
    }

    BinaryReader reader(buffer);
    bool foundEnd = false;
    Bodies bodies;
    Animations animations;

    while (!reader.atEnd()) {
        const int blockStart = reader.pos();
        const quint8 opCode = reader.readUint8();
        if (reader.error())
            break;
        if (opCode == OPEOF) {
            foundEnd = true;
            break;
        }

        const quint8 logicalIndex = reader.readUint8();
        const quint8 blockSize = reader.readUint8();
        if (reader.error() || blockSize < 1) {
            mErrorString = QString("Invalid FILE3D block header at byte %1").arg(blockStart);
            return false;
        }

        // FILE3D stores the distance from the size byte to the next opcode.
        const int blockEnd = blockStart + 2 + blockSize;
        if (blockEnd < reader.pos() || blockEnd > reader.size()) {
            mErrorString = QString("Truncated FILE3D block at byte %1").arg(blockStart);
            return false;
        }

        if (opCode == OPBODY) {
            if (blockEnd - reader.pos() < 3) {
                mErrorString = QString("Truncated FILE3D body block at byte %1").arg(blockStart);
                return false;
            }

            Body nextBody;
            nextBody.index = logicalIndex;
            nextBody.bodyIndex = reader.readInt16();
            nextBody.hasCollisionBox = reader.readUint8() != 0;

            if (nextBody.hasCollisionBox) {
                if (blockEnd - reader.pos() < 13) {
                    mErrorString = QString("Truncated FILE3D collision box at byte %1").arg(blockStart);
                    return false;
                }
                const quint8 collisionDataSize = reader.readUint8();
                if (collisionDataSize != 14) {
                    mErrorString = QString("Invalid FILE3D collision box size %1 at byte %2")
                            .arg(collisionDataSize).arg(blockStart);
                    return false;
                }

                const qint16 bottomLeftX = reader.readInt16();
                const qint16 bottomLeftY = reader.readInt16();
                const qint16 bottomLeftZ = reader.readInt16();
                const qint16 topRightX = reader.readInt16();
                const qint16 topRightY = reader.readInt16();
                const qint16 topRightZ = reader.readInt16();
                nextBody.bottomLeft = QVector3D(bottomLeftX, bottomLeftY, bottomLeftZ);
                nextBody.topRight = QVector3D(topRightX, topRightY, topRightZ);
            }
            bodies << nextBody;
        } else if (opCode == OPANIM) {
            if (blockEnd - reader.pos() < 2) {
                mErrorString = QString("Truncated FILE3D animation block at byte %1").arg(blockStart);
                return false;
            }

            Animation nextAnimation;
            nextAnimation.index = logicalIndex;
            nextAnimation.animationIndex = reader.readInt16();
            nextAnimation.actions = reader.readBlock(blockEnd - reader.pos());
            animations << nextAnimation;
        }

        if (reader.error() || !reader.seek(blockEnd)) {
            mErrorString = QString("Cannot read FILE3D block at byte %1").arg(blockStart);
            return false;
        }
    }

    if (!foundEnd) {
        mErrorString = "FILE3D entity has no end marker";
        return false;
    }
    if (bodies.isEmpty()) {
        mErrorString = "FILE3D entity contains no body";
        return false;
    }

    mBodies = bodies;
    mAnimations = animations;
    mValid = true;
    return true;
}

//-------------------------------------------------------------------------------------------
bool LbaCharacter::isValid() const
{
    return mValid;
}

//-------------------------------------------------------------------------------------------
QString LbaCharacter::errorString() const
{
    return mErrorString;
}

//-------------------------------------------------------------------------------------------
const LbaCharacter::Bodies &LbaCharacter::bodies() const
{
    return mBodies;
}

//-------------------------------------------------------------------------------------------
const LbaCharacter::Animations &LbaCharacter::animations() const
{
    return mAnimations;
}

//-------------------------------------------------------------------------------------------
const LbaCharacter::Body *LbaCharacter::body(int logicalIndex) const
{
    for (const Body &nextBody: mBodies) {
        if (nextBody.index == logicalIndex)
            return &nextBody;
    }
    return NULL;
}

//-------------------------------------------------------------------------------------------
const LbaCharacter::Animation *LbaCharacter::animation(int logicalIndex) const
{
    for (const Animation &nextAnimation: mAnimations) {
        if (nextAnimation.index == logicalIndex)
            return &nextAnimation;
    }
    return NULL;
}

//-------------------------------------------------------------------------------------------
QString LbaCharacter::bodyName(int logicalIndex)
{
    return logicalIndex == 0 ? QString("default") : QString("body-%1").arg(logicalIndex);
}

//-------------------------------------------------------------------------------------------
QString LbaCharacter::animationName(int logicalIndex)
{
    static const char *names[] = {
        "idle", "walk", "walk-backward", "turn-left", "turn-right",
        "hit", "big-hit", "fall", "landing", "landing-hit", "land-death",
        "action", "climb-ladder", "top-ladder", "jump", "throw-ball",
        "hide", "kick", "right-punch", "left-punch", "found-item",
        "drawn", "hit-2", "sabre-attack"
    };
    const int knownNameCount = int(sizeof(names) / sizeof(names[0]));
    if (logicalIndex >= 0 && logicalIndex < knownNameCount)
        return QString::fromLatin1(names[logicalIndex]);
    return QString("animation-%1").arg(logicalIndex);
}
