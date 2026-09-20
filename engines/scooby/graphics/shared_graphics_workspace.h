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

#ifndef SCOOBY_SHARED_GRAPHICS_WORKSPACE_H
#define SCOOBY_SHARED_GRAPHICS_WORKSPACE_H

#include "common/scummsys.h"
#include "common/algorithm.h"

#include "common/array.h"

#include "scooby/common/span.h"

// Preserves the overlapping recovered lifetimes that consume the shared
// graphics workspace prefix.
//
// Ghidra g_abSharedGraphicsWorkspace spans 0xFF2C00-0xFF37FF. The currently
// recovered inventory-pattern lifetime extends through 0xFF37FF; interface
// transitions and the interaction-action menu alias its first 0x180 bytes.
// Process reset clears this complete prefix before any owner can consume it.

namespace Scooby {

class SharedGraphicsWorkspace {
public:
	SharedGraphicsWorkspace() : _bytes(kInventoryPatternByteCount) {
		Common::fill(_bytes.begin(), _bytes.end(), 0);
	}

	// Gets the aliased 0xFF2C00-0xFF2D7F prefix transferred by room/interface
	// transitions and preserved around the interaction-action menu.
	Span<uint8> interfaceTileBlock() {
		return Span<uint8>(_bytes.data(), kInterfaceTileBlockByteCount);
	}

	// Gets the complete 0xFF2C00-0xFF37FF inventory-pattern staging range
	// without breaking its alias with interface-transition and
	// interaction-action menu lifetimes.
	Span<uint8> inventoryPatterns() {
		return Span<uint8>(_bytes.data(), _bytes.size());
	}

private:
	static const std::size_t kInterfaceTileBlockByteCount = 0x180;
	static const std::size_t kInventoryPatternByteCount = 0xC00;

	Common::Array<uint8> _bytes;
};
} // namespace Scooby

#endif // SCOOBY_SHARED_GRAPHICS_WORKSPACE_H
