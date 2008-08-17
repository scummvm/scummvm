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

	const ControlDescriptor *listControls();
	int getNumControls();
	bool controlIsAxis(int num);
	float getControlAxis(int num);
	bool getControlState(int num);
	bool pollEvent(Event &event);
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

protected:

	Audio::MixerImpl *_mixer;
};

#endif
