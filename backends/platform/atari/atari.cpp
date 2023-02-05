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

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <mint/cookie.h>
#include <mint/falcon.h>
#include <mint/osbind.h>

// We use some stdio.h functionality here thus we need to allow some
// symbols. Alternatively, we could simply allow everything by defining
// FORBIDDEN_SYMBOL_ALLOW_ALL
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_fputs
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "common/scummsys.h"

#if defined(ATARI)
#include "backends/graphics/atari/atari-graphics-asm.h"
#include "backends/keymapper/hardware-input.h"
#include "backends/modular-backend.h"
#include "backends/mutex/null/null-mutex.h"
#include "base/main.h"

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "backends/mixer/atari/atari-mixer.h"
#include "backends/graphics/atari/atari-graphics.h"
#include "backends/graphics/atari/atari-graphics-supervidel.h"
#include "backends/graphics/atari/atari-graphics-videl.h"
#include "common/hashmap.h"
#include "gui/debugger.h"

/*
 * Include header files needed for the getFilesystemFactory() method.
 */
#include "backends/fs/posix/posix-fs-factory.h"

class OSystem_Atari : public ModularMixerBackend, public ModularGraphicsBackend, Common::EventSource {
public:
	OSystem_Atari();
	virtual ~OSystem_Atari();

	void initBackend() override;

	bool pollEvent(Common::Event &event) override;

	Common::MutexInternal *createMutex() override;
	uint32 getMillis(bool skipRecord = false) override;
	void delayMillis(uint msecs) override;
	void getTimeAndDate(TimeDate &td, bool skipRecord = false) const override;

	Common::KeymapArray getGlobalKeymaps() override;
	Common::HardwareInputSet *getHardwareInputSet() override;

	void quit() override;

	void logMessage(LogMessageType::Type type, const char *message) override;

	void addSysArchivesToSearchSet(Common::SearchSet &s, int priority) override;
	Common::String getDefaultConfigFileName() override;

private:
	AtariGraphicsManager *_atariGraphicsManager;

	clock_t _startTime;

	bool _video_initialized = false;
	bool _ikbd_initialized = false;

	bool _oldLmbDown = false;
	bool _oldRmbDown = false;

	bool _lshiftActive = false;
	bool _rshiftActive = false;
	bool _ctrlActive = false;
	bool _altActive = false;
	bool _capslockActive = false;

	byte _unshiftToAscii[128];
	byte _shiftToAscii[128];
	byte _capsToAscii[128];

