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
 */

#include "osys_psp_gu.h"
#include "powerman.h"
#include "trace.h"
#include "common/events.h"

#include <zlib.h>

#include <pspdisplay.h>

#define PIXEL_SIZE (4)
#define BUF_WIDTH (512)
#define	PSP_SCREEN_WIDTH	480
#define	PSP_SCREEN_HEIGHT	272
#define PSP_FRAME_SIZE (BUF_WIDTH * PSP_SCREEN_HEIGHT * PIXEL_SIZE)
#define MOUSE_SIZE	128
#define	KBD_DATA_SIZE	130560

#define	MAX_FPS	30

unsigned int __attribute__((aligned(16))) list[2048];
unsigned short __attribute__((aligned(16))) clut256[256];
unsigned short __attribute__((aligned(16))) mouseClut[256];
unsigned short __attribute__((aligned(16))) cursorPalette[256];
unsigned short __attribute__((aligned(16))) kbClut[256];
unsigned int __attribute__((aligned(16))) mouseBuf256[MOUSE_SIZE*MOUSE_SIZE];

extern unsigned long RGBToColour(unsigned long r, unsigned long g, unsigned long b);

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


OSystem_PSP_GU::OSystem_PSP_GU() {
	//sceKernelDcacheWritebackAll();

	// setup
	sceGuInit();
	sceGuStart(0, list);
	sceGuDrawBuffer(GU_PSM_8888, (void *)0, BUF_WIDTH);
	sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)PSP_FRAME_SIZE, BUF_WIDTH);
	sceGuDepthBuffer((void*)(PSP_FRAME_SIZE * 2), BUF_WIDTH);
	sceGuOffset(2048 - (PSP_SCREEN_WIDTH/2), 2048 - (PSP_SCREEN_HEIGHT/2));
	sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(1);

	//decompress keyboard data
	uLongf kbdSize = KBD_DATA_SIZE;
	keyboard_letters = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_letters, &kbdSize, (const Bytef *)keyboard_letters_compressed, size_keyboard_letters_compressed))
		error("OSystem_PSP_GU: uncompressing keyboard_letters failed");

	kbdSize = KBD_DATA_SIZE;
	keyboard_letters_shift = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_letters_shift, &kbdSize, (const Bytef *)keyboard_letters_shift_compressed, size_keyboard_letters_shift_compressed))
		error("OSystem_PSP_GU: uncompressing keyboard_letters_shift failed");

	kbdSize = KBD_DATA_SIZE;
	keyboard_symbols = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_symbols, &kbdSize, (const Bytef *)keyboard_symbols_compressed, size_keyboard_symbols_compressed))
		error("OSystem_PSP_GU: uncompressing keyboard_symbols failed");

	kbdSize = KBD_DATA_SIZE;
	keyboard_symbols_shift = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	if (Z_OK != uncompress((Bytef *)keyboard_symbols_shift, &kbdSize, (const Bytef *)keyboard_symbols_shift_compressed, size_keyboard_symbols_shift_compressed))
		error("OSystem_PSP_GU: uncompressing keyboard_symbols_shift failed");

	_keyboardVisible = false;
	_clut = clut256;	// Not uncached to prevent cache coherency issues
	_kbdClut = (unsigned short*)(((unsigned int)kbClut)|0x40000000);
	_mouseBuf = (byte *)mouseBuf256;
	_graphicMode = STRETCHED_480X272;
	_keySelected = 1;
	_keyboardMode = 0;
	_mouseX = PSP_SCREEN_WIDTH >> 1;
	_mouseY = PSP_SCREEN_HEIGHT >> 1;
}

OSystem_PSP_GU::~OSystem_PSP_GU() {
	free(keyboard_symbols_shift);
	free(keyboard_symbols);
	free(keyboard_letters_shift);
	free(keyboard_letters);

	_offscreen = 0;
	_overlayBuffer = 0;
	_mouseBuf = 0;
	 sceGuTerm();
}

void OSystem_PSP_GU::initSize(uint width, uint height) {
	PSPDebugTrace("initSize\n");
	_screenWidth = width;
	_screenHeight = height;

	_overlayWidth = PSP_SCREEN_WIDTH;	//width;
	_overlayHeight = PSP_SCREEN_HEIGHT;	//height;

	_overlayBuffer = (OverlayColor *)0x44000000 + PSP_FRAME_SIZE;

	_offscreen = (byte *)_overlayBuffer + _overlayWidth * _overlayHeight * sizeof(OverlayColor);
	bzero(_offscreen, width * height);
	clearOverlay();
	memset(_palette, 0xffff, 256 * sizeof(unsigned short));
	_kbdClut[0] = 0xffff;
	_kbdClut[246] = 0x4ccc;
	_kbdClut[247] = 0x0000;
	for (int i=1;i<31;i++)
		_kbdClut[i] = 0xf888;
	_mouseVisible = false;
	sceKernelDcacheWritebackAll();
}

