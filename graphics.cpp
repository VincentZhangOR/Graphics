#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"

#include "heli.550"

#define TOP_BLADE_RADIUS		 5.0
#define BLADE_WIDTH		 0.4
#define REAR_BLADE_RADIUS		 1.5
#define MS_IN_THE_ANIMATION_CYCLE		300


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.

// sphere
//bool	Distort;		// global -- true means to distort the texture

struct point {
	float x, y, z;		// coordinates
	float nx, ny, nz;	// surface normal
	float s, t;		// texture coords
};

int		NumLngs, NumLats;
struct point *	Pts;

struct point *
PtsPointer( int lat, int lng )
{
	if( lat < 0 )	lat += (NumLats-1);
	if( lng < 0 )	lng += (NumLngs-1);
	if( lat > NumLats-1 )	lat -= (NumLats-1);
	if( lng > NumLngs-1 )	lng -= (NumLngs-1);
	return &Pts[ NumLngs*lat + lng ];
}

// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT Sample -- Joe Graphics" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

#define MS_PER_CYCLE		0.25
// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

float BOXSIZE;



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	YORTHO,
	ZORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	"Red",
	"Yellow",
	"Green",
	"Cyan",
	"Blue",
	"Magenta",
	"White",
	"Black"
};


// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


float *
Array3( float a, float b, float c )
{
        static float array[4];

        array[0] = a;
        array[1] = b;
        array[2] = c;
        array[3] = 1.;
        return array;
}

float *
Array4( float a, float b, float c, float d )
{
        static float array[4];

        array[0] = a;
        array[1] = b;
        array[2] = c;
        array[3] = d;
        return array;
}

float *
BlendArray3( float factor, float array0[3], float array1[3] )
{
        static float array[4];

        array[0] = factor * array0[0]  +  ( 1.f - factor ) * array1[0];
        array[1] = factor * array0[1]  +  ( 1.f - factor ) * array1[1];
        array[2] = factor * array0[2]  +  ( 1.f - factor ) * array1[2];
        array[3] = 1.;
        return array;
}

float *
MulArray3( float factor, float array0[3] )
{
        static float array[4];

        array[0] = factor * array0[0];
        array[1] = factor * array0[1];
        array[2] = factor * array0[2];
        array[3] = 1.;
        return array;
}
// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };
const float   para        = 0.05;
const float   SunRadius      = 4.36/5/para;
const float   MercuryRadius  = 0.015/para;
const float   VenusRadius  = 0.038/para;
const float   EarthRadius  = 0.04/para;
const float   MarsRadius  = 0.021/para;
const float   JupiterRadius  = 0.44/para;
const float   SaturnRadius  = 0.36/para;
const float   UranusRadius  = 0.16/para;
const float   NeptuneRadius  = 0.15/para;

const float   newp       = 0.25;
const float   MercuryDis  = 5/newp;
const float   VenusDis  = 9.3/newp;
const float   EarthDis  = 12.9/newp;
const float   MarsDis  = 19.7/newp;
const float   JupiterDis  = 67.1/newp;
const float   SaturnDis  = 123/newp;
const float   UranusDis  = 247/newp;
const float   NeptuneDis  = 387/newp;
const int     slices      = 60;
const int     stacks      = 60;


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
//int		DistortOn;				// != 0 means to distort
//int		TextOn;
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
GLuint	SunList;				// distort object display list
GLuint	MercuryList;				// distort object display list
GLuint	VenusList;				// distort object display list
GLuint	EarthList;		// undistort object display list
GLuint	MarsList;		// undistort object display list
GLuint	JupiterList;		// undistort object display list
GLuint	SaturnList;		// undistort object display list
GLuint	UranusList;		// undistort object display list
GLuint	NeptuneList;		// undistort object display list
GLuint	HeliList;				//Heli display list
GLuint	TopBladeList;			//Top Blade display list
GLuint	RearBladeList;			//Rear Blade display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
float	Time;
float	BladeTime;
bool	Light0On;
bool	Light1On;
bool	Light2On;
bool	Freeze;

