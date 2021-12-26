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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ILLUSIONS_FILERESOURCEREADER_H
#define ILLUSIONS_FILERESOURCEREADER_H

#include "illusions/illusions.h"
#include "illusions/resourcereader.h"

namespace Illusions {

class ResourceReaderFileReader : public BaseResourceReader {
public:
	byte *readResource(uint32 sceneId, uint32 resId, uint32 &dataSize) override;
protected:
	Common::String buildResourceFilename(uint32 resId);
	const char *getResourceExtension(uint32 resId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_FILERESOURCEREADER_H
