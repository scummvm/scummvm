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

#include <stdio.h>
#include <time.h>

#include <gem.h>
#include <mint/cookie.h>
#include <mint/falcon.h>
#include <mint/osbind.h>

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fputs
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_sprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/platform/atari/osystem_atari.h"

#if defined(ATARI)

#include "backends/audiocd/default/default-audiocd.h"
#include "common/config-manager.h"
#include "backends/events/atari/atari-events.h"
#include "backends/events/default/default-events.h"
#include "backends/graphics/atari/atari-graphics-asm.h"
#include "backends/graphics/atari/atari-graphics-superblitter.h"
#include "backends/graphics/atari/atari-graphics-supervidel.h"
#include "backends/graphics/atari/atari-graphics-videl.h"
#include "backends/graphics/atari/atari-graphics.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/mixer/atari/atari-mixer.h"
#include "backends/mutex/null/null-mutex.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "base/main.h"
#include "gui/debugger.h"

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#include "backends/fs/posix/posix-fs-factory.h"

extern "C" void atari_kbdvec(void *);
extern "C" void atari_mousevec(void *);
extern "C" void atari_vkbderr(void);

extern "C" void atari_200hz_init();
extern "C" void atari_200hz_shutdown();
extern "C" volatile uint32 counter_200hz;

extern void nf_init(void);
extern void nf_print(const char* msg);

static int s_app_id = -1;
static int16 s_vdi_handle;
static int s_vdi_width, s_vdi_height;

static bool s_tt = false;
typedef void (*KBDVEC)(void *);
static KBDVEC s_kbdvec = nullptr;
static void (*s_vkbderr)(void) = nullptr;
static KBDVEC s_mousevec = nullptr;

static void (*s_old_procterm)(void) = nullptr;

static void exit_gem() {
	if (s_app_id != -1) {
		//wind_update(END_UPDATE);

		// redraw screen
		form_dial(FMD_FINISH, 0, 0, 0, 0, 0, 0, s_vdi_width, s_vdi_height);
		graf_mouse(M_ON, NULL);

		v_clsvwk(s_vdi_handle);
		appl_exit();
	}
}

static void critical_restore() {
	extern void AtariAudioShutdown();
	extern void AtariGraphicsShutdown();

	AtariAudioShutdown();
	AtariGraphicsShutdown();

	if (s_tt)
		Supexec(asm_screen_tt_restore);
	else
		Supexec(asm_screen_falcon_restore);
	Supexec(atari_200hz_shutdown);

	if (s_kbdvec && s_vkbderr && s_mousevec) {
		_KBDVECS *kbdvecs = Kbdvbase();
		((uintptr *)kbdvecs)[-1] = (uintptr)s_kbdvec;
		kbdvecs->vkbderr = s_vkbderr;
		kbdvecs->mousevec = s_mousevec;
		s_kbdvec = s_mousevec = nullptr;
	}

	exit_gem();
}

OSystem_Atari::OSystem_Atari() {
	_fsFactory = new POSIXFilesystemFactory();

	nf_init();

	enum {
		VDO_NO_ATARI_HW = 0xffff,
		VDO_ST = 0,
		VDO_STE,
		VDO_TT,
		VDO_FALCON,
		VDO_MILAN
	};

	long vdo = VDO_NO_ATARI_HW<<16;
	Getcookie(C__VDO, &vdo);
	vdo >>= 16;

	if (vdo != VDO_TT && vdo != VDO_FALCON) {
		error("ScummVM requires Atari TT/Falcon compatible video");
	}

	s_tt = (vdo == VDO_TT);

	enum {
		MCH_ST = 0,
		MCH_STE,
		MCH_TT,
		MCH_FALCON,
		MCH_CLONE,
		MCH_ARANYM
	};

	long mch = MCH_ST<<16;
	Getcookie(C__MCH, &mch);
	mch >>= 16;

	if (mch == MCH_ARANYM && Getcookie(C_fVDI, NULL) == C_FOUND) {
		error("Disable fVDI, ScummVM uses XBIOS video calls");
	}

	_KBDVECS *kbdvecs = Kbdvbase();
	s_kbdvec = (KBDVEC)(((uintptr *)kbdvecs)[-1]);
	s_vkbderr = kbdvecs->vkbderr;
	s_mousevec = kbdvecs->mousevec;

	((uintptr *)kbdvecs)[-1] = (uintptr)atari_kbdvec;
	kbdvecs->vkbderr = atari_vkbderr;
	kbdvecs->mousevec = atari_mousevec;

	Supexec(atari_200hz_init);
	_timerInitialized = true;

	if (s_tt)
		Supexec(asm_screen_tt_save);
	else
		Supexec(asm_screen_falcon_save);

	_videoInitialized = true;

	s_old_procterm = Setexc(VEC_PROCTERM, -1);
	(void)Setexc(VEC_PROCTERM, critical_restore);
}

