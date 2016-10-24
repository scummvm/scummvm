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
typedef int16 OSErr;

void SysBeep(int x) {
}

OSErr SetFPos(int16 handle, int16 mode, int32 pos) {
	return 0;
}

OSErr FSRead(int16 handle, int32 *size, void *buffer) {
	return 0;
}

void FlushEvents(int16 arg1, int16 arg2) {
}

// from mw lib???
int32 TickCount() {
	return g_system->getMillis();
}

///// CLTimer
volatile int32 TimerTicks = 0;   // incremented in realtime

///// CLView
void CLView_SetSrcZoomValues(View *view, int x, int y) {
	view->_zoom._srcLeft = x;
	view->_zoom._srcTop = y;
}
void CLView_SetDisplayZoomValues(View *view, int w, int h) {
	view->_zoom._width = w;
	view->_zoom._height = h;
}
void CLView_Free(View *view) {
	if (view->_bufferPtr && view->_allocated)
		free(view->_bufferPtr);
	if (view)
		free(view);
}
void CLView_InitDatas(View *view, int w, int h, void *buffer) {
	view->_bufferPtr = (byte *)buffer;
	view->_width = w;
	view->_height = h;
	view->_pitch = w;
	view->_doubled = false;
	view->_normal._srcLeft = 0;
	view->_normal._srcTop = 0;
	view->_normal._dstLeft = 0;
	view->_normal._dstTop = 0;
	view->_normal._width = w;
	view->_normal._height = h;
	view->_zoom._srcLeft = 0;
	view->_zoom._srcTop = 0;
	view->_zoom._dstLeft = 0;
	view->_zoom._dstTop = 0;
	view->_zoom._width = w;
	view->_zoom._height = h;
}
View *CLView_New(int w, int h) {
	View *view = (View *)malloc(sizeof(View));
	if (view) {
		void *buffer = (byte *)malloc(w * h);
		if (buffer) {
			view->_allocated = true;
			CLView_InitDatas(view, w, h, buffer);
		} else {
			view->_allocated = false;
			free(view);
			view = 0;
		}
	}
	return view;
}
void CLView_CenterIn(View *parent, View *child) {
	child->_normal._dstLeft = (parent->_width - child->_normal._width) / 2;
	child->_normal._dstTop = (parent->_height - child->_normal._height) / 2;
	child->_zoom._dstLeft = (parent->_width - child->_zoom._width) / 2;
	child->_zoom._dstTop = (parent->_height - child->_zoom._height) / 2;
}

///// CLScreenView
View ScreenView;

void CLScreenView_Init() {
	// ScreenView is the game's target screen (a pc display)
	// we use a dedicated surface for it, which at some point will be
	// presented to user by System::copyRectToScreen call
	CLView_InitDatas(&ScreenView, g_ed->_screen.w, g_ed->_screen.h, g_ed->_screen.getPixels());
}

void CLScreenView_CenterIn(View *view) {
	CLView_CenterIn(&ScreenView, view);
}

///// CLPalette
uint16 gIntervalLast, gIntervalFirst, gIntervalSet;
int16 gMacintize = 0;
color_t black_palette[256];
color_t last_palette[256];
void CLPalette_Init() {
	int16 i;
	for (i = 0; i < 256; i++)
		black_palette[i].r = black_palette[i].g = black_palette[i].b = 0;
}
void CLPalette_SetLastPalette(color_t *palette, int16 first, int16 count) {
	int16 i;
	for (i = first; i < first + count; i++)
		last_palette[i] = palette[i];
}
void CLPalette_GetLastPalette(color_t *palette) {
	int16 i;
	for (i = 0; i < 256; i++)
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
void CLPalette_BeBlack() {
	CLPalette_Send2Screen(black_palette, 0, 256);
}
void CLPalette_BeSystem() {
}

///// CLBlitter
static uint16 newPaletteCount, newPaletteFirst;
static color_t *pNewPalette;
static uint16 useNewPalette;

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
	useNewPalette = 1;
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
	if (!view->_doubled) {
		View *dest = &ScreenView;
		int16 srcpitch = view->_pitch;
		int16 dstpitch = dest->_pitch;

		//      this is not quite correct?
		//      CLBlitter_CopyView2ViewSimpleSize(view->_bufferPtr + view->_normal.src_top * srcpitch + view->_normal._srcLeft, view->_normal._width, srcpitch, view->_normal._height,
		//          dest->_bufferPtr + dest->_normal._dstTop * dstpitch + dest->_normal._dstLeft, dest->_normal._width, dstpitch, dest->_normal._height);

		CLBlitter_CopyView2ViewSimpleSize(view->_bufferPtr + view->_normal._srcTop * srcpitch + view->_normal._srcLeft, view->_normal._width, srcpitch, view->_normal._height,
		                                  dest->_bufferPtr + (dest->_normal._dstTop + view->_normal._dstTop) * dstpitch + dest->_normal._dstLeft + view->_normal._dstLeft, dest->_normal._width, dstpitch, dest->_normal._height);

	} else {
		assert(0);
	}
}
void CLBlitter_CopyView2Screen(View *view) {
	if (useNewPalette) {
		color_t palette[256];
		CLPalette_GetLastPalette(palette);
		CLPalette_Send2Screen(pNewPalette, newPaletteFirst, newPaletteCount);
		useNewPalette = 0;
	}

	//TODO: quick hack to force screen update
	if (view)
		CLBlitter_CopyView2ScreenCUSTOM(view);

	g_system->copyRectToScreen(ScreenView._bufferPtr, ScreenView._pitch, 0, 0, ScreenView._width, ScreenView._height);
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
	CLBlitter_FillView(&ScreenView, fill);
}


