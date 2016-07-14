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

#ifndef GRIM_COMPONENT_H
#define GRIM_COMPONENT_H

#include "math/matrix4.h"
#include "engines/grim/object.h"
#include "engines/grim/animation.h"

namespace Grim {

typedef uint32 tag32;

class Costume;
class CMap;
class SaveGame;

class Component {
public:
	Component(Component *parent, int parentID, const char *name, tag32 tag);

	CMap *getCMap();
	virtual void setColormap(CMap *c);
	bool isVisible();
	Component *getParent() { return _parent; }
	virtual void setMatrix(const Math::Matrix4 &) { };
	virtual void init() { }
	virtual void setKey(int) { }
	virtual void setMapName(char *) { }
	virtual int update(uint time) { return 0; }
	virtual void animate() { }
	virtual void draw() { }
	virtual void reset() { }
	virtual void fade(Animation::FadeMode, int) { }
	virtual void advance(uint msecs) { }
	virtual void setPaused(bool paused) { }
	virtual void resetColormap() { }
	virtual void saveState(SaveGame *) { }
	virtual void restoreState(SaveGame *) { }
	virtual ~Component();

	bool isComponentType(char a0, char a1, char a2, char a3) { return _tag == MKTAG(a0, a1, a2, a3); }

protected:
	ObjectPtr<CMap> _cmap, _previousCmap;
	tag32 _tag;
	int _parentID;
	bool _visible;
	Component *_parent, *_child, *_sibling;
	Costume *_cost;
	Common::String _name;

	void setCostume(Costume *cost) { _cost = cost; }
	void setParent(Component *newParent);
	void removeChild(Component *child);
	void resetHierCMap();

	friend class Costume;
	friend class EMICostume;
};

} // end of namespace Grim

#endif
