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

#include "backends/modular-backend.h"
#include "base/main.h"

#if defined(USE_NULL_DRIVER)
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "sound/mixer_intern.h"
#include "common/scummsys.h"

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#if defined(__amigaos4__)
	#include "backends/fs/amigaos4/amigaos4-fs-factory.h"
#elif defined(UNIX)
	#include "backends/fs/posix/posix-fs-factory.h"
#elif defined(WIN32)
	#include "backends/fs/windows/windows-fs-factory.h"
#endif

class OSystem_NULL : public ModularBackend {
public:
	OSystem_NULL();
	virtual ~OSystem_NULL();

	virtual void initBackend();

	virtual bool pollEvent(Common::Event &event);

	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs) {}
	virtual void getTimeAndDate(TimeDate &t) const {}

	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();
};

OSystem_NULL::OSystem_NULL() {
	#if defined(__amigaos4__)
		_fsFactory = new AmigaOSFilesystemFactory();
	#elif defined(UNIX)
		_fsFactory = new POSIXFilesystemFactory();
	#elif defined(WIN32)
		_fsFactory = new WindowsFilesystemFactory();
	#else
		#error Unknown and unsupported FS backend
	#endif
}

OSystem_NULL::~OSystem_NULL() {
}

void OSystem_NULL::initBackend() {
	_mutexManager = (MutexManager *)new NullMutexManager();
	_timerManager = new DefaultTimerManager();
	_eventManager = new DefaultEventManager(this);
	_savefileManager = new DefaultSaveFileManager();
	_graphicsManager = (GraphicsManager *)new NullGraphicsManager();
	_audiocdManager = (AudioCDManager *)new DefaultAudioCDManager();
	_mixer = new Audio::MixerImpl(this, 22050);
	
	((Audio::MixerImpl *)_mixer)->setReady(false);

	// Note that both the mixer and the timer manager are useless
	// this way; they need to be hooked into the system somehow to
	// be functional. Of course, can't do that in a NULL backend :).

	OSystem::initBackend();
}

bool OSystem_NULL::pollEvent(Common::Event &event) {
	return false;
}

uint32 OSystem_NULL::getMillis() {
	return 0;
}

#if defined(UNIX)
#if defined(SAMSUNGTV)
#define DEFAULT_CONFIG_FILE "/dtv/usb/sda1/.scummvmrc"
#else
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#endif
#endif

#if !defined(UNIX)
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

Common::SeekableReadStream *OSystem_NULL::createConfigReadStream() {
	Common::FSNode file(DEFAULT_CONFIG_FILE);
	return file.createReadStream();
}

Common::WriteStream *OSystem_NULL::createConfigWriteStream() {
#ifdef __DC__
	return 0;
#else
	Common::FSNode file(DEFAULT_CONFIG_FILE);
	return file.createWriteStream();
#endif
}

OSystem *OSystem_NULL_create() {
	return new OSystem_NULL();
}

int main(int argc, char *argv[]) {
	g_system = OSystem_NULL_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	delete (OSystem_NULL *)g_system;
	return res;
}

#else /* USE_NULL_DRIVER */

OSystem *OSystem_NULL_create() {
	return NULL;
}

#endif
