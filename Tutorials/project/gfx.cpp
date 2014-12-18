#include "gfx.h"
#include "gfxutils.h"

using namespace std;
using namespace chag;

//*****************************************************************************
//	Common variables:
//*****************************************************************************
float4 ones = { 1.0f, 1.0f, 1.0f, 1.0f };
float4 zeros = { 0.0f, 0.0f, 0.0f, 0.0f };

//*****************************************************************************
//	Global variables
//*****************************************************************************
float currentTime = 0.0f;		// Tells us the current time.
GLuint shaderProgram, shadowShaderProgram;
const float3 up = {0.0f, 1.0f, 0.0f};
const float3 down = {0.0f, -1.0f, 0.0f};

//*****************************************************************************
//	Shadow map
//*****************************************************************************
GLuint shadowMapTexture;
GLuint shadowMapFBO;
const int shadowMapResolution = 2048;

//*****************************************************************************
//	Cube map
//*****************************************************************************
GLuint cubeMapTexture;
fbo_info dcmFBO;

//*****************************************************************************
//	Background clear color:
//*****************************************************************************
const float4 clear_color = zeros;
const float4 shadow_clear_color = ones;

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
float4x4 lightViewMatrix;
float4x4 lightProjMatrix;

void gfxUpdateLightMatrices() {
	lightViewMatrix = lookAt(lightPosition, make_vector(0.0f, 0.0f, 0.0f), up);
	lightProjMatrix = perspectiveMatrix(25.0f, 1.0, 400.0f, 600.0f);
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

	// We need to setup these;
	// otherwise the texture is illegal as a render target.
	gfxLinear();
	gfxClampBorder();
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &ones.x);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_REF_TO_TEXTURE);

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

void gfxBindShadowMap() {
	gfxTexture2DBind( shadowMapTexture, 1, "shadowMap" );
}

void gfxLoadCubeMap() {
	// Construct array of cube map side files.
	string base = SCENES "/../project/cube";
	string ext = ".png";

	const char* sides[6];
	for ( int i = 0; i < 6; ++i ) {
		string file = base + to_string( i ) + ext;
		sides[i] = file.c_str();
	}

	// Load the files.
	cubeMapTexture = loadCubeMap(
		sides[0], sides[1], sides[2],
		sides[3], sides[4], sides[5] );

	glUseProgram( shaderProgram );
	setUniformSlow( shaderProgram, "cubeMap", 2 );
	glUseProgram( 0 );
}

void gfxBindCubeMap() {
	gfxTextureCMBind( cubeMapTexture, 2, "cubeMap" );
}

void gfxDCMInit() {
	// create FBO.
	dcmFBO.generate();
	dcmFBO.width = 1024;
	dcmFBO.height = 1024;

	// create depth buffer.
	dcmFBO.depth_buffer();
	dcmFBO.bind_depth_buffer();

	// create the cubemap
	dcmFBO.tex_buffer();
	glBindTexture( GL_TEXTURE_CUBE_MAP, dcmFBO.colorBuffer );
	gfxLinear( GL_TEXTURE_CUBE_MAP );
	gfxClampEdge( GL_TEXTURE_CUBE_MAP );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

	// set textures.
	for ( int i = 0; i < 6; ++i ) {
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, dcmFBO.width, dcmFBO.height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
	}

	// attach only the +X cubemap texture (for completeness)
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, dcmFBO.colorBuffer, 0 );

	dcmFBO.check_complete();
	dcmFBO.restore_default();
	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
}

void gfxDCMBind() {
	gfxTextureCMBind( dcmFBO.colorBuffer, 2, "cubeMap" );
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

	gfxUpdateLightMatrices();

	// Load shaders
	gfxLoadShaders();

	// Load the models from disk.
	gfxLoadModels();

	// Initialize shadow map.
	gfxInitShadowMap();

	// Load cube map.
	gfxDCMInit();
}

