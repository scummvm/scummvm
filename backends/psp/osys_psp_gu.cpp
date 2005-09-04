/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 * Copyright (C) 2005 Joost Peters PSP Backend
 * Copyright (C) 2005 Thomas Mayer PSP Backend
 * Copyright (C) 2005 Paolo Costabel PSP Backend
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
 */

#include <zlib.h>
#include "osys_psp_gu.h"
#include "./trace.h"

#define PIXEL_SIZE (4)
#define BUF_WIDTH (512)
#define	PSP_SCREEN_WIDTH	480
#define	PSP_SCREEN_HEIGHT	272
#define PSP_FRAME_SIZE (BUF_WIDTH * PSP_SCREEN_HEIGHT * PIXEL_SIZE)
#define MOUSE_SIZE	128	
#define	KBD_DATA_SIZE	130560

unsigned int __attribute__((aligned(16))) list[262144];
unsigned short __attribute__((aligned(16))) clut256[256];
unsigned short __attribute__((aligned(16))) mouseClut[256];
unsigned short __attribute__((aligned(16))) kbClut[256];
//unsigned int __attribute__((aligned(16))) offscreen256[640*480];
//unsigned int __attribute__((aligned(16))) overlayBuffer256[640*480*2];
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
unsigned int  kbd_code[] = {SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0, SDLK_MINUS, SDLK_EQUALS, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET,
							SDLK_BACKSLASH, SDLK_SEMICOLON, SDLK_QUOTE, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_BACKQUOTE};
unsigned char kbd_ascii_cl[] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '{', '}', '|', ':', '"', '<', '>', '?', '~'};
unsigned int  kbd_code_cl[]  = {SDLK_EXCLAIM, SDLK_AT, SDLK_HASH, SDLK_DOLLAR, 37, SDLK_CARET, SDLK_AMPERSAND, SDLK_ASTERISK, SDLK_LEFTPAREN, SDLK_RIGHTPAREN, SDLK_UNDERSCORE,
								SDLK_PLUS, 123, 125, 124, SDLK_COLON, SDLK_QUOTEDBL, SDLK_LESS, SDLK_GREATER, SDLK_QUESTION, 126};
#define CAPS_LOCK	(1 << 0)
#define SYMBOLS 	(1 << 1)

extern bool g_quit;

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
	assert(Z_OK == uncompress((Bytef *)keyboard_letters, &kbdSize, (const Bytef *)keyboard_letters_compressed, size_keyboard_letters_compressed));
	
	kbdSize = KBD_DATA_SIZE;
	keyboard_letters_shift = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	assert(Z_OK == uncompress((Bytef *)keyboard_letters_shift, &kbdSize, (const Bytef *)keyboard_letters_shift_compressed, size_keyboard_letters_shift_compressed));
	
	kbdSize = KBD_DATA_SIZE;
	keyboard_symbols = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	assert(Z_OK == uncompress((Bytef *)keyboard_symbols, &kbdSize, (const Bytef *)keyboard_symbols_compressed, size_keyboard_symbols_compressed));

	kbdSize = KBD_DATA_SIZE;
	keyboard_symbols_shift = (unsigned char *)memalign(16, KBD_DATA_SIZE);
	assert(Z_OK == uncompress((Bytef *)keyboard_symbols_shift, &kbdSize, (const Bytef *)keyboard_symbols_shift_compressed, size_keyboard_symbols_shift_compressed));
	
	_keyboardVisible = false;
	_clut = (unsigned short*)(((unsigned int)clut256)|0x40000000);
	_kbdClut = (unsigned short*)(((unsigned int)kbClut)|0x40000000);
	_mouseBuf = (byte *)mouseBuf256;
	_graphicMode = STRETCHED_480X272;
	_keySelected = 1;
	_keyboardMode = 0;
	//_graphicMode = CENTERED_435X272;
	//_graphicMode = CENTERED_320X200;
	//_graphicMode = CENTERED_362X272;
}

