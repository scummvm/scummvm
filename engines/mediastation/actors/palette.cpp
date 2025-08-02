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

#include "mediastation/mediastation.h"
#include "mediastation/actors/palette.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

PaletteActor::~PaletteActor() {
	delete _palette;
	_palette = nullptr;
}

void PaletteActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderPalette: {
		byte *buffer = new byte[Graphics::PALETTE_SIZE];
		chunk.read(buffer, Graphics::PALETTE_SIZE);
		_palette = new Graphics::Palette(buffer, Graphics::PALETTE_COUNT, DisposeAfterUse::YES);
		break;
	}

	default:
		Actor::readParameter(chunk, paramType);
	}
}

} // End of namespace MediaStation
