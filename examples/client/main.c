#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
//#include <math.h>
//#include <pthread.h>
//#include <future>

#include "parsec-dso.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#define PARSEC_APP_CLIPBOARD_MSG 7
/*
#define WINDOW_W 1280
#define WINDOW_H 720
*/
#define CHANNELS          2
#define SAMPLE_RATE       48000
#define FRAMES_PER_PACKET 960

#if defined(_WIN32)
	#include <GL/gl.h>
	#include "glut.h"
	#if !defined(BITS)
		#define BITS 64
	#endif
	#if (BITS == 64)
		#define SDK_PATH "../../sdk/windows/parsec.dll"
	#else
		#define SDK_PATH "../../sdk/windows/parsec32.dll"
	#endif
#elif defined(__APPLE__)
	#define GL_SILENCE_DEPRECATION
	#include <OpenGL/gl.h>
	#define SDK_PATH "../../sdk/macos/libparsec.dylib"
#else
	#include <GL/gl.h>
	#define SDK_PATH "../../sdk/linux/libparsec.so"
#endif

#pragma warning(disable:4996)

//void initText2D(const char * texturePath);
//void printText2D(const char * text, int x, int y, int size);
//void cleanupText2D();

//unsigned int Text2DTextureID;
//void initText2D(const char * texturePath){
	// Initialize texture
	//Text2DTextureID = loadTGA_glfw(texturePath);
//}

/*
#define CHECK_KEY(keycode) \
    case ##keycode: \
        tts((#keycode)[4]); \
        break;
void tts(const char *text) {
    printf("Key pressed: %s", text);
    // .... do otherstuff
}
#define CHECK_KEY(keycode) \
    case ##keycode: \
        printf("%s\n", (#keycode)[4]); \
        break;
*/

//double euclideanDistance (int x1, int y1, int x2, int y2);

struct context {
	bool done;
	ParsecDSO *parsec;

	// Video
	float scale;
	SDL_Window *window;
	SDL_GLContext *gl;
	SDL_Surface *surface;
	SDL_Cursor *cursor;
	//SDL_Renderer *renderer;
	//SDL_Texture *texture;

	// Audio
	SDL_AudioDeviceID audio;
	bool playing;
	uint32_t minBuffer;
	uint32_t maxBuffer;
};

//typedef struct pair
//{
//	int x;
//	int y;
//} pair;
/*
static void logCallback(ParsecLogLevel level, const char *msg, void *opaque)
{
	opaque;

	printf("[%s] %s\n", level == LOG_DEBUG ? "D" : "I", msg);
}
*/
static void audio(const int16_t *pcm, uint32_t frames, void *opaque)
{
	struct context *context = (struct context *) opaque;

	uint32_t size = SDL_GetQueuedAudioSize(context->audio);
	uint32_t queued_frames = size / (CHANNELS * sizeof(int16_t));
	uint32_t queued_packets = queued_frames / FRAMES_PER_PACKET;

	if (context->playing && queued_packets > context->maxBuffer) {
		SDL_ClearQueuedAudio(context->audio);
		SDL_PauseAudioDevice(context->audio, 1);
		context->playing = false;

	} else if (!context->playing && queued_packets >= context->minBuffer) {
		SDL_PauseAudioDevice(context->audio, 0);
		context->playing = true;
	}

	SDL_QueueAudio(context->audio, pcm, frames * CHANNELS * sizeof(int16_t));
}

static void userData(struct context *context, uint32_t id, uint32_t bufferKey)
{
	char *msg = ParsecGetBuffer(context->parsec, bufferKey);

	if (msg && id == PARSEC_APP_CLIPBOARD_MSG)
		SDL_SetClipboardText(msg);

	ParsecFree(context->parsec, msg);
}

static void cursor(struct context *context, ParsecCursor *cursor, uint32_t bufferKey)
{
	if (cursor->imageUpdate) {
		uint8_t *image = ParsecGetBuffer(context->parsec, bufferKey);

		if (image) {
			SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(image, cursor->width, cursor->height,
				32, cursor->width * 4, 0xff, 0xff00, 0xff0000, 0xff000000);
			SDL_Cursor *sdlCursor = SDL_CreateColorCursor(surface, cursor->hotX, cursor->hotY);

			//context->renderer = SDL_CreateRenderer(context->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			//context->texture = SDL_CreateTextureFromSurface(context->renderer, context->surface);

			SDL_FreeCursor(context->cursor);
			context->cursor = sdlCursor;

			SDL_FreeSurface(context->surface);
			context->surface = surface;

			ParsecFree(context->parsec, image);
		}
	}
	/*
	if (cursor->modeUpdate) {
		if (SDL_GetRelativeMouseMode() && !cursor->relative) {
			SDL_SetRelativeMouseMode(SDL_DISABLE);
			SDL_WarpMouseInWindow(context->window, cursor->positionX, cursor->positionY);

		} else if (!SDL_GetRelativeMouseMode() && cursor->relative) {
			SDL_SetRelativeMouseMode(SDL_ENABLE);
		}
	}
	*/
}

static int32_t audioThread(void *opaque)
{
	struct context *context = (struct context *) opaque;

	while (!context->done)
		ParsecClientPollAudio(context->parsec, audio, 100, context);

	return 0;
}

bool teste = false;
bool aidmode = false;
bool pingpoint = false;
bool snapmouse = false;
bool steermouse = false;
bool vocalsound = false;
bool controlsharing = false;
bool mousecontrol = false;
bool keyboardcontrol = false;
bool gamepadcontrol = false;

bool presshold = false;
int pressmode = 0;
//bool presskey = false;
//bool holdkey = false;


bool mensagem = false;
//bool inputdone = false;

char text[100] = "";

int language = 0;

bool drawping = false;
double pingX = -5;
double pingY = -5;

double hostX = -5;
double hostY = -5;


double clientX = -5;
double clientY = -5;


int color = 0;
bool hide = false;

float xMargin = 0;
float yMargin = 0;

//HDC    hdc; 
//HGLRC  hglrc;

// Re-scale X values from [0; Width] to [-1; 1]
double converterX (double x, int w) {
	return 2 * (x / w) - 1;
}

// Re-scale Y values from [0; Height] to [1; -1]
double converterY (double y, int h) {
	return -2 * (y / h) + 1;
}

// Write a character letter in a position
int writeLetter(char letter, float x, float y){
	glRasterPos2f(x, y);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, letter);
	return 0;
}

// Write a sentence in a position
int writeSentence(char *sentence, float x, float y){
	float current = x;
	for (int i = 0; i < strlen(sentence); i++){
		writeLetter(sentence[i], current, y);
		current = current + xMargin;
		//printf("%d\n", glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, 'A'));
	}
	return 0;
}

