/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

/* The bare pure X11 port done by Lionel 'BBrox' Ulmer */

#include "stdafx.h"
#include "scumm.h"
#include "gui.h"
#include "sound.h"

#include <sys/time.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <linux/soundcard.h>

#include <sched.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#undef BUILD_FOR_IPAQ

Scumm scumm;
ScummDebugger debugger;
Gui gui;
SoundEngine sound;
SOUND_DRIVER_TYPE snd_driv;
static unsigned char *local_fb;

static int window_width, window_height;
static int scumm_x, scumm_y;

static int x11_socket = -1;
static Display *display;
static int screen;
static Window window;
static GC black_gc;
static XImage *image;
static pthread_t sound_thread;
static int old_mouse_x, old_mouse_y;
static int old_mouse_h, old_mouse_w;
static bool has_mouse, hide_mouse;

#define MAX_NUMBER_OF_DIRTY_SQUARES 32
typedef struct {
  int x, y, w, h;
} dirty_square;
static dirty_square ds[MAX_NUMBER_OF_DIRTY_SQUARES];
static int num_of_dirty_square;

/* Milisecond-based timer management */
static struct timeval start_time;
static void init_timer(void) {
  gettimeofday(&start_time, NULL);
}
static unsigned int get_ms_from_start(void) {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return (((current_time.tv_sec  - start_time.tv_sec ) * 1000) + 
	  ((current_time.tv_usec - start_time.tv_usec) / 1000));
}

#define FRAG_SIZE 4096
static void *sound_and_music_thread(void *params) {
  /* Init sound */
  int sound_fd, param, frag_size;
  unsigned char sound_buffer[FRAG_SIZE];

  sound_fd = open("/dev/dsp", O_WRONLY);
  audio_buf_info info;
  if (sound_fd < 0) {
    error("Error opening sound device !\n");
    exit(1);
  }
  param = 0;
  frag_size = FRAG_SIZE /* audio fragment size */;
  while (frag_size) {
    frag_size >>= 1;
    param++;
  }
  param--;
  param |= /* audio_fragment_num */ 3 << 16;
  if (ioctl(sound_fd, SNDCTL_DSP_SETFRAGMENT, &param) != 0) {
    error("Error in the SNDCTL_DSP_SETFRAGMENT ioctl !\n");
    exit(1);
  }
  param = AFMT_S16_LE;
  if (ioctl(sound_fd, SNDCTL_DSP_SETFMT, &param) == -1) {
    perror("Error in the SNDCTL_DSP_SETFMT ioctl !\n");
    exit(1);
  }
  if (param != AFMT_S16_LE) {
    error("AFMT_S16_LE not supported !\n");
    exit(1);
  }
  param = 2;
  if (ioctl(sound_fd, SNDCTL_DSP_CHANNELS, &param) == -1) {
    error("Error in the SNDCTL_DSP_CHANNELS ioctl !\n");
    exit(1);
  }
  if (param != 2) {
    error("Stereo mode not supported !\n");
    exit(1);
  }
  param = 22050;
  if (ioctl(sound_fd, SNDCTL_DSP_SPEED, &param) == -1) {
    perror("Error in the SNDCTL_DSP_SPEED ioctl !\n");
    exit(1);
  }
  if (param != 22050) {
    error("22050 kHz not supported !\n");
    exit(1);
  }
  if (ioctl(sound_fd, SNDCTL_DSP_GETOSPACE, &info) != 0) {
    perror("SNDCTL_DSP_GETOSPACE");
    exit(-1);
  }

  while (1) {
    unsigned short *buf = (unsigned short *) sound_buffer;

    scumm.mixWaves((short *) sound_buffer, FRAG_SIZE >> 2);
    /* Now convert to stereo */
    for (int i = ((FRAG_SIZE >> 2) - 1); i >= 0; i--) {
      buf[2 * i + 1] = buf[2 * i] = buf[i];
    }
    if (write(sound_fd, sound_buffer, FRAG_SIZE) != FRAG_SIZE) {
      error("Bad write to the audio device !\n");
      exit(1);
    }
  }

  return NULL;
}

