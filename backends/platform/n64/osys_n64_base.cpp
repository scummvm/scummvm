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
 */

#include <romfs.h>

#include <malloc.h> // Required for memalign

#include "osys_n64.h"
#include "pakfs_save_manager.h"
#include "framfs_save_manager.h"
#include "backends/fs/n64/n64-fs-factory.h"

extern uint8 _romfs; // Defined by linker (used to calculate position of romfs image)

inline uint16 colBGR888toRGB555(byte r, byte g, byte b);

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{ "320x240 (PAL) fix overscan", "340x240 PAL",  OVERS_PAL_340X240   },
	{ "320x240 (PAL) overscan", "320x240 PAL",  NORM_PAL_320X240   },
	{ "320x240 (MPAL) fix overscan", "340x240 MPAL", OVERS_MPAL_340X240  },
	{ "320x240 (MPAL) overscan", "320x240 MPAL", NORM_MPAL_320X240  },
	{ "340x240 (NTSC) fix overscan", "340x240 NTSC", OVERS_NTSC_340X240 },
	{ "320x240 (NTSC) overscan", "320x240 NTSC", NORM_NTSC_320X240  },
	{ 0, 0, 0 }
};

OSystem_N64::OSystem_N64() {
	// Enable Mips interrupts
	set_MI_interrupt(1);

	// Initialize display: NTSC 340x240 (16 bit)
	initDisplay(NTSC_340X240_16BIT);

	// Prepare virtual text layer for debugging purposes
	initTextLayer();

	// Init PI interface
	PI_Init();

	// Screen size
	_screenWidth = 320;
	_screenHeight = 240;

	// Game screen size
	_gameHeight = 320;
	_gameWidth = 240;

	// Overlay size
	_overlayWidth = 320;
	_overlayHeight = 240;

	// Framebuffer width
	_frameBufferWidth = 340;

	// Pixels to skip
	_offscrPixels = 16;

	// Video clock
	_viClockRate = VI_NTSC_CLOCK;

	// Max FPS
	_maxFps = N64_NTSC_FPS;

	_overlayVisible = false;

	_shakeOffset = 0;

	// Allocate memory for offscreen buffers
	_offscreen_hic = (uint16*)memalign(8, _screenWidth * _screenHeight * 2);
	_offscreen_pal = (uint8*)memalign(8, _screenWidth * _screenHeight);
	_overlayBuffer = (uint16*)memalign(8, _overlayWidth * _overlayHeight * sizeof(OverlayColor));

	_cursor_pal = NULL;
	_cursor_hic = NULL;

	_cursorWidth = -1;
	_cursorHeight = -1;
	_cursorKeycolor = -1;
	_mouseHotspotX = _mouseHotspotY = -1;

	// Clean offscreen buffers
	memset(_offscreen_hic, 0, _screenWidth * _screenHeight * 2);
	memset(_offscreen_pal, 0, _screenWidth * _screenHeight);
	memset(_overlayBuffer, 0, _overlayWidth * _overlayHeight * sizeof(OverlayColor));

	// Default graphic mode
	_graphicMode = OVERS_NTSC_340X240;

	// Clear palette array
	_screenPalette = (uint16*)memalign(8, 256 * sizeof(uint16));
	memset(_screenPalette, 0, 256 * sizeof(uint16));
	memset(_cursorPalette, 0, 256 * sizeof(uint16));

	_dirtyPalette = false;
	_cursorPaletteDisabled = false;

	_audioEnabled = false;

	// Initialize ROMFS access interface
	initRomFSmanager((uint8*)(((uint32)&_romfs + (uint32)0xc00) | (uint32)0xB0000000));

	// Register vblank callback
	registerVIhandler(vblCallback);

	_mouseVisible = false;

	_mouseX = _overlayWidth  / 2;
	_mouseY = _overlayHeight / 2;
	_mouseMaxX = _overlayWidth;
	_mouseMaxY = _overlayHeight;

	_savefile = 0;
	_mixer = 0;
	_timer = 0;

	_dirtyOffscreen = false;

	_ctrlData = (controller_data_buttons*)memalign(8, sizeof(controller_data_buttons));

	_fsFactory = new N64FilesystemFactory();

}

