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

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

#include "backends/base-backend.h"

#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/graphics/sdl/sdl-graphics.h"

#include "graphics/scaler.h"


namespace Audio {
	class MixerImpl;
}

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
// Uncomment this to enable the 'on screen display' code.
#define USE_OSD	1
#endif

#if defined(MACOSX)
// On Mac OS X, we need to double buffer the audio buffer, else anything
// which produces sampled data with high latency (like the MT-32 emulator)
// will sound terribly.
// This could be enabled for more / most ports in the future, but needs some
// testing.
#define MIXER_DOUBLE_BUFFERING 1
#endif

class OSystem_SDL : public BaseBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	virtual void initBackend();


protected:
	SdlMutexManager *_mutexManager;
	SdlGraphicsManager *_graphicsManager;

public:
	void beginGFXTransaction();
	TransactionError endGFXTransaction();

#ifdef USE_RGB_COLOR
	// Game screen
	virtual Graphics::PixelFormat getScreenFormat() const;

	// Highest supported
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats();
#endif

	// Set the size and format of the video bitmap.
	// Typically, 320x200 CLUT8
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format); // overloaded by CE backend

	virtual int getScreenChangeID() const;

	// Set colors of the palette
	void setPalette(const byte *colors, uint start, uint num);

	// Get colors of the palette
	void grabPalette(byte *colors, uint start, uint num);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	virtual void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h); // overloaded by CE backend (FIXME)

	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();

	// Update the dirty areas of the screen
	void updateScreen();

	// Either show or hide the mouse cursor
	bool showMouse(bool visible);

	// Warp the mouse cursor. Where set_mouse_pos() only informs the
	// backend of the mouse cursor's current position, this function
	// actually moves the cursor to the specified position.
	virtual void warpMouse(int x, int y); // overloaded by CE backend (FIXME)

	// Set the bitmap that's used when drawing the cursor.
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format); // overloaded by CE backend (FIXME)

	// Set colors of cursor palette
	void setCursorPalette(const byte *colors, uint start, uint num);

	// Disables or enables cursor palette
	void disableCursorPalette(bool disable);

	// Shaking is used in SCUMM. Set current shake position.
	void setShakePos(int shake_pos);

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	// Get the next event.
	// Returns true if an event was retrieved.
	virtual bool pollEvent(Common::Event &event); // overloaded by CE backend

protected:
	virtual bool dispatchSDLEvent(SDL_Event &ev, Common::Event &event);

	// Handlers for specific SDL events, called by pollEvent.
	// This way, if a backend inherits fromt the SDL backend, it can
	// change the behavior of only a single event, without having to override all
	// of pollEvent.
	virtual bool handleKeyDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleKeyUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseMotion(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyAxisMotion(SDL_Event &ev, Common::Event &event);

public:


	// Define all hardware keys for keymapper
	virtual Common::HardwareKeySet *getHardwareKeySet();

	// Set function that generates samples
	virtual void setupMixer();
	static void mixCallback(void *s, byte *samples, int len);

	virtual void closeMixer();

	virtual Audio::Mixer *getMixer();

	// Poll CD status
	// Returns true if cd audio is playing
	bool pollCD();

	// Play CD audio track
	void playCD(int track, int num_loops, int start_frame, int duration);

	// Stop CD audio track
	void stopCD();

	// Update CD audio status
	void updateCD();

	// Quit
	virtual void quit(); // overloaded by CE backend

	void deinit();

	virtual void getTimeAndDate(TimeDate &t) const;
	virtual Common::TimerManager *getTimerManager();

	// Mutex handling
	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Overlay
	virtual Graphics::PixelFormat getOverlayFormat() const;

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();

	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;

	virtual void setWindowCaption(const char *caption);
	virtual bool openCD(int drive);

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual void preprocessEvents(SDL_Event *event) {}

#ifdef USE_OSD
	void displayMessageOnOSD(const char *msg);
#endif

	virtual Common::SaveFileManager *getSavefileManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();

protected:
	bool _inited;
	SDL_AudioSpec _obtainedRate;

	// CD Audio
	SDL_CD *_cdrom;
	int _cdTrack, _cdNumLoops, _cdStartFrame, _cdDuration;
	uint32 _cdEndTime, _cdStopTime;

	// Keyboard mouse emulation.  Disabled by fingolfin 2004-12-18.
	// I am keeping the rest of the code in for now, since the joystick
	// code (or rather, "hack") uses it, too.
	struct KbdMouse {
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	};

	KbdMouse _km;

	// Scroll lock state - since SDL doesn't track it
	bool _scrollLock;
	
	// joystick
	SDL_Joystick *_joystick;

#ifdef MIXER_DOUBLE_BUFFERING
	SDL_mutex *_soundMutex;
	SDL_cond *_soundCond;
	SDL_Thread *_soundThread;
	bool _soundThreadIsRunning;
	bool _soundThreadShouldQuit;

	byte _activeSoundBuf;
	uint _soundBufSize;
	byte *_soundBuffers[2];

	void mixerProducerThread();
	static int SDLCALL mixerProducerThreadEntry(void *arg);
	void initThreadedMixer(Audio::MixerImpl *mixer, uint bufSize);
	void deinitThreadedMixer();
#endif

	FilesystemFactory *_fsFactory;
	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;

	SDL_TimerID _timerID;
	Common::TimerManager *_timer;

protected:
	virtual void fillMouseEvent(Common::Event &event, int x, int y); // overloaded by CE backend
	void toggleMouseGrab();

	void handleKbdMouse();

	void setupIcon();

	virtual bool remapKey(SDL_Event &ev, Common::Event &event);
};

#endif
