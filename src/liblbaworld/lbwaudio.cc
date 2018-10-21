
#include "lbwaudio.h"

//-------------------------------------------------------------------------------------------------
LbwAudio::LbwAudio(const LbaRess &ress)
 : mRess(ress)
{
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::playFlaVoc(int index, int repeats)
{
    QByteArray buffer = mRess.flaSample(index);
    if (buffer.length() > 0)
        playVocBuffer(buffer, repeats);
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::playMusic(const QString &filename)
{
    playLocalMusicFile(filename);
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::stop()
{
    stopAll();
}

