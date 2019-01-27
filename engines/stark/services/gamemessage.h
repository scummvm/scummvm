/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
