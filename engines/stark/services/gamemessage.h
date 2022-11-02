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

#ifndef STARK_SERVICES_GAME_MESSAGE_H
#define STARK_SERVICES_GAME_MESSAGE_H

#include "common/str.h"
#include "common/hashmap.h"

namespace Stark {

/**
 * Game message service
 *
 * Provide message texts and dialogs used in various points
 */
class GameMessage {
public:
	GameMessage();
	~GameMessage() {}

	enum TextKey {
		kOverwriteSave = 10,
		kEndAndLoad = 13,
		kInventory = 353,
		kOptions = 354,
		kQuit = 355,
		kQuitGamePrompt = 356,
		kQuitPrompt = 357,
		kYes = 358,
		kNo = 359
	};

	/** Acquire a message text by a given key */
	Common::String getTextByKey(TextKey key) {
		if (_texts.contains(key)) return _texts[key];

		return getDefaultText(key);
	}

private:
	Common::HashMap<uint, Common::String> _texts;

	Common::String getDefaultText(TextKey key);
};

} // End of namespace Stark

#endif // STARK_SERVICES_GAME_MESSAGE_H
