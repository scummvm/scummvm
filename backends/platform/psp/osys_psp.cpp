/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include <pspgu.h>
#include <pspdisplay.h>

#include <time.h>
#include <zlib.h>

#include "common/config-manager.h"
#include "common/events.h"
#include "common/scummsys.h"

#include "osys_psp.h"
#include "trace.h"
#include "powerman.h"

#include "backends/saves/psp/psp-saves.h"
#include "backends/timer/default/default-timer.h"
#include "graphics/surface.h"
#include "sound/mixer_intern.h"


#define	SAMPLES_PER_SEC	44100

#define PIXEL_SIZE (4)
#define BUF_WIDTH (512)
#define	PSP_SCREEN_WIDTH	480
#define	PSP_SCREEN_HEIGHT	272
#define PSP_FRAME_SIZE (BUF_WIDTH * PSP_SCREEN_HEIGHT * PIXEL_SIZE)
#define MOUSE_SIZE	128
#define	KBD_DATA_SIZE	130560

#define	MAX_FPS	30

unsigned int __attribute__((aligned(16))) displayList[2048];
unsigned short __attribute__((aligned(16))) clut256[256];
unsigned short __attribute__((aligned(16))) mouseClut[256];
unsigned short __attribute__((aligned(16))) cursorPalette[256];
unsigned short __attribute__((aligned(16))) kbClut[256];
unsigned int __attribute__((aligned(16))) mouseBuf256[MOUSE_SIZE*MOUSE_SIZE];

extern unsigned int  size_keyboard_symbols_compressed;
extern unsigned char keyboard_symbols_compressed[];
extern unsigned int  size_keyboard_symbols_shift_compressed;
extern unsigned char keyboard_symbols_shift_compressed[];
extern unsigned int  size_keyboard_letters_compressed;
extern unsigned char keyboard_letters_compressed[];
extern unsigned int  size_keyboard_letters_shift_compressed;
extern unsigned char keyboard_letters_shift_compressed[];
unsigned char *keyboard_symbols;
unsigned char *keyboard_symbols_shift;
unsigned char *keyboard_letters;
unsigned char *keyboard_letters_shift;

unsigned char kbd_ascii[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '[', ']', '\\', ';', '\'', ',', '.', '/', '`'};
Common::KeyCode  kbd_code[] = {Common::KEYCODE_1, Common::KEYCODE_2, Common::KEYCODE_3, Common::KEYCODE_4, Common::KEYCODE_5, Common::KEYCODE_6, Common::KEYCODE_7, Common::KEYCODE_8, Common::KEYCODE_9, Common::KEYCODE_0, Common::KEYCODE_MINUS, Common::KEYCODE_EQUALS, Common::KEYCODE_LEFTBRACKET, Common::KEYCODE_RIGHTBRACKET,
							Common::KEYCODE_BACKSLASH, Common::KEYCODE_SEMICOLON, Common::KEYCODE_QUOTE, Common::KEYCODE_COMMA, Common::KEYCODE_PERIOD, Common::KEYCODE_SLASH, Common::KEYCODE_BACKQUOTE};
unsigned char kbd_ascii_cl[] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '{', '}', '|', ':', '"', '<', '>', '?', '~'};
Common::KeyCode  kbd_code_cl[]  = {Common::KEYCODE_EXCLAIM, Common::KEYCODE_AT, Common::KEYCODE_HASH, Common::KEYCODE_DOLLAR, (Common::KeyCode)37, Common::KEYCODE_CARET, Common::KEYCODE_AMPERSAND, Common::KEYCODE_ASTERISK, Common::KEYCODE_LEFTPAREN, Common::KEYCODE_RIGHTPAREN, Common::KEYCODE_UNDERSCORE,
								Common::KEYCODE_PLUS, (Common::KeyCode)123, (Common::KeyCode)125, (Common::KeyCode)124, Common::KEYCODE_COLON, Common::KEYCODE_QUOTEDBL, Common::KEYCODE_LESS, Common::KEYCODE_GREATER, Common::KEYCODE_QUESTION, (Common::KeyCode)126};
#define CAPS_LOCK	(1 << 0)
#define SYMBOLS		(1 << 1)




unsigned long RGBToColour(unsigned long r, unsigned long g, unsigned long b) {
	return (((b >> 3) << 10) | ((g >> 3) << 5) | ((r >> 3) << 0)) | 0x8000;
}

static int timer_handler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

const OSystem::GraphicsMode OSystem_PSP::s_supportedGraphicsModes[] = {
	{ "320x200 (centered)", "320x200 16-bit centered", CENTERED_320X200 },
	{ "435x272 (best-fit, centered)", "435x272 16-bit centered", CENTERED_435X272 },
	{ "480x272 (full screen)", "480x272 16-bit stretched", STRETCHED_480X272 },
	{ "362x272 (4:3, centered)", "362x272 16-bit centered", CENTERED_362X272 },
	{0, 0, 0}
};


