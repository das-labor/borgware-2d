/**
 * \defgroup unixsimulator Simulation of the Borg API for UNIX like platforms.
 */
/*@{*/

/**
 * @file main.c
 * @brief Simulator for Unix like platforms.
 * @author Martin Ongsiek, Peter Fuhrmann, Christian Kroll
 */

#ifdef OSX_
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <setjmp.h>

#include "../config.h"
#include "../display_loop.h"
#include "trackball.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <mosquitto.h>
int sockfd;
struct addrinfo *servinfo, *p;

/** Number of bytes per row. */
#define LINEBYTES (((NUM_COLS - 1) / 8) + 1)

/** Fake port for simulating joystick input. */
volatile unsigned char fakeport;
/** Flag which indicates if wait should jump to the menu if fire is pressed. */
volatile unsigned char waitForFire;
/** The simulated frame buffer of the borg. */
volatile unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];
/** Jump buffer which leads directly the menu. */
extern jmp_buf newmode_jmpbuf;

/** Width of the window. */
int WindWidth;
/** Height of the window. */
int WindHeight;

/** Rotation of the x-axis of the scene. */
float view_rotx = 0;
/** Rotation of the y-axis of the scene. */
float view_roty = 0;
/** Rotation of the z-axis of the scene. */
float view_rotz = 0;

/** GLUT window handle. */
int win;

char *host = "raum";
int port = 1883;
int keepalive = 60;
bool clean_session = true;
struct mosquitto *mosq = NULL;

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen){
		printf("%s %s\n", message->topic, message->payload);
	}else{
		printf("%s (null)\n", message->topic);
	}
	fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	int i;
	if(!result){
		/* Subscribe to broker information topics on successful connect. */
		mosquitto_subscribe(mosq, NULL, "$SYS/#", 2);
	}else{
		fprintf(stderr, "Connect failed\n");
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
	printf("%s\n", str);
}


/**
 * Simple wait function.
 * @param ms The requested delay in milliseconds.
 */
void wait(unsigned int ms) {
	if (waitForFire) {
		if (fakeport & 0x01) {
			longjmp(newmode_jmpbuf, 0xFEu);
		}
	}

	usleep(ms * 1000);
}


/**
 * Draw a LED in the given color (which is a list).
 * @param color List which contains a sphere.
 * @param pos_x x-coordinate
 * @param pos_y y-coordinate
 * @param pos_z z-coordinate
 */
void drawLED(int color, float pos_x, float pos_y, float pos_z) {
	glPushMatrix();
	glTranslatef(pos_x, pos_y, pos_z);
	glCallList(color);
	glPopMatrix();
}


/**
 * Draws the LED matrix.
 */
void display(void) {
	int x, y, z, level, color;
	tbReshape(WindWidth, WindHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(NUM_COLS * 2., 0., NUM_ROWS * 2.);
	tbMatrix();
	glRotatef(view_rotx, 1.0, 0.0, 0.0);
	glRotatef(view_roty, 0.0, 1.0, 0.0);
	glRotatef(view_rotz, 0.0, 0.0, 1.0);
	glTranslatef(-NUM_COLS * 2, 0., -NUM_ROWS * 2.);
	for (x = 0; x < 1; x++) {
		for (y = 0; y < NUM_COLS; y++) {
			for (z = 0; z < NUM_ROWS; z++) {
				color = 0;
				for (level = 0; level < NUMPLANE; level++) {
					if (pixmap[level][z % NUM_ROWS][y / 8] & (1 << y % 8)) {
						color = level + 1;
					}
				}
				drawLED(color, (float) y * 4.0, (float) x * 4.0,
						(float) (NUM_ROWS - 1 - z) * 4.0);
			}
		}
	}
	glPopMatrix();
	glutSwapBuffers();

	static uint8_t buffer[1280] = {0};
	static unsigned int divider = 0;
	unsigned int counter = 0;
	if (++divider == 10) {
		for (unsigned int y = 0; y < NUM_ROWS; ++y) {
			for (int lb = LINEBYTES -1; lb >= 0; --lb) {
				uint8_t pattern = pixmap[0][y][lb];
				for (int i = 8; i--;)
				{
					buffer[counter++] = ((1 << i) & pattern) ? '1' : '0';
				}
			}
		}
		mosquitto_publish(mosq, 0, "raum2/flipdot/image/set", 1280, buffer, 0, 0);
		divider = 0;
	}

	usleep(20000);
}


/**
 * Handler for processing key presses.
 * @param key The pressed key encoded in ASCII.
 * @param x X-position of the mouse pointer.
 * @param y Y-position of the mouse pointer.
 */
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q':
		printf("Quit\n");
		glutDestroyWindow(win);
		exit(0);
		break;
	case ' ':
		fakeport |= 0x01;
		break;
	case 'a':
		fakeport |= 0x02;
		break;
	case 'd':
		fakeport |= 0x04;
		break;
	case 's':
		fakeport |= 0x08;
		break;
	case 'w':
		fakeport |= 0x10;
		break;
	}
}


