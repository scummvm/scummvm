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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/events.h"
#include "common/timer.h"

#include "graphics/palette.h"

#include "cryo/cryo.h"
#include "cryo/cryolib.h"

namespace Cryo {

///// Mac APIs

void SysBeep(int x) {
}

void FlushEvents(int16 arg1, int16 arg2) {
}

///// CLView

View::View(int w, int h) {
	void *buffer = (byte *)malloc(w * h);
	if (buffer)
		initDatas(w, h, buffer);
	else
		error("Unable to allocate view buffer");
}

View::~View() {
	if (_bufferPtr)
		free(_bufferPtr);
}

// Original name: CLView_SetSrcZoomValues
void View::setSrcZoomValues(int x, int y) {
	_zoom._srcLeft = x;
	_zoom._srcTop = y;
}

// Original name: CLView_SetDisplayZoomValues
void View::setDisplayZoomValues(int w, int h) {
	_zoom._width = w;
	_zoom._height = h;
}

// Original name: CLView_InitDatas
void View::initDatas(int w, int h, void *buffer) {
	_bufferPtr = (byte *)buffer;
	_width = w;
	_height = h;
	_pitch = w;
	_normal._srcLeft = 0;
	_normal._srcTop = 0;
	_normal._dstLeft = 0;
	_normal._dstTop = 0;
	_normal._width = w;
	_normal._height = h;
	_zoom._srcLeft = 0;
	_zoom._srcTop = 0;
	_zoom._dstLeft = 0;
	_zoom._dstTop = 0;
	_zoom._width = w;
	_zoom._height = h;
}

// Original name: CLView_CenterIn
void View::centerIn(View *parent) {
	_normal._dstLeft = (parent->_width - _normal._width) / 2;
	_normal._dstTop = (parent->_height - _normal._height) / 2;
	_zoom._dstLeft = (parent->_width - _zoom._width) / 2;
	_zoom._dstTop = (parent->_height - _zoom._height) / 2;
}

///// CLPalette
uint16 gIntervalLast, gIntervalFirst, gIntervalSet;
int16 gMacintize = 0;
color_t black_palette[256];
color_t last_palette[256];

void CLPalette_Init() {
	for (int16 i = 0; i < 256; i++)
		black_palette[i].r = black_palette[i].g = black_palette[i].b = 0;
}

void CLPalette_SetLastPalette(color_t *palette, int16 first, int16 count) {
	for (int16 i = first; i < first + count; i++)
		last_palette[i] = palette[i];
}

void CLPalette_GetLastPalette(color_t *palette) {
	for (int16 i = 0; i < 256; i++)
		palette[i] = last_palette[i];
}

void CLPalette_SetRGBColor(color_t *palette, uint16 index, color3_t *rgb) {
	palette[index].r = rgb->r;
	palette[index].g = rgb->g;
	palette[index].b = rgb->b;
	palette[index].a = 0;
}

void CLPalette_Macintize(int16 macintize) {
	gMacintize = macintize;
}

void CLPalette_SetInterval(uint16 first, uint16 last) {
	gIntervalFirst = first;
	gIntervalSet = 1;
	gIntervalLast = last;
}

void CLPalette_DeactivateInterval() {
	gIntervalSet = 0;
}

void CLPalette_Send2Screen(struct color_t *palette, uint16 first, uint16 count) {
	if (gMacintize) {
		palette[0].r = palette[0].g = palette[0].b = 0xFFFF;
		palette[255].r = palette[255].g = palette[255].b = 0;
	}
	if (gIntervalSet) {
		if (first < gIntervalFirst)
			first = gIntervalFirst;
		if (first + count > gIntervalLast)
			count = gIntervalLast - first;
	}

	byte buffer[256 * 3];
	for (int i = 0; i < 256; i++) {
		buffer[i * 3] = palette[i].r >> 8;
		buffer[i * 3 + 1] = palette[i].g >> 8;
		buffer[i * 3 + 2] = palette[i].b >> 8;
	}

	g_system->getPaletteManager()->setPalette(buffer, first, count);
	g_system->updateScreen();

	CLPalette_SetLastPalette(palette, first, count);
}

void CLPalette_BeSystem() {
}

///// CLBlitter
static uint16 newPaletteCount, newPaletteFirst;
static color_t *pNewPalette;
static bool useNewPalette;

void CLBlitter_CopyViewRect(View *view1, View *view2, Common::Rect *rect1, Common::Rect *rect2) {
	int dy = rect2->top;
	int w = rect1->right - rect1->left + 1;
	//  debug("- Copy rect %3d:%3d-%3d:%3d -> %3d:%3d-%3d:%3d - %s",
	//      rect1->sx, rect1->sy, rect1->ex, rect1->ey,
	//      rect2->sx, rect2->sy, rect2->ex, rect2->ey,
	//      (rect1->ex - rect1->sx == rect2->ex - rect2->sx && rect1->ey - rect1->sy == rect2->ey - rect2->sy) ? "ok" : "BAD");
	assert(rect1->right - rect1->left == rect2->right - rect2->left && rect1->bottom - rect1->top == rect2->bottom - rect2->top);
	for (int sy = rect1->top; sy <= rect1->bottom; sy++, dy++) {
		byte *s = view1->_bufferPtr + sy * view1->_pitch + rect1->left;
		byte *d = view2->_bufferPtr + dy * view2->_pitch + rect2->left;
		for (int x = 0; x < w; x++)
			*d++ = *s++;
	}
}

void CLBlitter_Send2ScreenNextCopy(color_t *palette, uint16 first, uint16 count) {
	pNewPalette = palette;
	useNewPalette = true;
	newPaletteFirst = first;
	newPaletteCount = count;
}

void CLBlitter_OneBlackFlash() {
}

void CLBlitter_CopyView2ViewSimpleSize(byte *src, int16 srcw, int16 srcp, int16 srch,
                                       byte *dst, int16 dstw, int16 dstp, int16 dsth) {
	for (int16 y = 0; y < srch; y++) {
		for (int16 x = 0; x < srcw; x++)
			*dst++ = *src++;
		src += srcp - srcw;
		dst += dstp - dstw;
	}
}

void CLBlitter_CopyView2ScreenCUSTOM(View *view) {
	View *dest = g_ed->_screenView;
	int16 srcpitch = view->_pitch;
	int16 dstpitch = dest->_pitch;

	CLBlitter_CopyView2ViewSimpleSize(view->_bufferPtr + view->_normal._srcTop * srcpitch + view->_normal._srcLeft,
									  view->_normal._width, srcpitch, view->_normal._height,
									  dest->_bufferPtr + (dest->_normal._dstTop + view->_normal._dstTop) * dstpitch + dest->_normal._dstLeft + view->_normal._dstLeft,
									  dest->_normal._width, dstpitch, dest->_normal._height);
}

void CLBlitter_CopyView2Screen(View *view) {
	if (useNewPalette) {
		color_t palette[256];
		CLPalette_GetLastPalette(palette);
		CLPalette_Send2Screen(pNewPalette, newPaletteFirst, newPaletteCount);
		useNewPalette = false;
	}

	//HACK: Quick hack to force screen update
	if (view)
		CLBlitter_CopyView2ScreenCUSTOM(view);

	g_system->copyRectToScreen(g_ed->_screenView->_bufferPtr, g_ed->_screenView->_pitch, 0, 0, g_ed->_screenView->_width, g_ed->_screenView->_height);
	g_system->updateScreen();
}

void CLBlitter_UpdateScreen() {
	CLBlitter_CopyView2Screen(nullptr);
}

void CLBlitter_FillView(View *view, unsigned int fill) {
	byte *d = view->_bufferPtr;
	assert((fill & 0xFF) * 0x01010101 == fill);

	for (int16 y = 0; y < view->_height; y++) {
		for (int16 x = 0; x < view->_width; x++)
			*d++ = fill;
		d += view->_pitch - view->_width;
	}
}

void CLBlitter_FillScreenView(unsigned int fill) {
	CLBlitter_FillView(g_ed->_screenView, fill);
}

///// events wrapper
int _mouseButton;
byte _keyState[256];

void CryoEngine::pollEvents() {
	g_system->delayMillis(10);

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// Handle keypress
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		case Common::EVENT_KEYDOWN:
			return;
		case Common::EVENT_KEYUP:
			//          _keyState[(byte)toupper(event.kbd.ascii)] = false;
			return;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButton = 1;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButton = 2;
			return;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mouseButton = 0;
			return;
		default:
			break;
		}
	}
}