OSystem_PSP_GU::~OSystem_PSP_GU()
{
	free(keyboard_symbols_shift);
	free(keyboard_symbols);
	free(keyboard_letters_shift);
	free(keyboard_letters);
	
	_offscreen = 0;
	_overlayBuffer = 0;
	_mouseBuf = 0;
	 sceGuTerm();
}

void OSystem_PSP_GU::initSize(uint width, uint height, int overlayScale) {
	PSPDebugTrace("initSize\n");
	_screenWidth = width;
	_screenHeight = height;
//	_offscreen = (byte *)offscreen256;
	_overlayBuffer = (OverlayColor *)0x44000000 + PSP_FRAME_SIZE;

	if(overlayScale == -1 || width == 640)
		overlayScale = 1;
	
	_overlayScale = overlayScale;
	_offscreen = (unsigned byte *)_overlayBuffer+_screenWidth*_screenHeight*_overlayScale*_overlayScale*sizeof(OverlayColor);
	bzero(_offscreen, width * height);
	bzero(_overlayBuffer, width * overlayScale * height * overlayScale);
	_kbdClut[0] = 0xffff;
	_kbdClut[246] = 0x4ccc;
	_kbdClut[247] = 0x0000;
	for(int i=1;i<31;i++)
		_kbdClut[i] = 0xf888;
	_mouseVisible = false;
	sceKernelDcacheWritebackAll();
}

int OSystem_PSP_GU::getDefaultGraphicsMode() const
{
	return STRETCHED_480X272;
}

bool OSystem_PSP_GU::setGraphicsMode(int mode)
{
	_graphicMode = mode;
	return true;
}

bool OSystem_PSP_GU::setGraphicsMode(const char *name)
{
	int i = 0;

	while(s_supportedGraphicsModes[i].name)
	{
		if(!strcmpi(s_supportedGraphicsModes[i].name, name))
		{
			_graphicMode = s_supportedGraphicsModes[i].id;
			return true;
		}
		i++;
	}

	return false;
}

