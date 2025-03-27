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

#ifndef HODJNPODJ_VIEWS_H
#define HODJNPODJ_VIEWS_H

#include "bagel/hodjnpodj/views/rules.h"
#include "bagel/hodjnpodj/views/main_menu.h"
#include "bagel/hodjnpodj/views/message_box.h"
#include "bagel/hodjnpodj/metagame/views/boardgame_options.h"
#include "bagel/hodjnpodj/metagame/views/boardgame.h"
#include "bagel/hodjnpodj/metagame/views/credits.h"
#include "bagel/hodjnpodj/metagame/views/grand_tour.h"
#include "bagel/hodjnpodj/metagame/views/minigames.h"
#include "bagel/hodjnpodj/metagame/views/movie.h"
#include "bagel/hodjnpodj/metagame/views/title_menu.h"
#include "bagel/hodjnpodj/metagame/views/top_scores.h"
#include "bagel/hodjnpodj/fuge/fuge.h"
#include "bagel/hodjnpodj/mazedoom/maze_doom.h"
#include "bagel/hodjnpodj/novacancy/no_vacancy.h"

namespace Bagel {
namespace HodjNPodj {

struct Views {
	MainMenu _mainMenu;
	MessageBox _messageBox;
	Rules _rules;
	Metagame::BoardgameOptions _boardgameOptions;
	Metagame::Boardgame _boardgame;
	Metagame::Credits _credits;
	Metagame::GrandTour _grandTour;
	Metagame::Minigames _minigames;
	Metagame::Movie _movie;
	Metagame::TitleMenu _titleMenu;
	Metagame::TopScores _topScores;
	Fuge::Fuge _fuge;
	MazeDoom::MazeDoom _mazeDoom;
	NoVacancy::NoVacancy _noVacancy;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
