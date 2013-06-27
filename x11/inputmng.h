/*	$Id: inputmng.h,v 1.2 2003/04/22 07:15:51 yui Exp $	*/

#define LBUTTON_BIT	(1 << 0)
#define RBUTTON_BIT	(1 << 1)
#define LBUTTON_DOWNBIT	(1 << 2)
#define RBUTTON_DOWNBIT	(1 << 3)
#define LBUTTON_UPBIT	(1 << 4)
#define RBUTTON_UPBIT	(1 << 5)
#define MOUSE_MOVEBIT	(1 << 6)

#define KEY_ENTER	0x01
#define KEY_MENU	0x02
#define KEY_SKIP	0x04
#define KEY_EXT		0x08
#define KEY_UP		0x10
#define KEY_DOWN	0x20
#define KEY_LEFT	0x40
#define KEY_RIGHT	0x80

#ifdef __cplusplus
extern "C" {
#endif

void inputmng_init(void);

void inputmng_buttondown(UINT bit);
void inputmng_buttonup(UINT bit);

void inputmng_keybind(short key, UINT bit);
void inputmng_keyset(short key);
void inputmng_keyreset(short key);

UINT inputmng_getmouse(int *x, int *y);
void inputmng_resetmouse(UINT mask);
UINT inputmng_getkey(void);
void inputmng_resetkey(UINT mask);

#ifdef __cplusplus
}
#endif