float	Speed = 1;
float	CurrentSpeed = 1;
float	CurrentX = -35;
float	CurrentY = -10;
float	CurrentZ = 35;
float	DirectionX = 0;
float	DirectionZ = 0;
float	UpDown = 0;

int 	width;
int 	height;

//float	r;
//float	g;
//float	b;
//float	shininess;

int 	Outside;				// = 0 means to view from outside

float White[ ] = { 1.,1.,1.,1. };

unsigned char *Texture;




// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoViewMenu( int );
//void	DoDistortMenu( int );
//void	DoTextMenu( int );
void	DoColorMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void	Axes( float );
void	HsvRgb( float[3], float [3] );

void	DrawPoint( struct point *);
unsigned char *BmpToTexture( char *, int *, int * );

void	SetMaterial( float, float, float,  float);
void	SetPointLight( int, float, float, float,  float, float, float );
void	SetSpotLight( int, float, float, float,  float, float, float, float, float, float );

float	Dot( float[3], float [3] );
void	Cross( float [3], float [3], float [3] );
float	Unit( float [3], float [3] );

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)
	

	glutInit( &argc, argv );


	// setup all the graphics stuff:

	InitGraphics( );


	// create the display structures that will not change:

	InitLists( );


	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );


	// setup all the user interface stuff:

	InitMenus( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	// force a call to Display( ) next time it is convenient:
	int ms = glutGet( GLUT_ELAPSED_TIME );
	//ms %= MS_PER_CYCLE;
	Time = (float)ms / (float)MS_PER_CYCLE;
	if ((WhichProjection == ZORTHO) || (WhichProjection == PERSP))
		Time /= 3;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if ( Outside != 1)
	{
		if( WhichProjection == YORTHO )
			glOrtho( -800., 800.,     -800., 800.,     0.1, 1000. );
		else if ( WhichProjection == ZORTHO )
			glOrtho( -50., 50.,     -50., 50.,     0.1, 1000. );
		else
			gluPerspective( 90., 1.,	0.1, 10000. );
	}
	else
	{
		WhichProjection = PERSP;
		//glOrtho( -5., 5.,     -5., 5.,     -1, 1000. );
		gluPerspective( 90., 1.,	0.1, 10000. );
	}


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	// set the eye position, look-at position, and up-vector:
	if ( Outside != 1)
	{
		if (WhichProjection == YORTHO )
			gluLookAt( -770., 800., 0,     -770., 0., 0.,     0., 0, 1 );
		else if (WhichProjection == ZORTHO )
			gluLookAt( 0., 0., 50,     0., 0., 0.,     0., 1, 0 );
		else
			gluLookAt( -30, 0., 30.,     -30, 0., 0.,     0., 1., 0. );
	}
	
	else
	{ 
		//Reset();
		gluLookAt( CurrentX+DirectionX-0.4, 1.8+CurrentY+UpDown, CurrentZ-DirectionZ-4.9,     CurrentX+DirectionX-0.4, 1.8+CurrentY+UpDown, CurrentZ-DirectionZ-5.,     0., 1., 0. );
	}


	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );


	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );


	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}


	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}
	// set light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,MulArray3(.3,White));
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	//glDisable(GL_LIGHTING);
	SetPointLight( GL_LIGHT0, 0, 0, 0.,  1, 1, 1 );
	
	SetSpotLight( GL_LIGHT1, CurrentX+DirectionX,CurrentY+UpDown,CurrentZ-DirectionZ,0,0,-1,  1, 1, 1 );
	//glDisable(GL_LIGHT0);
	
	
	glEnable(GL_LIGHTING);
	if (Light0On)
		glEnable(GL_LIGHT0);
	else
		glDisable(GL_LIGHT0);
	if (Light1On)
		glEnable(GL_LIGHT1);
	else
		glDisable(GL_LIGHT1);
	glEnable( GL_NORMALIZE );
	/*
	if (Light0On)
	{
		glColor3f(r,g,b);
		glTranslatef(5-Time*10, 1.5, 0.);
			glutSolidSphere(0.05, 10, 10);
		glTranslatef(-5+Time*10, -1.5, 0.);
	}
	
	r=1.0;
	g=0;
	b=0;
	SetPointLight( GL_LIGHT1, 0., 0., 0.,  r, g, b );
	if (Light1On)
	{
		glColor3f(r,g,b);
		glutSolidSphere(0.05, 10, 10);
	}
	
	r=1.0;
	g=1.0;
	b=1.0;
	SetSpotLight( GL_LIGHT2, 2.5, 0., 0, -1., 0., 0., r, g, b );
	if (Light2On)
	{
		glColor3f(r,g,b);
		glTranslatef(2.5, 0, 0.);
			glutSolidSphere(0.05, 10, 10);
		glTranslatef(-2.5, 0, 0.);
	}

	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );
	glEnable(GL_LIGHTING);
	if (Light0On)
		glEnable(GL_LIGHT0);
	else
		glDisable(GL_LIGHT0);
	if (Light1On)
		glEnable(GL_LIGHT1);
	else
		glDisable(GL_LIGHT1);
	if (Light2On)
		glEnable(GL_LIGHT2);
	else
		glDisable(GL_LIGHT2);

	r=1.0;
	g=0;
	b=0;
	shininess = 10;
	SetMaterial(r, g, b, shininess);
	glShadeModel(GL_SMOOTH);
	glTranslatef(-1.0,0.3,-2);
	glRotatef(45, 1, 1, 1);
	//glCallList( BoxList );
	//glRotatef(90, 0, 1, 0);
	//glutSolidCone(0.5, 0.6, 100, 100);
	glutSolidCube(0.5);
	//glutSolidTeapot(0.5);
	glRotatef(-45, 1, 1, 1);
	glTranslatef(1.0,-0.3,2);
	
	r=0.0;
	g=0.0;
	b=1.0;
	shininess = 0.5;
	SetMaterial(r, g, b, shininess);
	glShadeModel(GL_FLAT);
	glTranslatef(0,1.5,0);
	glRotatef(90, 0, 1, 0);
	glutSolidTorus(0.15,0.45,30,30);
	glRotatef(-90, 0, 1, 0);
	glTranslatef(0,-1.5,0);
	
	//glCallList( CubeList );
	r=0.0;
	g=1.0;
	b=0.0;
	shininess = 30;
	SetMaterial(r, g, b, shininess);
	glShadeModel(GL_SMOOTH);
	glRotatef(360 * Time * 3, 0, 0, 1);
	glTranslatef(0,-1.5,0);
	glutSolidSphere(0.25, 100, 100);
	glTranslatef(0,1.5,0);
	glRotatef(-360 * Time * 3, 0, 0, 1);


	// draw the current object:
	//glEnable( GL_TEXTURE_2D );
	//glRotatef(360 * Time, 0., 1, 0);
	
	if ( TextOn == 0)
	{
		glTranslatef(1,0,1);
		glRotatef(360 * Time * 2, 0, 1, 0);
			glCallList( NotDistortSphereList );
		glRotatef(-360 * Time * 2, 0, 1, 0);
		glTranslatef(-1,0,-1);
	}
	else
	{
		r=0.5;
		g=0.5;
		b=0.5;
		shininess = 5;
		SetMaterial(r, g, b, shininess);
		glShadeModel(GL_SMOOTH);
		glTranslatef(1,0,1);
		glRotatef(360 * Time * 2, 0, 1, 0);
			glCallList( SphereList );
		glRotatef(-360 * Time * 2, 0, 1, 0);
		glTranslatef(-1,0,-1);
	}
	*/
	glRotatef(Time/500,0,1,0);
	glCallList( SunList );
	glRotatef(-Time/500,0,1,0);
	
	glRotatef(Time/sqrt(pow(0.10*MercuryDis,3)),0,1,0);
	glTranslatef(MercuryDis,0,0);
	glRotatef(Time/1000,0,1,0);
	glCallList( MercuryList );
	glRotatef(-Time/1000,0,1,0);
	glTranslatef(-MercuryDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*MercuryDis,3)),0,1,0);

	glRotatef(Time/sqrt(pow(0.10*VenusDis,3)),0,1,0);
	glTranslatef(VenusDis,0,0);
	glRotatef(Time/5000,0,1,0);
	glCallList( VenusList );
	glRotatef(-Time/5000,0,1,0);
	glTranslatef(-VenusDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*VenusDis,3)),0,1,0);
	
	glRotatef(Time/sqrt(pow(0.10*EarthDis,3)),0,1,0);
	glTranslatef(EarthDis,0,0);
	glRotatef(Time/20,0,1,0);
	glCallList( EarthList );
	glRotatef(-Time/20,0,1,0);
	glTranslatef(-EarthDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*EarthDis,3)),0,1,0);
	
	glRotatef(Time/sqrt(pow(0.10*MarsDis,3)),0,1,0);
	glTranslatef(MarsDis,0,0);
	glRotatef(Time/20,0,1,0);
	glCallList( MarsList );
	glRotatef(-Time/20,0,1,0);
	glTranslatef(-MarsDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*MarsDis,3)),0,1,0);
	
	glRotatef(Time/sqrt(pow(0.10*JupiterDis,3)),0,1,0);
	glTranslatef(JupiterDis,0,0);
	glRotatef(Time/7,0,1,0);
	glCallList( JupiterList );
	glRotatef(-Time/7,0,1,0);
	glTranslatef(-JupiterDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*JupiterDis,3)),0,1,0);
	
	glRotatef(Time/sqrt(pow(0.10*SaturnDis,3)),0,1,0);
	glTranslatef(SaturnDis,0,0);
	glRotatef(Time/8,0,1,0);
	glCallList( SaturnList );
	glRotatef(-Time/8,0,1,0);
	glTranslatef(-SaturnDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*SaturnDis,3)),0,1,0);
	
	glRotatef(Time/sqrt(pow(0.10*UranusDis,3)),0,1,0);
	glTranslatef(UranusDis,0,0);
	glRotatef(Time/15,0,1,0);
	glCallList( UranusList );
	glRotatef(-Time/15,0,1,0);
	glTranslatef(-UranusDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*UranusDis,3)),0,1,0);
	
	glRotatef(Time/sqrt(pow(0.10*NeptuneDis,3)),0,1,0);
	glTranslatef(NeptuneDis,0,0);
	glRotatef(Time/13,0,1,0);
	glCallList( NeptuneList );
	glRotatef(-Time/13,0,1,0);
	glTranslatef(-NeptuneDis,0,0);
	glRotatef(-Time/sqrt(pow(0.10*NeptuneDis,3)),0,1,0);
	//SetSpotLight( GL_LIGHT2, 2.5, 0., 0, -1., 0., 0., r, g, b );


	glDisable(GL_LIGHTING);
	glColor3f( 1., 1., 1. );
	

	glTranslatef(CurrentX+DirectionX,CurrentY+UpDown,CurrentZ-DirectionZ);
	CurrentX+=DirectionX;
	CurrentY+=UpDown;
	CurrentZ-=DirectionZ;
	DirectionX = 0;
	UpDown = 0;
	DirectionZ = 0;
	//glTranslatef(0,0,5);
	//glScalef(0.1,0.1,0.1);
	//BladeTime = glutGet( GLUT_ELAPSED_TIME ) / 10 ;
	glCallList( HeliList );
	glTranslatef(0.,2.9,-2.);
	glRotatef(360 * Time, 0., 1, 0);
	glCallList( TopBladeList );
	glRotatef(-360 * Time, 0., 1, 0);
	glTranslatef(0.,-2.9,2.);
	glTranslatef( .5, 2.5, 9 );
	glRotatef(360 * Time * 2, 1, 0, 0);
	glCallList( RearBladeList );
	glRotatef(-360 * Time * 2, 1, 0, 0);
	glTranslatef( -0.5, -2.5, -9 );
	//glScalef(10,10,10);
	//glTranslatef(35,10,Time/1000);
	//glTranslatef(0,0,-5);


	// draw some gratuitous text that just rotates on top of the scene:
	glDisable(GL_LIGHTING);
	glDisable( GL_DEPTH_TEST );

	//DoRasterString( 0., 1., 0., "Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1., 1., 1. );
	DoRasterString( 5., 5., 0., "He Zhang" );


	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}

