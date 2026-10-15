#ifndef LBWSCREEN_H
#define LBWSCREEN_H

#include <libqtr3d/qtr3dwidget.h>

class FlaMovie;
class QImage;
class Lbw2dLayer;
class LbwConsole;
class LbwScreen : public Qtr3dWidget
{
    Q_OBJECT
public:
    LbwScreen(LbwConsole *c, QWidget *parent = 0);
    virtual ~LbwScreen();

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
    bool processState(float speed);
    void toggleFullscreen();

    QString            mStatusText;
    QList<Lbw2dLayer*> m2dLayers;
    LbwConsole        *mConsole;
    bool               mInitialized;
};

#endif // LBWSCREEN_H
