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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GOB_PREGOB_ONCEUPON_ONCEUPON_H
#define GOB_PREGOB_ONCEUPON_ONCEUPON_H

#include "common/system.h"

#include "gob/pregob/pregob.h"

namespace Gob {

class Surface;
class Font;

class ANIObject;

namespace OnceUpon {

class OnceUpon : public PreGob {
public:
	static const uint kLanguageCount = 5;

	OnceUpon(GobEngine *vm);
	~OnceUpon();

protected:
	enum MenuType {
		kMenuTypeMainStart  = 0, ///< The big main menu at game start.
		kMenuTypeMainIngame,     ///< The big main menu during the game.
		kMenuTypeIngame          ///< The small popup menu during the game.
	};

	enum MenuAction {
		kMenuActionNone = 0, ///< No action.
		kMenuActionAnimals , ///< Do the animal names.
		kMenuActionPlay    , ///< Play the game.
		kMenuActionRestart , ///< Restart the section.
		kMenuActionMainMenu, ///< Go to the main menu.
		kMenuActionQuit      ///< Quit the game.
	};

	enum Difficulty {
		kDifficultyBeginner     = 0,
		kDifficultyIntermediate = 1,
		kDifficultyAdvanced     = 2,
		kDifficultyMAX
	};

	enum Sound {
		kSoundClick = 0,
		kSoundMAX
	};

	struct MenuButton {
		bool needDraw;
		int16 left, top, right, bottom;
		int16 srcLeft, srcTop, srcRight, srcBottom;
		int16 dstX, dstY;
		uint id;
	};

	static const uint kSectionCount = 15;


	void init();
	void deinit();

	void setAnimalsButton(const MenuButton *animalsButton);

	void setGamePalette(uint palette);
	void setGameCursor();

	Common::String fixString(const Common::String &str) const;
	void fixTXTStrings(TXTFile &txt) const;

	bool doCopyProtection(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4]);

	void showWait(uint palette = 0xFFFF);  ///< Show the wait / loading screen.
	void showIntro();                      ///< Show the whole intro.

	void showChapter(int chapter); ///< Show a chapter intro text.

	void showByeBye(); ///< Show the "bye bye" screen

	MenuAction doMenu(MenuType type);

	void doAnimalNames(uint count, const MenuButton *buttons, const char * const *names);

	void drawLineByLine(const Surface &src, int16 left, int16 top, int16 right, int16 bottom,
	                    int16 x, int16 y) const;


	// Fonts
	Font *_jeudak;
	Font *_lettre;
	Font *_plettre;
	Font *_glettre;

	Difficulty _difficulty;
	uint8      _section;

private:
	static const MenuButton kMainMenuDifficultyButton[];
	static const MenuButton kSectionButtons[];
	static const MenuButton kIngameButtons[];

	static const MenuButton kAnimalNamesBack;
	static const MenuButton kLanguageButtons[];

	static const char *kSound[kSoundMAX];


	void setCopyProtectionPalette();

	void setAnimState(ANIObject &ani, uint16 state, bool once, bool pause) const;

	// Copy protection helpers
	int8 cpSetup(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4], const Surface sprites[2]);
	int8 cpFindShape(int16 x, int16 y) const;
	void cpWrong();

	// Intro parts
	void showQuote();
	void showTitle();

	// Title music
	void playTitleMusic();
	void playTitleMusicDOS();
	void playTitleMusicAmiga();
	void playTitleMusicAtariST();
	void stopTitleMusic();

	// Menu helpers
	MenuAction doMenuMainStart();
	MenuAction doMenuMainIngame();
	MenuAction doMenuIngame();

	void drawMenuMainStart();
	void drawMenuMainIngame();
	void drawMenuIngame();
	void drawMenuDifficulty();

	void clearMenuIngame(const Surface &background);

	int checkButton(const MenuButton *buttons, uint count, int16 x, int16 y, int failValue = -1) const;
	void drawButton(Surface &dest, const Surface &src, const MenuButton &button) const;
	void drawButtons(Surface &dest, const Surface &src, const MenuButton *buttons, uint count) const;
	void drawButtonBorder(const MenuButton &button, uint8 color);

	// Animal names helpers
	void anSetupChooser();
	void anSetupNames(const MenuButton &animal);
	void anPlayAnimalName(const Common::String &animal, uint language);

	bool _openedArchives;

	const MenuButton *_animalsButton;
};

} // End of namespace OnceUpon

} // End of namespace Gob

#endif // GOB_PREGOB_ONCEUPON_ONCEUPON_H
