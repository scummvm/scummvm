/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef XPLIB_XPLIB_H
#define XPLIB_XPLIB_H

#include "bolt/bolt.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/events.h"
#include "common/file.h"
#include "common/mutex.h"

#include "graphics/paletteman.h"

namespace Bolt {

// RENDER FLAGS

#define RF_OVERLAY_ACTIVE    0x01  // RLE overlay source stored on front surface
#define RF_DOUBLE_BUFFER     0x02  // Double-buffer compositing enabled
#define RF_FULL_REDRAW       0x04  // Full-screen overwrite, skip dirty tracking
#define RF_FRONT_DIRTY       0x08  // Front surface pixels changed
#define RF_BACK_DIRTY        0x10  // Back surface pixels changed
#define RF_FRONT_PAL_DIRTY   0x20  // Front surface palette changed
#define RF_BACK_PAL_DIRTY    0x40  // Back surface palette changed
#define RF_CURSOR_VISIBLE    0x80  // Cursor should be drawn this frame

struct DisplaySpecs;
class BoltEngine;

struct XPCycleState {
	int16 startIndex;
	int16 endIndex;
	int16 delay;     // ms between rotations
	uint32 nextFire; // getMillis() deadline for next rotation
	bool active;

	XPCycleState() {
		startIndex = 0;
		endIndex = 0;
		delay = 0;
		nextFire = 0;
		active = false;
	}
};

typedef struct XPPicDesc {
	byte *pixelData;
	int16 width;
	int16 height;
	byte *palette;
	int16 paletteStart;
	int16 paletteCount;
	int16 flags;

	XPPicDesc() {
		pixelData = nullptr;
		width = 0;
		height = 0;
		palette = nullptr;
		paletteStart = 0;
		paletteCount = 0;
		flags = 0;
	}
} XPPicDesc;

enum XPSurfaceType : int {
	stFront = 0,
	stBack
};

typedef struct XPSurface {
	XPPicDesc mainPic;
	XPPicDesc overlayPic;
	int16 dirtyPalStart;
	int16 dirtyPalEnd;

	XPSurface() {
		dirtyPalStart = 0;
		dirtyPalEnd = 0;
	}
} XPSurface;

enum XPEventTypes : int16 {
	etEmpty      = 0,
	etTimer      = 1,
	etMouseMove  = 2,
	etMouseDown  = 3,
	etMouseUp    = 4,
	etJoystick   = 5,
	etSound      = 6,
	etInactivity = 7,
	etTrigger    = 8
};

enum XPEventKeyStates : int16 {
	eksInputOff     = 0,
	eksMouseMode    = 1,
	eksJoystickMode = 2
};

enum CustomEventType {
	EVENT_TIMER = Common::EVENT_USER_FIRST_AVAILABLE
};

typedef struct XPEvent {
	XPEvent *prev;
	XPEvent *next;
	XPEventTypes type;
	uint32 payload;
	byte *payloadPtr;

	XPEvent() {
		prev = nullptr;
		next = nullptr;
		type = etEmpty;
		payload = 0;
		payloadPtr = nullptr;
	}
} XPEvent;

typedef struct XPTimer {
	uint32 id;
	uint32 deadline;
	bool active;

	XPTimer() {
		id = 0;
		deadline = 0;
		active = false;
	}
} XPTimer;

class XpLib {
friend class BoltEngine;

public:
	XpLib(BoltEngine *bolt);
	~XpLib();

	bool initialize();
	void terminate();