	const Common::KeyCode _asciiToKeycode[128 - 32 - 1] = {
		Common::KEYCODE_SPACE,
		Common::KEYCODE_EXCLAIM,
		Common::KEYCODE_QUOTEDBL,
		Common::KEYCODE_HASH,
		Common::KEYCODE_DOLLAR,
		Common::KEYCODE_PERCENT,
		Common::KEYCODE_AMPERSAND,
		Common::KEYCODE_QUOTE,
		Common::KEYCODE_LEFTPAREN,
		Common::KEYCODE_RIGHTPAREN,
		Common::KEYCODE_ASTERISK,
		Common::KEYCODE_PLUS,
		Common::KEYCODE_COMMA,
		Common::KEYCODE_MINUS,
		Common::KEYCODE_PERIOD,
		Common::KEYCODE_SLASH,
		Common::KEYCODE_0,
		Common::KEYCODE_1,
		Common::KEYCODE_2,
		Common::KEYCODE_3,
		Common::KEYCODE_4,
		Common::KEYCODE_5,
		Common::KEYCODE_6,
		Common::KEYCODE_7,
		Common::KEYCODE_8,
		Common::KEYCODE_9,
		Common::KEYCODE_COLON,
		Common::KEYCODE_SEMICOLON,
		Common::KEYCODE_LESS,
		Common::KEYCODE_EQUALS,
		Common::KEYCODE_GREATER,
		Common::KEYCODE_QUESTION,
		Common::KEYCODE_AT,
		Common::KEYCODE_a,
		Common::KEYCODE_b,
		Common::KEYCODE_c,
		Common::KEYCODE_d,
		Common::KEYCODE_e,
		Common::KEYCODE_f,
		Common::KEYCODE_g,
		Common::KEYCODE_h,
		Common::KEYCODE_i,
		Common::KEYCODE_j,
		Common::KEYCODE_k,
		Common::KEYCODE_l,
		Common::KEYCODE_m,
		Common::KEYCODE_n,
		Common::KEYCODE_o,
		Common::KEYCODE_p,
		Common::KEYCODE_q,
		Common::KEYCODE_r,
		Common::KEYCODE_s,
		Common::KEYCODE_t,
		Common::KEYCODE_u,
		Common::KEYCODE_v,
		Common::KEYCODE_w,
		Common::KEYCODE_x,
		Common::KEYCODE_y,
		Common::KEYCODE_z,
		Common::KEYCODE_LEFTBRACKET,
		Common::KEYCODE_BACKSLASH,
		Common::KEYCODE_RIGHTBRACKET,
		Common::KEYCODE_CARET,
		Common::KEYCODE_UNDERSCORE,
		Common::KEYCODE_BACKQUOTE,
		Common::KEYCODE_a,
		Common::KEYCODE_b,
		Common::KEYCODE_c,
		Common::KEYCODE_d,
		Common::KEYCODE_e,
		Common::KEYCODE_f,
		Common::KEYCODE_g,
		Common::KEYCODE_h,
		Common::KEYCODE_i,
		Common::KEYCODE_j,
		Common::KEYCODE_k,
		Common::KEYCODE_l,
		Common::KEYCODE_m,
		Common::KEYCODE_n,
		Common::KEYCODE_o,
		Common::KEYCODE_p,
		Common::KEYCODE_q,
		Common::KEYCODE_r,
		Common::KEYCODE_s,
		Common::KEYCODE_t,
		Common::KEYCODE_u,
		Common::KEYCODE_v,
		Common::KEYCODE_w,
		Common::KEYCODE_x,
		Common::KEYCODE_y,
		Common::KEYCODE_z,
		Common::KEYCODE_INVALID,	// {
		Common::KEYCODE_INVALID,	// |
		Common::KEYCODE_INVALID,	// }
		Common::KEYCODE_TILDE
	};
	Common::HashMap<byte, Common::KeyCode> _scancodeToKeycode;
};

extern "C" void atari_ikbd_init();
extern "C" void atari_ikbd_shutdown();

extern void nf_init(void);
extern void nf_print(const char* msg);

