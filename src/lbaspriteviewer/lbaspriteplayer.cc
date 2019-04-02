#include <QDebug>
#include "lbaspriteplayer.h"
#include "ui_lbaspriteplayer.h"
#include <lbasprite.h>
#include <lbaimage.h>
#include <lbapalette.h>

//---------------------------------------------------------------------------------------------------------------------
LbaSpritePlayer::LbaSpritePlayer(const LbaRess &ress, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LbaSpritePlayer),
    mLbaRess(ress)
{
    ui->setupUi(this);
    connect(ui->spbIndex, SIGNAL(valueChanged(int)), this, SLOT(showSprite()));
    connect(ui->spbPalette, SIGNAL(valueChanged(int)), this, SLOT(showSprite()));
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
    LbaRess::Source source   = ui->btnLba1->isChecked() ? LbaRess::LBA1 : ui->btnLba2->isChecked() ? LbaRess::LBA2 : LbaRess::LBAW;
    LbaRess::Content content = ui->btnSprites->isChecked() ? LbaRess::Sprites : LbaRess::SpritesRaw;
    content = ui->btnRess->isChecked() ? LbaRess::Ress : content;

    ui->lblCount->setText(QString("%1").arg(mLbaRess.count(source,content)));

    if (index < 0 || index >= mLbaRess.count(source,content))
        return;
    LbaPalette pal(mLbaRess.data(source,LbaRess::Ress,ui->spbPalette->value()));

    LbaSprite::Type spriteType;

    switch(content) {
    case LbaRess::Sprites:    spriteType = LbaSprite::Sprite;    break;
    case LbaRess::SpritesRaw: spriteType = LbaSprite::RawSprite; break;
    case LbaRess::Ress:       spriteType = LbaSprite::Image;     break;
    }

    LbaSprite sprite(pal,mLbaRess.data(source,content,index), spriteType);

    QImage img = sprite.image();

    ui->lblResolution->setText(QString("%1x%2").arg(img.rect().width()).arg(img.rect().height()));
    if (img.width() < 200)
        img = img.scaled(QSize(200,200),Qt::KeepAspectRatio);
    ui->lblSprite->setPixmap(QPixmap::fromImage(img));
}
