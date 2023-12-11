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

#include "math/matrix3.h"
#include "scenegraph.h"

namespace Twp {

Node::Node(const Common::String &name)
	: _name(name), _color(1.f, 1.f, 1.f, 1.f), _computedColor(1.f, 1.f, 1.f, 1.f), _visible(true), _rotation(0.f), _rotationOffset(0.f) {
}

Node::~Node() {}

void Node::addChild(Node *child) {
	if (child->_parent) {
		child->_pos -= getAbsPos();
		for (int i = 0; i < _children.size(); i++) {
			if (_children[i] == child) {
				child->_parent->_children.erase(child->_parent->_children.begin() + i);
				break;
			}
		}
	}
	_children.push_back(child);
	child->_parent = this;
	child->updateColor();
	child->updateAlpha();
}

void Node::updateColor() {
	Color parentColor = !_parent ? Color(1.f, 1.f, 1.f) : _parent->_computedColor;
	updateColor(parentColor);
}

void Node::updateAlpha() {
	float parentOpacity = !_parent ? 1.f : _parent->_computedColor.rgba.a;
	updateAlpha(parentOpacity);
}

void Node::updateColor(Color parentColor) {
	_computedColor.rgba.r = _color.rgba.r * parentColor.rgba.r;
	_computedColor.rgba.g = _color.rgba.g * parentColor.rgba.g;
	_computedColor.rgba.b = _color.rgba.b * parentColor.rgba.b;
	onColorUpdated(_computedColor);
	for (int i = 0; i < _children.size(); i++) {
		Node *child = _children[i];
		child->updateColor(_computedColor);
	}
}

void Node::updateAlpha(float parentAlpha) {
	_computedColor.rgba.a = _color.rgba.a * parentAlpha;
	onColorUpdated(_computedColor);
	for (int i = 0; i < _children.size(); i++) {
		Node *child = _children[i];
		child->updateAlpha(_computedColor.rgba.a);
	}
}

static bool cmpNodes()(const Node* x, const Node* y)
	return x->() < y.getZSort();
}

void Node::draw(Math::Matrix4 parent) {
	// Draws `self` node.
	if (_visible) {
		Math::Matrix4 trsf = getTrsf(parent);
		Math::Matrix4 myTrsf(trsf);
		myTrsf.translate(Math::Vector3d(-_anchor.getX(), _anchor.getY(), 0.f));
		//_children.sort(proc(x, y : Node) : int = cmp(y.getZSort, x.getZSort));
		Common::sort(_children.begin(), _children.end(), );
		drawCore(myTrsf);
		for (int i = 0; i < _children.size(); i++) {
			Node *child = _children[i];
			child->draw(trsf);
		}
	}
}

Math::Matrix4 Node::getTrsf(Math::Matrix4 parentTrsf) {
  // Gets the full transformation for this node.
  return parentTrsf * getLocalTrsf();
}

Math::Matrix4 Node::getLocalTrsf() {
  // Gets the location transformation = translation * rotation * scale.
	// TODO: scale
	Math::Vector2d p = _pos + _offset + _shakeOffset;
	Math::Matrix4 m1;
	m1.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
	Math::Matrix3 mRot;
	mRot.buildAroundZ(Math::Angle(-_rotation+_rotationOffset));
	Math::Matrix4 m2;
	m2.setRotation(mRot);
	 Math::Matrix4 m3;
	 m3.translate(Math::Vector3d(_renderOffset.getX(), _renderOffset.getY(), 0.f));
	return m1*m2*m3;
}

} // namespace Twp
