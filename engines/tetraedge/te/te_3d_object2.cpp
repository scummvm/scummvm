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

#include "tetraedge/te/te_3d_object2.h"

namespace Tetraedge {

Te3DObject2::Te3DObject2() : _childListChanged(false), _parent(nullptr), _scale(1.0f, 1.0f, 1.0f), _color(255, 255, 255, 255), _visible(true), _colorInheritance(true) {
	_onWorldVisibleChangedParentCallback.reset(
			new TeCallback0Param<Te3DObject2>(this, &Te3DObject2::onWorldVisibleChangedSlot));
	_onWorldColorChangedParentCallback.reset(
			new TeCallback0Param<Te3DObject2>(this, &Te3DObject2::onParentWorldColorChanged));
	_onWorldTransformationMatrixChangedParentCallback.reset(
			new TeCallback0Param<Te3DObject2>(this, &Te3DObject2::onParentWorldTransformationMatrixChanged));
}

Te3DObject2::~Te3DObject2() {
	for (auto *child : _children) {
		child->setParent(nullptr);
	}
	// clear list in case parent->removeChild triggers a signal which ends up referencing it.
	_children.clear();
	if (parent()) {
		parent()->removeChild(this);
	}
	setParent(nullptr);
}

void Te3DObject2::addChild(Te3DObject2 *newChild) {
	assert(newChild != this && newChild != _parent);
	for (auto *c : _children) {
		if (c == newChild)
			error("Trying to re-add child %s to object %s", newChild->name().c_str(), _name.c_str());
	}

	_children.push_back(newChild);
	newChild->setParent(this);
	_childListChangedSignal.call();
}

void Te3DObject2::addChildBefore(Te3DObject2 *newChild, const Te3DObject2 *ref) {
	assert(newChild != this && newChild != _parent);
	for (auto *c : _children) {
		if (c == newChild)
			error("Trying to re-add child %s to object %s", newChild->name().c_str(), _name.c_str());
	}

	Common::Array<Te3DObject2 *>::iterator iter;
	for (iter = _children.begin(); iter != _children.end(); iter++) {
		if (*iter == ref) {
			_children.insert(iter, newChild);
			break;
		}
	}
	if (iter == _children.end())
		_children.push_back(newChild);

	newChild->setParent(this);
	_childListChangedSignal.call();
}

Te3DObject2 *Te3DObject2::child(int offset) {
	return _children[offset];
}

int Te3DObject2::childIndex(Te3DObject2 *c) const {
	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i] == c)
			return i;
	}
	return -1;
}

/*static*/
void Te3DObject2::deserialize(Common::ReadStream &stream, Te3DObject2 &dest, bool includesName /* = true */) {
	if (includesName) {
		Common::String str = deserializeString(stream);
		dest.setName(str);
	}

	TeVector3f32 vect;
	TeVector3f32::deserialize(stream, vect);
	dest.setPosition(vect);

	TeQuaternion quat;
	TeQuaternion::deserialize(stream, quat);
	dest.setRotation(quat);

	TeVector3f32::deserialize(stream, vect);
	dest.setScale(vect);
}

/*static*/
void Te3DObject2::serialize(Common::WriteStream &stream, Te3DObject2 &src) {
	const Common::String &name = src.name();
	stream.writeUint32LE(name.size());
	stream.write(name.c_str(), name.size());

	const TeVector3f32 pos = src.position();
	TeVector3f32::serialize(stream, pos);

	const TeQuaternion rot = src.rotation();
	TeQuaternion::serialize(stream, rot);

	const TeVector3f32 sca = src.scale();
	TeVector3f32::serialize(stream, sca);
}

bool Te3DObject2::onParentWorldColorChanged() {
	_onParentWorldColorChangedSignal.call();
	return false;
}

bool Te3DObject2::onParentWorldTransformationMatrixChanged() {
	_onParentWorldTransformationMatrixChangedSignal.call();
	return false;
}

void Te3DObject2::removeChild(Te3DObject2 *child) {
	uint i;
	for (i = 0; i < _children.size(); i++) {
		if (_children[i] == child) {
			break;
		}
	}
	if (i < _children.size()) {
		_children[i]->setParent(nullptr);
		_children.remove_at(i);
		_childListChangedSignal.call();
	} else {
		Common::String cname("nullptr");
		if (child)
			cname = child->name();
		debug("Request to remove child (%s) which is not a child of this (%s).", cname.c_str(), name().c_str());
	}
}

bool Te3DObject2::onWorldVisibleChangedSlot() {
	_onWorldVisibleChangedSlotSignal.call();
	return false;
}

void Te3DObject2::removeChildren() {
	for (auto *child : _children) {
		child->setParent(nullptr);
	}
	_children.clear();
	_childListChangedSignal.call();
}

void Te3DObject2::rotate(const TeQuaternion &rot) {
	const TeQuaternion newRot = rotation() * rot;
	setRotation(newRot);
}

void Te3DObject2::setColor(const TeColor &col) {
	_color = col;
	_onParentWorldColorChangedSignal.call();
}

