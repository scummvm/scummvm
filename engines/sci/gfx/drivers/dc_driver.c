/***************************************************************************
 dc_driver.c Copyright (C) 2002-2005 Walter van Niftrik


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

   Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#include <kos/thread.h>
#include <kos/sem.h>
#include <dc/maple.h>
#include <dc/maple/mouse.h>
#include <dc/maple/keyboard.h>
#include <dc/maple/controller.h>
#include <dc/pvr.h>

#include <sci_memory.h>
#include <gfx_driver.h>
#include <gfx_tools.h>

#include "keyboard.h"

/* Event queue struct */

struct dc_event_t {
	sci_event_t event;
	struct dc_event_t *next;
};

#define SCI_DC_RENDER_PVR	(1 << 0) /* 0 = VRAM rendering, 1 = PVR */
#define SCI_DC_REFRESH_50HZ	(1 << 1) /* 0 = 60Hz refresh rate, 1 = 50Hz */

static int flags = 0;

struct _dc_state {
	/* 0 = static buffer, 1 = back buffer, 2 = front buffer */
	/* Visual maps */
	byte *visual[3];
	/* Priority maps */
	byte *priority[2];
	/* Line pitch of visual buffers */
	int line_pitch[3];

	/* PVR only */
	/* Polygon header */
	pvr_poly_hdr_t pvr_hdr;
	/* Polygon header for virtual keyboard */
	pvr_poly_hdr_t pvr_hdr_vkbd;
	/* Texture for virtual keyboard */
	uint16 *vkbd_txr;

	/* Pointers to first and last event in the event queue */
	struct dc_event_t *first_event;
	struct dc_event_t *last_event;
	
	/* Semaphores for mouse pointer location and event queue updates */
	semaphore_t *sem_event, *sem_pointer;
	
	/* The dx and dy of the mouse pointer since last get_event() */
	int pointer_dx, pointer_dy;
	
	/* The current bucky state of the keys */
	int buckystate;
	
	/* Thread ID of the input thread */
	kthread_t *thread;
	
	/* Flag to stop the input thread. (!0 = run, 0 = stop) */
	int run_thread;

	/* Controller key repeat timer */
	int timer;

	/* Controller state */
	unsigned int cont_state;

	/* Virtual keyboard on/off flag */
	int vkbd;
};

#define S ((struct _dc_state *)(drv->state))

#define XFACT drv->mode->xfact
#define YFACT drv->mode->yfact
#define BYTESPP drv->mode->bytespp

#define DC_MOUSE_LEFT (1<<0)
#define DC_MOUSE_RIGHT (1<<1)

#define DC_KEY_CAPSLOCK (1<<0)
#define DC_KEY_NUMLOCK (1<<1)
#define DC_KEY_SCRLOCK (1<<2)
#define DC_KEY_INSERT (1<<3)

static void
pvr_init_gfx(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
/* Initialises the graphics driver for PVR rendering mode
** Parameters: (_gfx_driver *) drv: The driver to use
**             (int) xfact, yfact: X and Y scaling factors. (xfact, yfact)
**                     has to be either (1, 1) or (2, 2).
**             (int) bytespp: The display depth in bytes per pixel. Must be 2.
** Returns   : void
*/
{
	pvr_poly_cxt_t cxt;

	/* Initialize PVR to defaults and set background color to black */

	if (flags & SCI_DC_REFRESH_50HZ)
		vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
	else
		vid_set_mode(DM_640x480, PM_RGB565);
	
	pvr_init_defaults();
	pvr_set_bg_color(0,0,0);

	/* Allocate and initialize texture RAM */
	S->visual[2] = pvr_mem_malloc(512*256*bytespp*xfact*yfact);
	S->line_pitch[2] = 512*bytespp*xfact;
	memset(S->visual[2], 0, 512*256*bytespp*xfact*yfact);

	/* Create textured polygon context */
	pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 |
		PVR_TXRFMT_NONTWIDDLED, 512*xfact, 256*yfact, S->visual[2], 0);

	/* Create polygon header from context */
	pvr_poly_compile(&(S->pvr_hdr), &cxt);

	/* Allocate and initialize texture RAM for virtual keyboard */
	S->vkbd_txr = pvr_mem_malloc(512*64*bytespp*xfact*yfact);
	memset(S->vkbd_txr, 0, 512*64*bytespp*xfact*yfact);

	/* Create textured polygon context for virtual keyboard */
	pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_RGB565 |
		PVR_TXRFMT_NONTWIDDLED, 512*xfact, 64*yfact, S->vkbd_txr, 0);

	/* Create polygon header from context for virtual keyboard */
	pvr_poly_compile(&(S->pvr_hdr_vkbd), &cxt);

	vkbd_init((uint16 *) S->vkbd_txr, 512);
	vkbd_draw();
}

