/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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

#include "Gs2dScreen.h"
#include <kernel.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <fileio.h>
#include <math.h> // for sqrt()
#include "DmaPipe.h"
#include "GsDefs.h"
#include <sio.h>

enum Buffers {
	SCREEN = 0,
	MOUSE,
	TEXT
};

#define DEFAULT_PAL_X		175
#define DEFAULT_PAL_Y		40
#define DEFAULT_NTSC_X		165
#define DEFAULT_NTSC_Y		45
#define ORG_X 256
#define ORG_Y 256
#define ORIGIN_X (ORG_X << 4)
#define ORIGIN_Y (ORG_Y << 4)
#define TEX_POW 10

#define SCALE(x) ((x) << 4)
#define COORD_XY(x, y) SCALE((x) + ORG_X), SCALE((y) + ORG_Y)
#define COORD_UV(u, v) SCALE(u), SCALE(v)

/*#define COORD_X1(x)		SCALE((x) + ORG_X + GS_RECT_OFFSET)
#define COORD_Y1(y)		SCALE((y) + ORG_Y + GS_RECT_OFFSET)
#define COORD_X2(x)		SCALE((x) + ORG_X - GS_RECT_OFFSET)
#define COORD_Y2(y)		SCALE((y) + ORG_Y - GS_RECT_OFFSET)
#define COORD_XY1(x, y) COORD_X1(x), COORD_Y1(y)
#define COORD_XY2(x, y) COORD_X2(x), COORD_Y2(y)*/

#define M_SIZE 128
#define M_POW 7

#define PI 3.1415926535897932384626433832795

static volatile uint32 g_VblankCmd, g_DmacCmd;

int32 vblankHandler(int32 cause) {
	// start of VBlank period
	if (g_VblankCmd) {			  // is there a new image waiting?
		GS_DISPFB1 = g_VblankCmd; // show it.
		g_VblankCmd = 0;
	}
	return 0;
}

int32 dmacHandler(int32 channel) {
	if (g_DmacCmd && (channel == 2)) { // GS DMA transfer finished,
		g_VblankCmd = g_DmacCmd;	   // we want to show the image
		g_DmacCmd = 0;				   // when the next vblank occurs
	}
	return 0;
}

void createAnimThread(Gs2dScreen *screen);