OSystem_Atari::OSystem_Atari() {
	_fsFactory = new POSIXFilesystemFactory();

	_scancodeToKeycode[0x01] = Common::KEYCODE_ESCAPE;
	_scancodeToKeycode[0x0e] = Common::KEYCODE_BACKSPACE;
	_scancodeToKeycode[0x0f] = Common::KEYCODE_TAB;
	_scancodeToKeycode[0x1c] = Common::KEYCODE_RETURN;
	_scancodeToKeycode[0x39] = Common::KEYCODE_SPACE;
	_scancodeToKeycode[0x3b] = Common::KEYCODE_F1;
	_scancodeToKeycode[0x3c] = Common::KEYCODE_F2;
	_scancodeToKeycode[0x3d] = Common::KEYCODE_F3;
	_scancodeToKeycode[0x3e] = Common::KEYCODE_F4;
	_scancodeToKeycode[0x3f] = Common::KEYCODE_F5;
	_scancodeToKeycode[0x40] = Common::KEYCODE_F6;
	_scancodeToKeycode[0x41] = Common::KEYCODE_F7;
	_scancodeToKeycode[0x42] = Common::KEYCODE_F8;
	_scancodeToKeycode[0x43] = Common::KEYCODE_F9;
	_scancodeToKeycode[0x44] = Common::KEYCODE_F10;
	_scancodeToKeycode[0x45] = Common::KEYCODE_PAGEUP;	// Eiffel only
	_scancodeToKeycode[0x46] = Common::KEYCODE_PAGEDOWN;	// Eiffel only
	_scancodeToKeycode[0x47] = Common::KEYCODE_HOME;
	_scancodeToKeycode[0x48] = Common::KEYCODE_UP;
	_scancodeToKeycode[0x4a] = Common::KEYCODE_KP_MINUS;
	_scancodeToKeycode[0x4b] = Common::KEYCODE_LEFT;
	_scancodeToKeycode[0x4c] = Common::KEYCODE_LMETA;
	_scancodeToKeycode[0x4d] = Common::KEYCODE_RIGHT;
	_scancodeToKeycode[0x4e] = Common::KEYCODE_KP_PLUS;
	_scancodeToKeycode[0x4f] = Common::KEYCODE_PAUSE;	// Eiffel only
	_scancodeToKeycode[0x50] = Common::KEYCODE_DOWN;
	_scancodeToKeycode[0x52] = Common::KEYCODE_INSERT;
	_scancodeToKeycode[0x53] = Common::KEYCODE_DELETE;
	_scancodeToKeycode[0x55] = Common::KEYCODE_END;	// Eiffel only
	_scancodeToKeycode[0x5b] = Common::KEYCODE_TILDE;	// Eiffel only
	_scancodeToKeycode[0x61] = Common::KEYCODE_F12;	// UNDO
	_scancodeToKeycode[0x62] = Common::KEYCODE_F11;	// HELP
	_scancodeToKeycode[0x63] = Common::KEYCODE_SLASH;	// KEYPAD /
	_scancodeToKeycode[0x64] = Common::KEYCODE_KP_DIVIDE;
	_scancodeToKeycode[0x65] = Common::KEYCODE_KP_MULTIPLY;
	_scancodeToKeycode[0x66] = Common::KEYCODE_KP_MULTIPLY;	// duplicate?
	_scancodeToKeycode[0x67] = Common::KEYCODE_7;	// KEYPAD 7
	_scancodeToKeycode[0x68] = Common::KEYCODE_8;	// KEYPAD 8
	_scancodeToKeycode[0x69] = Common::KEYCODE_9;	// KEYPAD 9
	_scancodeToKeycode[0x6a] = Common::KEYCODE_4;	// KEYPAD 4
	_scancodeToKeycode[0x6b] = Common::KEYCODE_5;	// KEYPAD 5
	_scancodeToKeycode[0x6c] = Common::KEYCODE_6;	// KEYPAD 6
	_scancodeToKeycode[0x6d] = Common::KEYCODE_1;	// KEYPAD 1
	_scancodeToKeycode[0x6e] = Common::KEYCODE_2;	// KEYPAD 2
	_scancodeToKeycode[0x6f] = Common::KEYCODE_3;	// KEYPAD 3
	_scancodeToKeycode[0x70] = Common::KEYCODE_0;	// KEYPAD 0
	_scancodeToKeycode[0x71] = Common::KEYCODE_KP_PERIOD;
	_scancodeToKeycode[0x72] = Common::KEYCODE_KP_ENTER;
}

OSystem_Atari::~OSystem_Atari() {
	debug("OSystem_Atari::~OSystem_Atari()");

	if (_video_initialized) {
		Supexec(asm_screen_falcon_restore);
		_video_initialized = false;
	}

	if (_ikbd_initialized) {
		Supexec(atari_ikbd_shutdown);
		_ikbd_initialized = false;
	}
}

static void ikbd_and_video_restore() {
	Supexec(asm_screen_falcon_restore);
	Supexec(atari_ikbd_shutdown);
}

void OSystem_Atari::initBackend() {
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

	if (mch != MCH_FALCON && mch != MCH_ARANYM) {
		error("ScummVM works only on Atari Falcon and ARAnyM");
	}

	if (mch == MCH_ARANYM && Getcookie(C_fVDI, NULL) == C_FOUND) {
		logMessage(LogMessageType::kError, "Disable fVDI, ScummVM accesses Videl directly\n");
		quit();
	}

	nf_init();

	_startTime = clock();

	bool superVidel = VgetMonitor() == MON_VGA && Getcookie(C_SupV, NULL) == C_FOUND;

	_timerManager = new DefaultTimerManager();
	_eventManager = new DefaultEventManager(makeKeyboardRepeatingEventSource(this));
	_savefileManager = new DefaultSaveFileManager();
	if (superVidel)
        _atariGraphicsManager = new AtariSuperVidelManager();
	else
		_atariGraphicsManager = new AtariVidelManager();
	_graphicsManager = _atariGraphicsManager;
	_mixerManager = new AtariMixerManager();
	// Setup and start mixer
	_mixerManager->init();

	_KEYTAB *pKeyTables = (_KEYTAB*)Keytbl(KT_NOCHANGE, KT_NOCHANGE, KT_NOCHANGE);

	memcpy(_unshiftToAscii, pKeyTables->unshift, 128);
	memcpy(_shiftToAscii, pKeyTables->shift, 128);
	memcpy(_capsToAscii, pKeyTables->caps, 128);

	Supexec(atari_ikbd_init);
	_ikbd_initialized = true;

	Supexec(asm_screen_falcon_save);
	_video_initialized = true;

	Setexc(VEC_PROCTERM, ikbd_and_video_restore);

	BaseBackend::initBackend();
}