static void
pvr_do_frame(struct _gfx_driver *drv)
/* Renders a frame for PVR rendering mode
** Parameters: (_gfx_driver *) drv: The driver to use
** Returns   : void
*/
{
	pvr_vertex_t vert;
	
	/* Wait until we can send another frame to the PVR */
	pvr_wait_ready();
	
	/* Start a new scene */
	pvr_scene_begin();

	/* Start an opaque polygon list */
	pvr_list_begin(PVR_LIST_OP_POLY);

	/* Submit polygon header */
	pvr_prim(&(S->pvr_hdr), sizeof(S->pvr_hdr));
	
	/* Create and submit vertices */
	vert.flags = PVR_CMD_VERTEX;
	vert.x = 0.0f;
	vert.y = 480.0f;
	vert.z = 1.0f;
	vert.u = 0.0f;
	vert.v = 0.78125f;
	vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));
	
	vert.y = 0.0f;
	vert.v = 0.0f;
	pvr_prim(&vert, sizeof(vert));
	
	vert.x = 640.0f;
	vert.y = 480.0f;
	vert.u = 0.625f;
	vert.v = 0.78125f;
	pvr_prim(&vert, sizeof(vert));
	
	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.y = 0.0f;
	vert.v = 0.0f;
	pvr_prim(&vert, sizeof(vert));
	
	/* End list */
	pvr_list_finish();

	/* Display virtual keyboard */
	if (S->vkbd) {
		/* Start an translucent polygon list */
		pvr_list_begin(PVR_LIST_TR_POLY);
	
		/* Submit polygon header */
		pvr_prim(&(S->pvr_hdr_vkbd), sizeof(S->pvr_hdr_vkbd));
		
		/* Create and submit vertices */
		vert.flags = PVR_CMD_VERTEX;
		vert.x = 0.0f;
		vert.y = 480.0f;
		vert.z = 1.0f;
		vert.u = 0.0f;
		vert.v = 0.625f;
		vert.argb = PVR_PACK_COLOR(0.8f, 1.0f, 1.0f, 1.0f);
		vert.oargb = 0;
		pvr_prim(&vert, sizeof(vert));
		
		vert.y = 400.0f;
		vert.v = 0.0f;
		pvr_prim(&vert, sizeof(vert));
		
		vert.x = 640.0f;
		vert.y = 480.0f;
		vert.u = 0.625f;
		vert.v = 0.625f;
		pvr_prim(&vert, sizeof(vert));
		
		vert.flags = PVR_CMD_VERTEX_EOL;
		vert.y = 400.0f;
		vert.v = 0.0f;
		pvr_prim(&vert, sizeof(vert));
		
		/* End list */
		pvr_list_finish();
	}

	/* End scene */
	pvr_scene_finish();
}

