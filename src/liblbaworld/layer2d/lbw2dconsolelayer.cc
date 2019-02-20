#include <QDebug>
#include <math.h>
#include "layer2d/lbw2dconsolelayer.h"
#include "lbwconsole.h"

//-------------------------------------------------------------------------------------------------
Lbw2dConsoleLayer::Lbw2dConsoleLayer(LbwConsole *c)
 : mIsActive(false), mConsole(c)
{
    mPosCurrent = mPosTarget = -600;
}

//-------------------------------------------------------------------------------------------------
bool Lbw2dConsoleLayer::processState(float speed)
{
    mPlasma.process(speed);
    if (mPosCurrent != mPosTarget) {
        mPosCurrent = mPosCurrent + (mPosTarget-mPosCurrent)*0.2*speed;
        if (inPosition())
            mPosCurrent = mPosTarget;
        return true;
    }
    return mIsActive;
}

//-------------------------------------------------------------------------------------------------
void Lbw2dConsoleLayer::render(const QRect &window, QPainter &p)
{
    if (!mIsActive && inPosition())
        return;

    int width  = qMin(window.width()-30,800);
    int height = 600;
    int margin = (window.width()-width)/2;

    p.translate(margin,mPosCurrent);
    QRect rect(0,0,width, height);
    p.setClipRect(rect);

    p.save();
    mPlasma.render(rect,p);
    p.restore();

    p.setPen(QPen(Qt::black,2));
    p.drawRect(rect);

    p.setPen(Qt::white);
    p.setFont(QFont("monospace"));
    p.drawText(QRect(rect.x(),rect.height()-20,rect.width(),20),Qt::AlignLeft | Qt::AlignVCenter,"] " + mCurrentInput);

    for (int i=0; i<mConsole->logCount(); i++) {
        int logIndex = mConsole->logCount()-1-i;
        p.setPen(mConsole->logColor(logIndex));
        p.drawText(QRect(rect.x(),rect.height()-(i+2)*20,rect.width(),20),Qt::AlignLeft | Qt::AlignVCenter,mConsole->logEntry(logIndex));
    }
}

//-------------------------------------------------------------------------------------------------
bool Lbw2dConsoleLayer::keyEvent(QKeyEvent *ke)
{
    if (ke->type() == QEvent::KeyPress && ke->key() == Qt::Key_F1) {
        mIsActive = !mIsActive;
        if (mIsActive)
            mPosTarget = -200;
        else
            mPosTarget = -600;
        return true;
    }

    if (mIsActive && ke->type() == QEvent::KeyPress) {
        handleKeyEvent(ke);
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
void Lbw2dConsoleLayer::handleKeyEvent(QKeyEvent *ke)
{
    switch (ke->key()) {
    case Qt::Key_Backspace: {
        if (mCurrentInput.isEmpty())
            return;
        mCurrentInput.remove(mCurrentInput.length()-1,1);
    } break;
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        mConsole->exec(mCurrentInput);
        mCurrentInput.clear();
    } break;
    default: mCurrentInput += ke->text();
    }
}

//-------------------------------------------------------------------------------------------------
bool Lbw2dConsoleLayer::inPosition() const
{
    return fabs(mPosCurrent-mPosTarget) < 1;
}
