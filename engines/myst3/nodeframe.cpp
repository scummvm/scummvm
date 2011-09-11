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

#include "engines/myst3/nodeframe.h"

namespace Myst3 {

NodeFrame::NodeFrame() {
}

NodeFrame::~NodeFrame() {
}

void NodeFrame::load(Archive &archive, uint16 index) {
	const DirectorySubEntry *jpegDesc = archive.getDescription(index, 1, DirectorySubEntry::kFrame);
	Common::MemoryReadStream *jpegStream = archive.getData(jpegDesc);

	if (jpegStream) {
		Graphics::JPEG jpeg;
		jpeg.read(jpegStream);

		setFaceTextureJPEG(0, &jpeg);

		delete jpegStream;
	}
}

void NodeFrame::draw() {
	// Size of the frame
	float t = 0.85f;
	float s = 360.0f / 640.0f * t;

	// Used fragment of texture
	float u = 640 / (float)_cubeTextureSize;
	float v = 360 / (float)_cubeTextureSize;

	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, _cubeTextures[0]);
	glBegin(GL_TRIANGLE_STRIP);			// Z+
		glTexCoord2f(0, v); glVertex3f( t,-s, 1.0f);
		glTexCoord2f(u, v); glVertex3f(-t,-s, 1.0f);
		glTexCoord2f(0, 0); glVertex3f( t, s, 1.0f);
		glTexCoord2f(u, 0); glVertex3f(-t, s, 1.0f);
	glEnd();

	glDepthMask(GL_TRUE);
}

} /* namespace Myst3 */
