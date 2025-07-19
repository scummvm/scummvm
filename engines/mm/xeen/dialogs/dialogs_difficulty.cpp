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

#include "mm/xeen/dialogs/dialogs_difficulty.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

#ifdef USE_TTS

static const char *difficultyButtonsText[] = {
	"Adventurer\nWarrior",													// English
	"Abenteurer\nK\204mpfer",												// German
	"Aventurier\nGuerrier",													// French
	"Aventurero\nGuerrero",													// Spanish
	"\x80\xa2\xa0\xad\xe2\xee\xe0\xa8\xe1\n\x82\xae\xa8\xad",				// Russian (Авантюрис, Воин)
	"\xab\x5f\xc0\x49\xbc\xd2\xa6\xa1\n\xbe\xd4\xa4\x68\xbc\xd2\xa6\xa1"	// Chinese (冒險模式, 戰士模式)
};

#endif

enum DifficultyButtonTTSTextIndex {
	kDifficultyAdventurer = 0,
	kDifficultyWarrior = 1
};

int DifficultyDialog::show(XeenEngine *vm) {
	DifficultyDialog *dlg = new DifficultyDialog(vm);
	int result = dlg->execute();
	delete dlg;

	return result;
}

DifficultyDialog::DifficultyDialog(XeenEngine *vm) : ButtonContainer(vm) {
	loadButtons();
#ifdef USE_TTS
	setButtonTexts(difficultyButtonsText[_vm->_ttsLanguage]);
#endif
}

int DifficultyDialog::execute() {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;

	Window &w = windows[6];
	w.open();
#ifdef USE_TTS
	_vm->stopTextToSpeech();
#endif
	w.writeString(Res.DIFFICULTY_TEXT);
	drawButtons(&w);

	int result = -1;
	while (!_vm->shouldExit()) {
		events.pollEventsAndWait();
		checkEvents(_vm);

		if (Res.KeyConstants.DialogsDifficulty.KEY_ADVENTURER == _buttonValue)
			result = ADVENTURER;
		else if (Res.KeyConstants.DialogsDifficulty.KEY_WARRIOR == _buttonValue)
			result = WARRIOR;
		else if (Common::KEYCODE_ESCAPE != _buttonValue)
			continue;

		break;
	}

	w.close();
	return result;
}

void DifficultyDialog::loadButtons() {
	_sprites.load("choice.icn");

	addButton(Common::Rect(68, 167, 158, 187),  Res.KeyConstants.DialogsDifficulty.KEY_ADVENTURER, &_sprites, kDifficultyAdventurer);
	addButton(Common::Rect(166, 167, 256, 187), Res.KeyConstants.DialogsDifficulty.KEY_WARRIOR, &_sprites, kDifficultyWarrior);
}

} // End of namespace Xeen
} // End of namespace MM
