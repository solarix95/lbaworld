
#include "lbwaudio.h"

//-------------------------------------------------------------------------------------------------
LbwAudio::LbwAudio(const LbaRess &ress)
 : mRess(ress)
{
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::init()
{
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::playVoc(LbaRess::Source source, LbaRess::Content cont, int index, int repeats)
{
    QByteArray buffer = mRess.data(source,cont,index);
    if (buffer.length() > 0)
        playVocBuffer(buffer, repeats);
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::playFlaVoc(int index, int repeats)
{
    QByteArray buffer = mRess.data(LbaRess::LBA1,LbaRess::FlaSmpl,index);
    if (buffer.length() > 0)
        playVocBuffer(buffer, repeats);
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::playMusic(const QString &filename)
{
    playLocalMusicFile(mRess.pathByTrack(filename));
}

//-------------------------------------------------------------------------------------------------
void LbwAudio::stop()
{
    stopAll();
}

