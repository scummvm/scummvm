/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/grim/costume/component.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"

namespace Grim {

Component::Component(Component *p, int parentID, const char *name, tag32 t) :
		_visible(true), _tag(t), _parentID(parentID), _name(name),
		_previousCmap(nullptr), _cmap(nullptr), _cost(nullptr) {

	setParent(p);
}

Component::~Component() {
	if (_parent)
		_parent->removeChild(this);

	Component *child = _child;
	while (child) {
		child->_parent = nullptr;
		child = child->_sibling;
	}
}

void Component::setColormap(CMap *c) {
	if (c)
		_cmap = c;
	if (getCMap()) {
		resetHierCMap();
	}
}

bool Component::isVisible() {
	if (_visible && _parent)
		return _parent->isVisible();
	return _visible;
}

CMap *Component::getCMap() {
	if (!_cmap && _previousCmap)
		return _previousCmap;
	else if (!_cmap && _parent)
		return _parent->getCMap();
	else if (!_cmap && !_parent && _cost)
		return _cost->getCMap();
	else
		return _cmap;
}

void Component::setParent(Component *newParent) {
	_parent = newParent;
	_child = nullptr;
	_sibling = nullptr;
	if (_parent) {
		_sibling = _parent->_child;
		_parent->_child = this;
	}
}

void Component::removeChild(Component *child) {
	Component **childPos = &_child;
	while (*childPos && *childPos != child)
		childPos = &(*childPos)->_sibling;
	if (*childPos) {
		*childPos = child->_sibling;
		child->_parent = nullptr;
	}
}

void Component::resetHierCMap() {
	resetColormap();

	Component *child = _child;
	while (child) {
		child->resetHierCMap();
		child = child->_sibling;
	}
}

} // end of namespace Grim