OSystem_PSP::OSystem_PSP() : _screenWidth(0), _screenHeight(0), _overlayWidth(0), _overlayHeight(0),
	_offscreen(0), _overlayBuffer(0), _overlayVisible(false), _shakePos(0), _lastScreenUpdate(0), _mouseBuf(0), _prevButtons(0), _lastPadCheck(0), _padAccel(0), _mixer(0) {

	memset(_palette, 0, sizeof(_palette));

	_cursorPaletteDisabled = true;

	_samplesPerSec = 0;

	//init SDL
	uint32	sdlFlags = SDL_INIT_AUDIO | SDL_INIT_TIMER;
	SDL_Init(sdlFlags);


	//decompress keyboard data
	uLongf kbdSize = KBD_DATA_SIZE;
	keyboard_letters = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_letters, &kbdSize, (const Bytef *)keyboard_letters_compressed, size_keyboard_letters_compressed))
		error("OSystem_PSP: uncompressing keyboard_letters failed");

	kbdSize = KBD_DATA_SIZE;
	keyboard_letters_shift = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_letters_shift, &kbdSize, (const Bytef *)keyboard_letters_shift_compressed, size_keyboard_letters_shift_compressed))
		error("OSystem_PSP: uncompressing keyboard_letters_shift failed");

	kbdSize = KBD_DATA_SIZE;
	keyboard_symbols = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_symbols, &kbdSize, (const Bytef *)keyboard_symbols_compressed, size_keyboard_symbols_compressed))
		error("OSystem_PSP: uncompressing keyboard_symbols failed");

	kbdSize = KBD_DATA_SIZE;
	keyboard_symbols_shift = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_symbols_shift, &kbdSize, (const Bytef *)keyboard_symbols_shift_compressed, size_keyboard_symbols_shift_compressed))
		error("OSystem_PSP: uncompressing keyboard_symbols_shift failed");

	_keyboardVisible = false;
	_clut = clut256;	// Mustn't use uncached as it'll cause cache coherency issues
	_kbdClut = (unsigned short *)(((unsigned int)kbClut) | 0x40000000);
	_mouseBuf = (byte *)mouseBuf256;
	_graphicMode = STRETCHED_480X272;
	_keySelected = 1;
	_keyboardMode = 0;
	_mouseX = PSP_SCREEN_WIDTH >> 1;
	_mouseY = PSP_SCREEN_HEIGHT >> 1;



	// Init GU
	sceGuInit();
	sceGuStart(0, displayList);
	sceGuDrawBuffer(GU_PSM_8888, (void *)0, BUF_WIDTH);
	sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)PSP_FRAME_SIZE, BUF_WIDTH);
	sceGuDepthBuffer((void*)(PSP_FRAME_SIZE * 2), BUF_WIDTH);
	sceGuOffset(2048 - (PSP_SCREEN_WIDTH/2), 2048 - (PSP_SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuDepthRange(0xC350, 0x2710);
	sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

}

OSystem_PSP::~OSystem_PSP() {
	free(keyboard_symbols_shift);
	free(keyboard_symbols);
	free(keyboard_letters_shift);
	free(keyboard_letters);

	free(_offscreen);
	free(_overlayBuffer);
	free(_mouseBuf);

	_offscreen = 0;
	_overlayBuffer = 0;
	_mouseBuf = 0;
	 sceGuTerm();
}


void OSystem_PSP::initBackend() {
	_savefile = new PSPSaveFileManager;

	_timer = new DefaultTimerManager();
	setTimerCallback(&timer_handler, 10);

	setupMixer();

	OSystem::initBackend();
}


bool OSystem_PSP::hasFeature(Feature f) {
	return (f == kFeatureOverlaySupportsAlpha || f == kFeatureCursorHasPalette);
}

void OSystem_PSP::setFeatureState(Feature f, bool enable) {
}

bool OSystem_PSP::getFeatureState(Feature f) {
	return false;
}

const OSystem::GraphicsMode* OSystem_PSP::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_PSP::getDefaultGraphicsMode() const {
	return STRETCHED_480X272;
}

bool OSystem_PSP::setGraphicsMode(int mode) {
	_graphicMode = mode;
	return true;
}

bool OSystem_PSP::setGraphicsMode(const char *name) {
	int i = 0;

	while (s_supportedGraphicsModes[i].name) {
		if (!strcmpi(s_supportedGraphicsModes[i].name, name)) {
			_graphicMode = s_supportedGraphicsModes[i].id;
			return true;
		}
		i++;
	}

	return false;
}

int OSystem_PSP::getGraphicsMode() const {
	return _graphicMode;
}

void OSystem_PSP::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	PSPDebugTrace("initSize\n");

	_screenWidth = width;
	_screenHeight = height;

	const int scrBufSize = _screenWidth * _screenHeight * (format ? format->bytesPerPixel : 4);

	_overlayWidth = PSP_SCREEN_WIDTH;	//width;
	_overlayHeight = PSP_SCREEN_HEIGHT;	//height;

	free(_overlayBuffer);
	_overlayBuffer = (OverlayColor *)memalign(16, _overlayWidth * _overlayHeight * sizeof(OverlayColor));

	free(_offscreen);
	_offscreen = (byte *)memalign(16, scrBufSize);
	bzero(_offscreen, scrBufSize);
	
	clearOverlay();
	memset(_palette, 0xFFFF, 256 * sizeof(unsigned short));
	_kbdClut[0] = 0xFFFF;
	_kbdClut[246] = 0x4CCC;
	_kbdClut[247] = 0x0000;

	for (int i = 1; i < 31; i++)
		_kbdClut[i] = 0xF888;

	_mouseVisible = false;
	sceKernelDcacheWritebackAll();
}

