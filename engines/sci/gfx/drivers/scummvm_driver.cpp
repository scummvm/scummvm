#include "common/scummsys.h"
#include "common/system.h"
#include "common/events.h"

#include "sci/include/gfx_driver.h"
#include "sci/include/gfx_tools.h"

struct _scummvm_driver_state {
	gfx_pixmap_t *priority[2];
	byte *visual[3];
	byte *pointer_data[2];
	int xsize, ysize;
	//int buckystate;
	bool update_screen;
	bool update_mouse;
};

#define S ((struct _scummvm_driver_state *)(drv->state))

static int
scummvm_init_specific(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
{
	int i;

	if (!drv->state) // = S
		drv->state = new _scummvm_driver_state;
	if (!drv->state)
		return GFX_FATAL;

	S->xsize = xfact * 320;
	S->ysize = yfact * 200;

	S->pointer_data[0] = NULL;
	S->pointer_data[1] = NULL;
	//S->buckystate = 0;

	for (i = 0; i < 2; i++) {
		S->priority[i] = gfx_pixmap_alloc_index_data(gfx_new_pixmap(S->xsize, S->ysize, GFX_RESID_NONE, -i, -777));
		if (!S->priority[i]) {
			printf("Out of memory: Could not allocate priority maps! (%dx%d)\n",
					S->xsize, S->ysize);
			return GFX_FATAL;
		}
	}
	// create the visual buffers
	for (i = 0; i < 3; i++) {
		S->visual[i] = NULL;
		S->visual[i] = new byte[S->xsize * S->ysize];
		if (!S->visual[i]) {
			printf("Out of memory: Could not allocate visual buffers! (%dx%d)\n",
					S->xsize, S->ysize);
			return GFX_FATAL;
		}
		memset(S->visual[i], 0, S->xsize * S->ysize);
	}

	drv->mode = gfx_new_mode(xfact, yfact, bytespp,
				0, 0, 0, 0,
				0, 0, 0, 0, 256, 0);

	return GFX_OK;
}

static int
scummvm_init(struct _gfx_driver *drv)
{
	return scummvm_init_specific(drv, 1, 1, GFX_COLOR_MODE_INDEX);
}

static void
scummvm_exit(struct _gfx_driver *drv)
{
	int i;
	if (S) {
		for (i = 0; i < 2; i++) {
			gfx_free_pixmap(drv, S->priority[i]);
			S->priority[i] = NULL;
		}

		for (i = 0; i < 3; i++) {
			delete[] S->visual[i];
			S->visual[i] = NULL;
		}

		for (i = 0; i < 2; i++)
			if (S->pointer_data[i]) {
				delete[] S->pointer_data[i];
				S->pointer_data[i] = NULL;
			}

		delete S;
	}
}


// Drawing operations

/* This code shamelessly lifted from the SDL_gfxPrimitives package */
static void
lineColor2(byte *dst, int16 x1, int16 y1, int16 x2, int16 y2, uint32 color)
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
	pixx = 1;
	pixy = 320;
	pixel = ((uint8*)dst) + pixx * (int)x1 + pixy * (int)y1;
	pixx *= sx;
	pixy *= sy;
	if (dx < dy) {
		swaptmp = dx; dx = dy; dy = swaptmp;
		swaptmp = pixx; pixx = pixy; pixy = swaptmp;
	}

	/* Draw */
	x=0;
	y=0;
	for(; x < dx; x++, pixel += pixx) {
		*pixel = color;
		y += dy;
		if (y >= dx) {
			y -= dx; pixel += pixy;
		}
	}
}

