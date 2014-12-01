#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <stdlib.h>
#include <algorithm>

#include <glutil.h>

#include "gfx.h"

using namespace std;
using namespace chag;

//*****************************************************************************
//	Global variables
//*****************************************************************************
bool paused = false;			// Tells us wether sun animation is paused.

//*****************************************************************************
//	Mouse input state variables
//*****************************************************************************
bool leftDown = false;
bool middleDown = false;
bool rightDown = false;

mouse_change mouse;

void display( void ) {
	gfxDisplay();
	CHECK_GL_ERROR();
}

void handleKeys( unsigned char key, int /*x*/, int /*y*/ ) {
	switch(key) {
	case 27:    /* ESC */
		exit(0); /* dirty exit */
		break;   /* unnecessary, I know */
	case 32:    /* space */
		paused = !paused;
		break;
	}
}

void handleSpecialKeys( int key, int /*x*/, int /*y*/ ) {
	switch(key) {
	case GLUT_KEY_LEFT:
		printf("Left arrow\n");
		break;
	case GLUT_KEY_RIGHT:
		printf("Right arrow\n");
		break;
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
		break;
	}
}

void mouseButton( int button, int state, int x, int y ) {
	// reset the previous position, such that we only get movement performed after the button
	// was pressed.
	mouse.set( x, y );

	bool buttonDown = state == GLUT_DOWN;

	switch(button) {
	case GLUT_LEFT_BUTTON:
		leftDown = buttonDown;
		break;
	case GLUT_MIDDLE_BUTTON:
		middleDown = buttonDown;
		break;
	case GLUT_RIGHT_BUTTON: 
		rightDown = buttonDown;
	default:
		break;
	}
}

void motion(int x, int y) {
	mouse.delta( x - mouse.x, y - mouse.y );

	if ( middleDown ) {
		gfxMiddleDown( mouse );
	}

	if ( leftDown ) {
		gfxLeftDown( mouse );
	}

	if ( rightDown ) {
		gfxRightDown( mouse );
	}

	mouse.set( x, y );
}

void idle( void ) {
	static float startTime = float(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f;
	// Here is a good place to put application logic.
	if ( !paused ) {
		float currentTime = float(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f - startTime;
		gfxCurrentTime( currentTime );
	}
	gfxIdle();

	glutPostRedisplay();  
	// Uncommenting the line above tells glut that the window 
	// needs to be redisplayed again. This forces the display to be redrawn
	// over and over again. 
}

int main( int argc, char *argv[] ) {
#	if defined(__linux__)
	linux_initialize_cwd();
#	endif // ! __linux__

	glutInit(&argc, argv);

	/* Request a double buffered window, with a sRGB color buffer, and a depth
	 * buffer. Also, request the initial window size to be 800 x 600.
	 *
	 * Note: not all versions of GLUT define GLUT_SRGB; fall back to "normal"
	 * RGB for those versions.
	 */
#	if defined(GLUT_SRGB) && defined(WIN32)
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_SRGB | GLUT_DEPTH);
#	else // !GLUT_SRGB
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	printf( "--\n" );
	printf( "-- WARNING: your GLUT doesn't support sRGB / GLUT_SRGB\n" );
#	endif // ~ GLUT_SRGB
	glutInitWindowSize(800,600);

	/* Require at least OpenGL 3.0. Also request a Debug Context, which allows
	 * us to use the Debug Message API for a somewhat more humane debugging
	 * experience.
	 */
	glutInitContextVersion(3,0);
	glutInitContextFlags(GLUT_DEBUG);

	/* Request window
	 */
	glutCreateWindow("Project");

	/* Set callbacks that respond to various events. Most of these should be
	 * rather self-explanatory (i.e., the MouseFunc is called in response to
	 * a mouse button press/release). The most important callbacks are however
	 *
	 *   - glutDisplayFunc : called whenever the window is to be redrawn
	 *   - glutIdleFunc : called repeatedly
	 *
	 * The window is redrawn once at startup (at the beginning of
	 * glutMainLoop()), and whenever the window changes (overlap, resize, ...).
	 * To repeatedly redraw the window, we need to manually request that via
	 * glutPostRedisplay(). We call this from the glutIdleFunc.
	 */
	glutIdleFunc(idle);
	glutDisplayFunc(display);

	glutKeyboardFunc(handleKeys); // standard key is pressed/released
	glutSpecialFunc(handleSpecialKeys); // "special" key is pressed/released
	glutMouseFunc(mouseButton); // mouse button pressed/released
	glutMotionFunc(motion); // mouse moved *while* any button is pressed

	/* Now that we should have a valid GL context, perform our OpenGL 
	 * initialization, before we enter glutMainLoop().
	 */
	gfxInit();

	/* If sRGB is available, enable rendering in sRGB. Note: we should do
	 * this *after* initGL(), since initGL() initializes GLEW.
	 */
	glEnable(GL_FRAMEBUFFER_SRGB);

	/* Start the main loop. Note: depending on your GLUT version, glutMainLoop()
	 * may never return, but only exit via std::exit(0) or a similar method.
	 */
	glutMainLoop();

	return 0;          
}