int16 OSystem_PSP::getWidth() {
	return _screenWidth;
}

int16 OSystem_PSP::getHeight() {
	return _screenHeight;
}

void OSystem_PSP::setPalette(const byte *colors, uint start, uint num) {
	const byte *b = colors;

	for (uint i = 0; i < num; ++i) {
		_palette[start + i] = RGBToColour(b[0], b[1], b[2]);
		b += 4;
	}

	//copy to CLUT
	memcpy(_clut, _palette, 256 * sizeof(unsigned short));

	//force update of mouse CLUT as well, as it may have been set up before this palette was set
	memcpy(mouseClut, _palette, 256 * sizeof(unsigned short));
	mouseClut[_mouseKeyColour] = 0;

	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::setCursorPalette(const byte *colors, uint start, uint num) {
	const byte *b = colors;

	for (uint i = 0; i < num; ++i) {
		cursorPalette[start + i] = RGBToColour(b[0], b[1], b[2]);
		b += 4;
	}

	cursorPalette[0] = 0;

	_cursorPaletteDisabled = false;

	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;
}

void OSystem_PSP::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;


	byte *dst = _offscreen + y * _screenWidth + x;

	if (_screenWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w);
	} else {
		do {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
	sceKernelDcacheWritebackAll();

}

Graphics::Surface *OSystem_PSP::lockScreen() {
	_framebuffer.pixels = _offscreen;
	_framebuffer.w = _screenWidth;
	_framebuffer.h = _screenHeight;
	_framebuffer.pitch = _screenWidth;
	_framebuffer.bytesPerPixel = 1;

	return &_framebuffer;
}

void OSystem_PSP::unlockScreen() {
	// The screen is always completely update anyway, so we don't have to force a full update here.
	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::updateScreen() {
	u32 now = getMillis();
	if (now - _lastScreenUpdate < 1000 / MAX_FPS)
		return;

	_lastScreenUpdate = now;

	sceGuStart(0, displayList);

	sceGuClearColor(0xFF000000);
	sceGuClear(GU_COLOR_BUFFER_BIT);

	sceGuClutMode(GU_PSM_5551, 0, 0xFF, 0);
	sceGuClutLoad(32, clut256); // upload 32*8 entries (256)
	sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
	if (_screenWidth == 320)
		sceGuTexImage(0, 512, 256, _screenWidth, _offscreen);
	else
		sceGuTexImage(0, 512, 512, _screenWidth, _offscreen);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuTexOffset(0,0);
	sceGuAmbientColor(0xFFFFFFFF);
	sceGuColor(0xFFFFFFFF);

	Vertex *vertices = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
	vertices[0].u = 0.5f;
	vertices[0].v = 0.5f;
	vertices[1].u = _screenWidth - 0.5f;
	vertices[1].v = _screenHeight - 0.5f;

	switch (_graphicMode) {
		case CENTERED_320X200:
			vertices[0].x = (PSP_SCREEN_WIDTH - 320) / 2; 
			vertices[0].y = (PSP_SCREEN_HEIGHT - 200) / 2;
			vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH - (PSP_SCREEN_WIDTH - 320) / 2;
			vertices[1].y = PSP_SCREEN_HEIGHT - (PSP_SCREEN_HEIGHT - 200) / 2;
			vertices[1].z = 0;
		break;
		case CENTERED_435X272:
			vertices[0].x = (PSP_SCREEN_WIDTH - 435) / 2;
			vertices[0].y = 0; vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH - (PSP_SCREEN_WIDTH - 435) / 2;
			vertices[1].y = PSP_SCREEN_HEIGHT;
			vertices[1].z = 0;
		break;
		case STRETCHED_480X272:
			vertices[0].x = 0;
			vertices[0].y = 0;
			vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH;
			vertices[1].y = PSP_SCREEN_HEIGHT;
			vertices[1].z = 0;
		break;
		case CENTERED_362X272:
			vertices[0].x = (PSP_SCREEN_WIDTH - 362) / 2;
			vertices[0].y = 0;
			vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH - (PSP_SCREEN_WIDTH - 362) / 2;
			vertices[1].y = PSP_SCREEN_HEIGHT;
			vertices[1].z = 0;
		break;
	}

	if (_shakePos) {
		vertices[0].y += _shakePos;
		vertices[1].y += _shakePos;
	}

	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
	if (_screenWidth == 640) {
		// 2nd draw
		Vertex *vertices2 = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
		sceGuTexImage(0, 512, 512, _screenWidth, _offscreen+512);
		vertices2[0].u = 512 + 0.5f;
		vertices2[0].v = vertices[0].v;
		vertices2[1].u = vertices[1].u; 
		vertices2[1].v = _screenHeight - 0.5f;
		vertices2[0].x = vertices[0].x + (vertices[1].x - vertices[0].x) * 511 / 640; 
		vertices2[0].y = 0; 
		vertices2[0].z = 0;
		vertices2[1].x = vertices[1].x; 
		vertices2[1].y = vertices[1].y; 
		vertices2[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices2);
	}


	// draw overlay
	if (_overlayVisible) {
		Vertex *vertOverlay = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
		vertOverlay[0].x = 0; 
		vertOverlay[0].y = 0; 
		vertOverlay[0].z = 0;
		vertOverlay[1].x = PSP_SCREEN_WIDTH; 
		vertOverlay[1].y = PSP_SCREEN_HEIGHT; 
		vertOverlay[1].z = 0;
		vertOverlay[0].u = 0.5f;
		vertOverlay[0].v = 0.5f;
		vertOverlay[1].u = _overlayWidth - 0.5f;
		vertOverlay[1].v = _overlayHeight - 0.5f;
		sceGuTexMode(GU_PSM_4444, 0, 0, 0); // 16-bit image
		sceGuDisable(GU_ALPHA_TEST);
		sceGuEnable(GU_BLEND);

		//sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGuBlendFunc(GU_ADD, GU_FIX, GU_ONE_MINUS_SRC_ALPHA, 0xFFFFFFFF, 0);

		if (_overlayWidth > 320)
			sceGuTexImage(0, 512, 512, _overlayWidth, _overlayBuffer);
		else
			sceGuTexImage(0, 512, 256, _overlayWidth, _overlayBuffer);

		sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertOverlay);
		// need to render twice for textures > 512
		if ( _overlayWidth > 512) {
			Vertex *vertOverlay2 = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
			sceGuTexImage(0, 512, 512, _overlayWidth, _overlayBuffer + 512);
			vertOverlay2[0].u = 512 + 0.5f;
			vertOverlay2[0].v = vertOverlay[0].v;
			vertOverlay2[1].u = vertOverlay[1].u; 
			vertOverlay2[1].v = _overlayHeight - 0.5f;
			vertOverlay2[0].x = PSP_SCREEN_WIDTH * 512 / 640; 
			vertOverlay2[0].y = 0; 
			vertOverlay2[0].z = 0;
			vertOverlay2[1].x = PSP_SCREEN_WIDTH; 
			vertOverlay2[1].y = PSP_SCREEN_HEIGHT; 
			vertOverlay2[1].z = 0;			
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertOverlay2);
		}
		sceGuDisable(GU_BLEND);
	}

	// draw mouse
	if (_mouseVisible) {
		sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
		sceGuClutMode(GU_PSM_5551, 0, 0xFF, 0);
		sceGuClutLoad(32, _cursorPaletteDisabled ? mouseClut : cursorPalette); // upload 32*8 entries (256)
		sceGuAlphaFunc(GU_GREATER, 0, 0xFF);
		sceGuEnable(GU_ALPHA_TEST);
		sceGuTexImage(0, MOUSE_SIZE, MOUSE_SIZE, MOUSE_SIZE, _mouseBuf);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);

		Vertex *vertMouse = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
		vertMouse[0].u = 0.5f;
		vertMouse[0].v = 0.5f;
		vertMouse[1].u = _mouseWidth - 0.5f;
		vertMouse[1].v = _mouseHeight - 0.5f;

		//adjust cursor position
		int mX = _mouseX - _mouseHotspotX;
		int mY = _mouseY - _mouseHotspotY;

		if (_overlayVisible) {
			float scalex, scaley;

			scalex = (float)PSP_SCREEN_WIDTH /_overlayWidth;
			scaley = (float)PSP_SCREEN_HEIGHT /_overlayHeight;

			vertMouse[0].x = mX * scalex;
			vertMouse[0].y = mY * scaley;
			vertMouse[0].z = 0;
			vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
			vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
			vertMouse[1].z = 0;
		} else
			switch (_graphicMode) {
			case CENTERED_320X200:
				vertMouse[0].x = (PSP_SCREEN_WIDTH - 320) / 2 + mX;
				vertMouse[0].y = (PSP_SCREEN_HEIGHT - 200) / 2 + mY;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight;
				vertMouse[1].z = 0;
			break;
			case CENTERED_435X272:
			{
				float scalex, scaley;

				scalex = 435.0f / _screenWidth;
				scaley = 272.0f / _screenHeight;

				vertMouse[0].x = (PSP_SCREEN_WIDTH - 435) / 2 + mX * scalex;
				vertMouse[0].y = mY * scaley;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
				vertMouse[1].z = 0;
			}
			break;
			case CENTERED_362X272:
			{
				float scalex, scaley;

				scalex = 362.0f / _screenWidth;
				scaley = 272.0f / _screenHeight;

				vertMouse[0].x = (PSP_SCREEN_WIDTH - 362) / 2 + mX * scalex;
				vertMouse[0].y = mY * scaley;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
				vertMouse[1].z = 0;
			}
			break;
			case STRETCHED_480X272:
			{
				float scalex, scaley;

				scalex = (float)PSP_SCREEN_WIDTH / _screenWidth;
				scaley = (float)PSP_SCREEN_HEIGHT / _screenHeight;

				vertMouse[0].x = mX * scalex;
				vertMouse[0].y = mY * scaley;
				vertMouse[0].z = 0;
				vertMouse[1].x = vertMouse[0].x + _mouseWidth * scalex;
				vertMouse[1].y = vertMouse[0].y + _mouseHeight * scaley;
				vertMouse[1].z = 0;
			}
			break;
		}
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertMouse);
	}

	if (_keyboardVisible) {
		sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
		sceGuClutMode(GU_PSM_4444, 0, 0xFF, 0);
		sceGuClutLoad(32, kbClut); // upload 32*8 entries (256)
		sceGuDisable(GU_ALPHA_TEST);
		sceGuEnable(GU_BLEND);
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		switch (_keyboardMode) {
		case 0:
			sceGuTexImage(0, 512, 512, 480, keyboard_letters);
			break;
		case CAPS_LOCK:
			sceGuTexImage(0, 512, 512, 480, keyboard_letters_shift);
			break;
		case SYMBOLS:
			sceGuTexImage(0, 512, 512, 480, keyboard_symbols);
			break;
		case (CAPS_LOCK | SYMBOLS):
			sceGuTexImage(0, 512, 512, 480, keyboard_symbols_shift);
			break;
		}
		sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);

		Vertex *vertKB = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));
		vertKB[0].u = 0.5f;
		vertKB[0].v = 0.5f;
		vertKB[1].u = PSP_SCREEN_WIDTH - 0.5f;
		vertKB[1].v = PSP_SCREEN_HEIGHT - 0.5f;
		vertKB[0].x = 0;
		vertKB[0].y = 0;
		vertKB[0].z = 0;
		vertKB[1].x = PSP_SCREEN_WIDTH;
		vertKB[1].y = PSP_SCREEN_HEIGHT;
		vertKB[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertKB);
		sceGuDisable(GU_BLEND);
	}

	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

