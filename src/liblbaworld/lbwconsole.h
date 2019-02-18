#ifndef LBWCONSOLE_H
#define LBWCONSOLE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QObject>

class LbwConsole : public QObject
{
    Q_OBJECT
public:
    LbwConsole();
    virtual ~LbwConsole();

    void exec(const QString &expr);

    int     stackSize() const;
    QString stackEntry(int i) const;

    inline int     logCount() const      { return mLogs.count(); }
    inline QString logEntry(int i) const { return mLogs[i];      }

public slots:
    void addOutout(const QString &logmsg);

private:
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

    QList<Expr>    mExprStack;
    QList<QString> mLogs;
};

#endif // LBWCONSOLE_H
