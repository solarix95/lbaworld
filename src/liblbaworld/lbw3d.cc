#include <math.h>
#include "lbw3d.h"

//-------------------------------------------------------------------------------------------------
void LbwVector::normalize()
{
    float sum = sqrt((x*x) + (y*y) + (z*z));
    if (sum > 0) {
        x /= sum;
        y /= sum;
        z /= sum;
    }
}
