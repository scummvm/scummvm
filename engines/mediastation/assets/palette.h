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

#ifndef MEDIASTATION_PALETTE_H
#define MEDIASTATION_PALETTE_H

#include "graphics/palette.h"

#include "mediastation/asset.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class Palette : public Asset {
public:
	Palette() : Asset(kAssetTypePalette) {};
	virtual ~Palette() override;

	virtual void readParameter(Chunk &chunk, AssetHeaderSectionType paramType) override;

	Graphics::Palette *_palette = nullptr;
};

} // End of namespace MediaStation

#endif
