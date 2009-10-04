/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef BACKEND_DRIVER_SDL_H
#define BACKEND_DRIVER_SDL_H

#include "backends/base-backend.h"

#include <SDL.h>
#ifdef USE_OPENGL
#include <SDL_opengl.h>
#endif

#include <time.h>

namespace Audio {
	class MixerImpl;
}

#if defined(MACOSX)
#define MIXER_DOUBLE_BUFFERING 1
#endif

class OSystem_SDL : public BaseBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	virtual void initBackend();

	// Set the size of the video bitmap.
	virtual void launcherInitSize(uint w, uint h);

	virtual byte *setupScreen(int screenW, int screenH, bool fullscreen, bool accel3d);

	// Update the dirty areas of the screen
	void updateScreen();

	// Either show or hide the mouse cursor
	bool showMouse(bool visible);

	// Warp the mouse cursor. Where set_mouse_pos() only informs the
	// backend of the mouse cursor's current position, this function
	// actually moves the cursor to the specified position.
	virtual void warpMouse(int x, int y);

	// Set the bitmap that's used when drawing the cursor.
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale); // overloaded by CE backend (FIXME)

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	// Get the next event.
	// Returns true if an event was retrieved.
	virtual bool pollEvent(Common::Event &event);

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
	virtual void quit();

	virtual void getTimeAndDate(struct tm &t) const;
	virtual Common::TimerManager *getTimerManager();

	// Mutex handling
	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Overlay
	virtual Graphics::PixelFormat getOverlayFormat() const { return _overlayFormat; }
	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual int16 getOverlayHeight()  { return _overlayHeight; }
	virtual int16 getOverlayWidth()   { return _overlayWidth; }

	virtual void setWindowCaption(const char *caption);
	virtual bool openCD(int drive);

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);

	virtual Common::SaveFileManager *getSavefileManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);
	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();

private:

#ifdef USE_OPENGL
	bool _opengl;
#endif
	bool _fullscreen;
	SDL_Surface *_screen;

	// overlay
	SDL_Surface *_overlayscreen;
	bool _overlayVisible;
	Graphics::PixelFormat _overlayFormat;
	int _overlayWidth, _overlayHeight;
	bool _overlayDirty;
	int _overlayNumTex;
	GLuint *_overlayTexIds;

	void closeOverlay();

	// Audio
	int _samplesPerSec;

	// CD Audio
	SDL_CD *_cdrom;
	int _cdTrack, _cdNumLoops, _cdStartFrame, _cdDuration;
	uint32 _cdEndTime, _cdStopTime;

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

	virtual void fillMouseEvent(Common::Event &event, int x, int y);

	// Keyboard mouse emulation.  Disabled by fingolfin 2004-12-18.
	// I am keeping the rest of the code in for now, since the joystick
	// code (or rather, "hack") uses it, too.
	struct KbdMouse {
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	};

	// mouse
	KbdMouse _km;

	void setupIcon();

	void handleKbdMouse();

	bool remapKey(SDL_Event &ev, Common::Event &event);
};

#endif