OSystem_N64::~OSystem_N64() {
	delete _savefile;
	delete _mixer;
	delete _timer;
	delete _fsFactory;
}

void OSystem_N64::initBackend() {
	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_high_quality", false);
	ConfMan.setBool("FM_medium_quality", true);
	ConfMan.set("gui_theme", "modern"); // In case of modern theme being present, use it.

	FRAM_Init();

	if (FRAM_Detect()) { // Use FlashRAM
		initFramFS();
		_savefile = new FRAMSaveManager();
	} else { // Use PakFS
		// Init Controller Pak
		initPakFs();

		// Use the first controller pak found
		uint8 ctrl_num;
		for (ctrl_num = 0; ctrl_num < 4; ctrl_num++) {
			int8 pak_type = identifyPak(ctrl_num);
			if (pak_type == 1) {
				loadPakData(ctrl_num);
				break;
			}
		}

		_savefile = new PAKSaveManager();
	}

	_mixer = new Audio::MixerImpl(this);
	_mixer->setReady(false);

	_timer = new DefaultTimerManager();

	setTimerCallback(&timer_handler, 10);

	setupMixer();

	OSystem::initBackend();

}

bool OSystem_N64::hasFeature(Feature f) {
	return (f == kFeatureCursorHasPalette);
}

void OSystem_N64::setFeatureState(Feature f, bool enable) {
	return;
}

bool OSystem_N64::getFeatureState(Feature f) {
	return false;
}

const OSystem::GraphicsMode* OSystem_N64::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_N64::getDefaultGraphicsMode() const {
	return OVERS_NTSC_340X240;
}

bool OSystem_N64::setGraphicsMode(const char *mode) {
	int i = 0;
	while (s_supportedGraphicsModes[i].name) {
		if (!strcmpi(s_supportedGraphicsModes[i].name, mode)) {
			_graphicMode = s_supportedGraphicsModes[i].id;

			switchGraphicModeId(_graphicMode);

			return true;
		}
		i++;
	}

	return true;
}

bool OSystem_N64::setGraphicsMode(int mode) {
	_graphicMode = mode;
	switchGraphicModeId(_graphicMode);

	return true;
}

void OSystem_N64::switchGraphicModeId(int mode) {
	switch (mode) {
	case NORM_PAL_320X240:
		disableAudioPlayback();
		_viClockRate = VI_PAL_CLOCK;
		_maxFps = N64_PAL_FPS;
		initDisplay(PAL_320X240_16BIT);
		_frameBufferWidth = 320;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 0;
		_graphicMode = NORM_PAL_320X240;
		enableAudioPlayback();
		break;

	case OVERS_PAL_340X240:
		disableAudioPlayback();
		_viClockRate = VI_PAL_CLOCK;
		_maxFps = N64_PAL_FPS;
		initDisplay(PAL_340X240_16BIT);
		_frameBufferWidth = 340;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 16;
		_graphicMode = OVERS_PAL_340X240;
		enableAudioPlayback();
		break;

	case NORM_MPAL_320X240:
		disableAudioPlayback();
		_viClockRate = VI_MPAL_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(MPAL_320X240_16BIT);
		_frameBufferWidth = 320;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 0;
		_graphicMode = NORM_MPAL_320X240;
		enableAudioPlayback();
		break;

	case OVERS_MPAL_340X240:
		disableAudioPlayback();
		_viClockRate = VI_MPAL_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(MPAL_340X240_16BIT);
		_frameBufferWidth = 340;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 16;
		_graphicMode = OVERS_MPAL_340X240;
		enableAudioPlayback();
		break;

	case NORM_NTSC_320X240:
		disableAudioPlayback();
		_viClockRate = VI_NTSC_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(NTSC_320X240_16BIT);
		_frameBufferWidth = 320;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 0;
		_graphicMode = NORM_NTSC_320X240;
		enableAudioPlayback();
		break;

	case OVERS_NTSC_340X240:
	default:
		disableAudioPlayback();
		_viClockRate = VI_NTSC_CLOCK;
		_maxFps = N64_NTSC_FPS;
		initDisplay(NTSC_340X240_16BIT);
		_frameBufferWidth = 340;
		_screenWidth = 320;
		_screenHeight = 240;
		_offscrPixels = 16;
		_graphicMode = OVERS_NTSC_340X240;
		enableAudioPlayback();
		break;
	}
}

