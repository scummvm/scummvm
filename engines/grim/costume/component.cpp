/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "engines/grim/costume.h"
#include "engines/grim/colormap.h"

namespace Grim {

Costume::Component::Component(Component *p, int parentID, tag32 t)  {
	_visible = true;
	_previousCmap = NULL;
	_cmap = NULL;
	_cost = NULL;
	_parent = NULL;
	_parentID = parentID;
	setParent(p);
	_tag = t;
}

Costume::Component::~Component()
{
	if (_parent)
		_parent->removeChild(this);

	Component *child = _child;
	while (child) {
		child->_parent = NULL;
		child = child->_sibling;
	}
}

void Costume::Component::setColormap(CMap *c) {
	if (c)
		_cmap = c;
	if (getCMap()) {
		resetColormap();
		resetHierCMap();
	}
}

bool Costume::Component::isVisible() {
	if (_visible && _parent)
		return _parent->isVisible();
	return _visible;
}

CMap *Costume::Component::getCMap() {
	if (!_cmap && _previousCmap)
		return _previousCmap;
	else if (!_cmap && _parent)
		return _parent->getCMap();
	else if (!_cmap && !_parent && _cost)
		return _cost->_cmap;
	else
		return _cmap;
}

void Costume::Component::setParent(Component *newParent) {
	_parent = newParent;
	_child = NULL;
	_sibling = NULL;
	if (_parent) {
		_sibling = _parent->_child;
		_parent->_child = this;
	}
}

void Costume::Component::removeChild(Component *child) {
	Component **childPos = &_child;
	while (*childPos && *childPos != child)
		childPos = &(*childPos)->_sibling;
	if (*childPos) {
		*childPos = child->_sibling;
		child->_parent = NULL;
	}
}

void Costume::Component::resetHierCMap() {
	resetColormap();

	Component *child = _child;
	while (child) {
		child->resetHierCMap();
		child = child->_sibling;
	}
}

} // end of namespace Grim
