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

#ifndef HARVESTER_DIALOGUE_H
#define HARVESTER_DIALOGUE_H

#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/str.h"
#include "harvester/startup_script.h"

namespace Common {
struct Event;
}

namespace Harvester {

class HankDialogueHandler;
class HarvesterEngine;
class JimmyDialogueHandler;
class MomDialogueHandler;
class NpcDialogueHandler;
class DialogueRuntime;
class PtaMomDialogueHandler;
class StartupFlow;
class StartupText;
class WaspWomanDialogueHandler;
struct IndexedBitmap;

class DialogueSystem {
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
		Common::String currentTopicBuffer;
		int currentTopicBufferLineIndex = -1;
	};

	struct MomRoomDialogueState {
		bool introPending = true;
		bool sameDayIntroLineEnabled = false;
		bool postIntroDefaultLineEnabled = false;
		int introDayIndex = 0;
		bool stephMidgameShown = false;
		bool dinerBurnedKarinMissingOrDeadShown = false;
		bool burnedTvStationShown = false;
		bool scratchedTuckerShown = false;
		bool barberPoleStolenShown = false;
		bool boltOfClothTakenShown = false;
		bool dinerBurnedKarinAliveShown = false;
		bool escapedJailShown = false;
		bool karinKidnapedUnresolvedShown = false;
		bool karinFoundAliveShown = false;
		bool karinFoundDeadShown = false;
		bool butcherAbsentShown = false;
		bool moynahanAbsentShown = false;
		bool jimmyAbsentShown = false;
		bool waspWomanAbsentShown = false;
		bool stephanieDeadPreMidgameShown = false;
		bool day5Shown = false;
		bool day6Shown = false;
		bool fatherTopicState = false;
		bool goodCauseDay5State = false;
	};

	struct JimmyRoomDialogueState {
		bool firstNoItemLinePending = true;
		bool paperHandoffStateSet = false;
	};

	struct WaspWomanRoomDialogueState {
		bool introPending = true;
	};

	DialogueSystem(HarvesterEngine &engine, Common::Point &mousePos);
	~DialogueSystem();

	void resetRoomNpcDialogueState();
	Common::Error runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const StartupNpcRecord &npc, const Common::String &usedItemName,
		StartupFlow &startupFlow);

private:
	friend class HankDialogueHandler;
	friend class JimmyDialogueHandler;
	friend class MomDialogueHandler;
	friend class PtaMomDialogueHandler;
	friend class WaspWomanDialogueHandler;

	void registerNpcHandlers();

	Common::Error handleJimmyDialogue(DialogueRuntime &runtime, const Common::String &usedItemName);
	Common::Error handleWaspWomanDialogue(DialogueRuntime &runtime, const Common::String &usedItemName);
	Common::Error handleMomDialogue(DialogueRuntime &runtime, const Common::String &usedItemName);
	Common::Error handleHankDialogue(DialogueRuntime &runtime, const Common::String &usedItemName);
	Common::Error handlePtaMomDialogue(DialogueRuntime &runtime, const Common::String &usedItemName);

	HarvesterEngine &_engine;
	Common::Point &_mousePos;
	HankRoomDialogueState _hankRoomDialogueState;
	MomRoomDialogueState _momRoomDialogueState;
	JimmyRoomDialogueState _jimmyRoomDialogueState;
	WaspWomanRoomDialogueState _waspWomanRoomDialogueState;
	bool _sharedKarinKidnapedDialogueState = false;
	bool _sharedDiscussedLodgeTopic = false;
	bool _sharedWaspWomanDialogueState = false;
	Common::Array<NpcDialogueHandler *> _npcHandlers;
};

} // End of namespace Harvester

#endif // HARVESTER_DIALOGUE_H
