#ifndef LBWSCREEN_H
#define LBWSCREEN_H

#include <libqtr3d/qtr3dwidget.h>

class FlaMovie;
class QImage;
class Lbw2dLayer;
class LbwConsole;
class LbaRess;
class LbwActor;
class LbwCharacter;
class LbwMap;
class LbwMapData;
class LbwPlayerController;
class LbwScreen : public Qtr3dWidget
{
    Q_OBJECT
public:
    LbwScreen(LbwConsole *c, const LbaRess &resources, QWidget *parent = 0);
    virtual ~LbwScreen();

    bool    loadMap(const LbwMapData &data, QString *error = 0);
    void    unloadMap();
    QString currentMapName() const;

public slots:
    void setStatusText(const QString &statustext);
    void fadeTo(const QImage &img);
    void fadeTo(FlaMovie *movie);

signals:
    void ready2run();
    void requestFadeTo(const QImage &img);
    void requestFadeTo(FlaMovie *movie);

protected:
    virtual void paint2D() override;
    virtual bool event(QEvent *e) override;

private:
    void initWidget();
    bool ensurePlayer(QString *error);
    void setPlayerVisible(bool visible);
    void resetPlayerInput();
    bool processState(float speed);
    void toggleFullscreen();

    QString            mStatusText;
    QList<Lbw2dLayer*> m2dLayers;
    LbwConsole        *mConsole;
    const LbaRess     &mResources;
    LbwMap            *mMap;
    LbwCharacter      *mPlayerCharacter;
    LbwActor          *mPlayer;
    LbwPlayerController *mPlayerController;
    bool               mInitialized;
};

#endif // LBWSCREEN_H
