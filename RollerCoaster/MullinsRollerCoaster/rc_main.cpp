/* Programmer:	Ryan S Mullins
 * Class:		CMPSC 458
 * Section:		001
 * Project:		Roller Coaster
 * Date:		9/24/2010
 * Description:	Roller Coaster simulation using OpenGL. 
 */

#ifdef WIN32
// For VC++ you need to include this file as glut.h and gl.h refer to it
#include <windows.h>
#endif

#include <stdio.h>     // Standard Header For Most Programs
#include <stdlib.h>    // Additional standard Functions (exit() for example)
#include <time.h>
#ifdef __APPLE__

// This ONLY APPLIES to OSX
//
// Remember to add the -framework OpenGL - framework GLUT to
// to the gcc command line or include those frameworks
// using Xcode

#include <OpenGL/gl.h>		// The GL Header File
#include <OpenGL/glu.h>		// The GL Utilities (GLU) header
#include <GLUT/glut.h>		// The GL Utility Toolkit (Glut) Header
#else
#include <GL/gl.h>			// The GL Header File
#include <GL/glu.h>			// The GL Utilities (GLU) header
#include <GL/glut.h>		// The GL Utility Toolkit (Glut) Header
#endif

// Interface to libpicio, provides functions to load/save jpeg files
#include <pic.h>
#include "rc_spline.h"

/* Here we will load the spline that represents the track */
rc_Spline g_Track;

/* glut Id for the context menu */
int g_iMenuId;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define xMax 100
#define yMax 100
#define zMax 100

/******** Texture IDs ********
 * Each texture has to have its own id, they are assigned 
 * using glGenTextures, which provides a unique integer 
 * identifying that texture which you can load using 
 * glLoadTexture before drawing shapes with texturing 
 */
GLuint frontTextureId;
GLuint rearTextureId;
GLuint leftTextureId;
GLuint rightTextureId;
GLuint deckTextureId;
GLuint ceilingTextureId;
GLuint wormholeTextureId;
/******** End Texture IDs ********/

/******** Display List IDs ********
 * Each Display Listis given is own unique ID, assigned by
 * glGenLists(). Lists are then called at display time and 
 * drawn from the stored geometery. 
 */
GLuint skyboxListID;
GLuint trackListID;
GLuint trackPointListID;
GLuint wormholeListID;
/******** End Display List IDs ********/

/******** Other Global Declarations ********/
const int pixelWidth= 50;						// Width of each pixel in model space
float TrackPointIndex = 0.0;					// Current position along track
pointVectorIter ViewPosition;					// Iterator for current viewing position
Pic * wormholeImage;
Vec3f TrackTranslation( 0, 0, 0 );
Vec3f LookPoint( 0, 10, 0 );					// Vector for the LookAt point
Vec3f UpPoint( 0, 0, 10 );						// Vector for the Up direction
/******** End Other Global Declarations ********/

/******** OpenGL Callback Declarations ********
 * Definitions are at the end to avoid clutter
 */
void InitGL ( GLvoid );
void InitLight();
void InitMaterial();
void doIdle();
void display ( void );
void keyboardfunc (unsigned char key, int x, int y) ;
void menufunc( int value );
void loadTexture (char *filename, GLuint &textureID);
void genSkyboxDisplayList();
void genTrackPointDisplayList();
void genTrackDisplayList();
void genWormholeDisplayList();
void drawTrack();
void drawSplineSegment( Vec3f pt1, Vec3f pt2, Vec3f pt3, Vec3f pt4 );
void cameraUpdate( int val );
Vec3f findUValue( float t, Vec3f pt1, Vec3f pt2, Vec3f pt3, Vec3f pt4 );
/******** End OpenGL Callback Declarations ********/

