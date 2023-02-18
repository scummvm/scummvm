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

#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/object3d.h"
#include "tetraedge/game/object_settings_xml_parser.h"

#include "tetraedge/te/te_lua_script.h"

namespace Tetraedge {

/*static*/
Common::HashMap<Common::String, Object3D::ObjectSettings> *Object3D::_objectSettings = nullptr;

/*static*/
void Object3D::cleanup() {
	if (_objectSettings)
		delete _objectSettings;
	_objectSettings = nullptr;
}


// start and end frames not initialized in original, but to guarantee we don't use
// uninitialized values we set it here.
Object3D::Object3D() : _translateTime(-1), _rotateTime(-1), _objScale(1.0f, 1.0f, 1.0f),
_startFrame(-1), _endFrame(9999) {
}

bool Object3D::loadModel(const Common::String &name) {
	_modelPtr = new TeModel();
	Common::HashMap<Common::String, ObjectSettings>::iterator settings = _objectSettings->find(name);
	if (settings != _objectSettings->end()) {
		_modelFileName = settings->_value._modelFileName;
		_defaultScale = settings->_value._defaultScale;
		_modelPtr->setTexturePath("objects/Textures");
		bool loaded = _modelPtr->load(Common::Path("objects").join(_modelFileName).toString());
		if (loaded) {
			_modelPtr->setName(name);
			_modelPtr->setScale(_defaultScale);
			return true;
		}
	}
	return false;
}

void Object3D::setObjectMoveDest(const TeVector3f32 &vec) {
	_moveAnim._startVal = TeVector3f32();
	_moveAnim._endVal = vec;
}

void Object3D::setObjectMoveTime(float time) {
	_moveAnim._duration = time * 1000;
	_moveAnim._callbackObj = this;
	Common::Array<float> curve;
	curve.push_back(0.0f);
	curve.push_back(1.0f);
	_moveAnim.setCurve(curve);
	_moveAnim.onFinished().remove(this, &Object3D::onMoveAnimFinished);
	_moveAnim.onFinished().add(this, &Object3D::onMoveAnimFinished);
	_moveAnim.play();
}

bool Object3D::onMoveAnimFinished() {
	g_engine->getGame()->luaScript().execute("OnObjectMoveFinished", _modelPtr->name());
	_moveAnim.onFinished().remove(this, &Object3D::onMoveAnimFinished);
	return false;
}

void Object3D::setCurMovePos(const TeVector3f32 &vec) {
	_curMovePos = vec;
}

/*static*/
bool Object3D::loadSettings(const Common::String &path) {
	ObjectSettingsXmlParser parser;
	parser.setAllowText();

	if (_objectSettings)
		delete _objectSettings;
	_objectSettings = new Common::HashMap<Common::String, ObjectSettings>();
	parser.setObjectSettings(_objectSettings);

	if (!parser.loadFile(path))
		error("Object3D::loadSettings: Can't load %s", path.c_str());
	if (!parser.parse())
		error("Object3D::loadSettings: Can't parse %s", path.c_str());
	parser.finalize();

	return true;
}

void Object3D::ObjectSettings::clear() {
	_name.clear();
	_modelFileName.clear();
	_defaultScale = TeVector3f32();
	_originOffset = TeVector3f32();
	_invertNormals = false;
}


} // end namespace Tetraedge
