#ifndef LBWSCREEN_H
#define LBWSCREEN_H

#include "lbw3dwidget.h"

class FlaMovie;
class Lbw2dLayer;
class LbwConsole;
class LbwScreen : public Lbw3dWidget
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
    void requestFadeTo(const QImage &img);
    void requestFadeTo(FlaMovie *movie);

protected:
    virtual void initWidget() override;
    virtual void paint2dLayers() override;
    virtual bool event(QEvent *e) override;
    virtual bool processState(float speed) override;

private:
    void toggleFullscreen();

    QString            mStatusText;
    QList<Lbw2dLayer*> m2dLayers;
    LbwConsole        *mConsole;
};

#endif // LBWSCREEN_H