//! bit 0: rmb
//! bit 1: lmb
volatile uint8	g_atari_ikbd_mouse_buttons_state = 0;
volatile int16	g_atari_ikbd_mouse_delta_x = 0;
volatile int16	g_atari_ikbd_mouse_delta_y = 0;

#define SCANCODES_SIZE 256
volatile uint8	g_atari_ikbd_scancodes[SCANCODES_SIZE];
uint16			g_atari_ikbd_scancodes_mask = SCANCODES_SIZE-1;
volatile uint16	g_atari_ikbb_scancodes_head = 0;
static uint16	g_atari_ikbb_scancodes_tail = 0;

bool OSystem_Atari::pollEvent(Common::Event &event) {
	{
		static uint32 startMillis = getMillis();
		static uint32 oldMillis = getMillis();
		uint32 curMillis = getMillis();

		uint32 diff = curMillis - oldMillis;
		oldMillis = curMillis;

		if (diff > 0) {
			static float avgFpsSum;
			static int avgFpsCount;

			avgFpsSum += 1000.0f / diff;
			avgFpsCount++;

			if (curMillis - startMillis >= 1000) {
				float avgFps = avgFpsSum / avgFpsCount;
				debug("*** Average FPS in 1s: %f ***", avgFps);
				startMillis = curMillis;
				avgFpsSum = 0;
				avgFpsCount = 0;
			}
		}
	}

	((DefaultTimerManager *)_timerManager)->checkTimers();
	((AtariMixerManager *)_mixerManager)->update();

	if ((g_atari_ikbd_mouse_buttons_state & 0x01) && !_oldRmbDown) {
		event.type = Common::EVENT_RBUTTONDOWN;
		event.mouse = _atariGraphicsManager->getMousePosition();
		_oldRmbDown = true;
		return true;
	}

	if (!(g_atari_ikbd_mouse_buttons_state & 0x01) && _oldRmbDown) {
		event.type = Common::EVENT_RBUTTONUP;
		event.mouse = _atariGraphicsManager->getMousePosition();
		_oldRmbDown = false;
		return true;
	}

	if ((g_atari_ikbd_mouse_buttons_state & 0x02) && !_oldLmbDown) {
		event.type = Common::EVENT_LBUTTONDOWN;
		event.mouse = _atariGraphicsManager->getMousePosition();
		_oldLmbDown = true;
		return true;
	}

	if (!(g_atari_ikbd_mouse_buttons_state & 0x02) && _oldLmbDown) {
		event.type = Common::EVENT_LBUTTONUP;
		event.mouse = _atariGraphicsManager->getMousePosition();
		_oldLmbDown = false;
		return true;
	}

	if (g_atari_ikbd_mouse_delta_x != 0 || g_atari_ikbd_mouse_delta_y != 0) {
		const int deltaX = g_atari_ikbd_mouse_delta_x;
		const int deltaY = g_atari_ikbd_mouse_delta_y;

		g_atari_ikbd_mouse_delta_x = g_atari_ikbd_mouse_delta_y = 0;

		_atariGraphicsManager->updateMousePosition(deltaX, deltaY);

		event.type = Common::EVENT_MOUSEMOVE;
		event.mouse = _atariGraphicsManager->getMousePosition();
		event.relMouse = Common::Point(deltaX, deltaY);
		return true;
	}

	if (g_atari_ikbb_scancodes_head != g_atari_ikbb_scancodes_tail) {
		byte scancode = g_atari_ikbd_scancodes[g_atari_ikbb_scancodes_tail++];
		g_atari_ikbb_scancodes_tail &= SCANCODES_SIZE-1;

		bool pressed = !(scancode & 0x80);
		scancode &= 0x7f;

		if (scancode == 0x1d)
			_ctrlActive = pressed;

		if (scancode == 0x2a)
			_lshiftActive = pressed;

		if (scancode == 0x36)
			_rshiftActive = pressed;

		if (scancode == 0x38)
			_altActive = pressed;

		if (scancode == 0x3a && pressed)
			_capslockActive = !_capslockActive;

		// Eiffel only
		if (scancode == 0x59) {
			event.type = Common::EVENT_WHEELUP;
			event.mouse = _atariGraphicsManager->getMousePosition();
			return true;
		}

		// Eiffel only
		if (scancode == 0x5a) {
			event.type = Common::EVENT_WHEELDOWN;
			event.mouse = _atariGraphicsManager->getMousePosition();
			return true;
		}

		uint16 ascii;
		if (_lshiftActive || _rshiftActive) {
			ascii = _shiftToAscii[scancode];
		} else if (_capslockActive) {
			ascii = _capsToAscii[scancode];
		} else {
			ascii = _unshiftToAscii[scancode];
		}

		Common::KeyCode keycode = _scancodeToKeycode.getValOrDefault(scancode, Common::KEYCODE_INVALID);
		switch (keycode) {
		case Common::KEYCODE_BACKSPACE:
			ascii = Common::ASCII_BACKSPACE;
			break;
		case Common::KEYCODE_TAB:
			ascii = Common::ASCII_TAB;
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			ascii = Common::ASCII_RETURN;
			break;
		case Common::KEYCODE_ESCAPE:
			ascii = Common::ASCII_ESCAPE;
			break;
		case Common::KEYCODE_SPACE:
			ascii = Common::ASCII_SPACE;
			break;
		case Common::KEYCODE_F1:
			ascii = Common::ASCII_F1;
			break;
		case Common::KEYCODE_F2:
			ascii = Common::ASCII_F2;
			break;
		case Common::KEYCODE_F3:
			ascii = Common::ASCII_F3;
			break;
		case Common::KEYCODE_F4:
			ascii = Common::ASCII_F4;
			break;
		case Common::KEYCODE_F5:
			ascii = Common::ASCII_F5;
			break;
		case Common::KEYCODE_F6:
			ascii = Common::ASCII_F6;
			break;
		case Common::KEYCODE_F7:
			ascii = Common::ASCII_F7;
			break;
		case Common::KEYCODE_F8:
			ascii = Common::ASCII_F8;
			break;
		case Common::KEYCODE_F9:
			ascii = Common::ASCII_F9;
			break;
		case Common::KEYCODE_F10:
			ascii = Common::ASCII_F10;
			break;
		case Common::KEYCODE_F11:
			ascii = Common::ASCII_F11;
			break;
		case Common::KEYCODE_F12:
			ascii = Common::ASCII_F12;
			break;
		default:
			break;
		}

		if (ascii >= ' ' && ascii <= '~') {
			if (keycode == Common::KEYCODE_INVALID)
				keycode = _asciiToKeycode[ascii - ' '];
		}

		event.type = pressed ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
		event.kbd = Common::KeyState(keycode, ascii);
		event.kbd.flags |= _ctrlActive ? Common::KBD_CTRL : 0;
		event.kbd.flags |= _altActive ? Common::KBD_ALT : 0;
		event.kbd.flags |= (_lshiftActive || _rshiftActive) ? Common::KBD_SHIFT : 0;
		event.kbd.flags |= _capslockActive ? Common::KBD_CAPS : 0;

		return true;
	}

	return false;
}