Gs2dScreen::Gs2dScreen(uint16 width, uint16 height, TVMode tvMode) {

	g_DmacCmd = g_VblankCmd = 0;
	AddIntcHandler(INT_VBLANK_START, vblankHandler, 0);
	AddDmacHandler(2, dmacHandler, 0);	 // 2 = 2nd dma channel = EE <-> GS

	_dmaPipe = new DmaPipe(0x2000);

	EnableIntc(INT_VBLANK_START);
    EnableDmac(2);

	_width = width;
	_height = height;
	_pitch = (width + 127) & ~127;

	_screenBuf = (uint8*)memalign(64, _width * _height);
	_overlayBuf = (uint16*)memalign(64, _width * _height * 2);
	_clut = (uint32*)memalign(64, 256 * 4);

	memset(_screenBuf, 0, _width * _height);
	for (uint32 cnt = 0; cnt < 256; cnt++)
		_clut[cnt] = GS_RGBA(0, 0, 0, 0x80);
	clearOverlay();

	if (tvMode == TV_DONT_CARE) {
		if (PAL_NTSC_FLAG == 'E')
			_videoMode = TV_NTSC;
		else
			_videoMode = TV_PAL;
	} else
		_videoMode = tvMode;

	printf("Setting up %s mode\n", (_videoMode == TV_PAL) ? "PAL" : "NTSC");

    // set screen size, 640x576 for pal, 640x448 for ntsc
	_tvWidth = 640;
	_tvHeight = ((_videoMode == TV_PAL) ? 576 : 448);
	
	uint32 tvFrameSize = _tvWidth * _tvHeight * 4;  // 32 bits per pixel

	// setup frame buffer pointers
	_frameBufPtr[0] = 0;
	_frameBufPtr[1] = tvFrameSize;
	_clutPtrs[SCREEN] = tvFrameSize * 2;
	_clutPtrs[MOUSE]  = _clutPtrs[SCREEN] + 0x1000; // the cluts in PSMCT32 take up half a memory page each
	_clutPtrs[TEXT]   = _clutPtrs[SCREEN] + 0x2000;
	_texPtrs[SCREEN]  = _clutPtrs[SCREEN] + 0x3000;
	_texPtrs[TEXT]    = 0;
	_texPtrs[MOUSE]	  = 128 * 128 * 4;			  // mouse cursor is loaded into the gaps of the frame buffer

	_showOverlay = true;
	_showMouse = false;
	_mouseScaleX = (_tvWidth << 8) / _width;
	_mouseScaleY = (_tvHeight << 8) / _height;
	setMouseXy(_width / 2, _height / 2);
	_mTraCol = 255;
	_shakePos = 0;

	// setup hardware now.
	GS_CSR = GS_SET_CSR(0, 0, 0, 0, 0, 1, 0);
	asm ("sync.p");
	GS_CSR = 0;
	GsPutIMR(0xFF00);

	uint16 dispPosX, dispPosY;

	if (_videoMode == TV_PAL) {
		SetGsCrt(GS_INTERLACED, 3, 0);
		dispPosX = DEFAULT_PAL_X;
		dispPosY = DEFAULT_PAL_Y;
	} else {
		SetGsCrt(GS_INTERLACED, 2, 0);
		dispPosX = DEFAULT_NTSC_X;
		dispPosY = DEFAULT_NTSC_Y;
	}

	asm("di");
	GS_PMODE = GS_SET_PMODE(1, 0, 1, 1, 0, 255);
	GS_BGCOLOUR = GS_RGBA(0, 0, 0, 0);
	GS_DISPLAY1 = GS_SET_DISPLAY(_tvWidth, _tvHeight, dispPosX, dispPosY);
	asm("ei");

	_curDrawBuf = 0;

	_dmaPipe->setOrigin(ORIGIN_X, ORIGIN_Y);
	_dmaPipe->setConfig(1, 0, 1);
	_dmaPipe->setScissorRect(0, 0, _tvWidth - 1, _tvHeight - 1);
	_dmaPipe->setDrawBuffer(_frameBufPtr[_curDrawBuf], _tvWidth, GS_PSMCT24, 0);

	_clutChanged = _screenChanged = _overlayChanged = true;
	updateScreen();
	
	createAnimTextures();

	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_screenSema = CreateSema(&newSema);
	
	newSema.init_count = 0;
	_timerSema = CreateSema(&newSema);

	if ((_screenSema < 0) || (_timerSema < 0)) {
		printf("Can't create semaphores.\n");
		SleepThread();
	}	

	createAnimThread(this);
}

void Gs2dScreen::createAnimTextures(void) {
	uint8 *buf = (uint8*)memalign(64, 14 * 64);
	uint32 vramDest = _texPtrs[TEXT];
	for (int i = 0; i < 16; i++) {
		uint32 *destPos = (uint32*)buf;
		for (int ch = 15; ch >= 0; ch--) {
			uint32 *src = (uint32*)(_binaryData + ((_binaryPattern[i] >> ch) & 1) * 4 * 14);
			for (int line = 0; line < 14; line++)
				destPos[line << 4] = src[line];
			destPos++;
		}
		if (!(i & 1))
			_dmaPipe->uploadTex( vramDest, 128, 0, 0,  GS_PSMT4HH, buf, 128, 14);
		else {
			_dmaPipe->uploadTex( vramDest, 128, 0, 0,  GS_PSMT4HL, buf, 128, 14);
			vramDest += 128 * 16 * 4;
		}
		_dmaPipe->flush();
		_dmaPipe->waitForDma();
	}
	_dmaPipe->uploadTex(_clutPtrs[TEXT], 64, 0, 0, GS_PSMCT32, _binaryClut, 8, 2);
	free(buf);
}

void Gs2dScreen::newScreenSize(uint16 width, uint16 height) {
	if ((width == _width) && (height == _height))
		return;

	printf("New screen size %d/%d\n", width, height);
	_dmaPipe->flush();
	_screenChanged = _overlayChanged = false;
	_width = width;
	_height = height;
	_pitch = (width + 127) & ~127;
	waitForImage(); // if there's a frame waiting to be shown, wait for vblank handler
	
	// now malloc new buffers
	free(_screenBuf);
	free(_overlayBuf);
	_screenBuf = (uint8*)memalign(64, _width * _height);
	memset(_screenBuf, 0, _width * height);
	_overlayBuf = (uint16*)memalign(64, _width * _height * 2);
	memset(_overlayBuf, 0, _width * height * 2);
	_screenChanged = _overlayChanged = true;

	_mouseScaleX = (_tvWidth << 8) / _width;
	_mouseScaleY = (_tvHeight << 8) / _height;
	setMouseXy(_width / 2, _height / 2);
	printf("done\n");
}

