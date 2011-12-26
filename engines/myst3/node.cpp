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

#include "engines/myst3/node.h"

#include "common/debug.h"
#include "common/rect.h"

namespace Myst3 {

void Face::setTextureFromJPEG(Graphics::JPEG *jpeg) {
	_bitmap = new Graphics::Surface();
	_bitmap->create(jpeg->getComponent(1)->w, jpeg->getComponent(1)->h, Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	byte *y = (byte *)jpeg->getComponent(1)->getBasePtr(0, 0);
	byte *u = (byte *)jpeg->getComponent(2)->getBasePtr(0, 0);
	byte *v = (byte *)jpeg->getComponent(3)->getBasePtr(0, 0);
	
	byte *ptr = (byte *)_bitmap->getBasePtr(0, 0);
	for (int i = 0; i < _bitmap->w * _bitmap->h; i++) {
		byte r, g, b;
		Graphics::YUV2RGB(*y++, *u++, *v++, r, g, b);
		*ptr++ = r;
		*ptr++ = g;
		*ptr++ = b;
	}
}

void Face::createTexture() {
	glGenTextures(1, &_textureId);

	glBindTexture(GL_TEXTURE_2D, _textureId);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, Node::_cubeTextureSize, Node::_cubeTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Face::uploadTexture() {
	glBindTexture(GL_TEXTURE_2D, _textureId);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _bitmap->w, _bitmap->h, GL_RGB, GL_UNSIGNED_BYTE, _bitmap->pixels);
}

void Face::unload() {
	_bitmap->free();
	delete _bitmap;
	_bitmap = 0;

	glDeleteTextures(1, &_textureId);
}

void Node::dumpFaceMask(Archive &archive, uint16 index, int face) {
	byte *mask = new byte[640 * 640];
	memset(mask, 0, sizeof(mask));
	uint32 headerOffset = 0;
	uint32 dataOffset = 0;

	const DirectorySubEntry *maskDesc = archive.getDescription(index, face, DirectorySubEntry::kFaceMask);
	Common::MemoryReadStream *maskStream = archive.getData(maskDesc);

	while (headerOffset < 400) {
		int blockX = (headerOffset / sizeof(dataOffset)) % 10;
		int blockY = (headerOffset / sizeof(dataOffset)) / 10;

		maskStream->seek(headerOffset, SEEK_SET);
		dataOffset = maskStream->readUint32LE();
		headerOffset = maskStream->pos();

		if (dataOffset != 0) {
			maskStream->seek(dataOffset, SEEK_SET);

			for(int i = 63; i >= 0; i--) {
				int x = 0;
				byte numValues = maskStream->readByte();
				for (int j = 0; j < numValues; j++) {
					byte repeat = maskStream->readByte();
					byte value = maskStream->readByte();
					for (int k = 0; k < repeat; k++) {
						mask[((blockY * 64) + i) * 640 + blockX * 64 + x] = value;
						x++;
					}
				}
			}
		}
	}

	delete maskStream;

	Common::DumpFile outFile;
	outFile.open("dump/1-1.masku");
	outFile.write(mask, sizeof(mask));
	outFile.close();
	delete[] mask;
}

void Node::unload() {
	for (int i = 0; i < 6; i++) {
		_faces[i].unload();
	}
}

} // end of namespace Myst3