/* Function used to hide the mouse cursor */
static void create_empty_cursor(Display *display,
				int screen,
				Window window) {
  XColor bg;
  Pixmap pixmapBits;
  Cursor cursor = None;
  static const char data[] = { 0 };

  bg.red = bg.green = bg.blue = 0x0000;
  pixmapBits = XCreateBitmapFromData(display, XRootWindow(display, screen), data, 1, 1);
  if (pixmapBits) {
    cursor = XCreatePixmapCursor(display, pixmapBits, pixmapBits,
				 &bg, &bg, 0, 0 );
    XFreePixmap(display, pixmapBits);
  } 
  XDefineCursor(display, window, cursor);
}

void cd_playtrack(int track, int offset, int delay) {
  /* No CD on the iPAQ => stub function */
}

void BoxTest(int num) {
  /* No debugger on the iPAQ => stub function */
}

/* Initialize the graphics sub-system */
void initGraphics(Scumm *s, bool fullScreen) {
  char buf[512], *gameName;
  static XShmSegmentInfo shminfo;
  XWMHints *wm_hints;
  XGCValues values;
  XTextProperty window_name;
  char *name = (char *) &buf;
  
  /* For the window title */
  sprintf(buf, "ScummVM - %s", gameName = s->getGameName());
  free(gameName);

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    error("Could not open display !\n");
    exit(1);
  }
  screen = DefaultScreen(display);
  x11_socket = ConnectionNumber(display);

  window_width = 320;
  window_height = 200;
  scumm_x = 0;
  scumm_y = 0;
  window = XCreateSimpleWindow(display, XRootWindow(display, screen), 0, 0,
			       320, 200, 0, 0, 0);
  wm_hints = XAllocWMHints();
  if (wm_hints == NULL) {
    error("Not enough memory to allocate Hints !\n");
    exit(1);
  }
  wm_hints->flags = InputHint | StateHint;
  wm_hints->input = True;
  wm_hints->initial_state = NormalState;
  XStringListToTextProperty( &name, 1, &window_name );
  XSetWMProperties(display, window, &window_name, &window_name,
		   NULL /* argv */, 0 /* argc */, NULL /* size hints */, wm_hints, NULL /* class hints */ );

  XSelectInput(display, window,
	       ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
	       ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);
  image = XShmCreateImage(display, DefaultVisual(display, screen), 16, ZPixmap, NULL, &shminfo, 320, 200);
  shminfo.shmid = shmget(IPC_PRIVATE, 320 * 200 * 2, IPC_CREAT | 0700);
  shminfo.shmaddr = (char *) shmat(shminfo.shmid, 0, 0);
  image->data = shminfo.shmaddr;
  shminfo.readOnly = False;
  if (XShmAttach(display, &shminfo) == 0) {
    error("Could not attach shared memory segment !\n");
    exit(1);
  }
  shmctl(shminfo.shmid, IPC_RMID, 0);

  values.foreground = BlackPixel(display, screen);
  black_gc = XCreateGC(display, window, GCForeground, &values);

  XMapWindow(display, window);
  XFlush(display);

  while (1) {
    XEvent event;
    XNextEvent(display, &event);
    switch (event.type) {
    case Expose:
      goto out_of_loop;
    }
  }
 out_of_loop:
  create_empty_cursor(display, screen, window);

  /* And finally start the music thread */
  pthread_create(&sound_thread, NULL, sound_and_music_thread, NULL);

  /* Initialize the 'local' frame buffer */
  local_fb = (unsigned char *) malloc(320 * 200 * sizeof(unsigned char));
}

void setShakePos(Scumm *s, int shake_pos) {

}

