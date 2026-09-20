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

#include "actor_animation_descriptor_catalog.h"

namespace Scooby {

bool ActorAnimationDescriptorCatalog::tryResolve(int16 shapeIndex, int &descriptorOffset,
												 int &catalogIndex) const {
	for (catalogIndex = 0; catalogIndex < kShapeCount; ++catalogIndex) {
		descriptorOffset = static_cast<int>(
			_rom.readUInt32(kDescriptorCatalogOffset + catalogIndex * static_cast<int>(sizeof(uint32))));
		if (_rom.readInt16(descriptorOffset) == shapeIndex) {
			return true;
		}
	}

	descriptorOffset = 0;
	catalogIndex = -1;
	return false;
}

uint32 ActorAnimationDescriptorCatalog::readHorizontalStep(int catalogIndex) const {
	return _rom.readUInt32(
		kHorizontalStepCatalogOffset + catalogIndex * static_cast<int>(sizeof(uint32)));
}
} // namespace Scooby