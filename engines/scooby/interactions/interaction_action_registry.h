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

#ifndef SCOOBY_INTERACTION_ACTION_REGISTRY_H
#define SCOOBY_INTERACTION_ACTION_REGISTRY_H

#include "common/array.h"

#include "interaction_action.h"

// Owns the bounded interaction-action choices rebuilt by room-script condition scans.

namespace Scooby {

class InteractionActionRegistry {
public:
	// The five-entry bound enforced by original command 02 before any indexed write.
	static const int Capacity = 5;

	InteractionActionRegistry() : _entries(Capacity), _count(0) {
	}

	// Ghidra g_wInteractionActionCount at 0xFF080E.
	int count() const { return _count; }

	// Returns one backing-table entry without narrowing access to the current registered count.
	const InteractionAction &operator[](int index) const { return _entries[static_cast<std::size_t>(index)]; }

	// Appends an entry after command 02 has proved that the original five-entry bound permits it.
	void registerEntry(const InteractionAction &entry) { _entries[static_cast<std::size_t>(_count++)] = entry; }

	// Starts a fresh scan while preserving backing entries for original raw-table indexing branches.
	void reset() { _count = 0; }

private:
	Common::Array<InteractionAction> _entries;
	int _count;
};
} // namespace Scooby

#endif // SCOOBY_INTERACTION_ACTION_REGISTRY_H
