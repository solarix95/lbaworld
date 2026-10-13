#include <QDebug>
#include "lbacharacter.h"
#include "binaryreader.h"



//-------------------------------------------------------------------------------------------
LbaCharacter::LbaCharacter()
{

}

//-------------------------------------------------------------------------------------------
bool LbaCharacter::fromBuffer(const QByteArray &buffer)
{
    BinaryReader reader(buffer);

    quint8 opCode;

    // http://lbafileinfo.kaziq.net/index.php/LBA1:File3D_entity
    // twin-e: actor.c:initBody()
    do {
        opCode            = reader.readUint8();
        quint8 blockIndex = reader.readUint8();
        quint8 blockSize  = reader.readUint8()-1;
        int    pos        = reader.pos();

         switch (opCode) {

            case OPBODY: {
             qDebug() << "BODY";
            /*
+00:       byte        Body index (in File3D.HQR)
01:        byte        Body data size - 1 (only take the following values)
                 |-> 0x04 (4) -> When no collision box is used
                 |--> 0x11 (17) -> When used a collision box
02:        s16        Body real index (in Body.HQR)
04:        byte        Collision box flag
                 |-> flag = 0x00 (0) -> Model doesn't use collision box
                 |--> flag = 0x01 (1) -> Model use collision box

            */

            qint16 bodyRealIndex    = reader.readInt16();
            quint8 collisionBoxFlag = reader.readUint8();

            Body body;
            body.bodyIndex = bodyRealIndex;
            if (body.bodyIndex == 70)
            qDebug() << "BODY" << body.bodyIndex;
            if (collisionBoxFlag == UseCollisionBox) {
                /*

    [ IF Collision box flag is set ]
    05:        byte        Number of bytes to read minus 2. In this case the value is always 0x0E, so it will have more 0x0E-0x02 = 0x0C bytes
    06:        u16        X (bottom left)
    08:        u16        Y (bottom left)
    0A:        u16        Z (bottom left)
    0C:        u16        X (top right)
    0E:        u16        Y (top right)
    10:        u16        Z (top right)

                */

                // quint8  bytesToRead  = reader.readUint8();
                reader.skip(1);
                quint16 bottomLeftX  = reader.readUint16();
                quint16 bottomLeftY  = reader.readUint16();
                quint16 bottomLeftZ  = reader.readUint16();

                quint16 topRightX    = reader.readUint16();
                quint16 topRightY    = reader.readUint16();
                quint16 topRightZ    = reader.readUint16();

                body.bottomLeft = QVector3D(bottomLeftX,bottomLeftY,bottomLeftZ);
                body.topRight   = QVector3D(topRightX,topRightY,topRightZ);


               qDebug() << "BODY " << body.bodyIndex << body.bottomLeft/800.0 << body.topRight/800.0;
            }
            mBodies << body;

        } break;
        case OPANIM: {
              qDebug() << "ANIM";
              qint16 bodyAnimIndex    = reader.readInt16();
              qDebug() << "ANIM" << bodyAnimIndex;
              // qDebug() << blockIndex << bodyAnimIndex;
            } break;
         case OPEOF:
               //qDebug() << "EOF";

             break;
        default:
             //qDebug() << "WTF";
            break;
        }

        reader.skip(blockSize - (reader.pos() - pos));

    } while (opCode != OPEOF && !reader.atEnd());

    return !mBodies.isEmpty();
}

