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

#ifdef DEPRECATED_MADS_GAME_NEBULAR_H
#define MADS_GAME_NEBULAR_H

#include "common/scummsys.h"
#include "mads/nebular/core/game.h"
#include "mads/nebular/core/globals.h"
#include "mads/nebular/globals_nebular.h"

namespace MADS {
namespace Nebular {


enum ProtectionResult {
	PROTECTION_SUCCEED = 0, PROTECTION_FAIL = 1, PROTECTION_ESCAPE = 2
};

class GameNebular : public Game {
	friend class Game;
private:
	ProtectionResult checkCopyProtection();
protected:
	GameNebular(RexNebularEngine *vm);

	void startGame() override;

	void initializeGlobals() override;

	void setSectionHandler() override;

	void checkShowDialog() override;
public:
	NebularGlobals _globals;
	StoryMode _storyMode;
	Difficulty _difficulty;

	Globals &globals() override { return _globals; }

	void doObjectAction() override;

	void showRecipe();

	void unhandledAction() override;

	void step() override;

	void synchronize(Common::Serializer &s, bool phase1) override;

	void setNaughtyMode(bool naughtyMode) override { _storyMode = naughtyMode ? STORYMODE_NAUGHTY : STORYMODE_NICE; }
	bool getNaughtyMode() const override { return _storyMode == STORYMODE_NAUGHTY; }
};

// Section handlers aren't needed in ScummVM implementation
class Section1Handler : public SectionHandler {
public:
	Section1Handler(RexNebularEngine *vm) : SectionHandler(vm) {}

	void preLoadSection() override {}
	void sectionPtr2() override {}
	void postLoadSection() override {}
};

typedef Section1Handler Section2Handler;
typedef Section1Handler Section3Handler;
typedef Section1Handler Section4Handler;
typedef Section1Handler Section5Handler;
typedef Section1Handler Section6Handler;
typedef Section1Handler Section7Handler;
typedef Section1Handler Section8Handler;

} // namespace Nebular
} // namespace MADS

#endif