static void
vram_init_gfx(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
/* Initialises the graphics driver for VRAM rendering mode
** Parameters: (_gfx_driver *) drv: The driver to use
**             (int) xfact, yfact: X and Y scaling factors. (xfact, yfact)
**                     has to be either (1, 1) or (2, 2).
**             (int) bytespp: The display depth in bytes per pixel. Must be
**                     either 2 or 4.
** Returns   : void
*/
{
	int vidres = 0, vidcol = 0;

	/* Center screen vertically */
	S->visual[2] = (byte *) vram_s+320*xfact*20*yfact*bytespp;

	S->line_pitch[2] = 320*xfact*bytespp;

	memset(S->visual[2], 0, 320*xfact*240*yfact*bytespp);

	switch(bytespp) {
		case 2:
			vidcol = PM_RGB565;
			break;
		case 4:
			vidcol = PM_RGB888;
	}

	if (flags & SCI_DC_REFRESH_50HZ) switch (xfact) {
		case 1:
			vidres = DM_320x240_PAL;
			break;
		case 2:
			vidres = DM_640x480_PAL_IL;
	}
	else switch (xfact) {
		case 1:
			vidres = DM_320x240;
			break;
		case 2:
			vidres = DM_640x480;
	}
		
	vid_set_mode(vidres, vidcol);

	vkbd_init((uint16 *) (S->visual[2] + 320 * xfact * 200 * yfact * bytespp), 320);
}

static void
vram_hide_keyboard(struct _gfx_driver *drv)
/* Hides the virtual keyboard in VRAM rendering mode
** Parameters: (_gfx_driver *) drv: The driver to use
** Returns   : void
*/
{
	vid_set_start(0);
	memset4(S->visual[2] + 320 * XFACT * 200 * YFACT * BYTESPP, 0, 320 * XFACT * 40 * YFACT * BYTESPP);
}

static void
vram_show_keyboard(struct _gfx_driver *drv)
/* Displays the virtual keyboard in VRAM rendering mode
** Parameters: (_gfx_driver *) drv: The driver to use
** Returns   : void
*/
{
	vid_set_start(320 * XFACT * 20 * YFACT * BYTESPP);
	vkbd_draw();
}

static int
dc_add_event(struct _gfx_driver *drv, sci_event_t *event)
/* Adds an event to the end of an event queue
** Parameters: (_gfx_driver *) drv: The driver to use
**             (sci_event_t *) event: The event to add
** Returns   : (int) 1 on success, 0 on error
*/
{
	struct dc_event_t *dc_event;
	if (!(dc_event = sci_malloc(sizeof(dc_event)))) {
		printf("Error: Could not reserve memory for event\n");
		return 0;
	}
	
	dc_event->event = *event;
	dc_event->next = NULL;

	/* Semaphore prevents get_event() from removing the last event in
	** the event queue while a next event is being attached to it.
	*/

	sem_wait(S->sem_event);
	if (!(S->last_event)) {
		/* Event queue is empty */
		S->first_event = dc_event;
		S->last_event = dc_event;
		sem_signal(S->sem_event);
		return 1;
	}

	S->last_event->next = dc_event;
	S->last_event = dc_event;
	sem_signal(S->sem_event);
	return 1;
}

static int
dc_map_key(int *keystate, uint8 key)
/* Converts a kos keycode to a freesci keycode. This function also adjusts
** the caps lock, num lock, scroll lock and insert states in keystate.
** Parameters: (int *) keystate: Pointer to the keystate variable
**             (uint8) key: The kos keycode to convert
** Returns   : (int) Converted freesci keycode on success, 0 on error
*/
{
	if ((key >= KBD_KEY_A) && (key <= KBD_KEY_Z))
		return 'a' + (key - KBD_KEY_A);

	if ((key >= KBD_KEY_1) && (key <= KBD_KEY_9))
		return '1' + (key - KBD_KEY_1);

	switch (key) {
		case KBD_KEY_0:			return '0';
		case KBD_KEY_BACKSPACE:		return SCI_K_BACKSPACE;
		case KBD_KEY_TAB:		return 9;
		case KBD_KEY_ESCAPE:		return SCI_K_ESC;
		case KBD_KEY_ENTER:
		case KBD_KEY_PAD_ENTER:		return SCI_K_ENTER;
		case KBD_KEY_DEL:
		case KBD_KEY_PAD_PERIOD:	return SCI_K_DELETE;
		case KBD_KEY_INSERT:
		case KBD_KEY_PAD_0:		*keystate ^= DC_KEY_INSERT;
						return SCI_K_INSERT;
		case KBD_KEY_END:
		case KBD_KEY_PAD_1:		return SCI_K_END;
		case KBD_KEY_DOWN:
		case KBD_KEY_PAD_2:		return SCI_K_DOWN;
		case KBD_KEY_PGDOWN:
		case KBD_KEY_PAD_3:		return SCI_K_PGDOWN;
		case KBD_KEY_LEFT:
		case KBD_KEY_PAD_4:		return SCI_K_LEFT;
		case KBD_KEY_PAD_5:		return SCI_K_CENTER;
		case KBD_KEY_RIGHT:
		case KBD_KEY_PAD_6:		return SCI_K_RIGHT;
		case KBD_KEY_HOME:
		case KBD_KEY_PAD_7:		return SCI_K_HOME;
		case KBD_KEY_UP:
		case KBD_KEY_PAD_8:		return SCI_K_UP;
		case KBD_KEY_PGUP:
		case KBD_KEY_PAD_9:		return SCI_K_PGUP;
		case KBD_KEY_F1:		return SCI_K_F1;
		case KBD_KEY_F2:		return SCI_K_F2;
		case KBD_KEY_F3:		return SCI_K_F3;
		case KBD_KEY_F4:		return SCI_K_F4;
		case KBD_KEY_F5:		return SCI_K_F5;
		case KBD_KEY_F6:		return SCI_K_F6;
		case KBD_KEY_F7:		return SCI_K_F7;
		case KBD_KEY_F8:		return SCI_K_F8;
		case KBD_KEY_F9:		return SCI_K_F9;
		case KBD_KEY_F10:		return SCI_K_F10;
		case KBD_KEY_PAD_PLUS:		return '+';
		case KBD_KEY_SLASH:
		case KBD_KEY_PAD_DIVIDE:	return '/';
		case KBD_KEY_MINUS:
		case KBD_KEY_PAD_MINUS:		return '-';
		case KBD_KEY_PAD_MULTIPLY:	return '*';
		case KBD_KEY_COMMA:		return ',';
		case KBD_KEY_PERIOD:		return '.';
		case KBD_KEY_BACKSLASH:		return '\\';
		case KBD_KEY_SEMICOLON:		return ';';
		case KBD_KEY_QUOTE:		return '\'';
		case KBD_KEY_LBRACKET:		return '[';
		case KBD_KEY_RBRACKET:		return ']';
		case KBD_KEY_TILDE:		return '`';
		case KBD_KEY_PLUS:		return '=';
		case KBD_KEY_SPACE:		return ' ';
		case KBD_KEY_CAPSLOCK:		*keystate ^= DC_KEY_CAPSLOCK;
						return 0;
		case KBD_KEY_SCRLOCK:		*keystate ^= DC_KEY_SCRLOCK;
						return 0;
		case KBD_KEY_PAD_NUMLOCK:	*keystate ^= DC_KEY_NUMLOCK;
						return 0;
	}

	printf("Warning: Unmapped key: %02x\n", key);
	
	return 0;
}

static void
dc_input_thread(struct _gfx_driver *drv)
/* Thread that checks the dreamcast keyboard and mouse states. It adds
** keypresses and mouseclicks to the end of the event queue. It also updates
** drv->state.buckystate and drv->state.pointer_dx/dy.
** Parameters: (_gfx_driver *) drv: The driver to use
** Returns   : void
*/
{
	/* State of mouse buttons */
	unsigned int mstate = 0;
	/* Last key pressed */
	unsigned int lastkey = KBD_KEY_NONE;
	/* State of caps lock, scroll lock, num lock and insert keys */
	int keystate = DC_KEY_INSERT;

	while (S->run_thread) {
		maple_device_t *kaddr = NULL, *maddr, *caddr;
		mouse_state_t *mouse;
		kbd_state_t *kbd = 0;
		cont_state_t *cont;
		uint8 key;
		int skeys;	
		int bucky = 0, vkbd_bucky = 0;
		sci_event_t event;

		if (!(flags & SCI_DC_RENDER_PVR))
			/* Sleep for 10ms */
			thd_sleep(10);
		else
			pvr_do_frame(drv);

		/* Keyboard handling */
		/* Experimental workaround for the Mad Catz adapter problem */
		if (!kaddr)
			kaddr = maple_enum_type(0, MAPLE_FUNC_KEYBOARD);
		if (kaddr && (kbd = maple_dev_status(kaddr))) {
			key = kbd->cond.keys[0];
			skeys = kbd->shift_keys;
		
			bucky =	((skeys & (KBD_MOD_LCTRL | KBD_MOD_RCTRL))?
				  SCI_EVM_CTRL : 0) |
				((skeys & (KBD_MOD_LALT | KBD_MOD_RALT))?
				  SCI_EVM_ALT : 0) |
				((skeys & KBD_MOD_LSHIFT)?
				  SCI_EVM_LSHIFT : 0) |
				((skeys & KBD_MOD_RSHIFT)?
				  SCI_EVM_RSHIFT : 0) |
				((keystate & DC_KEY_NUMLOCK)?
				  SCI_EVM_NUMLOCK : 0) |
				((keystate & DC_KEY_SCRLOCK)?
				  SCI_EVM_SCRLOCK : 0) |
				((keystate & DC_KEY_INSERT)?
				  SCI_EVM_INSERT : 0);
		
			/* If a shift key is pressed when caps lock is on, set
			** both shift key states to 0. If no shift keys are
			** pressed when caps lock is on, set both shift key
			** states to 1
			*/

			if (keystate & DC_KEY_CAPSLOCK) {
				if ((bucky & SCI_EVM_LSHIFT) ||
				  (bucky & SCI_EVM_RSHIFT))
					bucky &=
					  ~(SCI_EVM_LSHIFT | SCI_EVM_RSHIFT);
				else bucky |= SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;
			}
		
			if ((key != lastkey) && (key != KBD_KEY_NONE)) {
				event.type = SCI_EVT_KEYBOARD;
				event.data = dc_map_key(&keystate, key);
				event.buckybits = bucky | vkbd_bucky;
				if (event.data) dc_add_event(drv, &event);
			}		
			lastkey = key;
		}
		else kaddr = NULL;

		/* Mouse handling */
		if ((maddr = maple_enum_type(0, MAPLE_FUNC_MOUSE)) &&
		  (mouse = maple_dev_status(maddr))) {

			/* Enable mouse support */
			drv->capabilities |= GFX_CAPABILITY_MOUSE_SUPPORT;
			
			/* Semaphore prevents get_event() from accessing
			** S->pointer_dx/dy while they are being updated
			*/
			sem_wait(S->sem_pointer);
			S->pointer_dx += mouse->dx;
			S->pointer_dy += mouse->dy;
			sem_signal(S->sem_pointer);

			if ((mouse->buttons & MOUSE_LEFTBUTTON) &&
			  !(mstate & DC_MOUSE_LEFT)) {
				event.type = SCI_EVT_MOUSE_PRESS;
				event.data = 1;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate |= DC_MOUSE_LEFT;
			}
			if ((mouse->buttons & MOUSE_RIGHTBUTTON) &&
			  !(mstate & DC_MOUSE_RIGHT)) {
				event.type = SCI_EVT_MOUSE_PRESS;
				event.data = 2;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate |= DC_MOUSE_RIGHT;
			}
			if (!(mouse->buttons & MOUSE_LEFTBUTTON) &&
			  (mstate & DC_MOUSE_LEFT)) {
				event.type = SCI_EVT_MOUSE_RELEASE;
				event.data = 1;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate &= ~DC_MOUSE_LEFT;
			}
			if (!(mouse->buttons & MOUSE_RIGHTBUTTON) &&
			  (mstate & DC_MOUSE_RIGHT)) {
				event.type = SCI_EVT_MOUSE_RELEASE;
				event.data = 2;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate &= ~DC_MOUSE_RIGHT;
			}
		}
		else if ((caddr = maple_enum_type(0, MAPLE_FUNC_CONTROLLER)) &&
		  (cont = maple_dev_status(caddr))) {
			/* Enable mouse support */
			drv->capabilities |= GFX_CAPABILITY_MOUSE_SUPPORT;
			
			/* Semaphore prevents get_event() from accessing
			** S->pointer_dx/dy while they are being updated
			*/
			sem_wait(S->sem_pointer);
			S->pointer_dx += cont->joyx/20;
			S->pointer_dy += cont->joyy/20;
			sem_signal(S->sem_pointer);

			if ((cont->ltrig > 5) &&
			  !(mstate & DC_MOUSE_LEFT)) {
				event.type = SCI_EVT_MOUSE_PRESS;
				event.data = 1;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate |= DC_MOUSE_LEFT;
			}
			if ((cont->rtrig > 5) &&
			  !(mstate & DC_MOUSE_RIGHT)) {
				event.type = SCI_EVT_MOUSE_PRESS;
				event.data = 2;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate |= DC_MOUSE_RIGHT;
			}
			if ((cont->ltrig <= 5) &&
			  (mstate & DC_MOUSE_LEFT)) {
				event.type = SCI_EVT_MOUSE_RELEASE;
				event.data = 1;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate &= ~DC_MOUSE_LEFT;
			}
			if ((cont->rtrig <= 5) &&
			  (mstate & DC_MOUSE_RIGHT)) {
				event.type = SCI_EVT_MOUSE_RELEASE;
				event.data = 2;
				event.buckybits = bucky | vkbd_bucky;
				dc_add_event(drv, &event);
				mstate &= ~DC_MOUSE_RIGHT;
			}
			if (S->timer > 0)
				S->timer--;
			if ((cont->buttons != S->cont_state) || !S->timer) {
				S->cont_state = cont->buttons;
				S->timer = 25;
				if (cont->buttons & CONT_START) {
					S->vkbd = S->vkbd ^ 1;
					if (!(flags & SCI_DC_RENDER_PVR)) {
						if(S->vkbd)
							vram_show_keyboard(drv);
						else
							vram_hide_keyboard(drv);
					}
				}
				if (S->vkbd) {
					if (cont->buttons & CONT_DPAD_RIGHT)
						vkbd_handle_input(KBD_RIGHT);
					if (cont->buttons & CONT_DPAD_LEFT)
						vkbd_handle_input(KBD_LEFT);
					if (cont->buttons & CONT_DPAD_UP)
						vkbd_handle_input(KBD_UP);
					if (cont->buttons & CONT_DPAD_DOWN)
						vkbd_handle_input(KBD_DOWN);
					if (cont->buttons & CONT_A) {
						int vkbd_key;
						if (vkbd_get_key(&vkbd_key, &vkbd_bucky)) {
							event.type = SCI_EVT_KEYBOARD;
							event.data = vkbd_key;
							event.buckybits = bucky | vkbd_bucky;
							dc_add_event(drv, &event);
						}
					}
				}
				else {
					event.data = 0;
					if (cont->buttons & CONT_DPAD_RIGHT)
						event.data = SCI_K_RIGHT;
					else if (cont->buttons & CONT_DPAD_LEFT)
						event.data = SCI_K_LEFT;
					else if (cont->buttons & CONT_DPAD_UP)
						event.data = SCI_K_UP;
					else if (cont->buttons & CONT_DPAD_DOWN)
						event.data = SCI_K_DOWN;
					event.type = SCI_EVT_KEYBOARD;
					event.buckybits = bucky | vkbd_bucky;
					if (event.data) dc_add_event(drv, &event);
				}
				event.data = 0;
				if (cont->buttons & CONT_B)
					event.data = SCI_K_ENTER;
				else if (cont->buttons & CONT_X)
					event.data = ' ';
				else if (cont->buttons & CONT_Y)
					event.data = SCI_K_ESC;
				event.type = SCI_EVT_KEYBOARD;
				event.buckybits = bucky | vkbd_bucky;
				if (event.data)
					dc_add_event(drv, &event);
			}
		}
		else drv->capabilities &= ~GFX_CAPABILITY_MOUSE_SUPPORT;

		S->buckystate = bucky | vkbd_bucky;
	}
}

static uint32
dc_get_color(struct _gfx_driver *drv, gfx_color_t col)
/* Converts a color as described in col to it's representation in memory
** Parameters: (_gfx_driver *) drv: The driver to use
**             (gfx_color_t) color: The color to convert
** Returns   : (uint32) the color's representation in memory
*/
{
	uint32 retval;
	uint32 temp;

	retval = 0;

	temp = col.visual.r;
	temp |= temp << 8;
	temp |= temp << 16;
	retval |= (temp >> drv->mode->red_shift) & (drv->mode->red_mask);
	temp = col.visual.g;
	temp |= temp << 8;
	temp |= temp << 16;
	retval |= (temp >> drv->mode->green_shift) & (drv->mode->green_mask);
	temp = col.visual.b;
	temp |= temp << 8;
	temp |= temp << 16;
	retval |= (temp >> drv->mode->blue_shift) & (drv->mode->blue_mask);

	return retval;
}

static void
dc_draw_line_buffer(byte *buf, int line, int bytespp, int x1,
  int y1, int x2, int y2, uint32 color)
/* Draws a line in a buffer
** This function was taken from sdl_driver.c with small modifications
** Parameters: (byte *) buf: The buffer to draw in
**             (int) line: line pitch of buf in bytes
**             (int) bytespp: number of bytes per pixel of buf
**             (int) x1, y1, x2, y2: The line to draw: (x1,y1)-(x2,y2).
**             (uint32) color: The color to draw with
** Returns   : (void)
*/
{
	int pixx, pixy;
	int x,y;
	int dx,dy;
	int sx,sy;
	int swaptmp;
	uint8 *pixel;

	dx = x2 - x1;
	dy = y2 - y1;
	sx = (dx >= 0) ? 1 : -1;
	sy = (dy >= 0) ? 1 : -1;

	dx = sx * dx + 1;
	dy = sy * dy + 1;
	pixx = bytespp;
	pixy = line;
	pixel = ((uint8*) buf) + pixx * x1 + pixy * y1;
	pixx *= sx;
	pixy *= sy;
	if (dx < dy) {
		swaptmp = dx; dx = dy; dy = swaptmp;
		swaptmp = pixx; pixx = pixy; pixy = swaptmp;
	}

	x=0;
	y=0;
	switch(bytespp) {
		case 1:
			for(; x < dx; x++, pixel += pixx) {
				*pixel = color;
				y += dy;
				if (y >= dx) {
					y -= dx;
					pixel += pixy;
				}
			}
			break;
		case 2:
			for (; x < dx; x++, pixel += pixx) {
				*(uint16*)pixel = color;
				y += dy;
				if (y >= dx) {
					y -= dx;
					pixel += pixy;
				}
			}
			break;
		case 4:
			for(; x < dx; x++, pixel += pixx) {
				*(uint32*)pixel = color;
				y += dy;
				if (y >= dx) {
					y -= dx;
					pixel += pixy;
				}
			}
			break;
	}

}

static void
dc_draw_filled_rect_buffer(byte *buf, int line, int bytespp, rect_t rect,
  uint32 color)
/* Draws a filled rectangle in a buffer
** Parameters: (byte *) buf: The buffer to draw in
**             (int) line: line pitch of buf in bytes
**             (int) bytespp: number of bytes per pixel of buf
**             (rect_t) rect: The rectangle to fill
**             (uint32) color: The fill color
** Returns   : (void)
*/
{
	buf += rect.y*line + rect.x*bytespp;
	int i;
	
	switch (bytespp) {
		case 1:	for (i = 0; i<rect.yl; i++) {
				memset(buf, color, rect.xl);
				buf += line;
			}
			break;
		case 2:	for (i = 0; i<rect.yl; i++) {
				memset2(buf, color, rect.xl*2);
				buf += line;
			}
			break;
		case 4:	for (i = 0; i<rect.yl; i++) {
				memset4(buf, color, rect.xl*4);
				buf += line;
			}
	}
}


static void
dc_copy_rect_buffer(byte *src, byte *dest, int srcline, int destline,
  int bytespp, rect_t sr, point_t dp)
/* Copies a rectangle from one buffer to another
** Parameters: (byte *) src: The source buffer
**             (byte *) dest: The destination buffer
**             (int) srcline: line pitch of src in bytes
**             (int) destline: line pitch of dest in bytes
**             (int) bytespp: number of bytes per pixel of src and dest
**             (rect_t) sr: Rectangle of src to copy
**             (point_t) dp: Left top corner in dest where copy should go
** Returns   : (void)
*/
{
	src += sr.y*srcline + sr.x*bytespp;
	dest += dp.y*destline + dp.x*bytespp;
	int i;
	
	switch (bytespp) {
		case 1:	for (i = 0; i<sr.yl; i++) {
				memcpy(dest, src, sr.xl);
				src += srcline;
				dest += destline;
			}
			break;
		case 2:	for (i = 0; i<sr.yl; i++) {
				memcpy2(dest, src, sr.xl*2);
				src += srcline;
				dest += destline;
			}
			break;
		case 4:	for (i = 0; i<sr.yl; i++) {
				memcpy4(dest, src, sr.xl*4);
				src += srcline;
				dest += destline;
			}
	}
}

static int
dc_set_parameter(struct _gfx_driver *drv, char *attribute, char *value)
{
	if (!strcasecmp(attribute, "render_mode")) {
		if (!strcasecmp(value, "vram")) {
			flags &= ~SCI_DC_RENDER_PVR;
			return GFX_OK;
		}
		else if (!strcasecmp(value, "pvr")) {
			flags |= SCI_DC_RENDER_PVR;
			return GFX_OK;
		}
		else {
			sciprintf("Fatal error: Invalid value `%s' specified for attribute `render_mode'\n", value);
			return GFX_FATAL;
		}
	}
	if (!strcasecmp(attribute, "refresh_rate")) {
		if (!strcasecmp(value, "60Hz")) {
			flags &= ~SCI_DC_REFRESH_50HZ;
			return GFX_OK;
		}
		else if (!strcasecmp(value, "50Hz")) {
			flags |= SCI_DC_REFRESH_50HZ;
			return GFX_OK;
		}
		else {
			sciprintf("Fatal error: Invalid value `%s' specified for attribute `refresh_rate'\n", value);
			return GFX_FATAL;
		}
	}
	
	sciprintf("Fatal error: Attribute '%s' does not exist\n", attribute);
	return GFX_FATAL;
}


static int
dc_init_specific(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
{
	int i;
	int rmask = 0, gmask = 0, bmask = 0, rshift = 0, gshift = 0;
	int bshift = 0;

	sciprintf("Initialising video mode\n");

	pvr_shutdown();

	if (!drv->state /* = S */)
		drv->state = sci_malloc(sizeof(struct _dc_state));
	if (!S)
		return GFX_FATAL;

	if ((flags & SCI_DC_RENDER_PVR) && ((xfact != 1 && xfact != 2)
	  || bytespp != 2 || xfact != yfact)) {
		sciprintf("Error: PVR rendering mode does not support "
		  "buffers with scale factors (%d,%d) and bpp=%d\n",
		  xfact, yfact, bytespp);
		return GFX_ERROR;
	}
	else if ((xfact != 1 && xfact != 2) || (bytespp != 2 && bytespp != 4)
	  || xfact != yfact) {
		sciprintf("Error: VRAM rendering mode does not support "
		  "buffers with scale factors (%d,%d) and bpp=%d\n",
		  xfact, yfact, bytespp);
		return GFX_ERROR;
	}

	for (i = 0; i < 2; i++) {
		if (!(S->priority[i] = sci_malloc(320*xfact*200*yfact)) ||
		  !(S->visual[i] = sci_malloc(320*xfact*200*yfact* bytespp))) {
			sciprintf("Error: Could not reserve memory for buffer\n");
			return GFX_ERROR;
		}
	}	

	for (i = 0; i < 2; i++) {
		S->line_pitch[i] = 320*xfact*bytespp;
		memset(S->visual[i], 0, 320*xfact*200*yfact*bytespp);
		memset(S->priority[i], 0, 320*xfact*200*yfact);
	}

	S->pointer_dx = 0;
	S->pointer_dy = 0;
	S->buckystate = 0;
	S->timer = 0;
	S->vkbd = 0;

	switch(bytespp) {
		case 2:	rmask = 0xF800;
			gmask = 0x7E0;
			bmask = 0x1F;
			rshift = 16;
			gshift = 21;
			bshift = 27;
			break;
		case 4:	rmask = 0xFF0000;
			gmask = 0xFF00;
			bmask = 0xFF;
			rshift = 8;
			gshift = 16;
			bshift = 24;
	}

	if (!(flags & SCI_DC_RENDER_PVR))
		vram_init_gfx(drv, xfact, yfact, bytespp);
	else
		pvr_init_gfx(drv, xfact, yfact, bytespp);

	drv->mode = gfx_new_mode(xfact, yfact, bytespp, rmask, gmask, bmask, 0,
	  rshift, gshift, bshift, 0, 0, 0);

	printf("Video mode initialisation completed succesfully\n");
	
	S->run_thread = 1;

	S->thread = thd_create((void *) dc_input_thread, drv);

	S->first_event = NULL;
	S->last_event = NULL;
	
	if (!(S->sem_event = sem_create(1)) ||
	  !(S->sem_pointer = sem_create(1))) {
		printf("Error: Could not reserve memory for semaphore\n");
		return GFX_ERROR;
	};

	return GFX_OK;
}

static int
dc_init(struct _gfx_driver *drv)
{
	if (dc_init_specific(drv, 1, 1, 2) != GFX_OK)
		return GFX_FATAL;
	
	return GFX_OK;
}

static void
dc_exit(struct _gfx_driver *drv)
{
	if (S) {
		sciprintf("Freeing graphics buffers\n");
		sci_free(S->visual[0]);
		sci_free(S->visual[1]);
		sci_free(S->priority[0]);
		sci_free(S->priority[1]);
		if (flags & SCI_DC_RENDER_PVR) {
			pvr_mem_free(S->visual[2]);
			pvr_mem_free(S->vkbd_txr);
		}

		sciprintf("Waiting for input thread to exit... ");
		S->run_thread = 0;
		thd_wait(S->thread);
		sciprintf("ok\n");

		sciprintf("Freeing semaphores\n");
		sem_destroy(S->sem_event);
		sem_destroy(S->sem_pointer);
		sci_free(S);
		drv->state /* = S */ = NULL;
	}
}

	/*** Drawing operations ***/
	
static int
dc_draw_line(struct _gfx_driver *drv, point_t start, point_t end,
	      gfx_color_t color, gfx_line_mode_t line_mode,
	      gfx_line_style_t line_style)
{
	uint32 scolor;
	int xfact = (line_mode == GFX_LINE_MODE_FINE)? 1: XFACT;
	int yfact = (line_mode == GFX_LINE_MODE_FINE)? 1: YFACT;
	int xsize = XFACT*320;
	int ysize = YFACT*200;
	
	int xc, yc;
	int x1, y1, x2, y2;

	scolor = dc_get_color(drv, color);

	for (xc = 0; xc < xfact; xc++)
		for (yc = 0; yc < yfact; yc++) {
			x1 = start.x + xc;
			y1 = start.y + yc;
			x2 = end.x + xc;
			y2 = end.y + yc;

			if (x1 < 0)
				x1 = 0;
			if (x2 < 0)
				x2 = 0;
			if (y1 < 0)
				y1 = 0;
			if (y2 < 0)
				y2 = 0;
				
			if (x1 > xsize)
				x1 = xsize;
			if (x2 >= xsize)
				x2 = xsize - 1;
			if (y1 > ysize)
				y1 = ysize;
			if (y2 >= ysize)
				y2 = ysize - 1;

			if (color.mask & GFX_MASK_VISUAL)
				dc_draw_line_buffer(S->visual[1],
				  XFACT*320*BYTESPP, BYTESPP, x1, y1, x2, y2,
				  dc_get_color(drv, color));

			if (color.mask & GFX_MASK_PRIORITY)
				dc_draw_line_buffer(S->priority[1], XFACT*320,
				  1, x1, y1, x2, y2, color.priority);
		}

	return GFX_OK;
}

static int
dc_draw_filled_rect(struct _gfx_driver *drv, rect_t rect,
  gfx_color_t color1, gfx_color_t color2, gfx_rectangle_fill_t shade_mode)
{
	if (color1.mask & GFX_MASK_VISUAL)
		dc_draw_filled_rect_buffer(S->visual[1], S->line_pitch[1],
		  BYTESPP, rect, dc_get_color(drv, color1));
	
	if (color1.mask & GFX_MASK_PRIORITY)
		dc_draw_filled_rect_buffer(S->priority[1], XFACT*320, 1, rect,
		  color1.priority);

	return GFX_OK;
}

	/*** Pixmap operations ***/

static int
dc_register_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	return GFX_ERROR;
}

static int
dc_unregister_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	return GFX_ERROR;
}