	// Blit
	void blit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height);
	void maskBlit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height);

	// Palette
	void getPalette(int16 startIndex, int16 count, byte *destBuf);
	void setPalette(int16 count, int16 startIndex, byte *srcBuf);
	bool startCycle(XPCycleState *specs);
	void cycleColors();
	void stopCycle();
	void setScreenBrightness(uint8 percent);

	// Cursor
	bool readCursor(uint16 *outButtons, int16 *outX, int16 *outY);
	void setCursorPos(int16 x, int16 y);
	void setCursorImage(byte *bitmap, int16 hotspotX, int16 hotspotY);
	void setCursorColor(byte r, byte g, byte b);
	bool showCursor();
	void hideCursor();
	void updateCursorPosition();

	// Events
	int16 getEvent(int16 filter, uint32 *outData, byte **outPtrData = nullptr);
	int16 peekEvent(int16 filter, uint32 *outData, byte **outPtrData = nullptr);
	void postEvent(XPEventTypes type, uint32 data, byte *ptrData = nullptr);
	int16 setInactivityTimer(int16 seconds);
	int16 setScreenSaverTimer(int16 seconds);
	bool enableController();
	void disableController();

	// Display
	bool setDisplaySpec(int *outMode, DisplaySpecs *spec);
	void setCoordSpec(int16 x, int16 y, int16 width, int16 height);
	void displayPic(XPPicDesc *pic, int16 x, int16 y, int16 page);
	void setFrameRate(int16 fps);
	void updateDisplay();
	void setTransparency(bool toggle);
	void fillDisplay(byte color, int16 page);

	// Random
	int16 getRandom(int16 range);
	void randomize();

	// File
	Common::File *openFile(const char *fileName, int16 flags);
	void closeFile(Common::File *handle);
	bool readFile(Common::File *handle, void *buffer, uint32 *size);
	bool setFilePos(Common::File *handle, int32 offset, int32 origin);
	void *allocMem(uint32 size);
	void *tryAllocMem(uint32 size);
	void freeMem(void *mem);

	// Sound
	bool playSound(byte *data, uint32 size, int16 sampleRate);
	bool pauseSound();
	bool resumeSound();
	bool stopSound();

	// Timer
	uint32 startTimer(int16 delay);
	void updateTimers();
	bool killTimer(uint32 timerId);

