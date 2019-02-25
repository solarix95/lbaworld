#include "lbwsdlaudio.h"
#include <QDebug>
#include <QTimer>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

/** Original audio frequency */
#define ORIGINAL_GAME_FREQUENCY		11025
/** High quality audio frequency */
#define HIGH_QUALITY_FREQUENCY		44100

//-------------------------------------------------------------------------------------------------
LbwSdlAudio::LbwSdlAudio(const LbaRess &ress)
 : LbwAudio(ress)
{
    memset(&mChannels, 0, sizeof(mChannels));
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::init()
{
    emit log(tr("initialze SDL..."));
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        qWarning() << "SDL INIT ERROR";
        emit log(tr("SDL_Init error: '%1'").arg(SDL_GetError()));
    } else
        emit log(tr("#00ff00 ...done"));

    emit log(tr("initialze Mixer with %1 %2 2 256...").arg(ORIGINAL_GAME_FREQUENCY).arg(AUDIO_S16));
    if (Mix_OpenAudio(ORIGINAL_GAME_FREQUENCY, AUDIO_S16, 2, 256) < 0) {
        qWarning() << "Mix_OpenAudio" << Mix_GetError();
        emit log(tr("Mix_OpenAudio error: '%1'").arg(Mix_GetError()));
    } else
        emit log(tr("#00ff00 ...done"));

    QTimer *t = new QTimer();
    connect(t, SIGNAL(timeout()), this, SLOT(checkChannels()));
    t->start(66);
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::playLocalMusicFile(const QString &filename)
{
    Mix_Music *mus = Mix_LoadMUS(filename.toUtf8().constData());
    Mix_PlayMusic(mus,1); //Music loop=1
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::playVocBuffer(const QByteArray &vocData, int repeats)
{
    if (vocData.isEmpty())
        return;

    int nextChannel = nextFreeChannel();
    if (nextChannel < 0)
        return;

    QByteArray data = vocData; // "C"-Bug in original HQR-Files..
    if (data[0] != 'C')
        data[0] = 'C';

    SDL_RWops *rw     = SDL_RWFromConstMem(data.constData(), data.length());
    mChannels[nextChannel] = Mix_LoadWAV_RW(rw, 1);
    if(!mChannels[nextChannel]) {
        qWarning() << "Mix_LoadWAV_RW" << Mix_GetError();
        return;
    }
    Mix_Volume(nextChannel, 50);
    qWarning() << Mix_PlayChannel(nextChannel, (Mix_Chunk*)mChannels[nextChannel], repeats);
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::stopAll()
{
    for (int i=0; i<CHANNELS; i++) {
        if (!mChannels[i])
            continue;
        if (Mix_Playing(i)) {
            Mix_HaltChannel(i);
        }
        Mix_FreeChunk((Mix_Chunk*)mChannels[i]);
        mChannels[i] = NULL;

    }
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::checkChannels()
{
    for (int i=0; i<CHANNELS; i++) {
        if (!mChannels[i])
            continue;
        if (!Mix_Playing(i)) {
            Mix_FreeChunk((Mix_Chunk*)mChannels[i]);
            mChannels[i] = NULL;
        }
    }
}

//-------------------------------------------------------------------------------------------------
int LbwSdlAudio::nextFreeChannel() const
{
    for (int i=0; i<CHANNELS; i++) {
        if (!mChannels[i])
            return i;
    }
    return -1;
}