/**
 * Helper function to set the matrices used for transform an lighting in our
 * shaders, this code is factored into its own function as we use it more than
 * once (needed once for each model drawn).
 */
void setLightingMatrices( GLuint shaderProgram, const float4x4 &modelMatrix, const float4x4 &viewMatrix, const float4x4 &projectionMatrix ) {
	float4x4 modelViewMatrix = viewMatrix * modelMatrix;	
	float4x4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
	float4x4 normalMatrix = transpose(inverse(modelViewMatrix));

	// Update the matrices used in the vertex shader
	setUniformSlow(shaderProgram, "modelMatrix", modelMatrix );
	setUniformSlow(shaderProgram, "modelViewMatrix", modelViewMatrix);
	setUniformSlow(shaderProgram, "modelViewProjectionMatrix", modelViewProjectionMatrix);
	setUniformSlow(shaderProgram, "normalMatrix", normalMatrix);
}

void drawModel( OBJModel *model, const float4x4 &modelMatrix, const float4x4 &viewMatrix, const float4x4 &projectionMatrix ) {
	setLightingMatrices( gfxCurrentProgram(), modelMatrix, viewMatrix, projectionMatrix );
	model->render();
}

/**
* In this function, add all scene elements that should cast shadow, that way
* there is only one draw call to each of these, as this function is called twice.
*/
void drawShadowCasters( const float4x4 &viewMatrix, const float4x4 &projectionMatrix, bool renderCar = true ) {
	float4x4 worldMatrix = make_identity<float4x4>();
	drawModel(world, worldMatrix, viewMatrix, projectionMatrix );

	if ( !renderCar ) {
		return;
	}

	gfxObjectReflectiveness( 0.5f );
	float4x4 carMatrix = make_translation(make_vector(0.0f, 0.0f, 0.0f));
	drawModel(car, carMatrix, viewMatrix, projectionMatrix );
	gfxObjectReflectiveness( 0.0f );
}

void drawShadowMap(const float4x4 &viewMatrix, const float4x4 &projectionMatrix) {
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 2.5, 10 );
	glBindFramebuffer( GL_FRAMEBUFFER, shadowMapFBO );

	gfxViewport( shadowMapResolution, shadowMapResolution );
	gfxClear( shadow_clear_color );

	// Get current shader, so we can restore it afterwards.
	// Also, switch to the shadow shader used to draw the shadow map.
	GLint prevProgram = gfxUseProgram( shadowShaderProgram );

	// draw shadow casters
	drawShadowCasters( viewMatrix, projectionMatrix );

	// Restore old shader
	gfxUseProgram(prevProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_POLYGON_OFFSET_FILL);
}

float4x4 gfxViewMatrix() {
	float3 camera_position = sphericalToCartesian( camera_theta, camera_phi, camera_r );
	float3 camera_lookAt = make_vector( 0.0f, camera_target_altitude, 0.0f );
	float3 camera_up = make_vector( 0.0f, 1.0f, 0.0f );
	float4x4 viewMatrix = lookAt( camera_position, camera_lookAt, camera_up );
	return viewMatrix;	
}

float4x4 gfxProjectionMatrix( int w, int h, float fov = 45.0f ) {
	return perspectiveMatrix( fov, float(w) / float(h), 0.1f, 1000.0f );
}

float4x4 gfxLightMatrix( float4x4& lightProjectionMatrix, float4x4& lightViewMatrix, float4x4& viewMatrix ) {
	return make_translation(make_vector(0.5f, 0.5f, 0.5f)) *
		make_scale<float4x4, float>(0.5) *
		lightProjectionMatrix *
		lightViewMatrix *
		inverse(viewMatrix);
}

void gfxDCMDraw();

