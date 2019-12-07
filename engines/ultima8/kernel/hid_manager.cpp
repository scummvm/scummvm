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

#include "ultima8/misc/pent_include.h"
#include "ultima8/kernel/hid_manager.h"
#include "ultima8/conf/setting_manager.h"
#include "ultima8/misc/console.h"
#include "ultima8/misc/util.h"
#include "ultima8/conf/config_file_manager.h" // temporary!
 //#include "SDL_timer.h"

namespace Ultima8 {

HIDManager *HIDManager::hidmanager = 0;

HIDManager::HIDManager() {
	con->Print(MM_INFO, "Creating HIDManager...\n");

	assert(hidmanager == 0);
	hidmanager = this;

	InitJoystick();

	resetBindings();
}

HIDManager::~HIDManager() {
	std::vector<Console::ArgvType *>::iterator it;
	con->Print(MM_INFO, "Destroying HIDManager...\n");

	for (it = commands.begin(); it != commands.end(); ++it) {
		if (*it) {
			delete *it;
		}
	}
	commands.clear();

	ShutdownJoystick();
	hidmanager = 0;
}

bool HIDManager::handleEvent(const HID_Key key, const HID_Event evn) {
	bool handled = false;

	if (key < HID_LAST && evn < HID_EVENT_LAST) {
		Console::ArgvType *command = bindings[key][evn];
		if (command) {
			con->ExecuteConsoleCommand(*command);
			handled = true;
		}
	}

	return handled;
}

void HIDManager::resetBindings() {
	uint16 key, event;
	std::vector<Console::ArgvType *>::iterator it;

	for (key = 0; key < HID_LAST; ++key) {
		for (event = 0; event < HID_EVENT_LAST; ++event) {
			bindings[key][event] = 0;
		}
	}

	for (it = commands.begin(); it != commands.end(); ++it) {
		if (*it) {
			delete *it;
		}
	}
	commands.clear();

	bind(HID_BACKQUOTE, HID_EVENT_PREEMPT, "ConsoleGump::toggle");
	bind(HID_TILDE, HID_EVENT_PREEMPT, "ConsoleGump::toggle");
	bind(HID_F5, HID_EVENT_PREEMPT, "ConsoleGump::toggle");
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
	uint16 key, event;
	SettingManager *settings = SettingManager::get_instance();
	Pentagram::istring section = "keys/";
	Pentagram::istring confkey;

	for (key = 0; key < HID_LAST; ++key) {
		for (event = 0; event < HID_EVENT_LAST; ++event) {
			confkey = section +
			          HID_GetEventName((HID_Event) event) + ' ' +
			          HID_GetKeyName((HID_Key) key);
			if (bindings[key][event]) {
				Console::ArgsType command;
				Pentagram::ArgvToString(*bindings[key][event], command);
				settings->set(confkey, command);
			} else if (settings->exists(confkey)) {
				settings->unset(confkey);
			}
		}
	}
}

void HIDManager::bind(const Pentagram::istring &control, const Console::ArgvType &argv) {
	HID_Key key = HID_LAST;
	HID_Event event = HID_EVENT_DEPRESS;
	std::vector<Pentagram::istring> ctrl_argv;

	Pentagram::StringToArgv(control, ctrl_argv);
	if (ctrl_argv.size() == 1) {
		key = HID_GetKeyFromName(ctrl_argv[0]);
	} else if (ctrl_argv.size() > 1) {
		// we have a event
		event = HID_GetEventFromName(ctrl_argv[0]);
		key = HID_GetKeyFromName(ctrl_argv[1]);
	}

	if (event < HID_EVENT_LAST && key < HID_LAST) {
		bind(key, event, argv);
	} else {
		pout << "Error: Cannot bind " << control << std::endl;
	}
}

void HIDManager::bind(const Pentagram::istring &control, const Console::ArgsType &args) {
	Console::ArgvType argv;
	Pentagram::StringToArgv(args, argv);
	bind(control, argv);
}

void HIDManager::bind(HID_Key key, HID_Event event, const Console::ArgvType &argv) {
	Console::ArgvType *command = 0;
	if (! argv.empty()) {
		std::vector<Console::ArgvType *>::iterator it;
		for (it = commands.begin(); it != commands.end(); ++it) {
			if (argv == (**it)) {
				// Change from iterator to pointer
				command = *it;
				break;
			}
		}

		if (!command) {
			command = new Console::ArgvType(argv);
			commands.push_back(command);
		}
	}
	bindings[key][event] = command;
}

void HIDManager::bind(HID_Key key, HID_Event event, const Console::ArgsType &args) {
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
			pout << "Usage: " << argv[0] << " <key> <action> [<arg> ...]: binds a key or button to an action" << std::endl;
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
			pout << "Usage: " << argv[0] << " <key>: unbinds a key or button" << std::endl;
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

	for (key = 0; key < HID_LAST; ++key) {
		for (event = 0; event < HID_EVENT_LAST; ++event) {
			if (bindings[key][event]) {
				Pentagram::ArgvToString(*bindings[key][event], command);
				if (event != HID_EVENT_DEPRESS) {
					pout << HID_GetEventName((HID_Event) event) << ' ' <<
					     HID_GetKeyName((HID_Key) key) <<
					     " = " << command << std::endl;
				} else {
					pout << HID_GetKeyName((HID_Key) key) <<
					     " = " << command << std::endl;
				}
			}
		}
	}
}

} // End of namespace Ultima8
