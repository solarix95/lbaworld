#include "lbacharacter.h"
#include "binaryreader.h"

enum CharacterOpCodes {
    OPBODY =  0x01,
    OPANIM =  0x03,
    OPEOF  =  0xff
};

//-------------------------------------------------------------------------------------------
LbaCharacter::LbaCharacter()
{

}

//-------------------------------------------------------------------------------------------
bool LbaCharacter::fromBuffer(const QByteArray &buffer)
{
    BinaryReader reader(buffer);

    quint8 opCode;

    // http://lbafileinfo.kazekr.net/index.php?title=LBA1:File3D_entity
    // twin-e: actor.c:initBody()
    do {
        opCode = reader.readUint8();
        quint8 entityIndex        = reader.readUint8(); // body or Ani-Index
        quint8 entityDataSize     = reader.readUint8(); //

        switch (opCode) {
        case OPBODY: {
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


        } break;
        case OPANIM:

            break;
        default:
            break;
        }

    } while (opCode != OPEOF && !reader.atEnd());

    return true;
}

