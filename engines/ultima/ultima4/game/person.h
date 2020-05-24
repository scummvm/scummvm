/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_GAME_PERSON_H
#define ULTIMA4_GAME_PERSON_H

#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

class Conversation;
class Dialogue;
class Response;
class ResponsePart;

typedef enum {
	NPC_EMPTY,
	NPC_TALKER,
	NPC_TALKER_BEGGAR,
	NPC_TALKER_GUARD,
	NPC_TALKER_COMPANION,
	NPC_VENDOR_WEAPONS,
	NPC_VENDOR_ARMOR,
	NPC_VENDOR_FOOD,
	NPC_VENDOR_TAVERN,
	NPC_VENDOR_REAGENTS,
	NPC_VENDOR_HEALER,
	NPC_VENDOR_INN,
	NPC_VENDOR_GUILD,
	NPC_VENDOR_STABLE,
	NPC_LORD_BRITISH,
	NPC_HAWKWIND,
	NPC_MAX
} PersonNpcType;

class Person : public Creature {
public:
	Person(MapTile tile);
	Person(const Person *p);

	bool canConverse() const;
	bool isVendor() const;
	Common::String getName() const override;
	void goToStartLocation();
	void setDialogue(Dialogue *d);
	MapCoords &getStart() {
		return _start;
	}
	PersonNpcType getNpcType() const {
		return _npcType;
	}
	void setNpcType(PersonNpcType t);

	Common::List<Common::String> getConversationText(Conversation *cnv, const char *inquiry);

	/**
	 * Get the prompt shown after each reply.
	 */
	Common::String getPrompt(Conversation *cnv);

	/**
	 * Returns the valid keyboard choices for a given conversation.
	 */
	const char *getChoices(Conversation *cnv);

	Common::String getIntro(Conversation *cnv);
	Common::String processResponse(Conversation *cnv, Response *response);
	void runCommand(Conversation *cnv, const ResponsePart &command);
	Common::String getResponse(Conversation *cnv, const char *inquiry);
	Common::String talkerGetQuestionResponse(Conversation *cnv, const char *inquiry);
	Common::String beggarGetQuantityResponse(Conversation *cnv, const char *response);
	Common::String lordBritishGetQuestionResponse(Conversation *cnv, const char *answer);
	Common::String getQuestion(Conversation *cnv);

private:
	Dialogue *_dialogue;
	MapCoords _start;
	PersonNpcType _npcType;
};

bool isPerson(Object *punknown);

Common::List<Common::String> replySplit(const Common::String &text);
int linecount(const Common::String &s, int columnmax);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