protected:
	BoltEngine *_bolt = nullptr;
	bool _xpInitialized = false;

	// Blit
	void dirtyBlit(byte *src, byte *dst, uint16 width, uint16 height, byte *dirtyFlags);
	void compositeBlit(byte *src, byte *background, byte *dst, uint16 stride, uint16 width, uint16 height);
	void rleBlit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height);
	void rleMaskBlit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height);
	void rleCompositeBlit(byte *rle, byte *background, byte *dst, uint16 width, uint16 height, byte *dirtyFlags);
	uint16 rleDataSize(byte *rleData, uint16 height);
	void markCursorPixels(byte *buffer, uint32 count);

	// Palette
	XPCycleState _cycleStates[4];
	byte _paletteBuffer[3 * 256];
	byte _shiftedPaletteBuffer[3 * 256];
	byte _cycleTempPalette[3 * 20];
	uint32 _cycleTimerIds[4];
	int16 _brightnessShift = 0;

	// Cursor
	bool initCursor();
	void shutdownCursor();
	void readJoystick(int16 *outX, int16 *outY);

	byte _cursorBuffer[16 * 16];
	int16 _cursorHotspotX = 0;
	int16 _cursorHotspotY = 0;
	int16 _lastCursorX = 0;
	int16 _lastCursorY = 0;
	int16 _cursorViewportWidth = 0;
	int16 _cursorViewportHeight = 0;
	int16 _cursorHidden = 1;

	// Events
	bool initEvents();
	void shutdownEvents();
	void unlinkEvent(XPEvent *node);
	void enqueueEvent(XPEvent *node);
	void pumpMessages();
	void handleTimer(uint32 timerId);
	void handleMouseMove(bool *mouseMoved);
	void handleMouseButton(int16 down, int16 button);
	void handleKey(Common::KeyCode vkey, int16 down);
	void postJoystickEvent(int16 source, int16 dx, int16 dy);
	bool canDropEvent(int16 type);
	void activateScreenSaver();
	void resetInactivity();
	void enableMouse();
	void disableMouse();
	int16 getButtonState();

	XPEvent _events[40];

	int8 _keyStateLeft = 0;
	int8 _keyStateRight = 0;
	int8 _keyStateUp = 0;
	int8 _keyStateDown = 0;
	int16 _lastJoystickX = 0;
	int16 _lastJoystickY = 0;
	int16 _mouseButtonPrev = 0;
	int16 _mouseButtonState = 0;
	int16 _eventMouseMoved = 0;
	int16 _eventKeyStates = 0;
	uint32 _inactivityDeadline = 0;

	XPEvent *_eventQueueHead = nullptr;
	XPEvent *_eventQueueTail = nullptr;
	XPEvent *_eventFreeList = nullptr;

	uint32 _lastMouseEventData = 0;
	Common::Point _lastRegisteredMousePos;

	// Display
	bool initDisplay();
	void shutdownDisplay();
	bool createSurface(XPSurface *surf);
	void freeSurface(XPSurface *surf);
	void virtualToScreen(int16 *x, int16 *y);
	void screenToVirtual(int16 *x, int16 *y);
	void dispatchBlit(int16 mode, byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height);
	bool clipAndBlit(XPPicDesc *src, XPPicDesc *dest, int16 x, int16 y, Common::Rect *outClip);
	void addDirtyRect(Common::Rect *rect);
	void waitForFrameRate();
	void handlePaletteTransitions();
	void flushPalette();
	void overlayComposite();
	void compositeToScreen();
	void mergeDirtyRects();
	void blitDirtyRects(Common::Rect *rects, int16 count);
	void compositeDirtyRects(Common::Rect *rects, int16 count);
	void applyCursorPalette();
	void prepareBackSurface();

	XPSurface _surfaces[2];

	int16 _virtualWidth = 0;
	int16 _virtualHeight = 0;
	int16 _currentDisplayPage = 0;
	int16 _prevRenderFlags = 0;
	int16 _renderFlags = 0;
	int16 _frameRateFPS = 0;
	int16 _overlayCount = 0;
	int16 _prevDirtyCount = 0;
	int16 _prevDirtyValid = 0;
	int16 _surfaceWidth = 0;
	int16 _surfaceHeight = 0;
	int16 _viewportOffsetX = 0;
	int16 _viewportOffsetY = 0;
	uint32 _nextFrameTime = 0;

	Common::Rect _dirtyRects[30];
	Common::Rect _prevDirtyRects[30];
	Common::Rect _cursorRect;
	Common::Rect _prevCursorRect;
	Common::Rect _overlayCursorRect;
	Common::Rect _prevOverlayCursorRect;

	byte *_vgaFramebuffer = nullptr;
	byte *_rowDirtyFlags = nullptr;
	byte _cursorBackgroundSaveBuffer[16 * 16];

	XPPicDesc _cursorBackgroundSave;
	XPPicDesc _cursorSprite;

	// File
	void fileError(const char *message);

	// Sound
	bool pollSound(byte **outData);
	bool initSound();
	void shutdownSound();

	Audio::QueuingAudioStream *_audioStream = nullptr;
	Audio::SoundHandle _soundHandle;
	Common::Queue<uint32> _durationQueue;
	uint32 _nextSoundDeadlineMs = 0;
	uint32 _pauseTimeMs = 0;
	int16 _sndPlayState = 0;
	int16 _sndQueued = 0;
	int _sndCompletedCount = 0;
	bool _sndPaused = false;
	int16 _sndSampleRate = 22050;
	uint32 _sndNextDeadline = 0;
	uint32 _sndBufferQueueTime = 0;
	Common::Queue<byte *> _bufferSourceQueue;

	// Timer
	bool initTimer();
	void shutdownTimer();

	uint32 _inactivityTimerId = 0;
	int16 _inactivityCountdown = 0;
	int16 _inactivityTimerValue = 0;
	int16 _screensaverCountdown = 0;
	int16 _screenSaverTimerValue = 0;
	int16 _inactivityTimeout = 0;

	bool _timerInitialized = false;

	XPTimer _timers[128];
	uint16 _nextTimerId = 0;
};

} // End of namespace Bolt

#endif // XPLIB_XPLIB_H