static int
scummvm_draw_line(struct _gfx_driver *drv, point_t start, point_t end,
		gfx_color_t color,
		gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	uint32 scolor = color.visual.global_index;
	int xsize = S->xsize;
	int ysize = S->ysize;

	if (color.mask & GFX_MASK_VISUAL) {
		point_t nstart, nend;

		nstart.x = start.x;
		nstart.y = start.y;
		nend.x = end.x;
		nend.y = end.y;

		if (nstart.x < 0)
			nstart.x = 0;
		if (nend.x < 0)
			nstart.x = 0;
		if (nstart.y < 0)
			nstart.y = 0;
		if (nend.y < 0)
			nend.y = 0;
		if (nstart.x > xsize)
			nstart.x = xsize;
		if (nend.x >= xsize)
			nend.x = xsize -1;
		if (nstart.y > ysize)
			nstart.y = ysize;
		if (nend.y >= ysize)
			nend.y = ysize -1;

		lineColor2(S->visual[1], (int16)nstart.x, (int16)nstart.y,
			  (int16)nend.x, (int16)nend.y, scolor);

		if (color.mask & GFX_MASK_PRIORITY) {
			gfx_draw_line_pixmap_i(S->priority[0], nstart, nend,
					       color.priority);
		}
	}

	return GFX_OK;
}

static int
scummvm_draw_filled_rect(struct _gfx_driver *drv, rect_t rect,
		gfx_color_t color1, gfx_color_t color2,
		gfx_rectangle_fill_t shade_mode)
{
	if (color1.mask & GFX_MASK_VISUAL) {
		for (int i = rect.y; i < rect.y + rect.yl; i++) {
			memset(S->visual[1] + i * S->xsize + rect.x, color1.visual.global_index, rect.xl);
		}
	}

	if (color1.mask & GFX_MASK_PRIORITY)
		gfx_draw_box_pixmap_i(S->priority[0], rect, color1.priority);

	return GFX_OK;
}


// Pixmap operations

static int
scummvm_draw_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
	rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	int bufnr = (buffer == GFX_BUFFER_STATIC)? 2:1;
	int pribufnr = bufnr -1;

	if (dest.xl != src.xl || dest.yl != src.yl) {
		printf("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n",
			src.xl, src.yl, dest.xl, dest.yl);
		return GFX_ERROR;
	}

	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest,
			S->visual[bufnr], S->xsize,
			S->priority[pribufnr]->index_data,
			S->priority[pribufnr]->index_xl, 1, 0);

	return GFX_OK;
}

