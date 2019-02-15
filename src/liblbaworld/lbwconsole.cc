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
int LbwConsole::stackSize() const
{
    return mExprStack.size();
}

//-------------------------------------------------------------------------------------------------
QString LbwConsole::stackEntry(int i) const
{
    Q_ASSERT(i >= 0);
    Q_ASSERT(i < mExprStack.size());
    return mExprStack[i].expression;
}

//-------------------------------------------------------------------------------------------------
void LbwConsole::addOutout(const QString &logmsg)
{
    mLogs << logmsg;
}
