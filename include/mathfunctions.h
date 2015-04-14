#ifndef MATHFUNCTIONS_H
#define MATHFUNCTIONS_H

#include "ngl/Vec3.h"
#include <cmath>

// http://stackoverflow.com/questions/3451553/value-remapping
inline float map(float _value, float low1, float high1, float low2, float high2)
{
    return low2 + (_value - low1) * (high2 - low2) / (high1 - low1);
}

float angleBetween(ngl::Vec3 _v1, ngl::Vec3 _v2)
{
    float dot = _v1.dot(_v2);
    float theta = (float)acos(dot / (_v1.length() * _v2.length()));
    return theta;
}

#endif // MATHFUNCTIONS_H
