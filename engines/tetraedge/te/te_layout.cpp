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
#include "tetraedge/game/application.h"
#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_i_3d_object2.h"

namespace Tetraedge {

TeLayout::TeLayout() : Te3DObject2(), _autoz(true), _needZUpdate(true), _updatingZ(false),
	_needZSizeUpdate(true), _updatingZSize(false), _sizeChanged(true), _updatingSize(false),
	_positionChanged(true), _updatingPosition(false), _worldMatrixChanged(true),
	_updatingWorldMatrix(false), _drawMode(TeILayout::DrawMode0),
	_sizeType(CoordinatesType::ABSOLUTE), _userSize(1.0f, 1.0f, 1.0f),
	_anchor(0.5f, 0.5f, 0.5f), _ratio(1.0f), _safeAreaRatio(1.333333f),
	_ratioMode(RATIO_MODE_NONE), _positionType(CoordinatesType::RELATIVE_TO_PARENT)
{
	_userPosition = _position = TeVector3f32(0.5f, 0.5f, 0.5f);
	_size = TeVector3f32(1.0f, 1.0f, 1.0f);
	_onChildSizeChangedCallback.reset(
		new TeCallback0Param<TeLayout>(this, &TeLayout::onChildSizeChanged));
	_onParentSizeChangedCallback.reset(
		new TeCallback0Param<TeLayout>(this, &TeLayout::onParentSizeChanged));
	_onParentWorldTransformationMatrixChangedCallback.reset(
		new TeCallback0Param<TeLayout>(this, &TeLayout::onParentWorldTransformationMatrixChanged));

	updateSize();
	updateMesh();
}

TeLayout::~TeLayout() {
	if (parent() && _onParentSizeChangedCallback) {
		parent()->onSizeChanged().remove(_onParentSizeChangedCallback);
		_onParentSizeChangedCallback.reset();
		parent()->onWorldTransformationMatrixChanged().remove(_onParentWorldTransformationMatrixChangedCallback);
		_onParentWorldTransformationMatrixChangedCallback.reset();
	}

	if (_onChildSizeChangedCallback) {
		for (auto &child : childList()) {
			child->onSizeChanged().remove(_onChildSizeChangedCallback);
		}
	}
}

void TeLayout::addChild(Te3DObject2 *child) {
	Te3DObject2::addChild(child);
	if (_onChildSizeChangedCallback) {
		child->onSizeChanged().push_back(_onChildSizeChangedCallback);
	}
	_needZSizeUpdate = true;
	_needZUpdate = true;
	updateZSize();
	updateZ();
}

void TeLayout::addChildBefore(Te3DObject2 *child, const Te3DObject2 *ref) {
	Te3DObject2::addChildBefore(child, ref);
	if (_onChildSizeChangedCallback) {
		child->onSizeChanged().push_back(_onChildSizeChangedCallback);
	}
	_needZSizeUpdate = true;
	_needZUpdate = true;
	updateZSize();
	updateZ();
}

void TeLayout::removeChild(Te3DObject2 *child) {
	if (_onChildSizeChangedCallback) {
		child->onSizeChanged().remove(_onChildSizeChangedCallback);
	}
	Te3DObject2::removeChild(child);
	_needZSizeUpdate = true;
	_needZUpdate = true;
	updateZSize();
	updateZ();
}

const TeVector3f32 &TeLayout::anchor() {
	return _anchor;
}

void TeLayout::disableAutoZ() {
	_autoz = false;
}

void TeLayout::enableAutoZ() {
	_autoz = true;
}

bool TeLayout::isAutoZEnabled() {
	return _autoz;
}

void TeLayout::draw() {
	if (visible() && worldVisible()) {
		// Ensure world transform is up-to-date.
		worldTransformationMatrix();
		for (auto &child : childList()) {
			child->draw();
		}
	}
}

static const float EPSILON = 1.192093e-07f;

bool TeLayout::isMouseIn(const TeVector2s32 &mouseloc) {
	const TeVector3f32 transformedPos = transformMousePosition(mouseloc);
	const TeVector3f32 halfSize = size() / 2.0;

	return (   (transformedPos.x() >= -halfSize.x() - EPSILON)
			&& (transformedPos.x() < halfSize.x() + EPSILON)
			&& (transformedPos.y() >= -halfSize.y() - EPSILON)
			&& (transformedPos.y() < halfSize.y() + EPSILON));
}

TeILayout::DrawMode TeLayout::mode() {
	return _drawMode;
}

bool TeLayout::onChildSizeChanged() {
	_needZSizeUpdate = true;
	_needZUpdate = true;

	updateSize();
	if (!_updatingZSize)
		updateZSize();

	return false;
}

bool TeLayout::onParentSizeChanged() {
	_sizeChanged = true;
	_positionChanged = true;
	_worldMatrixChanged = true;
	return false;
}

bool TeLayout::onParentWorldTransformationMatrixChanged() {
	_worldMatrixChanged = true;
	return false;
}

TeVector3f32 TeLayout::position() {
	updateZ();
	updatePosition();
	return _position;
}

TeLayout::CoordinatesType TeLayout::positionType() const {
	return _positionType;
}

float TeLayout::ratio() const {
	return _ratio;
}

TeLayout::RatioMode TeLayout::ratioMode() const {
	return _ratioMode;
}

float TeLayout::safeAreaRatio() const {
	return _safeAreaRatio;
}

void TeLayout::setAnchor(const TeVector3f32 &anchor) {
	if (_anchor != anchor) {
		_anchor = anchor;
		_positionChanged = true;
		_worldMatrixChanged = true;
		updatePosition();
	}
}

void TeLayout::setMode(DrawMode mode) {
	_drawMode = mode;
}

void TeLayout::setParent(Te3DObject2 *parent) {
	assert(parent != this);
	Te3DObject2 *oldParent = Te3DObject2::parent();
	if (oldParent) {
		if (_onParentSizeChangedCallback)
			oldParent->onSizeChanged().remove(_onParentSizeChangedCallback);
		if (_onParentWorldTransformationMatrixChangedCallback)
			oldParent->onWorldTransformationMatrixChanged().remove(_onParentWorldTransformationMatrixChangedCallback);
	}

	//
	TeLayout &mainWindowLayout = g_engine->getApplication()->getMainWindow();
	mainWindowLayout.onSizeChanged().remove(_onMainWindowChangedCallback);

	Te3DObject2::setParent(parent);
	if (parent) {
		if (_onParentSizeChangedCallback)
			parent->onSizeChanged().push_back(_onParentSizeChangedCallback);
		if (_onParentWorldTransformationMatrixChangedCallback)
			parent->onWorldTransformationMatrixChanged().push_back(_onParentWorldTransformationMatrixChangedCallback);
		if (_onMainWindowChangedCallback)
			mainWindowLayout.onSizeChanged().push_back(_onMainWindowChangedCallback);
	}
	_needZUpdate = true;
	_sizeChanged = true;
	_positionChanged = true;
	_worldMatrixChanged = true;
	updateSize();
}

void TeLayout::setPosition(const TeVector3f32 &pos) {
	TeVector3f32 pos3d(pos.x(), pos.y(), _userPosition.z());
	if (_userPosition != pos3d) {
		_userPosition.x() = pos.x();
		_userPosition.y() = pos.y();
		_positionChanged = true;
		_worldMatrixChanged = true;
	}
}

void TeLayout::setPositionType(CoordinatesType newtype) {
	if (_positionType != newtype) {
		_positionType = newtype;
		_positionChanged = true;
		_worldMatrixChanged = true;
	}
}

void TeLayout::setRatio(float val) {
	if (_ratio != val) {
		_ratio = val;
		_sizeChanged = true;
		_worldMatrixChanged = true;
	}
}

void TeLayout::setRatioMode(RatioMode mode) {
	if (_ratioMode != mode) {
		_ratioMode = mode;
		_sizeChanged = true;
		_worldMatrixChanged = true;
	}
}

void TeLayout::setRotation(const TeQuaternion &rot) {
	if (rot != _rotation) {
		Te3DObject2::setRotation(rot);
		_worldMatrixChanged = true;
	}
}

void TeLayout::setSafeAreaRatio(float ratio) {
	if (_safeAreaRatio != ratio) {
		_safeAreaRatio = ratio;
		_sizeChanged = true;
		_worldMatrixChanged = true;
	}
}

void TeLayout::setScale(const TeVector3f32 &scale) {
	Te3DObject2::setScale(scale);
	_worldMatrixChanged = true;
}

void TeLayout::setSize(const TeVector3f32 &size) {
	const TeVector3f32 size3d(size.x(), size.y(), _userSize.z());
	if (_userSize != size3d) {
		_userSize.x() = size.x();
		_userSize.y() = size.y();
		_sizeChanged = true;
		_positionChanged = true;
		_worldMatrixChanged = true;
	}
}

void TeLayout::setSizeType(CoordinatesType coordtype) {
	assert(coordtype == RELATIVE_TO_PARENT || coordtype == ABSOLUTE);
	if (_sizeType != coordtype) {
		_sizeType = coordtype;
		_sizeChanged = true;
		_worldMatrixChanged = true;
	}
}

void TeLayout::setZPosition(float zpos) {
	if (_userPosition.z() != zpos) {
		_userPosition.z() = zpos;
		_positionChanged = true;
		_worldMatrixChanged = true;
	}
}

TeVector3f32 TeLayout::size() {
	updateSize();
	updateZSize();
	return _size;
}

TeLayout::CoordinatesType TeLayout::sizeType() const {
	return _sizeType;
}

TeVector3f32 TeLayout::transformMousePosition(const TeVector2s32 &mousepos) {
	TeVector3f32 relativeMouse(mousepos);

	relativeMouse.x() -= g_system->getWidth() / 2;
	relativeMouse.y() -= g_system->getHeight() / 2;

	TeMatrix4x4 transform = worldTransformationMatrix();
	transform.inverse();
	TeVector3f32 transformpos = transform * relativeMouse;

	//debug("transformMousePosition: %d %d -> %f %f", mousepos._x, mousepos._y, transformpos.x(), transformpos.y());
	return transformpos;
}

void TeLayout::updatePosition() {
	if (!_positionChanged)
		return;

	_positionChanged = false;
	_updatingPosition = true;
	TeVector3f32 oldpos = _position;
	Te3DObject2 *parentObj = parent();
	static const TeVector3f32 midPoint(0.5f, 0.5f, 0.5f);
	if (_positionType == RELATIVE_TO_PARENT && parentObj) {
		const TeVector3f32 offsetUserPos = _userPosition - midPoint;
		const TeVector3f32 parentSize(parentObj->xSize(), parentObj->ySize(), 0.0);
		const TeVector3f32 offsetAnchor =  midPoint - _anchor;
		const TeVector3f32 thisSize(xSize(), ySize(), 0.0);
		const TeVector3f32 newpos = (offsetUserPos * parentSize) + (offsetAnchor * thisSize);
		_position = newpos;
	} else if (_positionType == RELATIVE_TO_PARENT && !parentObj) {
		// Not in original, but no parent -> set midpoint.
		const TeVector3f32 offsetAnchor =  midPoint - _anchor;
		const TeVector3f32 thisSize(xSize(), ySize(), 0.0);
		_position = offsetAnchor * thisSize;
	} else if (_positionType == ABSOLUTE) {
		_position = _userPosition;
	}
	_position.z() = _userPosition.z();
	_worldMatrixChanged = true;
	_updatingPosition = false;

	if (_position != oldpos) {
		onPositionChanged().call();
	}
}

void TeLayout::updateSize() {
	if (!_sizeChanged)
		return;

	_sizeChanged = false;
	_updatingSize = true;
	const TeVector3f32 oldSize = _size;

	if (_sizeType == ABSOLUTE) {
		TeVector3f32 newSize = _userSize;
		_size.x() = abs(newSize.x());
		_size.y() = abs(newSize.y());
		// don't set Z val.
	} else if (_sizeType == RELATIVE_TO_PARENT) {
		Te3DObject2 *parentObj = parent();
		if (parentObj) {
			const TeVector3f32 parentSize(parentObj->xSize(), parentObj->ySize(), 0.0);
			TeVector3f32 newSize = _userSize * parentSize;
			if (newSize.x() > 0.0f && newSize.y() > 0.0f && _ratio > 0.0f && _safeAreaRatio > 0.0f) {
				float newSizeRatio = newSize.x() / newSize.y();
				if (_ratioMode  == RATIO_MODE_PAN_SCAN) {
					if (_safeAreaRatio <= newSizeRatio) {
						newSize.x() = _ratio * newSize.y();
					} else {
						newSize.x() =
							(1.0f - (_safeAreaRatio - newSizeRatio) / _safeAreaRatio) *
							_ratio * newSize.y();
					}
				} else if (_ratioMode == RATIO_MODE_LETTERBOX) {
					if (_ratio < newSizeRatio)
						newSize.x() = _ratio * newSize.y();
					else
						newSize.y() = newSize.x() / _ratio;
				}
			}

			_size.x() = newSize.x();
			_size.y() = newSize.y();
		} else {
			_size.x() = 0.0f;
			_size.y() = 0.0f;
		}
	}

	_updatingSize = false;
	// TODO: check this, is it the right flag to set?
	_positionChanged = true;

	updateMesh();

	if (_size != oldSize) {
		onSizeChanged().call();
	}
}

void TeLayout::updateWorldMatrix() {
	if (!_worldMatrixChanged)
		return;

	_worldMatrixChanged = false;
	_updatingWorldMatrix = true;
	const TeMatrix4x4 oldMatrix = _worldMatrixCache;
	_worldMatrixCache = Te3DObject2::worldTransformationMatrix();
	_updatingWorldMatrix = false;
	if (_worldMatrixCache != oldMatrix) {
		onWorldTransformationMatrixChanged().call();
	}
}

void TeLayout::updateZ() {
	if (!_needZUpdate || !_autoz)
		return;

	_needZUpdate = false;
	_updatingZ = true;

	float ztotal = 0.1f;
	for (auto &child : childList()) {
		child->setZPosition(ztotal);
		ztotal += child->zSize();
	}
	_updatingZ = false;
}

void TeLayout::updateZSize() {
	if (!_needZSizeUpdate)
		return;

	_needZSizeUpdate = false;
	_updatingZSize = true;
	const TeVector3f32 oldSize = _size;
	_size.z() = 0.1f;

	for (auto &child : childList()) {
		_size.z() += child->zSize();
	}

	_positionChanged = true;
	_updatingZSize = false;
	if (_size != oldSize) {
		onSizeChanged().call();
	}
}

TeVector3f32 TeLayout::userPosition() const {
	return _userPosition;
}

TeVector3f32 TeLayout::userSize() {
	updateZ();
	return _userSize;
}

TeVector3f32 TeLayout::worldPosition() {
	if (!parent()) {
		return position();
	} else {
		return parent()->worldPosition() + position();
	}
}

TeMatrix4x4 TeLayout::worldTransformationMatrix() {
	updateZ();
	updatePosition();
	updateWorldMatrix();
	return _worldMatrixCache;
}

bool TeLayout::worldVisible() {
	bool visible = Te3DObject2::visible();
	if (visible && parent()) {
		return parent()->worldVisible();
	}
	return visible;
}

float TeLayout::xSize() {
	updateSize();
	return _size.x();
}

float TeLayout::ySize() {
	updateSize();
	return _size.y();
}

float TeLayout::zSize() {
	updateZSize();
	return _size.z();
}

} // end namespace Tetraedge
