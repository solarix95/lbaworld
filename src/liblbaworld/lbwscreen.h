#ifndef LBWSCREEN_H
#define LBWSCREEN_H

#include "lbw3dwidget.h"

class Lbw2dLayer;
class LbwScreen : public Lbw3dWidget
{
    Q_OBJECT
public:
    LbwScreen(QWidget *parent = 0);
    virtual ~LbwScreen();

public slots:
    void setStatusText(const QString &statustext);
    void fadeTo(const QImage &img);

signals:
    void requestFadeTo(const QImage &img);

protected:
    virtual void initWidget() override;
    virtual void paint2dLayers() override;
    virtual bool event(QEvent *e) override;
    virtual bool processState(float speed) override;

private:
    void toggleFullscreen();

    QString            mStatusText;
    QList<Lbw2dLayer*> m2dLayers;
};

#endif // LBWSCREEN_H
