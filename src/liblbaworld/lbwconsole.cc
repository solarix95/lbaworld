#include <QDebug>
#include <QApplication>
#include "lbwconsole.h"

//-------------------------------------------------------------------------------------------------
LbwConsole::LbwConsole()
{
}

//-------------------------------------------------------------------------------------------------
LbwConsole::~LbwConsole()
{
}

//-------------------------------------------------------------------------------------------------
void LbwConsole::exec(const QString &expr)
{
    if (expr.trimmed().isEmpty())
        return;
    mExprStack << expr;
    mStackCursor = mExprStack.count()-1;
    run(split(expr));
}

//-------------------------------------------------------------------------------------------------
int LbwConsole::stackSize() const
{
    return mExprStack.size();
}

//-------------------------------------------------------------------------------------------------
QString LbwConsole::stackEntry(int i) const
{
    Q_ASSERT(i >= 0);
    Q_ASSERT(i < mExprStack.size());
    return mExprStack[i];
}

//-------------------------------------------------------------------------------------------------
QString LbwConsole::cursor() const
{
    if (mExprStack.isEmpty())
        return "";
    if (mStackCursor < 0)
        return mExprStack.first();
    if (mStackCursor >= mExprStack.count())
        return mExprStack.last();
    return mExprStack[mStackCursor];
}

//-------------------------------------------------------------------------------------------------
void LbwConsole::incCursor(int dir)
{
    mStackCursor += (dir < 0 ? -1 : 1);

    if (mStackCursor < 0)
        mStackCursor = 0;
    if (mStackCursor >= mExprStack.count())
        mStackCursor = mExprStack.count()-1;
}

//-------------------------------------------------------------------------------------------------
void LbwConsole::init()
{
    foreach(QString arg, QApplication::arguments()) {
        if (arg.startsWith("--exec=")) {
            exec(arg.split("=").last());
        }
    }
}

//-------------------------------------------------------------------------------------------------
void LbwConsole::addOutput(const QString &logmsg)
{
    if (logmsg.startsWith("#") && logmsg.length() > 7) {
        QString color = logmsg.mid(0,7); // red -> #ff0000
        QString msg   = logmsg.mid(8);
        mLogs << Log(msg.trimmed(),color);
    } else
        mLogs << Log(logmsg, Qt::white);
}

//-------------------------------------------------------------------------------------------------
void LbwConsole::run(const QStringList &parts)
{
    if (parts.isEmpty())
        return;
    mLogs << Log("] " + parts.join(' '),Qt::white);
    QStringList args = parts;
    QString     cmd  = args.takeFirst();
    emit requestCmd(cmd, args);
}

//-------------------------------------------------------------------------------------------------
QStringList LbwConsole::split(const QString &input)
/*
  '123 456 78'  -> '123' + '456' '78'
  '123 "456 78" -> '123' + '456 78'
*/

{
    QStringList ret;

    enum ParserState {
        Undef,
        InPlaintString,
        InEscapedString
    };

    ParserState state = Undef;
    int pos = 0;
    QString nextToken;
    while (pos < input.length()) {
        QChar next = input[pos];
        switch (state) {
        case Undef:
            if (next == '\"')
                state = InEscapedString;
            else if (next.isSpace()); // skip spaces
            else {
                nextToken += next;
                state = InPlaintString;
            }
            break;
        case InPlaintString:
            if (next.isSpace()) {
                ret << nextToken;
                nextToken.clear();
                state = Undef;
            }
            else nextToken += next;
            break;
        case InEscapedString: {
            if (next == '\\') {
                QChar nextNext = pos < input.length()-1 ? input[pos+1] : QChar();

                if (nextNext == '\"') { // x\"x -> x"x
                    nextToken += "\"";
                    pos++; // skip two characters total
                } else if (nextNext == '\\'){
                    nextToken += "\\";
                    pos++; // skip two characters total
                } else {
                    // Unknown escape Sequence
                }
            } else if (next == '\"') {
                ret << nextToken;
                nextToken.clear();
                state = Undef;
            } else
                nextToken += next;
        } break;
        default:
            Q_ASSERT(0 && "Never ever reach this point");
        }
        pos++;
    }
    if (!nextToken.isEmpty())
        ret << nextToken;
    return ret;
}
