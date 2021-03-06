#include	"compiler.h"
#include	"inputmng.h"


typedef struct {
	short	key;
	UINT	bit;
} KEYBIND;

typedef struct {
	int		x;
	int		y;
	UINT	btn;
	UINT	key;
	UINT	kbs;
	KEYBIND	kb[32];
} INPMNG;

static INPMNG inpmng;

static const KEYBIND keybind[] = {
		{SDLK_UP,		KEY_UP},
		{SDLK_DOWN,		KEY_DOWN},
		{SDLK_LEFT,		KEY_LEFT},
		{SDLK_RIGHT,	KEY_RIGHT},
		{SDLK_RETURN,	KEY_ENTER},
		{SDLK_ESCAPE,	KEY_MENU},
		{SDLK_TAB,		KEY_SKIP}};		/* とりあえずね… */


void inputmng_init(void) {

	INPMNG	*im;

	im = &inpmng;
	ZeroMemory(im, sizeof(*im));
	im->kbs = sizeof(keybind)/sizeof(KEYBIND);
	CopyMemory(im->kb, keybind, sizeof(keybind));
}

void inputmng_buttondown(UINT bit) {

	INPMNG	*im;

	im = &inpmng;
	if (bit & LBUTTON_BIT) {
		im->btn |= LBUTTON_BIT | LBUTTON_DOWNBIT;
	}
	if (bit & RBUTTON_BIT) {
		im->btn |= RBUTTON_BIT | RBUTTON_DOWNBIT;
	}
}

void inputmng_buttonup(UINT bit) {

	INPMNG	*im;

	im = &inpmng;
	if (bit & LBUTTON_BIT) {
		im->btn |= LBUTTON_UPBIT;
		im->btn &= ~LBUTTON_BIT;
	}
	if (bit & RBUTTON_BIT) {
		im->btn |= RBUTTON_UPBIT;
		im->btn &= ~RBUTTON_BIT;
	}
}


// ----

void inputmng_keybind(short key, UINT bit) {

	INPMNG	*im;
	UINT	i;

	im = &inpmng;
	for (i=0; i<im->kbs; i++) {
		if (im->kb[i].key == key) {
			im->kb[i].bit = bit;
			return;
		}
	}
	if (im->kbs < (sizeof(im->kb)/sizeof(KEYBIND))) {
		im->kb[im->kbs].key = key;
		im->kb[im->kbs].bit = bit;
		im->kbs++;
	}
}

void inputmng_keyset(short key) {

	INPMNG	*im;
	UINT	i;

	im = &inpmng;
	for (i=0; i<im->kbs; i++) {
		if (im->kb[i].key == key) {
			im->key |= im->kb[i].bit;
			break;
		}
	}
}

void inputmng_keyreset(short key) {

	INPMNG	*im;
	UINT	i;

	im = &inpmng;
	for (i=0; i<im->kbs; i++) {
		if (im->kb[i].key == key) {
			im->key &= ~im->kb[i].bit;
			break;
		}
	}
}


// ----

UINT inputmng_getmouse(int *x, int *y) {

	INPMNG	*im;
	int		cx, cy;

	SDL_GetMouseState(&cx, &cy);
	im = &inpmng;
	if ((im->x != cx) || (im->y != cy)) {
		im->x = cx;
		im->y = cy;
		im->btn |= MOUSE_MOVEBIT;
	}
	if (x) {
		*x = im->x;
	}
	if (y) {
		*y = im->y;
	}
	return(im->btn);
}

void inputmng_resetmouse(UINT mask) {

	inpmng.btn &= mask;
}

UINT inputmng_getkey(void) {

	return(inpmng.key);
}

void inputmng_resetkey(UINT mask) {

	inpmng.key &= mask;
}

