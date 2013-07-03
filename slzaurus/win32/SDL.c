#include	"compiler.h"


static	char	__sdl_error[256];


int SDL_InitSubSystem(DWORD flags) {

	return(0);
}

void SDL_Quit(void) {
}

char *SDL_GetError(void) {

	return(__sdl_error);
}

void __sdl_seterror(const char *error) {

	milstr_ncpy(__sdl_error, error, sizeof(__sdl_error));
}


// ----

static	const char	szClassName[] = "Windebug-SDL";
static	const char	szCaptionName[] = "SDL-Window";

		BOOL		__sdl_avail = FALSE;
		HWND		__sdl_hWnd = NULL;
		SDL_Surface	*__sdl_vsurf = NULL;
		int			__sdl_eventw = 0;
		int			__sdl_mousex = 0;
		int			__sdl_mousey = 0;
extern	void		__sdl_videoinit(void);
extern	void		__sdl_videopaint(HWND hWnd, SDL_Surface *screen);
extern	void		__sdl_audio_cb(UINT msg, HWAVEOUT hwo, WAVEHDR *whd);

typedef struct {
	short	wincode;
	short	sdlcode;
} SDLKEYTBL;

static const SDLKEYTBL sdlkeytbl[] = {
		{VK_UP,		SDLK_UP},		{VK_DOWN,	SDLK_DOWN},
		{VK_LEFT,	SDLK_LEFT},		{VK_RIGHT,	SDLK_RIGHT},
		{VK_RETURN,	SDLK_RETURN},	{VK_ESCAPE,	SDLK_ESCAPE},
		{VK_TAB,	SDLK_TAB}};

static short cnvsdlkey(short wparam) {

const SDLKEYTBL	*key;
const SDLKEYTBL	*keyterm;

	key = sdlkeytbl;
	keyterm = key + (sizeof(sdlkeytbl)/sizeof(SDLKEYTBL));
	while(key < keyterm) {
		if (key->wincode == wparam) {
			return(key->sdlcode);
		}
		key++;
	}
	return(SDLK_UNKNOWN);
}

static LRESULT CALLBACK SdlProc(HWND hWnd, UINT msg,
											WPARAM wParam, LPARAM lParam) {

	PAINTSTRUCT	ps;
	HDC			hdc;
	SDL_Event	event;

	switch (msg) {
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			__sdl_videopaint(hWnd, __sdl_vsurf);
			EndPaint(hWnd, &ps);
			break;

		case WM_MOUSEMOVE:
			__sdl_mousex = LOWORD(lParam);
			__sdl_mousey = HIWORD(lParam);
			break;

		case WM_LBUTTONDOWN:
			__sdl_mousex = LOWORD(lParam);
			__sdl_mousey = HIWORD(lParam);
			ZeroMemory(&event, sizeof(event));
			event.button.type = SDL_MOUSEBUTTONDOWN;
			event.button.button = SDL_BUTTON_LEFT;
			SDL_PushEvent(&event);
			break;

		case WM_LBUTTONUP:
			__sdl_mousex = LOWORD(lParam);
			__sdl_mousey = HIWORD(lParam);
			ZeroMemory(&event, sizeof(event));
			event.button.type = SDL_MOUSEBUTTONUP;
			event.button.button = SDL_BUTTON_LEFT;
			SDL_PushEvent(&event);
			break;

		case WM_RBUTTONDOWN:
			__sdl_mousex = LOWORD(lParam);
			__sdl_mousey = HIWORD(lParam);
			ZeroMemory(&event, sizeof(event));
			event.button.type = SDL_MOUSEBUTTONDOWN;
			event.button.button = SDL_BUTTON_RIGHT;
			SDL_PushEvent(&event);
			break;

		case WM_RBUTTONUP:
			__sdl_mousex = LOWORD(lParam);
			__sdl_mousey = HIWORD(lParam);
			ZeroMemory(&event, sizeof(event));
			event.button.type = SDL_MOUSEBUTTONUP;
			event.button.button = SDL_BUTTON_RIGHT;
			SDL_PushEvent(&event);
			break;

		case WM_KEYDOWN:
			ZeroMemory(&event, sizeof(event));
			event.key.type = SDL_KEYDOWN;
			event.key.keysym.sym = cnvsdlkey((short)wParam);
			if (event.key.keysym.sym != SDLK_UNKNOWN) {
				SDL_PushEvent(&event);
			}
			break;

		case WM_KEYUP:
			ZeroMemory(&event, sizeof(event));
			event.key.type = SDL_KEYUP;
			event.key.keysym.sym = cnvsdlkey((short)wParam);
			if (event.key.keysym.sym != SDLK_UNKNOWN) {
				SDL_PushEvent(&event);
			}
			break;

		case WM_CLOSE:
			ZeroMemory(&event, sizeof(event));
			event.type = SDL_QUIT;
			SDL_PushEvent(&event);
			__sdl_avail = FALSE;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
#if 1
		case MM_WOM_DONE:
			__sdl_audio_cb(MM_WOM_DONE, (HWAVEOUT)wParam, (WAVEHDR *)lParam);
			break;
#endif
		default:
			return(DefWindowProc(hWnd, msg, wParam, lParam));
	}
	return(0L);
}

static BOOL sdlinit(void) {

	HINSTANCE	hInstance;
	WNDCLASS	sdlcls;

	hInstance = (HINSTANCE)GetWindowLong(HWND_DESKTOP, GWL_HINSTANCE);
	sdlcls.style = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
	sdlcls.lpfnWndProc = SdlProc;
	sdlcls.cbClsExtra = 0;
	sdlcls.cbWndExtra = 0;
	sdlcls.hInstance = hInstance;
	sdlcls.hIcon = NULL;
	sdlcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	sdlcls.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	sdlcls.lpszMenuName = NULL;
	sdlcls.lpszClassName = szClassName;
	if (!RegisterClass(&sdlcls)) {
		return(FAILURE);
	}
	__sdl_hWnd = CreateWindowEx(0,
						szClassName, szCaptionName,
						WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION |
						WS_MINIMIZEBOX,
						CW_USEDEFAULT, CW_USEDEFAULT, 320, 240,
						NULL, NULL, hInstance, NULL);
	if (__sdl_hWnd == NULL) {
		return(FAILURE);
	}
	ShowWindow(__sdl_hWnd, SW_SHOW);
	UpdateWindow(__sdl_hWnd);
	__sdl_avail = TRUE;
	return(SUCCESS);
}

static void sdlterm(void) {
}


int main(int argc, char **argv) {

	int		r;

	__sdl_videoinit();
	if (sdlinit() != SUCCESS) {
		return(0);
	}
	r = SDL_main(argc, argv);
	sdlterm();
	return(r);
}

