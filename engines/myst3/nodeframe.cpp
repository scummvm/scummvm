/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/nodeframe.h"
#include "engines/myst3/scene.h"

namespace Myst3 {

NodeFrame::NodeFrame(Myst3Engine *vm, Archive *archive, uint16 id) :
	Node(vm, archive, id) {
	const DirectorySubEntry *jpegDesc = archive->getDescription(id, 1, DirectorySubEntry::kFrame);
	Common::MemoryReadStream *jpegStream = archive->getData(jpegDesc);

	if (jpegStream) {
		Graphics::JPEG jpeg;
		jpeg.read(jpegStream);

		_faces[0] = new Face();
		_faces[0]->setTextureFromJPEG(&jpeg);
		_faces[0]->uploadTexture();

		delete jpegStream;
	}
}

NodeFrame::~NodeFrame() {
}

void NodeFrame::draw() {
	// Size of the frame
	static const float w = Scene::_originalWidth;
	static const float h = Scene::_frameHeight;

	// Position of the frame
	static const float top = Scene::_topBorderHeight;

	// Used fragment of texture
	const float u = w / (float)_cubeTextureSize;
	const float v = h / (float)_cubeTextureSize;

	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, _faces[0]->_textureId);
	glBegin(GL_TRIANGLE_STRIP);			// Z+
		glTexCoord2f(0, v); glVertex3f( 0, top + h, 1.0f);
		glTexCoord2f(u, v); glVertex3f( w, top + h, 1.0f);
		glTexCoord2f(0, 0); glVertex3f( 0, top + 0, 1.0f);
		glTexCoord2f(u, 0); glVertex3f( w, top + 0, 1.0f);
	glEnd();

	glDepthMask(GL_TRUE);
}

} /* namespace Myst3 */
