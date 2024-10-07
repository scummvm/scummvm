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

// Resource library

#include "common/file.h"
#include "common/fs.h"

#include "dgds/sound/resource/sci_resource.h"
#include "dgds/sound/scispan.h"

namespace Dgds {

// A cut-down version of SciResource so we don't have to change
// the midi code too much.
SciResource::SciResource(const byte *data, int dataSz, int16 id)
: SciSpan<const byte>(data, dataSz, Common::String::format("%d", id)) {
}

SciResource::~SciResource() {
}

void SciResource::unalloc() {
}

} // End of namespace Dgds