int OSystem_N64::getGraphicsMode() const {
	return _graphicMode;
}

void OSystem_N64::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	_gameWidth = width;
	_gameHeight = height;

	if (_gameWidth > _screenWidth)
		_gameWidth = _screenWidth;
	if (_gameHeight > _screenHeight)
		_gameHeight = _screenHeight;

	_mouseMaxX = _gameWidth;
	_mouseMaxY = _gameHeight;
}

int16 OSystem_N64::getHeight() {
	return _screenHeight;
}

int16 OSystem_N64::getWidth() {
	return _screenWidth;
}

void OSystem_N64::setPalette(const byte *colors, uint start, uint num) {
	for (int i = 0; i < num; ++i) {
		uint8 c[4];
		_screenPalette[start + i] = colBGR888toRGB555(colors[2], colors[1], colors[0]);
		colors += 4;
	}

	if (_cursorPaletteDisabled)
		rebuildOffscreenMouseBuffer();

	_dirtyPalette = true;
	_dirtyOffscreen = true;
}

void OSystem_N64::rebuildOffscreenGameBuffer(void) {
	// Regenerate hi-color offscreen buffer
	uint64 four_col_hi;
	uint32 four_col_pal;

	for (int h = 0; h < _gameHeight; h++) {
		for (int w = 0; w < _gameWidth; w += 4) {
			four_col_pal = *(uint32*)(_offscreen_pal + ((h * _screenWidth) + w));

			four_col_hi = 0;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >> 24) & 0xFF)] << 48;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >> 16) & 0xFF)] << 32;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >>  8) & 0xFF)] << 16;
			four_col_hi |= (uint64)_screenPalette[((four_col_pal >>  0) & 0xFF)] <<  0;

			// Save the converted pixels into hicolor buffer
			*(uint64*)((_offscreen_hic) + (h * _screenWidth) + w) = four_col_hi;
		}
	}
}

void OSystem_N64::rebuildOffscreenMouseBuffer(void) {
	uint16 width, height;
	uint16 *_pal_src = _cursorPaletteDisabled ? _screenPalette : _cursorPalette;

	for (height = 0; height < _cursorHeight; height++) {
		for (width = 0; width < _cursorWidth; width++) {
			_cursor_hic[(_cursorWidth * height) + width] = _pal_src[_cursor_pal[(_cursorWidth * height) + width]];
		}
	}
}

void OSystem_N64::grabPalette(byte *colors, uint start, uint num) {
	uint32 i;
	uint16 color;

	for (i = start; i < start + num; i++) {
		color = _screenPalette[i];

		// Color format on the n64 is RGB - 1555
		*colors++ = ((color & 0x1F) << 3);
		*colors++ = (((color >> 5) & 0x1F) << 3);
		*colors++ = (((color >> 10) & 0x1F) << 3);
		*colors++ = 0;
	}

	return;
}

void OSystem_N64::setCursorPalette(const byte *colors, uint start, uint num) {
	for (int i = 0; i < num; ++i) {
		_cursorPalette[start + i] = colBGR888toRGB555(colors[2], colors[1], colors[0]);
		colors += 4;
	}

	_cursorPaletteDisabled = false;

	rebuildOffscreenMouseBuffer();

	_dirtyOffscreen = true;
}