/******** The ubiquituous main function ********/
int main ( int argc, char** argv )								// Create Main Function For Bringing It All Together
{
	if (argc!=2)												// get the the filename from the commandline.
	{		
		printf("usage: %s trackfilname\n", argv[0]);
		system("PAUSE");
		exit(1);
	}

	wormholeImage = jpeg_read("texture/wormhole.jpg", wormholeImage);

	g_Track.loadSplineFrom(argv[1]);							// load the track, this routine aborts if it fails 

	/*** The following are commands for setting up GL      ***/
	/*** No OpenGl call should be before this sequence!!!! ***/
	glutInit(&argc,argv);										// Initialize glut
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);	// Set up window modes 
	glutInitWindowPosition(0,0);								// Set window position (where it will appear when it opens)
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);			// Set size of window 
	glutCreateWindow    ( "CMPSC 458: Rollercoaster" );			// Create window

	/**** Call to our initialization routine****/
	InitGL ();
	glutDisplayFunc(display);									// Tells glut to use a particular display function to redraw
	g_iMenuId = glutCreateMenu(menufunc);						// Set menu function callback 
	glutSetMenu(g_iMenuId);										// Set identifier for menu
	glutAddMenuEntry("Quit",0);									// Add quit option to menu 

	/* Add any other menu functions you may want here.  The format is:
	 * glutAddMenuEntry(MenuEntry, EntryIndex)
	 * The index is used in the menufunc to determine what option was selected
	 */
	 
	glutAttachMenu(GLUT_RIGHT_BUTTON);							// Attach menu to right button clicks

	/* Set idle function.  You can change this to call code for your animation,
	 * or place your animation code in doIdle 
	 */
	glutIdleFunc(doIdle);
	glutKeyboardFunc(keyboardfunc);								// callback for keyboard input 

	glutMainLoop();												// Initialize The Main Loop

	return 0;
}

/************************************************************
 ******** Here are all the standard OpenGL callbacks ********
 ************************************************************/

/* initGL will perform the one time initialization by
 * setting some state variables that are not going to
 * be changed
 */
void InitGL ( GLvoid )     // Create Some Everyday Functions
{
	printf("Entered: InitGL()\n");
	glMatrixMode( GL_PROJECTION );								// Select The Projection Matrix 
	glLoadIdentity();											// Reset The Projection Matrix  
	gluPerspective( 45.0f,										// Set Projection Matrix to perspetive view
		(GLfloat)WINDOW_WIDTH/(GLfloat)WINDOW_HEIGHT, 
		0.1f, 
		5000.0f );
	glEnable(GL_DEPTH_TEST);									// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);										
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);						// Black Background, 50% Opaque

	/******** Load Textures ********/
	loadTexture( "texture/front.jpg", frontTextureId );			// Front Wall
	loadTexture( "texture/left.jpg", leftTextureId );			// Left Wall
	loadTexture( "texture/right.jpg", rightTextureId );			// Right Wall
	loadTexture( "texture/rear.jpg", rearTextureId );			// Rear Wall
	loadTexture( "texture/deck.jpg", deckTextureId );			// Deck 
	loadTexture( "texture/ceiling.jpg", ceilingTextureId );		// Ceiling
	loadTexture( "texture/wormhole.jpg", wormholeTextureId );	// Worm Hole 
	/******** End Load Textures ********/

	InitLight();
	InitMaterial();

	/******** Generate Display Lists 1 ********
	 * These Display Lists represent objects that are unaffected 
	 * by the Translate, Rotate, and Scale functions. 
	 */
	genSkyboxDisplayList();										// Generate Skybox
	genTrackPointDisplayList();									// Generate Track
	genTrackDisplayList();										// Generate Track
	/******** End Generate Display Lists 1 ********/

	/******** Generate Display Lists 2 ********
	 * These Display Lists represent objects that are affected 
	 * by the Translate, Rotate, and Scale functions. 
	 */
	genWormholeDisplayList();									// Generate Wormhole
	/******** End Generate Display Lists 2 ********/ 

	ViewPosition = g_Track.points().begin();
	printf("Exited: InitGL()\n");
}

