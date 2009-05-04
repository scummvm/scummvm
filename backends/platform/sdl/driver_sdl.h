/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BACKEND_DRIVER_SDL_H
#define BACKEND_DRIVER_SDL_H

#include "common/sys.h"

#include "engine/color.h"
#include "engine/model.h"
#include "engine/colormap.h"
#include "engine/bitmap.h"
#include "engine/vector3d.h"
#include "engine/backend/platform/driver.h"
#include "engine/backend/events/default/default-events.h"

#include <SDL.h>

#include <time.h>

// NOTE: This is not a complete driver, it needs to be subclassed
//       to provide rendering functionality.

namespace Audio {
	class MixerImpl;
	class Mixer;
}

class DriverSDL : public Driver, EventProvider {
public:
	DriverSDL();
	virtual ~DriverSDL();

	void init();

	void setupIcon();

	const char *getVideoDeviceName();

	void warpMouse(int x, int y);
	bool pollEvent(Common::Event &event);
	uint32 getMillis();
	void delayMillis(uint msecs);
	Common::TimerManager *getTimerManager();
	Common::EventManager *getEventManager();
	void getTimeAndDate(struct tm &t) const;

	// Set function that generates samples
	virtual void setupMixer();
	static void mixCallback(void *s, byte *samples, int len);

	virtual void closeMixer();

	virtual Audio::Mixer *getMixer();


	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	void quit();

	virtual Common::SaveFileManager *getSavefileManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();

private:

	int _samplesPerSec;

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

	void handleKbdMouse();

	bool remapKey(SDL_Event &ev, Common::Event &event);
};

#endif
