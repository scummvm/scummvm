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

#include "illusions/resourcesystem.h"
#include "illusions/fileresourcereader.h"
#include "illusions/illusions.h"
#include "common/file.h"
#include "common/str.h"

namespace Illusions {

byte *ResourceReaderFileReader::readResource(uint32 sceneId, uint32 resId, uint32 &dataSize) {
	debug("ResourceReaderFileReader::readResource(%08X, %08X)", sceneId, resId);

	Common::String filename = buildResourceFilename(resId);
	Common::File fd;
	if (!fd.open(filename))
		error("Resource::loadData() Could not open %s for reading", filename.c_str());
	dataSize = fd.size();
	byte *data = (byte*)malloc(dataSize);
	fd.read(data, dataSize);
	return data;
}

Common::String ResourceReaderFileReader::buildResourceFilename(uint32 resId) {
	const char *ext = getResourceExtension(resId);
	return Common::String::format("%08X%s", resId, ext);
}

const char *ResourceReaderFileReader::getResourceExtension(uint32 resId) {
	// TODO Make constants
	switch (ResourceTypeId(resId)) {
	case 0x00060000:
	case 0x00100000:
		// ActorResource
		return ".act";
	case 0x00080000:
		// SoundGroupResource
		return ".sg";
	case 0x000D0000:
		// ScriptResource
		return ".scr";
	case 0x000F0000:
		// TalkResource
		return ".tlk";
	case 0x00110000:
		// BackgroundResource
		return ".bg";
	case 0x00120000:
		// FontResource
		return ".fnt";
	case 0x00170000:
		// SpecialCode
		return "";
	default:
		return "";
	}
}

} // End of namespace Illusions