void Gs2dScreen::copyScreenRect(const uint8 *buf, uint16 pitch, uint16 x, uint16 y, uint16 w, uint16 h) {
	waitForDma();

	assert((x + w <= _width) && (y + h <= _height));
	uint8 *dest = _screenBuf + y * _width + x;
	for (uint16 cnt = 0; cnt < h; cnt++) {
		memcpy(dest, buf, w);
		buf += pitch;
		dest += _width;
	}
	_screenChanged = true;
}

void Gs2dScreen::setPalette(const uint32 *pal, uint8 start, uint16 num) {
	waitForDma();

	assert(start + num <= 256);
	for (uint16 cnt = 0; cnt < num; cnt++) {
		uint16 dest = start + cnt;
		dest = (dest & 0x7) | ((dest & 0x8) << 1) | ((dest & 0x10) >> 1) | (dest & 0xE0); // rearrange like the GS expects it
		_clut[dest] = pal[cnt] & 0xFFFFFF;
	}
	_clutChanged = true;
}

void Gs2dScreen::updateScreen(void) {
	/* we can't draw more than 50 images on PAL and 60 on NTSC, wait until the other buffer is shown.
	   especcially necessary for BS2 which does hundreds of updateScreen()s per second */
	waitForImage();
	
	WaitSema(_screenSema);

	if (_clutChanged) {
		_clutChanged = false;
		uint32 tmp = _clut[_mTraCol];
		_clut[_mTraCol] = GS_RGBA(0, 0, 0, 0x80); // this mouse color is transparent
		_dmaPipe->uploadTex(_clutPtrs[MOUSE], 64, 0, 0, GS_PSMCT32, _clut, 16, 16);
		_dmaPipe->flush();
		_dmaPipe->waitForDma();
		_clut[_mTraCol] = tmp;

		_dmaPipe->uploadTex(_clutPtrs[SCREEN], 64, 0, 0, GS_PSMCT32, _clut, 16, 16);
	}

	drawScreen();

	g_DmacCmd = GS_SET_DISPFB(_frameBufPtr[_curDrawBuf], _tvWidth, GS_PSMCT24); // put it here for dmac/vblank handler
	_dmaPipe->flush();
	_curDrawBuf ^= 1;
	_dmaPipe->setDrawBuffer(_frameBufPtr[_curDrawBuf], _tvWidth, GS_PSMCT24, 0);
	SignalSema(_screenSema);
}

void Gs2dScreen::drawScreen(void) {
	static GsVertex fullScreen[2] = {
		{ COORD_XY(0, 0), SCALE(0) },
		{ COORD_XY(_tvWidth, _tvHeight), SCALE(0) }
	};

	_dmaPipe->flatRect(fullScreen + 0, fullScreen + 1, GS_RGBA(0, 0, 0, 0)); // clear screen

	if (_showOverlay) {
		if (_overlayChanged) {
			_dmaPipe->uploadTex(_texPtrs[SCREEN], _width, 0, 0, GS_PSMCT16, _overlayBuf, _width, _height);
			_overlayChanged = false;
		}
		_dmaPipe->setTex(_texPtrs[SCREEN], _width, TEX_POW, TEX_POW, GS_PSMCT16, 0, 0, 0, 0);
		_dmaPipe->textureRect(COORD_XY(0, 0), COORD_UV(1, 1), 
			COORD_XY(_tvWidth, _tvHeight), COORD_UV(_width, _height), 0, GS_RGBA(0x80, 0x80, 0x80, 0x80));
	} else {
		if (_screenChanged) {
			_dmaPipe->uploadTex(_texPtrs[SCREEN], _pitch, 0, 0, GS_PSMT8, _screenBuf, _width, _height);
			_screenChanged = false;
		}
		_dmaPipe->setTex(_texPtrs[SCREEN], _pitch, TEX_POW, TEX_POW, GS_PSMT8, _clutPtrs[SCREEN], 0, 64, GS_PSMCT32);
		_dmaPipe->textureRect(COORD_XY(0, -_shakePos), COORD_UV(1, 1), 
			COORD_XY(_tvWidth, _tvHeight - _shakePos), COORD_UV(_width, _height), 0, GS_RGBA(0x80, 0x80, 0x80, 0x80));
	}

	if (_showMouse) {
		_dmaPipe->setTex(_texPtrs[MOUSE], M_SIZE, M_POW, M_POW, GS_PSMT8H, _clutPtrs[MOUSE], 0, 64, GS_PSMCT32);
		uint16 mpX1 = (((_mouseX - _hotSpotX) * _mouseScaleX + 128) >> 4) + ORIGIN_X;
		uint16 mpY1 = (((_mouseY - _hotSpotY) * _mouseScaleY + 128) >> 4) + ORIGIN_Y;
		uint16 mpX2 = mpX1 + ((M_SIZE * _mouseScaleX + 128) >> 4);
		uint16 mpY2 = mpY1 + ((M_SIZE * _mouseScaleY + 128) >> 4);
		_dmaPipe->textureRect(mpX1, mpY1, COORD_UV(0, 0), 
			mpX2, mpY2, COORD_UV(M_SIZE - 1, M_SIZE - 1), 0, GS_RGBA(0x80, 0x80, 0x80, 0x80));
	}
}

