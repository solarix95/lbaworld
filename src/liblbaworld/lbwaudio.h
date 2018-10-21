#ifndef LBA_AUDIO_H
#define LBA_AUDIO_H

#include <QObject>
#include <lbaress.h>

class LbwAudio : public QObject
{
    Q_OBJECT
public:
    LbwAudio(const LbaRess &ress);

public slots:
    void playFlaVoc(int index, int repeats = 0);
    void playMusic(const QString &filename);
    void stop();

protected:
    virtual void playLocalMusicFile(const QString &filename) = 0;
    virtual void playVocBuffer(const QByteArray &vocData, int repeats) = 0;
    virtual void stopAll() = 0;

private:
    const LbaRess &mRess;
};

#endif
