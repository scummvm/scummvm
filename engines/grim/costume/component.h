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

#ifndef GRIM_COMPONENT_H
#define GRIM_COMPONENT_H

#include "math/matrix4.h"
#include "engines/grim/object.h"

namespace Grim {

typedef uint32 tag32;

class Costume;
class CMap;
class SaveGame;

class Component {
public:
	Component(Component *parent, int parentID, tag32 tag);

	tag32 getTag() { return _tag; }
	CMap *getCMap();
	virtual void setColormap(CMap *c);
	bool isVisible();
	Component *getParent() { return _parent; }
	virtual void setMatrix(Math::Matrix4) { };
	virtual void init() { }
	virtual void setKey(int) { }
	virtual void setMapName(char *) { }
	virtual int update(float time) { return 0; }
	virtual void animate() { }
	virtual void setupTexture() { }
	virtual void draw() { }
	virtual void reset() { }
	virtual void resetColormap() { }
	virtual void saveState(SaveGame *) { }
	virtual void restoreState(SaveGame *) { }
	virtual ~Component();

protected:
	ObjectPtr<CMap> _cmap, _previousCmap;
	tag32 _tag;
	int _parentID;
	bool _visible;
	Component *_parent, *_child, *_sibling;
	Math::Matrix4 _matrix;
	Costume *_cost;
	void setCostume(Costume *cost) { _cost = cost; }
	void setParent(Component *newParent);
	void removeChild(Component *child);
	void resetHierCMap();

	friend class Costume;
};

} // end of namespace Grim

#endif
