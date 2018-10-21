#ifndef LBA_FLA_MOVIE_H
#define LBA_FLA_MOVIE_H

#include <QByteArray>
#include <QImage>
#include <QList>
#include <QTimer>
#include <QVector>
#include "binaryreader.h"

class FlaMovie : public QObject
{
    Q_OBJECT

public:
    FlaMovie();

    // Loading from FLA-File
    bool fromFile(const QString &filename);
    bool fromBuffer(const QByteArray &buffer);

    int           frameCount() const;
    const QImage &frame(int i);
    int           fps() const;
    QSize         size() const;

    // Player

    int           currentFrameIndex() const;
    const QImage &currentFrame() const;

signals:
    void loaded();
    void playFrame(int i);
    void playFlaSample(int i, int repeats);
    void finished();

public slots:
    void          start();
    void          stop();
    void          requestFrame(int i);
    void          requestFps(int f);

private slots:
    void nextFrame();

private:

    // FLA-Parser
    bool processFrame();
    void appendKeyFrame(quint8 *inBuffer);
    void appendDeltaFrame(quint8 *inBuffer);
    void fadeToBlack();
    void fadeFromBlack();

    BinaryReader  mBinReader;
    QVector<QRgb> mColorTable;
    int           mFade;

    // Parsed FLA-Animation
    struct Frame {
        QImage frame;
        QList<QPair<int,int> > samples; // index / repeats
        Frame() {}
        Frame(const QImage &f) : frame(f) {}
    };

    QList<Frame>  mFrames;
    int           mSpeed;

    // Player
    void          setCurrentFrame(int i);
    QTimer        mAnimationTimer;
    int           mCurrentFrame;
};


#endif
