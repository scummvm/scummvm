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

#ifndef HOLLYWOOD_GAMEPLAY_ACTOR_RENDERER_H
#define HOLLYWOOD_GAMEPLAY_ACTOR_RENDERER_H

#include "common/array.h"
#include "common/types.h"

namespace Hollywood {

struct ActorDepthTest {
	bool enabled;
	const Common::Array<byte> *savedFramebuffer;
	const Common::Array<byte> *colorToDepthClassMap;
	const Common::Array<uint16> *depthYThresholds;
	int actorWorldY;
};

int drawActorRunStream(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
	int spriteX, int spriteY, int minimumYExclusive, Common::Array<byte> &destination,
	const ActorDepthTest *depthTest);

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_ACTOR_RENDERER_H
