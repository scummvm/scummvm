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

#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "graphics/mactoolbox/toolbox.h"

namespace Graphics {
namespace MacToolbox {

static const struct MacKeyCodeMapping {
	Common::KeyCode scummvm;
	int mac;
} MackeyCodeMappings[] = {
	{ Common::KEYCODE_ESCAPE,		53 },
	{ Common::KEYCODE_F1,			122 },
	{ Common::KEYCODE_F2,			120 },
	{ Common::KEYCODE_F3,			99 },
	{ Common::KEYCODE_F4,			118 },
	{ Common::KEYCODE_F5,			96 },
	{ Common::KEYCODE_F6,			97 },
	{ Common::KEYCODE_F7,			98 },
	{ Common::KEYCODE_F8,			100 },
	{ Common::KEYCODE_F9,			101 },
	{ Common::KEYCODE_F10,			109 },
	{ Common::KEYCODE_F11,			103 },
	{ Common::KEYCODE_F12,			111 },
	{ Common::KEYCODE_F13,			105 }, // mirrored by print
	{ Common::KEYCODE_F14,			107 }, // mirrored by scroll lock
	{ Common::KEYCODE_F15,			113 }, // mirrored by pause

	{ Common::KEYCODE_BACKQUOTE,	10 },
	{ Common::KEYCODE_1,			18 },
	{ Common::KEYCODE_2,			19 },
	{ Common::KEYCODE_3,			20 },
	{ Common::KEYCODE_4,			21 },
	{ Common::KEYCODE_5,			23 },
	{ Common::KEYCODE_6,			22 },
	{ Common::KEYCODE_7,			26 },
	{ Common::KEYCODE_8,			28 },
	{ Common::KEYCODE_9,			25 },
	{ Common::KEYCODE_0,			29 },
	{ Common::KEYCODE_MINUS,		27 },
	{ Common::KEYCODE_EQUALS,		24 },
	{ Common::KEYCODE_BACKSPACE,	51 },

	{ Common::KEYCODE_TAB,			48 },
	{ Common::KEYCODE_q,			12 },
	{ Common::KEYCODE_w,			13 },
	{ Common::KEYCODE_e,			14 },
	{ Common::KEYCODE_r,			15 },
	{ Common::KEYCODE_t,			17 },
	{ Common::KEYCODE_y,			16 },
	{ Common::KEYCODE_u,			32 },
	{ Common::KEYCODE_i,			34 },
	{ Common::KEYCODE_o,			31 },
	{ Common::KEYCODE_p,			35 },
	{ Common::KEYCODE_LEFTBRACKET,	33 },
	{ Common::KEYCODE_RIGHTBRACKET,	30 },
	{ Common::KEYCODE_BACKSLASH,	42 },

	{ Common::KEYCODE_CAPSLOCK,		57 },
	{ Common::KEYCODE_a,			0 },
	{ Common::KEYCODE_s,			1 },
	{ Common::KEYCODE_d,			2 },
	{ Common::KEYCODE_f,			3 },
	{ Common::KEYCODE_g,			5 },
	{ Common::KEYCODE_h,			4 },
	{ Common::KEYCODE_j,			38 },
	{ Common::KEYCODE_k,			40 },
	{ Common::KEYCODE_l,			37 },
	{ Common::KEYCODE_SEMICOLON,	41 },
	{ Common::KEYCODE_QUOTE,		39 },
	{ Common::KEYCODE_RETURN,		36 },

	{ Common::KEYCODE_LSHIFT,		56 },
	{ Common::KEYCODE_z,			6 },
	{ Common::KEYCODE_x,			7 },
	{ Common::KEYCODE_c,			8 },
	{ Common::KEYCODE_v,			9 },
	{ Common::KEYCODE_b,			11 },
	{ Common::KEYCODE_n,			45 },
	{ Common::KEYCODE_m,			46 },
	{ Common::KEYCODE_COMMA,		43 },
	{ Common::KEYCODE_PERIOD,		47 },
	{ Common::KEYCODE_SLASH,		44 },
	{ Common::KEYCODE_RSHIFT,		56 },

	{ Common::KEYCODE_LCTRL,		54 }, // control
	{ Common::KEYCODE_LALT,			58 }, // option
	{ Common::KEYCODE_LSUPER,		55 }, // command
	{ Common::KEYCODE_SPACE,		49 },
	{ Common::KEYCODE_RSUPER,		55 }, // command
	{ Common::KEYCODE_RALT,			58 }, // option
	{ Common::KEYCODE_RCTRL,		54 }, // control

	{ Common::KEYCODE_LEFT,			123 },
	{ Common::KEYCODE_RIGHT,		124 },
	{ Common::KEYCODE_DOWN,			125 },
	{ Common::KEYCODE_UP,			126 },

	{ Common::KEYCODE_NUMLOCK,		71 },
	{ Common::KEYCODE_KP_EQUALS,	81 },
	{ Common::KEYCODE_KP_DIVIDE,	75 },
	{ Common::KEYCODE_KP_MULTIPLY,	67 },

	{ Common::KEYCODE_KP7,			89 },
	{ Common::KEYCODE_KP8,			91 },
	{ Common::KEYCODE_KP9,			92 },
	{ Common::KEYCODE_KP_MINUS,		78 },

	{ Common::KEYCODE_KP4,			86 },
	{ Common::KEYCODE_KP5,			87 },
	{ Common::KEYCODE_KP6,			88 },
	{ Common::KEYCODE_KP_PLUS,		69 },

	{ Common::KEYCODE_KP1,			83 },
	{ Common::KEYCODE_KP2,			84 },
	{ Common::KEYCODE_KP3,			85 },

	{ Common::KEYCODE_KP0,			82 },
	{ Common::KEYCODE_KP_PERIOD,	65 },
	{ Common::KEYCODE_KP_ENTER,		76 },

	{ Common::KEYCODE_MENU,			50 }, // international
	{ Common::KEYCODE_PRINT,		105 }, // mirrored by F13
	{ Common::KEYCODE_SCROLLOCK,	107 }, // mirrored by F14
	{ Common::KEYCODE_PAUSE,		113 }, // mirrored by F15
	{ Common::KEYCODE_INSERT,		114 },
	{ Common::KEYCODE_HOME,			115 },
	{ Common::KEYCODE_PAGEUP,		116 },
	{ Common::KEYCODE_DELETE,		117 },
	{ Common::KEYCODE_END,			119 },
	{ Common::KEYCODE_PAGEDOWN,		121 },

	{ Common::KEYCODE_INVALID,		0 }
};



Toolbox::Toolbox(Graphics::MacWindowManager *wm) : _wm(wm) {
	_fileModalCallback = nullptr;
	_frameLimiter = new Graphics::FrameLimiter(g_system, 60, false);
	for (const MacKeyCodeMapping *k = MackeyCodeMappings; k->scummvm != Common::KEYCODE_INVALID; k++) {
		_keyMap[k->scummvm] = k->mac;
	}
	Audio::Mixer *mixer = g_system->getMixer();
	if (mixer) {
		_swSynth = new Audio::PCSpeakerStream(mixer->getOutputRate());
		mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundDriver,
			              _swSynth, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	}

}

Toolbox::~Toolbox() {
	delete _frameLimiter;
	Audio::Mixer *mixer = g_system->getMixer();
	if (mixer) {
		mixer->stopHandle(_soundDriver);
	}
	delete _swSynth;
	_swSynth = nullptr;
}


void Toolbox::_pumpEvents() {
	Common::Event event;
	Common::EventManager *manager = g_system->getEventManager();
	while (manager->pollEvent(event)) {
		EventRecord newRecord;
		newRecord.when = TickCount();
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mouse = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_mouse = event.mouse;
			_modifiers &= ~kModMouseButtonUp;
			newRecord.what = kMouseDown;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mouse = event.mouse;
			_modifiers |= kModMouseButtonUp;
			newRecord.what = kMouseUp;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_KEYDOWN:
			newRecord.what = kKeyDown;
			newRecord.where = _mouse;
			newRecord.message = (_keyMap.getValOrDefault(event.kbd.keycode, 0) << 8);
			newRecord.message |= event.kbd.ascii;
			if (event.kbd.flags & Common::KBD_META) {
				_modifiers |= kModCommandKeyDown;
			} else {
				_modifiers &= ~kModCommandKeyDown;
			}
			if (event.kbd.flags & Common::KBD_ALT) {
				_modifiers |= kModLOptionKeyDown;
			} else {
				_modifiers &= ~kModLOptionKeyDown;
			}
			if (event.kbd.flags & Common::KBD_CTRL) {
				_modifiers |= kModLControlKeyDown;
			} else {
				_modifiers &= ~kModLControlKeyDown;
			}
			if (event.kbd.flags & Common::KBD_SHIFT) {
				_modifiers |= kModLShiftKeyDown;
			} else {
				_modifiers &= ~kModLShiftKeyDown;
			}
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_KEYUP:
			newRecord.what = kKeyUp;
			newRecord.where = _mouse;
			newRecord.message = (_keyMap.getValOrDefault(event.kbd.keycode, 0) << 8);
			newRecord.message |= event.kbd.ascii;
			if (event.kbd.flags & Common::KBD_META) {
				_modifiers |= kModCommandKeyDown;
			} else {
				_modifiers &= ~kModCommandKeyDown;
			}
			if (event.kbd.flags & Common::KBD_ALT) {
				_modifiers |= kModLOptionKeyDown;
			} else {
				_modifiers &= ~kModLOptionKeyDown;
			}
			if (event.kbd.flags & Common::KBD_CTRL) {
				_modifiers |= kModLControlKeyDown;
			} else {
				_modifiers &= ~kModLControlKeyDown;
			}
			if (event.kbd.flags & Common::KBD_SHIFT) {
				_modifiers |= kModLShiftKeyDown;
			} else {
				_modifiers &= ~kModLShiftKeyDown;
			}
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_QUIT:
			manager->resetQuit();
			newRecord.what = kScummVMQuitEvt;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_RETURN_TO_LAUNCHER:
			manager->resetReturnToLauncher();
			newRecord.what = kScummVMReturnToLauncherEvt;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		default:
			break;
		}
	}
	// erase the older events if they aren't consumed in time
	while (_events.size() > 2048) {
		_events.pop_front();
	}
}

void Toolbox::_updateScreen() {
	_wm->draw();
	_frameLimiter->delayBeforeSwap();
	g_system->updateScreen();
	_frameLimiter->startFrame();
}

uint32 Toolbox::Delay(uint32 numTicks) {
	uint32 target = g_system->getMillis() + (numTicks * 1000 / 60);
	int updateCount = 0;
	do  {
		_pumpEvents();
		_updateScreen();
		updateCount++;
	} while (g_system->getMillis() < target);
	if (debugChannelSet(8, kDebugLevelMacToolbox)) {
		debugC(8, kDebugLevelMacToolbox, "Toolbox::Delay: %d screen updates in %d ticks", updateCount, numTicks);
	}
	return (uint32)(g_system->getMillis() * 60 / 1000);
}

void Toolbox::FlushEvents(uint32 eventMask, uint32 stopMask) {
	if ((eventMask == 0xffffffff) && (stopMask == 0)) {
		_events.clear();
		//_modifiers = kModMouseButtonUp;
		return;
	}
	for (auto it = _events.begin(); it != _events.end(); ) {
		if ((1 << it->what) & stopMask) {
			return;
		} else if ((1 << it->what) & eventMask) {
			it = _events.erase(it);
		} else {
			++it;
		}
	}
	//if (eventMask & 0x6) {
	//	  _modifiers = kModMouseButtonUp;
	//}
}

bool Toolbox::GetNextEvent(int32 eventMask, EventRecord &theEvent) {
	_pumpEvents();
	if (!_events.empty() && eventMask) {
		for (auto it = _events.begin(); it != _events.end(); ++it) {
			if ((1 << it->what) & eventMask) {
				theEvent = Common::move(*it);
				_events.erase(it);
				return true;
			}
		}
	}
	theEvent.what = kNullEvent;
	theEvent.where = _mouse;
	theEvent.modifiers = _modifiers;
	return false;
}

uint32 Toolbox::TickCount() {
	return (uint32)(g_system->getMillis() * 60 / 1000);
}


} // End of namespace MacToolbox
} // End of namespace Graphics
