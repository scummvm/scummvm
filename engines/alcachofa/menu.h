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

#ifndef ALCACHOFA_MENU_H
#define ALCACHOFA_MENU_H

#include "common/savefile.h"

namespace Alcachofa {

class Room;

enum class MainMenuAction : int32 {
	ContinueGame = 0,
	Save,
	Load,
	InternetMenu,
	OptionsMenu,
	Exit,
	NextSave,
	PrevSave,
	NewGame,
	AlsoExit // there seems to be no difference to Exit
};

enum class OptionsMenuAction : int32 {
	SubtitlesOn = 0,
	SubtitlesOff,
	HighQuality,
	LowQuality,
	Bits32,
	Bits16,
	MainMenu
};

enum class OptionsMenuValue : int32 {
	Music = 0,
	Speech = 1
};

class Menu {
public:
	Menu();

	inline bool isOpen() const { return _isOpen; }
	inline uint32 millisBeforeMenu() const { return _millisBeforeMenu; }
	inline Room *previousRoom() { return _previousRoom; }
	inline FakeSemaphore &interactionSemaphore() { return _interactionSemaphore; }

	void resetAfterLoad();
	void updateOpeningMenu();
	void triggerMainMenuAction(MainMenuAction action);
	void triggerLoad();

	void openOptionsMenu();
	void triggerOptionsAction(OptionsMenuAction action);
	void triggerOptionsValue(OptionsMenuValue valueId, float value);

	// if we do still have a big thumbnail, any autosaves, ScummVM-saves, ingame-saves
	// do not have to render themselves, they can just reuse the one we have.
	// as such - may return nullptr
	const Graphics::Surface *getBigThumbnail() const;

private:
	void triggerSave();
	void updateSelectedSavefile(bool hasJustSaved);
	bool tryReadOldSavefile();
	void continueGame();
	void continueMainMenu();
	void setOptionsState();

	bool
		_isOpen = false,
		_openAtNextFrame = false;
	uint32
		_millisBeforeMenu = 0,
		_selectedSavefileI = 0;
	Room *_previousRoom = nullptr;
	FakeSemaphore _interactionSemaphore; // to prevent ScummVM loading during button clicks
	Common::String _selectedSavefileDescription = "<unset>";
	Common::Array<Common::String> _savefiles;
	Graphics::ManagedSurface
		_bigThumbnail, // big because it is for the in-game menu, not for ScummVM
		_selectedThumbnail;
	Common::SaveFileManager *_saveFileMgr;
};

}

#endif // ALCACHOFA_MENU_H
