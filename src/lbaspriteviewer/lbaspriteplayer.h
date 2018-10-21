#ifndef LBASPRITEPLAYER_H
#define LBASPRITEPLAYER_H

#include <QWidget>
#include <lbaress.h>

namespace Ui {
class LbaSpritePlayer;
}

class LbaSpritePlayer : public QWidget
{
    Q_OBJECT

public:
    explicit LbaSpritePlayer(const LbaRess &ress, QWidget *parent = 0);
    ~LbaSpritePlayer();

    void initFromCli();

private slots:
    void showSprite();

private:
    Ui::LbaSpritePlayer *ui;
    const LbaRess    &mLbaRess;
};

#endif
