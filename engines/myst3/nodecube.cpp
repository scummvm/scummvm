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

#include "engines/myst3/nodecube.h"

namespace Myst3 {

NodeCube::NodeCube() {

}

NodeCube::~NodeCube() {
}

void NodeCube::load(Archive &archive, uint16 index) {
	for (int i = 0; i < 6; i++) {
		const DirectorySubEntry *jpegDesc = archive.getDescription(index, i + 1, DirectorySubEntry::kCubeFace);
		Common::MemoryReadStream *jpegStream = archive.getData(jpegDesc);

		if (jpegStream) {
			Graphics::JPEG jpeg;
			jpeg.read(jpegStream);

			setFaceTextureJPEG(i, &jpeg);

			delete jpegStream;
		}
	}

	// HACK: To load some of the movies of a frame
	loadMovie(archive, index + 10000);
	loadMovie(archive, index + 11000);
	loadMovie(archive, index + 20000);
}

void NodeCube::loadMovie(Archive &archive, uint16 id) {
	static const float scale = 50.0f;

	const DirectorySubEntry *binkDesc = archive.getDescription(id, 0, DirectorySubEntry::kMovie);

	if (!binkDesc)
		return;

	Common::MemoryReadStream *binkStream = archive.getData(binkDesc);
	const VideoData &videoData = binkDesc->getVideoData();

	Graphics::Vector3d planeDirection = videoData.v1;
	planeDirection.normalize();

	Graphics::Vector3d u;
	u.set(planeDirection.z(), 0.0f, -planeDirection.x());
	u.normalize();

	Graphics::Vector3d v = Graphics::cross(planeDirection, u);
	v.normalize();

	Graphics::Vector3d planeOrigin = planeDirection * scale;

	float left = (videoData.u - 320) * 0.003125f;
	float right = (videoData.u + videoData.width - 320) * 0.003125f;
	float top = (320 - videoData.v) * 0.003125f;
	float bottom = (320 - videoData.v - videoData.height) * 0.003125f;

	Graphics::Vector3d vLeft = scale * left * u;
	Graphics::Vector3d vRight = scale * right * u;
	Graphics::Vector3d vTop = scale * top * v;
	Graphics::Vector3d vBottom = scale * bottom * v;

	Movie movie;

	movie.pTopLeft = planeOrigin + vTop + vLeft;
	movie.pBottomLeft = planeOrigin + vBottom + vLeft;
	movie.pBottomRight = planeOrigin + vBottom + vRight;
	movie.pTopRight = planeOrigin + vTop + vRight;

	_movies.push_back(movie);

	delete binkStream;
}

void NodeCube::draw() {
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

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	for (uint i = 0; i < _movies.size(); i++) {
		glBegin(GL_POLYGON);
			glColor4f(0.0f, 0.5f, 0.0f, 0.2f);
			glVertex3f(-_movies[i].pTopLeft.x(), _movies[i].pTopLeft.y(), _movies[i].pTopLeft.z());
			glVertex3f(-_movies[i].pBottomLeft.x(), _movies[i].pBottomLeft.y(), _movies[i].pBottomLeft.z());
			glVertex3f(-_movies[i].pBottomRight.x(), _movies[i].pBottomRight.y(), _movies[i].pBottomRight.z());
			glVertex3f(-_movies[i].pTopRight.x(), _movies[i].pTopRight.y(), _movies[i].pTopRight.z());
		glEnd();
	}

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

} /* namespace Myst3 */
