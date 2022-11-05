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

#include "tetraedge/te/te_scene.h"

namespace Tetraedge {

TeScene::TeScene() : _currentCameraIndex(0) {
}

void TeScene::close() {
	_cameras.clear();
	_models.clear();
}

TeIntrusivePtr<TeCamera> TeScene::camera(const Common::String &cname) {
	for (auto &c : _cameras) {
		if (c->name() == cname)
			return c;
	}
	return TeIntrusivePtr<TeCamera>();
}

TeIntrusivePtr<TeModel> TeScene::model(const Common::String &mname) {
	for (auto &m : _models) {
		if (m->name() == mname)
			return m;
	}
	return TeIntrusivePtr<TeModel>();
}

TeIntrusivePtr<TeCamera> TeScene::currentCamera() {
	if (!_cameras.size() || _currentCameraIndex >= _cameras.size())
		return TeIntrusivePtr<TeCamera>();
	return _cameras[_currentCameraIndex];
}

Common::String TeScene::currentCameraName() const {
	if (_currentCameraIndex < _cameras.size())
		return _cameras[_currentCameraIndex]->name();
	else
		return Common::String("");
}

void TeScene::draw() {
	if (_currentCameraIndex >= _cameras.size())
		return;

	currentCamera()->apply();
	for (auto &m : _models) {
		//debug("Draw scene model %s", m->name().c_str());
		m->draw();
	}
	TeCamera::restore();
}

void TeScene::removeModel(const Common::String &mname) {
	uint n = _models.size();
	for (uint i = 0; i < n; i++) {
		if (_models[i]->name() == mname) {
			_models.remove_at(i);
			break;
		}
	}
}

void TeScene::setCurrentCamera(const Common::String &cname) {
	uint n = _cameras.size();
	uint i = 0;
	for (; i < n; i++) {
		if (_cameras[i]->name() == cname) {
			break;
		}
	}
	if (i == n) {
		debug("TeScene::setCurrentCamera: Couldn't find camera %s", cname.c_str());
		return;
	}
	_currentCameraIndex = i;
	TeCamera *c = _cameras[i].get();
	assert(c);
	// debug("TeScene::setCurrentCamera: Set %s", c->name().c_str());
}

void TeScene::update() {
	for (auto &m : _models) {
		m->update();
	}
}

} // end namespace Tetraedge
