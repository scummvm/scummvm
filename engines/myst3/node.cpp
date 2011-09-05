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

void Node::setFaceTextureJPEG(int face, Graphics::JPEG *jpeg) {
	Graphics::Surface texture;
	texture.create(jpeg->getComponent(1)->w, jpeg->getComponent(1)->h, Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	byte *y = (byte *)jpeg->getComponent(1)->getBasePtr(0, 0);
	byte *u = (byte *)jpeg->getComponent(2)->getBasePtr(0, 0);
	byte *v = (byte *)jpeg->getComponent(3)->getBasePtr(0, 0);
	
	byte *ptr = (byte *)texture.getBasePtr(0, 0);
	for (int i = 0; i < texture.w * texture.h; i++) {
		byte r, g, b;
		Graphics::YUV2RGB(*y++, *u++, *v++, r, g, b);
		*ptr++ = r;
		*ptr++ = g;
		*ptr++ = b;
	}
	
	setFaceTextureRGB(face, &texture);
	
	texture.free();
}

void Node::setFaceTextureRGB(int face, Graphics::Surface *texture) {
	glGenTextures(1, &_cubeTextures[face]);

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[face]);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, _cubeTextureSize, _cubeTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->w, texture->h, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Node::dumpFaceMask(Archive &archive, uint16 index, int face) {
	byte *mask = new byte[640 * 640];
	memset(mask, 0, sizeof(mask));
	uint32 headerOffset = 0;
	uint32 dataOffset = 0;

	Common::MemoryReadStream *maskStream = archive.dumpToMemory(index, face, DirectorySubEntry::kFaceMask);

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

	Common::DumpFile outFile;
	outFile.open("dump/1-1.masku");
	outFile.write(mask, sizeof(mask));
	outFile.close();
	delete[] mask;
}

void Node::unload() {
	for (int i = 0; i < 6; i++) {
		glDeleteTextures(1, &_cubeTextures[i]);
	}
}

} // end of namespace Myst3
