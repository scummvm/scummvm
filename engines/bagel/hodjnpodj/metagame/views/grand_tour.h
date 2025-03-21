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

#ifndef HODJNPODJ_METAGAME_VIEWS_GRAND_TOUR_H
#define HODJNPODJ_METAGAME_VIEWS_GRAND_TOUR_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

struct GRANDTRSTRUCT {
	int nHodjSkillLevel;
	int nPodjSkillLevel;
	int nGameSelection;
	int nCurrGameCode;
	int nHodjScore;
	int nPodjScore;
	int nHodjLastGame;
	int nPodjLastGame;
	int nHodjLastScore;
	int nPodjLastScore;
	bool bPlayingHodj;
	bool bPlayMusic;
	bool bPlayFX;
	bool bMidGrandTour;
	bool abHGamePlayed[18] = {};
	bool abPGamePlayed[18] = {};
	GAMESTRUCT stMiniGame;

	GRANDTRSTRUCT() {
		reset();
	}
	void reset();
};

class GrandTour : public View {
private:
	GfxSurface _background;
	GRANDTRSTRUCT _grandTour;

public:
	GrandTour();

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