void OSystem_N64::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;

	rebuildOffscreenMouseBuffer();

	_dirtyOffscreen = true;
}

void OSystem_N64::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
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

	uint8 *dst_pal = _offscreen_pal + ((y * _screenWidth) + x);
	uint16 *dst_hicol = _offscreen_hic + ((y * _screenWidth) + x);

	do {
		for (int hor = 0; hor < w; hor++) {
			if (dst_pal[hor] != buf[hor]) {
				uint16 color = _screenPalette[buf[hor]];
				dst_hicol[hor] = color;  // Save image converted to 16-bit
				dst_pal[hor] = buf[hor]; // Save palettized display
			}
		}

		buf += pitch;
		dst_pal += _screenWidth;
		dst_hicol += _screenWidth;
	} while (--h);

	_dirtyOffscreen = true;

	return;
}

void OSystem_N64::updateScreen() {
#ifdef LIMIT_FPS
	static uint32 _lastScreenUpdate = 0;
	uint32 now = getMillis();
	if (now - _lastScreenUpdate < 1000 / _maxFps)
		return;

	_lastScreenUpdate = now;
#endif

	// Check if audio buffer needs refill
	// Done here because this gets called regularly
	refillAudioBuffers();

	if (!_dirtyOffscreen && !_dirtyPalette) return; // The offscreen is clean

	uint8 skip_lines = (_screenHeight - _gameHeight) / 4;
	uint8 skip_pixels = (_screenWidth - _gameWidth) / 2; // Center horizontally the image
	skip_pixels -= (skip_pixels % 8); // To keep aligned memory access

	if (_dirtyPalette)
		rebuildOffscreenGameBuffer();

	while (!(_dc = lockDisplay()));

	uint16 *overlay_framebuffer = (uint16*)_dc->conf.framebuffer; // Current screen framebuffer
	uint16 *game_framebuffer = overlay_framebuffer + (_frameBufferWidth * skip_lines * 2); // Skip some lines to center the image vertically

	uint16 currentHeight, currentWidth;
	uint16 *tmpDst;
	uint16 *tmpSrc;

	// Copy the game buffer to screen
	if (!_overlayVisible) {
		tmpDst = game_framebuffer;
		tmpSrc = _offscreen_hic + (_shakeOffset * _screenWidth);
		for (currentHeight = _shakeOffset; currentHeight < _gameHeight; currentHeight++) {
			uint64 *game_dest = (uint64*)(tmpDst + skip_pixels + _offscrPixels);
			uint64 *game_src = (uint64*)tmpSrc;

			// With uint64 we copy 4 pixels at a time
			for (currentWidth = 0; currentWidth < _gameWidth; currentWidth += 4) {
				*game_dest++ = *game_src++;
			}
			tmpDst += _frameBufferWidth;
			tmpSrc += _screenWidth;
		}

		uint16 _clearLines = _shakeOffset; // When shaking we must take care of remaining lines to clear
		while (_clearLines--) {
			memset(tmpDst + skip_pixels + _offscrPixels, 0, _screenWidth * 2);
			tmpDst += _frameBufferWidth;
		}
	} else { // If the overlay is enabled, draw it on top of game screen
		tmpDst = overlay_framebuffer;
		tmpSrc = _overlayBuffer;
		for (currentHeight = 0; currentHeight < _overlayHeight; currentHeight++) {
			uint64 *over_dest = (uint64*)(tmpDst + _offscrPixels);
			uint64 *over_src = (uint64*)tmpSrc;

			// Copy 4 pixels at a time
			for (currentWidth = 0; currentWidth < _overlayWidth; currentWidth += 4) {
				*over_dest++ = *over_src++;
			}
			tmpDst += _frameBufferWidth;
			tmpSrc += _overlayWidth;
		}
	}

	// Draw mouse cursor
	if ((_mouseVisible || _overlayVisible) && _cursorHeight > 0 && _cursorWidth > 0) {
		uint16 *mouse_framebuffer;
		uint16 horiz_pix_skip;

		if (_overlayVisible) {
			mouse_framebuffer = overlay_framebuffer;
			horiz_pix_skip = 0;
		} else {
			mouse_framebuffer = game_framebuffer;
			horiz_pix_skip = skip_pixels;
		}

		int mX = _mouseX - _mouseHotspotX;
		int mY = _mouseY - _mouseHotspotY;

		for (int h = 0; h < _cursorHeight; h++)
			for (int w = 0; w < _cursorWidth; w++) {
				// Draw pixel
				if (((mY + h) >= 0) && ((mY + h) < _mouseMaxY) && ((mX + w) >= 0) && ((mX + w) < _mouseMaxX)) {
					uint16 cursor_pixel_hic = _cursor_hic[(h * _cursorWidth) + w];
					uint8 cursor_pixel_pal = _cursor_pal[(h * _cursorWidth) + w];

					if (cursor_pixel_pal != _cursorKeycolor)
						mouse_framebuffer[((mY + h) * _frameBufferWidth) + ((mX + w) + _offscrPixels + horiz_pix_skip)] = cursor_pixel_hic;
				}
			}
	}

#ifndef _ENABLE_DEBUG_
	showDisplay(_dc);
#else
	showDisplayAndText(_dc);
#endif

	_dc = NULL;
	_dirtyOffscreen = false;
	_dirtyPalette = false;

	return;
}