void OSystem_PSP::setShakePos(int shakeOffset) {
	_shakePos = shakeOffset;
}

void OSystem_PSP::showOverlay() {
	_overlayVisible = true;
}

void OSystem_PSP::hideOverlay() {
	PSPDebugTrace("OSystem_PSP::hideOverlay called\n");
	_overlayVisible = false;
}

void OSystem_PSP::clearOverlay() {
	PSPDebugTrace("clearOverlay\n");

	bzero(_overlayBuffer, _overlayWidth * _overlayHeight * sizeof(OverlayColor));
	
	sceKernelDcacheWritebackAll();
}

void OSystem_PSP::grabOverlay(OverlayColor *buf, int pitch) {
	int h = _overlayHeight;
	OverlayColor *src = _overlayBuffer;

	do {
		memcpy(buf, src, _overlayWidth * sizeof(OverlayColor));
		src += _overlayWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_PSP::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	PSPDebugTrace("copyRectToOverlay\n");

	//Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _overlayWidth - x) {
		w = _overlayWidth - x;
	}

	if (h > _overlayHeight - y) {
		h = _overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;


	OverlayColor *dst = _overlayBuffer + (y * _overlayWidth + x);

	if (_overlayWidth == pitch && pitch == w) {
		memcpy(dst, buf, h * w * sizeof(OverlayColor));
	} else {
		do {
			memcpy(dst, buf, w * sizeof(OverlayColor));
			buf += pitch;
			dst += _overlayWidth;
		} while (--h);
	}
	sceKernelDcacheWritebackAll();
}

int16 OSystem_PSP::getOverlayWidth() {
	return _overlayWidth;
}

int16 OSystem_PSP::getOverlayHeight() {
	return _overlayHeight;
}


void OSystem_PSP::grabPalette(byte *colors, uint start, uint num) {
	uint i;
	uint16 color;

	for (i = start; i < start + num; i++) {
		color = _palette[i];
		*colors++ = ((color & 0x1F) << 3);
		*colors++ = (((color >> 5) & 0x1F) << 3);
		*colors++ = (((color >> 10) & 0x1F) << 3);
		*colors++ = (color & 0x8000 ? 255 : 0);
	}
}

bool OSystem_PSP::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;
	return last;
}