void Te3DObject2::setParent(Te3DObject2 *newparent) {
	assert(newparent != this);
	if (_parent) {
		if (_onWorldVisibleChangedParentCallback)
			_parent->onWorldVisibleChanged().remove(_onWorldVisibleChangedParentCallback);
		if (_onWorldTransformationMatrixChangedParentCallback)
			_parent->onWorldTransformationMatrixChanged().remove(_onWorldTransformationMatrixChangedParentCallback);
		if (_onWorldColorChangedParentCallback)
			_parent->onWorldColorChanged().remove(_onWorldColorChangedParentCallback);
	}
	_parent = newparent;
	if (newparent) {
		if (_onWorldVisibleChangedParentCallback)
			_parent->onWorldVisibleChanged().push_back(_onWorldVisibleChangedParentCallback);
		if (_onWorldTransformationMatrixChangedParentCallback)
			_parent->onWorldTransformationMatrixChanged().push_back(_onWorldTransformationMatrixChangedParentCallback);
		if (_onWorldColorChangedParentCallback)
			_parent->onWorldColorChanged().push_back(_onWorldColorChangedParentCallback);

		_onWorldVisibleChangedSlotSignal.call();
		_onParentWorldTransformationMatrixChangedSignal.call();
		_onParentWorldColorChangedSignal.call();
	}
}

void Te3DObject2::setPosition(const TeVector3f32 &pos) {
	if (_position == pos)
		return;

	// FIXME: remove this debugging code.
	if ((_position - pos).length() > 2.0f && name() == "Kate" && _position != TeVector3f32()) {
		debug("Large position move %s %s -> %s", name().c_str(),
			_position.dump().c_str(), pos.dump().c_str());
	}

	_position = pos;
	_onPositionChangedSignal.call();
	_onParentWorldTransformationMatrixChangedSignal.call();
}

void Te3DObject2::setPositionFast(const TeVector3f32 &pos) {
	_position = pos;
}

void Te3DObject2::setRotation(const TeQuaternion &rot) {
	if (_rotation == rot)
		return;

	_rotation = rot;
	_onParentWorldTransformationMatrixChangedSignal.call();
}

void Te3DObject2::setScale(const TeVector3f32 &scale) {
	if (_scale == scale)
		return;

	_scale = scale;
	_onParentWorldTransformationMatrixChangedSignal.call();
}

void Te3DObject2::setSize(const TeVector3f32 &size) {
	if (_size == size)
		return;

	_size = size;
	_onSizeChangedSignal.call();
}

void Te3DObject2::setVisible(bool visible) {
	if (_visible == visible)
		return;

	_visible = visible;
	onWorldVisibleChangedSlot();
}

void Te3DObject2::setZPosition(float zpos) {
	TeVector3f32 pos = position();
	pos.z() = zpos;
	setPosition(pos);
}

TeMatrix4x4 Te3DObject2::transformationMatrix() {
	TeMatrix4x4 retval;
	retval.translate(position());
	retval.rotate(rotation());
	retval.scale(scale());
	return retval;
}

void Te3DObject2::translate(const TeVector3f32 &vec) {
	TeVector3f32 pos = position();
	pos += vec;
	setPosition(pos);
}

TeColor Te3DObject2::worldColor() {
	if (!_parent || !_colorInheritance) {
		return color();
	} else {
		const TeColor parentCol = _parent->worldColor();
		const TeColor thisCol = color();
		return parentCol * thisCol;
	}
}

TeVector3f32 Te3DObject2::worldPosition() {
	if (!_parent) {
		return position();
	} else {
		return _parent->worldPosition() + position();
	}
}

TeQuaternion Te3DObject2::worldRotation() {
	if (!_parent) {
		return rotation();
	} else {
		return _parent->worldRotation() * rotation();
	}
}

TeVector3f32 Te3DObject2::worldScale() {
	if (!_parent) {
		return scale();
	} else {
		return _parent->worldScale() * scale();
	}
}

TeMatrix4x4 Te3DObject2::worldTransformationMatrix() {
	if (!_parent) {
		return transformationMatrix();
	} else {
		return _parent->worldTransformationMatrix() * transformationMatrix();
	}
}

bool Te3DObject2::worldVisible() {
	if (!_parent) {
		return visible();
	} else {
		return _parent->worldVisible() && visible();
	}
}

/*static*/
bool Te3DObject2::loadAndCheckFourCC(Common::ReadStream &stream, const char *str) {
	char buf[5];
	buf[4] = '\0';
	stream.read(buf, 4);
	bool result = !strncmp(buf, str, 4);
	if (!result)
		debug("loadAndCheckFourCC: Look for %s, got %s", str, buf);
	return result;
}

/*static*/
Common::String Te3DObject2::deserializeString(Common::ReadStream &stream) {
	uint slen = stream.readUint32LE();
	if (slen > 1024 * 1024)
		error("Improbable string size %d", slen);

	if (slen) {
		char *buf = new char[slen + 1];
		buf[slen] = '\0';
		stream.read(buf, slen);
		Common::String str(buf);
		delete[] buf;
		return str;
	}
	return Common::String();
}

/*static*/
void Te3DObject2::deserializeVectorArray(Common::ReadStream &stream, Common::Array<TeVector3f32> &dest) {
	uint32 nentries = stream.readUint32LE();
	if (nentries > 1000000)
		error("TeFreeMoveZone improbable number of vectors %d", nentries);
	dest.resize(nentries);
	for (uint i = 0; i < nentries; i++)
		TeVector3f32::deserialize(stream, dest[i]);
}

/*static*/
void Te3DObject2::deserializeUintArray(Common::ReadStream &stream, Common::Array<uint> &dest) {
	uint32 nentries = stream.readUint32LE();
	if (nentries > 1000000)
		error("TeFreeMoveZone improbable number of ints %d", nentries);
	dest.resize(nentries);
	for (uint i = 0; i < nentries; i++)
		dest[i] = stream.readUint32LE();
}


} // end namespace Tetraedge
