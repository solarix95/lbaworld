#ifndef LBW2DLAYER_H
#define LBW2DLAYER_H

#include <QObject>
#include <QPainter>
#include <QRect>
#include <QKeyEvent>

class Lbw2dLayer : public QObject
{
    Q_OBJECT
public:
    Lbw2dLayer();

    virtual void render(const QRect &window, QPainter &p) = 0;
    virtual bool keyEvent(QKeyEvent *ke);
    virtual bool processState(float speed);

signals:
    void updateRequest();
};

#endif // LBW2DLAYER_H
