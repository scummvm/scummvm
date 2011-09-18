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

#ifndef MOVIE_H_
#define MOVIE_H_

#include "engines/myst3/node.h"

#include "math/vector3d.h"
#include "video/bink_decoder.h"

namespace Myst3 {

class Movie {
public:
	Movie(Archive &archive, uint16 id);
	virtual ~Movie();
	void draw();

private:
	static const int _movieTextureSize = 1024;

	Math::Vector3d _pTopLeft;
	Math::Vector3d _pBottomLeft;
	Math::Vector3d _pBottomRight;
	Math::Vector3d _pTopRight;
	Video::BinkDecoder _bink;
	GLuint _texture;
};

} /* namespace Myst3 */
#endif /* MOVIE_H_ */