/* Function to initialize lighting in the environment */
void InitLight( void ){
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	// Set up global ambient
	GLfloat global_ambient[] = { .2, .2, .2, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

	// Set Light 1 Properties
	GLfloat light0Ambient[]	 = { 1, 1, 1, 1 };
	GLfloat light0Diffuse[]	 = { 1, 1, 1, 1 };
	GLfloat light0Specular[] = { 1, 1, 1, 1 };
	GLfloat light0Position[] = { -50, 50, 0, 1 };

	// Light 0 Init
	glLightfv(GL_LIGHT0, GL_AMBIENT , light0Ambient );
	glLightfv(GL_LIGHT0, GL_DIFFUSE , light0Diffuse );
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
	
	// Set Light 1 Properties
	GLfloat light1Ambient[]	 = { 0.8, 0.45, 0, 1 };
	GLfloat light1Diffuse[]	 = { 1, 1, 1, 1 };
	GLfloat light1Specular[] = { 1, 1, 1, 1 };
	GLfloat light1Position[] = { 50, -50, 0, 1 };

	// Light 1 Init
	glLightfv(GL_LIGHT1, GL_AMBIENT , light1Ambient );
	glLightfv(GL_LIGHT1, GL_DIFFUSE , light1Diffuse );
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light1Position);

	//Turn on light
	glEnable(GL_LIGHT0);
}

/* Function to initialize materials for objects
 * in the environment
 */
void InitMaterial ( void )
{
	GLfloat mat_amb_diff[] = { 0.7, 0.2, 0.5, 1.0 };
	GLfloat lowShininess[] = { 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);
	glMaterialfv(GL_FRONT, GL_SHININESS, lowShininess);
}

/* Main GL display loop.
 * This function is called by GL whenever it wants to update the display.
 * We will assume that the resulting image has already been created, and 
 * we will just paint it to the display.
 */
void display ( void )   // Create The Display Function
{ 
	printf("Entered: display()\n");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear buffers

	glViewport( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT );		// Reset The Current Viewport
	glMatrixMode( GL_MODELVIEW );							// Select the Modelview Matrix 
	glLoadIdentity();										// Reset the Modelview Matrix
	
	//gluLookAt( 0, -50, 0, 0, 0, 0, 0, 0, 10 );	 

	glCallList( wormholeListID );

	glTranslatef( TrackTranslation.x(), TrackTranslation.y(), TrackTranslation.z() );
	/******** Draw Environment Display Lists ********/
	glCallList( skyboxListID );								// Draw Skybox
	//glCallList( trackPointListID );							// Draw Track Control Points
	glCallList( trackListID );								// Draw Track
	/******** End Draw Environment Display Lists ********/

	glutSwapBuffers();										// Swap buffers, so one we just drew is displayed 
	printf("Exited: display()\n");
}

/* This function will be called by GL whenever a keyboard key is pressed.
 * It recieves the ASCII value of the key that was pressed, along with the x
 * and y coordinates of the mouse at the time the key was pressed.
 */
void keyboardfunc (unsigned char key, int x, int y) {

	// User pressed quit key 
	if(key == 'q' || key == 'Q' || key == 27) {
		exit(0);
	}
}

/* Function that GL runs once a menu selection has been made.
 * This receives the number of the menu item that was selected
 */
void menufunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	}
}

/* This function is called by GL whenever it is idle, usually after calling
 * display.
 */
void doIdle()
{

	glutTimerFunc( 500, cameraUpdate, 1 );

	// make the screen update. 
	glutPostRedisplay(); 
}

Vec3f findUValue( float t, Vec3f pt1, Vec3f pt2, Vec3f pt3, Vec3f pt4 )
{
	// Calculate blending functions 
	float p[4];
	p[0] =-1.0 * t * t * t + 2.0 * t * t - 1.0 * t + 0.0; 
	p[1] = 3.0 * t * t * t - 5.0 * t * t + 0.0 * t + 2.0; 
	p[2] =-3.0 * t * t * t + 4.0 * t * t + 1.0 * t + 0.0; 
	p[3] = 1.0 * t * t * t - 1.0 * t * t + 0.0 * t + 0.0;

	// Calctlate the X, Y and Z position of the vertex and draw it
	float x, y, z;
	x = y = z = 0.0; 
	x = p[0]*pt1.x() + p[1]*pt2.x() + p[2]*pt3.x() + p[3]*pt4.x();
	y = p[0]*pt1.y() + p[1]*pt2.y() + p[2]*pt3.y() + p[3]*pt4.y();
	z = p[0]*pt1.z() + p[1]*pt2.z() + p[2]*pt3.z() + p[3]*pt4.z();

	return Vec3f( x, y, z );
}

