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

#ifndef SCOOBY_ACTOR_ANIMATION_DESCRIPTOR_CATALOG_H
#define SCOOBY_ACTOR_ANIMATION_DESCRIPTOR_CATALOG_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"

// Resolves the shared authored actor-shape catalog without duplicating its parallel table rules.
//
// Ghidra g_apActorAnimationDescriptorCatalog at 0x00032670-0x000326FF contains exactly 36 descriptor
// pointers. The parallel signed 16.16 horizontal steps occupy 0x000325E0-0x0003266F with the same indices.

namespace Scooby {

class ActorAnimationDescriptorCatalog {
public:
	// Binds the immutable descriptor and movement-step catalogs to the verified cartridge.
	// rom: verified cartridge containing both parallel catalogs and their descriptors.
	explicit ActorAnimationDescriptorCatalog(const ScoobyDooRom &rom) : _rom(rom) {
	}

	// Scans all 36 descriptors by their leading signed shape word.
	// shapeIndex: authored shape identity to match exactly.
	// descriptorOffset: matched descriptor cartridge offset, or zero when absent.
	// catalogIndex: matched parallel-table index, or -1 when absent.
	// Returns true for the first exact match; otherwise false.
	bool tryResolve(int16 shapeIndex, int &descriptorOffset, int &catalogIndex) const;

	// Reads the raw signed 16.16 horizontal step parallel to a resolved descriptor.
	// catalogIndex: resolved index from TryResolve.
	// Returns the exact authored 32-bit step bit pattern.
	uint32 readHorizontalStep(int catalogIndex) const;

private:
	static const int kDescriptorCatalogOffset = 0x32670;
	static const int kHorizontalStepCatalogOffset = 0x325E0;
	static const int kShapeCount = 36;

	const ScoobyDooRom &_rom;
};
} // namespace Scooby

#endif // SCOOBY_ACTOR_ANIMATION_DESCRIPTOR_CATALOG_H