int OSystem_PSP_GU::getGraphicsMode() const
{
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
	sceKernelDcacheWritebackAll();
	
	for(unsigned int i=0;i<h;i++)
		memcpy(_mouseBuf+i*MOUSE_SIZE, buf+i*w, w);
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


void OSystem_PSP_GU::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) 
{
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

void
OSystem_PSP_GU::updateScreen()
{
	float scale;

	sceGuStart(0,list);
	
	sceGuClearColor(0xff000000);
	sceGuClear(GU_COLOR_BUFFER_BIT);

	sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);
	sceGuClutLoad(32, clut256); // upload 32*8 entries (256)
	sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
	if(_screenWidth == 320)
	    	sceGuTexImage(0, 512, 256, _screenWidth, _offscreen);
	else
	    	sceGuTexImage(0, 512, 512, _screenWidth, _offscreen);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB); 
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuTexOffset(0,0);
	sceGuAmbientColor(0xffffffff);
	sceGuColor(0xffffffff);

	struct Vertex* vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
	vertices[0].u = 0.5; vertices[0].v = 0.5;
	vertices[1].u = _screenWidth-0.5; vertices[1].v = _screenHeight-0.5;
	switch(_graphicMode)
	{
		case CENTERED_320X200:
			vertices[0].x = (PSP_SCREEN_WIDTH-320)/2; vertices[0].y = (PSP_SCREEN_HEIGHT-200)/2; vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH-(PSP_SCREEN_WIDTH-320)/2; vertices[1].y = PSP_SCREEN_HEIGHT-(PSP_SCREEN_HEIGHT-200)/2; vertices[1].z = 0;
		break;
		case CENTERED_435X272:
			vertices[0].x = (PSP_SCREEN_WIDTH-435)/2; vertices[0].y = 0; vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH-(PSP_SCREEN_WIDTH-435)/2; vertices[1].y = PSP_SCREEN_HEIGHT; vertices[1].z = 0;
		break;
		case STRETCHED_480X272:
			vertices[0].x = 0; vertices[0].y = 0; vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH; vertices[1].y = PSP_SCREEN_HEIGHT; vertices[1].z = 0;
		break;
		case CENTERED_362X272:
			vertices[0].x = (PSP_SCREEN_WIDTH-362)/2; vertices[0].y = 0; vertices[0].z = 0;
			vertices[1].x = PSP_SCREEN_WIDTH-(PSP_SCREEN_WIDTH-362)/2; vertices[1].y = PSP_SCREEN_HEIGHT; vertices[1].z = 0;
		break;
	}

	if (_shakePos) {
		vertices[0].y += _shakePos;
		vertices[1].y += _shakePos;
	}

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	if(_screenWidth==640)
	{
		sceGuTexImage(0, 512, 512, _screenWidth, _offscreen+512);
		vertices[0].u = 512 + 0.5; vertices[1].v = _screenHeight  - 0.5;
		vertices[0].x += (vertices[1].x-vertices[0].x)*511/640; vertices[0].y = 0; vertices[0].z = 0;
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
			
	
	// draw overlay
	if(_overlayVisible)
	{
		vertices[0].x = 0; vertices[0].y = 0; vertices[0].z = 0;
		vertices[1].x = PSP_SCREEN_WIDTH; vertices[1].y = PSP_SCREEN_HEIGHT; vertices[1].z = 0;
		vertices[0].u = 0.5; vertices[0].v = 0.5;
		vertices[1].u = _screenWidth*_overlayScale-0.5; vertices[1].v = _screenHeight*_overlayScale-0.5;
		sceGuTexMode(GU_PSM_5551, 0, 0, 0); // 16-bit image
		sceGuAlphaFunc(GU_GREATER,0,0xff);
		sceGuEnable(GU_ALPHA_TEST);
		if(_overlayScale==2 || _screenWidth == 640)
			sceGuTexImage(0, 512, 512, _screenWidth*_overlayScale, _overlayBuffer);
		else
			sceGuTexImage(0, 512, 256, _screenWidth*_overlayScale, _overlayBuffer);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA); 
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
		// need to render twice for textures > 512
		if(_overlayScale==2 || _screenWidth == 640)
		{
			sceGuTexImage(0, 512, 512, _screenWidth*_overlayScale, _overlayBuffer+512);
			vertices[0].u = 512 + 0.5; vertices[1].v = _screenHeight * _overlayScale - 0.5;
			vertices[0].x = PSP_SCREEN_WIDTH*512/640; vertices[0].y = 0; vertices[0].z = 0;
			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
		}
			
	}
	
	// draw mouse
   	if (_mouseVisible) 
	{
		sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
		sceGuClutMode(GU_PSM_5551, 0, 0xff, 0);
		sceGuClutLoad(32, mouseClut); // upload 32*8 entries (256)
		sceGuAlphaFunc(GU_GREATER,0,0xff);
		sceGuEnable(GU_ALPHA_TEST);
		sceGuTexImage(0, MOUSE_SIZE, MOUSE_SIZE, MOUSE_SIZE, _mouseBuf);
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA); 
		
		vertices[0].u = 0.5; vertices[0].v = 0.5;
		vertices[1].u = _mouseWidth-0.5; vertices[1].v = _mouseHeight-0.5;

		//adjust cursor position
		int mX = _mouseX - _mouseHotspotX;
		int mY = _mouseY - _mouseHotspotY;
		
		if(_overlayVisible)
		{
			float scalex, scaley;

			scalex = (float)PSP_SCREEN_WIDTH/(_screenWidth*_overlayScale);
			scaley = (float)PSP_SCREEN_HEIGHT/(_screenHeight*_overlayScale);

			vertices[0].x = mX*scalex; vertices[0].y = mY*scaley; vertices[0].z = 0;
			vertices[1].x = vertices[0].x+_mouseWidth*scalex; vertices[1].y = vertices[0].y + _mouseHeight*scaley; vertices[0].z = 0;
		}
		else
		switch(_graphicMode)
		{
			case CENTERED_320X200:
				vertices[0].x = (PSP_SCREEN_WIDTH-320)/2+mX; vertices[0].y = (PSP_SCREEN_HEIGHT-200)/2+mY; vertices[0].z = 0;
				vertices[1].x = vertices[0].x+_mouseWidth; vertices[1].y = vertices[0].y + _mouseHeight; vertices[1].z = 0;
			break;
			case CENTERED_435X272:
			{
				scale = 435.0f/_screenWidth;
				vertices[0].x = (PSP_SCREEN_WIDTH-435)/2 +mX*scale; vertices[0].y = mY*scale; vertices[0].z = 0;
				vertices[1].x = vertices[0].x+_mouseWidth*scale; vertices[1].y = vertices[0].y + _mouseHeight*scale; vertices[0].z = 0;
			}
			break;
			case CENTERED_362X272:
			{
				float scalex, scaley;
				
				scalex = 362.0f/_screenWidth;
				scaley = 272.0f/_screenHeight;

				vertices[0].x = (PSP_SCREEN_WIDTH-362)/2 +mX*scalex; vertices[0].y = mY*scaley; vertices[0].z = 0;
				vertices[1].x = vertices[0].x+_mouseWidth*scalex; vertices[1].y = vertices[0].y + _mouseHeight*scaley; vertices[0].z = 0;
			}
			break;
			case STRETCHED_480X272:
			{
				float scalex, scaley;

				scalex = (float)PSP_SCREEN_WIDTH/_screenWidth;
				scaley = (float)PSP_SCREEN_HEIGHT/_screenHeight;
				
				vertices[0].x = mX*scalex; vertices[0].y = mY*scaley; vertices[0].z = 0;
				vertices[1].x = vertices[0].x+_mouseWidth*scalex; vertices[1].y = vertices[0].y + _mouseHeight*scaley; vertices[0].z = 0;
			}
			break;
		}
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}

	if(_keyboardVisible)
	{
		sceGuTexMode(GU_PSM_T8, 0, 0, 0); // 8-bit image
		sceGuClutMode(GU_PSM_4444, 0, 0xff, 0);
		sceGuClutLoad(32, kbClut); // upload 32*8 entries (256)
		sceGuDisable(GU_ALPHA_TEST);
		sceGuEnable(GU_BLEND);
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		switch(_keyboardMode)
		{
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
		
		vertices[0].u = 0.5; vertices[0].v = 0.5;
		vertices[1].u = PSP_SCREEN_WIDTH-0.5; vertices[1].v = PSP_SCREEN_HEIGHT-0.5;
		vertices[0].x = 0; vertices[0].y = 0; vertices[0].z = 0;
		vertices[1].x = PSP_SCREEN_WIDTH; vertices[1].y = PSP_SCREEN_HEIGHT; vertices[0].z = 0;
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
		sceGuDisable(GU_BLEND);
	}
	sceKernelDcacheWritebackAll();
	
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();

	//sceKernelDcacheWritebackAll();
}