void
DoViewMenu( int id )
{
	Outside = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

/*
void
DoDistortMenu( int id )
{
	DistortOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoTextMenu( int id )
{
	TextOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}
*/

void
DoColorMenu( int id )
{
	WhichColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );
	
	int viewmenu = glutCreateMenu( DoViewMenu );
	glutAddMenuEntry( "Outside",  0 );
	glutAddMenuEntry( "Inside",   1 );

	/*
	int distortmenu = glutCreateMenu( DoDistortMenu );
	glutAddMenuEntry( "Off",  1 );
	glutAddMenuEntry( "On",   0 );
	
	
	int textmenu = glutCreateMenu( DoTextMenu );
	glutAddMenuEntry( "Off",  1 );
	glutAddMenuEntry( "On",   0 );
	*/

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Y-Orthographic",  YORTHO );
	glutAddMenuEntry( "Z-Orthographic",  ZORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	//glutAddSubMenu(   "Distort",       distortmenu);
	//glutAddSubMenu(   "Text",          textmenu);
	glutAddSubMenu(   "View",          viewmenu);
	glutAddSubMenu(   "Colors",        colormenu);
	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutIdleFunc( Animate );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	//glutIdleFunc( NULL );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	glutSetWindow( MainWindow );
	// create the blob-ish object
	
	//Sun
	SunList = glGenLists( 1 );
	glNewList( SunList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	struct point top, bot;		// top, bottom points
	struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = SunRadius * x;
			p->y  = SunRadius * y;
			p->z  = SunRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = SunRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -SunRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Sun.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );


	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );
	
	
	
	
	//Mercury:
	MercuryList = glGenLists( 1 );
	glNewList( MercuryList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = MercuryRadius * x;
			p->y  = MercuryRadius * y;
			p->z  = MercuryRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = MercuryRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -MercuryRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Mercury.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );

	
	
	
	
	//Venus:
	VenusList = glGenLists( 1 );
	glNewList( VenusList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = VenusRadius * x;
			p->y  = VenusRadius * y;
			p->z  = VenusRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = VenusRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -VenusRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Venus.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );

	
	
	
	
		//Earth:
	EarthList = glGenLists( 1 );
	glNewList( EarthList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = EarthRadius * x;
			p->y  = EarthRadius * y;
			p->z  = EarthRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = EarthRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -EarthRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Earth.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );
	
	
	
	
	
		//Mars:
	MarsList = glGenLists( 1 );
	glNewList( MarsList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = MarsRadius * x;
			p->y  = MarsRadius * y;
			p->z  = MarsRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = MarsRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -MarsRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Mars.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );
	
	
	
		//Jupiter:
	JupiterList = glGenLists( 1 );
	glNewList( JupiterList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = JupiterRadius * x;
			p->y  = JupiterRadius * y;
			p->z  = JupiterRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = JupiterRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -JupiterRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Jupiter.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );
	
	
	
		//Saturn:
	SaturnList = glGenLists( 1 );
	glNewList( SaturnList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = SaturnRadius * x;
			p->y  = SaturnRadius * y;
			p->z  = SaturnRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = SaturnRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -SaturnRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Saturn.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );
	
	
	
	
		//Uranus:
	UranusList = glGenLists( 1 );
	glNewList( UranusList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = UranusRadius * x;
			p->y  = UranusRadius * y;
			p->z  = UranusRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = UranusRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -UranusRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Uranus.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );
	
	
	
	
	
		//Neptune:
	NeptuneList = glGenLists( 1 );
	glNewList( NeptuneList, GL_COMPILE );
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	//struct point top, bot;		// top, bottom points
	//struct point *p;
	// set the globals:
	NumLngs = slices;
	NumLats = stacks;
	if( NumLngs < 3 )
		NumLngs = 3;
	if( NumLats < 3 )
		NumLats = 3;
	// allocate the point data structure:
	Pts = new struct point[ NumLngs * NumLats ];
	// fill the Pts structure:
	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng );
			float z = -xz * sin( lng );
			p = PtsPointer( ilat, ilng );
			p->x  = NeptuneRadius * x;
			p->y  = NeptuneRadius * y;
			p->z  = NeptuneRadius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = ( lng + M_PI    ) / ( 2.*M_PI );
			p->t = ( lat + M_PI/2. ) / M_PI;
		}
	}
	top.x =  0.;		top.y  = NeptuneRadius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;
	bot.x =  0.;		bot.y  = -NeptuneRadius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;
	// connect the north pole to the latitude NumLats-2:
	//glTranslatef(0.0, -2, 0.0);
	Texture = BmpToTexture("Neptune.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( NumLats-1, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng );
			DrawPoint( p );

			p = PtsPointer( NumLats-2, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( NumLats-1, ilng+1 );
			DrawPoint( p );
		}
		glEnd( );
		// connect the south pole to the latitude 1:
		glBegin( GL_QUADS );
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( 0, ilng );
			DrawPoint( p );

			p = PtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( 1, ilng );
			DrawPoint( p );
		}
		glEnd( );
		// connect the other 4-sided polygons:
		glBegin( GL_QUADS );
		for( int ilat = 2; ilat < NumLats-1; ilat++ )
		{
			for( int ilng = 0; ilng < NumLngs-1; ilng++ )
			{
				p = PtsPointer( ilat-1, ilng );
				DrawPoint( p );

				p = PtsPointer( ilat-1, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng+1 );
				DrawPoint( p );

				p = PtsPointer( ilat, ilng );
				DrawPoint( p );
			}
		}
		glEnd( );
		//glTranslatef(0.0, 2, 0.0);
	glDisable( GL_TEXTURE_2D );
	delete [ ] Pts;
	Pts = NULL;
	glEndList( );
	
	
	
	
	// create heli:
	int i;
	struct Helip *p0, *p1, *p2;
	struct tri *tp;
	float p01[3], p02[3], n[3];
	
	HeliList = glGenLists( 1 );
	glNewList(HeliList, GL_COMPILE);
	//glTranslatef(0,0,12.5);
	glPushMatrix( );
	glTranslatef( 0., -1., 0. );
	glRotatef(  97.,   0., 1., 0. );
	glRotatef( -15.,   0., 0., 1. );
	/*
	Texture = BmpToTexture("Camo.bmp", &width, &height);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	
	glEnable( GL_TEXTURE_2D );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture );
	*/
		glBegin( GL_TRIANGLES );
			for( i=0, tp = Helitris; i < Helintris; i++, tp++ )
			{
				p0 = &Helipoints[ tp->p0 ];
				p1 = &Helipoints[ tp->p1 ];
				p2 = &Helipoints[ tp->p2 ];

				/* fake "lighting" from above:			*/

				p01[0] = p1->x - p0->x;
				p01[1] = p1->y - p0->y;
				p01[2] = p1->z - p0->z;
				p02[0] = p2->x - p0->x;
				p02[1] = p2->y - p0->y;
				p02[2] = p2->z - p0->z;
				Cross( p01, p02, n );
				Unit( n, n );
				n[1] = fabs( n[1] );
				n[1] += .25;
				if( n[1] > 1. )
					n[1] = 1.;
				glColor3f( 0., n[1], 0. );

				glVertex3f( p0->x, p0->y, p0->z );
				glVertex3f( p1->x, p1->y, p1->z );
				glVertex3f( p2->x, p2->y, p2->z );
			}
		glEnd( );
	glPopMatrix( );
	glEndList();

	// create top blades:

	TopBladeList = glGenLists( 1 );
	glNewList(TopBladeList, GL_COMPILE);
	//glTranslatef( 0., 2.9, -2 );
		glBegin( GL_TRIANGLES );
			glColor3f(1., 1., 1.);
			glVertex2f(  TOP_BLADE_RADIUS,  BLADE_WIDTH/2. );
			glVertex2f(  0., 0. );
			glVertex2f(  TOP_BLADE_RADIUS, -BLADE_WIDTH/2. );

			glVertex2f( -TOP_BLADE_RADIUS, -BLADE_WIDTH/2. );
			glVertex2f(  0., 0. );
			glVertex2f( -TOP_BLADE_RADIUS,  BLADE_WIDTH/2. );
		glEnd( );
	//glTranslatef( 0., -2.9, 2 );
	glEndList();
	
	// create rear blades:

	RearBladeList = glGenLists( 1 );
	glNewList(RearBladeList, GL_COMPILE);
	//glTranslatef( 0., -2.9, 2 );
	//glTranslatef( .5, 2.5, 9 );
		glBegin( GL_TRIANGLES );
			glColor3f(1., 1., 1.);
			glVertex2f(  BLADE_WIDTH/2., REAR_BLADE_RADIUS  );
			glVertex2f(  0., 0. );
			glVertex2f(  -BLADE_WIDTH/2.,REAR_BLADE_RADIUS  );

			glVertex2f( -BLADE_WIDTH/2., -REAR_BLADE_RADIUS );
			glVertex2f(  0., 0. );
			glVertex2f( BLADE_WIDTH/2., -REAR_BLADE_RADIUS  );
		glEnd( );
	//glTranslatef( -0.5, -2.5, -9 );
	glEndList();

	
	
	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'y':
		case 'Y':
			WhichProjection = YORTHO;
			break;
		
		case 'z':
		case 'Z':
			WhichProjection = ZORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler
		
		case 'f':
		case 'F':
			Freeze = ! Freeze;
			if( Freeze )
				glutIdleFunc( NULL );
			else
				glutIdleFunc( Animate );
			break;
		
		case '0':
			Light0On = ! Light0On;
			break;
		
		case '1':
			Light1On = ! Light1On;
			break;
			
		case 'w':
		case 'W':
			DirectionZ += 0.1*Speed;
			break;
		
		case 's':
		case 'S':
			DirectionZ += -0.1*Speed;
			break;
			
		case 'a':
		case 'A':
			DirectionX += -0.1*Speed;
			break;
		
		case 'd':
		case 'D':
			DirectionX += +0.1*Speed;
			break;
		
		case '>':
			UpDown += 0.1*Speed;
			break;
		
		case '<':
			UpDown += -0.1*Speed;
			break;
		
		case '+':
			Speed += 1;
			break;
		
		case '-':
			Speed += -1;
			break;

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 0;
	//DistortOn = 1;
	//TextOn = 1;
	DebugOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = ZORTHO;
	Xrot = Yrot = 0.;
	Light0On = 1;
	Light1On = 0;
	//Light2On = 0;
	Freeze = 0; 
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r, g, b;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
DrawPoint( struct point *p )
{
	glNormal3f( p->nx, p->ny, p->nz );
	glTexCoord2f( p->s, p->t );
	glVertex3f( p->x, p->y, p->z );
}

