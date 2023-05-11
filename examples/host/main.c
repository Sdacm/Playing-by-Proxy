#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <math.h>
#include <string.h>
#include <windowsx.h>
#include <MMsystem.h>
#include <wingdi.h>
//#include <limits.h>
//#include <sstream>
//#include <graphics.h>
//#include <Python.h>
//#include <sapi.h>
//#include "./gtts.h"
//#include <memory>
//#include "include/flite.h"

#define _ATL_APARTMENT_THREADED
/*
#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override something,
//but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
*/  

#pragma comment(lib, "UxTheme")
#include <uxtheme.h>

#include "parsec-dso.h"

#if defined(_WIN32)
	#if !defined(BITS)
		#define BITS 64
	#endif
	#if (BITS == 64)
		#define SDK_PATH "../../sdk/windows/parsec.dll"
	#else
		#define SDK_PATH "../../sdk/windows/parsec32.dll"
	#endif
#elif defined(__APPLE__)
	#define SDK_PATH "../../sdk/macos/libparsec.dylib"
#else
	#define SDK_PATH "../../sdk/linux/libparsec.so"
#endif

#pragma warning(disable:4996)

//int guestID = -1;
/*
static void logCallback(ParsecLogLevel level, const char *msg, void *opaque)
{
	opaque;
	printf("[%s] %s\n", level == LOG_DEBUG ? "D" : "I", msg);
}
*/

static void guestStateChange(ParsecGuest *guest)
{
	switch (guest->state) {
		// new case:
		case GUEST_WAITING:
            //ParsecHostAllowGuest(parsec, guest->id, true);
			printf("%s#%d waiting to enter. Check the option 'Can connect without approval (careful!)' inside the Parsec app.\n", guest->name, guest->userID);
            break;
		case GUEST_CONNECTED:
			//guestID = guest->userID; 
			printf("%s#%d connected.\n", guest->name, guest->userID);
			break;
		case GUEST_DISCONNECTED:
			//guestID = -1;
			printf("%s#%d disconnected.\n", guest->name, guest->userID);
			break;
		default:
			break;
	}
}

int euclideanDistance (int x1, int y1, int x2, int y2);

