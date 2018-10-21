#ifndef LBACHARACTER_H
#define LBACHARACTER_H

#include <QByteArray>

class LbaCharacter
{
public:
    LbaCharacter();

    bool fromBuffer(const QByteArray &buffer);
};

#endif // LBACHARACTER_H
