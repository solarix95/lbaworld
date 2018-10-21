#include <QDebug>
#include "lbaflaplayer.h"
#include "ui_lbaflaplayer.h"
#include <QFileDialog>
#include "flamovie.h"

//---------------------------------------------------------------------------------------------------------------------
LbaFlaPlayer::LbaFlaPlayer(const LbaRess &ress, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LbaFlaPlayer),
    mLbaRess(ress)
{
    ui->setupUi(this);

    connect(ui->btnLoad, SIGNAL(clicked(bool)), this, SLOT(openFla()));
    connect(ui->sldFrame, SIGNAL(sliderMoved(int)), &mMovie, SLOT(requestFrame(int)));
    connect(ui->btnPlay, SIGNAL(clicked(bool)), &mMovie, SLOT(start()));
    connect(ui->btnPause, SIGNAL(clicked(bool)), &mMovie, SLOT(stop()));
    connect(ui->cbxFlas, SIGNAL(activated(QString)), this, SLOT(playRess(QString)));
    connect(&mMovie, SIGNAL(playFrame(int)), ui->sldFrame, SLOT(setValue(int)));
    connect(&mMovie, SIGNAL(loaded()), this, SLOT(flaLoaded()));
    connect(&mMovie, SIGNAL(playFlaSample(int,int)), this, SIGNAL(playSample(int,int)));
    connect(ui->spbFps, SIGNAL(valueChanged(int)), &mMovie, SLOT(requestFps(int)));
    ui->wdgMovie->play(&mMovie);


    foreach(QString next, mLbaRess.flas())
        ui->cbxFlas->addItem(next);

}

//---------------------------------------------------------------------------------------------------------------------
LbaFlaPlayer::~LbaFlaPlayer()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void LbaFlaPlayer::initFromCli()
{
    foreach(QString arg, qApp->arguments()) {
        if (arg == "--maximized")
            setWindowState(Qt::WindowMaximized);
        if (arg == "--fullscreen")
            setWindowState(Qt::WindowFullScreen);
        if (QFile::exists(arg) && arg.toLower().endsWith(".fla")) {
            mMovie.fromFile(arg);
            mMovie.start();
        }
        if (arg == "--close")
            connect(&mMovie, SIGNAL(finished()), this, SLOT(close()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void LbaFlaPlayer::playRess(QString name)
{
    mMovie.fromBuffer(mLbaRess.fla(name));
}

//---------------------------------------------------------------------------------------------------------------------
void LbaFlaPlayer::openFla()
{
    QString flafile = QFileDialog::getOpenFileName(this, "Select FLA","",tr("LBA Movie (*.fla *.FLA)"));
    if (flafile.isEmpty())
        return;

    mMovie.fromFile(flafile);
}

//---------------------------------------------------------------------------------------------------------------------
void LbaFlaPlayer::flaLoaded()
{
    ui->sldFrame->setMaximum(mMovie.frameCount());
    ui->sldFrame->setValue(0);
    ui->spbFps->setValue(mMovie.fps());
   //  ui->spbFps;
}
