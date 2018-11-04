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
    connect(ui->btnLba1, SIGNAL(clicked(bool)), this, SLOT(showSprite()));
    connect(ui->btnLba2, SIGNAL(clicked(bool)), this, SLOT(showSprite()));
    connect(ui->btnLbaw, SIGNAL(clicked(bool)), this, SLOT(showSprite()));
    connect(ui->btnSprites, SIGNAL(clicked(bool)), this, SLOT(showSprite()));
    connect(ui->btnSpritesRaw, SIGNAL(clicked(bool)), this, SLOT(showSprite()));
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
    LbaRess::Source source   = ui->btnLba1->isChecked() ? LbaRess::LBA1 : LbaRess::LBA2;
    LbaRess::Content content = ui->btnSprites->isChecked() ? LbaRess::Sprites : LbaRess::SpritesRaw;

    ui->lblCount->setText(QString("%1").arg(mLbaRess.count(source,content)));

    if (index < 0 || index >= mLbaRess.count(source,content))
        return;
    LbaPalette pal(mLbaRess.data(source,LbaRess::Ress,0));

    LbaSprite sprite(pal,mLbaRess.data(source,content,index), content == LbaRess::SpritesRaw);

    QImage img = sprite.image();

    if (img.width() < 200)
        img = img.scaled(QSize(200,200),Qt::KeepAspectRatio);
    ui->lblSprite->setPixmap(QPixmap::fromImage(img));
}