static int
scummvm_grab_pixmap(struct _gfx_driver *drv, rect_t src, gfx_pixmap_t *pxm,
	gfx_map_mask_t map)
{
	if (src.x < 0 || src.y < 0) {
		printf("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data) {
		printf("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}

	switch (map) {

	case GFX_MASK_VISUAL:
		pxm->xl = src.xl;
		pxm->yl = src.yl;
		for (int i = 0; i < src.yl; i++) {
			memcpy(pxm->data + i * src.xl, S->visual[1] + (i + src.y) * S->xsize + src.x, src.xl);
		}
		break;

	case GFX_MASK_PRIORITY:
		printf("FIXME: priority map grab not implemented yet!\n");
		break;

	default:
		printf("Attempt to grab pixmap from invalid map 0x%02x\n", map);
		return GFX_ERROR;
	}

	return GFX_OK;
}


// Buffer operations

static int
scummvm_update(struct _gfx_driver *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	//TODO
	int data_source = (buffer == GFX_BUFFER_BACK)? 2 : 1;
	int data_dest = data_source - 1;

	/*
	if (src.x != dest.x || src.y != dest.y) {
		printf("Updating %d (%d,%d)(%dx%d) to (%d,%d) on %d\n", buffer, src.x, src.y,
			src.xl, src.yl, dest.x, dest.y, data_dest);
	} else {
		printf("Updating %d (%d,%d)(%dx%d) to %d\n", buffer, src.x, src.y, src.xl, src.yl, data_dest);
	}
	*/

	switch (buffer) {
	case GFX_BUFFER_BACK:
		//memcpy(S->visual[data_dest], S->visual[data_source],
		//	S->xsize * S->ysize);
		for (int i = 0; i < src.yl; i++) {
			memcpy(S->visual[data_dest] + (dest.y + i) * S->xsize + dest.x,
				S->visual[data_source] + (src.y + i) * S->xsize + src.x, src.xl);
		}

		if ((src.x == dest.x) && (src.y == dest.y))
			gfx_copy_pixmap_box_i(S->priority[0], S->priority[1], src);
		break;
	case GFX_BUFFER_FRONT:
		memcpy(S->visual[data_dest], S->visual[data_source],
			S->xsize * S->ysize);

		g_system->copyRectToScreen(S->visual[data_dest] + src.x + src.y * S->xsize,
			S->xsize, dest.x, dest.y, src.xl, src.yl);
		/*
		g_system->copyRectToScreen(S->visual[data_dest],
			S->xsize, 0, 0, S->xsize, S->ysize);
		*/
		S->update_screen = true;
		break;
	default:
		GFXERROR("Invalid buffer %d in update!\n", buffer);
		return GFX_ERROR;
	}


	return GFX_OK;
}

static int
scummvm_set_static_buffer(struct _gfx_driver *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{
	memcpy(S->visual[2], pic->data, S->xsize * S->ysize);
	/*gfx_crossblit_pixmap(drv->mode, pic, 0, rect, rect,
			S->visual[2], S->xsize,
			S->priority[1]->index_data,
			S->priority[1]->index_xl, 1, 0);*/

	gfx_copy_pixmap_box_i(S->priority[1], priority, gfx_rect(0, 0, S->xsize, S->ysize));

	return GFX_OK;
}


// Mouse pointer operations

static int
scummvm_set_pointer(struct _gfx_driver *drv, gfx_pixmap_t *pointer)
{
	if (pointer == NULL) {
		g_system->showMouse(false);
	} else {
		g_system->setMouseCursor(pointer->index_data, pointer->xl, pointer->yl, pointer->xoffset, pointer->yoffset);
		g_system->showMouse(true);
	}

	// Pointer pixmap or mouse position has changed
	S->update_mouse = true;
	return GFX_OK;
}


// Palette operations

static int
scummvm_set_palette(struct _gfx_driver *drv, int index, byte red, byte green, byte blue)
{
	byte color[] = {red, green, blue, 255};
	g_system->setPalette(color, index, 1);
	return GFX_OK;
}


// Event management

static sci_event_t
scummvm_get_event(struct _gfx_driver *drv)
{
	sci_event_t input;
	input.type = SCI_EVT_NONE;

	Common::EventManager *em = g_system->getEventManager();
	Common::Event ev;

	bool found = em->pollEvent(ev);
	Common::Point p = ev.mouse;

	// Don't generate events for mouse movement
	while (found && ev.type == Common::EVENT_MOUSEMOVE) {
		found = em->pollEvent(ev);
		p = ev.mouse;
		drv->pointer_x = p.x;
		drv->pointer_y = p.y;
		S->update_mouse = true;
	}

	// Update the screen here, since it's called very often
	if (S->update_mouse)
		g_system->warpMouse(drv->pointer_x, drv->pointer_y);
	if (S->update_screen || S->update_mouse) {
		g_system->updateScreen();
		S->update_screen = false;
		S->update_mouse = false;
	}

	if (found && !ev.synthetic && ev.type != Common::EVENT_MOUSEMOVE) {
		int modifiers;
		if (ev.type == Common::EVENT_KEYDOWN)
			modifiers = ev.kbd.flags;
		else
			modifiers = em->getModifierState();

		input.buckybits =
			((modifiers & Common::KBD_ALT) ? SCI_EVM_ALT : 0) |
			((modifiers & Common::KBD_CTRL) ? SCI_EVM_CTRL : 0) |
			((modifiers & Common::KBD_SHIFT) ? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0);
		//TODO: SCI_EVM_SCRLOCK SCI_EVM_NUMLOCK SCI_EVM_CAPSLOCK SCI_EVM_INSERT       

		switch (ev.type) {
		// Keyboard events
		case Common::EVENT_KEYDOWN:
			input.data = ev.kbd.keycode;
			input.character = ev.kbd.ascii;

			if (!(input.data & 0xFF00)) {
				// Directly accept most common keys without conversion
				input.type = SCI_EVT_KEYBOARD;
				if (input.data == Common::KEYCODE_TAB) {
					// Tab
					input.type = SCI_EVT_KEYBOARD;
					input.data = SCI_K_TAB;
					if (input.buckybits & (SCI_EVM_LSHIFT | SCI_EVM_RSHIFT))
						input.character = SCI_K_SHIFT_TAB;
					else 
						input.character = SCI_K_TAB;
				}
			} else if ((input.data >= Common::KEYCODE_F1) && input.data <= Common::KEYCODE_F10) {
				// F1-F10
				input.type = SCI_EVT_KEYBOARD;
				// SCI_K_F1 == 59 << 8
				// SCI_K_SHIFT_F1 == 84 << 8
				input.data = (input.data - Common::KEYCODE_F1 + SCI_K_F1) << 8;
				if (input.buckybits & (SCI_EVM_LSHIFT | SCI_EVM_RSHIFT))
					input.character = input.data + ((SCI_K_SHIFT_F1 - SCI_K_F1) << 8);
				else 
					input.character = input.data;
			} else {
				// Special keys that need conversion
				input.type = SCI_EVT_KEYBOARD;
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_UP:
					input.data = SCI_K_UP;
					break;
				case Common::KEYCODE_DOWN:
					input.data = SCI_K_DOWN;
					break;
				case Common::KEYCODE_RIGHT:
					input.data = SCI_K_RIGHT;
					break;
				case Common::KEYCODE_LEFT:
					input.data = SCI_K_LEFT;
					break;
				case Common::KEYCODE_INSERT:
					input.data = SCI_K_INSERT;
					break;
				case Common::KEYCODE_HOME:
					input.data = SCI_K_HOME;
					break;
				case Common::KEYCODE_END:
					input.data = SCI_K_END;
					break;
				case Common::KEYCODE_PAGEUP:
					input.data = SCI_K_PGUP;
					break;
				case Common::KEYCODE_PAGEDOWN:
					input.data = SCI_K_PGDOWN;
					break;
				case Common::KEYCODE_DELETE:
					input.data = SCI_K_DELETE;
					break;
				//TODO: SCI_K_CENTER
				default:
					input.type = SCI_EVT_NONE;
					break;
				}
				input.character = input.data;
			}
			break;

		// Mouse events
		case Common::EVENT_LBUTTONDOWN:
			input.type = SCI_EVT_MOUSE_PRESS;
			input.data = 1;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_RBUTTONDOWN:
			input.type = SCI_EVT_MOUSE_PRESS;
			input.data = 2;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_LBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 1;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;
		case Common::EVENT_RBUTTONUP:
			input.type = SCI_EVT_MOUSE_RELEASE;
			input.data = 2;
			drv->pointer_x = p.x;
			drv->pointer_y = p.y;
			break;

		// Misc events
		case Common::EVENT_QUIT:
			input.type = SCI_EVT_QUIT;
			break;

		default:
			break;
		}
	}

	return input;
}

static int
scummvm_usec_sleep(struct _gfx_driver *drv, long usecs)
{
	g_system->delayMillis(usecs/1000);
	return GFX_OK;
}

gfx_driver_t
gfx_driver_scummvm = {
	"ScummVM",
	"0.1",
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0, 0,
	GFX_CAPABILITY_MOUSE_POINTER | GFX_CAPABILITY_COLOR_MOUSE_POINTER | GFX_CAPABILITY_MOUSE_SUPPORT | GFX_CAPABILITY_FINE_LINES | GFX_CAPABILITY_WINDOWED,
	0,
	NULL,
	scummvm_init_specific,
	scummvm_init,
	scummvm_exit,
	scummvm_draw_line,
	scummvm_draw_filled_rect,
	NULL,
	NULL,
	scummvm_draw_pixmap,
	scummvm_grab_pixmap,
	scummvm_update,
	scummvm_set_static_buffer,
	scummvm_set_pointer,
	scummvm_set_palette,
	scummvm_get_event,
	scummvm_usec_sleep,
	NULL
};
