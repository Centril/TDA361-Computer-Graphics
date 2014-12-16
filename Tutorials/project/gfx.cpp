#include "gfx.h"

using namespace std;
using namespace chag;

//*****************************************************************************
//	Global variables
//*****************************************************************************
float currentTime = 0.0f;		// Tells us the current time.
GLuint shaderProgram, shadowShaderProgram;
const float3 up = {0.0f, 1.0f, 0.0f};

//*****************************************************************************
//	Shadow map
//*****************************************************************************
GLuint shadowMapTexture;
GLuint shadowMapFBO;
const int shadowMapResolution = 2048;

//*****************************************************************************
//	Background clear color:
//*****************************************************************************
const float3 clear_color = {0.2, 0.2, 0.8};

//*****************************************************************************
//	OBJ Model declarations
//*****************************************************************************
OBJModel* world; 
OBJModel* water; 
OBJModel* skybox; 
OBJModel* skyboxnight; 
OBJModel* car; 

//*****************************************************************************
//	Camera state variables (updated in motion())
//*****************************************************************************
float camera_theta = M_PI / 6.0f;
float camera_phi = M_PI / 4.0f;
float camera_r = 30.0; 
float camera_target_altitude = 5.2; 

//*****************************************************************************
//	Light state variables (updated in idle())
//*****************************************************************************
float3 lightPosition = {30.1f, 450.0f, 0.1f};

void gfxClampEdge() {
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void gfxLoadShaders() {
	shaderProgram = loadShaderProgram( "shading.vert", "shading.frag" );
	glBindAttribLocation( shaderProgram, 0, "position" );
	glBindAttribLocation( shaderProgram, 2, "texCoordIn" );
	glBindAttribLocation( shaderProgram, 1, "normalIn" );
	glBindFragDataLocation( shaderProgram, 0, "fragmentColor" );
	linkShaderProgram( shaderProgram );	

	shadowShaderProgram = loadShaderProgram("shadow.vert", "shadow.frag");
	glBindAttribLocation(shadowShaderProgram, 0, "position");
	glBindFragDataLocation(shadowShaderProgram, 0, "fragmentColor");
	linkShaderProgram(shadowShaderProgram);
}

void gfxLoadModels() {
	world = new OBJModel();
	world->load( SCENES "/island_blender_new.obj" );

	skybox = new OBJModel();
	skybox->load( SCENES "/skybox.obj" );

	skyboxnight = new OBJModel();
	skyboxnight->load( SCENES "/skyboxnight.obj" );

	// Make the textures of the skyboxes use clamp to edge to avoid seams
	for ( int i = 0; i < 6; i++ ) {
		glBindTexture( GL_TEXTURE_2D, skybox->getDiffuseTexture( i ) );
		gfxClampEdge();

		glBindTexture( GL_TEXTURE_2D, skyboxnight->getDiffuseTexture( i ) );
		gfxClampEdge();
	}

	water = new OBJModel(); 
	water->load( SCENES "/water.obj" );

	car = new OBJModel(); 
	car->load( SCENES "/car.obj" );
}

void gfxInitShadowMap() {
	// Generate and bind our shadow map texture
	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	// Specify the shadow map texture’s format: GL_DEPTH_COMPONENT[32] is
	// for depth buffers/textures.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		shadowMapResolution, shadowMapResolution, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, 0
		);
	// We need to setup these; otherwise the texture is illegal as a
	// render target.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Cleanup: unbind the texture again - we’re finished with it for now
	glBindTexture(GL_TEXTURE_2D, 0);
	// Generate and bind our shadow map frame buffer
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	// Bind the depth texture we just created to the FBO’s depth attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, shadowMapTexture, 0);
	// We’re rendering depth only, so make sure we’re not trying to access
	// the color buffer by setting glDrawBuffer() and glReadBuffer() to GL_NONE
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// Cleanup: activate the default frame buffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gfxInit() {
	/* Initialize GLEW; this gives us access to OpenGL Extensions.
	 */
	glewInit();

	/* Print information about OpenGL and ensure that we've got at a context 
	 * that supports least OpenGL 3.0. Then setup the OpenGL Debug message
	 * mechanism.
	 */
	startupGLDiagnostics();
	setupGLDebugMessages();

	/* Initialize DevIL, the image library that we use to load textures. Also
	 * tell IL that we intent to use it with OpenGL.
	 */
	ilInit();
	ilutRenderer(ILUT_OPENGL);

	/* Workaround for AMD. It might no longer be necessary, but I dunno if we
	 * are ever going to remove it. (Consider it a piece of living history.)
	 */
	if( !glBindFragDataLocation ) {
		glBindFragDataLocation = glBindFragDataLocationEXT;
	}

	/* As a general rule, you shouldn't need to change anything before this 
	 * comment in initGL().
	 */

	//*************************************************************************
	//	Load shaders
	//*************************************************************************
	gfxLoadShaders();

	//*************************************************************************
	// Load the models from disk
	//*************************************************************************
	gfxLoadModels();

	gfxInitShadowMap();
}