Graphics::Surface *OSystem_N64::lockScreen() {
	_framebuffer.pixels = _offscreen_pal;
	_framebuffer.w = _gameWidth;
	_framebuffer.h = _gameHeight;
	_framebuffer.pitch = _screenWidth;
	_framebuffer.bytesPerPixel = 1;

	return &_framebuffer;
}

void OSystem_N64::unlockScreen() {
	_dirtyPalette = true;
	_dirtyOffscreen = true;
}

void OSystem_N64::setShakePos(int shakeOffset) {
	_shakeOffset = shakeOffset;
	_dirtyOffscreen = true;

	return;
}

void OSystem_N64::showOverlay() {
	// Change min/max mouse coords
	_mouseMaxX = _overlayWidth;
	_mouseMaxY = _overlayHeight;

	// Relocate the mouse cursor given the new limitations
	warpMouse(_mouseX, _mouseY);

	_overlayVisible = true;
	_dirtyOffscreen = true;
}

void OSystem_N64::hideOverlay() {
	// Change min/max mouse coords
	_mouseMaxX = _gameWidth;
	_mouseMaxY = _gameHeight;

	// Relocate the mouse cursor given the new limitations
	warpMouse(_mouseX, _mouseY);

	_overlayVisible = false;

	// Clear double buffered display
	clearAllVideoBuffers();

	_dirtyOffscreen = true;
}

void OSystem_N64::clearOverlay() {
	memset(_overlayBuffer, 0, _overlayWidth * _overlayHeight * sizeof(OverlayColor));

	uint8 skip_lines = (_screenHeight - _gameHeight) / 4;
	uint8 skip_pixels = (_screenWidth - _gameWidth) / 2; // Center horizontally the image

	uint16 *tmpDst = _overlayBuffer + (_overlayWidth * skip_lines * 2);
	uint16 *tmpSrc = _offscreen_hic + (_shakeOffset * _screenWidth);
	for (uint16 currentHeight = _shakeOffset; currentHeight < _gameHeight; currentHeight++) {
		memcpy((tmpDst + skip_pixels), tmpSrc, _gameWidth * 2);
		tmpDst += _overlayWidth;
		tmpSrc += _screenWidth;
	}

	_dirtyOffscreen = true;
}