static int32_t renderThread(void *opaque)
{
	struct context *context = (struct context *) opaque;
	
	/*
	char *data[100 * 100 * 3];
	for (int y = 0; y < 100; ++y){
		for (int x = 0; x < 100; ++x){
			data[y * 100 * 3 + x * 3] = 0xff;
			data[y * 100 * 3 + x * 3 + 1] = 0x00;
			data[y * 100 * 3 + x * 3 + 2] = 0x00;
		}
	}
	for (int y = 25; y < 75; ++y){
		for (int x = 25; x < 75; ++x){
			data[y * 100 * 3 + x * 3] = 0x00;
			data[y * 100 * 3 + x * 3 + 1] = 0x00;
			data[y * 100 * 3 + x * 3 + 2] = 0xff;
		}
	}
	*/
	
	SDL_GL_MakeCurrent(context->window, context->gl);
	SDL_GL_SetSwapInterval(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//int x1 = 50, x2 = 100, y1 = 50, y2 = 100;

	while (!context->done) {
		int32_t w = 0, h = 0;
		SDL_GetWindowSize(context->window, &w, &h);
		ParsecClientSetDimensions(context->parsec, DEFAULT_STREAM, w, h, context->scale);

		glViewport(0, 0, w, h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ParsecClientGLRenderFrame(context->parsec, DEFAULT_STREAM, NULL, NULL, 100);
		
		//glDrawPixels(100, 100, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glRectd(50, 50, 100, 100);

		//glOrtho(0, 1366, 0, 768, -1, 1);
		/*
		// x axis
		glColor3ub(254, 0, 0);
		glBegin(GL_LINES);
		glVertex3f(-1.0, 0.0, 0.0);
		glVertex3f(1.0, 0.0, 0.0);
		glEnd();

		// y axis
		glColor3ub(31, 255, 0);
		glBegin(GL_LINES);
		glVertex3f(0.0, -1.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);
		glEnd();

		// line
		glPointSize(1);
		glColor3ub(254, 0, 0);
		glBegin(GL_LINES);
		glVertex3f(-0.5, -0.5, 0.0);
		glVertex3f(0.5, 0.5, 0.0);
		glEnd();
		*/
		
		/*
		// point
		glPointSize(10);
		glBegin(GL_POINTS);
		glColor3ub(0, 255, 0);
		glVertex2d(hostX, hostY);
		glEnd();
		*/

		if (drawping && pingpoint) {
			// point
			glPointSize(10);
			glBegin(GL_POINTS);
			glColor3ub(51, 204, 255);
			glVertex2d(pingX, pingY);
			glEnd();
		}
		
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		/*
		// point
		glPointSize(20);
		glBegin(GL_POINTS);
		glColor3f(0, 0.5, 0);
		glVertex2d(0.7, 0.7);
		glEnd();

		// triangle
		glBegin(GL_TRIANGLES);
		glColor3ub(255, 171, 0);
		glVertex2d(0.1, 0.1);
		glVertex2d(0.3, 0.1);
		glVertex2d(0.2, 0.3);
		glEnd();
		*/

		//glDisable( GL_BLEND );

		//glClearColor(1,1,1,0);
 		//glMatrixMode(GL_PROJECTION);
 		//glOrtho(0,40,0,40,0,20);

		//glClear(GL_COLOR_BUFFER_BIT); 

		if (hide) {
			glBegin(GL_QUADS);
			glColor4f(0, 0, 0, 0.75);
			glVertex2d(-0.92, 0.85);
			glVertex2d(-0.90 + 17 * xMargin, 0.85);
			glVertex2d(-0.90 + 17 * xMargin, 0.85 - 2 * yMargin);
			glVertex2d(-0.92, 0.85 - 2 * yMargin);
			glEnd();
			switch (color) {
				case 0: glColor3f(0.0,1.0,0.0); break;
				//case 1: glColor3f(0.0,0.0,1.0); break;
				//case 2: glColor3f(1.0,0.0,0.0); break;
				case 1: glColor3f(0.0,1.0,1.0); break;
				case 2: glColor3f(1.0,1.0,0.0); break;
				//case 5: glColor3f(1.0,0.0,1.0); break;
				//case 6: glColor3f(0.0,0.0,0.0); break;
				case 3: glColor3f(1.0,1.0,1.0); break;
			}
			writeSentence("[F10] Show Menu", -0.900, 0.800);
		}
		else {
			// Transparent rectangle boxes
			if (!aidmode) {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 24 * xMargin, 0.85);
				glVertex2d(-0.92 + 24 * xMargin, 0.85 - 5 * yMargin);
				glVertex2d(-0.92, 0.85 - 5 * yMargin);
				glEnd();
			}
			else if (pingpoint || snapmouse || steermouse) {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 46 * xMargin, 0.85);
				glVertex2d(-0.92 + 46 * xMargin, 0.85 - 4 * yMargin);
				glVertex2d(-0.92, 0.85 - 4 * yMargin);
				glEnd();
			}
			else if (vocalsound) {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 29 * xMargin, 0.85);
				glVertex2d(-0.92 + 29 * xMargin, 0.85 - 17 * yMargin);
				glVertex2d(-0.92, 0.85 - 17 * yMargin);
				glEnd();
			}
			else if (controlsharing) {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 32 * xMargin, 0.85);
				glVertex2d(-0.92 + 32 * xMargin, 0.85 - 6 * yMargin);
				glVertex2d(-0.92, 0.85 - 6 * yMargin);
				glEnd();
			}
			else if (presshold) {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 35 * xMargin, 0.85);
				glVertex2d(-0.92 + 35 * xMargin, 0.85 - 5 * yMargin);
				glVertex2d(-0.92, 0.85 - 5 * yMargin);
				glEnd();
			}
			else if (mensagem) {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 53 * xMargin, 0.85);
				glVertex2d(-0.92 + 53 * xMargin, 0.85 - 8 * yMargin);
				glVertex2d(-0.92, 0.85 - 8 * yMargin);
				glEnd();
			}
			else if (mousecontrol && keyboardcontrol) {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 48 * xMargin, 0.85);
				glVertex2d(-0.92 + 48 * xMargin, 0.85 - 12 * yMargin);
				glVertex2d(-0.92, 0.85 - 12 * yMargin);
				glEnd();
			}
			else {
				glBegin(GL_QUADS);
				glColor4f(0, 0, 0, 0.75);
				glVertex2d(-0.92, 0.85);
				glVertex2d(-0.92 + 46 * xMargin, 0.85);
				glVertex2d(-0.92 + 46 * xMargin, 0.85 - 12 * yMargin);
				glVertex2d(-0.92, 0.85 - 12 * yMargin);
				glEnd();
			}

			switch (color) {
				case 0: glColor3f(0.0,1.0,0.0); break;
				//case 1: glColor3f(0.0,0.0,1.0); break;
				//case 2: glColor3f(1.0,0.0,0.0); break;
				case 1: glColor3f(0.0,1.0,1.0); break;
				case 2: glColor3f(1.0,1.0,0.0); break;
				//case 5: glColor3f(1.0,0.0,1.0); break;
				//case 6: glColor3f(0.0,0.0,0.0); break;
				case 3: glColor3f(1.0,1.0,1.0); break;
			}
			if (!aidmode){
				/*
				writeLetter('[', -0.800, 0.8);
				writeLetter('F', -0.775, 0.8);
				writeLetter('1', -0.750, 0.8);
				writeLetter('2', -0.8 - yMargin, 0.8);
				writeLetter(']', -0.700, 0.8);
				writeLetter(' ', -0.675, 0.8);
				writeLetter('A', -0.8 - 2 * yMargin, 0.8);
				*/
				writeSentence("Aid mode: Off", -0.900, 0.800);
				writeSentence("[F12] Toggle aid mode", -0.900, 0.8 - yMargin);
				writeSentence("[F11] Change UI color", -0.900, 0.8 - 2 * yMargin);
				writeSentence("[F10] Hide Menu", -0.900, 0.8 - 3 * yMargin);
			}
			if (aidmode){
				if (pingpoint) {
					writeSentence("Ping point mode: On", -0.900, 0.800);
					writeSentence("Click on the screen to send a ping point...", -0.900, 0.8 - yMargin);
					writeSentence("[F1] Untoggle ping point mode", -0.900, 0.8 - 2 * yMargin);
				}
				else if (snapmouse) {
					writeSentence("Snap mouse mode: On", -0.900, 0.800);
					writeSentence("Click on the screen to snap host's mouse...", -0.900, 0.8 - yMargin);
					writeSentence("[F2] Untoggle snap mouse mode", -0.900, 0.8 - 2 * yMargin);
				}
				else if (steermouse) {
					writeSentence("Steer mouse mode: On", -0.900, 0.800);
					writeSentence("Press the arrow keys to move host's mouse...", -0.900, 0.8 - yMargin);
					writeSentence("[F3] Untoggle steer mouse mode", -0.900, 0.8 - 2 * yMargin);
				}
				else if (vocalsound) {
					writeSentence("List of vocal sounds:", -0.900, 0.800);
					writeSentence("[Y] Yes!", -0.900, 0.8 - yMargin);
					writeSentence("[N] No!", -0.900, 0.8 - 2 * yMargin);
					writeSentence("[Z] Jazzy Sound!", -0.900, 0.8 - 3 * yMargin);
					writeSentence("[O] Okay!", -0.900, 0.8 - 4 * yMargin);
					writeSentence("[S] Shoot!", -0.900, 0.8 - 5 * yMargin);
					writeSentence("[R] Run!", -0.900, 0.8 - 6 * yMargin);
					writeSentence("[T] Stop!", -0.900, 0.8 - 7 * yMargin);
					writeSentence("[G] Great!", -0.900, 0.8 - 8 * yMargin);
					writeSentence("[W] Wait!", -0.900, 0.8 - 9 * yMargin);
					writeSentence("[J] Jump!", -0.900, 0.8 - 10 * yMargin);
					writeSentence("[Q] Quick!", -0.900, 0.8 - 11 * yMargin);
					writeSentence("[,] Correct Sound!", -0.900, 0.8 - 12 * yMargin);
					writeSentence("[.] Wrong Sound!", -0.900, 0.8 - 13 * yMargin);
					writeSentence("[ARROWS] Up/Down/Left/Right", -0.900, 0.8 - 14 * yMargin);
					writeSentence("[F4] Hide list", -0.900, 0.8 - 15 * yMargin);
				}
				else if (controlsharing) {
					writeSentence("Choose which input to control:", -0.900, 0.800);
					writeSentence("[M] Mouse", -0.900, 0.8 - yMargin);
					writeSentence("[K] Keyboard", -0.900, 0.8 - 2 * yMargin);
					writeSentence("[G] Gamepad", -0.900, 0.8 - 3 * yMargin);
					writeSentence("[F5] Go back", -0.900, 0.8 - 4 * yMargin);
				}
				else if (presshold) {
					writeSentence("Press a key to inform the host...", -0.900, 0.8 - yMargin);
					if (pressmode == 0) {
						writeSentence("Press Key mode: On", -0.900, 0.800);
						writeSentence("[ALT] Other 'Press Key' modes", -0.900, 0.8 - 2 * yMargin);
					}
					else if (pressmode == 1) {
						writeSentence("Hold Key mode: On", -0.900, 0.800);
						writeSentence("[ALT] Other 'Press Key' modes", -0.900, 0.8 - 2 * yMargin);
					}
					else if (pressmode == 2) {
						writeSentence("Double Press mode: On", -0.900, 0.800);
						writeSentence("[ALT] Other 'Press Key' modes", -0.900, 0.8 - 2 * yMargin);
					}
					else {
						writeSentence("Press Repeatedly mode: On", -0.900, 0.800);
						writeSentence("[ALT] Other 'Press Key' modes", -0.900, 0.8 - 2 * yMargin);
					}
					writeSentence("[F6] Untoggle press/hold key mode", -0.900, 0.8 - 3 * yMargin);
				}
				/*
				else if (holdkey) {
					writeSentence("Hold Key mode: On", -0.900, 0.800);
					writeSentence("Press a key to inform the host...", -0.900, 0.8 - yMargin);
					writeSentence("[F7] Untoggle hold key mode", -0.900, 0.8 - 2 * yMargin);
				}
				*/
				else if (mensagem) {
					writeSentence("Text-to-speech message mode: On", -0.900, 0.800);
					writeSentence("Write a sentence to inform the host...", -0.900, 0.8 - yMargin);
					writeSentence("Text:", -0.900, 0.8 - 2 * yMargin);
					writeSentence(text, -0.900 + 6 * xMargin, 0.8 - 2 * yMargin);
					writeSentence("Press [ENTER] to send message", -0.900, 0.8 - 3 * yMargin);
					writeSentence("Click on screen to send labelled ping point", -0.900, 0.8 - 4 * yMargin);
					writeSentence("Press the arrow keys to change language", -0.900, 0.8 - 5 * yMargin);
					switch (language) {
						case 0: writeSentence("(Portugues)", -0.900 + 40 * xMargin, 0.8 - 5 * yMargin); break;
						case 1: writeSentence("(English)", -0.900 + 40 * xMargin, 0.8 - 5 * yMargin); break;
					}
					writeSentence("[F7] Untoggle tts message mode", -0.900, 0.8 - 6 * yMargin);
				}
				else {
					writeSentence("Aid mode: On", -0.900, 0.800);
					if (mousecontrol && keyboardcontrol && gamepadcontrol) writeSentence("(CONTROLLING ALL INPUTS)", -0.900 + 13 * xMargin, 0.800);
					else if (mousecontrol && keyboardcontrol) writeSentence("(CONTROLLING MOUSE AND KEYBOARD)", -0.900 + 13 * xMargin, 0.800);
					else if (mousecontrol && gamepadcontrol) writeSentence("(CONTROLLING MOUSE AND GAMEPAD)", -0.900 + 13 * xMargin, 0.800);
					else if (keyboardcontrol && gamepadcontrol) writeSentence("(CONTROLLING KEYBOARD AND GAMEPAD)", -0.900 + 13 * xMargin, 0.800);
					else if (mousecontrol) writeSentence("(CONTROLLING THE MOUSE)", -0.900 + 13 * xMargin, 0.800);
					else if (keyboardcontrol) writeSentence("(CONTROLLING THE KEYBOARD)", -0.900 + 13 * xMargin, 0.800);
					else if (gamepadcontrol) writeSentence("(CONTROLLING THE GAMEPAD)", -0.900 + 13 * xMargin, 0.800);
					writeSentence("[F12] Untoggle aid mode", -0.900, 0.8 - yMargin);
					writeSentence("[F1] Send ping point of mouse position", -0.900, 0.8 - 2 * yMargin);
					writeSentence("[F2] Snap host's mouse to mouse position", -0.900, 0.8 - 3 * yMargin);
					writeSentence("[F3] Steer host's mouse", -0.900, 0.8 - 4 * yMargin);
					writeSentence("[F4] Show list of vocal sounds", -0.900, 0.8 - 5 * yMargin);
					writeSentence("[F5] Control sharing", -0.900, 0.8 - 6 * yMargin);
					writeSentence("[F6] Send 'Press' or 'Hold' key indication", -0.900, 0.8 - 7 * yMargin);
					//writeSentence("[F7] Send 'Hold Key' indication", -0.900, 0.200);
					writeSentence("[F7] Send voiced message/labelled ping point", -0.900, 0.8 - 8 * yMargin);
					writeSentence("[F11] Change UI color", -0.900, 0.8 - 9 * yMargin);
					writeSentence("[F10] Hide Menu", -0.900, 0.8 - 10 * yMargin);
				}
			}
		}

		/*
		// Client cursor point
		glPointSize(10);
		glBegin(GL_POINTS);
		switch (color) {
			case 0: glColor3f(0.0,1.0,0.0); break;
			//case 1: glColor3f(0.0,0.0,1.0); break;
			//case 2: glColor3f(1.0,0.0,0.0); break;
			case 1: glColor3f(0.0,1.0,1.0); break;
			case 2: glColor3f(1.0,1.0,0.0); break;
			//case 5: glColor3f(1.0,0.0,1.0); break;
			//case 6: glColor3f(0.0,0.0,0.0); break;
			case 3: glColor3f(1.0,1.0,1.0); break;
		}
		glVertex2d(clientX, clientY);
		glEnd();
		*/
		
		//glRasterPos2f(-0.5,0.5);
		//glutStrokeCharacter(GLUT_BITMAP_9_BY_15, 'B');
		//glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "eu gosto de queijo");
		glBegin(GL_LINES);  
		   //glVertex2i(20,9);
		   //glVertex2i(30,9);
		glEnd();

		glFlush();

		//glSwapBuffers(context->window);

		//SDL_Surface *surf = SDL_CreateRGBSurface(0, 500, 500, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
		
		//context->surface = SDL_GetWindowSurface(context->window);
		//SDL_Rect rect = {100,100,100,100};
		//uint32_t color = 0xffff33;
		//SDL_FillRect (context->surface, &rect, color);
		//SDL_BlitSurface(surf, &rect, context->surface, &rect);
		//SDL_UpdateWindowSurfaceRects(context->window, &rect, 1);
		//SDL_UpdateWindowSurface(context->window);

		SDL_GL_SwapWindow(context->window);
		glFinish();
		//SDL_FreeSurface(context->surface);
	}

	ParsecClientGLDestroy(context->parsec, DEFAULT_STREAM);
	SDL_GL_DeleteContext(context->gl);

	return 0;
}

