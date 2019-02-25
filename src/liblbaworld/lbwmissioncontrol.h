#ifndef LBAMISSIONCONTROL_H
#define LBAMISSIONCONTROL_H

#include <QObject>

class LbwAudio;
class LbwScreen;
class LbaRess;
class LbwMissionControl : public QObject
{
    Q_OBJECT
public:
    LbwMissionControl(LbaRess &ress);
    virtual ~LbwMissionControl();

    void registerAudio(LbwAudio *audioSystem);
    void registerScreen(LbwScreen *screen);

public slots:
    void init();
    void exec(const QString &cmd, const QStringList &args);

private:
    Q_INVOKABLE void playspl(const QStringList &args);
    Q_INVOKABLE void playmus(const QStringList &args);
    Q_INVOKABLE void quit(const QStringList &args);
    Q_INVOKABLE void showposter(const QStringList &args);

    LbaRess   &mRess;
    LbwAudio  *mAudio;
    LbwScreen *mScreen;
};

#endif // LBAMISSIONCONTROL_H
