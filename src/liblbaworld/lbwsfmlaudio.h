#ifndef LBA_SFML_AUDIO_H
#define LBA_SFML_AUDIO_H


#include "lbwaudio.h"

namespace sf {
  class Music;
}
class LbwSfmlAudio : public LbwAudio
{
public:
    LbwSfmlAudio(const LbaRess &ress);

protected:
    virtual void playLocalMusicFile(const QString &filename);
    virtual void playVocBuffer(const QByteArray &vocData);

private:
    sf::Music *mMusic;
};

#endif