void drawScene(float4x4 lightViewMatrix, float4x4 lightProjMatrix, float4x4& viewMatrix, int w, int h, float fov, bool renderCar = true ) {
	// enable Z-buffering.
	glEnable(GL_DEPTH_TEST);
	// enable back face culling.
	glEnable(GL_CULL_FACE);	

	//*******************************************************************************
	// START: Render the scene from the cameras viewpoint, to the default framebuffer
	//*******************************************************************************
	gfxViewport( w, h );
	gfxClear( clear_color );

	// Use shader and set up uniforms
	glUseProgram( shaderProgram );

	// Setup our matrices & uniforms:
	float4x4 projectionMatrix = gfxProjectionMatrix( w, h, fov );
	float4x4 lightMatrix = gfxLightMatrix( lightProjMatrix, lightViewMatrix, viewMatrix );
	setUniformSlow( shaderProgram, "lightpos", lightPosition );
	setUniformSlow( shaderProgram, "inverseViewNormalMatrix", transpose( viewMatrix ) );
	setUniformSlow( shaderProgram, "lightMatrix", lightMatrix );

	// Shadow map texture:
	gfxBindShadowMap();

	// Cube map:
	gfxBindCubeMap();
//	gfxDCMDraw();
//	gfxDCMBind();

	float4x4 waterModelMatrix = make_translation(make_vector(0.0f, -6.0f, 0.0f));
	drawModel(water, waterModelMatrix, viewMatrix, projectionMatrix );
	drawShadowCasters( viewMatrix, projectionMatrix, renderCar );

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const float4x4 modelMatrix = make_identity<float4x4>();
	drawModel( skyboxnight, modelMatrix, viewMatrix, projectionMatrix );

	gfxObjectAlpha( max<float>(0.0f, cosf((currentTime / 20.0f) * 2.0f * M_PI)) );
	drawModel( skybox, modelMatrix, viewMatrix, projectionMatrix );
	//*************************************************************************
	// END: Render.
	//*************************************************************************

	gfxObjectAlpha( 1.0f );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );
	glUseProgram( 0 );	
}

void gfxDCMDraw() {
	float3 camera_position = make_vector( 0.0f, 0.0f, 0.0f );
	const float distance = 5.0f;
	const float avgdist = 1.0f;

	for ( int i = 0; i < 6; ++i ) {
		int face = i;

		// bind, clear, attach.
		dcmFBO.bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, dcmFBO.colorBuffer, 0);
		gfxViewport( 1024, 1024 );
		gfxClear( ones );

		// setup lookat depending on current face
		float3 camera_lookAt;
		float3 camera_up = up;

		switch ( face ) {
			case 0: // +X
				camera_lookAt = make_vector( distance, avgdist, avgdist );
				break;
			case 1: // -X
				camera_lookAt = make_vector( -distance, avgdist, avgdist );
				break;
			case 2: // +Y
				camera_lookAt = make_vector( avgdist, distance, avgdist );
				break;
			case 3: // -Y
				camera_lookAt = make_vector( avgdist, -distance, avgdist );
				camera_up = down;
				break;
			case 4: // +Z
				camera_lookAt = make_vector( avgdist, avgdist, distance );
				break;
			case 5: // -Z
				camera_lookAt = make_vector( avgdist, avgdist, -distance );
				break;
        };

		// compute view matrix.
		float4x4 viewMatrix = lookAt( camera_position, camera_lookAt, camera_up );

		drawScene( lightViewMatrix, lightProjMatrix, viewMatrix, 1024, 1024, 90.0f, false );
	}
}

void gfxDisplay() {
	float4x4 viewMatrix = gfxViewMatrix();

	drawShadowMap( lightViewMatrix, lightProjMatrix );

	gfxDCMDraw();

	int w = gfxViewportWidth();
	int h = gfxViewportHeight();
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	drawScene( lightViewMatrix, lightProjMatrix, viewMatrix, w, h, 45.0f, true );
	glutSwapBuffers();  // swap front and back buffer. This frame will now be displayed.
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
	gfxUpdateLightMatrices();
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