void Gs2dScreen::showOverlay(void) {
	_showOverlay = true;
	clearOverlay();
}

void Gs2dScreen::hideOverlay(void) {
	_screenChanged = true;
	_showOverlay = false;
}

void Gs2dScreen::setShakePos(int shake) {
	_shakePos = (shake * _mouseScaleY) >> 8;
}

void Gs2dScreen::copyOverlayRect(const uint16 *buf, uint16 pitch, uint16 x, uint16 y, uint16 w, uint16 h) {
	waitForDma();

	_overlayChanged = true;
	uint16 *dest = _overlayBuf + y * _width + x;
	for (uint32 cnt = 0; cnt < h; cnt++) {
		memcpy(dest, buf, w * 2);
		dest += _width;
		buf += pitch;
	}
}

void Gs2dScreen::clearOverlay(void) {
	waitForDma();
	
	_overlayChanged = true;
	// first convert our clut to 16 bit RGBA for the overlay...
	uint16 palette[256];
	for (uint32 cnt = 0; cnt < 256; cnt++) {
		uint32 rgba = _clut[(cnt & 0x7) | ((cnt & 0x8) << 1) | ((cnt & 0x10) >> 1) | (cnt & 0xE0)];
		palette[cnt] = ((rgba >> 3) & 0x1F) | (((rgba >> 11) & 0x1F) << 5) | (((rgba >> 19) & 0x1F) << 10);
	}
	// now copy the current screen over
	for (uint32 cnt = 0; cnt < _width * _height; cnt++)
		_overlayBuf[cnt] = palette[_screenBuf[cnt]];
}

void Gs2dScreen::grabOverlay(uint16 *buf, uint16 pitch) {
	uint16 *src = _overlayBuf;
	for (uint32 cnt = 0; cnt < _height; cnt++) {
		memcpy(buf, src, _width * 2);
		buf += pitch;
        src += _width;
	}	
}

void Gs2dScreen::setMouseOverlay(const uint8 *buf, uint16 width, uint16 height, uint16 hotSpotX, uint16 hotSpotY, uint8 transpCol) {
	assert((width <= M_SIZE) && (height <= M_SIZE));

	_hotSpotX = hotSpotX;
	_hotSpotY = hotSpotY;
	if (_mTraCol != transpCol) {
		_mTraCol = transpCol;
		_clutChanged = true;
	}
	uint8 *bufCopy = (uint8*)memalign(64, M_SIZE * M_SIZE); // make a copy to align to 64 bytes
	memset(bufCopy, _mTraCol, M_SIZE * M_SIZE);
	for (int cnt = 0; cnt < height; cnt++)
		memcpy(bufCopy + cnt * M_SIZE, buf + cnt * width, width);

	_dmaPipe->uploadTex( _texPtrs[MOUSE], M_SIZE, 0, 0,  GS_PSMT8H, bufCopy, M_SIZE, M_SIZE);
	_dmaPipe->flush();
	_dmaPipe->waitForDma(); // make sure all data has been transferred when we free bufCopy
	free(bufCopy);
}

void Gs2dScreen::waitForDma(void) {
	// wait until dma transfer finished
	while (g_DmacCmd)
		;
}