static int32_t inputThread(void *opaque)
{
	struct context *context = (struct context *) opaque;

	//printf("Yoooo\n");

	SDL_Event ev;
	SDL_StartTextInput();
	POINT cursorPos;
	int posX = -1;
	int posY = -1;
	int clientResX, clientResY;
	clientResX = GetSystemMetrics(SM_CXSCREEN);
	clientResY = GetSystemMetrics(SM_CYSCREEN);
	int WINDOW_W = clientResX;
	int WINDOW_H = clientResY;

	while (!context->done){
		if (mensagem) {
			while (SDL_PollEvent(&ev) != 0) {
				if (ev.type == SDL_QUIT) {
					context->done = true;
				}
				else if(ev.type == SDL_TEXTINPUT || ev.type == SDL_KEYDOWN) {
					//printf("Pitonga.\n");
					//system("cls");
					if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_BACKSPACE && strlen(text) > 0) {
						text[strlen(text)-1] = '\0';
						//printf("BACK::: %s\n", text);
					}
					else if (ev.type == SDL_KEYDOWN && (ev.key.keysym.sym == SDLK_ESCAPE 
						|| ev.key.keysym.sym == SDLK_F8 || ev.key.keysym.sym == SDLK_F7
						|| ev.key.keysym.sym == SDLK_F6 || ev.key.keysym.sym == SDLK_F5
						|| ev.key.keysym.sym == SDLK_F4 || ev.key.keysym.sym == SDLK_F3
						|| ev.key.keysym.sym == SDLK_F2 || ev.key.keysym.sym == SDLK_F1
						|| ev.key.keysym.sym == SDLK_F12 || ev.key.keysym.sym == SDLK_F10)) {
						mensagem = false;
						drawping = false;
						text[0] = '\0';
						//printf("OUT::: %s\n", text);
					}
					else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_RETURN && strlen(text) > 0) {
						ParsecClientSendUserData(context->parsec, 42, text);
						printf("Message sent: %s\n", text);
					}
					else if (ev.type == SDL_KEYDOWN && (ev.key.keysym.sym == SDLK_RIGHT ||
						ev.key.keysym.sym == SDLK_LEFT)) {
						//language++;
						//if (language > 1) {
						//	language = 0;
						//}
						if (language == 0) {
							language = 1;
							ParsecClientSendUserData(context->parsec, 43, "English");
						}
						else {
							language = 0;
							ParsecClientSendUserData(context->parsec, 44, "Portugues");
						}
						//if (language == 0) ParsecClientSendUserData(context->parsec, 43, "Portugues");
						//else ParsecClientSendUserData(context->parsec, 44, "English");
						//printf("Message sent: %s\n", text);
					}
					/*
					else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_LEFT) {
						language--;
						if (language < 0) {
							language = 1;
						}
						if (language == 0) ParsecClientSendUserData(context->parsec, 43, "Portugues");
						else ParsecClientSendUserData(context->parsec, 44, "English");
						//printf("Message sent: %s\n", text);
					}
					*/
					else if (ev.type == SDL_TEXTINPUT) {
						strcat(text, ev.text.text);
						//printf("TEXT::: %s\n", text);
					}
				}
				else if (ev.type == SDL_MOUSEBUTTONDOWN && strlen(text) > 0) {
					//printf("LHALH\n");
					GetCursorPos(&cursorPos);
					posX = cursorPos.x;
					posY = cursorPos.y;
					//printf("Pos X: %d\n", posX);
					//printf("Pos Y: %d\n", posY);
					char px[5];
					char py[5];
					_itoa_s(posX, px, 5, 10);
					_itoa_s(posY, py, 5, 10);
					char* msg = (char *) malloc(20 + strlen(px) + strlen(py));
					strcpy(msg, px);
					strcat(msg, ",");
					strcat(msg, py);
					strcat(msg, ",");
					strcat(msg, text);
					ParsecClientSendUserData(context->parsec, 45, msg);
					printf("Labelled point sent to position (%d, %d)\n", posX, posY);
					drawping = true;
					double posX_aux = posX;
					double posY_aux = posY;
					pingX = converterX(posX_aux, WINDOW_W);
					pingY = converterY(posY_aux, WINDOW_H);
					printf("%s\n", msg);
				}
				
				else if (ev.type == SDL_MOUSEMOTION) {
					GetCursorPos(&cursorPos);
					clientX = converterX(cursorPos.x, WINDOW_W);
					clientY = converterY(cursorPos.y, WINDOW_H);
				}
				
				if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
					//printf("FECHOU\n");
					mensagem = false;
					drawping = false;
					text[0] = '\0';
					ParsecClientSendUserData(context->parsec, 3, "Labelled ping point mode off.");
				}
			}
		}
		//if (mensagem && !inputdone) {
			
			/*
			char *name = malloc(256);
    		printf("Enter name: \n");
    		fgets(name, 256, stdin);
    		if (name != NULL) {
    			printf("Your name is %s.\n", name);
    			inputdone = true;
    			mensagem = false;
    			//printf("Waiting...\n");
    		}
    		*/
		//}
	}
	//printf("***** \n");
	//printf("TEXT::: %s\n", text);

	SDL_StopTextInput();

	return 0;
}

