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

#ifndef GOT_VIEWS_GAME_CONTENT_H
#define GOT_VIEWS_GAME_CONTENT_H

#include "got/data/defines.h"
#include "got/views/view.h"

namespace Got {
namespace Views {

class GameContent : public View {
private:
	GfxSurface _surface;
	Common::Point _moveDelta;
	int _transitionPos = 0;
	int _deathCtr = 0;
	bool _phased[240] = {};
	int _lightningCtr = 0;
	int _pixelX[8][25] = {};
	int _pixelY[8][25] = {};
	byte _pixelP[8][25] = {};
	byte _pixelC[8] = {};
	int _pauseCtr = 0;
	int _closingStateCtr = 0;

	void drawBackground(GfxSurface &s);
	void drawObjects(GfxSurface &s);
	void drawActors(GfxSurface &s);
	void drawBossHealth(GfxSurface &s);
	void drawLightning(GfxSurface &s);
	void placePixel(GfxSurface &s, int dir, int num);
	void checkThunderShake();
	void checkSwitchFlag();
	void checkForItem();
	void moveActors();
	void updateActors();
	void checkForBossDead();
	void checkForAreaChange();
	void thorDies();
	void spinThor();
	void thorDead();
	void checkForCheats();
	void throwLightning();
	void lightningCountdownDone();
	void closingSequence();

public:
	GameContent();
	virtual ~GameContent() {}

	void draw() override;
	bool msgGame(const GameMessage &msg) override;
	bool tick() override;
};

} // namespace Views
} // namespace Got

#endif
