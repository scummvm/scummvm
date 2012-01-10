/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef MYST3_DIRECTORYSUBENTRY_H
#define MYST3_DIRECTORYSUBENTRY_H

#include "common/memstream.h"
#include "math/vector3d.h"

namespace Myst3 {

struct SpotItemData {
	uint32 u;
	uint32 v;
};

struct VideoData {
	Math::Vector3d v1;
	Math::Vector3d v2;
	int32 u;
	int32 v;
	int32 width;
	int32 height;
};

typedef uint32 MiscData[20];

class Archive;

class DirectorySubEntry {
	public:
		enum ResourceType {
			kCubeFace = 0,
			kFaceMask = 1,
			kSpotItem = 5,
			kFrame = 6,
			kCursor = 7,
			kMovie = 8,
			kStillMovie = 10,
			kMetadata = 13,
			kMenuSpotItem = 69,
			kMenuFrame = 70,
			kImagerMovie = 72
		};

		DirectorySubEntry() {}
		DirectorySubEntry(Archive *archive);

		void readFromStream(Common::SeekableReadStream &inStream);
		void dump();
		void dumpToFile(Common::SeekableReadStream &inStream, uint16 index);
		Common::MemoryReadStream *getData() const;
		uint16 getFace() const { return _face; }
		ResourceType getType() const { return _type; }
		const SpotItemData &getSpotItemData() const { return _spotItemData; }
		const VideoData &getVideoData() const { return _videoData; }
		uint32 getMiscData(uint index) const;

	private:
		uint32 _offset;
		uint32 _size;
		uint16 _metadataSize;
		byte _face;
		ResourceType _type;

		// Metadata
		SpotItemData _spotItemData;
		VideoData _videoData;
		MiscData _miscData;

		Archive *_archive;
};

} // end of namespace Myst3

#endif // MYST3_DIRECTORYSUBENTRY_H