void CryoEngine::hideMouse() {
}

void CryoEngine::showMouse() {
}

void CryoEngine::getMousePosition(int16 *x, int16 *y) {
	*x = g_system->getEventManager()->getMousePos().x;
	*y = g_system->getEventManager()->getMousePos().y;
}

void CryoEngine::setMousePosition(int16 x, int16 y) {
	g_system->warpMouse(x, y);
}

bool CryoEngine::isMouseButtonDown() {
	pollEvents();
	return _mouseButton != 0;
}

///// CLTimer
void CLTimer_Action(void *arg) {
	//  long& counter = *((long*)arg);
	//  counter++;
	g_ed->_timerTicks++;
}

///// CRYOLib
void CRYOLib_ManagersInit() {
	g_system->getTimerManager()->installTimerProc(CLTimer_Action, 10000, nullptr, "100hz timer");
	g_ed->_screenView->initDatas(g_ed->_screen.w, g_ed->_screen.h, g_ed->_screen.getPixels());
}

void CRYOLib_ManagersDone() {
	g_system->getTimerManager()->removeTimerProc(CLTimer_Action);
}

PakHeaderNode::PakHeaderNode(int count) {
	_count = count;
	_files = new PakHeaderItem[count];
}

PakHeaderNode::~PakHeaderNode() {
	_count = 0;
	delete[] _files;
}

} // End of namespace Cryo
