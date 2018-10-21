#ifndef LBAMOVIEWIDGET_H
#define LBAMOVIEWIDGET_H

#include <QWidget>
#include <flamovie.h>

class LbaMovieWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LbaMovieWidget(QWidget *parent = 0);

    void play(FlaMovie *movie);

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    FlaMovie *mMovie;
};

#endif // LBAMOVIEWIDGET_H
