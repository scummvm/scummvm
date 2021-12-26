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

#include "xeen/dialogs/dialogs_control_panel.h"
#include "xeen/dialogs/dialogs_query.h"
#include "xeen/party.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

int ControlPanel::show(XeenEngine *vm) {
	ControlPanel *dlg = new ControlPanel(vm);
	int result = dlg->execute();
	delete dlg;

	return result;
}

int ControlPanel::execute() {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	SavesManager &saves = *_vm->_saves;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w = windows[23];
	Window &w3 = windows[3];

	loadButtons();

	int result = 0, debugCtr = 0;
	w.open();

	do {
		Common::String btnText = getButtonText();
		Common::String text = Common::String::format(Res.CONTROL_PANEL_TEXT, btnText.c_str());

		drawButtons(&w);
		w.writeString(text);
		w.writeString("\xB""000\t000\x1");
		w.update();

		events.updateGameCounter();
		intf.draw3d(false, false);

		do {
			w.writeString("\r");
			drawButtons(&w);
			w.writeString(text);
			w.writeString("\v000\t000");
			w.frame();

			if (_debugFlag)
				w.writeString(getTimeText());

			w3.update();
			w.update();

			events.pollEventsAndWait();
			checkEvents(_vm);
			if (_vm->shouldExit())
				return 0;
		} while (!_buttonValue && events.timeElapsed() < 2);

		if (Res.KeyConstants.DialogsControlPanel.KEY_QUIT == _buttonValue) {
			if (Confirm::show(g_vm, Res.CONFIRM_QUIT)) {
				g_vm->_gameMode = GMODE_QUIT;
				result = 1;
			}

		} else if (Res.KeyConstants.DialogsControlPanel.KEY_MRWIZARD == _buttonValue) {
			if (Confirm::show(g_vm, Res.MR_WIZARD)) {
				w.close();
				if (!windows[2]._enabled) {
					sound.playFX(51);

					if (g_vm->getGameID() == GType_WorldOfXeen) {
						map._loadCcNum = 0;
						map.load(28);
						party._mazeDirection = DIR_EAST;
					} else {
						map._loadCcNum = 1;
						map.load(29);
						party._mazeDirection = DIR_SOUTH;
					}
					party.moveToRunLocation();
				}

				party._gems = 0;
				result = 2;
			}

		} else if (Res.KeyConstants.DialogsControlPanel.KEY_LOAD == _buttonValue) {
			if (_vm->_mode == MODE_COMBAT) {
				ErrorScroll::show(_vm, Res.NO_LOADING_IN_COMBAT);
			} else {
				// Close dialog and show loading dialog
				result = 3;
			}

		} else if (Res.KeyConstants.DialogsControlPanel.KEY_SAVE == _buttonValue) {
			if (_vm->_mode == MODE_COMBAT) {
				ErrorScroll::show(_vm, Res.NO_SAVING_IN_COMBAT);
			} else {
				// Close dialog and show saving dialog
				result = 4;
			}

		} else if (Res.KeyConstants.DialogsControlPanel.KEY_FXON == _buttonValue) {
			sound.setFxOn(!sound._fxOn);

		} else if (Res.KeyConstants.DialogsControlPanel.KEY_MUSICON == _buttonValue) {
			sound.setMusicOn(!sound._musicOn);

		} else if (Common::KEYCODE_ESCAPE == _buttonValue) {
			result = 1;

		} else if (Common::KEYCODE_g == _buttonValue) { // Goober cheat sequence
			debugCtr = 1;
		} else if (Common::KEYCODE_o == _buttonValue) {
			debugCtr = (debugCtr == 1 || debugCtr == 2) ? 2 : 0;
		} else if (Common::KEYCODE_b == _buttonValue) {
			debugCtr = (debugCtr == 2) ? 3 : 0;
		} else if (Common::KEYCODE_r == _buttonValue) {
			if (debugCtr == 3)
				_debugFlag = true;
			else
				debugCtr = 0;
		}

	} while (!result);

	w.close();
	intf.drawParty(true);

	if (result == 3) {
		if (g_vm->canLoadGameStateCurrently())
			saves.loadGame();
	} else if (result == 4) {
		saves.saveGame();
	}

	return result;
}

void ControlPanel::loadButtons() {
	_iconSprites.load("cpanel.icn");
	addButton(Common::Rect(214, 56, 244, 69), Res.KeyConstants.DialogsControlPanel.KEY_FXON, 0, &_iconSprites);
	addButton(Common::Rect(214, 75, 244, 88), Res.KeyConstants.DialogsControlPanel.KEY_MUSICON, 0, &_iconSprites);
	addButton(Common::Rect(135, 56, 165, 69), Res.KeyConstants.DialogsControlPanel.KEY_LOAD, 0, &_iconSprites);
	addButton(Common::Rect(135, 75, 165, 88), Res.KeyConstants.DialogsControlPanel.KEY_SAVE, 0, &_iconSprites);

	// For ScummVM we've merged both Save and Save As into a single
	// save item, so we don't need this one
	addButton(Common::Rect(), 0);

	addButton(Common::Rect(135, 94, 165, 107), Res.KeyConstants.DialogsControlPanel.KEY_QUIT, 0, &_iconSprites);
	addButton(Common::Rect(175, 113, 205, 126), Res.KeyConstants.DialogsControlPanel.KEY_MRWIZARD, 0, &_iconSprites);
}

Common::String ControlPanel::getButtonText() {
	Sound &sound = *g_vm->_sound;
	_btnSoundText = sound._fxOn ? Res.ON : Res.OFF;
	_btnMusicText = sound._musicOn ? Res.ON : Res.OFF;

	return Common::String::format(Res.CONTROL_PANEL_BUTTONS,
		_btnSoundText.c_str(), _btnMusicText.c_str());
}

Common::String ControlPanel::getTimeText() const {
	TimeDate td;
	g_system->getTimeAndDate(td);
	Common::String timeStr = Common::String::format("%d:%.2d:%.2d%c",
		td.tm_hour == 0 || td.tm_hour == 12 ? 12 : (td.tm_hour % 12),
		td.tm_min, td.tm_sec, (td.tm_hour >= 12) ? 'p' : 'c');

	uint32 playtime = g_vm->_events->playTime() / GAME_FRAME_RATE;
	Common::String playtimeStr = Common::String::format("%d:%.2d:%.2d",
		playtime / 3600, (playtime / 60) % 60, playtime % 60);
	return Common::String::format(
		"\x2\x3l\xB""000\t000\x4""160%s\x3r\xB""000\t000%s\x1",
		timeStr.c_str(), playtimeStr.c_str());
}

} // End of namespace Xeen