int OSystem_PSP_GU::getDefaultGraphicsMode() const {
	return STRETCHED_480X272;
}

bool OSystem_PSP_GU::setGraphicsMode(int mode) {
	_graphicMode = mode;
	return true;
}

bool OSystem_PSP_GU::setGraphicsMode(const char *name) {
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

int OSystem_PSP_GU::getGraphicsMode() const {
	return _graphicMode;
}

void OSystem_PSP_GU::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
	//TODO: handle cursorTargetScale
	_mouseWidth = w;
	_mouseHeight = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColour = keycolor;

	memcpy(mouseClut, _palette, 256*sizeof(unsigned short));
	mouseClut[_mouseKeyColour] = 0;

	for (unsigned int i=0;i<h;i++)
		memcpy(_mouseBuf+i*MOUSE_SIZE, buf+i*w, w);
		
	sceKernelDcacheWritebackAll();		
}

void OSystem_PSP_GU::setPalette(const byte *colors, uint start, uint num) {
	const byte *b = colors;

	for (uint i = 0; i < num; ++i) {
		_palette[start + i] = RGBToColour(b[0], b[1], b[2]);
		b += 4;
	}

	//copy to CLUT
	memcpy(_clut, _palette, 256*sizeof(unsigned short));

	//force update of mouse CLUT as well, as it may have been set up before this palette was set
	memcpy(mouseClut, _palette, 256*sizeof(unsigned short));
	mouseClut[_mouseKeyColour] = 0;

	sceKernelDcacheWritebackAll();
}

void OSystem_PSP_GU::setCursorPalette(const byte *colors, uint start, uint num) {
	const byte *b = colors;

	for (uint i = 0; i < num; ++i) {
		cursorPalette[start + i] = RGBToColour(b[0], b[1], b[2]);
		b += 4;
	}

	cursorPalette[0] = 0;

	_cursorPaletteDisabled = false;
	
	sceKernelDcacheWritebackAll();
}

void OSystem_PSP_GU::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;
}

void OSystem_PSP_GU::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
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

	if (_screenWidth == pitch && pitch == w)
	{
		memcpy(dst, buf, h * w);
/*
		sceGuStart(0,list);
		sceGuCopyImage( 3, 0, 0, w/2, h, w/2, (void *)buf, x/2, y, _screenWidth /2, _offscreen);
		sceGuFinish();
		sceGuSync(0,0);
*/
	}
	 else
	{
		do
		{
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _screenWidth;
		} while (--h);
	}
}

void OSystem_PSP_GU::updateScreen() {
	u32 now = getMillis();
	if (now - _lastScreenUpdate < 1000 / MAX_FPS)
		return;

	_lastScreenUpdate = now;


	sceGuStart(0,list);

	sceGuClearColor(0xff000000);
	sceGuClear(GU_COLOR_BUFFER_BIT);

	sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);
	sceGuClutLoad(32, clut256); // upload 32*8 entries (256)
	sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
	if (_screenWidth == 320)
			sceGuTexImage(0, 512, 256, _screenWidth, _offscreen);
	else
			sceGuTexImage(0, 512, 512, _screenWidth, _offscreen);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuTexOffset(0,0);
	sceGuAmbientColor(0xffffffff);
	sceGuColor(0xffffffff);

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
		sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);
		sceGuClutLoad(32, _cursorPaletteDisabled ? mouseClut : cursorPalette); // upload 32*8 entries (256)
		sceGuAlphaFunc(GU_GREATER,0,0xff);
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
		sceGuClutMode(GU_PSM_4444, 0, 0xff, 0);
		sceGuClutLoad(32, kbClut); // upload 32*8 entries (256)
		sceGuDisable(GU_ALPHA_TEST);
		sceGuEnable(GU_BLEND);
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		switch(_keyboardMode) {
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
	//sceKernelDcacheWritebackAll();

	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();

	//sceKernelDcacheWritebackAll();
}

bool OSystem_PSP_GU::pollEvent(Common::Event &event) {
	float nub_angle = -1;
	int x, y;

	// If we're polling for events, we should check for pausing the engine
	// Pausing the engine is a necessary fix for games that use the timer for music synchronization
	//      recovering many hours later causes the game to crash. We're polling without mutexes since it's not critical to
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
		return OSystem_PSP::pollEvent(event);

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
		_kbdClut[_keySelected] = 0xffff;

		if  (buttonsChanged & PSP_CTRL_CROSS) {
			event.type = (pad.Buttons & PSP_CTRL_CROSS) ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
			if (_keySelected > 26) {
				event.kbd.flags = 0;
				switch(_keySelected) {
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
				switch( _keyboardMode) {
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

