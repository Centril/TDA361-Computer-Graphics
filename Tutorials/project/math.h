#ifndef MATH_H_GUARD
#define MATH_H_GUARD

#include <float3x3.h>
using namespace chag;

// Helper function to turn spherical coordinates into cartesian (x,y,z)
chag::float3 sphericalToCartesian( float theta, float phi, float r );

#endif