OSystem_Atari::~OSystem_Atari() {
	debug("OSystem_Atari::~OSystem_Atari()");

	// _audiocdManager needs to be deleted before _mixerManager to avoid a crash.
	delete _audiocdManager;
	_audiocdManager = nullptr;

	delete _mixerManager;
	_mixerManager = nullptr;

	delete _graphicsManager;
	_graphicsManager = nullptr;

	delete _eventManager;
	_eventManager = nullptr;

	delete _savefileManager;
	_savefileManager = nullptr;

	delete _timerManager;
	_timerManager = nullptr;

	delete _fsFactory;
	_fsFactory = nullptr;

	if (_videoInitialized) {
		if (s_tt)
			Supexec(asm_screen_tt_restore);
		else {
			Supexec(asm_screen_falcon_restore);
		}

		_videoInitialized = false;
	}

	if (_timerInitialized) {
		Supexec(atari_200hz_shutdown);
		_timerInitialized = false;
	}

	if (s_kbdvec && s_vkbderr && s_mousevec) {
		_KBDVECS *kbdvecs = Kbdvbase();
		((uintptr *)kbdvecs)[-1] = (uintptr)s_kbdvec;
		kbdvecs->vkbderr = s_vkbderr;
		kbdvecs->mousevec = s_mousevec;
		s_kbdvec = s_mousevec = nullptr;
	}
}

void OSystem_Atari::initBackend() {
	_timerManager = new DefaultTimerManager();
	_savefileManager = new DefaultSaveFileManager("saves");

	AtariEventSource *atariEventSource = new AtariEventSource();
	_eventManager = new DefaultEventManager(makeKeyboardRepeatingEventSource(atariEventSource));

	// AtariGraphicsManager needs _eventManager ready
	AtariGraphicsManager *atariGraphicsManager;
#ifdef USE_SUPERVIDEL
	if (hasSuperVidel())
		atariGraphicsManager = new AtariSuperVidelManager();
	else
#endif
		atariGraphicsManager = new AtariVidelManager();
	_graphicsManager = atariGraphicsManager;

	atariEventSource->setGraphicsManager(atariGraphicsManager);

	_mixerManager = new AtariMixerManager();
	// Setup and start mixer
	_mixerManager->init();

	_startTime = counter_200hz;

	BaseBackend::initBackend();
}

Common::MutexInternal *OSystem_Atari::createMutex() {
	return new NullMutexInternal();
}

uint32 OSystem_Atari::getMillis(bool skipRecord) {
	// CLOCKS_PER_SEC is 200, so no need to use floats
	return 1000 * (counter_200hz - _startTime) / CLOCKS_PER_SEC;
}

void OSystem_Atari::delayMillis(uint msecs) {
	const uint32 threshold = getMillis() + msecs;
	while (getMillis() < threshold);
}