/* Function to update Viewpoint */
void cameraUpdate( int val )
{
	/******** Update Position ********/
	if( val )
	{
		float x, y, z, u;
		int TrackArraySize = g_Track.points().size();
		Vec3f Point0, Point1, Point2, Point3, Point4;
		Vec3f viewPoint, nextPoint;

		if( TrackPointIndex == 0 )
		{
			Point0 = g_Track.points()[g_Track.points().size() - 1];
			Point1 = g_Track.points()[0];
			Point2 = g_Track.points()[1];
			Point3 = g_Track.points()[2];
			Point4 = g_Track.points()[3];
		}
		else if( TrackPointIndex < g_Track.points().size() - 3 )
		{
			Point0 = g_Track.points()[(int)TrackPointIndex - 1];
			Point1 = g_Track.points()[(int)TrackPointIndex];
			Point2 = g_Track.points()[(int)TrackPointIndex + 1];
			Point3 = g_Track.points()[(int)TrackPointIndex + 2];
			Point4 = g_Track.points()[(int)TrackPointIndex + 3];
		}
		else if( TrackPointIndex < g_Track.points().size() - 2 )
		{
			Point0 = g_Track.points()[g_Track.points().size() - 4];
			Point1 = g_Track.points()[g_Track.points().size() - 3];
			Point2 = g_Track.points()[g_Track.points().size() - 2];
			Point3 = g_Track.points()[g_Track.points().size() - 1];
			Point4 = g_Track.points()[0];
		}
		else if( TrackPointIndex < g_Track.points().size() - 1 )
		{
			Point0 = g_Track.points()[g_Track.points().size() - 3];
			Point1 = g_Track.points()[g_Track.points().size() - 2];
			Point2 = g_Track.points()[g_Track.points().size() - 1];
			Point3 = g_Track.points()[0];
			Point4 = g_Track.points()[1];
		}
		else
		{
			Point0 = g_Track.points()[g_Track.points().size() - 2];
			Point1 = g_Track.points()[g_Track.points().size() - 1];
			Point2 = g_Track.points()[0];
			Point3 = g_Track.points()[1];
			Point4 = g_Track.points()[2];
			TrackPointIndex = 0;
		}

		u = TrackPointIndex - (int)TrackPointIndex;

		viewPoint = findUValue( u, Point0, Point1, Point2, Point3 );
		nextPoint = findUValue( u, Point1, Point2, Point3, Point4 );

		TrackTranslation.x() = -nextPoint.x();
		TrackTranslation.y() = -nextPoint.y();
		TrackTranslation.z() = -nextPoint.z();

		x = nextPoint.x() - viewPoint.x();
		y = nextPoint.y() - viewPoint.y();
		z = nextPoint.z() - viewPoint.z();

		TrackPointIndex += abs(sqrt( 2 * 9.8 * ( yMax - nextPoint.y() ) / 1000 ));

		gluLookAt( 0, 0, 2,
				   LookPoint.x(), LookPoint.y(), LookPoint.z(),
				   0, 0, 10);

		glutPostRedisplay();
	} 
	/******** End Update Position ********/
}

/* This function takes the name of a jpg file and a texture ID (by reference)
 * It creates a texture from the image specified and sets the ID specified to 
 * the value OpenGL generates for that texture
 */
