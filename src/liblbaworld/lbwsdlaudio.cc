#include "lbwsdlaudio.h"
#include <QDebug>
#include <QTimer>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <csignal>

/** Original audio frequency */
#define ORIGINAL_GAME_FREQUENCY		11025
/** High quality audio frequency */
#define HIGH_QUALITY_FREQUENCY		44100

//-------------------------------------------------------------------------------------------------
LbwSdlAudio::LbwSdlAudio(const LbaRess &ress)
 : LbwAudio(ress)
 , mMusic(nullptr)
 , mSdlInitialized(false)
 , mMixerOpen(false)
{
    memset(&mChannels, 0, sizeof(mChannels));
}

//-------------------------------------------------------------------------------------------------
LbwSdlAudio::~LbwSdlAudio()
{
    if (mMixerOpen) {
        stopAll();
        Mix_HaltMusic();
        if (mMusic) {
            Mix_FreeMusic(static_cast<Mix_Music*>(mMusic));
            mMusic = nullptr;
        }
        // Mix_CloseAudio waits for the mixer callback thread. It must happen
        // before SDL unloads the audio subsystem during process shutdown.
        Mix_CloseAudio();
        mMixerOpen = false;
    }
    if (mSdlInitialized) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        mSdlInitialized = false;
    }
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::init()
{
    if (mMixerOpen)
        return;

    emit log(tr("initialze SDL..."));
    // Qt owns the event loop and Unix signal handling. SDL 1.2's parachute
    // handlers would otherwise swallow SIGTERM without an SDL event pump.
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) < 0) {
        qWarning() << "SDL INIT ERROR";
        emit log(tr("SDL_Init error: '%1'").arg(SDL_GetError()));
        return;
    } else {
        mSdlInitialized = true;
        emit log(tr("#00ff00 ...done"));
    }

#if defined(Q_OS_UNIX)
    // SDL 1.2 still installs quit handlers on some builds. There is no SDL
    // event pump in this Qt application, so restore normal process handling.
    std::signal(SIGINT, SIG_DFL);
    std::signal(SIGTERM, SIG_DFL);
#endif

    emit log(tr("initialze Mixer with %1 %2 2 256...").arg(ORIGINAL_GAME_FREQUENCY).arg(AUDIO_S16));
    if (Mix_OpenAudio(ORIGINAL_GAME_FREQUENCY, AUDIO_S16, 2, 256) < 0) {
        qWarning() << "Mix_OpenAudio" << Mix_GetError();
        emit log(tr("Mix_OpenAudio error: '%1'").arg(Mix_GetError()));
        return;
    } else {
        mMixerOpen = true;
        Mix_AllocateChannels(CHANNELS);
        emit log(tr("#00ff00 ...done"));
    }

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(checkChannels()));
    t->start(66);
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::playLocalMusicFile(const QString &filename)
{
    if (!mMixerOpen || filename.isEmpty())
        return;

    Mix_HaltMusic();
    if (mMusic) {
        Mix_FreeMusic(static_cast<Mix_Music*>(mMusic));
        mMusic = nullptr;
    }

    mMusic = Mix_LoadMUS(filename.toUtf8().constData());
    if (!mMusic) {
        emit log(tr("Cannot load music '%1': %2").arg(filename, Mix_GetError()));
        return;
    }
    if (Mix_PlayMusic(static_cast<Mix_Music*>(mMusic), 1) < 0)
        emit log(tr("Cannot play music '%1': %2").arg(filename, Mix_GetError()));
}

//-------------------------------------------------------------------------------------------------
void LbwSdlAudio::playVocBuffer(const QByteArray &vocData, int repeats)
{
    if (!mMixerOpen || vocData.isEmpty())
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
    if (!mMixerOpen)
        return;

    Mix_HaltMusic();
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
    if (!mMixerOpen)
        return;

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
