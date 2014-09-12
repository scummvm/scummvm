/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

typedef uint32 MiscData[22];

class Archive;

class DirectorySubEntry {
	public:
		enum ResourceType {
			kCubeFace = 0,
			kWaterEffectMask = 1,
			kLavaEffectMask = 2,
			kMagneticEffectMask = 3,
			kShieldEffectMask = 4,
			kSpotItem = 5,
			kFrame = 6,
			kRawData = 7,
			kMovie = 8,
			kStillMovie = 10,
			kText = 11,
			kTextMetadata = 12,
			kNumMetadata = 13,
			kLocalizedSpotItem = 69,
			kLocalizedFrame = 70,
			kMultitrackMovie = 72,
			kDialogMovie = 74
		};

		DirectorySubEntry() {}
		DirectorySubEntry(Archive *archive);

		void readFromStream(Common::SeekableReadStream &inStream);
		void dumpToFile(Common::SeekableReadStream &inStream, const char* room, uint32 index);
		Common::MemoryReadStream *getData() const;
		uint16 getFace() const { return _face; }
		ResourceType getType() const { return _type; }
		const SpotItemData &getSpotItemData() const { return _spotItemData; }
		const VideoData &getVideoData() const { return _videoData; }
		uint32 getMiscData(uint index) const;
		Common::String getTextData(uint index) const;

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

} // End of namespace Myst3

#endif // MYST3_DIRECTORYSUBENTRY_H