Common::MutexInternal *OSystem_Atari::createMutex() {
	return new NullMutexInternal();
}

uint32 OSystem_Atari::getMillis(bool skipRecord) {
	// CLOCKS_PER_SEC is 200, so no need to use floats
	return 1000 * (clock() - _startTime) / CLOCKS_PER_SEC;
}

void OSystem_Atari::delayMillis(uint msecs) {
	usleep(msecs * 1000);
}

void OSystem_Atari::getTimeAndDate(TimeDate &td, bool skipRecord) const {
	debug("getTimeAndDate");
	time_t curTime = time(0);
	// TODO: if too slow (e.g. when calling RandomSource::RandomSource()), rewrite
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

	Common::Keymap *keymap = _atariGraphicsManager->getKeymap();
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
	Common::String str = Common::String::format("OSystem_Atari::quit()\n");
	logMessage(LogMessageType::kDebug, str.c_str());

	g_system->destroy();

	exit(0);
}

void OSystem_Atari::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);

	nf_print(message);
}

void OSystem_Atari::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
#ifdef DATA_PATH
	// Add the global DATA_PATH to the directory search list
	// See also OSystem_SDL::addSysArchivesToSearchSet()
	Common::FSNode dataNode(DATA_PATH);
	if (dataNode.exists() && dataNode.isDirectory()) {
		s.add(DATA_PATH, new Common::FSDirectory(dataNode, 4), priority);
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
