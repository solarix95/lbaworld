#ifndef LBAMISSIONCONTROL_H
#define LBAMISSIONCONTROL_H

#include <QMap>
#include <QObject>
#include <QTimer>
#include <QList>

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

signals:
    void log(const QString &output);

public slots:
    void init();
    void exec(const QString &cmd, const QStringList &args);

private slots:
    void sleepEnd();
    void processCmdQueue();
    void processCmd(const QString &cmd, const QStringList &args);

private:
    Q_INVOKABLE void help(const QStringList &args);
    Q_INVOKABLE void sleep(const QStringList &args);
    Q_INVOKABLE void playspl(const QStringList &args);
    Q_INVOKABLE void playmus(const QStringList &args);
    Q_INVOKABLE void playfla(const QStringList &args);
    Q_INVOKABLE void quit(const QStringList &args);
    Q_INVOKABLE void showposter(const QStringList &args);
    Q_INVOKABLE void showstatus(const QStringList &args);
    Q_INVOKABLE void setwindowrect(const QStringList &args);
    Q_INVOKABLE void printfla(const QStringList &args);

    void setupHelp();

    LbaRess   &mRess;
    LbwAudio  *mAudio;
    LbwScreen *mScreen;

    QMap<QString,QString> mCommandHelp;

    enum CommandState {
        Execute,
        Sleeping,
        Recording // Macro recording
    };

    CommandState                        mCmdState;
    QTimer                              mSleepTimer;
    QList<QPair<QString, QStringList> > mCmdQueue;
};

#endif // LBAMISSIONCONTROL_H
