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

void gfxTextureFilter( GLenum type, GLint how );
void gfxTextureFilter( GLint how );
void gfxClamp( GLenum type, GLint how );
void gfxClamp( GLint how );
void gfxNearest( GLenum type );
void gfxLinear( GLenum type );
void gfxNearest();
void gfxLinear();
void gfxClampBorder( GLenum type );
void gfxClampEdge( GLenum type );
void gfxClampBorder();
void gfxClampEdge();

void gfxObjectAlpha( float alpha );
void gfxObjectReflectiveness( float v );

void gfxClear( float4 color );

int gfxViewportWidth();
int gfxViewportHeight();
void gfxViewport( int w, int h );

class fbo_info {
public:
	GLuint id;
	GLuint colorBuffer;
	GLuint depthBuffer;
	int width, height;

	fbo_info() {}

	fbo_info( GLuint _colorBuffer, GLuint _depthBuffer, int _width, int _height ) :
		fbo_info( 0, _colorBuffer, _depthBuffer, _width, _height ) {
		generate();
	}

	fbo_info( GLuint _id, GLuint _colorBuffer, GLuint _depthBuffer, int _width, int _height ) :
		id( _id ), colorBuffer( _colorBuffer ), depthBuffer( _depthBuffer ), width( _width ), height( _height ) {
	}

	void bind() {
		glBindFramebuffer( GL_FRAMEBUFFER, id );		
	}

	void generate() {
		glGenFramebuffers( 1, &id );
		bind();
	}

	void tex_buffer() {
		glGenTextures( 1, &colorBuffer );
	}

	void depth_buffer() {
		glGenRenderbuffers( 1, &depthBuffer );
		glBindRenderbuffer( GL_RENDERBUFFER, depthBuffer );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
	}

	void bind_depth_buffer() {
		// Associate our created depth buffer with the FBO.
		glBindFramebuffer( GL_FRAMEBUFFER, id );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer );
	}

	void check_complete() {
		glBindFramebuffer( GL_FRAMEBUFFER, id );
		GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		if ( status != GL_FRAMEBUFFER_COMPLETE ) {
			fatal_error("Framebuffer not complete");
		}
	}

	void restore_default() {
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
};

#endif