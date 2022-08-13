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

#include "common/str.h"
#include "common/array.h"

#include "tetraedge/te/te_animation.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector3f32.h"

#ifndef TETRAEDGE_TE_TE_MODEL_VERTEX_ANIMATION_H
#define TETRAEDGE_TE_TE_MODEL_VERTEX_ANIMATION_H

namespace Tetraedge {

class TeModelVertexAnimation : public TeAnimation, public TeResource {
public:
	struct KeyData {
		float _f;
		Common::Array<TeVector3f32> _vectors;
		Common::Array<TeMatrix4x4> _matricies;
	};

	TeModelVertexAnimation();

	const Common::String &head() const { return _head; }

private:
	Common::String _head;
	Common::Array<KeyData> _keydata;

	// TODO add private members

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MODEL_VERTEX_ANIMATION_H
