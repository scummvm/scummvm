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

#ifndef GOT_VIEWS_H
#define GOT_VIEWS_H

#include "got/views/credits.h"
#include "got/views/dialogs/ask.h"
#include "got/views/dialogs/high_scores.h"
#include "got/views/dialogs/main_menu.h"
#include "got/views/dialogs/options_menu.h"
#include "got/views/dialogs/quit.h"
#include "got/views/dialogs/quit_game.h"
#include "got/views/dialogs/save_game.h"
#include "got/views/dialogs/say.h"
#include "got/views/dialogs/select_game.h"
#include "got/views/dialogs/select_item.h"
#include "got/views/dialogs/select_scroll.h"
#include "got/views/dialogs/select_slow.h"
#include "got/views/dialogs/set_music.h"
#include "got/views/dialogs/set_sound.h"
#include "got/views/game.h"
#include "got/views/opening.h"
#include "got/views/part_title.h"
#include "got/views/splash_screen.h"
#include "got/views/story.h"
#include "got/views/title_background.h"

namespace Got {
namespace Views {

struct Views {
	Credits _credits;
	Game _game;
	Opening _opening;
	PartTitle _partTitle;
	SplashScreen _splashScreen;
	Story _story;
	TitleBackground _titleBackground;

	Dialogs::Ask _ask;
	Dialogs::HighScores _highScores;
	Dialogs::MainMenu _mainMenu;
	Dialogs::OptionsMenu _optionsMenu;
	Dialogs::Quit _quit;
	Dialogs::QuitGame _quitGame;
	Dialogs::SaveGame _saveGame;
	Dialogs::Say _say;
	Dialogs::SelectGame _selectGame;
	Dialogs::SelectItem _selectItem;
	Dialogs::SelectScroll _selectScroll;
	Dialogs::SelectSlow _selectSlow;
	Dialogs::SetMusic _setMusic;
	Dialogs::SetSound _setSound;
};

} // namespace Views
} // namespace Got

#endif
