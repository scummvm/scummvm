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
#include <stdlib.h>
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
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_exit

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
#include "backends/platform/atari/atari-debug.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "base/main.h"

#define INPUT_ACTIVE

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#include "backends/fs/posix/posix-fs-factory.h"

extern "C" void atari_kbdvec(void *);
extern "C" void atari_mousevec(void *);
typedef void (*KBDVEC)(void *);
extern "C" KBDVEC atari_old_kbdvec;
extern "C" KBDVEC atari_old_mousevec;

extern void nf_init(void);
extern void nf_print(const char* msg);

static int s_app_id = -1;

static volatile uint32 counter_200hz;

static bool exit_already_called = false;

static long atari_200hz_init(void)
{
	__asm__ __volatile__(
	"\tmove		%%sr,-(%%sp)\n"
	"\tor.w		#0x700,%%sr\n"

	"\tmove.l	0x114.w,old_200hz\n"
	"\tmove.l	#my_200hz,0x114.w\n"

	"\tmove		(%%sp)+,%%sr\n"
	"\tjbra		1f\n"

	"\tdc.l		0x58425241\n" /* "XBRA" */
	"\tdc.l		0x5343554d\n" /* "SCUM" */
"old_200hz:\n"
	"\tdc.l		0\n"
"my_200hz:\n"
	"\taddq.l	#1,%0\n"

	"\tmove.l	old_200hz(%%pc),-(%%sp)\n"
	"\trts\n"
"1:\n"
	: /* output */
	: "m"(counter_200hz) /* inputs */
	: "memory", "cc");

	return 0;
}

static long atari_200hz_shutdown(void)
{
	__asm__ __volatile__(
	"\tmove		%%sr,-(%%sp)\n"
	"\tor.w		#0x700,%%sr\n"

	"\tmove.l	old_200hz,0x114.w\n"

	"\tmove		(%%sp)+,%%sr\n"
	: /* output */
	: /* inputs */
	: "memory", "cc");

	return 0;
}

static void critical_restore() {
	extern void AtariAudioShutdown();
	extern void AtariGraphicsShutdown();

	AtariAudioShutdown();
	AtariGraphicsShutdown();

	Supexec(atari_200hz_shutdown);

#ifdef INPUT_ACTIVE
	if (atari_old_kbdvec && atari_old_mousevec) {
		_KBDVECS *kbdvecs = Kbdvbase();
		((uintptr *)kbdvecs)[-1] = (uintptr)atari_old_kbdvec;
		kbdvecs->mousevec = atari_old_mousevec;
		atari_old_kbdvec = atari_old_mousevec = nullptr;
	}

	// don't call GEM cleanup in the critical handler: it seems that v_clsvwk()
	// somehow manipulates the same memory area used for the critical handler's stack
	// what causes v_clsvwk() never returning and leading to a bus error (and another
	// critical_restore() called...)
	if (s_app_id != -1) {
		// ok, restore mouse cursor at least
		graf_mouse(M_ON, NULL);
	}
#endif
}

// called on normal program termination (via exit() or returning from main())
static void exit_restore() {
	if (!exit_already_called)
		g_system->destroy();
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
		fprintf(stderr, "ScummVM requires Atari TT/Falcon compatible video\n");
		exit(EXIT_FAILURE);
	}

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
		fprintf(stderr, "Disable fVDI, ScummVM uses XBIOS video calls\n");
		exit(EXIT_FAILURE);
	}

#ifdef INPUT_ACTIVE
	_KBDVECS *kbdvecs = Kbdvbase();
	atari_old_kbdvec = (KBDVEC)(((uintptr *)kbdvecs)[-1]);
	atari_old_mousevec = kbdvecs->mousevec;

	((uintptr *)kbdvecs)[-1] = (uintptr)atari_kbdvec;
	kbdvecs->mousevec = atari_mousevec;
#endif

	Supexec(atari_200hz_init);
	_timerInitialized = true;

	// protect against sudden exit()
	atexit(exit_restore);
	// protect against sudden crash
	_old_procterm = Setexc(VEC_PROCTERM, -1);
	(void)Setexc(VEC_PROCTERM, critical_restore);
}

