#include "lbamoviewidget.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>

LbaMovieWidget::LbaMovieWidget(QWidget *parent) : QWidget(parent),
    mMovie(NULL)
{
    if (qApp->arguments().contains("fullscreen"))
        setWindowState(Qt::WindowFullScreen);
}

void LbaMovieWidget::play(FlaMovie *movie)
{
    mMovie = movie;
    if (!mMovie)
        return;

    // resize(mMovie->size());
    connect(mMovie, SIGNAL(playFrame(int)), this, SLOT(update()));
    mMovie->start();
}

void LbaMovieWidget::paintEvent(QPaintEvent *)
{
    if (!mMovie || mMovie->currentFrameIndex() < 0)
        return;

    QPainter p(this);
    QImage img = mMovie->currentFrame();
    img = img.scaled(size(),Qt::KeepAspectRatio, Qt::SmoothTransformation);
    int x = (size().width()-img.width())/2;
    int y = (size().height()-img.height())/2;
    if (x || y) {
        p.setBrush(Qt::black);
        p.drawRect(0,0,size().width(),size().height());
    }
    p.drawImage(x,y,img);
}