int	ReadInt( FILE * );
short	ReadShort( FILE * );


struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };



/**
 ** read a BMP file into a Texture:
 **/

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not 0x4d42, the file is not a bmp:

	if( FileHeader.bfType != 0x4d42 )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}


	FileHeader.bfSize = ReadInt( fp );
	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );
	FileHeader.bfOffBits = ReadInt( fp );


	InfoHeader.biSize = ReadInt( fp );
	InfoHeader.biWidth = ReadInt( fp );
	InfoHeader.biHeight = ReadInt( fp );

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	InfoHeader.biBitCount = ReadShort( fp );
	InfoHeader.biCompression = ReadInt( fp );
	InfoHeader.biSizeImage = ReadInt( fp );
	InfoHeader.biXPelsPerMeter = ReadInt( fp );
	InfoHeader.biYPelsPerMeter = ReadInt( fp );
	InfoHeader.biClrUsed = ReadInt( fp );
	InfoHeader.biClrImportant = ReadInt( fp );


	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );


	texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\b" );
		return NULL;
	}


	// extra padding bytes:

	numextra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;


	// we do not support compression:

	if( InfoHeader.biCompression != birgb )
	{
		fprintf( stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}
	


	rewind( fp );
	fseek( fp, 14+40, SEEK_SET );

	if( InfoHeader.biBitCount == 24 )
	{
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( e = 0; e < numextra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}



int
ReadInt( FILE *fp )
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	b2 = fgetc( fp );
	b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}


short
ReadShort( FILE *fp )
{
	unsigned char b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}

void
SetMaterial( float r, float g, float b,  float shininess )
{
	glMaterialfv( GL_BACK, GL_EMISSION, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_BACK, GL_AMBIENT, MulArray3( .4f, White ) );
	glMaterialfv( GL_BACK, GL_DIFFUSE, MulArray3( 1., White ) );
	glMaterialfv( GL_BACK, GL_SPECULAR, Array3( 0., 0., 0. ) );
	glMaterialf ( GL_BACK, GL_SHININESS, 5.f );

	glMaterialfv( GL_FRONT, GL_EMISSION, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_FRONT, GL_AMBIENT, Array3( r, g, b ) );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, Array3( r, g, b ) );
	glMaterialfv( GL_FRONT, GL_SPECULAR, MulArray3( .8f, White ) );
	glMaterialf ( GL_FRONT, GL_SHININESS, shininess );
}