OSystem_Atari::~OSystem_Atari() {
	atari_debug("OSystem_Atari::~OSystem_Atari()");

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

	if (_timerInitialized) {
		Supexec(atari_200hz_shutdown);
		_timerInitialized = false;
	}

	if (atari_old_kbdvec && atari_old_mousevec) {
		_KBDVECS *kbdvecs = Kbdvbase();
		((uintptr *)kbdvecs)[-1] = (uintptr)atari_old_kbdvec;
		kbdvecs->mousevec = atari_old_mousevec;
		atari_old_kbdvec = atari_old_mousevec = nullptr;
	}

	if (s_app_id != -1) {
		//wind_update(END_UPDATE);

		// redraw screen
		form_dial(FMD_FINISH, 0, 0, 0, 0, 0, 0, _vdi_width, _vdi_height);
		graf_mouse(M_ON, NULL);

		v_clsvwk(_vdi_handle);
		appl_exit();
	}

	// graceful exit
	exit_already_called = true;
	(void)Setexc(VEC_PROCTERM, _old_procterm);
}

void OSystem_Atari::initBackend() {
	s_app_id = appl_init();
	if (s_app_id != -1) {
		// get the ID of the current physical screen workstation
		int16 dummy;
		_vdi_handle = graf_handle(&dummy, &dummy, &dummy, &dummy);
		if (_vdi_handle < 1) {
			appl_exit();
			error("graf_handle() failed");
		}

		int16 work_in[16] = {};
		int16 work_out[57] = {};

		// open a virtual screen workstation
		v_opnvwk(work_in, &_vdi_handle, work_out);

		if (_vdi_handle == 0) {
			appl_exit();
			error("v_opnvwk() failed");
		}

		_vdi_width = work_out[0] + 1;
		_vdi_height = work_out[1] + 1;

#ifdef INPUT_ACTIVE
		graf_mouse(M_OFF, NULL);
		// see https://github.com/freemint/freemint/issues/312
		//wind_update(BEG_UPDATE);
#endif
	}

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

#ifdef DISABLE_FANCY_THEMES
	// On the lite build force "STMIDI" as the audio driver, i.e. do not attempt
	// to emulate anything by default. That prevents mixing silence and enable
	// us to stop DMA playback which takes unnecessary cycles.
	if (!ConfMan.hasKey("music_driver")) {
		ConfMan.set("music_driver", "stmidi");
	}
	if (!ConfMan.hasKey("gm_device")) {
		ConfMan.set("gm_device", "auto");
	}
	if (!ConfMan.hasKey("mt32_device")) {
		ConfMan.set("mt32_device", "auto");
	}
#endif

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
	while (getMillis() < threshold) {
		update();
	}
}

void OSystem_Atari::getTimeAndDate(TimeDate &td, bool skipRecord) const {
	//atari_debug("getTimeAndDate");
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
	atari_debug("OSystem_Atari::quit()");

	destroy();
}

void OSystem_Atari::logMessage(LogMessageType::Type type, const char *message) {
	extern long nf_stderr_id;

	static char str[1024+1];
	snprintf(str, sizeof(str), "[%08d] %s", getMillis(), message);

	if (nf_stderr_id) {
		nf_print(str);
	} else {
		FILE *output = 0;

		if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
			output = stdout;
		else
			output = stderr;

		fputs(str, output);
		fflush(output);
	}
}

void OSystem_Atari::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	{
		Common::FSDirectory currentDirectory{ Common::Path(getFilesystemFactory()->makeCurrentDirectoryFileNode()->getPath()) };
		Common::FSDirectory *dataDirectory = currentDirectory.getSubDirectory("data");
		if (dataDirectory) {
			Common::FSNode dataNode = dataDirectory->getFSNode();
			if (dataNode.exists() && dataNode.isDirectory() && dataNode.isReadable()) {
				s.addDirectory(dataNode.getPath(), priority);
			}
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

Common::Path OSystem_Atari::getDefaultConfigFileName() {
	const Common::Path baseConfigName = OSystem::getDefaultConfigFileName();

	const char *envVar = getenv("HOME");
	if (envVar && *envVar) {
		Common::Path configFile(envVar);
		configFile.joinInPlace(baseConfigName);

		if (configFile.toString(Common::Path::kNativeSeparator).size() < MAXPATHLEN)
			return configFile;
	}

	return baseConfigName;
}

void OSystem_Atari::update() {
	// avoid a recursion loop if a timer callback decides to call OSystem::delayMillis()
	static bool inTimer = false;

	if (!inTimer) {
		inTimer = true;
		((DefaultTimerManager *)_timerManager)->checkTimers();
		inTimer = false;
	} else {
		const Common::ConfigManager::Domain *activeDomain = ConfMan.getActiveDomain();
		warning("%s/%s calls update() from timer",
			activeDomain->getValOrDefault("engineid").c_str(),
			activeDomain->getValOrDefault("gameid").c_str());
	}

	((AtariMixerManager *)_mixerManager)->update();
}

OSystem *OSystem_Atari_create() {
	return new OSystem_Atari();
}

int main(int argc, char *argv[]) {
	g_system = OSystem_Atari_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	g_system->destroy();

	return res;
}

#endif
