#include "lbwscreen.h"
#include "layer2d/lbw2dposterlayer.h"
#include "layer2d/lbw2dconsolelayer.h"

#include <libqtr3d/qtr3dassets.h>
#include <libqtr3d/physics/qtr3dfpsloop.h>

namespace {

const float LegacyFrameDurationMs = 66.0f;

}

//-------------------------------------------------------------------------------------------------
LbwScreen::LbwScreen(LbwConsole *c, QWidget *parent)
    : Qtr3dWidget(parent),
      mStatusText("LBW Engine"),
      mConsole(c),
      mInitialized(false)
{
    Q_ASSERT(c);

    connect(this, &Qtr3dWidget::initialized, this, &LbwScreen::initWidget);
    connect(&assets()->loop(), &Qtr3dFpsLoop::step, this,
            [this](float simulationTimeMs, float, int) {
        if (processState(simulationTimeMs/LegacyFrameDurationMs))
            update();
    });
    assets()->loop().setFps(25);
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
    if (mInitialized)
        return;
    mInitialized = true;

    // Setup Poster-Renderer
    m2dLayers << new Lbw2dPosterLayer();
    m2dLayers.last()->setParent(this);
    bool done =
       connect(this, SIGNAL(requestFadeTo(QImage)), m2dLayers.last(), SLOT(showImage(QImage))) &&
       connect(this, SIGNAL(requestFadeTo(FlaMovie*)), m2dLayers.last(), SLOT(showFla(FlaMovie*)));
    Q_ASSERT(done);
    done = connect(m2dLayers.last(), SIGNAL(updateRequest()), this, SLOT(update())); Q_ASSERT(done);

    m2dLayers << new Lbw2dConsoleLayer(mConsole);
    m2dLayers.last()->setParent(this);
    done = connect(m2dLayers.last(), SIGNAL(updateRequest()), this, SLOT(update())); Q_ASSERT(done);

    emit ready2run();
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::paint2D()
{
    Qtr3dWidget::paint2D();
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
    return Qtr3dWidget::event(e);
}

//-------------------------------------------------------------------------------------------------
bool LbwScreen::processState(float speed)
{
    bool subResult = false;

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