void Gs2dScreen::waitForImage(void) {
	/* if there's an image waiting to be shown on next vblank, wait for it.
	   however, first we must wait for the dma transfer (if running) as it will
	   result in a new image */
	waitForDma();
	while (g_VblankCmd)
		;
	/* both waitForImage and waitForDma should only get called by the main thread,
	   so they may be implemented as busy waits, as both the timer- and sound-thread have
	   a higher priority than this one. There's no thread we could switch to anyways... */
}

void Gs2dScreen::showMouse(bool show) {
	_showMouse = show;
}

void Gs2dScreen::setMouseXy(int16 x, int16 y) {
	_mouseX = x;
	_mouseY = y;
}

uint8 Gs2dScreen::tvMode(void) {
	return _videoMode;
}

#define LINE_SPACE 20
#define SCRL_TIME 8
#define V 1000
#define Z_TRANSL 65

void Gs2dScreen::wantAnim(bool runIt) {
	_runAnim = runIt;
}

void Gs2dScreen::animThread(void) {
	// animate zeros and ones while game accesses memory card, etc.
	_runAnim = false;
	float yPos   = 0.0;
	uint8 texSta = 0;
	float scrlSpeed = (_videoMode == TV_PAL) ? (_tvHeight / (SCRL_TIME * 50.0)) : (_tvHeight / (SCRL_TIME * 60.0));
	uint8 texMax = (_tvHeight / LINE_SPACE) + (ORG_Y / LINE_SPACE);
	TexVertex texNodes[4] = {
		{ COORD_UV(  0,  0) }, { COORD_UV(  0, 14) },
		{ COORD_UV(128,  0) }, { COORD_UV(128, 14) }
	};
	float angleStep = ((2 * PI) / _tvHeight);

	while (1) {
		WaitSema(_timerSema);
		if (_runAnim && !g_DmacCmd && !g_VblankCmd) {
			if (PollSema(_screenSema) > 0) {
				drawScreen(); // draw the last screen the engine did again

				_dmaPipe->setAlphaBlend(SOURCE_COLOR, ZERO_COLOR, SOURCE_ALPHA, DEST_COLOR, 0);
				yPos -= scrlSpeed;
				if (yPos <= -LINE_SPACE) {
					yPos += LINE_SPACE;
					texSta++;
				}

				float drawY = yPos;

				for (int i = 0; i < texMax; i++) {
					uint8 texIdx = (texSta + i) & 0xF;

					float x[4] = { -64.0, -64.0, 64.0, 64.0 };
					float y[4];
					y[0] = y[2] = drawY - _tvHeight / 2 - LINE_SPACE / 2;
					y[1] = y[3] = y[0] + LINE_SPACE;
					float z[4] = { 0.0, 0.0, 0.0, 0.0 };
					GsVertex nodes[4];

					float angle = PI / 2 + angleStep * drawY;
					float rotSin = sinf(angle);
					float rotCos = cosf(angle);
					for (int coord = 0; coord < 4; coord++) {
						z[coord] = rotCos * x[coord];
						x[coord] = rotSin * x[coord];
					
						nodes[coord].z = 0;
						nodes[coord].x = (uint16)(((V * x[coord]) / (z[coord] + V + Z_TRANSL)) * 16);
						nodes[coord].y = (uint16)(((V * y[coord]) / (z[coord] + V + Z_TRANSL)) * 16);
						nodes[coord].x += SCALE(_tvWidth - 80 + ORG_X);
						nodes[coord].y += SCALE(_tvHeight / 2 + ORG_Y);
					}

					uint32 texPtr = _texPtrs[TEXT] + 128 * 16 * 4 * (texIdx >> 1);
					if (texIdx & 1)
						_dmaPipe->setTex(_texPtrs[TEXT], 128, 7, 4, GS_PSMT4HL, _clutPtrs[TEXT], 0, 64, GS_PSMCT32);
					else
						_dmaPipe->setTex(_texPtrs[TEXT], 128, 7, 4, GS_PSMT4HH, _clutPtrs[TEXT], 0, 64, GS_PSMCT32);
					
					_dmaPipe->textureRect(nodes + 0, nodes + 1, nodes + 2, nodes + 3,
						texNodes + 0, texNodes + 1, texNodes + 2, texNodes + 3, GS_RGBA(0x80, 0x80, 0x80, 0x80));

					drawY += LINE_SPACE;
				}
				g_DmacCmd = GS_SET_DISPFB(_frameBufPtr[_curDrawBuf], _tvWidth, GS_PSMCT24); // put it here for dmac/vblank handler
				_dmaPipe->flush();
				_curDrawBuf ^= 1;
				_dmaPipe->setDrawBuffer(_frameBufPtr[_curDrawBuf], _tvWidth, GS_PSMCT24, 0);
				_dmaPipe->setAlphaBlend(DEST_COLOR, ZERO_COLOR, SOURCE_ALPHA, SOURCE_COLOR, 0);
				SignalSema(_screenSema);
			}
		}
	}
}