/**
 * Handler for processing key releases.
 * @param key The released key encoded in ASCII.
 * @param x X-position of the mouse pointer.
 * @param y Y-position of the mouse pointer.
 */
void keyboardup(unsigned char key, int x, int y) {
	switch (key) {
	case ' ':
		fakeport &= ~0x01;
		break;
	case 'a':
		fakeport &= ~0x02;
		break;
	case 'd':
		fakeport &= ~0x04;
		break;
	case 's':
		fakeport &= ~0x08;
		break;
	case 'w':
		fakeport &= ~0x10;
		break;
	}
}


/**
 * Relays mouse position and button state to the trackball implementation.
 * @param button Currently monitored button.
 * @param state State of that button.
 * @param x X-position of the mouse pointer.
 * @param y Y-position of the mouse pointer.
 */
void mouse(int button, int state, int x, int y) {
	tbMouse(button, state, x, y);
}


/**
 * Relays motion request to the trackball implementation.
 * @param x X-position for the motion direction.
 * @param y Y-position for the motion direction.
 */
void motion(int x, int y) {
	tbMotion(x, y);
}

/**
 * Updating the window size.
 * @param width Width of the window.
 * @param height Height of the window.
 */
void reshape(int width, int height) {

	tbReshape(width, height);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float) WindWidth / (float) WindWidth, 5., 1000.);
	gluLookAt(NUM_ROWS * 2., NUM_ROWS * 2. + 50., NUM_COLS * 2., NUM_ROWS * 2.,
			NUM_ROWS * 2., NUM_COLS * 2., 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	WindWidth = width;
	WindHeight = height;
}


/**
 * Handler for special keys (the arrow keys in particular) for adjusting the
 * view angle of the scene.
 * @param k The pressed special key using GLUT's nomenclature.
 * @param x X-position of the mouse pointer.
 * @param y Y-position of the mouse pointer.
 */
static void special(int k, int x, int y) {
	switch (k) {
	case GLUT_KEY_UP:
		view_rotx += 5.0;
		break;
	case GLUT_KEY_DOWN:
		view_rotx -= 5.0;
		break;
	case GLUT_KEY_LEFT:
		view_rotz += 5.0;
		break;
	case GLUT_KEY_RIGHT:
		view_rotz -= 5.0;
		break;
	default:
		return;
	}
	glutPostRedisplay();
}

int init_socket() {
    struct addrinfo hints = {.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
    int rv;
    int numbytes;

    if ((rv = getaddrinfo("raum", "8883", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    return 0;
}

void free_socket() {
    freeaddrinfo(servinfo);
    close(sockfd);
}

/**
 * Entry point for starting the display loop thread.
 * @param unused Not used. Only here to satisfy signature constraints.
 */
void *display_loop_run(void * unused) {
	display_loop();
	return 0;
}


/**
 * Main function for the simulator.
 * @param argc The argument count.
 * @param argv Command line arguments.
 * @return Exit codem, always zero.
 */
int main(int argc, char **argv) {
	//init_socket();
	int i;

	mosquitto_lib_init(); 	 

	mosq = mosquitto_new(NULL, clean_session, NULL);
	if(!mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	//mosquitto_log_callback_set(mosq, my_log_callback);
	//mosquitto_connect_callback_set(mosq, my_connect_callback);
	//mosquitto_message_callback_set(mosq, my_message_callback);
	//mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);

	if(mosquitto_connect(mosq, host, port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}

	WindHeight = 700;
	WindWidth = 700;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WindHeight, WindWidth);
	win = glutCreateWindow("16x16 Borg Simulator");

	// callback
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	// clearcolor & main loop
	glClearColor(0, 0, 0, 1.0);
	gluPerspective(60.0, (float) WindWidth / (float) WindWidth, 5., 1000.);
	gluLookAt(NUM_COLS * 2., NUM_COLS * 2. + 50., NUM_ROWS * 2., NUM_COLS * 2.,
			NUM_COLS * 2., NUM_ROWS * 2., 0.0, 0.0, 1.0);

	// init Call List for LED
	GLUquadric* quad = gluNewQuadric();
	glNewList(0, GL_COMPILE);
		glColor4f(0.8, 0.0, 0.0, 1.);
		gluSphere(quad, 1.0, 12, 12);
	glEndList();
	glNewList(1, GL_COMPILE);
		glColor4f(0.5, 0.0, 0.0, 1.);
		gluSphere(quad, 1.4, 12, 12);
	glEndList();
	glNewList(2, GL_COMPILE);
		glColor4f(0.7, 0.0, 0.0, 1.);
		gluSphere(quad, 1.55, 12, 12);
	glEndList();
	glNewList(3, GL_COMPILE);
		glColor4f(1.00, 0.0, 0.0, 1.);
		gluSphere(quad, 1.7, 12, 12);
	glEndList();

	tbInit(GLUT_LEFT_BUTTON);
	tbAnimate(GL_FALSE);

	pthread_t simthread;
	pthread_create(&simthread, NULL, display_loop_run, NULL);

	glutMainLoop();

//	free_socket();
    mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return 0;
}

/*@}*/
