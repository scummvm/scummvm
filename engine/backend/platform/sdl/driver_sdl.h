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

#include <SDL.h>

#include <time.h>

// NOTE: This is not a complete driver, it needs to be subclassed
//       to provide rendering functionality.

namespace Audio {
	class MixerImpl;
	class Mixer;
}

class DriverSDL : public Driver {
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
	void getTimeAndDate(struct tm &t) const;

	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	void setupMixer();
	static void mixCallback(void *s, byte *samples, int len);
	Audio::Mixer *getMixer();

	void quit();
	FilesystemFactory *getFilesystemFactory();
	Common::SaveFileManager *getSavefileManager();

private:

	int _samplesPerSec;
	Common::SaveFileManager *_savefile;
	Common::TimerManager *_timer;
	SDL_TimerID _timerID;

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

protected:

	Audio::MixerImpl *_mixer;
};

#endif
