#include "gfx.h"

using namespace std;
using namespace chag;

//*****************************************************************************
//	Global variables
//*****************************************************************************
float currentTime = 0.0f;		// Tells us the current time.
GLuint shaderProgram;
const float3 up = {0.0f, 1.0f, 0.0f};

//*****************************************************************************
//	Background clear color:
//*****************************************************************************
const float3 clear_color = {0.2, 0.2, 0.8};

//*****************************************************************************
//	OBJ Model declarations
//*****************************************************************************
OBJModel *world; 
OBJModel *water; 
OBJModel *skybox; 
OBJModel *skyboxnight; 
OBJModel *car; 

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	shaderProgram = loadShaderProgram("simple.vert", "simple.frag");
	glBindAttribLocation(shaderProgram, 0, "position"); 	
	glBindAttribLocation(shaderProgram, 2, "texCoordIn");
	glBindAttribLocation(shaderProgram, 1, "normalIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
	linkShaderProgram(shaderProgram);

	//*************************************************************************
	// Load the models from disk
	//*************************************************************************
	world = new OBJModel();
	world->load(SCENES "/island_blender_new.obj");
	world->load(SCENES "/island.obj");

	skybox = new OBJModel();
	skybox->load(SCENES "/skybox.obj");

	skyboxnight = new OBJModel();
	skyboxnight->load(SCENES "/skyboxnight.obj");

	// Make the textures of the skyboxes use clamp to edge to avoid seams
	for ( int i = 0; i < 6; i++ ) {
		glBindTexture(GL_TEXTURE_2D, skybox->getDiffuseTexture(i));
		gfxClampEdge();

		glBindTexture(GL_TEXTURE_2D, skyboxnight->getDiffuseTexture(i));
		gfxClampEdge();
	}

	water = new OBJModel(); 
	water->load(SCENES "/water.obj");

	car = new OBJModel(); 
	car->load(SCENES "/car.obj");
}

void drawScene();

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

void drawScene(void) {
	glEnable(GL_DEPTH_TEST);	// enable Z-buffering 

	// enable back face culling.
	glEnable(GL_CULL_FACE);	

	//*************************************************************************
	// Render the scene from the cameras viewpoint, to the default framebuffer
	//*************************************************************************
	gfxClear();
	int w = gfxViewportWidth();
	int h = gfxViewportHeight();
	gfxViewport( w, h );

	// Use shader and set up uniforms
	glUseProgram( shaderProgram );			
	float3 camera_position = sphericalToCartesian(camera_theta, camera_phi, camera_r);
	float3 camera_lookAt = make_vector(0.0f, camera_target_altitude, 0.0f);
	float3 camera_up = make_vector(0.0f, 1.0f, 0.0f);
	float4x4 viewMatrix = lookAt(camera_position, camera_lookAt, camera_up);
	float4x4 projectionMatrix = perspectiveMatrix(45.0f, float(w) / float(h), 0.1f, 1000.0f);
	setUniformSlow(shaderProgram, "viewMatrix", viewMatrix);
	setUniformSlow(shaderProgram, "projectionMatrix", projectionMatrix);
	setUniformSlow(shaderProgram, "lightpos", lightPosition); 

	drawModel(water, make_translation(make_vector(0.0f, -6.0f, 0.0f)));
	drawShadowCasters();

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	drawModel(skyboxnight, make_identity<float4x4>());

	setUniformSlow(shaderProgram, "object_alpha", max<float>(0.0f, cosf((currentTime / 20.0f) * 2.0f * M_PI))); 
	drawModel(skybox, make_identity<float4x4>());

	setUniformSlow(shaderProgram, "object_alpha", 1.0f); 
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE); 

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