static int
dc_draw_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
		rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	int bufnr = (buffer == GFX_BUFFER_STATIC)? 0:1;
	
	return gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest,
	  S->visual[bufnr], S->line_pitch[bufnr], S->priority[bufnr], XFACT*320,
	  1, 0);
}	

static int
dc_grab_pixmap(struct _gfx_driver *drv, rect_t src, gfx_pixmap_t *pxm,
		gfx_map_mask_t map)
{
	switch (map) {
		case GFX_MASK_VISUAL:
			dc_copy_rect_buffer(S->visual[1], pxm->data,
			  S->line_pitch[1], src.xl*BYTESPP, BYTESPP, src,
			  gfx_point(0, 0));
			pxm->xl = src.xl;
			pxm->yl = src.yl;
			return GFX_OK;
		case GFX_MASK_PRIORITY:
			dc_copy_rect_buffer(S->priority[1], pxm->index_data,
			  XFACT*320, src.xl, 1, src, gfx_point(0, 0));
			pxm->index_xl = src.xl;
			pxm->index_yl = src.yl;
			return GFX_OK;
		default:
			sciprintf("Error: attempt to grab pixmap from invalid map");
			return GFX_ERROR;
	}
}


	/*** Buffer operations ***/

static int
dc_update(struct _gfx_driver *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	int tbufnr = (buffer == GFX_BUFFER_BACK)? 1:2;

	dc_copy_rect_buffer(S->visual[tbufnr-1], S->visual[tbufnr],
	  S->line_pitch[tbufnr-1], S->line_pitch[tbufnr], BYTESPP, src, dest);
		
	if ((tbufnr == 1) && (src.x == dest.x) && (src.y == dest.y)) 
		dc_copy_rect_buffer(S->priority[0], S->priority[1], XFACT*320,
		  XFACT*320, 1, src, dest);
	
	return GFX_OK;
}