#define AddDirtyRec(xi,yi,wi,hi) 				\
  if (num_of_dirty_square < MAX_NUMBER_OF_DIRTY_SQUARES) {	\
    ds[num_of_dirty_square].x = xi;				\
    ds[num_of_dirty_square].y = yi;				\
    ds[num_of_dirty_square].w = wi;				\
    ds[num_of_dirty_square].h = hi;				\
    num_of_dirty_square++;					\
  }
void blitToScreen(Scumm *s, byte *src, int x, int y, int w, int h) {
  unsigned char *dst = local_fb + 320 * y + x;

  if (h<=0)	return;

  hide_mouse = true;
  if (has_mouse) {
    s->drawMouse();
  }
  
  AddDirtyRec(x, y, w, h);
  while (h-- > 0) {
    memcpy(dst, src, w);
    dst += 320;
    src += 320;
  }
}

#define BAK_WIDTH 40
#define BAK_HEIGHT 40
unsigned char old_backup[BAK_WIDTH * BAK_HEIGHT];

void drawMouse(Scumm *s, int xdraw, int ydraw, int w, int h, byte *buf, bool visible) {
  unsigned char *dst,*bak;

  if ((xdraw >= 320) || ((xdraw + w) <= 0) ||
      (ydraw >= 200) || ((ydraw + h) <= 0)) {
    if (hide_mouse) visible = false;
    if (has_mouse) has_mouse = false;
    if (visible) has_mouse = true;
    return;
  }

  if (hide_mouse)
    visible = false;
  
  assert(w<=BAK_WIDTH && h<=BAK_HEIGHT);

  if (has_mouse) {
    int old_h = old_mouse_h;

    has_mouse = false;
    AddDirtyRec(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

    dst = local_fb + (old_mouse_y * 320) + old_mouse_x;
    bak = old_backup;
    
    while (old_h > 0) {
      memcpy(dst, bak, old_mouse_w);
      bak += BAK_WIDTH;
      dst += 320;
      old_h--;
    }
  }

  if (visible) {
    int real_w;
    int real_h;
    int real_h_2;
    unsigned char *dst2;

    if (ydraw < 0) {
      real_h = h + ydraw;
      buf += (-ydraw) * w;
      ydraw = 0;
    } else {
      real_h = (ydraw + h) > 200 ? (200 - ydraw) : h;
    }
    if (xdraw < 0) {
      real_w = w + xdraw;
      buf += (-xdraw);
      xdraw = 0;
    } else {
      real_w = (xdraw + w) > 320 ? (320 - xdraw) : w;
    }
    
    dst = local_fb + (ydraw * 320) + xdraw;
    dst2 = dst;
    bak = old_backup;
        
    has_mouse = true;

    AddDirtyRec(xdraw, ydraw, real_w, real_h);
    old_mouse_x = xdraw;
    old_mouse_y = ydraw;
    old_mouse_w = real_w;
    old_mouse_h = real_h;
    
    real_h_2 = real_h;
    while (real_h_2 > 0) {
      memcpy(bak, dst, real_w);
      bak += BAK_WIDTH;
      dst += 320;
      real_h_2--;
    }
    while (real_h > 0) {
      int width = real_w;
      while (width > 0) {
	unsigned char color = *buf;
	if (color != 0xFF) {
	  *dst2 = color;
	}
	buf++;
	dst2++;
	width--;
      }
      buf += w - real_w;
      dst2 += 320 - real_w;
      real_h--;
    }
  }
}

static unsigned short palette[256];
static void update_palette(Scumm *s) {
  int first = s->_palDirtyMin;
  int num = s->_palDirtyMax - first + 1;
  int i;
  unsigned char *data = s->_currentPalette;
  unsigned short *pal = &(palette[first]);

  data += first*3;
  for (i = 0; i < num; i++, data += 3) {
    *pal++ = ((data[0] & 0xF8) << 8) | ((data[1] & 0xFC) << 3) | (data[2] >> 3);
  }
  s->_palDirtyMax = -1;
  s->_palDirtyMin = 0x3E8; 
}

static void update_screen(Scumm *s, const dirty_square *d, dirty_square *dout) {
  int x, y;
  unsigned char *ptr_src = local_fb + (320 * d->y) + d->x;
  unsigned short *ptr_dst = ((unsigned short *) image->data) + (320 * d->y) + d->x;
  for (y = 0; y < d->h; y++) {
    for (x = 0; x < d->w; x++) {
      *ptr_dst++ = palette[*ptr_src++];
    }
    ptr_dst += 320 - d->w;
    ptr_src += 320 - d->w;
  }
  if (d->x < dout->x) dout->x = d->x;
  if (d->y < dout->y) dout->y = d->y;
  if ((d->x + d->w) > dout->w) dout->w = d->x + d->w;
  if ((d->y + d->h) > dout->h) dout->h = d->y + d->h;
}

void updateScreen(Scumm *s) {
  bool full_redraw = false;
  bool need_redraw = false;
  static const dirty_square ds_full = { 0, 0, 320, 200 };
  dirty_square dout = {320, 200, 0, 0 };
  
  if (s->_fastMode&2)
    return;
  
  if (hide_mouse) {
    hide_mouse = false;
    s->drawMouse();
  }

  if (s->_palDirtyMax != -1) {
    update_palette(s);
    full_redraw = true;
    num_of_dirty_square = 0;
  } else if (num_of_dirty_square > MAX_NUMBER_OF_DIRTY_SQUARES) {
    full_redraw = true;
    num_of_dirty_square = 0;
  }

  if (full_redraw) {
    update_screen(s, &ds_full, &dout);
    need_redraw = true;
  } else if (num_of_dirty_square > 0) {
    need_redraw = true;
    while (num_of_dirty_square > 0) {
      num_of_dirty_square--;
      update_screen(s, &(ds[num_of_dirty_square]), &dout);
    }
  }
  if (need_redraw == true) {
    XShmPutImage(display, window, DefaultGC(display, screen), image, 
		 dout.x, dout.y, 
		 scumm_x + dout.x, scumm_y + dout.y, 
		 dout.w - dout.x, dout.h - dout.y, 
		 0);
    XFlush(display);
  }
}

/* This function waits for 'msec_delay' miliseconds and handles external events */
void waitForTimer(Scumm *s, int msec_delay) {
  int start_time = get_ms_from_start();
  int end_time;
  fd_set rfds;
  struct timeval tv;
  XEvent event;

  if (s->_fastMode&2)
    msec_delay = 0;
  else if (s->_fastMode&1)
    msec_delay = 10;
  end_time = start_time + msec_delay;


  while (1) {
    FD_ZERO(&rfds);
    FD_SET(x11_socket, &rfds);

    msec_delay = end_time - get_ms_from_start();
    tv.tv_sec = 0;
    if (msec_delay <= 0) {
      tv.tv_usec = 0;
    } else {
      tv.tv_usec = msec_delay * 1000;
    }    
    if (select(x11_socket + 1, &rfds, NULL, NULL, &tv) == 0)
      break; /* This is the timeout */
    while (XPending(display)) {
      XNextEvent(display,&event);
      switch (event.type) {
      case Expose: {
	int real_w, real_h;
	int real_x, real_y;
	real_x = event.xexpose.x;
	real_y = event.xexpose.y;
	real_w = event.xexpose.width;
	real_h = event.xexpose.height;

	if (real_x < scumm_x) {
	  real_w -= scumm_x - real_x;
	  real_x = 0;
	} else {
	  real_x -= scumm_x;
	}
	if (real_y < scumm_y) {
	  real_h -= scumm_y - real_y;
	  real_y = 0;
	} else {
	  real_y -= scumm_y;
	}
	if ((real_h <= 0) || (real_w <= 0)) break;
	if ((real_x >= 320) || (real_y >= 200)) break;
	
	if ((real_x + real_w) >= 320) {
	  real_w = 320 - real_x;
	}
	if ((real_y + real_h) >= 200) {
	  real_h = 200 - real_y;
	}

	/* Compute the intersection of the expose event with the real ScummVM display zone */
	AddDirtyRec(real_x, real_y, real_w, real_h);
      } break;

      case KeyPress:
	/* Do nothing for now... Will be useful to implement 'pixel hunting mode' */
	break;

      case KeyRelease:
	/* I am using keycodes here and NOT keysyms to be sure that even if the user
	   remaps his iPAQ's keyboard, it will still work.
	*/
	switch (event.xkey.keycode) {
	case 9: /* Escape on my PC */
	case 130: /* Calendar on the iPAQ */
	  s->_keyPressed = 27;
	  break;

	case 71: /* F5 on my PC */
	case 128: /* Record on the iPAQ */
	  s->_keyPressed = 319;
	  break;
	  
	case 65: /* Space on my PC */
	case 131: /* Schedule on the iPAQ */
	  s->_keyPressed = 32;
	  break;

	default: {
	    KeySym xsym;
	    xsym = XKeycodeToKeysym(display, event.xkey.keycode, 0);
	    if ((xsym >= 'a') && (xsym <= 'z') && (event.xkey.state & 0x01)) xsym &= ~0x20; /* Handle shifted keys */
	    s->_keyPressed = xsym;
	  }
	}
	break;
	
      case ButtonPress:
	if (event.xbutton.button == 1)
	  s->_leftBtnPressed |= msClicked|msDown;
	else if (event.xbutton.button == 3)
	  s->_rightBtnPressed |= msClicked|msDown;
	break;

      case ButtonRelease:
	if (event.xbutton.button == 1)
	  s->_leftBtnPressed &= ~msDown;
	else if (event.xbutton.button == 3)
	  s->_rightBtnPressed &= ~msDown;
	break;	

      case MotionNotify: {
	int newx,newy;
	newx = event.xmotion.x - scumm_x;
	newy = event.xmotion.y - scumm_y;
	if ((newx != s->mouse.x) || (newy != s->mouse.y)) {
	  s->mouse.x = newx;
	  s->mouse.y = newy;
	  s->drawMouse();
	  updateScreen(s);
	}
      } break;

      case ConfigureNotify: {
	if ((window_width != event.xconfigure.width) ||
	    (window_height != event.xconfigure.height)) {
	  window_width = event.xconfigure.width;
	  window_height = event.xconfigure.height;
	  scumm_x = (window_width - 320) / 2;
	  scumm_y = (window_height - 200) / 2;
	  XFillRectangle(display, window, black_gc, 0, 0, window_width, window_height);
	}
      } break;

      default:
	printf("%d\n", event.type);
	break;
      }
    }
  }
}

/* Main function for the system-dependent part. Needs to handle :
    - handle command line arguments
    - initialize all the 'globals' (sound driver, Scumm object, ...)
    - do the main loop of the game
*/
int main(int argc, char* argv[]) {
  int delta;
  int last_time, new_time;
  
  sound.initialize(&scumm, &snd_driv);
  
  scumm._gui = &gui;
  scumm.scummMain(argc, argv);
  
  if (!(scumm._features & GF_SMALL_HEADER))
    gui.init(&scumm);
  num_of_dirty_square = 0;

  /* Start the milisecond counter */
  init_timer();
  last_time = 0;
  delta = 0;
  while (1) {
    updateScreen(&scumm);
    
    new_time = get_ms_from_start();
    waitForTimer(&scumm, delta * 15 + last_time - new_time);
    last_time = get_ms_from_start();
    
    if (gui._active) {
      gui.loop();
      delta = 5;
    } else {
      delta = scumm.scummLoop(delta);
    }
  }
  
  return 0;
}
