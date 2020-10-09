/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/services/gamemessage.h"

#include "common/file.h"
#include "common/ini-file.h"

#include "gui/message.h"

namespace Stark {

GameMessage::GameMessage() :
		_texts() {
	Common::File tmp;
	if (!tmp.open("language.ini")) {
		warning("Unable to open 'language.ini'");
		return;
	}

	// Skip the head of the language.ini that is in an unsupported format
	Common::String line;
	while (!tmp.eos() && !tmp.err()) {
		line = tmp.readLine();
		if (line.size() > 2 && line[0] == '-' && line[1] == '-') {
			break;
		}
	}

	Common::INIFile file;
	if(file.loadFromStream(tmp)) {
		Common::String section = "Language";
		Common::String key, text;

		for (uint i = 2; i <= 400; ++i) {
			key = Common::String::format("%03d", i);
			if (file.hasKey(key, section)) {
				file.getKey(key, section, text);
				_texts[i] = text;
			}
		}
	}

	// Pre-process some of the texts
	if (_texts.contains(kYes)) {
		Common::replace(_texts[kYes], "&", "");
	}
	if (_texts.contains(kNo)) {
		Common::replace(_texts[kNo], "&", "");
	}
	if (_texts.contains(kOverwriteSave)) {
		Common::replace(_texts[kOverwriteSave], "\\n", "\n");
	}
}

Common::String GameMessage::getDefaultText(TextKey key) {
	switch(key) {
		case kOverwriteSave:
			return "Are you sure you want to overwrite the savegame:\n'%s' ?";
		case kEndAndLoad:
			return "Are you sure you want to end your current game and load a new one ?";
		case kInventory:
			return "Inventory";
		case kOptions:
			return "Options";
		case kQuit:
			return "Quit";
		case kQuitGamePrompt:
			return "Are you sure you want to quit this game ?";
		case kQuitPrompt:
			return "Are you sure you want to quit ?";
		case kYes:
			return "Yes";
		case kNo:
			return "No";
		default:
			return "Unimplemented message text";
	}
}

} // End of namespace Stark
