#include "lbwscreen.h"
#include "layer2d/lbw2dposterlayer.h"
#include "layer2d/lbw2dconsolelayer.h"

//-------------------------------------------------------------------------------------------------
LbwScreen::LbwScreen(LbwConsole *c, QWidget *parent)
    : Lbw3dWidget(parent),
      mStatusText("LBW Engine"),
      mConsole(c)
{
    Q_ASSERT(c);
}

//-------------------------------------------------------------------------------------------------
LbwScreen::~LbwScreen()
{
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::setStatusText(const QString &statustext)
{
    mStatusText = statustext;
    update();
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::fadeTo(const QImage &img)
{
    emit requestFadeTo(img); // redirect request to 2D-Layer "PosterLayer"
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::fadeTo(FlaMovie *movie)
{
    emit requestFadeTo(movie);
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::initWidget()
{
    // Setup Poster-Renderer
    m2dLayers << new Lbw2dPosterLayer();
    bool done =
       connect(this, SIGNAL(requestFadeTo(QImage)), m2dLayers.last(), SLOT(showImage(QImage))) &&
       connect(this, SIGNAL(requestFadeTo(FlaMovie*)), m2dLayers.last(), SLOT(showFla(FlaMovie*)));
    Q_ASSERT(done);
    done = connect(m2dLayers.last(), SIGNAL(updateRequest()), this, SLOT(update())); Q_ASSERT(done);

    m2dLayers << new Lbw2dConsoleLayer(mConsole);
    done = connect(m2dLayers.last(), SIGNAL(updateRequest()), this, SLOT(update())); Q_ASSERT(done);

    Lbw3dWidget::initWidget();
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::paint2dLayers()
{
    QPainter p(this);

    if (!mStatusText.isEmpty()) {
        p.save();
        p.setPen(Qt::white);
        p.drawText(rect(),Qt::AlignCenter, mStatusText);
        p.restore();
    }

    foreach (Lbw2dLayer *layer, m2dLayers) {
        if (!layer->isVisible())
            continue;
        p.save();
        layer->render(rect(), p);
        p.restore();
    }
}

//-------------------------------------------------------------------------------------------------
bool LbwScreen::event(QEvent *e)
{
    if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = dynamic_cast<QKeyEvent*>(e);
        Q_ASSERT(ke);
        if (e->type() == QEvent::KeyPress) {
            if (ke->key() == Qt::Key_F11) {
                toggleFullscreen();
            }
        }
        for (int i=m2dLayers.count()-1; i >= 0; i--) {
            if (m2dLayers.at(i)->keyEvent(ke)) {
                update();
                return true;
            }
        }
    }
    return Lbw3dWidget::event(e);
}

//-------------------------------------------------------------------------------------------------
bool LbwScreen::processState(float speed)
{
    bool subResult = Lbw3dWidget::processState(speed);

    for (int i=m2dLayers.count()-1; i >= 0; i--) {
        if (m2dLayers.at(i)->processState(speed)) {
            subResult = true;
        }
    }

    return subResult;
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::toggleFullscreen()
{
    if (windowState().testFlag(Qt::WindowFullScreen))
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    else
        setWindowState(windowState() | Qt::WindowFullScreen);
}
