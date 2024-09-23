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

#include "mm/xeen/dialogs/dialogs_info.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

void InfoDialog::show(XeenEngine *vm) {
	InfoDialog *dlg = new InfoDialog(vm);
	dlg->execute();
	delete dlg;
}

void InfoDialog::execute() {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;

	protectionText();
	Common::String statusText = "";
	for (uint idx = 0; idx < _lines.size(); ++idx)
		statusText += _lines[idx];

	Common::String gameName;
	if (_vm->getGameID() == GType_Swords)
		gameName = Res.SWORDS_GAME_TEXT;
	else if (_vm->getGameID() == GType_Clouds)
		gameName = Res.CLOUDS_GAME_TEXT;
	else if (_vm->getGameID() == GType_DarkSide)
		gameName = Res.DARKSIDE_GAME_TEXT;
	else
		gameName = Res.WORLD_GAME_TEXT;

	// Form the display message
	int hour = party._minutes / 60;
	Common::String details = Common::String::format(Res.GAME_INFORMATION,
		gameName.c_str(), Res.WEEK_DAY_STRINGS[party._day % 10],
		(hour > 12) ? hour - 12 : (!hour ? 12 : hour),
		party._minutes % 60, (hour > 11) ? 'p' : 'a',
		party._day, party._year, statusText.c_str());

	Window &w = windows[28];
	w.setBounds(Common::Rect(88, 20, 248, 112 + (_lines.empty() ? 0 : _lines.size() * 9 + 13)));
	w.open();
	w.writeString(details);

	do {
		events.updateGameCounter();
		intf.draw3d(false, false);
		w.frame();
		w.writeString(details);
		w.update();

		events.wait(1);
	} while (!_vm->shouldExit() && !events.isKeyMousePressed());

	events.clearEvents();
	w.close();
}

void InfoDialog::protectionText() {
	Party &party = *_vm->_party;
//	Common::StringArray _lines;
	const char *const AA_L024 = "\x3l\n\x9""024";
	const char *const AA_R124 = "\x3r\x9""124";

	if (party._lightCount) {
		_lines.push_back(Common::String::format(Res.LIGHT_COUNT_TEXT, party._lightCount));
	}

	if (party._fireResistance) {
		_lines.push_back(Common::String::format(Res.FIRE_RESISTANCE_TEXT,
			_lines.size() == 0 ? 10 : 1, AA_L024, AA_R124, party._fireResistance));
	}

	if (party._electricityResistance) {
		_lines.push_back(Common::String::format(Res.ELECRICITY_RESISTANCE_TEXT,
			_lines.size() == 0 ? 10 : 1, AA_L024, AA_R124, party._electricityResistance));
	}

	if (party._coldResistance) {
		_lines.push_back(Common::String::format(Res.COLD_RESISTANCE_TEXT,
			_lines.size() == 0 ? 10 : 1, AA_L024, AA_R124, party._coldResistance));
	}

	if (party._poisonResistance) {
		_lines.push_back(Common::String::format(Res.POISON_RESISTANCE_TEXT,
			_lines.size() == 0 ? 10 : 1, AA_L024, AA_R124, party._poisonResistance));
	}

	if (party._clairvoyanceActive) {
		_lines.push_back(Common::String::format(Res.CLAIRVOYANCE_TEXT,
			_lines.size() == 0 ? 10 : 1, AA_L024, AA_R124));
	}

	if (party._levitateCount) {
		_lines.push_back(Common::String::format(Res.LEVITATE_TEXT,
			_lines.size() == 0 ? 10 : 1, AA_L024, AA_R124));
	}

	if (party._walkOnWaterActive) {
		_lines.push_back(Common::String::format(Res.WALK_ON_WATER_TEXT,
			_lines.size() == 0 ? 10 : 1, AA_L024, AA_R124));
	}
}

} // End of namespace Xeen
} // End of namespace MM
