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
#include "harvester/npc/dialogue_handler.h"
#include "harvester/script.h"

namespace Common {
struct Event;
}

namespace Harvester {

class HarvesterEngine;
class NpcDialogueHandler;
class Flow;
class Text;
struct IndexedBitmap;

class DialogueSystem {
public:
	DialogueSystem(HarvesterEngine &engine, Common::Point &mousePos);
	~DialogueSystem();

	bool hasRoomNpcHandler(const Common::String &npcName) const;
	void resetRoomNpcDialogueState();
	void syncRuntimeSaveState(Common::Serializer &s);
	Common::Error runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const NpcRecord &npc, const Common::String &usedItemName,
		Flow &flow);

private:
	void registerNpcHandlers();

	HarvesterEngine &_engine;
	Common::Point &_mousePos;
	DialogueSharedState _sharedDialogueState;
	Common::Array<NpcDialogueHandler *> _npcHandlers;
};

} // End of namespace Harvester

#endif // HARVESTER_DIALOGUE_H