void Gs2dScreen::timerTick(void) {
	if (_runAnim)
		SignalSema(_timerSema);
}

void runAnimThread(Gs2dScreen *param) {
	param->animThread();
}

#define ANIM_STACK_SIZE (1024 * 32)

void createAnimThread(Gs2dScreen *screen) {
	ee_thread_t animThread, thisThread;
	ReferThreadStatus(GetThreadId(), &thisThread);

	animThread.initial_priority = thisThread.current_priority - 1;
	animThread.stack = malloc(ANIM_STACK_SIZE);
	animThread.stack_size = ANIM_STACK_SIZE;
	animThread.func = (void *)runAnimThread;
	//animThread.gp_reg = _gp;		for some reason _gp is always NULL
	asm("move %0, $gp\n": "=r"(animThread.gp_reg));

	int tid = CreateThread(&animThread);
	if (tid >= 0) {
		StartThread(tid, screen);
	} else
		free(animThread.stack);
}

// data for the animated zeros and ones...
const uint8 Gs2dScreen::_binaryData[4 * 14 * 2] = {
	// figure zero
	0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x22, 0x22, 0x00, 0x31, 0x13,
	0x31, 0x13, 0x20, 0x02, 0x22, 0x02, 0x31, 0x13, 0x33, 0x13, 0x20, 0x02, 0x20, 0x02,
	0x31, 0x33, 0x31, 0x13, 0x20, 0x22, 0x20, 0x02, 0x31, 0x13, 0x31, 0x13, 0x00, 0x22,
	0x22, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
	// figure one
	0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x20, 0x02, 0x00, 0x11, 0x33,
	0x13, 0x11, 0x22, 0x22, 0x02, 0x00, 0x11, 0x31, 0x13, 0x11, 0x00, 0x20, 0x02, 0x00,
	0x11, 0x31, 0x13, 0x11, 0x00, 0x20, 0x02, 0x00, 0x11, 0x31, 0x13, 0x11, 0x00, 0x20,
	0x02, 0x00, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11
};

const uint16 Gs2dScreen::_binaryPattern[16] = {
	0xD992, 0x344B, 0xA592, 0x110D,
	0x9234, 0x2326, 0x5199, 0xC8A6,
	0x4D29, 0x18B0, 0xA5AA, 0x2949,
	0x6DB3, 0xB2AA, 0x64A4, 0x3329 
};

const uint32 Gs2dScreen::_binaryClut[16] __attribute__((aligned(64))) = {
	GS_RGBA(   0,    0,    0, 0x40),
	GS_RGBA(  50,   50,   50, 0x40),
	GS_RGBA( 204,  204, 0xFF, 0x40),
	GS_RGBA( 140,  140, 0xFF, 0x40),

	GS_RGBA(0xFF, 0xFF, 0xFF, 0x80), GS_RGBA(0xFF, 0xFF, 0xFF, 0x80),
	GS_RGBA(0xFF, 0xFF, 0xFF, 0x80), GS_RGBA(0xFF, 0xFF, 0xFF, 0x80),
	GS_RGBA(0xFF, 0xFF, 0xFF, 0x80), GS_RGBA(0xFF, 0xFF, 0xFF, 0x80),
	GS_RGBA(0xFF, 0xFF, 0xFF, 0x80), GS_RGBA(0xFF, 0xFF, 0xFF, 0x80),
	GS_RGBA(0xFF, 0xFF, 0xFF, 0x80), GS_RGBA(0xFF, 0xFF, 0xFF, 0x80),
	GS_RGBA(0xFF, 0xFF, 0xFF, 0x80), GS_RGBA(0xFF, 0xFF, 0xFF, 0x80)
};