void
SetPointLight( int ilight, float x, float y, float z,  float r, float g, float b )
{
	glLightfv( ilight, GL_POSITION,  Array3( x, y, z ) );
	glLightfv( ilight, GL_AMBIENT,   Array3( 0., 0., 0. ) );
	glLightfv( ilight, GL_DIFFUSE,   Array3( r, g, b ) );
	glLightfv( ilight, GL_SPECULAR,  Array3( r, g, b ) );
	glLightf ( ilight, GL_CONSTANT_ATTENUATION, 1. );
	glLightf ( ilight, GL_LINEAR_ATTENUATION, 0. );
	glLightf ( ilight, GL_QUADRATIC_ATTENUATION, 0. );
	glEnable( ilight );
}


void
SetSpotLight( int ilight, float x, float y, float z,  float xdir, float ydir, float zdir, float r, float g, float b )
{
	glLightfv( ilight, GL_POSITION,  Array3( x, y, z ) );
	glLightfv( ilight, GL_SPOT_DIRECTION,  Array3(xdir,ydir,zdir) );
	glLightf(  ilight, GL_SPOT_EXPONENT, 1. );
	glLightf(  ilight, GL_SPOT_CUTOFF, 45. );
	glLightfv( ilight, GL_AMBIENT,   Array3( 0., 0., 0. ) );
	glLightfv( ilight, GL_DIFFUSE,   Array3( r, g, b ) );
	glLightfv( ilight, GL_SPECULAR,  Array3( r, g, b ) );
	glLightf ( ilight, GL_CONSTANT_ATTENUATION, 1. );
	glLightf ( ilight, GL_LINEAR_ATTENUATION, 0. );
	glLightf ( ilight, GL_QUADRATIC_ATTENUATION, 0. );
	glEnable( ilight );
}


float
Dot( float v1[3], float v2[3] )
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}
void
Cross( float v1[3], float v2[3], float vout[3] )
{
	float tmp[3];
	tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
	tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
	tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Unit( float vin[3], float vout[3] )
{
	float dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];
	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}