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

#ifndef TESORO_CORE_INVENTORYITEM_H
#define TESORO_CORE_INVENTORYITEM_H

#include "tesoro/core/storyprop.h"

namespace Tesoro {

class World;

class InventoryItem;

/**
 * Object kind 13: an inventory object
 */
class InventoryItem : public InteractiveObject {
public:

	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host);

	int update() override;
	int setVerbAndClick(int verb) override;
	/** Picking up (true) or putting back (false). */
	void setActive(bool active) override;
	int verb(int verb) override;
	int useItem(int item) override;

	int itemNumber() const { return _itemNumber; }
	uint combinationCount() const { return _comboKeys.size(); }
	uint64 combinationKey(uint i) const { return _comboKeys[i]; }
	const SpeechTable::Line *combinationLine(uint i) const { return _comboLines[i]; }
	int subtype() const { return _subtype; }

private:
	int dispatch(int verb);

	int sayCombination(const SpeechTable::Line *line);
	int run(const char *block, const char *label, bool withState);
	int runWith(const char *block, const char *label, byte *variables);
	byte *state() { return _stateBlob.empty() ? nullptr : _stateBlob.data(); }
	void setItemActive(int level, const char *name, bool active);

	uint16 _itemNumber = 0;
	int _subtype = 0;
	Common::Array<uint64> _comboKeys;
	Common::Array<const SpeechTable::Line *> _comboLines;
};

} // End of namespace Tesoro

#endif
