#include "lbwsfmlaudio.h"
#include <QDebug>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>


//-------------------------------------------------------------------------------------------------
LbwSfmlAudio::LbwSfmlAudio(const LbaRess &ress)
 : LbwAudio(ress),
   mMusic(NULL)
{
}

//-------------------------------------------------------------------------------------------------
void LbwSfmlAudio::playLocalMusicFile(const QString &filename)
{
    mMusic = new sf::Music();
    mMusic->openFromFile(filename.toStdString());
    mMusic->play();
}

//-------------------------------------------------------------------------------------------------
void LbwSfmlAudio::playVocBuffer(const QByteArray &vocData)
{
    sf::SoundBuffer b;
    b.loadFromMemory(vocData.constData(), vocData.length());
}
