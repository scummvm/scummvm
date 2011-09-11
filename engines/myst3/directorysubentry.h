/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/memstream.h"
#include "graphics/vector3d.h"

namespace Myst3 {

struct SpotItemData {
	uint32 u;
	uint32 v;
};

struct VideoData {
	Graphics::Vector3d v1;
	Graphics::Vector3d v2;
	uint32 u;
	uint32 v;
	uint32 width;
	uint32 height;
};

class DirectorySubEntry {
	private:
		uint32 _offset;
		uint32 _size;
		uint16 _metadataSize;
		byte _face;
		byte _type;

		// Metadata
		SpotItemData _spotItemData;
		VideoData _videoData;

	public:
		enum ResourceType {
			kCubeFace = 0,
			kFaceMask = 1,
			kSpotItem = 5,
			kFrame = 6,
			kMovie = 8
		};

		void readFromStream(Common::SeekableReadStream &inStream);
		void dump();
		void dumpToFile(Common::SeekableReadStream &inStream, uint16 index);
		Common::MemoryReadStream *dumpToMemory(Common::SeekableReadStream &inStream) const;
		uint16 getFace() { return _face; }
		ResourceType getType() { return static_cast<ResourceType>(_type); }
		const SpotItemData &getSpotItemData() { return _spotItemData; }
		const VideoData &getVideoData() { return _videoData; }
};

} // end of namespace Myst3
