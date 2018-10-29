#include <QDebug>
#include "lbaspriteplayer.h"
#include "ui_lbaspriteplayer.h"
#include <lbasprite.h>
#include <lbapalette.h>

//---------------------------------------------------------------------------------------------------------------------
LbaSpritePlayer::LbaSpritePlayer(const LbaRess &ress, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LbaSpritePlayer),
    mLbaRess(ress)
{
    ui->setupUi(this);
    connect(ui->spbIndex, SIGNAL(valueChanged(int)), this, SLOT(showSprite()));

    showSprite();
}

//---------------------------------------------------------------------------------------------------------------------
LbaSpritePlayer::~LbaSpritePlayer()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void LbaSpritePlayer::showSprite()
{
    int index = ui->spbIndex->value();
    if (index < 0 || index >= mLbaRess.count(LbaRess::LBA1,LbaRess::Sprites))
        return;
    LbaPalette pal(mLbaRess.data(LbaRess::LBA1,LbaRess::Ress,0));

    LbaSprite sprite(pal,mLbaRess.data(LbaRess::LBA1,LbaRess::Sprites,index));

    QImage img = sprite.image();

    if (img.width() < 200)
        img = img.scaled(QSize(200,200),Qt::KeepAspectRatio);
    ui->lblSprite->setPixmap(QPixmap::fromImage(img));
}
