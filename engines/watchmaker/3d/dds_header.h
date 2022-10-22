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

#ifndef WATCHMAKER_DDSHEADER_H
#define WATCHMAKER_DDSHEADER_H

#include "common/ptr.h"
#include "common/stream.h"

namespace Watchmaker {

enum class DxtCompression : uint32 {
	UNCOMPRESSED = 0,
	DXT1 = MKTAG('1','T','X','D'),
	DXT2 = MKTAG('2','T','X','D'),
	DXT3 = MKTAG('3','T','X','D'),
	DXT4 = MKTAG('4','T','X','D'),
	DXT5 = MKTAG('5','T','X','D')
};

class TextureData {
public:
	DxtCompression _compression;
	TextureData(DxtCompression compression) : _compression(compression) {}
	virtual ~TextureData() {}
	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
	virtual int getDataSize() const = 0;
	virtual const void *getData() const = 0;
};

class Texture {
public:
	virtual ~Texture() {}
	virtual void assignData(const TextureData &data) = 0;
	virtual void bind() = 0;
};


struct DDSHeader {
	DDSHeader() {}
	DDSHeader(Common::SeekableReadStream &stream);
	int height = 0;
	int width = 0;
	uint32 dataSize() const;
	DxtCompression compression = DxtCompression::UNCOMPRESSED;
};

//Common::SharedPtr<Texture> loadTgaTextureData(Common::SeekableReadStream &stream);
Common::SharedPtr<TextureData> loadDdsTexture(Common::SeekableReadStream& stream, DDSHeader &header);
Common::SharedPtr<TextureData> loadDdsTexture(Common::SeekableReadStream& stream);

} // End of namespace Watchmaker

#endif // WATCHMAKER_DDSHEADER_H
