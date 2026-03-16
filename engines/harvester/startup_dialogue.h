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

#ifndef HARVESTER_STARTUP_DIALOGUE_H
#define HARVESTER_STARTUP_DIALOGUE_H

#include "common/error.h"
#include "common/rect.h"
#include "common/str.h"
#include "harvester/startup_script.h"

namespace Common {
struct Event;
}

namespace Harvester {

class HarvesterEngine;
class StartupFlow;
struct IndexedBitmap;

class StartupDialogueSystem {
public:
	struct HankRoomDialogueState {
		bool pendingInitialConversation = true;
		bool hasTrackedDayState = false;
		bool pendingRangshotSequence = false;
		bool pendingSameDayFollowup = false;
		int trackedDayIndex = 0;
		bool stephMidgamePlayedShown = false;
		bool burnedTvStationShown = false;
		bool bustedOnceShown = false;
		bool karinKidnapedShown = false;
		bool karinFoundAliveShown = false;
		bool karinFoundDeadShown = false;
		bool discussedCasketPhotoEvidence = false;
		bool discussedWhaleyHerrillPhoto = false;
		bool discussedLodgeTopic = false;
	};

	StartupDialogueSystem(HarvesterEngine &engine, Common::Point &mousePos);

	void resetRoomNpcDialogueState();
	Common::Error runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const StartupNpcRecord &npc, const Common::String &usedItemName,
		StartupFlow &startupFlow);

private:
	HarvesterEngine &_engine;
	Common::Point &_mousePos;
	HankRoomDialogueState _hankRoomDialogueState;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_DIALOGUE_H
