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

#ifndef TETRAEDGE_TE_TE_MODEL_VERTEX_ANIMATION_H
#define TETRAEDGE_TE_TE_MODEL_VERTEX_ANIMATION_H

#include "common/str.h"
#include "common/array.h"
#include "common/stream.h"

#include "tetraedge/te/te_animation.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_model.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeModel;
class TeModelAnimation;

class TeModelVertexAnimation : public TeAnimation, public TeResource {
public:
	struct KeyData {
		float _frame;
		Common::Array<TeVector3f32> _vectors;
		Common::Array<TeMatrix4x4> _matricies;
	};

	TeModelVertexAnimation();
	virtual ~TeModelVertexAnimation() {
		_keydata.clear();
	}

	void bind(TeIntrusivePtr<TeModel> &model);
	// void deleteLater() // original overrides this, but just calls the super.
	void destroy();

	const Common::String &head() const { return _head; }
	TeVector3f32 getKeyVertex(uint keyno, uint vertexno);
	Common::Array<TeVector3f32> getVertices();

	bool load(Common::ReadStream &stream);
	void save(Common::WriteStream &stream) const;

	void update(double millis) override;

	static void deleteLaterStatic(TeModelVertexAnimation *obj) { TeObject::deleteLaterStatic(obj); }

private:
	float _lastMillis;
	TeIntrusivePtr<TeModel> _model;
	TeModelAnimation *_modelAnim;
	TeQuaternion _rot;
	Common::String _head;
	Common::Array<KeyData> _keydata;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MODEL_VERTEX_ANIMATION_H