void OSystem_PSP::warpMouse(int x, int y) {
	//assert(x > 0 && x < _screenWidth);
	//assert(y > 0 && y < _screenHeight);
	_mouseX = x;
	_mouseY = y;
}

void OSystem_PSP::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	//TODO: handle cursorTargetScale
	_mouseWidth = w;
	_mouseHeight = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColour = keycolor & 0xFF;

	memcpy(mouseClut, _palette, 256 * sizeof(unsigned short));
	mouseClut[_mouseKeyColour] = 0;

	for (unsigned int i = 0; i < h; i++)
		memcpy(_mouseBuf + i * MOUSE_SIZE, buf + i * w, w);

	sceKernelDcacheWritebackAll();		
}

#define PAD_CHECK_TIME	40
#define PAD_DIR_MASK	(PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT)

bool OSystem_PSP::processInput(Common::Event &event) {
	s8 analogStepAmountX = 0;
	s8 analogStepAmountY = 0;
/*
	SceCtrlData pad;

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	sceCtrlReadBufferPositive(&pad, 1);
*/
	uint32 buttonsChanged = pad.Buttons ^ _prevButtons;

	if (buttonsChanged & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_SELECT | PSP_CTRL_SQUARE | PSP_CTRL_TRIANGLE)) {
		if (buttonsChanged & PSP_CTRL_CROSS) {
			event.type = (pad.Buttons & PSP_CTRL_CROSS) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
		} else if (buttonsChanged & PSP_CTRL_CIRCLE) {
			event.type = (pad.Buttons & PSP_CTRL_CIRCLE) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
		} else {
			//any of the other buttons.
			event.type = buttonsChanged & pad.Buttons ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
			event.kbd.ascii = 0;
			event.kbd.flags = 0;

			if (buttonsChanged & PSP_CTRL_LTRIGGER) {
				event.kbd.keycode = Common::KEYCODE_ESCAPE;
				event.kbd.ascii = 27;
			} else if (buttonsChanged & PSP_CTRL_START) {
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
				if (pad.Buttons & PSP_CTRL_RTRIGGER) {
					event.kbd.flags = Common::KBD_CTRL;	// Main menu to allow RTL
				}
/*			} else if (buttonsChanged & PSP_CTRL_SELECT) {
				event.kbd.keycode = Common::KEYCODE_0;
				event.kbd.ascii = '0';
*/			} else if (buttonsChanged & PSP_CTRL_SQUARE) {
				event.kbd.keycode = Common::KEYCODE_PERIOD;
				event.kbd.ascii = '.';
			} else if (buttonsChanged & PSP_CTRL_TRIANGLE) {
				event.kbd.keycode = Common::KEYCODE_RETURN;
				event.kbd.ascii = 13;
			} else if (pad.Buttons & PSP_CTRL_RTRIGGER) {
				event.kbd.flags |= Common::KBD_SHIFT;
			}

		}

		event.mouse.x = _mouseX;
		event.mouse.y = _mouseY;
		_prevButtons = pad.Buttons;
		return true;
	}

	uint32 time = getMillis();
	if (time - _lastPadCheck > PAD_CHECK_TIME) {
		_lastPadCheck = time;
		int16 newX = _mouseX;
		int16 newY = _mouseY;

		if (pad.Lx < 100) {
			analogStepAmountX = pad.Lx - 100;
		} else if (pad.Lx > 155) {
			analogStepAmountX = pad.Lx - 155;
		}

		if (pad.Ly < 100) {
			analogStepAmountY = pad.Ly - 100;
		} else if (pad.Ly > 155) {
			analogStepAmountY = pad.Ly - 155;
		}

		if (pad.Buttons & PAD_DIR_MASK ||
		    analogStepAmountX != 0 || analogStepAmountY != 0) {
			if (_prevButtons & PAD_DIR_MASK) {
				if (_padAccel < 16)
					_padAccel++;
			} else
				_padAccel = 0;

			_prevButtons = pad.Buttons;

			if (pad.Buttons & PSP_CTRL_LEFT)
				newX -= _padAccel >> 2;
			if (pad.Buttons & PSP_CTRL_RIGHT)
				newX += _padAccel >> 2;
			if (pad.Buttons & PSP_CTRL_UP)
				newY -= _padAccel >> 2;
			if (pad.Buttons & PSP_CTRL_DOWN)
				newY += _padAccel >> 2;

			// If no movement then this has no effect
			if (pad.Buttons & PSP_CTRL_RTRIGGER) {
				// Fine control mode for analog
					if (analogStepAmountX != 0) {
						if (analogStepAmountX > 0)
							newX += analogStepAmountX - (analogStepAmountX - 1);
						else
							newX -= -analogStepAmountX - (-analogStepAmountX - 1);
					}

					if (analogStepAmountY != 0) {
						if (analogStepAmountY > 0)
							newY += analogStepAmountY - (analogStepAmountY - 1);
						else
							newY -= -analogStepAmountY - (-analogStepAmountY - 1);
					}
			} else {
				newX += analogStepAmountX >> ((_screenWidth == 640) ? 2 : 3);
				newY += analogStepAmountY >> ((_screenWidth == 640) ? 2 : 3);
			}

			if (newX < 0)
				newX = 0;
			if (newY < 0)
				newY = 0;
			if (_overlayVisible) {
				if (newX >= _overlayWidth)
					newX = _overlayWidth - 1;
				if (newY >= _overlayHeight)
					newY = _overlayHeight - 1;
			} else {
				if (newX >= _screenWidth)
					newX = _screenWidth - 1;
				if (newY >= _screenHeight)
					newY = _screenHeight - 1;
			}

			if ((_mouseX != newX) || (_mouseY != newY)) {
				event.type = Common::EVENT_MOUSEMOVE;
				event.mouse.x = _mouseX = newX;
				event.mouse.y = _mouseY = newY;
				return true;
			}
		} else {
			//reset pad acceleration
			_padAccel = 0;
		}
	}

	return false;
}

