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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ACTOR_TYPES_H
#define HOLLYWOOD_SCENES_PLAYABLE_ACTOR_TYPES_H

#include "common/types.h"

namespace Hollywood {

// Decoded actor sprite metadata from RESOURCE.000.
struct ActiveActorSpriteDescriptor {
	uint32 runStreamOffset;
	uint32 opaqueRunCount;
	// Header-only palette-remap runs drawn after opaque pixels; used for actor shadows.
	uint32 paletteRunCount;
	int16 anchorX;
	int16 anchorY;
	uint16 width;
	uint16 height;
};

// Smaller overlay actor descriptors used by talking/idling secondary actors.
struct SecondaryActorSpriteDescriptor {
	uint32 runStreamOffset;
	uint32 runCount;
	int16 anchorX;
	int16 anchorY;
};

// One rendered step in the active actor walking queue.
struct ActorPathFrame {
	byte drawOrderMode;
	byte facing;
	byte cel;
	int16 worldX;
	int16 worldY;
};

// Mutable route builder state while expanding palette-region paths.
struct ActorPathBuildState {
	byte drawOrderMode;
	byte facing;
	byte cel;
	int x;
	int y;
};

} // End of namespace Hollywood

#endif
