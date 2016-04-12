//Modifier: Sabrina Smith
//Purpose: Homework 1

//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
//extern "C" {
#include "fonts.h"
//}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 1000
#define GRAVITY 0.1
#define BOXES 5
#define rnd() (float)rand()/(float)RAND_MAX

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape box[BOXES];
    Shape circle;
    Particle particle[MAX_PARTICLES];
    int n;
    bool startBubbler;
    bool touched;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);

int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;
    game.startBubbler = false;
    game.touched = false;

    //declare a circle shape
    game.circle.radius = 150.0;
    game.circle.center.x = 600;
    game.circle.center.y = 0;

    //declare box shapes
    for (int i = 0; i < BOXES; i++) {
	game.box[i].width = 100;
	game.box[i].height = 20;
	game.box[i].center.x = 150 + i*65;
	game.box[i].center.y = 500 - i*60;
    }

    //start animation
    while(!done) {
	while(XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_mouse(&e, &game);
	    done = check_keys(&e, &game);
	}
	movement(&game);
	render(&game);
	glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    cleanup_fonts();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "CMPS 335 HW 1 ~PARTICLE RAVE~");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
	std::cout << "\n\tcannot connect to X server\n" << std::endl;
	exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
	std::cout << "\n\tno appropriate visual found\n" << std::endl;
	exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	ButtonPress | ButtonReleaseMask |
	PointerMotionMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
	    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);

    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
	return;
    //std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd() - 0.5f;
    p->velocity.x = rnd() - 0.5f;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;

    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    makeParticle(game, e->xbutton.x, y);
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	savex = e->xbutton.x;
	savey = e->xbutton.y;
	if (++n < 10)
	    return;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	if (key == XK_Escape) {
	    return 1;
	}
	//You may check other keys here.
	switch(key) {
	    case XK_b:
		game->startBubbler = !(game->startBubbler);
		break; 
	}
    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if (game->startBubbler == 1) {
	for (int i = 0; i < MAX_PARTICLES; i++) {
	    makeParticle(game, 100, WINDOW_HEIGHT-80);
	}
    }

    if (game->n <= 0)
	return;

    for (int j = 0; j < game->n; j++) {
	p = &game->particle[j];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;

	//Gravity
	p->velocity.y -= 0.1;

	//check for collision with shapes...
	for (int k = 0; k < BOXES; k++) {
	    Shape *s = &game->box[k];
	    if (p->s.center.y >= (s->center.y - s->height) && 
		    p->s.center.y <= (s->center.y + s->height) &&
		    p->s.center.x >= (s->center.x - s->width) &&
		    p->s.center.x <= (s->center.x + s->width)) {
		p->velocity.y = 1.0 + ((rnd()*2.5)*0.4);
	    }
	}

	Shape *c = &game->circle;
	float d0, d1, dist;
	d0 = p->s.center.x - c->center.x;		
	d1 = p->s.center.y - c->center.y;
	dist = sqrt((d0 * d0) + (d1 * d1));
	if (dist <= c->radius) {
	    p->velocity.x += (d0/dist) * 2.25;
	    p->velocity.y += (d1/dist) * 2.25;
	    game->touched = true;
	}	

	//check for off-screen
	if (p->s.center.y < 0.0) {
	    std::cout << "off screen" << std::endl;
	    game->particle[j] = game->particle[game->n-1];
	    game->n--;
	}
    }
}

void render(Game *game)
{
    float w, h;
    Rect title1;
    Rect title2;
    Rect title3;
    Rect boxText;
    glClear(GL_COLOR_BUFFER_BIT);

    //Draw shapes...
    const int n = 40;
    static Vec vert[n];
    static bool drawCircle = true;

    if(drawCircle) {
	float angle=0.0, inc=(3.14159 * 2)/(float)n;
	for(int i = 0; i < n; i++) {
	    vert[i].x=cos(angle)*game->circle.radius;
	    vert[i].y=sin(angle)*game->circle.radius;
	    angle+=inc;
	}
	drawCircle = false;
    }

    glBegin(GL_TRIANGLE_FAN);
    if(game->touched)
	glColor3ub(rnd()*256, rnd()*256, rnd()*256);
    
    else {	
	glColor3ub(255,255,255);
    }

    for(int i = 0; i < n; i++) {
	glVertex2i(game->circle.center.x+vert[i].x,
		   game->circle.center.y+vert[i].y);
    }

    glEnd();


    //draw boxes
    Shape *s;
    for(int i = 0; i < BOXES; i++) {
	glColor3ub(255,255,255);
	s = &game->box[i];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w,-h);
	glVertex2i(-w, h);
	glVertex2i( w, h);
	glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
    }

    //draw all particles here
    glPushMatrix();
    glColor3ub(rnd()*256,rnd()*256,rnd()*256);
    for (int i = 0; i < game->n; i++) {
	Vec *c = &game->particle[i].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
    }


    title1.bot = WINDOW_HEIGHT - 30;
    title2.bot = WINDOW_HEIGHT - 40;
    title3.bot = WINDOW_HEIGHT - 50;
    title1.left = 10;
    title2.left = 10;
    title3.left = 10;
    title1.center = 0;
    title2.center = 0;
    title3.center = 0;

    ggprint8b(&title1, 36, 0x00ffffff, "Waterfall Model");
    ggprint8b(&title2, 36, 0x00ffffff, "~Rave Style~");
    ggprint8b(&title3, 36, 0x00ffffff, "Press B to activate Bubbler");

    for (int i = 0; i < BOXES; i++) {
	Shape *s;
	s = &game->box[i];
	w = s->width;
	h = s->height;
	boxText.bot = s->center.y - (h/2);
	boxText.left = s->center.x - (w/2);
	boxText.center = 0;
	if (i == 0)
	    ggprint16(&boxText, 36, 0, "Requirements");
	if (i == 1)
	    ggprint16(&boxText, 36, 0, "Design");
	if (i == 2)
	    ggprint16(&boxText, 36, 0, "Coding");
	if (i == 3)
	    ggprint16(&boxText, 36, 0, "Testing");
	if (i == 4)
	    ggprint16(&boxText, 36, 0, "Maintenance");
    }
}




