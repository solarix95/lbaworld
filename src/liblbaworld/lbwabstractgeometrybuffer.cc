#include "lbwabstractgeometrybuffer.h"

LbwAbstractGeometryBuffer::LbwAbstractGeometryBuffer(const QString &url)
 : mUrl(url)
{
}

LbwAbstractGeometryBuffer::~LbwAbstractGeometryBuffer()
{

}

//-------------------------------------------------------------------------------------------------
QString LbwAbstractGeometryBuffer::url() const
{
    return mUrl;
}

//-------------------------------------------------------------------------------------------------
void LbwAbstractGeometryBuffer::registerState(LbwAbstractGeometryState *state)
{
    mStates << state;
}

//-------------------------------------------------------------------------------------------------
void LbwAbstractGeometryBuffer::unregisterState(LbwAbstractGeometryState *state)
{
    mStates.removeAll(state);
}

//-------------------------------------------------------------------------------------------------
const LbwAbstractGeometryStates &LbwAbstractGeometryBuffer::states() const
{
    return mStates;
}
