#ifndef LBASDLAUDIO_H
#define LBASDLAUDIO_H

#include "lbwaudio.h"

#define CHANNELS  32

class LbwSdlAudio : public LbwAudio
{
    Q_OBJECT
public:
    LbwSdlAudio(const LbaRess &ress);

    virtual void init() override;

protected:
    virtual void playLocalMusicFile(const QString &filename);
    virtual void playVocBuffer(const QByteArray &vocData, int repeats);
    virtual void stopAll();

private slots:
    void checkChannels();

private:
    int   nextFreeChannel() const;
    void *mChannels[CHANNELS];
};

#endif // LBASDLAUDIO_H
