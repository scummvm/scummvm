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

#ifndef MADS_MADS_H
#define MADS_MADS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"
#include "mads/detection.h"

namespace MADS {

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

enum MADSDebugChannels {
	kDebugPath = 1,
	kDebugScripts,
	kDebugGraphics,
	kDebugConversations
};

enum ScreenFade {
	SCREEN_FADE_SMOOTH = 0,
	SCREEN_FADE_MEDIUM = 1,
	SCREEN_FADE_FAST = 2
};

enum MADSActions {
	kActionNone,
	kActionEscape,
	kActionGameMenu,
	kActionSave,
	kActionRestore,
	kActionScrollUp,
	kActionScrollDown,
	kActionStartGame,
	kActionResumeGame,
	kActionShowIntro,
	kActionCredits,
	kActionQuotes,
	kActionRestartAnimation
};

class MADSEngine : public Engine {
protected:
	const MADSGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	bool hasFeature(EngineFeature f) const override;

public:
	MADSEngine(OSystem *syst, const MADSGameDescription *gameDesc);
	~MADSEngine() override;

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	uint32 getGameID() const;
	uint32 getGameFeatures() const;
	bool isDemo() const;

	int getRandomNumber(int maxNumber);
	int getRandomNumber(int minNumber, int maxNumber);
};

} // namespace MADS

#endif
