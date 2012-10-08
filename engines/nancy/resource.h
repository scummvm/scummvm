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

#ifndef NANCY_RESOURCE_H
#define NANCY_RESOURCE_H

namespace Common {
class String;
}

namespace Graphics {
class Surface;
}

namespace Nancy {

class NancyEngine;
class CifTree;

class ResourceManager {
public:
	enum ResType {
		kResTypeAny,
		kResTypeImage = 2,
		kResTypeScript
	};

	enum ResCompression {
		kResCompressionNone = 1,
		kResCompression
	};

	struct CifInfo {
		Common::String name;
		byte type; // ResType
		byte comp; // ResCompression
		uint16 width, pitch, height;
		byte depth; // Bit depth
		uint32 compressedSize, size, sizeUnk;
		uint32 dataOffset;
	};

	ResourceManager(NancyEngine *vm);
	~ResourceManager();

	void initialize();
	bool loadCifTree(const Common::String filename);
	byte *loadResource(const Common::String name, uint &size);
	bool loadImage(const Common::String name, Graphics::Surface &surf);

	// Debugger functions
	void listResources(Common::Array<Common::String> &list, uint type);
	Common::String getResourceDesc(const Common::String name);
private:
	NancyEngine *_vm;
	const CifTree *_cifTree;

	byte *decompress(const CifInfo &res, uint &size);
};

} // End of namespace Nancy

#endif
