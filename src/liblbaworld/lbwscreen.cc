#include "lbwscreen.h"
#include "layer2d/lbw2dposterlayer.h"
#include "layer2d/lbw2dconsolelayer.h"
#include "lbaress.h"
#include "lbacharacter.h"
#include "lbwactor.h"
#include "lbwcharacter.h"
#include "lbwcharacterloader.h"
#include "lbwmap.h"
#include "lbwmapdata.h"
#include "lbwplayercontroller.h"

#include <libqtr3d/qtr3dassets.h>
#include <libqtr3d/qtr3dcamera.h>
#include <libqtr3d/extras/qtr3dthirdpersoncameracontroller.h>
#include <libqtr3d/physics/qtr3dfpsloop.h>
#include <libqtr3d/physics/qtr3dsimulationworld.h>

namespace {

const float LegacyFrameDurationMs = 66.0f;

}

//-------------------------------------------------------------------------------------------------
LbwScreen::LbwScreen(LbwConsole *c, const LbaRess &resources, QWidget *parent)
    : Qtr3dWidget(parent),
      mConsole(c),
      mResources(resources),
      mMap(nullptr),
      mPlayerCharacter(nullptr),
      mPlayer(nullptr),
      mPlayerController(nullptr),
      mInitialized(false)
{
    Q_ASSERT(c);

    connect(this, &Qtr3dWidget::initialized, this, &LbwScreen::initWidget);
    connect(&assets()->loop(), &Qtr3dFpsLoop::step, this,
            [this](float simulationTimeMs, float, int) {
        const bool playerChanged = mPlayerController
                && mPlayerController->update(simulationTimeMs/1000.0f);
        const bool playerVisible = mMap && mPlayer && mPlayer->currentState()
                && mPlayer->currentState()->enabled();
        if (playerChanged || playerVisible
                || processState(simulationTimeMs/LegacyFrameDurationMs))
            update();
    });
    assets()->loop().setFps(25);
}

//-------------------------------------------------------------------------------------------------
LbwScreen::~LbwScreen()
{
    clearCameraController();
    makeCurrent();
    if (mPlayer && mPlayer->world())
        mPlayer->world()->removeActor(mPlayer);
    delete mPlayerController;
    mPlayerController = nullptr;
    delete mPlayer;
    mPlayer = nullptr;
    delete mPlayerCharacter;
    mPlayerCharacter = nullptr;
    delete mMap;
    mMap = nullptr;
    doneCurrent();
}

//-------------------------------------------------------------------------------------------------
bool LbwScreen::loadMap(const LbwMapData &data, QString *error)
{
    if (error)
        error->clear();
    if (!mInitialized) {
        if (error)
            *error = "3D screen is not initialized";
        return false;
    }

    makeCurrent();
    if (!ensurePlayer(error)) {
        doneCurrent();
        return false;
    }

    LbwMap *nextMap = new LbwMap(assets());
    const bool loaded = nextMap->init(data, error);
    if (!loaded)
        delete nextMap;
    else {
        delete mMap;
        mMap = nextMap;
    }
    doneCurrent();
    if (!loaded)
        return false;

    clearCameraController();
    resetPlayerInput();
    mPlayer->setRotation(QVector3D());
    mPlayer->setLookAt(QVector3D(0.0f, 0.0f, 1.0f));
    mMap->placeActor(mPlayer, data.playerSpawn());
    setPlayerVisible(true);

    Qtr3dThirdPersonCameraController *cameraController =
            qobject_cast<Qtr3dThirdPersonCameraController *>(
                setCameraController(Qtr3d::thirdPersonCamera(
                                        mPlayer->currentState(), 7.0f, 3.0f)));
    if (cameraController) {
        cameraController->setTargetDirection(mPlayer->lookAt());
        connect(mPlayer, &Qtr3dStandardEntity::lookAtChanged, cameraController,
                &Qtr3dThirdPersonCameraController::setTargetDirection);
    }

    assets()->environment().setClearColor(data.backgroundColor());
    mStatusText.clear();
    update();
    return true;
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::unloadMap()
{
    clearCameraController();
    resetPlayerInput();
    setPlayerVisible(false);
    makeCurrent();
    delete mMap;
    mMap = nullptr;
    doneCurrent();
    update();
}

//-------------------------------------------------------------------------------------------------
QString LbwScreen::currentMapName() const
{
    return mMap && mMap->isLoaded() ? mMap->data().name() : QString();
}

//-------------------------------------------------------------------------------------------------
bool LbwScreen::ensurePlayer(QString *error)
{
    if (mPlayer)
        return true;

    LbwCharacter *character = new LbwCharacter("Twinsen");
    QString loaderError;
    if (!LbwCharacterLoader::loadCostume(*character, *assets(), mResources,
                                         LbaRess::LBA1, 0, 0, &loaderError)) {
        delete character;
        if (error)
            *error = QString("Cannot load Twinsen from the LBA1 resources: %1")
                    .arg(loaderError);
        return false;
    }

    LbwActor *player = new LbwActor(character);
    player->setParent(this);
    if (!player->setCostume(LbaCharacter::bodyName(0), "idle")) {
        delete player;
        delete character;
        if (error)
            *error = "Cannot select Twinsen's default body and idle animation";
        return false;
    }

    mPlayerCharacter = character;
    mPlayerCharacter->setParent(this);
    mPlayer = player;
    mPlayerController = new LbwPlayerController(mPlayer);
    setPlayerVisible(false);
    return true;
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::setPlayerVisible(bool visible)
{
    if (!mPlayer || !mPlayer->currentState())
        return;

    mPlayer->currentState()->setEnabled(visible);
    if (visible)
        assets()->space().addActor(mPlayer);
    else if (mPlayer->world())
        mPlayer->world()->removeActor(mPlayer);
}

//-------------------------------------------------------------------------------------------------
void LbwScreen::resetPlayerInput()
{
    if (mPlayerController)
        mPlayerController->reset();
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
                if (ke->type() == QEvent::KeyPress && ke->key() == Qt::Key_F1)
                    resetPlayerInput();
                update();
                return true;
            }
        }
        if (mMap && mPlayerController && mPlayerController->keyEvent(ke)) {
            update();
            return true;
        }
    } else if (e->type() == QEvent::FocusOut
               || e->type() == QEvent::WindowDeactivate) {
        resetPlayerInput();
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
