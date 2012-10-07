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

#include "common/file.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "graphics/surface.h"
#include "nancy/resource.h"

namespace Nancy {

ResourceManager::ResourceManager(NancyEngine *vm) : _vm(vm) {
}

ResourceManager::~ResourceManager() {
}

void ResourceManager::initialize() {
	Common::File cifTree;

	if (!cifTree.open("ciftree.dat"))
		error("Failed to open ciftree.dat");

	char id[20];
	cifTree.read(id, 20);
	id[19] = 0;

	if (cifTree.eos() || Common::String(id) != "CIF TREE WayneSikes")
		error("Invalid id string found in ciftree.dat");

	// 4 bytes unused
	cifTree.skip(4);

	// Probably some kind of version number
	uint16 ver[2];
	ver[0] = cifTree.readUint16LE();
	ver[1] = cifTree.readUint16LE();

	if (cifTree.eos() || ver[0] != 2 || ver[1] != 0)
		error("ciftree.dat version %i.%i not supported", ver[0], ver[1]);

	// Number of info blocks
	int infoBlockCount = cifTree.readUint16LE();

	for (int i = 0; i < kHashMapSize; i++)
		_hashMap[i] = cifTree.readUint16LE();

	if (cifTree.eos())
		error("Failed to read hash map from ciftree.dat");

	for (int i = 0; i < infoBlockCount; i++) {
		ResInfo info;

		char name[9];
		cifTree.read(name, 9);
		name[8] = 0;
		info.name = name;

		cifTree.skip(2); // Index of this block
		info.width = cifTree.readUint16LE();
		info.pitch = cifTree.readUint16LE();
		info.height = cifTree.readUint16LE();
		info.depth = cifTree.readByte();

		info.comp = cifTree.readByte();
		info.dataOffset = cifTree.readUint32LE();
		info.size = cifTree.readUint32LE();
		info.sizeUnk = cifTree.readUint32LE(); // Unknown
		info.compressedSize = cifTree.readUint32LE();

		info.type = cifTree.readByte();

		info.next = cifTree.readUint16LE();
		if (cifTree.eos())
			error("Failed to read info blocks from ciftree.dat");

		_resInfo.push_back(info);
	}

	cifTree.close();
}

byte *ResourceManager::loadResource(const Common::String name, uint &size) {
	const ResInfo *res = findResource(name);

	if (!res) {
		debug("Resource '%s' not found", name.c_str());
		return 0;
	}

	byte *buf = decompress(*res, size);
	return buf;
}

bool ResourceManager::loadImage(const Common::String name, Graphics::Surface &surf) {
	const ResInfo *res = findResource(name);

	if (!res) {
		debug("Resource '%s' not found", name.c_str());
		return 0;
	}

	if (res->type != kResTypeImage) {
		debug("Resource '%s' is not an image", name.c_str());
		return 0;
	}

	if (res->depth != 16) {
		debug("Image '%s' has unsupported depth %i", name.c_str(), res->depth);
		return 0;
	}

	uint size;
	byte *buf = decompress(*res, size);

	Graphics::PixelFormat format(2, 5, 5, 5, 0, 10, 5, 0, 0);
	surf.w = res->width;
	surf.h = res->height;
	surf.pitch = res->pitch;
	surf.setPixels(buf);
	surf.format = format;
	return true;
}

const ResourceManager::ResInfo *ResourceManager::findResource(const Common::String name) {
	uint hash = 0;

	for (uint i = 0; i < name.size(); i++)
		hash += name[i];

	hash &= kHashMapSize - 1;

	uint16 index = _hashMap[hash];
	while (index != 0xffff) {
		const ResInfo &info = _resInfo[index];
		if (name == info.name)
			return &_resInfo[index];
		index = info.next;
	}

	return 0;
}

byte *ResourceManager::decompress(const ResInfo &res, uint &size) {
	// TODO: clean this up...
	Common::File cifTree;
	uint read = 0, written = 0;

	if (res.comp != kResCompression) {
		debug("Resource '%s' is not compressed", res.name.c_str());
		return 0;
	}

	if (!cifTree.open("ciftree.dat"))
		error("Failed to open ciftree.dat");

	cifTree.seek(res.dataOffset);

	const int bufSize = 4096;
	const int bufStart = 4078;
	byte *buf = new byte[bufSize];
	byte *output = new byte[res.size];
	uint bufpos = bufStart;

	memset(buf, ' ', bufStart);
	uint16 bits = 0;
	byte val = 0;

	while(1) {
		bits >>= 1;

		// The highest 8 bits are used to keep track of how many bits are left to process
		if (!(bits & 0x100)) {
			// Out of bits
			if (cifTree.eos() || read == res.compressedSize)
				break;
			bits = 0xff00 | ((cifTree.readByte() - val++) & 0xff);
			++read;
		}

		if (bits & 1) {
			// Literal byte
			if (cifTree.eos() || read == res.compressedSize)
				break;
			byte b = cifTree.readByte() - val++;
			++read;
			output[written++] = b;
			if (written == res.size)
				break;
			buf[bufpos++] = b;
			bufpos &= bufSize - 1;
		} else {
			// Copy from buffer
			if (cifTree.eos() || read == res.compressedSize)
				break;
			byte b1 = cifTree.readByte() - val++;
			++read;

			if (cifTree.eos() || read == res.compressedSize)
				break;
			byte b2 = cifTree.readByte() - val++;
			++read;

			uint16 offset = b1 | ((b2 & 0xf0) << 4);
			uint16 len = (b2 & 0xf) + 3;
	
			for (uint i = 0; i < len; i++) {
				byte t = buf[(offset + i) & (bufSize - 1)];
				output[written++] = t;
				if (written == res.size)
					break;
				buf[bufpos++] = t;
				bufpos &=  bufSize - 1;
			}
		}
	}

	delete[] buf;

	if (read != res.compressedSize || written != res.size) {
		debug("Failed to decompress resource");
		return 0;
	}

	size = written;
	return output;
}

void ResourceManager::listResources(Common::Array<Common::String> &list, uint type) {
	for (uint i = 0; i < _resInfo.size(); i++) {
		if (type == kResTypeAny || _resInfo[i].type == type)
			list.push_back(_resInfo[i].name);
	}
}

Common::String ResourceManager::getResourceDesc(const Common::String name) {
	const ResInfo *info = findResource(name);

	if (!info)
		return Common::String::format("Resource '%s' not found\n", name.c_str());

	Common::String desc;
	desc = Common::String::format("Name: %s\n", info->name.c_str());
	desc += Common::String::format("Type: %i\n", info->type);
	desc += Common::String::format("Compression: %i\n", info->comp);
	desc += Common::String::format("Data offset: %i\n", info->dataOffset);
	desc += Common::String::format("Size: %i\n", info->size);
	desc += Common::String::format("Size (unknown): %i\n", info->sizeUnk);
	desc += Common::String::format("Compressed size: %i\n", info->compressedSize);
	desc += Common::String::format("Width: %i\n", info->width);
	desc += Common::String::format("Pitch: %i\n", info->pitch);
	desc += Common::String::format("Height: %i\n", info->height);
	desc += Common::String::format("Bit depth: %i\n", info->depth);
	return desc;
}

} // End of namespace Nancy
