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

#include "common/util.h"
#include "hollywood/runaway/inventory.h"

namespace Hollywood {

RunawayInventory::Reply RunawayInventory::combineTownItems(byte first, byte second) {
	// Pouring is directional: the selected item is the source.
	if ((first == 25 && second == 26) || (first == 26 && second == 25)) {
		byte &source = first == 25 ? _town.tube30 : _town.tube50;
		byte &target = second == 25 ? _town.tube30 : _town.tube50;
		const byte capacity = second == 25 ? 3 : 5;
		const uint cue = first == 25 ? 234 : 236;
		if (!source) {
			return Reply(cue);
		}
		if (target == capacity) {
			return Reply(cue, 1);
		}
		if (_town.tube50 == 4 && first == 25) {
			return Reply(228, 4);
		}
		const byte amount = MIN<byte>(source, capacity - target);
		source -= amount;
		target += amount;
		_selected = 0;
		return Reply(_town.tube50 == 4 ? 236 : 234, 2);
	}
	if (first == 12 && (second == 25 || second == 26)) { // Fill a measuring tube from the concentrate.
		byte &contents = second == 25 ? _town.tube30 : _town.tube50;
		const byte capacity = second == 25 ? 3 : 5;
		if (second == 26 && contents == 4) {
			return Reply(228, 4);
		}
		if (contents == capacity) {
			return Reply(second == 25 ? 236 : 234, 1);
		}
		contents = capacity;
		_selected = 0;
		return Reply(228, second == 25 ? 2 : 3);
	}
	if ((first == 25 || first == 26) && second == 12) { // Empty a tube back into the concentrate.
		byte &contents = first == 25 ? _town.tube30 : _town.tube50;
		if (!contents) {
			return Reply(first == 25 ? 234 : 236);
		}
		contents = 0;
		_selected = 0;
		return Reply(228);
	}
	if (first == 25 && second == 24) { // The small tube cannot measure the required forty cc.
		if (!_town.tube30) {
			return Reply(235);
		}
		return Reply(237, _town.bottleContents == 2 ? 4 : 0);
	}
	if ((first == 12 || first == 26) && second == 24) { // Add measured concentrate to one liter of water.
		if (_town.bottleContents == 2) {
			return Reply(237, 4);
		}
		if (!_town.bottleContents) {
			return Reply(237, 1);
		}
		if (first == 12) {
			return Reply(237);
		}
		if (!_town.tube50) {
			return Reply(235);
		}
		if (_town.tube50 != 4) {
			return Reply(237);
		}
		_town.tube50 = 0;
		_town.bottleContents = 2;
		_selected = 0;
		return Reply(237, 2);
	}
	if (first == 23 && second == 24) { // Transfer the oil can's water to the one-liter bottle.
		if (!_town.oilCanFull) {
			return Reply(231);
		}
		if (_town.bottleContents) {
			return Reply(231, 1);
		}
		_town.oilCanFull = 0;
		_town.bottleContents = 1;
		_selected = 0;
		return Reply(231, 2);
	}
	if (first == 12 && (second == 8 || second == 13 || second == 16 || second == 21 || second == 23 || second == 31)) {
		// Concentrate belongs in the measuring tubes.
		return Reply(229);
	}
	if (first == 23 && (second == 8 || second == 12 || second == 13 || second == 21 ||
			second == 25 || second == 26 || second == 31)) {
		// Pouring the oil can into anything other than the bottle wastes its water.
		return Reply(_town.oilCanFull ? 232 : 231);
	}
	if (first == 24 && (second == 25 || second == 26)) { // Do not put water or prepared gasoline into a measuring tube.
		return _town.bottleContents == 1 ? Reply(238) : Reply(233, _town.bottleContents);
	}
	if (first == 24 && (second == 8 || second == 12 || second == 13 || second == 21 || second == 23 || second == 31)) {
		// Preserve the bottle's exact measure or prepared fuel.
		return Reply(233, _town.bottleContents);
	}
	if ((first == 25 || first == 26) && (second == 8 || second == 21 || second == 23 || second == 31 ||
			(first == 26 && second == 13))) {
		// Do not waste measured concentrate in an unsuitable container.
		return Reply(235, (first == 25 ? _town.tube30 : _town.tube50) != 0);
	}
	if (first == 8 && second == 28) { // The chewing tobacco already contains liniment.
		return Reply(226);
	}
	if (first == 8 && second == 31) { // The liniment must be combined with the leaves.
		return Reply(243);
	}
	// The remaining combinations work in either direction.
	if (first > second) {
		SWAP(first, second);
	}
	if (first == 11 && second == 20) { // Staple the ends of the drive belt together.
		if (_town.strap != 1) {
			return Reply(227, _town.strap == 2 ? 3 : 0);
		}
		_town.strap = 2;
		_selected = 0;
		_newItem = 11;
		return Reply(227, 1);
	}
	if (first == 8 && second == 27) { // Make menthol chewing tobacco using the improvised mortar.
		if (!_town.mortar) {
			_town.mortarIdea = 1;
			return Reply(225);
		}
		remove(27);
		add(28);
		return Reply(225, 1);
	}
	if (first == 27 && second == 31) { // Grind the leaves only once the mortar is ready.
		if (_town.mortar && contains(8)) {
			return combineTownItems(8, 27);
		}
		return Reply(239);
	}
	if (first == 31 && second == 32) { // Use the bank stamp as the mortar's pestle.
		if (!_town.mortarIdea) {
			return Reply(241);
		}
		_town.mortar = 1;
		remove(32);
		return Reply(241, 1);
	}
	if (first == 4 && second == 29) { // The room supplies Saturn's prerequisite for decorating the sculpture.
		return Reply(224);
	}
	if (first == 29 && second == 30) { // The room supplies Saturn's prerequisite for sanding the stone.
		return Reply(240);
	}
	return Reply::fromAction(1);
}

} // End of namespace Hollywood