void drawScene();

void drawShadowCasters();

void drawShadowMap(const float4x4 &viewMatrix, const float4x4 &projectionMatrix)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.5, 10);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glViewport(0, 0, shadowMapResolution, shadowMapResolution);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	// Get current shader, so we can restore it afterwards. Also, switch to
	// the shadow shader used to draw the shadow map.
	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	glUseProgram(shadowShaderProgram);

	// draw shadow casters
	drawShadowCasters();

	// Restore old shader
	glUseProgram(currentProgram);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_POLYGON_OFFSET_FILL);
}

void gfxDisplay() {
	drawScene();
	glutSwapBuffers();  // swap front and back buffer. This frame will now be displayed.
}

void drawModel(OBJModel *model, const float4x4 &modelMatrix) {
	setUniformSlow(shaderProgram, "modelMatrix", modelMatrix); 
	model->render();
}

/**
* In this function, add all scene elements that should cast shadow, that way
* there is only one draw call to each of these, as this function is called twice.
*/
void drawShadowCasters() {
	drawModel(world, make_identity<float4x4>());
	setUniformSlow(shaderProgram, "object_reflectiveness", 0.5f); 

	drawModel(car, make_translation(make_vector(0.0f, 0.0f, 0.0f))); 
	setUniformSlow(shaderProgram, "object_reflectiveness", 0.0f); 
}

void gfxClear() {
	glClearColor( clear_color.x, clear_color.y, clear_color.z, 1.0 );
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

void gfxSetupMatrices( int w, int h ) {
	float3 camera_position = sphericalToCartesian( camera_theta, camera_phi, camera_r );
	float3 camera_lookAt = make_vector( 0.0f, camera_target_altitude, 0.0f );
	float3 camera_up = make_vector( 0.0f, 1.0f, 0.0f );
	float4x4 viewMatrix = lookAt( camera_position, camera_lookAt, camera_up );
	float4x4 projectionMatrix = perspectiveMatrix( 45.0f, float(w) / float(h), 0.1f, 1000.0f );
	setUniformSlow( shaderProgram, "viewMatrix", viewMatrix );
	setUniformSlow( shaderProgram, "projectionMatrix", projectionMatrix );
	setUniformSlow( shaderProgram, "lightpos", lightPosition );
	setUniformSlow( shaderProgram, "inverseViewNormalMatrix", transpose( viewMatrix ) );
}

void gfxObjectAlpha( float alpha ) {
	setUniformSlow( shaderProgram, "object_alpha", alpha ); 
}

void drawScene(void) {
	// enable Z-buffering.
	glEnable(GL_DEPTH_TEST);
	// enable back face culling.
	glEnable(GL_CULL_FACE);	

	//*******************************************************************************
	// START: Render the scene from the cameras viewpoint, to the default framebuffer
	//*******************************************************************************
	gfxClear();
	int w = gfxViewportWidth();
	int h = gfxViewportHeight();
	gfxViewport( w, h );

	// Use shader and set up uniforms
	glUseProgram( shaderProgram );
	gfxSetupMatrices( w, h );

	drawModel(water, make_translation(make_vector(0.0f, -6.0f, 0.0f)));
	drawShadowCasters();

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const float4x4 modelMatrix = make_identity<float4x4>();
	drawModel( skyboxnight, modelMatrix );

	gfxObjectAlpha( max<float>(0.0f, cosf((currentTime / 20.0f) * 2.0f * M_PI)) );
	drawModel( skybox, modelMatrix );
	//*************************************************************************
	// END: Render.
	//*************************************************************************

	gfxObjectAlpha( 1.0f );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
	glUseProgram( 0 );	
}

void gfxCurrentTime( float currtime ) {
	currentTime = currtime;
}

void gfxIdle() {
	// rotate light around X axis, sunlike fashion.
	// do one full revolution every 20 seconds.
	float4x4 rotateLight = make_rotation_x<float4x4>(2.0f * M_PI * currentTime / 20.0f);
	// rotate and update global light position.
	lightPosition = make_vector3(rotateLight * make_vector(30.1f, 450.0f, 0.1f, 1.0f));
}

void gfxMiddleDown( mouse_change& change ) {
	camera_r -= float(change.dy) * 0.3f;
	// make sure cameraDistance does not become too small
	camera_r = max(0.1f, camera_r);
}

void gfxLeftDown( mouse_change& change ) {
	camera_phi	-= float(change.dy) * 0.3f * float(M_PI) / 180.0f;
	camera_phi = min(max(0.01f, camera_phi), float(M_PI) - 0.01f);
	camera_theta -= float(change.dx) * 0.3f * float(M_PI) / 180.0f;
}

void gfxRightDown( mouse_change& change ) {
	camera_target_altitude += float(change.dy) * 0.1f; 
}