bool OSystem_PSP::pollEvent(Common::Event &event) {
	float nub_angle = -1;
	int x, y;
	
	// If we're polling for events, we should check for pausing the engine 
	// Pausing the engine is a necessary fix for games that use the timer for music synchronization
	// 	recovering many hours later causes the game to crash. We're polling without mutexes since it's not critical to
	//  get it right now.

	PowerMan.pollPauseEngine();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	sceCtrlReadBufferPositive(&pad, 1);

	uint32 buttonsChanged = pad.Buttons ^ _prevButtons;

	if  ((buttonsChanged & PSP_CTRL_SELECT) || (pad.Buttons & PSP_CTRL_SELECT)) {
		if ( !(pad.Buttons & PSP_CTRL_SELECT) )
			_keyboardVisible = !_keyboardVisible;
		_prevButtons = pad.Buttons;
		return false;
	}

	if (!_keyboardVisible)
		return processInput(event);

	if ( (buttonsChanged & PSP_CTRL_RTRIGGER) && !(pad.Buttons & PSP_CTRL_RTRIGGER))
		_keyboardMode ^= CAPS_LOCK;

	if ( (buttonsChanged & PSP_CTRL_LTRIGGER) && !(pad.Buttons & PSP_CTRL_LTRIGGER))
		_keyboardMode ^= SYMBOLS;

	if ( (buttonsChanged & PSP_CTRL_LEFT) && !(pad.Buttons & PSP_CTRL_LEFT)) {
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = Common::KEYCODE_LEFT;
		_prevButtons = pad.Buttons;
		return true;
	}

	if ( (buttonsChanged & PSP_CTRL_RIGHT) && !(pad.Buttons & PSP_CTRL_RIGHT)) {
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = Common::KEYCODE_RIGHT;
		_prevButtons = pad.Buttons;
		return true;
	}

	if ( (buttonsChanged & PSP_CTRL_UP) && !(pad.Buttons & PSP_CTRL_UP)) {
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = Common::KEYCODE_UP;
		_prevButtons = pad.Buttons;
		return true;
	}

	if ( (buttonsChanged & PSP_CTRL_DOWN) && !(pad.Buttons & PSP_CTRL_DOWN)) {
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = Common::KEYCODE_DOWN;
		_prevButtons = pad.Buttons;
		return true;
	}

	// compute nub direction
	x = pad.Lx-128;
	y = pad.Ly-128;
	_kbdClut[_keySelected] = 0xf888;
	if (x*x + y*y > 10000) {
		nub_angle = atan2(y, x);
		_keySelected = (int)(1 + (M_PI + nub_angle) * 30 / (2 * M_PI));
		_keySelected -= 2;
		if (_keySelected < 1)
			_keySelected += 30;
		_kbdClut[_keySelected] = 0xFFFF;

		if  (buttonsChanged & PSP_CTRL_CROSS) {
			event.type = (pad.Buttons & PSP_CTRL_CROSS) ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
			if (_keySelected > 26) {
				event.kbd.flags = 0;
				switch (_keySelected) {
				case 27:
					event.kbd.ascii = ' ';
					event.kbd.keycode = Common::KEYCODE_SPACE;
				break;
				case 28:
					event.kbd.ascii = 127;
					event.kbd.keycode = Common::KEYCODE_DELETE;
				break;
				case 29:
					event.kbd.ascii = 8;
					event.kbd.keycode = Common::KEYCODE_BACKSPACE;
				break;
				case 30:
					event.kbd.ascii = 13;
					event.kbd.keycode = Common::KEYCODE_RETURN;
				break;
				}
			} else {
				switch ( _keyboardMode) {
				case 0:
					event.kbd.flags = 0;
					event.kbd.ascii = 'a'+_keySelected-1;
					event.kbd.keycode = (Common::KeyCode)(Common::KEYCODE_a + _keySelected-1);
					break;
				case CAPS_LOCK:
					event.kbd.ascii = 'A'+_keySelected-1;
					event.kbd.keycode = (Common::KeyCode)(Common::KEYCODE_a + _keySelected-1);
					event.kbd.flags = Common::KBD_SHIFT;
					break;
				case SYMBOLS:
					if (_keySelected < 21) {
						event.kbd.flags = 0;
						event.kbd.ascii = kbd_ascii[_keySelected-1];
						event.kbd.keycode = kbd_code[ _keySelected-1];
					}
					break;
				case (SYMBOLS|CAPS_LOCK):
					if (_keySelected < 21) {
						event.kbd.flags = 0;
						event.kbd.ascii = kbd_ascii_cl[_keySelected-1];
						event.kbd.keycode = kbd_code_cl[ _keySelected-1];
					}
					break;
				}
			}
			_prevButtons = pad.Buttons;
			return true;
		}
	}

	_prevButtons = pad.Buttons;
	return false;
}


