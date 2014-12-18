#include "gfxutils.h"

GLint gfxCurrentProgram() {
	GLint currentProgram;
	glGetIntegerv( GL_CURRENT_PROGRAM, &currentProgram );
	return currentProgram;
}

GLint gfxUseProgram( GLint program ) {
	GLint current = gfxCurrentProgram();
	glUseProgram( program );
	return current;
}

void gfxTextureFilter( GLint how ) {
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, how );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, how );
}

void gfxClamp( GLint how ) {
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, how );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, how );	
}

void gfxNearest() {
	gfxTextureFilter( GL_NEAREST );
}

void gfxLinear() {
	gfxTextureFilter( GL_LINEAR );
}

void gfxClampBorder() {
	gfxClamp( GL_CLAMP_TO_BORDER );
}

void gfxClampEdge() {
	gfxClamp( GL_CLAMP_TO_EDGE );
}

void gfxClear( float4 color ) {
	glClearColor( color.x, color.y, color.z, color.w );
	glClearDepth( 1 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

int gfxViewportWidth() {
	return glutGet( (GLenum) GLUT_WINDOW_WIDTH );
}

int gfxViewportHeight() {
	return glutGet( (GLenum) GLUT_WINDOW_HEIGHT );
}

void gfxViewport( int w, int h ) {
	glViewport( 0, 0, w, h );
}
