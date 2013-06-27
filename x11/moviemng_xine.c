/*	$Id$	*/

#include "compiler.h"

#if defined(SUPPORT_MOVIE_XINE)

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <SDL_syswm.h>

#include <xine.h>
#include <xine/xineutils.h>

#include "gamecore.h"
#include "x11.h"
#include "moviemng.h"
#include "taskmng.h"

#if defined(MITSHM)
#include <X11/extensions/XShm.h>
#endif

struct wininfo {
	int x, y;
	int width, height;
	double aspect;
};

struct xineinfo {
	xine_t *xine;
	xine_video_port_t *video_port;
	xine_audio_port_t *audio_port;
	xine_stream_t *stream;
	xine_event_queue_t *queue;
};

static BOOL running = FALSE;


static void
dest_size_cb(void *user_data, int video_width, int video_height, double video_pixel_aspect, int *dest_width, int *dest_height, double *dest_pixel_aspect)
{
	struct wininfo *wi = (struct wininfo *)user_data;

	(void)video_width;
	(void)video_height;
	(void)video_pixel_aspect;

	*dest_width = wi->width;
	*dest_height = wi->height;
	*dest_pixel_aspect = wi->aspect;
}

static void
frame_output_cb(void *user_data, int video_width, int video_height, double video_pixel_aspect, int *dest_x, int *dest_y, int *dest_width, int *dest_height, double *dest_pixel_aspect, int *win_x, int *win_y)
{
	struct wininfo *wi = (struct wininfo *)user_data;

	(void)video_width;
	(void)video_height;
	(void)video_pixel_aspect;

	*dest_x = 0;
	*dest_y = 0;
	*win_x = wi->x;
	*win_y = wi->y;
	*dest_width = wi->width;
	*dest_height = wi->height;
	*dest_pixel_aspect = wi->aspect;
}

static void
event_listener(void *user_data, const xine_event_t *event)
{

	(void)user_data;

	switch (event->type) { 
	case XINE_EVENT_UI_PLAYBACK_FINISHED:
		running = 0;
		break;
	}
}

