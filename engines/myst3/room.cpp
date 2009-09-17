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
 * $URL$
 * $Id$
 *
 */

#include "engines/myst3/room.h"

namespace Myst3 {

void Room::setFaceTextureJPEG(int face, Graphics::JPEG *jpeg) {	
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

void Room::setFaceTextureRGB(int face, Graphics::Surface *texture) {
	glGenTextures(1, &_cubeTextures[face]);

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[face]);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, _cubeTextureSize, _cubeTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->w, texture->h, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Room::load(Archive &archive, uint16 index) {
	for (int i = 0; i < 6; i++) {
		Common::MemoryReadStream *jpegStream = archive.dumpToMemory(index, i + 1, DirectorySubEntry::kFaceTexture);

		if (jpegStream) {
			Graphics::JPEG jpeg;
			jpeg.read(jpegStream);

			setFaceTextureJPEG(i, &jpeg);
		}
	}
}

void Room::draw() {
	// Size of the cube
	float t = 1.0f;

	// Used fragment of the textures
	float s = 640 / (float)_cubeTextureSize;

	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[4]);
	glBegin(GL_TRIANGLE_STRIP);			// X-
		glTexCoord2f(0, s); glVertex3f(-t,-t, t); 	
		glTexCoord2f(s, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f(-t, t, t);
		glTexCoord2f(s, 0); glVertex3f(-t, t,-t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[3]);
	glBegin(GL_TRIANGLE_STRIP);			// X+
		glTexCoord2f(0, s); glVertex3f( t,-t,-t);
		glTexCoord2f(s, s); glVertex3f( t,-t, t);
		glTexCoord2f(0, 0); glVertex3f( t, t,-t); 
		glTexCoord2f(s, 0); glVertex3f( t, t, t); 	
	glEnd();

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[1]);
	glBegin(GL_TRIANGLE_STRIP);			// Y-
		glTexCoord2f(0, s); glVertex3f( t,-t,-t);
		glTexCoord2f(s, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f( t,-t, t);
		glTexCoord2f(s, 0); glVertex3f(-t,-t, t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[5]);
	glBegin(GL_TRIANGLE_STRIP);			// Y+
		glTexCoord2f(0, s); glVertex3f( t, t, t);
		glTexCoord2f(s, s); glVertex3f(-t, t, t); 
		glTexCoord2f(0, 0); glVertex3f( t, t,-t);
		glTexCoord2f(s, 0); glVertex3f(-t, t,-t); 	
	glEnd();

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[0]);
	glBegin(GL_TRIANGLE_STRIP);			// Z-
		glTexCoord2f(0, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(s, s); glVertex3f( t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f(-t, t,-t);
		glTexCoord2f(s, 0); glVertex3f( t, t,-t); 
	glEnd();

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[2]);
	glBegin(GL_TRIANGLE_STRIP);			// Z+
		glTexCoord2f(0, s); glVertex3f( t,-t, t);
		glTexCoord2f(s, s); glVertex3f(-t,-t, t);
		glTexCoord2f(0, 0); glVertex3f( t, t, t);
		glTexCoord2f(s, 0); glVertex3f(-t, t, t);
	glEnd();
	
	glDepthMask(GL_TRUE);
}

} // end of namespace Myst3
