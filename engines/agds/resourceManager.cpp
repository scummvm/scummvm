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

#include "agds/resourceManager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/algorithm.h"
#include "common/ptr.h"
#include "image/bmp.h"
#include "graphics/surface.h"

namespace AGDS {
	ResourceManager::ResourceManager()
	{ }

	ResourceManager::~ResourceManager()
	{ }

	void ResourceManager::decrypt(uint8 * data, unsigned size) {
		static const char * kKey = "Vyvojovy tym AGDS varuje: Hackerovani skodi obchodu!";
		const char *ptr = kKey;
		while(size--) {
			*data++ ^= 0xff ^ *ptr++;
			if (*ptr == 0)
				ptr = kKey;
		}
	}

	bool ResourceManager::addPath(const Common::String &grpFilename) {
		static const char *	kSignature = "AGDS group file\x1a";
		static const uint32	kMagic = 0x1a03c9e6;
		static const uint32	kVersion1 = 44;
		static const uint32	kVersion2 = 2;

		debug("adding path %s", grpFilename.c_str());
		Common::File grp;
		if (!grp.open(grpFilename)) {
			error("failing opening grp file %s", grpFilename.c_str());
			return false;
		}
		uint8 header[0x2c];
		if (grp.read(header, sizeof(header)) != sizeof(header))
			return false;

		decrypt(header, 0x10);
		if (strncmp(reinterpret_cast<const char*>(header), kSignature, 0x10) != 0)
			return false;

		Common::MemoryReadStreamEndian reader(header + 0x10, sizeof(header) - 0x10, false);
		uint32 version1 = reader.readUint32();
		if (version1 != kVersion1) {
			error("invalid version 1 (%d)", version1);
			return false;
		}

		uint32 magic = reader.readUint32();
		if (magic != kMagic) {
			error("invalid magic (0x%08x)", magic);
			return false;
		}

		uint32 version2 = reader.readUint32();
		if (version2 != kVersion2) {
			error("invalid version 2 (%d)", version2);
			return false;
		}

		unsigned dirCount = reader.readUint32();
		if (!reader.skip(3 * 4))
			return false;

		GrpFilePtr grpFile(new GrpFile(grpFilename));

		debug("+%u files in index", dirCount);
		while(dirCount--) {
			uint8 dirData[0x31];
			uint8 * dirDataEnd = dirData + sizeof(dirData);

			if (grp.read(dirData, sizeof(dirData)) != sizeof(dirData)) {
				error("short read, corrupted file");
				return false;
			}

			uint8 *nameEnd = Common::find(dirData, dirDataEnd, 0);
			if (nameEnd == dirDataEnd) {
				error("corrupted entry at %d", (int)grp.pos() - 0x31);
				continue;
			}

			unsigned nameLength = nameEnd - dirData;
			decrypt(dirData, nameLength);
			Common::String name(reinterpret_cast<char *>(dirData), nameLength);

			Common::MemoryReadStreamEndian dirReader(dirData + 0x21, 8, false);

			uint32 offset	= dirReader.readSint32();
			uint32 size		= dirReader.readSint32();
			//debug("\t\tfile %s %u %u", name.c_str(), offset, size);
			ResourcePtr resource(new Resource(grpFile, offset, size));
			_resources.setVal(name, resource);
		}

		debug("\t%u files in index", _resources.size());
		return true;
	}

	Common::SeekableReadStream * ResourceManager::getResource(const Common::String &name) const
	{
		ResourcesType::const_iterator i = _resources.find(name);
		if (i == _resources.end()) {
			error("no resource %s could be found", name.c_str());
			return NULL;
		}

		const ResourcePtr & resource = i->_value;
		assert(resource);

		const Common::String & filename = resource->grp->filename;
		Common::File grp;
		if (!grp.open(filename)) {
			error("could not open group file %s", filename.c_str());
			return NULL;
		}

		grp.seek(resource->offset);
		return grp.readStream(resource->size);
	}

	const Graphics::Surface * ResourceManager::loadPicture(const Common::String & name, const Graphics::PixelFormat &format) {
		Common::SeekableReadStream *stream = getResource(name);
		if (!stream)
			return NULL;

		Image::BitmapDecoder bmp;
		const Graphics::Surface * surface = bmp.loadStream(*stream)? bmp.getSurface()->convertTo(format): NULL;
		return surface;
	}


} // End of namespace AGDS
