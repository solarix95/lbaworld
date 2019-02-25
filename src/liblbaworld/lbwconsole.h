#ifndef LBWCONSOLE_H
#define LBWCONSOLE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QObject>
#include <QColor>

class LbwConsole : public QObject
{
    Q_OBJECT
public:
    LbwConsole();
    virtual ~LbwConsole();

    void exec(const QString &expr);

    int     stackSize() const;
    QString stackEntry(int i) const;
    QString cursor() const;
    void    incCursor(int dir);

    inline int     logCount() const      { return mLogs.count();  }
    inline QString logEntry(int i) const { return mLogs[i].msg;   }
    inline QColor  logColor(int i) const { return mLogs[i].color; }

signals:
    void requestCmd(const QString &cmd, const QStringList &args);

public slots:
    void init();
    void addOutput(const QString &logmsg);

private:
    void run(const QStringList &parts);
    static QStringList split(const QString &input);

    enum ExprState {
        Active = 0,
        Done   = 1,
        Error  = 2
    };

    struct Expr {
        QString   expression;
        ExprState state;
        QString   ret;
    };

    struct Log {
        QString msg;
        QColor  color;
        Log(const QString &s, QColor c): msg(s), color(c) {}
    };

    QList<QString> mExprStack;
    QList<Log>     mLogs;
    int            mStackCursor;
};

#endif // LBWCONSOLE_H