void OSystem_Atari::getTimeAndDate(TimeDate &td, bool skipRecord) const {
	//debug("getTimeAndDate");
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

Common::KeymapArray OSystem_Atari::getGlobalKeymaps() {
	Common::KeymapArray globalMaps = BaseBackend::getGlobalKeymaps();

	Common::Keymap *keymap = ((AtariGraphicsManager*)_graphicsManager)->getKeymap();
	globalMaps.push_back(keymap);

	return globalMaps;
}

Common::HardwareInputSet *OSystem_Atari::getHardwareInputSet() {
	Common::CompositeHardwareInputSet *inputSet = new Common::CompositeHardwareInputSet();
	inputSet->addHardwareInputSet(new Common::MouseHardwareInputSet(Common::defaultMouseButtons));
	inputSet->addHardwareInputSet(new Common::KeyboardHardwareInputSet(Common::defaultKeys, Common::defaultModifiers));

	return inputSet;
}

void OSystem_Atari::quit() {
	debug("OSystem_Atari::quit()");

	g_system->destroy();

	// graceful exit
	(void)Setexc(VEC_PROCTERM, s_old_procterm);

	exit_gem();
}

void OSystem_Atari::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	static char str[1024+1];
	sprintf(str, "[%08d] %s", getMillis(), message);

	fputs(str, output);
	fflush(output);

	nf_print(str);
}

void OSystem_Atari::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	{
		Common::FSDirectory currentDirectory{ getFilesystemFactory()->makeCurrentDirectoryFileNode()->getPath() };
		Common::FSNode dataNode = currentDirectory.getSubDirectory("data")->getFSNode();
		if (dataNode.exists() && dataNode.isDirectory() && dataNode.isReadable()) {
			s.addDirectory(dataNode.getPath(), dataNode, priority);
		}
	}
#ifdef DATA_PATH
	{
		// Add the global DATA_PATH to the directory search list
		// See also OSystem_SDL::addSysArchivesToSearchSet()
		Common::FSNode dataNode(DATA_PATH);
		if (dataNode.exists() && dataNode.isDirectory() && dataNode.isReadable()) {
			s.add(DATA_PATH, new Common::FSDirectory(dataNode, 4), priority);
		}
	}
#endif
}

Common::String OSystem_Atari::getDefaultConfigFileName() {
	const Common::String baseConfigName = OSystem::getDefaultConfigFileName();

	Common::String configFile;

	const char *envVar = getenv("HOME");
	if (envVar && *envVar) {
		configFile = envVar;
		configFile += '/';
		configFile += baseConfigName;

		if (configFile.size() < MAXPATHLEN)
			return configFile;
	}

	return baseConfigName;
}

void OSystem_Atari::update() {
	((DefaultTimerManager *)_timerManager)->checkTimers();
	((AtariMixerManager *)_mixerManager)->update();
}

OSystem *OSystem_Atari_create() {
	return new OSystem_Atari();
}

int main(int argc, char *argv[]) {
	s_app_id = appl_init();
	if (s_app_id != -1) {
		// get the ID of the current physical screen workstation
		int16 dummy;
		s_vdi_handle = graf_handle(&dummy, &dummy, &dummy, &dummy);
		if (s_vdi_handle < 1) {
			appl_exit();
			error("graf_handle() failed");
		}

		int16 work_in[16] = {};
		int16 work_out[57] = {};

		// open a virtual screen workstation
		v_opnvwk(work_in, &s_vdi_handle, work_out);

		if (s_vdi_handle == 0) {
			appl_exit();
			error("v_opnvwk() failed");
		}

		s_vdi_width = work_out[0] + 1;
		s_vdi_height = work_out[1] + 1;

		graf_mouse(M_OFF, NULL);
		// see https://github.com/freemint/freemint/issues/312
		//wind_update(BEG_UPDATE);
	}

	g_system = OSystem_Atari_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	g_system->destroy();

	// graceful exit
	(void)Setexc(VEC_PROCTERM, s_old_procterm);

	exit_gem();

	return res;
}

#endif