void
movie_play(const char *moviefile)
{
	/* xine */
	struct xineinfo xi = {
		NULL, NULL, NULL, NULL, NULL,
	};
	x11_visual_t visual;
	struct wininfo wininfo;

	/* x11 */
	Display *display;
	int screen;
	Window window, rootWindow, childWindow;
	Atom proto, delwin;
	XEvent ev;
	KeySym ksym;
	XWindowAttributes attr;
	unsigned int width, height, border, depth;
	int x, y;
	int shmcomptype;

	/* */
	char config_path[MAX_PATH];
	SDL_SysWMinfo info;

	if (moviefile == NULL || moviefile[0] == '\0')
		return;

	memset(&info, 0, sizeof(info));
	info.version.major = SDL_MAJOR_VERSION;
	info.version.minor = SDL_MINOR_VERSION;
	if (SDL_GetWMInfo(&info) <= 0)
		return;

	display = info.info.x11.display;
	screen = DefaultScreen(display);
	window = info.info.x11.window;

	XLockDisplay(display);
	proto = XInternAtom(display, "WM_PROTOCOLS", 0);
	delwin = XInternAtom(display, "WM_DELETE_WINDOW", 0);

#if defined(MITSHM) && defined(ShmCompletion)
	if (XShmQueryExtension(display))
		shmcomptype = XShmGetEventBase(display) + ShmCompletion;
	else
#endif
		shmcomptype = -1;

	XGetGeometry(display, window, &rootWindow, &x, &y, &width, &height, &border, &depth);
	XGetWindowAttributes(display, window, &attr);
	XTranslateCoordinates(display, window, attr.root, 0, 0, &x, &y, &childWindow);

	XSync(display, False);
	XUnlockDisplay(display);

	memset(&visual, 0, sizeof(visual));
	visual.display = display;
	visual.screen = screen;
	visual.d = window;
	visual.dest_size_cb = dest_size_cb;
	visual.frame_output_cb = frame_output_cb;
	visual.user_data = &wininfo;

	wininfo.x = x;
	wininfo.y = y;
	wininfo.width = width;
	wininfo.height = height;
	wininfo.aspect = 1.0;

	xi.xine = xine_new();
	if (xi.xine == NULL)
		goto stop;
	milstr_ncpy(config_path, xine_get_homedir(), sizeof(config_path));
	milstr_ncat(config_path, "/.xine/config", sizeof(config_path));
	xine_config_load(xi.xine, config_path);
	xine_init(xi.xine);

	xi.video_port = xine_open_video_driver(xi.xine, NULL, XINE_VISUAL_TYPE_X11, &visual);
	if (xi.video_port == NULL)
		goto stop;
	xi.audio_port = xine_open_audio_driver(xi.xine, NULL, NULL);
	if (xi.audio_port == NULL)
		goto stop;
	xi.stream = xine_stream_new(xi.xine, xi.audio_port, xi.video_port);
	if (xi.stream == NULL)
		goto stop;
	xi.queue = xine_event_new_queue(xi.stream);
	if (xi.queue == NULL)
		goto stop;
	xine_event_create_listener_thread(xi.queue, event_listener, &wininfo);
	xine_port_send_gui_data(xi.video_port, XINE_GUI_SEND_DRAWABLE_CHANGED, (void *)window);
	xine_port_send_gui_data(xi.video_port, XINE_GUI_SEND_VIDEOWIN_VISIBLE, (void *)1);

	if (!xine_open(xi.stream, moviefile))
		goto stop;
	if (!xine_play(xi.stream, 0, 0))
		goto stop;

	running = TRUE;
	while (running) {
		if (XPending(display) > 0) {
			XNextEvent(display, &ev);
			switch (ev.type) {
			case Expose:
				if ((ev.xexpose.count == 0)
				 && (ev.xexpose.window == window)) {
					xine_gui_send_vo_data(xi.stream, XINE_GUI_SEND_EXPOSE_EVENT, &ev);
				}
				break;

			case KeyPress:
				ksym = XLookupKeysym(&ev.xkey, 0);
				switch (ksym) {
				case XK_q:
				case XK_Q:
				case XK_space:
				case XK_Escape:
				case XK_Return:
				case XK_KP_Enter:
					running = FALSE;
					break;
				}
				break;

			case ButtonPress:
				switch (ev.xbutton.button) {
				case 1:
				case 2:
				case 3:
					running = FALSE;
					break;
				}
				break;

			case ConfigureNotify:
				if ((ev.xconfigure.x == 0) && (ev.xconfigure.y == 0)) {
					XLockDisplay(display);
					XGetWindowAttributes(display, ev.xconfigure.window, &attr);
					XTranslateCoordinates(display, ev.xconfigure.window, attr.root, 0, 0, &x, &y, &childWindow);
					XUnlockDisplay(display);
					wininfo.x = x;
					wininfo.y = y;
				} else {
					wininfo.x = ev.xconfigure.x;
					wininfo.y = ev.xconfigure.y;
				}
				wininfo.width = ev.xconfigure.width;
				wininfo.height = ev.xconfigure.height;
				break;

			case ClientMessage:
				if ((ev.xclient.message_type == proto)
				 && ((Atom)ev.xclient.data.l[0] == delwin)) {
					running = FALSE;
					taskmng_exit();
				}
				break;

			default:
				if (ev.type == shmcomptype) {
					xine_gui_send_vo_data(xi.stream, XINE_GUI_SEND_COMPLETION_EVENT, &ev);
				}
				break;
			}
		} else {
			usleep(1);
		}
	}
	XFlush(display);

stop:
	if (xi.stream) {
		xine_close(xi.stream);
	}
	if (xi.queue) {
		xine_event_dispose_queue(xi.queue);
	}
	if (xi.stream) {
		xine_dispose(xi.stream);
	}
	if (xi.xine) {
		if (xi.audio_port)
			xine_close_audio_driver(xi.xine, xi.audio_port);  
		if (xi.video_port)
			xine_close_video_driver(xi.xine, xi.video_port);  
		xine_exit(xi.xine);
	}
}
#endif