void loadTexture (char *filename, GLuint &textureID)
{
	printf("Entered: loadTexture()\n\tLoading: %s\n", filename);
	Pic *pBitMap = pic_read(filename, NULL);

	if(pBitMap == NULL)
	{
		printf("Could not load the texture file %s\n", filename);
		system("PAUSE");
		exit(1);
	}

	glEnable(GL_TEXTURE_2D); // Enable texturing
	glGenTextures(1, &textureID); // Obtain an id for the texture

	glBindTexture(GL_TEXTURE_2D, textureID); // Set as the current texture

	/* set some texture parameters */
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	/* Load the texture into OpenGL as a mipmap. !!! This is a very important step !!! */
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, pBitMap->nx, pBitMap->ny, GL_RGB, GL_UNSIGNED_BYTE, pBitMap->pix);
	glDisable(GL_TEXTURE_2D);
	pic_free(pBitMap); // now that the texture has been copied by OpenGL we can free our copy
	printf("Exited: loadTexture()\n");
}

/* Function to create the Skybox Display List */
void genSkyboxDisplayList()
{
	printf("Entered: genSkyboxDisplayList()\n");
	skyboxListID = glGenLists( 1 );
	glNewList( skyboxListID, GL_COMPILE );

	/******** Draw SkyBox ********/
	glEnable(GL_TEXTURE_2D);								// Enable texturing 
	glDisable(GL_DEPTH_TEST);								// Turn off depth testing
	glDepthMask(false);										// Turn off depth masking

	// Front Wall (Standard pattern for all other walls) 
	glBindTexture(GL_TEXTURE_2D, frontTextureId);			// Select which texture to use 
	glBegin(GL_QUADS);										// Use GL_QUADS to map teture onto
	glColor3f(1.0, 1.0, 1.0); 								// Set color for background

	glTexCoord2f(0.0 ,0.0);									// Set vertex for upper left corner 
	glVertex3f(-xMax, yMax, zMax);
	glTexCoord2f(0.0 ,1.0);									// Set vertex for lower left corner
	glVertex3f(-xMax, yMax, -zMax);
	glTexCoord2f(1.0 ,1.0);									// Set vertex for lower right corner
	glVertex3f(xMax, yMax, -zMax);
	glTexCoord2f(1.0 ,0.0);									// Set vertex for upper right corner
	glVertex3f(xMax, yMax, zMax);

	glEnd();												// End Wall definition and texture mapping

	// Left Wall
	glBindTexture(GL_TEXTURE_2D, leftTextureId);
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0); 	

	glTexCoord2f(0.0 ,0.0);		
	glVertex3f(-xMax, -yMax, zMax);
	glTexCoord2f(0.0 ,1.0);
	glVertex3f(-xMax, -yMax, -zMax);
	glTexCoord2f(1.0 ,1.0);
	glVertex3f(-xMax, yMax, -zMax);
	glTexCoord2f(1.0 ,0.0);
	glVertex3f(-xMax, yMax, zMax);

	glEnd();

	// Right Wall
	glBindTexture(GL_TEXTURE_2D, rightTextureId);  
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0); 	

	glTexCoord2f(0.0 ,0.0);		
	glVertex3f(xMax, yMax, zMax);
	glTexCoord2f(0.0 ,1.0);
	glVertex3f(xMax, yMax, -zMax);
	glTexCoord2f(1.0 ,1.0);
	glVertex3f(xMax, -yMax, -zMax);
	glTexCoord2f(1.0 ,0.0);
	glVertex3f(xMax, -yMax, zMax);

	glEnd();

	// Rear Wall
	glBindTexture(GL_TEXTURE_2D, rearTextureId);  
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0); 	

	glTexCoord2f(0.0 ,0.0);		
	glVertex3f(xMax, -yMax, zMax);
	glTexCoord2f(0.0 ,1.0);
	glVertex3f(xMax, -yMax, -zMax);
	glTexCoord2f(1.0 ,1.0);
	glVertex3f(-xMax, -yMax, -zMax);
	glTexCoord2f(1.0 ,0.0);
	glVertex3f(-xMax, -yMax, zMax);

	glEnd();

	// Deck
	glBindTexture(GL_TEXTURE_2D, deckTextureId);  
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0); 	

	glTexCoord2f(0.0 ,0.0);		
	glVertex3f(-xMax, yMax, -zMax);
	glTexCoord2f(0.0 ,1.0);
	glVertex3f(-xMax, -yMax, -zMax);
	glTexCoord2f(1.0 ,1.0);
	glVertex3f(xMax, -yMax, -zMax);
	glTexCoord2f(1.0 ,0.0);
	glVertex3f(xMax, yMax, -zMax);

	glEnd();

	// Ceiling
	glBindTexture(GL_TEXTURE_2D, ceilingTextureId);  
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0); 	

	glTexCoord2f(0.0 ,0.0);		
	glVertex3f(xMax, yMax, zMax);
	glTexCoord2f(0.0 ,1.0);
	glVertex3f(xMax, -yMax, zMax);
	glTexCoord2f(1.0 ,1.0);
	glVertex3f(-xMax, -yMax, zMax);
	glTexCoord2f(1.0 ,0.0);
	glVertex3f(-xMax, yMax, zMax);

	glEnd();

	glDisable(GL_TEXTURE_2D); //disable texturing so that the following objects do not use it
	/******** End Draw SkyBox ********/

	glEndList();
	printf("Exited: genSkyboxDisplayList()\n");
}

