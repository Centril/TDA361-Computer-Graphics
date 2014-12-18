#ifndef GFXUTILS_H_GUARD
#define GFXUTILS_H_GUARD

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

GLint gfxCurrentProgram();
GLint gfxUseProgram( GLint program );

void gfxTextureFilter( GLint how );
void gfxClamp( GLint how );
void gfxNearest();
void gfxLinear();
void gfxClampBorder();
void gfxClampEdge();

void gfxClear( float4 color );

int gfxViewportWidth();
int gfxViewportHeight();
void gfxViewport( int w, int h );

#endif