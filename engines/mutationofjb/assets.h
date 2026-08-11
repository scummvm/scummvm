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

#ifndef MUTATIONOFJB_ASSETS_H
#define MUTATIONOFJB_ASSETS_H

#include "mutationofjb/font.h"
#include "mutationofjb/conversationlinelist.h"
#include "mutationofjb/inventoryitemdefinitionlist.h"
#include "mutationofjb/hardcodedstrings.h"

namespace MutationOfJB {

class Game;

class Assets {
public:
	Assets(Game &game);

	Font &getSystemFont();
	Font &getSpeechFont();

	/**
	 * Access to "to say" list for conversations.
	 *
	 * @return Conversation line list.
	 */
	ConversationLineList &getToSayList();

	/**
	 * Access to "response" list for conversations.
	 *
	 * @return Conversation line list.
	 */
	ConversationLineList &getResponseList();

	/**
	 * Access to inventory definitions.
	 *
	 * @return Inventory item definition list.
	 */
	InventoryItemDefinitionList &getInventoryItemDefList();

	/**
	 * Access to strings hardcoded in game executable.
	 *
	 * @return Hardcoded strings.
	 */
	HardcodedStrings &getHardcodedStrings();
private:
	SystemFont _systemFont;
	SpeechFont _speechFont;
	ConversationLineList _toSayList;
	ConversationLineList _responseList;
	InventoryItemDefinitionList _invItemDefList;
	HardcodedStrings _hardcodedStrings;
};

}

#endif