/* Function to creat the Track Display List */
void genTrackPointDisplayList()
{
	printf("Entered: genTrackPointDisplayList()\n");
	trackPointListID = glGenLists( 1 );
	glNewList( trackPointListID, GL_COMPILE );

	/******** Draw Track Points ********/
	glBegin( GL_POINTS );
	pointVectorIter ptsiter = g_Track.points().begin();
	Vec3f currentpos( *ptsiter );
	ptsiter++;

	glVertex3f( currentpos.x(), currentpos.y(), currentpos.z() );

	// Here is the interesting example: iterate throught  the points 
	for(; ptsiter != g_Track.points().end(); ptsiter++)
	{
		// get the next point from the iterator 
		Vec3f pt( *ptsiter );
		currentpos = pt;
		glColor3f( 1, 0, 0 );
		glVertex3f( currentpos.x(), currentpos.y(), currentpos.z() );
	}

	glEnd();
	/******** End Draw Track Points ********/

	glEndList();
	printf("Exited: genTrackPointDisplayList()\n");
}

void genTrackDisplayList()
{
	printf("Entered: genTrackDisplayList()\n");
	trackListID = glGenLists( 1 );
	glNewList( trackListID, GL_COMPILE );

	/******** Draw Track ********/
	drawTrack();
	/******** End Draw Track ********/

	glEndList();
	printf("Exited: genTrackDisplayList()\n");
}

/* Function to create Wormhole Display List */
void genWormholeDisplayList()
{
	printf("Entered: genWormholeDisplayList()\n");
	float x = 0.0;	// X-Position of pixel
	float y = 0.0;	// Y-Position of pixel
	float z = 0.0;	// Z-Position of pixel

	wormholeListID = glGenLists( 1 );
	glNewList( wormholeListID, GL_COMPILE ); 

	/******** Draw Wormhole ********
	 * This wormhole uses the an implementation similar 
	 * to the heightmap used in the previous lab
	 */
	glEnable( GL_TEXTURE_2D );							// Enable OpenGL Texture Mapping 
	glBindTexture( GL_TEXTURE_2D, wormholeTextureId );	// Map Womrhole Texture to triangle strip
	glBegin( GL_TRIANGLE_STRIP );						// Begin wormhole triangle strip
	for(int i = 1; i < wormholeImage->ny; i++)
	{
		for(int j = 1; j < wormholeImage->nx; j++)
		{
			// Color Setup
			int red   = PIC_PIXEL( wormholeImage, j, i, 0 );
			int green = PIC_PIXEL( wormholeImage, j, i, 1 );
			int blue  = PIC_PIXEL( wormholeImage, j, i, 2 );
			float color = (float)(red + green + blue) / (3.0 * 255.0);
			glColor3f( (float)red, (float)green, (float)blue );

			// Vertex Setup
			x = 5 * (float)j/(float)wormholeImage->nx;
			y = 5 * color;
			z = 5 * (float)i/(float)wormholeImage->nx;
			glVertex3f( x, y, z );

			// Color 2 Setup
			int red1   = PIC_PIXEL( wormholeImage , j, i+1, 0 );	// Find RGB value for each pixel
			int green1 = PIC_PIXEL( wormholeImage , j, i+1, 1 );
			int blue1  = PIC_PIXEL( wormholeImage , j, i+1, 2 );
			float color1 = (float)( red1 + green1 + blue1 ) / ( 3.0 * 255.0 );
			glColor3f( (float)red1, (float)green1, (float)blue1 );

			// Vertex 2 Setup
			float z1 = 5 * (float)(j + 1)/(float)wormholeImage->ny;
			float y1 = 5 * color1;
			glVertex3f( x, y1, z1);
		}
	}
	glEnd();
	/******** End Draw Wormhole ********/

	glEndList();
	printf("Exited: genWormholeDisplayList()\n");
}

