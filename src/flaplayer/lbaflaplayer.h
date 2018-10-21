#ifndef LBAFLAPLAYER_H
#define LBAFLAPLAYER_H

#include <QWidget>
#include <lbaress.h>
#include <flamovie.h>

namespace Ui {
class LbaFlaPlayer;
}

class LbaFlaPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit LbaFlaPlayer(const LbaRess &ress, QWidget *parent = 0);
    ~LbaFlaPlayer();

    void initFromCli();

signals:
    void playSample(int,int);

private slots:
    void playRess(QString name);
    void openFla();
    void flaLoaded();

private:
    Ui::LbaFlaPlayer *ui;
    const LbaRess    &mLbaRess;
    FlaMovie          mMovie;
};

#endif // LBAFLAPLAYER_H