uint32 OSystem_PSP::getMillis() {
	return SDL_GetTicks();
}

void OSystem_PSP::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void OSystem_PSP::setTimerCallback(TimerProc callback, int interval) {
	SDL_SetTimer(interval, (SDL_TimerCallback)callback);
}

OSystem::MutexRef OSystem_PSP::createMutex(void) {
	return (MutexRef)SDL_CreateMutex();
}

void OSystem_PSP::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *)mutex);
}

void OSystem_PSP::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *)mutex);
}

void OSystem_PSP::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *)mutex);
}

void OSystem_PSP::mixCallback(void *sys, byte *samples, int len) {
	OSystem_PSP *this_ = (OSystem_PSP *)sys;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(samples, len);
}

void OSystem_PSP::setupMixer(void) {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	memset(&desired, 0, sizeof(desired));

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");
	else
		_samplesPerSec = SAMPLES_PER_SEC;

	// Originally, we always used 2048 samples. This loop will produce the
	// same result at 22050 Hz, and should hopefully produce something
	// sensible for other frequencies. Note that it must be a power of two.

	uint16 samples = 0x8000;

	for (;;) {
		if (samples / (_samplesPerSec / 1000) < 100)
			break;
		samples >>= 1;
	}

	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = samples;
	desired.callback = mixCallback;
	desired.userdata = this;

	assert(!_mixer);
	_mixer = new Audio::MixerImpl(this);
	assert(_mixer);

	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio: %s", SDL_GetError());
		_samplesPerSec = 0;
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		_samplesPerSec = obtained.freq;

		// Tell the mixer that we are ready and start the sound processing
		_mixer->setOutputRate(_samplesPerSec);
		_mixer->setReady(true);

		SDL_PauseAudio(0);
	}
}

void OSystem_PSP::quit() {
	SDL_CloseAudio();
	SDL_Quit();
	sceGuTerm();
	sceKernelExitGame();
}

void OSystem_PSP::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
}

#define PSP_CONFIG_FILE "ms0:/scummvm.ini"

Common::SeekableReadStream *OSystem_PSP::createConfigReadStream() {
	Common::FSNode file(PSP_CONFIG_FILE);
	return file.createReadStream();
}

Common::WriteStream *OSystem_PSP::createConfigWriteStream() {
	Common::FSNode file(PSP_CONFIG_FILE);
	return file.createWriteStream();
}