void OSystem_N64::grabOverlay(OverlayColor *buf, int pitch) {
	int h = _overlayHeight;
	OverlayColor *src = _overlayBuffer;

	do {
		memcpy(buf, src, _overlayWidth * sizeof(OverlayColor));
		src += _overlayWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_N64::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
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

	_dirtyOffscreen = true;

	return;
}

int16 OSystem_N64::getOverlayHeight() {
	return _overlayHeight;
}

int16 OSystem_N64::getOverlayWidth() {
	return _overlayWidth;
}


bool OSystem_N64::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;

	_dirtyOffscreen = true;

	return last;
}

void OSystem_N64::warpMouse(int x, int y) {

	if (x < 0)
		_mouseX = 0;
	else if (x >= _mouseMaxX)
		_mouseX = _mouseMaxX - 1;
	else
		_mouseX = x;

	if (y < 0)
		_mouseY = 0;
	else if (y >= _mouseMaxY)
		_mouseY = _mouseMaxY - 1;
	else
		_mouseY = y;

	_dirtyOffscreen = true;
}

void OSystem_N64::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	if (!w || !h) return;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	if (_cursor_pal && ((w != _cursorWidth) || (h != _cursorHeight))) {
		free(_cursor_pal);
		free(_cursor_hic);
		_cursor_pal = NULL;
		_cursor_hic = NULL;
	}

	if (!_cursor_pal) {
		_cursor_pal = (uint8*)malloc(w * h);
		_cursor_hic = (uint16*)malloc(w * h * sizeof(uint16));
	}

	_cursorWidth = w;
	_cursorHeight = h;

	memcpy(_cursor_pal, buf, w * h); // Copy the palettized cursor

	_cursorKeycolor = keycolor & 0xFF;

	rebuildOffscreenMouseBuffer();

	_dirtyOffscreen = true;

	return;
}

uint32 OSystem_N64::getMillis() {
	return getMilliTick();
}

void OSystem_N64::delayMillis(uint msecs) {
#ifndef _NORMAL_N64_DELAY_
	// In some cases a game might hang waiting for audio being
	// played. This is a workaround for all the situations i
	// found (kyra 1 & 2 DOS).

	uint32 oldTime = getMilliTick();
	refillAudioBuffers();
	uint32 pastMillis = (getMilliTick() - oldTime);

	if (pastMillis >= msecs)
		return;
	else
		delay(msecs - pastMillis);
#else
	delay(msecs);
#endif
}

OSystem::MutexRef OSystem_N64::createMutex(void) {
	return NULL;
}

void OSystem_N64::lockMutex(MutexRef mutex) {
	return;
}

void OSystem_N64::unlockMutex(MutexRef mutex) {
	return;
}

void OSystem_N64::deleteMutex(MutexRef mutex) {
	return;
}

void OSystem_N64::quit() {
	// Not much to do...
	return;
}

Common::SaveFileManager *OSystem_N64::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

Audio::Mixer *OSystem_N64::getMixer() {
	assert(_mixer);
	return _mixer;
}

Common::TimerManager *OSystem_N64::getTimerManager() {
	assert(_timer);
	return _timer;
}

void OSystem_N64::getTimeAndDate(TimeDate &t) const {
	// No clock inside the N64
	// TODO: use getMillis to provide some kind of time-counting feature?
	t.tm_sec  = 0;
	t.tm_min  = 0;
	t.tm_hour = 0;
	t.tm_mday = 0;
	t.tm_mon  = 0;
	t.tm_year = 0;

	return;
}

FilesystemFactory *OSystem_N64::getFilesystemFactory() {
	return _fsFactory;
}

void OSystem_N64::setTimerCallback(TimerProc callback, int interval) {
	if (callback != NULL) {
		_timerCallbackTimer = interval;
		_timerCallbackNext = getMillis() + interval;
		_timerCallback = callback;
	} else
		_timerCallback = NULL;
}

void OSystem_N64::setupMixer(void) {
	enableAudioPlayback();
}

inline uint16 colBGR888toRGB555(byte r, byte g, byte b) {
	return ((r >> 3) << 1) | ((g >> 3) << 6) | ((b >> 3) << 11);
}