///// events wrapper
int _mouseButton;
byte _keyState[256];

void pollEvents() {
	g_system->delayMillis(10);

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// Handle keypress
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			return;

		case Common::EVENT_KEYDOWN:
			//          _keyState[(byte)toupper(event.kbd.ascii)] = true;
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


///// CLKeyboard
int16 CLKeyboard_HasCmdDown() {
	return 0;
}

void CLKeyboard_Read() {
	pollEvents();
}

byte CLKeyboard_GetLastASCII() {
	return 0;
}

int16 CLKeyboard_IsScanCodeDown(int16 scancode) {
	return 0;
}

///// CLMouse
void CLMouse_Hide() {
}

void CLMouse_Show() {
}

void CLMouse_GetPosition(int16 *x, int16 *y) {
	*x = g_system->getEventManager()->getMousePos().x;
	*y = g_system->getEventManager()->getMousePos().y;
}

void CLMouse_SetPosition(int16 x, int16 y) {
	g_system->warpMouse(x, y);
}

uint16 CLMouse_IsDown() {
	pollEvents();
	return _mouseButton != 0;
}

///// CLFile
void CLFile_SetFilter(int a3, int a4, int a5, int a6, int a7) {
}

void CLFile_SetFinderInfos(void *fs, int a4, int a5) {
}

void CLFile_GetFullPath(void *a3, char *a4) {
}

void CLFile_MakeStruct(int a3, int a4, const char *name, filespec_t *fs) {
	strcpy(fs->name, name);
	fs->create = 0;
}

void CLFile_Create(filespec_t *fs) {
	fs->create = 1;
}

void CLFile_Open(filespec_t *fs, int16 mode, file_t &handle) {
	handle.open(fs->name);
}

void CLFile_Close(file_t &handle) {
	handle.close();
}

void CLFile_SetPosition(file_t &handle, int16 mode, int32 pos) {
	assert(mode == 1);
	handle.seek(pos, 0);
}

void CLFile_Read(file_t &handle, void *buffer, int32 *size) {
	handle.read(buffer, *size);
}

void CLFile_Write(file_t &handle, void *buffer, int32 *size) {
	assert(0);
}

///// CLSound
// base sound
void CLSound_PrepareSample(sound_t *sound, int16 mode) {
	sound->_mode = mode;
	sound->_locked = 0;
	sound->_loopTimes = 0;
	sound->_reversed = false;
	sound->ff_32 = 0;
	sound->_volume = 255;
}

void CLSound_SetWantsDesigned(int16 designed) {
}

void CLSound_SetLength(sound_t *sound, int length) {
}

///// CLSoundChannel
/// sound output device that plays queue of sounds
soundchannel_t *CLSoundChannel_New(int arg1) {
	int16 i;
	soundchannel_t *ch = (soundchannel_t *)malloc(sizeof(*ch));
	if (!ch)
		return 0;

	ch->_volumeLeft = ch->_volumeRight = 255;
	ch->_numSounds = 0;

	for (i = 0; i < kCryoMaxChSounds; i++)
		ch->_sounds[i] = 0;

	return ch;
}

void CLSoundChannel_Free(soundchannel_t *ch) {
	free(ch);
}

void CLSoundChannel_Stop(soundchannel_t *ch) {
	//  g_ed->_mixer->stopHandle(ch->ch);
}

void CLSoundChannel_Play(soundchannel_t *ch, sound_t *sound) {
}

int16 CLSoundChannel_GetVolume(soundchannel_t *ch) {
	return (ch->_volumeLeft + ch->_volumeRight) / 2;
}

void CLSoundChannel_SetVolume(soundchannel_t *ch, int16 volume) {
	if (volume < 0 || volume > 255)
		return;
	ch->_volumeLeft = volume;
	ch->_volumeRight = volume;
}

void CLSoundChannel_SetVolumeRight(soundchannel_t *ch, int16 volume) {
	if (volume < 0 || volume > 255)
		return;
	ch->_volumeRight = volume;
}

void CLSoundChannel_SetVolumeLeft(soundchannel_t *ch, int16 volume) {
	if (volume < 0 || volume > 255)
		return;
	ch->_volumeLeft = volume;
}

///// CLTimer
void CLTimer_Action(void *arg) {
	//  long& counter = *((long*)arg);
	//  counter++;
	TimerTicks++;
}

void CLTimer_Init() {
	g_system->getTimerManager()->installTimerProc(CLTimer_Action, 10000, nullptr, "100hz timer");
}

void CLTimer_Done() {
	g_system->getTimerManager()->removeTimerProc(CLTimer_Action);
}

///// CRYOLib
void CRYOLib_InstallExitPatch() {
}

void CRYOLib_RemoveExitPatch() {
}

void CRYOLib_Init() {
}

void CRYOLib_Done() {
}

void CRYOLib_MinimalInit() {
}

void CRYOLib_ManagersInit() {
	CLTimer_Init();
	CLScreenView_Init();
}

void CRYOLib_ManagersDone() {
	CLTimer_Done();
}

void CRYOLib_SetDebugMode(int16 enable) {
}

void CRYOLib_InstallEmergencyExit(void(*proc)()) {
}

void CRYOLib_SetupEnvironment() {
}

void CRYOLib_RestoreEnvironment() {
}

void CRYOLib_TestConfig() {
}

///// CLComputer
int16 CLComputer_Has68030() {
	return 0;
}
int16 CLComputer_Has68040() {
	return 0;
}

///// CLDesktop
void CLDesktop_TestOpenFileAtStartup() {
}

} // End of namespace Cryo
