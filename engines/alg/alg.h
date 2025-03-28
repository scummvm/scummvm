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

#ifndef ALG_H
#define ALG_H

#include "engines/advancedDetector.h"
#include "gui/debugger.h"

#include "alg/detection.h"

namespace Alg {

class Game;
class GameBountyHunter;
class GameCrimePatrol;
class GameDrugWars;
class GameJohnnyRock;
class GameMaddog;
class GameMaddog2;
class GameSpacePirates;

enum {
	kAlgDebugGeneral = 1,
	kAlgDebugGraphics
};

class AlgEngine : public Engine {
public:
	AlgEngine(OSystem *syst, const AlgGameDescription *desc);
	~AlgEngine();
	Common::Error run();
	bool hasFeature(EngineFeature f) const;
	Common::Platform getPlatform() const;
	bool isDemo() const;
	bool useSingleSpeedVideos() const { return _useSingleSpeedVideos; };

private:
	const AlgGameDescription *_gameDescription;
	Game *_game;
	GUI::Debugger *_debugger;
	bool _useSingleSpeedVideos = false;
};

} // End of namespace Alg

#endif
