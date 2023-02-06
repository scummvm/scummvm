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

#ifndef TETRAEDGE_GAME_OBJECT3D_H
#define TETRAEDGE_GAME_OBJECT3D_H

#include "common/str.h"
#include "common/hashmap.h"

#include "tetraedge/te/te_object.h"
#include "tetraedge/te/te_model.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class Object3D : public TeObject {
public:
	struct ObjectSettings {
		Common::String _name;
		Common::String _modelFileName;
		TeVector3f32 _defaultScale;
		TeVector3f32 _originOffset;
		bool _invertNormals;

		void clear();
	};

	Object3D();

	bool loadModel(const Common::String &name);

	static bool loadSettings(const Common::String &path);
	static void cleanup();

	TeIntrusivePtr<TeModel> model() { return _modelPtr; }

	float _rotateTime;
	TeTimer _rotateTimer;
	TeQuaternion _rotateStart;
	TeVector3f32 _rotateAmount;

	float _translateTime;
	TeTimer _translateTimer;
	TeVector3f32 _translateStart;
	TeVector3f32 _translateAmount;

	Common::String _onCharName;
	Common::String _onCharBone;

	// TRS relative to the character this object is "on"
	TeVector3f32 _objTranslation;
	TeQuaternion _objRotation;
	TeVector3f32 _objScale;

	int _startFrame;
	int _endFrame;

private:
	static Common::HashMap<Common::String, ObjectSettings> *_objectSettings;

	TeIntrusivePtr<TeModel> _modelPtr;
	Common::String _modelFileName;
	TeVector3f32 _defaultScale;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_OBJECT3D_H
