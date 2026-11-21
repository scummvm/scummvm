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

#include "hollywood/runaway/scene.h"

namespace Hollywood {

RunawayInventory::Reply RunawayInventory::combineHopiItems(byte first, byte second) {
	if (first == 10 && second == 11) {
		remove(10);
		add(13);
		return Reply(261);
	}
	if (first == 11 && second == 13) {
		return Reply(262);
	}
	if (first == 14 && second == 15) {
		_hopi.toolHandle = 1;
		remove(14);
		select(0);
		return Reply(263);
	}
	if (first == 10 && second == 15) {
		return Reply(264, _hopi.toolHandle);
	}
	if (first == 13 && second == 15) {
		return Reply(265);
	}
	return Reply::fromAction(1);
}

bool RunawayRoomScene::sayHopiInventory(const RunawayInventory::Reply &reply) {
	typedef RunawaySequence S;
	static const S::Command crucifix[] = {
		{S::kImage, 145}, {S::kSpeech, 200, 0, 0, 1}, {S::kWaitInput}, {S::kEnd}
	};
	static const S::Command map[] = {
		{S::kImage, 146}, {S::kSpeech, 252, 0, 0, 1}, {S::kWaitInput}, {S::kEnd}
	};
	static const S::Command finger[] = {
		{S::kImage, 147}, {S::kSpeech, 254, 0, 0, 1}, {S::kWaitInput}, {S::kEnd}
	};
	const S::Command *commands = nullptr;
	switch (reply.cue) {
	case 200: // Inspect the crucifix.
		commands = crucifix;
		break;
	case 252: // Read the map of the mine.
		commands = map;
		break;
	case 254: // Examine the bottle from the crypt.
		commands = finger;
		break;
	case 256: // Examine both ends of the tomahawk.
		say(256, 0, true);
		say(256, 1, true);
		return true;
	case 261: // Split the timber with the tomahawk.
	case 263: // Fit the femur into the mining tool.
		say(reply.cue, 0, true);
		_audio.playEffect(reply.cue == 261 ? 20 : 14);
		say(reply.cue, 1, true);
		return true;
	default:
		return false;
	}
	_inventorySequence = true;
	_sequence.start(commands, _text, _now, 'G');
	return true;
}

} // End of namespace Hollywood