int32_t main(int32_t argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: client sessionID peerID\n");
		return 1;
	}

	// Aid mode INSTRUCTIONS
	printf("********* PLAYING BY PROXY *********\n");
	printf("*********** INSTRUCTIONS ***********\n");
	printf("[F12] Toggle/untoggle aid mode\n");
	printf("[F1] Send ping point of mouse position\n");
	printf("[F2] Snap host's mouse to mouse position\n");
	printf("[F3] Steer host's mouse\n");
	printf("[F4] Show list of vocal sounds\n");
	printf("[F5] Toggle/untoggle control sharing\n");
	printf("[F6] Send 'Press Key' indication\n");
	//printf("[F7] Send 'Hold Key' indication\n");
	printf("[F7] Send voiced message/labelled ping point\n");
	printf("************************************\n");

	// Inicializar posições do rato
	int posX = -1;
	int posY = -1;
	//double d = -1;
	//pair pinned;
	//pinned.x = posX;
	//pinned.y = posY;

	//POINT pin;
	//pin.x = posX;
	//pin.y = posY;

	POINT cursorPos;

	// Get screen resolution and convert to string
	int clientResX, clientResY;
	clientResX = GetSystemMetrics(SM_CXSCREEN);
	clientResY = GetSystemMetrics(SM_CYSCREEN);
	xMargin = (float) clientResX / 100000;
	yMargin = (float) clientResY / 17000;
	//printf("X MARGIN::: %0.5f\n", xMargin);
	//printf("Y MARGIN::: %0.5f\n", yMargin);
	//printf("RES X: %d\n", clientResX);
	//printf("RES Y: %d\n", clientResY);
	char resX[10], resY[10];
	_itoa_s(clientResX, resX, 10, 10);
	_itoa_s(clientResY, resY, 10, 10);
	char* screenRes = (char *) malloc(2 + strlen(resX) + strlen(resY));
	strcpy_s(screenRes, 10, resX);
	strcat_s(screenRes, 11, ",");
	strcat_s(screenRes, 21, resY);

	int WINDOW_W = clientResX;
	int WINDOW_H = clientResY;

	int hostResX = -1;
	int hostResY = -1;

	struct context context = {0};

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);

	SDL_AudioSpec want = {0}, have;
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_S16;
	want.channels = CHANNELS;
	want.samples = 2048;

	context.minBuffer = 1; // The number of audio packets (960 frames) to buffer before we begin playing
	context.maxBuffer = 6; // The number of audio packets (960 frames) to buffer before overflow and clear

	context.audio = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

	context.window = SDL_CreateWindow("Parsec Client Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		/*1200, 400,*/ WINDOW_W, WINDOW_H, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN);

	context.gl = SDL_GL_CreateContext(context.window);
	#if !defined(__APPLE__)
		SDL_GL_MakeCurrent(context.window, NULL);
	#endif

	int32_t glW = 0;
	SDL_GL_GetDrawableSize(context.window, &glW, NULL);
	context.scale = (float) glW / (float) WINDOW_W;

	ParsecStatus e = ParsecInit(NULL, NULL, SDK_PATH, &context.parsec);
	if (e != PARSEC_OK) goto except;

	//ParsecSetLogCallback(context.parsec, logCallback, NULL);
	ParsecClientConnect(context.parsec, NULL, argv[1], argv[2]);

	SDL_Thread *render_thread = SDL_CreateThread(renderThread, "renderThread", &context);
	SDL_Thread *audio_thread = SDL_CreateThread(audioThread, "audioThread", &context);
	SDL_Thread *input_thread = SDL_CreateThread(inputThread, "inputThread", &context);

	////context.surface = SDL_GetWindowSurface(context.window);

	//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Título", "Wawi", context.window);

	//SDL_SetWindowFullscreen(context.window, true);
	//SDL_ShowCursor(SDL_ENABLE);

	//SDL_Window* janela = SDL_CreateWindow("Janela Teste", 100, 100,
	//		500, 500, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	//SDL_Surface* superficie = SDL_GetWindowSurface(janela);
	//SDL_Renderer *rend = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(0, 500, 500, 32, 0, 0xff, 0xff00, 0xff0000, 0xff000000);
	////SDL_Surface *surf = SDL_CreateRGBSurface(0, 500, 500, 64, 0, 0xff, 0xff00, 0xff0000, 0xff000000);
	//SDL_Texture *text = SDL_CreateTextureFromSurface(rend, surf);

	//context.renderer = SDL_CreateRenderer(context.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//context.texture = SDL_CreateTextureFromSurface(context.renderer, context.surface);
	
	//SDL_Rect rect = {50,50,50,50};
	//rect.x = 50;
	//rect.y = 50;
	//rect.w = 200;
	//rect.h = 200;
	//uint32_t color = SDL_MapRGB(context.surface->format, 255, 255, 0);
	//SDL_Color color = {0, 0, 255, 255};
	//uint32_t color = 0xffffff;

	char frase[100] = "Welcome to Playing By Proxy.";
	char charCommand[100];
	strcpy(charCommand, "espeak -v +f3 \"");
	strcat(charCommand, frase);
	strcat(charCommand, "\"");
	system(charCommand);

	////SDL_Rect rect = {100,100,100,100};
	////uint32_t color = 0xffff33;
	////SDL_FillRect (surf, &rect, color);
	//SDL_UpdateWindowSurface(context.window);
	
	//context.surface = SDL_SetVideoMode(800, 600, 0, NULL);

	//SDL_FillRect (superficie, &rect, color);
	//SDL_UpdateWindowSurface(janela);

	//SDL_Delay(4000);
	/*
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	*/

	while (!context.done) {
		////SDL_BlitSurface(surf, &rect, context.surface, &rect);
		////SDL_FillRect (context.surface, &rect, color);
		////SDL_UpdateWindowSurface(context.window);
		////SDL_GL_SwapWindow(context.window);

		//SDL_Delay(4000);
		//SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
		//SDL_RenderDrawRect(rend, &rect);
		
		//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		//SDL_SetRenderDrawColor(context.renderer, 0, 0, 0, 0);
		//SDL_RenderClear(context.renderer);

		/*
		//SDL_Rect rect = {cursorPos.x - 25, cursorPos.y - 25, 50, 50};
		SDL_Rect dest = {25,25,10,10};
		SDL_SetRenderDrawColor(context.renderer, 255, 255, 0, 255);
		SDL_RenderFillRect(context.renderer, &rect);
		SDL_RenderCopy(context.renderer, context.texture, &rect, &dest);
		SDL_RenderPresent(context.renderer);
		*/
		//SDL_RenderClear(renderer);
		//SDL_DestroyRenderer(renderer);

		ParsecClientEvent evento;
		bool newEvent = ParsecClientPollEvents(context.parsec, 100, &evento);
		if (newEvent) {
			char *msg = ParsecGetBuffer(context.parsec, evento.userData.key);
			// Get Host screen resolution
			if (msg){
				if (evento.userData.id == 101) {
					char *hostScreen = msg;
					char *temp = strtok(hostScreen, ",");
					char *hX = temp;
					temp = strtok(NULL, ",");
					char *hY = temp; 
					hostResX = atoi(hX);
					hostResY = atoi(hY);
				}
				// Get Host mouse position
				if (evento.userData.id == 102){
					char *pos = msg;
					char *temp = strtok(pos, ",");
					char *hX = temp;
					temp = strtok(NULL, ",");
					char *hY = temp;
					int receivedX = atoi(hX);
					int receivedY = atoi(hY);
					hostX = (receivedX * clientResX) / hostResX;
					hostY = (receivedY * clientResY) / hostResY;
					double hostX_aux = hostX;
					double hostY_aux = hostY;
					hostX = converterX(hostX_aux, WINDOW_W);
					hostY = converterY(hostY_aux, WINDOW_H);
				}
				ParsecFree(context.parsec, msg);
			}
			if (evento.userData.id == 28){
				printf("Host turned off control sharing.\n");
				controlsharing = false;
				mousecontrol = false;
				keyboardcontrol = false;
				gamepadcontrol = false;
			}
		}
		
		for (SDL_Event msg; SDL_PollEvent(&msg);) {
			ParsecMessage pmsg = {0};
			switch (msg.type) {
				case SDL_QUIT:
					context.done = true;
					break;
				case SDL_KEYDOWN:
					pmsg.type = MESSAGE_KEYBOARD;
					pmsg.keyboard.code = (ParsecKeycode) msg.key.keysym.scancode;
					pmsg.keyboard.mod = msg.key.keysym.mod;
					// KEY_A
					//if (pmsg.keyboard.code == 4) {
						//printf("Screen res do host (%d, %d)\n", hostResX, hostResY);
						//printf("Coordenadas actuais do host (%f, %f)\n", hostX, hostY);
						//printf("%s\n", pmsg.keyboard.value.name);
						//CHECK_KEY(4);
						//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "A", "A", context.window);
						//SDL_UpdateWindowSurface(context.window);
						//SDL_BlitSurface(surf, &rect, context.surface, &rect);
						//teste = !teste;
					//}
					// KEY ESCAPE
					if (pmsg.keyboard.code == 41) {
						if (pingpoint){
							pingpoint = false;
							drawping = false;
							ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
							printf("Ping point mode: Off\n");
						}
					}
					// CHANGE COLOR
					if (pmsg.keyboard.code == 68) {
						color++;
						if (color > 3) color = 0;
						//printf("Color::: %d\n", color);
					}
					// HIDE MENU
					if (pmsg.keyboard.code == 67) {
						hide = !hide;
					}
					// TOGGLE AID MODE
					if (pmsg.keyboard.code == 69) {
						aidmode = !aidmode;
						if (aidmode) {
							ParsecClientSendUserData(context.parsec, 100, screenRes);
							ParsecClientSendUserData(context.parsec, 0, "Aid Mode: On");
							printf("Aid mode: On\n");
							/*
							printf("modes now are: \n");
							print_modes(modess);
							*/
						}
						else {
							snapmouse = false;
							vocalsound = false;
							//controlsharing = false;
							mousecontrol = false;
							keyboardcontrol = false;
							gamepadcontrol = false;
							presshold = false;
							mensagem = false;
							//turnOffModes(modess, 0, context);
							if (pingpoint){
								pingpoint = false;
								drawping = false;
								ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
								printf("Ping point mode: Off\n");
							}
							if (steermouse){
								steermouse = false;
								ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
								printf("Steer mode: Off\n");
							}
							if (controlsharing){
								controlsharing = false;
								mousecontrol = false;
								keyboardcontrol = false;
								gamepadcontrol = false;
								ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
								printf("Steer mode: Off\n");
							}
							presshold = false;
							/*
							if (presskey) {
								presskey = false;
								ParsecClientSendUserData(context.parsec, 35, "Press Key mode: Off.");
								printf("Press Key mode: Off.\n");
							}
							if (holdkey) {
								holdkey = false;
								ParsecClientSendUserData(context.parsec, 38, "Hold Key mode: Off.");
								printf("Hold Key mode: Off.\n");
							}
							*/
							ParsecClientSendUserData(context.parsec, 1, "Aid mode: Off");
							printf("Aid mode: Off\n");
							//printf("modes now are: \n");
							//print_modes(modess);
						}
					}
					// AID MODE
					if (aidmode) {
						// PING POINT MODE
						if (pmsg.keyboard.code == 58) {
							pingpoint = !pingpoint;
							// Turn off other modes
							if (snapmouse) {
								snapmouse = false;
								ParsecClientSendUserData(context.parsec, 5, "Snap mode: Off");
								printf("Snap mode: Off\n");
							}
							vocalsound = false;
							mensagem = false;
							if (steermouse){
								steermouse = false;
								ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
								printf("Steer mode: Off\n");
							}
							if (controlsharing){
								controlsharing = false;
								mousecontrol = false;
								//keyboardcontrol = false;
								//gamepadcontrol = false;
								ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
								printf("Steer mode: Off\n");
							}
							presshold = false;
							/*
							if (presskey) {
								presskey = false;
								ParsecClientSendUserData(context.parsec, 35, "Press Key mode: Off.");
								printf("Press Key mode: Off.\n");
							}
							if (holdkey) {
								holdkey = false;
								ParsecClientSendUserData(context.parsec, 38, "Hold Key mode: Off.");
								printf("Hold Key mode: Off.\n");
							}
							*/
							//turnOffModes(modess, 1, context);
							// Ping point
							if (pingpoint) {
								ParsecClientSendUserData(context.parsec, 2, "Ping point mode: On");
								printf("Ping point mode: On (Click on the screen to send ping point)\n");
							}
							else {
								drawping = false;
								ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
								printf("Ping point mode: Off\n");
							}
						}
						// SNAP MOUSE MODE
						if (pmsg.keyboard.code == 59) {
							snapmouse = !snapmouse;
							// Turn off other modes
							vocalsound = false;
							mensagem = false;
							//turnOffModes(modess, 0, context);
							if (pingpoint){
								pingpoint = false;
								drawping = false;
								ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
								printf("Ping point mode: Off\n");
							}
							if (steermouse){
								steermouse = false;
								ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
								printf("Steer mode: Off\n");
							}
							if (controlsharing){
								controlsharing = false;
								mousecontrol = false;
								//keyboardcontrol = false;
								//gamepadcontrol = false;
								ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
								printf("Steer mode: Off\n");
							}
							presshold = false;
							/*
							if (presskey) {
								presskey = false;
								ParsecClientSendUserData(context.parsec, 35, "Press Key mode: Off.");
								printf("Press Key mode: Off.\n");
							}
							if (holdkey) {
								holdkey = false;
								ParsecClientSendUserData(context.parsec, 38, "Hold Key mode: Off.");
								printf("Hold Key mode: Off.\n");
							}
							*/
							//turnOffModes(modess, 2, context);
							// Snap mouse
							if (snapmouse) {
								ParsecClientSendUserData(context.parsec, 4, "Snap mode: On (Click on the screen to snap host's mouse)");
								printf("Snap mode: On (Click on the screen to snap host's mouse)\n");
							}
							else {
								ParsecClientSendUserData(context.parsec, 5, "Snap mode: Off");
								printf("Snap mode: Off\n");
							}
						}
						// STEER MOUSE MODE
						if (pmsg.keyboard.code == 60) {
							steermouse = !steermouse;
							// Turn off other modes
							snapmouse = false;
							vocalsound = false;
							mensagem = false;
							//turnOffModes(modess, 0, context);
							if (pingpoint){
								pingpoint = false;
								drawping = false;
								ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
								printf("Ping point mode: Off\n");
							}
							if (controlsharing){
								controlsharing = false;
								//mousecontrol = false;
								keyboardcontrol = false;
								//gamepadcontrol = false;
								ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
								printf("Steer mode: Off\n");
							}
							presshold = false;
							/*
							if (presskey) {
								presskey = false;
								ParsecClientSendUserData(context.parsec, 35, "Press Key mode: Off.");
								printf("Press Key mode: Off.\n");
							}
							if (holdkey) {
								holdkey = false;
								ParsecClientSendUserData(context.parsec, 38, "Hold Key mode: Off.");
								printf("Hold Key mode: Off.\n");
							}
							*/
							//turnOffModes(modess, 3, context);
							if (steermouse) {
								ParsecClientSendUserData(context.parsec, 6, "Steer mode: On (Press the arrow keys to move host's mouse)");
								printf("Steer mode: On (Press the arrow keys to move host's mouse)\n");
							}
							else {
								ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
								printf("Steer mode: Off\n");
							}
						}
						// Steer mouse
						if (steermouse) {
							// Move right
							if (pmsg.keyboard.code == 79) {
								ParsecClientSendUserData(context.parsec, 10, "Move right");
								//printf("Right!\n");
							}
							// Move left
							if (pmsg.keyboard.code == 80) {
								ParsecClientSendUserData(context.parsec, 11, "Move left");
								//printf("Left!\n");
							}
							// Move down
							if (pmsg.keyboard.code == 81) {
								ParsecClientSendUserData(context.parsec, 12, "Move down");
								//printf("Down!\n");
							}
							// Move up
							if (pmsg.keyboard.code == 82) {
								ParsecClientSendUserData(context.parsec, 13, "Move up");
								//printf("Up!\n");
							}
						}
						// Send vocal sound
						if (pmsg.keyboard.code == 61) {
							vocalsound = !vocalsound;
							ParsecClientSendUserData(context.parsec, 16, "List of vocal sounds:");
							// Turn off other modes
							if (pingpoint){
								pingpoint = false;
								drawping = false;
								ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
								printf("Ping point mode: Off\n");
							}
							if (snapmouse) {
								snapmouse = false;
								ParsecClientSendUserData(context.parsec, 5, "Snap mode: Off");
								printf("Snap mode: Off\n");
							}
							if (steermouse){
								steermouse = false;
								ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
								printf("Steer mode: Off\n");
							}
							if (controlsharing) {
								controlsharing = false;
								ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
							}
							presshold = false;
							/*
							if (presskey) {
								presskey = false;
								ParsecClientSendUserData(context.parsec, 35, "Press Key mode: Off.");
								printf("Press Key mode: Off.\n");
							}
							if (holdkey) {
								holdkey = false;
								ParsecClientSendUserData(context.parsec, 38, "Hold Key mode: Off.");
								printf("Hold Key mode: Off.\n");
							}
							*/
						}
						if (!keyboardcontrol && !presshold && !mensagem) {
							if (pmsg.keyboard.code == 28) {
								ParsecClientSendUserData(context.parsec, 18, "Vocal sound: Yes!");
								printf("Vocal sound sent: Yes!\n");
							}
							if (pmsg.keyboard.code == 17) {
								ParsecClientSendUserData(context.parsec, 19, "Vocal sound: No!");
								printf("Vocal sound sent: No!\n");
							}
							if (pmsg.keyboard.code == 29) {
								ParsecClientSendUserData(context.parsec, 20, "Vocal sound: Jazzy!");
								printf("Vocal sound sent: Jazzy!\n");
							}
							if (pmsg.keyboard.code == 18) {
								ParsecClientSendUserData(context.parsec, 40, "Vocal sound: Okay!");
								printf("Vocal sound sent: Okay!\n");
							}
							if (pmsg.keyboard.code == 54) {
								ParsecClientSendUserData(context.parsec, 14, "Vocal sound: Correct!");
								printf("Vocal sound sent: Correct!\n");
							}
							if (pmsg.keyboard.code == 55) {
								ParsecClientSendUserData(context.parsec, 15, "Vocal sound: Wrong!");
								printf("Vocal sound sent: Wrong!\n");
							}
							if (pmsg.keyboard.code == 22) {
								ParsecClientSendUserData(context.parsec, 29, "Vocal sound: Shoot!");
								printf("Vocal sound sent: Shoot!\n");
							}
							if (pmsg.keyboard.code == 82) {
								ParsecClientSendUserData(context.parsec, 30, "Vocal sound: Up!");
								printf("Vocal sound sent: Up!\n");
							}
							if (pmsg.keyboard.code == 81) {
								ParsecClientSendUserData(context.parsec, 31, "Vocal sound: Down!");
								printf("Vocal sound sent: Down!\n");
							}
							if (pmsg.keyboard.code == 80) {
								ParsecClientSendUserData(context.parsec, 32, "Vocal sound: Left!");
								printf("Vocal sound sent: Left!\n");
							}
							if (pmsg.keyboard.code == 79) {
								ParsecClientSendUserData(context.parsec, 33, "Vocal sound: Right!");
								printf("Vocal sound sent: Right!\n");
							}
							if (pmsg.keyboard.code == 21) {
								ParsecClientSendUserData(context.parsec, 41, "Vocal sound: Run!");
								printf("Vocal sound sent: Run!\n");
							}
							if (pmsg.keyboard.code == 23) {
								ParsecClientSendUserData(context.parsec, 46, "Vocal sound: Stop!");
								printf("Vocal sound sent: Stop!\n");
							}
							if (pmsg.keyboard.code == 10) {
								ParsecClientSendUserData(context.parsec, 47, "Vocal sound: Great!");
								printf("Vocal sound sent: Great!\n");
							}
							if (pmsg.keyboard.code == 26) {
								ParsecClientSendUserData(context.parsec, 48, "Vocal sound: Wait!");
								printf("Vocal sound sent: Wait!\n");
							}
							if (pmsg.keyboard.code == 13) {
								ParsecClientSendUserData(context.parsec, 52, "Vocal sound: Jump!");
								printf("Vocal sound sent: Quick!\n");
							}
							if (pmsg.keyboard.code == 20) {
								ParsecClientSendUserData(context.parsec, 53, "Vocal sound: Quick!");
								printf("Vocal sound sent: Quick!\n");
							}
						}
						// CONTROL SHARING
						if (pmsg.keyboard.code == 62) {
							controlsharing = !controlsharing;
							// Turn off other modes
							snapmouse = false;
							vocalsound = false;
							mensagem = false;
							//turnOffModes(modess, 0, context);
							if (pingpoint){
								pingpoint = false;
								drawping = false;
								ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
								printf("Ping point mode: Off\n");
							}
							if (steermouse){
								steermouse = false;
								ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
								printf("Steer mode: Off\n");
							}
							presshold = false;
							/*
							if (presskey) {
								presskey = false;
								ParsecClientSendUserData(context.parsec, 35, "Press Key mode: Off.");
								printf("Press Key mode: Off.\n");
							}
							if (holdkey) {
								holdkey = false;
								ParsecClientSendUserData(context.parsec, 38, "Hold Key mode: Off.");
								printf("Hold Key mode: Off.\n");
							}
							*/
							if (controlsharing) {
								ParsecClientSendUserData(context.parsec, 27, "Control sharing: On.");
								printf("Choose input to control:\n");
								printf("[M] Mouse\n");
								printf("[K] Keyboard\n");
								printf("[G] Gamepad\n");
							}
							else {
								//mousecontrol = false;
								//keyboardcontrol = false;
								//gamepadcontrol = false;
								ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
							}
						}
						if (controlsharing) {
							// MOUSE CONTROL MODE
							if (pmsg.keyboard.code == 16) {
								mousecontrol = !mousecontrol;
								controlsharing = false;
								if (mousecontrol){
									ParsecClientSendUserData(context.parsec, 21, "Mouse control: On.");
									printf("Controling the mouse.\n");
									// Turn other mouse modes off
									if (pingpoint) {
										pingpoint = false;
										drawping = false;
										ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off.");
										printf("Ping point mode: Off\n");
									}
									if (steermouse) {
										steermouse = false;
										ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off.");
										printf("Steer mode: Off.\n");
									}
									presshold = false;
									/*
									if (presskey) {
										presskey = false;
										printf("Press Key mode: Off.\n");
									}
									*/
								}
								else{
									mousecontrol = false;
									ParsecClientSendUserData(context.parsec, 22, "Mouse control: Off.");
									printf("No longer controling the mouse.\n");
								}
							}
							// KEYBOARD CONTROL MODE
							if (pmsg.keyboard.code == 14){
								keyboardcontrol = !keyboardcontrol;
								controlsharing = false;
								if (keyboardcontrol){
									ParsecClientSendUserData(context.parsec, 23, "Keyboard control: On.");
									printf("Controling the keyboard.\n");
								}
								else{
									keyboardcontrol = false;
									ParsecClientSendUserData(context.parsec, 24, "Keyboard control: Off.");
									printf("No longer controling the keyboard.\n");
								}
							}
							// GAMEPAD CONTROL MODE
							if (pmsg.keyboard.code == 10) {
								gamepadcontrol = !gamepadcontrol;
								controlsharing = false;
								if (gamepadcontrol){
									ParsecClientSendUserData(context.parsec, 25, "Gamepad control: On.");
									printf("Controling the gamepad.\n");
								}
								else{
									gamepadcontrol = false;
									ParsecClientSendUserData(context.parsec, 26, "Gamepad control: Off.");
									printf("No longer controling the gamepad.\n");
								}
							}
						}
						// TTS MESSAGE
						if (pmsg.keyboard.code == 64) {
							mensagem = !mensagem;
							//inputdone = false;
							//printf("%s\n", text);
							if (mensagem) {
								vocalsound = false;
								presshold = false;
								//presskey = false;
								//holdkey = false;
								printf("Message mode: On.\n");
								if (pingpoint){
									pingpoint = false;
									drawping = false;
									ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
									printf("Ping point mode: Off\n");
								}
								if (steermouse){
									steermouse = false;
									ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
									printf("Steer mode: Off\n");
								}
								if (snapmouse) {
									snapmouse = false;
									ParsecClientSendUserData(context.parsec, 5, "Snap mode: Off");
									printf("Snap mode: Off\n");
								}
								if (controlsharing) {
									controlsharing = false;
									ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
								}
								//ParsecClientPause(context.parsec, true, true);	
							}
							else {
								printf("Message mode: Off.\n");
								//SDL_StopTextInput();
								//ParsecClientPause(context.parsec, false, false);
							}
						}
					}
				case SDL_KEYUP:
					pmsg.type = MESSAGE_KEYBOARD;
					pmsg.keyboard.code = (ParsecKeycode) msg.key.keysym.scancode;
					pmsg.keyboard.mod = msg.key.keysym.mod;
					pmsg.keyboard.pressed = msg.key.type == SDL_KEYDOWN;
					// PRESS/HOLD KEY MODE
					if (msg.type != SDL_KEYDOWN && pmsg.keyboard.code == 63 && aidmode) {
						//presskey = !presskey;
						presshold = !presshold;
						mensagem = false;
						if (pingpoint){
							pingpoint = false;
							drawping = false;
							ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
							printf("Ping point mode: Off\n");
						}
						if (steermouse){
							steermouse = false;
							ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
							printf("Steer mode: Off\n");
						}
						if (vocalsound) {
							vocalsound = false;
						}
						if (snapmouse) {
							snapmouse = false;
							ParsecClientSendUserData(context.parsec, 5, "Snap mode: Off");
							printf("Snap mode: Off\n");
						}
						if (controlsharing) {
							controlsharing = false;
							ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
						}
						if (presshold) {
							//holdkey = false;
							//ParsecClientSendUserData(context.parsec, 34, "Press Key mode: On.");
							//printf("Press Key mode: On. (Press a key to inform the host.)\n");
							if (keyboardcontrol){
								keyboardcontrol = false;
								ParsecClientSendUserData(context.parsec, 24, "Keyboard control: Off.");
								printf("No longer controling the keyboard.\n");;
							}
						}
						//else {
							//ParsecClientSendUserData(context.parsec, 35, "Press Key mode: Off.");
							//printf("Press Key mode: Off.\n");
						//}
					}
					if (msg.type != SDL_KEYDOWN && presshold && aidmode) {
						char* key;
						if (pmsg.keyboard.code == 226 /*|| pmsg.keyboard.code == 230*/) {
					    	//if (pressmode == 0) pressmode = 1;
					    	//else pressmode = 0;
					    	pressmode++;
					    	if (pressmode >= 4) pressmode = 0;
					    	//printf("Pressmode::: %d\n", pressmode);
						}
						switch(pmsg.keyboard.code) {
						    case 4: key = "A"; break;
						    case 5: key = "B"; break;
						    case 6: key = "C"; break;
						    case 7: key = "D"; break;
						    case 8: key = "E"; break;
						    case 9: key = "F"; break;
						    case 10: key = "G"; break;
						    case 11: key = "H"; break;
						    case 12: key = "I"; break;
						    case 13: key = "J"; break;
						    case 14: key = "K"; break;
						    case 15: key = "L"; break;
						    case 16: key = "M"; break;
						    case 17: key = "N"; break;
						    case 18: key = "O"; break;
						    case 19: key = "P"; break;
						    case 20: key = "Q"; break;
						    case 21: key = "R"; break;
						    case 22: key = "S"; break;
						    case 23: key = "T"; break;
						    case 24: key = "U"; break;
						    case 25: key = "V"; break;
						    case 26: key = "W"; break;
						    case 27: key = "X"; break;
						    case 28: key = "Y"; break;
						    case 29: key = "Z"; break;
							case 30: key = "1"; break;
							case 31: key = "2"; break;
							case 32: key = "3"; break;
							case 33: key = "4"; break;
							case 34: key = "5"; break;
							case 35: key = "6"; break;
							case 36: key = "7"; break;
							case 37: key = "8"; break;
							case 38: key = "9"; break;
							case 39: key = "0"; break;
							case 40: key = "Enter"; break;
							case 41: key = "Escape"; break;
							case 42: key = "Backspace"; break;
							case 43: key = "Tab"; break;
							case 44: key = "Space"; break;
						    case 79: key = "Right Arrow"; break;
						    case 80: key = "Left Arrow"; break;
						    case 81: key = "Down Arrow"; break;
						    case 82: key = "Up Arrow"; break;
						    case 225: key = "Shift"; break;
						    /*
						    case 226:
						    	key = "";
						    	printf("LHALH\n");
						    	if (pressmode == 0) pressmode = 1;
						    	if (pressmode == 1) pressmode = 0;
						    	break;
						    */
						    case 224: key = "Control"; break;
						    case 228: key = "Control"; break;
						    case 229: key = "Shift"; break;
						    /*
						    case 230:
						    	key = "";
						    	printf("LHALH\n");
						    	if (pressmode == 0) pressmode = 1;
						    	if (pressmode == 1) pressmode = 0;
						    	break;
						    */
						    default:
						        key = "";
						        break;
						}
						if (key != "") {
							printf("Key sent: %s\n", key);
							if (pressmode == 0){
								ParsecClientSendUserData(context.parsec, 36, key);
								//Sleep(1000);
							}
							else if (pressmode == 1){
								ParsecClientSendUserData(context.parsec, 39, key);
								//Sleep(1000);
							}
							else if (pressmode == 2){
								ParsecClientSendUserData(context.parsec, 50, key);
							}
							else {
								ParsecClientSendUserData(context.parsec, 51, key);
							}
						}
						/*
						if (GetAsyncKeyState(VK_LBUTTON)) {
							key = "Left Click";
							printf("Key sent: %s\n", key);
							ParsecClientSendUserData(context.parsec, 36, key);
						}
						if (GetAsyncKeyState(VK_RBUTTON)) {
							key = "Right Click";
							printf("Key sent: %s\n", key);
							ParsecClientSendUserData(context.parsec, 36, key);
						}
						*/
					}
					/*
					if (pmsg.keyboard.code == 64) {
						holdkey = !holdkey;
						mensagem = false;
						if (pingpoint){
							pingpoint = false;
							ParsecClientSendUserData(context.parsec, 3, "Ping point mode: Off");
							printf("Ping point mode: Off\n");
						}
						if (steermouse){
							steermouse = false;
							ParsecClientSendUserData(context.parsec, 7, "Steer mode: Off");
							printf("Steer mode: Off\n");
						}
						if (vocalsound) {
							vocalsound = false;
						}
						if (snapmouse) {
							snapmouse = false;
							ParsecClientSendUserData(context.parsec, 5, "Snap mode: Off");
							printf("Snap mode: Off\n");
						}
						if (controlsharing) {
							controlsharing = false;
							ParsecClientSendUserData(context.parsec, 28, "Control sharing: Off.");
						}
						if (holdkey) {
							presskey = false;
							ParsecClientSendUserData(context.parsec, 37, "Hold Key mode: On.");
							printf("Hold Key mode: On. (Press a key to inform the host.)\n");
							if (keyboardcontrol) {
								keyboardcontrol = false;
								ParsecClientSendUserData(context.parsec, 24, "Keyboard control: Off.");
								printf("No longer controling the keyboard.\n");
							}
						}
						else {
							ParsecClientSendUserData(context.parsec, 38, "Hold Key mode: Off.");
							printf("Hold Key mode: Off.\n");
						}
					}
					/*/
					break;
				case SDL_MOUSEMOTION:
					//printf("AAAAAA\n");
					pmsg.type = MESSAGE_MOUSE_MOTION;
					pmsg.mouseMotion.relative = SDL_GetRelativeMouseMode();
					pmsg.mouseMotion.x = pmsg.mouseMotion.relative ? msg.motion.xrel : msg.motion.x;
					pmsg.mouseMotion.y = pmsg.mouseMotion.relative ? msg.motion.yrel : msg.motion.y;
					GetCursorPos(&cursorPos);
					clientX = converterX(cursorPos.x, WINDOW_W);
					clientY = converterY(cursorPos.y, WINDOW_H);
					posX = cursorPos.x;
					posY = cursorPos.y;
					//printf("Pos XXX: %d\n", posX);
					//printf("Pos YYY: %d\n", posY);
					//rect.x = cursorPos.x;
					//rect.y = cursorPos.y;
					//posX = pmsg.mouseMotion.x;
					//posY = pmsg.mouseMotion.y;
					//d = euclideanDistance(pinned.x, pinned.y, posX, posY);
					break;
				case SDL_MOUSEBUTTONDOWN:
					//pmsg.type = MESSAGE_MOUSE_MOTION;
					//pmsg.mouseMotion.x = pmsg.mouseMotion.relative ? msg.motion.xrel : msg.motion.x;
					//pmsg.mouseMotion.y = pmsg.mouseMotion.relative ? msg.motion.yrel : msg.motion.y;
					//ParsecClientSendMessage(context.parsec, &pmsg);
					//pmsg.type = MESSAGE_MOUSE_BUTTON;
					pmsg.mouseButton.button = msg.button.button;
					//pmsg.mouseButton.pressed = msg.button.type == SDL_MOUSEBUTTONDOWN;
					posX = cursorPos.x;
					posY = cursorPos.y;
					//printf("Pos XXX: %d\n", posX);
					//printf("Pos YYY: %d\n", posY);
					char px[5];
					char py[5];
					_itoa_s(posX, px, 5, 10);
					_itoa_s(posY, py, 5, 10);
					char* pos = (char *) malloc(2 + strlen(px) + strlen(py));
					strcpy_s(pos, 5, px);
					strcat_s(pos, 6, ",");
					strcat_s(pos, 11, py);
					if (pingpoint) {
						ParsecClientSendUserData(context.parsec, 8, pos);
						printf("Ping point sent to position (%d, %d)\n", posX, posY);
						drawping = true;
						double posX_aux = posX;
						double posY_aux = posY;
						pingX = converterX(posX_aux, WINDOW_W);
						pingY = converterY(posY_aux, WINDOW_H);
						//printf("Window: (%d, %d)\n", WINDOW_W, WINDOW_H);
						//printf("Printed Position:\n");
						//double posX_aux = posX;
						//printf("PosX::: %d\n", posX_aux);
						//printf("PosX / window:::%d\n", (posX_aux / WINDOW_W));
						//double convertX = 2 * (posX_aux / WINDOW_W) - 1;
						//printf("X: %f\n", pingX);
						//printf("Y: %f\n", pingY);
						//printf("RIGHT X: %f\n", convertX);
					}
					else if (snapmouse) {
						ParsecClientSendUserData(context.parsec, 9, pos);
						printf("Snapped mouse to position (%d, %d).\n", posX, posY);
					}
					else if (presshold) {
						if (pmsg.mouseButton.button == 1) {
							printf("Key sent: Left Click.\n");
							if (pressmode == 0) {
								ParsecClientSendUserData(context.parsec, 36, "Left Click");
							}
							else if (pressmode == 1){
								ParsecClientSendUserData(context.parsec, 39, "Left Click");
							}
							else if (pressmode == 2){
								ParsecClientSendUserData(context.parsec, 50, "Left Click");
							}
							else {
								ParsecClientSendUserData(context.parsec, 51, "Left Click");
							}
						}
						if (pmsg.mouseButton.button == 2) {
							printf("Key sent: Wheel Button.\n");
							if (pressmode == 0) {
								ParsecClientSendUserData(context.parsec, 36, "Wheel Button");
							}
							else if (pressmode == 1){
								ParsecClientSendUserData(context.parsec, 39, "Wheel Button");
							}
							else if (pressmode == 2){
								ParsecClientSendUserData(context.parsec, 50, "Wheel Button");
							}
							else {
								ParsecClientSendUserData(context.parsec, 51, "Wheel Button");
							}
						}
						if (pmsg.mouseButton.button == 3){
							printf("Key sent: Right Click.\n");
							if (pressmode == 0) {
								ParsecClientSendUserData(context.parsec, 36, "Right Click");
							}
							else if (pressmode == 1){
								ParsecClientSendUserData(context.parsec, 39, "Right Click");
							}
							else if (pressmode == 2){
								ParsecClientSendUserData(context.parsec, 50, "Right Click");
							}
							else {
								ParsecClientSendUserData(context.parsec, 51, "Right Click");
							}
						}
					}
				case SDL_MOUSEBUTTONUP:
					pmsg.type = MESSAGE_MOUSE_BUTTON;
					pmsg.mouseButton.button = msg.button.button;
					pmsg.mouseButton.pressed = msg.button.type == SDL_MOUSEBUTTONDOWN;
					break;
				case SDL_MOUSEWHEEL:
					pmsg.type = MESSAGE_MOUSE_WHEEL;
					pmsg.mouseWheel.x = msg.wheel.x;
					pmsg.mouseWheel.y = msg.wheel.y;
					if (presshold) {
						if (pmsg.mouseWheel.y > 0) {
							printf("Key sent: Mouse Wheel Up.\n");
							if (pressmode == 0) {
								ParsecClientSendUserData(context.parsec, 36, "Mouse Wheel Up");
							}
							else if (pressmode == 1){
								ParsecClientSendUserData(context.parsec, 39, "Mouse Wheel Up");
							}
							else if (pressmode == 2){
								ParsecClientSendUserData(context.parsec, 50, "Mouse Wheel Up");
							}
							else {
								ParsecClientSendUserData(context.parsec, 51, "Mouse Wheel Up");
							}
						}
						else if (pmsg.mouseWheel.y < 0){
							printf("Key sent: Mouse Wheel Down.\n");
							if (pressmode == 0) {
								ParsecClientSendUserData(context.parsec, 36, "Mouse Wheel Down");
							}
							else if (pressmode == 1){
								ParsecClientSendUserData(context.parsec, 39, "Mouse Wheel Down");
							}
							else if (pressmode == 2){
								ParsecClientSendUserData(context.parsec, 50, "Mouse Wheel Down");
							}
							else {
								ParsecClientSendUserData(context.parsec, 51, "Mouse Wheel Down");
							}
						}
					}
					break;
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
					pmsg.type = MESSAGE_GAMEPAD_BUTTON;
					pmsg.gamepadButton.id = msg.cbutton.which;
					pmsg.gamepadButton.button = msg.cbutton.button;
					pmsg.gamepadButton.pressed = msg.cbutton.type == SDL_CONTROLLERBUTTONDOWN;
					break;
				case SDL_CONTROLLERAXISMOTION:
					pmsg.type = MESSAGE_GAMEPAD_AXIS;
					pmsg.gamepadAxis.id = msg.caxis.which;
					pmsg.gamepadAxis.axis = msg.caxis.axis;
					pmsg.gamepadAxis.value = msg.caxis.value;
					break;
				case SDL_CONTROLLERDEVICEADDED:
					SDL_GameControllerOpen(msg.cdevice.which);
					break;
				case SDL_CONTROLLERDEVICEREMOVED:
					pmsg.type = MESSAGE_GAMEPAD_UNPLUG;
					pmsg.gamepadUnplug.id = msg.cdevice.which;
					SDL_GameControllerClose(SDL_GameControllerFromInstanceID(msg.cdevice.which));
					break;
				case SDL_CLIPBOARDUPDATE:
					ParsecClientSendUserData(context.parsec, PARSEC_APP_CLIPBOARD_MSG, SDL_GetClipboardText());
					break;
			}

			if (pmsg.type != 0)
				ParsecClientSendMessage(context.parsec, &pmsg);

			e = ParsecClientGetStatus(context.parsec, NULL);
			if (e != PARSEC_CONNECTING && e != PARSEC_OK)
				context.done = true;
		}

		for (ParsecClientEvent event; ParsecClientPollEvents(context.parsec, 0, &event);) {
			switch (event.type) {
				case CLIENT_EVENT_CURSOR:
					cursor(&context, &event.cursor.cursor, event.cursor.key);
					break;
				case CLIENT_EVENT_USER_DATA:
					userData(&context, event.userData.id, event.userData.key);
					break;
				case CLIENT_EVENT_RUMBLE:
					break;
				default:
					break;
			}
		}

		SDL_Delay(1);
	}

	SDL_WaitThread(audio_thread, NULL);
	SDL_WaitThread(render_thread, NULL);
	SDL_WaitThread(input_thread, NULL);

	except:

	if (e != PARSEC_OK) {
		char error[32];
		snprintf(error, 32, "Parsec error: %d\n", e);
		SDL_ShowSimpleMessageBox(0, "Parsec Error", error, NULL);
	}

	ParsecDestroy(context.parsec);

	//SDL_DestroyTexture(context.texture);
	//SDL_RenderClear(rend);
	//SDL_DestroyRenderer(context.renderer);
	//SDL_DestroyRenderer(rend);
	////SDL_FreeSurface(surf);
	//SDL_FreeSurface(context.surface);
	SDL_DestroyWindow(context.window);
	//SDL_DestroyWindow(janela);
	SDL_CloseAudioDevice(context.audio);
	SDL_Quit();

	return 0;
}