bool OSystem_PSP_GU::pollEvent(Event &event) {
	float nub_angle = -1;
	int x, y;

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	sceCtrlReadBufferPositive(&pad, 1);

	uint32 buttonsChanged = pad.Buttons ^ _prevButtons;

	/* check if user exited using the Home button */
	if (g_quit) {
		g_quit = false; //set g_quit back to false or else pollEvent keeps looping..

		warning("g_quit signal caught, sending EVENT_QUIT to VM engine");
		event.type = OSystem::EVENT_QUIT;
		return true;
	}
	
	if  ((buttonsChanged & PSP_CTRL_SELECT) || (pad.Buttons & PSP_CTRL_SELECT)) 
	{
		if( !(pad.Buttons & PSP_CTRL_SELECT) )
			_keyboardVisible = !_keyboardVisible;
		_prevButtons = pad.Buttons;
		return false;
	}

	if(!_keyboardVisible)
		return OSystem_PSP::pollEvent(event);

	if ( (buttonsChanged & PSP_CTRL_RTRIGGER) && !(pad.Buttons & PSP_CTRL_RTRIGGER))
		_keyboardMode ^= CAPS_LOCK;

	if ( (buttonsChanged & PSP_CTRL_LTRIGGER) && !(pad.Buttons & PSP_CTRL_LTRIGGER))
		_keyboardMode ^= SYMBOLS;

	if ( (buttonsChanged & PSP_CTRL_LEFT) && !(pad.Buttons & PSP_CTRL_LEFT))
	{
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = SDLK_LEFT;
		_prevButtons = pad.Buttons;
		return true;
	}
	
	if ( (buttonsChanged & PSP_CTRL_RIGHT) && !(pad.Buttons & PSP_CTRL_RIGHT))
	{
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = SDLK_RIGHT;
		_prevButtons = pad.Buttons;
		return true;
	}
	
	if ( (buttonsChanged & PSP_CTRL_UP) && !(pad.Buttons & PSP_CTRL_UP))
	{
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = SDLK_UP;
		_prevButtons = pad.Buttons;
		return true;
	}
	
	if ( (buttonsChanged & PSP_CTRL_DOWN) && !(pad.Buttons & PSP_CTRL_DOWN))
	{
		event.kbd.flags = 0;
		event.kbd.ascii = 0;
		event.kbd.keycode = SDLK_DOWN;
		_prevButtons = pad.Buttons;
		return true;
	}
	
	// compute nub direction
	//
	x = pad.Lx-128;
	y = pad.Ly-128;
	_kbdClut[_keySelected] = 0xf888;
	if(x*x + y*y > 10000)
	{
		nub_angle = atan2(y, x);
		_keySelected = (int)(1+(M_PI+nub_angle)*30/(2*M_PI));
		_keySelected -= 2;
		if(_keySelected < 1)
			_keySelected += 30;
		_kbdClut[_keySelected] = 0xffff;
	
		if  (buttonsChanged & PSP_CTRL_CROSS) 
		{
			event.type = (pad.Buttons & PSP_CTRL_CROSS) ? OSystem::EVENT_KEYDOWN : OSystem::EVENT_KEYUP;
			if(_keySelected >26)
			{
				event.kbd.flags = 0;
				switch(_keySelected)
				{
					case 27:
					event.kbd.ascii = ' ';
					event.kbd.keycode = SDLK_SPACE;
					break;
					case 28:
					event.kbd.ascii = 127;
					event.kbd.keycode = SDLK_DELETE;
					break;
					case 29:
					event.kbd.ascii = 8;
					event.kbd.keycode = SDLK_BACKSPACE;
					break;
					case 30:
					event.kbd.ascii = 13;
					event.kbd.keycode = SDLK_RETURN;
					break;
				}
			}
			else
			{			
				switch( _keyboardMode)
				{
					case 0:
						event.kbd.flags = 0;
						event.kbd.ascii = 'a'+_keySelected-1;
						event.kbd.keycode = SDLK_a + _keySelected-1;
						break;
					case CAPS_LOCK:
						event.kbd.ascii = 'A'+_keySelected-1;
						event.kbd.keycode = SDLK_a + _keySelected-1;
						event.kbd.flags = KBD_SHIFT;
						break;
					case SYMBOLS:
						if(_keySelected < 21)
						{
							event.kbd.flags = 0;
							event.kbd.ascii = kbd_ascii[_keySelected-1];
							event.kbd.keycode = kbd_code[ _keySelected-1];
						}
						break;
					case (SYMBOLS|CAPS_LOCK):
						if(_keySelected < 21)
						{
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

OSystem *OSystem_PSP_create() {
	return new OSystem_PSP_GU();
}