static int
dc_set_static_buffer(struct _gfx_driver *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{
	memcpy4(S->visual[0], pic->data, XFACT*320 * YFACT*200 * BYTESPP);
	memcpy4(S->priority[0], priority->index_data, XFACT*320 * YFACT*200);
	return GFX_OK;
}

	/*** Palette operations ***/

static int
dc_set_palette(struct _gfx_driver *drv, int index, byte red, byte green, byte blue)
{
	return GFX_ERROR;
}


	/*** Mouse pointer operations ***/

static int
dc_set_pointer (struct _gfx_driver *drv, gfx_pixmap_t *pointer)
{
	return GFX_ERROR;
}

	/*** Event management ***/

static sci_event_t
dc_get_event(struct _gfx_driver *drv)
{
	sci_event_t event;
	struct dc_event_t *first;
	sem_wait(S->sem_pointer);
	drv->pointer_x += S->pointer_dx;
	drv->pointer_y += S->pointer_dy;
	S->pointer_dx = 0;
	S->pointer_dy = 0;
	sem_signal(S->sem_pointer);

	if (drv->pointer_x < 0)
		drv->pointer_x = 0;
	if (drv->pointer_x >= 320*XFACT)
		drv->pointer_x = 320*XFACT-1;
	if (drv->pointer_y < 0)
		drv->pointer_y = 0;
	if (drv->pointer_y >= 200*YFACT)
		drv->pointer_y = 200*YFACT-1;

	sem_wait(S->sem_event);
	first = S->first_event;

	if (first) {
		event = first->event;
		S->first_event = first->next;
		free(first);
		if (S->first_event == NULL) S->last_event = NULL;
		sem_signal(S->sem_event);
		return event;
	}

	sem_signal(S->sem_event);
	event.type = SCI_EVT_NONE;
	event.buckybits = S->buckystate;
	return event;
}


static int
dc_usec_sleep(struct _gfx_driver *drv, long usecs)
{
	/* TODO: wake up on mouse move */
	int ms = usecs/1000;
	if (ms)
		thd_sleep(ms);
	return GFX_OK;
}

gfx_driver_t
gfx_driver_dc = {
	"dc",
	"0.2b",
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0,
	0,
	GFX_CAPABILITY_FINE_LINES,
	0,
	dc_set_parameter,
	dc_init_specific,
	dc_init,
	dc_exit,
	dc_draw_line,
	dc_draw_filled_rect,
	dc_register_pixmap,
	dc_unregister_pixmap,
	dc_draw_pixmap,
	dc_grab_pixmap,
	dc_update,
	dc_set_static_buffer,
	dc_set_pointer,
	dc_set_palette,
	dc_get_event,
	dc_usec_sleep,
	NULL
};
