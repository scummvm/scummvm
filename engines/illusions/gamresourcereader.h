/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_RESOURCEREADER_GAMARCHIVE_H
#define ILLUSIONS_RESOURCEREADER_GAMARCHIVE_H

#include "illusions/illusions.h"
#include "illusions/resourcereader.h"

namespace Illusions {

class GamArchive;

class ResourceReaderGamArchive : public BaseResourceReader {
public:
	ResourceReaderGamArchive(const char *filename);
	~ResourceReaderGamArchive() override;
	byte *readResource(uint32 sceneId, uint32 resId, uint32 &dataSize) override;
protected:
	GamArchive *_gamArchive;
};

} // End of namespace Illusions

#endif // ILLUSIONS_RESOURCEREADER_GAMARCHIVE_H