int euclideanDistance (int x1, int y1, int x2, int y2) {
   return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

/*
register_cmu_us_kal();
int flite_init();
float flite_file_to_speech(const char *filename, cst_voice *voice, const char *outtype);
float flite_text_to_speech(const char *text, cst_voice *voice, const char *outtype);
*/

int32_t main(int32_t argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: host sessionID\n");
		return 1;
	}

	//HCURSOR hcurseur = LoadCursor(GetModuleHandle(NULL),"IDC_CURSOR");
	//HCURSOR hcopiecurseur = CopyCursor(hcurseur);
	//HINSTANCE hinst;            // handle to current instance 
	//HCURSOR hCurs1, hCurs2;   
	//hCurs1 = LoadCursor(hinst, IDC_WAIT); 
	//SetSystemCursor(hCurs1, 32514);

	ParsecDSO *parsec = NULL;
	ParsecStatus e = ParsecInit(NULL, NULL, SDK_PATH, &parsec);
	if (e != PARSEC_OK) goto except;

	//ParsecSetLogCallback(parsec, logCallback, NULL);

	ParsecHostConfig cfg = PARSEC_HOST_DEFAULTS;
	ParsecHostStart(parsec, HOST_DESKTOP, &cfg, argv[1]);

	//int i = 0;
	POINT cursorPos;
	POINT pinned;
	bool pingpoint = false;
	bool steermouse = false;
	//bool beeeep = false;
	//bool boooop = false;
	int d = 0;
	//bool btest = true;
	int beepHertz = 0;
	int beepTime = 0;
	int waitTime = 0;
	//bool controlsharing = false;
	bool mousecontrol = false;
	bool keyboardcontrol = false;
	bool gamepadcontrol = false;
	ParsecGuest *guests = NULL;
	//ParsecPermissions *permissions;
	//ParsecHostSetPermissions(parsec, guests[0].userID, &permissions);
	int posX;
	int posY;
	bool labelledping = false;
	char text[100] = "";

	//permissions->mouse = true;
	//permissions->keyboard = false;
	//permissions->gamepad = false;

	int hostResX, hostResY;
	hostResX = GetSystemMetrics(SM_CXSCREEN);
	hostResY = GetSystemMetrics(SM_CYSCREEN);
	//printf("RES X: %d\n", hostResX);
	//printf("RES Y: %d\n", hostResY);
	char resX[10], resY[10];
	_itoa_s(hostResX, resX, 10, 10);
	_itoa_s(hostResY, resY, 10, 10);
	char* screenRes = (char *) malloc(2 + strlen(resX) + strlen(resY));
	strcpy_s(screenRes, 10, resX);
	strcat_s(screenRes, 11, ",");
	strcat_s(screenRes, 21, resY);
	int clientResX = -1;
	int clientResY = -1;
	//bool sentRes = false;

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

	//ISpVoice* pVoice=NULL;
	//HRESULT hr;
	//char* input;
	/*
	cst_voice *v;
	flite_init();
	v = register_cmu_us_kal(NULL);
	flite_text_to_speech("wiu",v,"play");
	*/

	char frase[100] = "Welcome to Playing By Proxy.";
	char charCommand[100];
	strcpy(charCommand, "espeak -v +f3 \"");
	strcat(charCommand, frase);
	strcat(charCommand, "\"");
	system(charCommand);
	char mouseProperties[10] = "main.cpl";
	system(mouseProperties);
	//DrvSetPointerShape();

	int language = 0;

	while (true) {
		GetCursorPos(&cursorPos);
		posX = cursorPos.x;
		posY = cursorPos.y;
		char px[5];
		char py[5];
		_itoa_s(posX, px, 5, 10);
		_itoa_s(posY, py, 5, 10);
		char* hostPos = (char *) malloc(2 + strlen(px) + strlen(py));
		strcpy_s(hostPos, 5, px);
		strcat_s(hostPos, 6, ",");
		strcat_s(hostPos, 11, py);
		if (guests != NULL) {
			ParsecHostSendUserData(parsec, guests[0].id, 102, hostPos);
		}
		//char g = _getch();
		//d = euclideanDistance(cursorPos.x, cursorPos.y, 0, 0);
		//int x, y;
		//char test[100];
		//if (g == 'S' || g == 's') {
			//printf("%d\n", d);
			//printf("Enter new position:\n");
			//fgets (test, 100, stdin);
			//printf("Ola, %s\n", test);
			//SetCursorPos(100,100);
			//GetCursorPos(&cursorPos);
			//printf("X: %d\n", cursorPos.x);
			//printf("Y: %d\n", cursorPos.y);
			//Beep(750, 300);
			//Beep(250, 300);
			//beeeep = true;
			//Sleep(2);
			//Beep(750, 800);
			//printf("Sound played\n");
			//PlaySound(TEXT("sounds/left.wav"), NULL, SND_FILENAME);
			//mciSendString("play sound.mp3",NULL,NULL,NULL);
		//}
		/*
		if (GetAsyncKeyState(0x41)){
			ParsecHostSendUserData(parsec, guests[0].id, 28, "Host turned off control sharing.");
			printf("Enviado! para %s#%d\n", guests[0].name, guests[0].id);
		}
		*/
		for (ParsecHostEvent event; ParsecHostPollEvents(parsec, 1000, &event);) {
			switch (event.type) {
				case HOST_EVENT_GUEST_STATE_CHANGE:
					guestStateChange(&event.guestStateChange.guest);
					// Lista de guests (ultimo argumento da seguinte função)
					ParsecHostGetGuests(parsec, GUEST_CONNECTED, &guests);
					ParsecHostSendUserData(parsec, guests[0].id, 101, screenRes);
					/*
					ParsecPermissions permissions = {0};
					permissions.mouse = true;
					ParsecHostSetPermissions(parsec, guests[0].userID, &permissions);
					*/
					//printf("Nome: %s, Id: %d\n", guests[0].name, guests[0].userID);
					break;
				case HOST_EVENT_USER_DATA:
					char *msg = ParsecGetBuffer(parsec, event.userData.key);
					if (msg) {
						if (event.userData.id == 100){
							char *clientScreen = msg;
							char *temp = strtok(clientScreen, ",");
							char *clientX = temp;
							temp = strtok(NULL, ",");
							char *clientY = temp; 
							clientResX = atoi(clientX);
							clientResY = atoi(clientY);
							//printf("Client Res X: %d, Y: %d\n", clientResX, clientResY);
						}
						if (event.userData.id != 8 && event.userData.id != 9 && event.userData.id != 36
							&& event.userData.id != 39 && event.userData.id != 50 && event.userData.id != 51
							&& event.userData.id != 100){
							printf("%s\n", msg);
						}
						if (event.userData.id == 0){
							PlaySound(TEXT("sounds/interface/on_pipe.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 1){
							PlaySound(TEXT("sounds/interface/off_mixkit_negative_tone.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 3) {
							pingpoint = false;
							labelledping = false;
							text[0] = '\0';
						}
						if (event.userData.id == 4){
							PlaySound(TEXT("sounds/interface/on_mixkit-select-click.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 5){
							PlaySound(TEXT("sounds/interface/off_mixkit-click-error.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 6){
							steermouse = true;
							pingpoint = false;
							labelledping = false;
							text[0] = '\0';
						}
						if (event.userData.id == 7) {
							steermouse = false;
							printf("Steer mode: Off\n");
						}
						// Ping point or Snap mouse
						if (event.userData.id == 8 ||
							event.userData.id == 9 ||
							event.userData.id == 45){
							steermouse = false;
							char *pos = msg;
							char *temp = strtok(pos, ",");
							char *pinX = temp;
							temp = strtok(NULL, ",");
							char *pinY = temp;
							int sentX = atoi(pinX);
							int sentY = atoi(pinY);
							pinned.x = (sentX * hostResX) / clientResX;
							pinned.y = (sentY * hostResY) / clientResY;
							if (event.userData.id == 8 || event.userData.id == 45){
								pingpoint = true;
								printf("Pinned position: (%d, %d)\n", pinned.x, pinned.y);
								printf("Click [ENTER] to go directly to the chosen position.\n");
								printf("Click [ESCAPE] to cancel ping points.\n");
								if (event.userData.id == 45) {
									temp = strtok(NULL, ",");
									strcat(text, temp);
									labelledping = true;
									//printf("%s\n", text);
								}
							}
							else if (event.userData.id == 9){
								pingpoint = false;
								labelledping = false;
								text[0] = '\0';
								Beep(200, 200);
								SetCursorPos(pinned.x, pinned.y);
								printf("Snapped mouse to position: (%d, %d)\n", pinned.x, pinned.y);
							}
						}
						// Vocal sounds
						/*
						if (event.userData.id == 16) {
							printf("List of sounds:\n");
							printf("[Y] Yes!\n");
							printf("[N] No!\n");
							printf("[J] Jazzy Sound!\n");
							printf("[O] Okay!\n");
							printf("[S] Shoot!\n");
							printf("[R] Run!\n");
							printf("[T] Stop!\n");
							printf("[G] Great!\n");
							printf("[W] Wait!\n");
							printf("[Q] Quick!\n");
							printf("[,] Correct Sound\n");
							printf("[.] Wrong Sound\n");
							printf("[ARROWS] Up/Down/Left/Right\n");
						}
						*/
						if (event.userData.id == 18) {
							PlaySound(TEXT("sounds/yes.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 19) {
							PlaySound(TEXT("sounds/no.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 20) {
							PlaySound(TEXT("sounds/jazzy.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 40) {
							PlaySound(TEXT("sounds/okay.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 14) {
							PlaySound(TEXT("sounds/correct.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 15) {
							PlaySound(TEXT("sounds/wrong.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 29) {
							PlaySound(TEXT("sounds/shoot.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 30) {
							PlaySound(TEXT("sounds/arrow_up.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 31) {
							PlaySound(TEXT("sounds/arrow_down.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 32) {
							PlaySound(TEXT("sounds/arrow_left.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 33) {
							PlaySound(TEXT("sounds/arrow_right.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 41) {
							PlaySound(TEXT("sounds/run.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 46) {
							PlaySound(TEXT("sounds/stop.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 47) {
							PlaySound(TEXT("sounds/great.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 48) {
							PlaySound(TEXT("sounds/wait.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 49) {
							PlaySound(TEXT("sounds/jump.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 52) {
							PlaySound(TEXT("sounds/jump.wav"), NULL, SND_FILENAME);
						}
						if (event.userData.id == 53) {
							PlaySound(TEXT("sounds/quick.wav"), NULL, SND_FILENAME);
						}
						// Control sharing
						if (event.userData.id == 27) {
							printf("Choose input to control:\n");
							printf("[M] Mouse\n");
							printf("[K] Keyboard\n");
							printf("[G] Gamepad\n");
							//controlsharing = true;
						}
						// Mouse control
						if (event.userData.id == 21) {
							PlaySound(TEXT("sounds/interface/on_mixkit-opening-software.wav"), NULL, SND_FILENAME);
							mousecontrol = true;
							guests[0].perms.mouse = true;
							ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
							printf("Controlling the mouse.\n");
						}
						if (event.userData.id == 22) {
							PlaySound(TEXT("sounds/interface/off_mixkit_negative_tone.wav"), NULL, SND_FILENAME);
							mousecontrol = false;
							guests[0].perms.mouse = false;
							ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
							printf("No longer controlling the mouse.\n");
						}
						// Keyboard control
						if (event.userData.id == 23) {
							PlaySound(TEXT("sounds/interface/on_mixkit-opening-software.wav"), NULL, SND_FILENAME);
							keyboardcontrol = true;
							guests[0].perms.keyboard = true;
							ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
							printf("Controlling the keyboard.\n");
						}
						if (event.userData.id == 24) {
							PlaySound(TEXT("sounds/interface/off_mixkit_negative_tone.wav"), NULL, SND_FILENAME);
							keyboardcontrol = false;
							guests[0].perms.keyboard = false;
							ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
							printf("No longer controlling the keyboard.\n");
						}
						// Gamepad control
						if (event.userData.id == 25) {
							PlaySound(TEXT("sounds/interface/on_mixkit-opening-software.wav"), NULL, SND_FILENAME);
							gamepadcontrol = true;
							guests[0].perms.gamepad = true;
							ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
							printf("Controlling the gamepad.\n");
						}
						if (event.userData.id == 26) {
							PlaySound(TEXT("sounds/interface/off_mixkit_negative_tone.wav"), NULL, SND_FILENAME);
							gamepadcontrol = false;
							guests[0].perms.gamepad = false;
							ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
							printf("No longer controlling the gamepad.\n");
						}
						/*
						if (event.userData.id == 28) {
							guests[0].perms.mouse = false;
							guests[0].perms.keyboard = false;
							guests[0].perms.gamepad = false;
							ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
						}
						*/
						// Press Key
						if (event.userData.id == 36) {
							system("espeak -v +f3 Press \"");
							printf("Press Key: %s\n", msg);
							char *presskey = msg;
							char command[100];
							strcpy(command, "espeak -v +f3 \"");
							//strcat(command, "Press ");
							strcat(command, presskey);
							strcat(command, "\"");
							system(command);
							//printf("Command: %s\n", command);
							//printf("PressKey: %s\n", presskey);
						}
						// Hold Key
						if (event.userData.id == 39) {
							system("espeak -v +f3 Hold \"");
							printf("Hold Key: %s\n", msg);
							char *presskey = msg;
							char command[100];
							strcpy(command, "espeak -v +f3 \"");
							strcat(command, presskey);
							strcat(command, "\"");
							system(command);
						}
						// Double Press
						if (event.userData.id == 50) {
							system("espeak -v +f3 Double_Press \"");
							printf("Double Press: %s\n", msg);
							char *presskey = msg;
							char command[100];
							strcpy(command, "espeak -v +f3 \"");
							strcat(command, presskey);
							strcat(command, "\"");
							system(command);
						}
						// Press Repeatedly
						if (event.userData.id == 51) {
							system("espeak -v +f3 Press_Repeatedly \"");
							printf("Press Repeatedly: %s\n", msg);
							char *presskey = msg;
							char command[100];
							strcpy(command, "espeak -v +f3 \"");
							strcat(command, presskey);
							strcat(command, "\"");
							system(command);
						}
						// TTS message
						if (event.userData.id == 42) {
							//printf("ENTROU\n");
							char tts[100];
							if (language == 0) {
								strcpy(tts, "espeak -vpt-pt+f2 \"");
							}
							else {
								strcpy(tts, "espeak -v+f3 \"");
							}
							printf("Mensagem recebida: %s\n", msg);
							char *mensagem = msg;
							//strcpy(command, "espeak -v +f3 \"");
							strcat(tts, mensagem);
							strcat(tts, "\"");
							system(tts);
						}
						if (event.userData.id == 43) {
							language = 1;
							printf("Portugues\n");
						}
						if (event.userData.id == 44) {
							language = 0;
							printf("English\n");
						}
						ParsecFree(parsec, msg);
					}
					break;
				default:
					break;
			}
		}
		// Ping Points
		while (pingpoint) { 
			GetCursorPos(&cursorPos);
			d = euclideanDistance(cursorPos.x, cursorPos.y, pinned.x, pinned.y);
			beepHertz = 500000 / d;
			beepTime =  d;
			waitTime =  d;
			if (beepHertz > 1200) beepHertz = 1200;
			if (beepTime > 1000) beepTime = 1000;
			else if (beepTime < 80) beepTime = 80;
			if (waitTime > 1000) waitTime = 1000;
			else if (waitTime < 80) waitTime = 80;
			Beep(beepHertz, beepTime);
			Sleep(waitTime);
			if (d < 75) {
				SetCursorPos(pinned.x, pinned.y);
				if (labelledping) {
					//printf("TEXT::: %s\n", text);
					char tts[100];
					if (language == 0) {
						strcpy(tts, "espeak -vpt-pt+f2 \"");
					}
					else {
						strcpy(tts, "espeak -v+f3 \"");
					}
					//char *mensagem = msg;
					//strcpy(command, "espeak -v +f3 \"");
					strcat(tts, text);
					strcat(tts, "\"");
					system(tts);
					//printf("TTS::: %s\n", tts);
				}
				else {
					printf("Position found!\n");
					PlaySound(TEXT("sounds/correct.wav"), NULL, SND_FILENAME);
				}
				pingpoint = false;
				labelledping = false;
				text[0] = '\0';
			}
			if (GetAsyncKeyState(VK_ESCAPE)){
				printf("Exited ping point mode.\n");
				PlaySound(TEXT("sounds/interface/off_mixkit-click-error.wav"), NULL, SND_FILENAME);
				pingpoint = false;
				labelledping = false;
				text[0] = '\0';
			}
			if (GetAsyncKeyState(VK_RETURN)){
				SetCursorPos(pinned.x, pinned.y);
				if (labelledping) {
					//printf("TEXT::: %s\n", text);
					char tts[100];
					if (language == 0) {
						strcpy(tts, "espeak -vpt-pt+f2 \"");
					}
					else {
						strcpy(tts, "espeak -v+f3 \"");
					}
					//char *mensagem = msg;
					//strcpy(command, "espeak -v +f3 \"");
					strcat(tts, text);
					strcat(tts, "\"");
					system(tts);
					//printf("TTS::: %s\n", tts);
				}
				else {
					printf("Gone to position!\a\n");
				}
				pingpoint = false;
				labelledping = false;
				text[0] = '\0';
			}
			ParsecHostEvent event;
			bool newEvent = ParsecHostPollEvents(parsec, 100, &event);
			if (newEvent) {
				/*
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
					printf("Exited ping point mode.\n");
					PlaySound(TEXT("sounds/interface/off_mixkit-click-error.wav"), NULL, SND_FILENAME);
					pingpoint = false;
					labelledping = false;
					text[0] = '\0';
					break;
				}
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
					SetCursorPos(pinned.x, pinned.y);
					if (labelledping) {
						printf("TEXT::: %s\n", text);
						char tts[100];
						if (language == 0) {
							strcpy(tts, "espeak -vpt-pt+f2 \"");
						}
						else {
							strcpy(tts, "espeak -v+f3 \"");
						}
						//char *mensagem = msg;
						//strcpy(command, "espeak -v +f3 \"");
						strcat(tts, text);
						strcat(tts, "\"");
						system(tts);
						printf("TTS::: %s\n", tts);
					}
					else {
						printf("Gone to position!\a\n");
					}
					pingpoint = false;
					labelledping = false;
					text[0] = '\0';
					break;
				}
				*/
				if (event.type != HOST_EVENT_USER_DATA) break;
				if (event.userData.id == 3) {
					pingpoint = false;
					labelledping = false;
					text[0] = '\0';
					printf("Ping point mode: Off\n");
					break;
				}
				char *msg = ParsecGetBuffer(parsec, event.userData.key);
				if (msg) {
					if (event.userData.id == 8 || event.userData.id == 45){
							char *pos = msg;
							char *temp = strtok(pos, ",");
							char *pinX = temp;
							temp = strtok(NULL, ",");
							char *pinY = temp;
							int sentX = atoi(pinX);
							int sentY = atoi(pinY);
							pinned.x = (sentX * hostResX) / clientResX;
							pinned.y = (sentY * hostResY) / clientResY;
							printf("Pinned position: (%d, %d)\n", pinned.x, pinned.y);
							if (event.userData.id == 45) {
								text[0] = '\0';
								temp = strtok(NULL, ",");
								strcat(text, temp);
							}
					}
					ParsecFree(parsec, msg);
				}
			}
		}
		// Steer mouse
		while (steermouse){
			GetCursorPos(&cursorPos);
			ParsecHostEvent event;
			bool newEvent = ParsecHostPollEvents(parsec, 100, &event);
			if (newEvent) {
				if (event.type != HOST_EVENT_USER_DATA) break;
				if (event.userData.id == 7) {
					steermouse = false;
					printf("Steer mode: Off\n");
					break;
				}
				// Right
				if (event.userData.id == 10){
					//printf("Old position: (%d, %d)\n", cursorPos.x, cursorPos.y);
					SetCursorPos(cursorPos.x + 10, cursorPos.y);
					//printf("Move Right: (%d, %d)\n", cursorPos.x, cursorPos.y);
				}
				// Left
				if (event.userData.id == 11){
					//printf("Old position: (%d, %d)\n", cursorPos.x, cursorPos.y);
					SetCursorPos(cursorPos.x - 10, cursorPos.y);
					//printf("Move Left: (%d, %d)\n", cursorPos.x, cursorPos.y);
				}
				// Down
				if (event.userData.id == 12){
					//printf("Old position: (%d, %d)\n", cursorPos.x, cursorPos.y);
					SetCursorPos(cursorPos.x, cursorPos.y + 10);
					//printf("Move Down: (%d, %d)\n", cursorPos.x, cursorPos.y);
				}
				// Up
				if (event.userData.id == 13){
					//printf("Old position: (%d, %d)\n", cursorPos.x, cursorPos.y);
					SetCursorPos(cursorPos.x, cursorPos.y - 10);
					//    printf("Move Up: (%d, %d)\n", cursorPos.x, cursorPos.y);
				}
			}
		}
		// Host turning off control sharing
		if (mousecontrol || keyboardcontrol || gamepadcontrol){
			if (GetAsyncKeyState(VK_ESCAPE)) {
				PlaySound(TEXT("sounds/interface/off_mixkit_negative_tone.wav"), NULL, SND_FILENAME);
				printf("Host turned off control sharing.\n");
				mousecontrol = false;
				keyboardcontrol = false;
				gamepadcontrol = false;
				guests[0].perms.mouse = false;
				guests[0].perms.keyboard = false;
				guests[0].perms.gamepad = false;
				ParsecHostSetPermissions(parsec, guests[0].id, &guests[0].perms);
				ParsecHostSendUserData(parsec, guests[0].id, 28, "Host turned off control sharing.");
			}
		}
	}

	ParsecFree(parsec, guests);

	except:

	ParsecDestroy(parsec);

	return 0;
}
