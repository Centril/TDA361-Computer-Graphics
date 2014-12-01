#ifndef GFX_H_GUARD
#define GFX_H_GUARD

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <IL/il.h>
#include <IL/ilut.h>

#include <stdlib.h>
#include <algorithm>

#include <OBJModel.h>
#include <glutil.h>
#include <float4x4.h>
#include <float3x3.h>

#include "math.h"

#ifdef WIN32
#define SCENES "../scenes"
#else
#define SCENES "../../../scenes"
#endif

using namespace std;
using namespace chag;

void gfxInit();

void gfxCurrentTime( float currtime );

void gfxIdle();

void gfxDisplay();

template<typename T>
struct position_change {
	T x, y;
	T dx, dy;

	position_change() :
		x( 0 ), y( 0 ), dx ( 0 ), dy( 0 )
		{}

	void set( T x, T y ) {
		this->x = x;
		this->y = y;
	}

	void delta( T x, T y ) {
		this->dx = x;
		this->dy = y;
	}
};

typedef struct position_change<int> mouse_change;

void gfxMiddleDown( mouse_change& change );

void gfxLeftDown( mouse_change& change );

void gfxRightDown( mouse_change& change );

#endif