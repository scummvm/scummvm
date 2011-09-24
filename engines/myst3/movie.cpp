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

#include "engines/myst3/movie.h"

namespace Myst3 {

Movie::Movie(Archive *archive, uint16 id) {
	static const float scale = 50.0f;

	const DirectorySubEntry *binkDesc = archive->getDescription(id, 0, DirectorySubEntry::kMovie);

	if (!binkDesc)
		return;

	Common::MemoryReadStream *binkStream = archive->getData(binkDesc);
	const VideoData &videoData = binkDesc->getVideoData();

	Math::Vector3d planeDirection = videoData.v1;
	planeDirection.normalize();

	Math::Vector3d u;
	u.set(planeDirection.z(), 0.0f, -planeDirection.x());
	u.normalize();

	Math::Vector3d v = Math::cross(planeDirection, u);
	v.normalize();

	Math::Vector3d planeOrigin = planeDirection * scale;

	float left = (videoData.u - 320) * 0.003125f;
	float right = (videoData.u + videoData.width - 320) * 0.003125f;
	float top = (320 - videoData.v) * 0.003125f;
	float bottom = (320 - videoData.v - videoData.height) * 0.003125f;

	Math::Vector3d vLeft = scale * left * u;
	Math::Vector3d vRight = scale * right * u;
	Math::Vector3d vTop = scale * top * v;
	Math::Vector3d vBottom = scale * bottom * v;

	_pTopLeft = planeOrigin + vTop + vLeft;
	_pBottomLeft = planeOrigin + vBottom + vLeft;
	_pBottomRight = planeOrigin + vBottom + vRight;
	_pTopRight = planeOrigin + vTop + vRight;
	_bink.loadStream(binkStream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glGenTextures(1, &_texture);

	glBindTexture(GL_TEXTURE_2D, _texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _movieTextureSize, _movieTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Movie::draw() {
	if (_bink.endOfVideo())
		_bink.seekToFrame(_bink.getFrameCount() / 2);

	const float w = _bink.getWidth() / (float)(_movieTextureSize);
	const float h = _bink.getHeight() / (float)(_movieTextureSize);

	glBindTexture(GL_TEXTURE_2D, _texture);

	if (_bink.needsUpdate()) {
		const Graphics::Surface *frame = _bink.decodeNextFrame();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->w, frame->h, GL_RGBA, GL_UNSIGNED_BYTE, frame->pixels);
	}

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0);
		glVertex3f(-_pTopLeft.x(), _pTopLeft.y(), _pTopLeft.z());

		glTexCoord2f(0, h);
		glVertex3f(-_pBottomLeft.x(), _pBottomLeft.y(), _pBottomLeft.z());

		glTexCoord2f(w, 0);
		glVertex3f(-_pTopRight.x(), _pTopRight.y(), _pTopRight.z());

		glTexCoord2f(w, h);
		glVertex3f(-_pBottomRight.x(), _pBottomRight.y(), _pBottomRight.z());
	glEnd();
}

Movie::~Movie() {
	glDeleteTextures(1, &_texture);
}

} /* namespace Myst3 */
