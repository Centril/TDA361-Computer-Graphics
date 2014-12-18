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

void gfxTextureCMBind( GLuint texture, GLint slot, const char* uniform ) {
	gfxTextureBind( GL_TEXTURE_CUBE_MAP, texture, slot, uniform );
}

void gfxTexture2DBind( GLuint texture, GLint slot, const char* uniform ) {
	gfxTextureBind( GL_TEXTURE_2D, texture, slot, uniform );
}

void gfxTextureBind( GLenum mode, GLuint texture, GLint slot, const char* uniform ) {
	setUniformSlow( gfxCurrentProgram(), uniform, slot );
	glActiveTexture( GL_TEXTURE0 + slot );
	glBindTexture( mode, texture );
}

void gfxTextureFilter( GLenum type, GLint how ) {
	glTexParameteri( type, GL_TEXTURE_MIN_FILTER, how );
	glTexParameteri( type, GL_TEXTURE_MAG_FILTER, how );
}

void gfxTextureFilter( GLint how ) {
	gfxTextureFilter( GL_TEXTURE_2D, how );
}

void gfxClamp( GLenum type, GLint how ) {
	glTexParameteri( type, GL_TEXTURE_WRAP_S, how );
	glTexParameteri( type, GL_TEXTURE_WRAP_T, how );	
}

void gfxClamp( GLint how ) {
	gfxClamp( GL_TEXTURE_2D, how );
}

void gfxNearest( GLenum type ) {
	gfxTextureFilter( type, GL_NEAREST );
}

void gfxLinear( GLenum type ) {
	gfxTextureFilter( type, GL_LINEAR );
}

void gfxNearest() {
	gfxTextureFilter( GL_NEAREST );
}

void gfxLinear() {
	gfxTextureFilter( GL_LINEAR );
}

void gfxClampBorder( GLenum type ) {
	gfxClamp( type, GL_CLAMP_TO_BORDER );
}

void gfxClampEdge( GLenum type ) {
	gfxClamp( type, GL_CLAMP_TO_EDGE );
}

void gfxClampBorder() {
	gfxClamp( GL_CLAMP_TO_BORDER );
}

void gfxClampEdge() {
	gfxClamp( GL_CLAMP_TO_EDGE );
}

void gfxObjectAlpha( float alpha ) {
	setUniformSlow( gfxCurrentProgram(), "object_alpha", alpha ); 
}

void gfxObjectReflectiveness( float v ) {
	setUniformSlow( gfxCurrentProgram(), "object_reflectiveness", v );
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
