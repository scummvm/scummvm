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

#include "common/textconsole.h"
#include "common/ptr.h"
#include "watchmaker/3d/dds_header.h"

namespace Watchmaker {

DDSHeader::DDSHeader(Common::SeekableReadStream &stream) {
	//warning("TODO: Implement DDS Header parsing");
	uint32 retv = MKTAG(' ', 'S', 'D', 'D');
	uint32 magic = stream.readUint32LE();
	if (magic != retv) {
		error("parseDDSHeader: Wrong Magic, expected %08X, got %08X\n", retv, magic);
	}
	// The size counts the datastructure, which doesn't include the magic
	int initialPos = stream.pos();
	uint32 size = stream.readUint32LE();
	uint32 flags = stream.readUint32LE();
	this->height = stream.readUint32LE();
	this->width = stream.readUint32LE();
	uint32 dataSize = stream.readUint32LE();
	stream.seek(13 * 4, SEEK_CUR);
	stream.readUint32LE();
	uint32 pfFlags = stream.readUint32LE();
	assert(pfFlags & 0x4); // For now we assume compressed DDS only.
	compression = (DxtCompression)stream.readUint32LE();
	// Since we're ignoring a fair amount of header, we still need to position
	// ourselves as if we read it.
	stream.seek(initialPos + size, SEEK_SET);
}

uint32 blockSize(DxtCompression compression) {
	switch (compression) {
	case DxtCompression::DXT1:
		return 8;
	default:
		return 16;
	}
}

uint32 DDSHeader::dataSize() const {
	int blockCount = ceil(width / 4.0) * ceil(height / 4.0);
	return blockCount * blockSize(compression);
}

class DDSTextureData : public TextureData {
private:
	unsigned int _dataSize = 0;
	byte *_data = nullptr;
	DDSHeader _header;
public:
	DDSTextureData(byte *data, uint32 dataSize, DDSHeader header) : TextureData(header.compression),
		_data(data),
		_dataSize(dataSize),
		_header(header) {}
	~DDSTextureData() override {
		delete[] _data;
	}
	DxtCompression _compression;
	int getWidth() const override {
		return _header.width;
	}
	int getHeight() const override {
		return _header.height;
	}
	int getDataSize() const override {
		return _dataSize;
	}
	const void *getData() const override {
		return _data;
	}
};

Common::SharedPtr<TextureData> loadDdsTexture(Common::SeekableReadStream &stream) {
	DDSHeader header(stream);
	return loadDdsTexture(stream, header);
}

Common::SharedPtr<TextureData> loadDdsTexture(Common::SeekableReadStream &stream, DDSHeader &header) {
	assert(header.width > 0);
	unsigned char *data = new unsigned char[header.dataSize()]();
	stream.read(data, header.dataSize());
	return Common::SharedPtr<TextureData>(new DDSTextureData(data, header.dataSize(), header));
}

} // End of namespace Watchmaker
