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

void gfxTextureCMBind( GLuint texture, GLint slot, const char* uniform );
void gfxTexture2DBind( GLuint texture, GLint slot, const char* uniform );
void gfxTextureBind( GLenum mode, GLuint texture, GLint slot, const char* uniform );

void gfxTextureFilter( GLint how );
void gfxClamp( GLint how );
void gfxNearest();
void gfxLinear();
void gfxClampBorder();
void gfxClampEdge();

void gfxObjectAlpha( float alpha );
void gfxObjectReflectiveness( float v );

void gfxClear( float4 color );

int gfxViewportWidth();
int gfxViewportHeight();
void gfxViewport( int w, int h );

#endif