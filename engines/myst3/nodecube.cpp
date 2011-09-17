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

#include "common/debug.h"

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
	movie.bink = new Video::BinkDecoder();
	movie.bink->loadStream(binkStream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glGenTextures(1, &movie.texture);

	glBindTexture(GL_TEXTURE_2D, movie.texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _cubeTextureSize, _cubeTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	_movies.push_back(movie);
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

	for (uint i = 0; i < _movies.size(); i++) {
		const Myst3::Movie &movie = _movies[i];

		if(movie.bink->endOfVideo())
			continue;

		const float w = movie.bink->getWidth() / (float)(_cubeTextureSize);
		const float h = movie.bink->getHeight() / (float)(_cubeTextureSize);

		glBindTexture(GL_TEXTURE_2D, _movies[i].texture);

		if (movie.bink->needsUpdate()) {
			const Graphics::Surface *frame = movie.bink->decodeNextFrame();
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->w, frame->h, GL_RGBA, GL_UNSIGNED_BYTE, frame->pixels);
		}

		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0, 0);
			glVertex3f(-movie.pTopLeft.x(), movie.pTopLeft.y(), movie.pTopLeft.z());

			glTexCoord2f(0, h);
			glVertex3f(-movie.pBottomLeft.x(), movie.pBottomLeft.y(), movie.pBottomLeft.z());

			glTexCoord2f(w, 0);
			glVertex3f(-movie.pTopRight.x(), movie.pTopRight.y(), movie.pTopRight.z());

			glTexCoord2f(w, h);
			glVertex3f(-movie.pBottomRight.x(), movie.pBottomRight.y(), movie.pBottomRight.z());
		glEnd();
	}

}

void NodeCube::unload() {

	for (uint i = 0; i < _movies.size(); i++) {
		const Myst3::Movie &movie = _movies[i];

		delete movie.bink;
		glDeleteTextures(1, &movie.texture);
	}

	Node::unload();
}

} /* namespace Myst3 */