/* Function to draw the Track */
void drawTrack()
{
	/* Select four control points to base the current 
	 * segment of the spline on.
	 */
	Vec3f control1( 0, 0, 0 );
	Vec3f control2( 0, 0, 0 );
	Vec3f control3( 0, 0, 0 );
	Vec3f control4( 0, 0, 0 );
	Vec3f currentP( 0, 0, 0 );

	/* Iterate through points vector keeping track of 
	 * the current position in the vector using an
	 * iterator and a integer counter. 
	 */
	pointVectorIter ptsiter = g_Track.points().begin(); 
	for(int pointCounter = 0; ptsiter != g_Track.points().end(); ptsiter++)
	{
		Vec3f pt( *ptsiter );
		currentP = pt;

		switch( pointCounter )
		{
			case 0:
				control2 = currentP;
				break;
			case 1:
				control3 = currentP;
				break;
			case 2:
				control4 = currentP;
				drawSplineSegment( control1, control2, control3, control4 );
				break;
			default:
				control1 = control2;
				control2 = control3;
				control3 = control4;
				control4 = currentP;
				drawSplineSegment( control1, control2, control3, control4 );
		}
		
		pointCounter++;
	}
}

/* Function to draw individual segments of
 * the Track. Splines are drawn using a
 * Catmul-Rom spline definition.
 */
void drawSplineSegment( Vec3f pt1, Vec3f pt2, Vec3f pt3, Vec3f pt4 )
{
	glBegin(GL_LINE_STRIP);
	glColor3f( 1, 1, 1 );

	// use the parametric time value 0 to 1
	for(int i = 0; i != pixelWidth; i++) 
	{ 
		float u = (float)i / (float)(pixelWidth - 1);

		// Calculate blending functions 
		float p[4];
		p[0] =-1.0 * u * u * u + 2.0 * u * u - 1.0 * u + 0.0; 
		p[1] = 3.0 * u * u * u - 5.0 * u * u + 0.0 * u + 2.0; 
		p[2] =-3.0 * u * u * u + 4.0 * u * u + 1.0 * u + 0.0; 
		p[3] = 1.0 * u * u * u - 1.0 * u * u + 0.0 * u + 0.0;

		// Calculate the X, Y and Z position of the vertex and draw it
		float x, y, z;
		x = y = z = 0.0; 
		x = p[0]*pt1.x() + p[1]*pt2.x() + p[2]*pt3.x() + p[3]*pt4.x();
		y = p[0]*pt1.y() + p[1]*pt2.y() + p[2]*pt3.y() + p[3]*pt4.y();
		z = p[0]*pt1.z() + p[1]*pt2.z() + p[2]*pt3.z() + p[3]*pt4.z();

		glVertex3f( x, y, z );
	} 

	glEnd();
}
