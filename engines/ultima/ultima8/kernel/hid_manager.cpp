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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/hid_manager.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/misc/console.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/conf/config_file_manager.h" // temporary!
 //#include "SDL_timer.h"

namespace Ultima {
namespace Ultima8 {

HIDManager *HIDManager::_hidManager = 0;

HIDManager::HIDManager() {
	_hidManager = this;
	con->Print(MM_INFO, "Creating HIDManager...\n");

	InitJoystick();
	resetBindings();
}

HIDManager::~HIDManager() {
	Std::vector<Console::ArgvType *>::iterator it;
	con->Print(MM_INFO, "Destroying HIDManager...\n");

	for (it = _commands.begin(); it != _commands.end(); ++it) {
		if (*it) {
			delete *it;
		}
	}
	_commands.clear();

	ShutdownJoystick();
	_hidManager = 0;
}

bool HIDManager::handleEvent(HID_Key key, HID_Events events) {
	bool handled = false;
	uint32 keyEvent = (uint32)key | ((uint32)events << 16);

 	if (_bindings.contains(keyEvent)) {
		con->ExecuteConsoleCommand(*_bindings[keyEvent]);
		handled = true;
	}

	return handled;
}

void HIDManager::resetBindings() {
	Std::vector<Console::ArgvType *>::iterator it;

	_bindings.clear();

	for (it = _commands.begin(); it != _commands.end(); ++it) {
		if (*it) {
			delete *it;
		}
	}
	_commands.clear();

	bind(HID_BACKQUOTE, HID_EVENT_PREEMPT, "ConsoleGump::toggle");
	bind(HID_TILDE, HID_EVENT_PREEMPT, "ConsoleGump::toggle");
}

void HIDManager::loadBindings() {
	Console::ArgsType args;

	con->Print(MM_INFO, "Loading HIDBindings...\n");

	SettingManager *settings = SettingManager::get_instance();
	KeyMap keys;
	keys = settings->listDataValues("keys");

	KeyMap::iterator i = keys.begin();
	KeyMap::iterator end = keys.end();

	if (i == end) {
		con->Print(MM_INFO, "Loading default HIDBindings...\n");
		ConfigFileManager *config = ConfigFileManager::get_instance();
		keys = config->listKeyValues("bindings/bindings");
		i = keys.begin();
		end = keys.end();
	}

	while (i != keys.end()) {
		args = i->_value.c_str();
		bind(i->_key, args);
		++i;
	}

	listBindings();
}

void HIDManager::saveBindings() {
	uint16 key, events;
	SettingManager *settings = SettingManager::get_instance();
	Pentagram::istring section = "keys/";
	Pentagram::istring confkey;

	for (Bindings::iterator it = _bindings.begin(); it != _bindings.end(); ++it) {
		key = it->_key & 0xffff;
		events = it->_key >> 16;

		confkey = section + HID_GetEventsName((HID_Events)events) + ' ' +
			HID_GetKeyName((HID_Key) key);

		Console::ArgsType command;
		Pentagram::ArgvToString(*(it->_value), command);
		settings->set(confkey, command);
//		settings->unset(confkey);
	}
}

void HIDManager::bind(const Pentagram::istring &control, const Console::ArgvType &argv) {
	HID_Key key = HID_LAST;
	HID_Events event = HID_EVENT_DEPRESS;
	Std::vector<Pentagram::istring> ctrl_argv;

	Pentagram::StringToArgv(control, ctrl_argv);
	if (ctrl_argv.size() == 1) {
		key = HID_GetKeyFromName(ctrl_argv[0]);
	} else if (ctrl_argv.size() > 1) {
		// we have a event
		key = HID_GetKeyFromName(ctrl_argv[1]);
		event = HID_GetEventFromName(ctrl_argv[0]);
	}

	bind(key, event, argv);
}

void HIDManager::bind(const Pentagram::istring &control, const Console::ArgsType &args) {
	Console::ArgvType argv;
	Pentagram::StringToArgv(args, argv);
	bind(control, argv);
}

void HIDManager::bind(HID_Key key, HID_Events event, const Console::ArgvType &argv) {
	uint32 keyEvent = (uint32)key | ((uint32)event << 16);

	Console::ArgvType *command = 0;
	if (! argv.empty()) {
		Std::vector<Console::ArgvType *>::iterator it;
		for (it = _commands.begin(); it != _commands.end(); ++it) {
			if (argv == (**it)) {
				// Change from iterator to pointer
				command = *it;
				break;
			}
		}

		if (!command) {
			command = new Console::ArgvType(argv);
			_commands.push_back(command);
		}
	}

	assert(command);
	_bindings[keyEvent] = command;
}

void HIDManager::bind(HID_Key key, HID_Events event, const Console::ArgsType &args) {
	Console::ArgvType argv;
	Pentagram::StringToArgv(args, argv);
	bind(key, event, argv);
}

void HIDManager::unbind(const Pentagram::istring &control) {
	// bind to an empty control
	Console::ArgvType command;
	bind(control, command);
}

void HIDManager::ConCmd_bind(const Console::ArgvType &argv) {
	Console::ArgvType argv2;
	Console::ArgvType::const_iterator it;
	if (argv.size() < 3) {
		if (! argv.empty())
			pout << "Usage: " << argv[0] << " <key> <action> [<arg> ...]: binds a key or button to an action" << Std::endl;
		return;
	}
	HIDManager *hid = HIDManager::get_instance();

	Pentagram::istring control(argv[1]);

	it = argv.begin();
	++it;
	++it;
	argv2.assign(it, argv.end());

	hid->bind(control, argv2);
}

void HIDManager::ConCmd_unbind(const Console::ArgvType &argv) {
	if (argv.size() != 2) {
		if (! argv.empty())
			pout << "Usage: " << argv[0] << " <key>: unbinds a key or button" << Std::endl;
		return;
	}
	HIDManager *hid = HIDManager::get_instance();

	Pentagram::istring control(argv[1]);

	hid->unbind(control);
}

void HIDManager::ConCmd_listbinds(const Console::ArgvType &argv) {
	HIDManager *hid = HIDManager::get_instance();
	hid->listBindings();
}

void HIDManager::ConCmd_save(const Console::ArgvType &argv) {
	HIDManager *hid = HIDManager::get_instance();
	hid->saveBindings();

	SettingManager *settings = SettingManager::get_instance();
	settings->write();
}

void HIDManager::listBindings() {
	uint16 key, event;
	Console::ArgsType command;

	for (Bindings::iterator it = _bindings.begin(); it != _bindings.end(); ++it) {
		key = it->_key & 0xffff;
		event = it->_key >> 16;
		Pentagram::ArgvToString(*(it->_value), command);

		if (event == HID_EVENT_DEPRESS) {
			pout << HID_GetKeyName((HID_Key)key) << " = " << command << Std::endl;
		} else {
			pout << HID_GetEventsName((HID_Events)event);
			pout << ' ' << HID_GetKeyName((HID_Key) key) << " = " << command << Std::endl;
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
