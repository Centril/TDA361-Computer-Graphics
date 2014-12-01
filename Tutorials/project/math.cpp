#include "math.h"

#include <stdlib.h>
#include <algorithm>

#include <float4x4.h>
using namespace chag;

// Helper function to turn spherical coordinates into cartesian (x,y,z)
float3 sphericalToCartesian( float theta, float phi, float r ) {
	return make_vector( r * sinf( theta ) * sinf( phi ),
					 	r * cosf( phi ), 
						r * cosf( theta ) * sinf( phi ) );
}