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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/metagame/views/grand_tour.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define FONT_SIZE 8

#define	NOPLAY		-1
#define GAME_ALPHA	0
#define GAME_GEO	1
#define GAME_RAND	2

void GRANDTRSTRUCT::reset() {
	bMidGrandTour = false;
	nHodjSkillLevel = SKILLLEVEL_LOW;
	nPodjSkillLevel = NOPLAY;
	nGameSelection = GAME_ALPHA;
	nCurrGameCode = 0;
	nHodjScore = 0;
	nPodjScore = 0;
	nHodjLastGame = 0;
	nPodjLastGame = 0;
	nHodjLastScore = 0;
	nPodjLastScore = 0;
	bPlayMusic = true;
	bPlayFX = true;
	bPlayingHodj = true;
	Common::fill(abHGamePlayed, abHGamePlayed + 18, false);
	Common::fill(abPGamePlayed, abPGamePlayed + 18, false);

	stMiniGame.lCrowns = 0;
	stMiniGame.lScore = 0;
	stMiniGame.nSkillLevel = SKILLLEVEL_LOW;
	stMiniGame.bSoundEffectsEnabled = bPlayFX;
	stMiniGame.bMusicEnabled = bPlayMusic;
	stMiniGame.bPlayingMetagame = true;
	stMiniGame.bPlayingHodj = true;
}

GrandTour::GrandTour() : View("GrandTour") {
}

bool GrandTour::msgOpen(const OpenMessage &msg) {
	View::msgOpen(msg);
	g_engine->_bReturnToGrandTour = true;
	_grandTour.reset();

	return true;
}

bool GrandTour::msgClose(const CloseMessage &msg) {
	View::msgClose(msg);
	g_engine->_bReturnToGrandTour = false;

	return true;
}

bool GrandTour::msgGame(const GameMessage &msg) {
	return false;
}

void GrandTour::draw() {
	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);

	s.clear();
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
