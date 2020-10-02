#include "agds/mouseMap.h"
#include "agds/agds.h"
#include "agds/region.h"
#include "agds/object.h"

namespace AGDS {

int MouseMap::add(const MouseRegion & area) {
	int id = _nextId++;
	_mouseRegions.push_back(area);
	_mouseRegions.back().id = id;
	return id;
}

MouseRegion *MouseMap::find(Common::Point pos) {
	if (_disabled)
		return NULL;
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end(); ++i) {
		MouseRegion &mouse = *i;
		if (mouse.enabled && mouse.region->pointIn(pos))
			return &mouse;
	}
	return NULL;
}

MouseRegion *MouseMap::find(int id) {
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end(); ++i) {
		MouseRegion &mouse = *i;
		if (mouse.id == id)
			return &mouse;
	}
	return NULL;
}

void MouseMap::remove(AGDSEngine *engine, int id) {
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end();) {
		MouseRegion &mouse = *i;
		if (mouse.id == id) {
			i->disable(engine);
			i = _mouseRegions.erase(i);
		} else
			++i;
	}
}

void MouseRegion::show(AGDSEngine *engine) {
	if (visible)
		return;

	visible = true;
	debug("calling mouseArea[%d].onEnter: %s", id, onEnter.c_str());
	engine->runObject(onEnter);
}

void MouseRegion::hide(AGDSEngine *engine) {
	if (!visible)
		return;

	visible = false;
	debug("calling mouseArea[%d].onLeave: %s...", id, onLeave.c_str());
	engine->runObject(onLeave);
}

void MouseMap::hideAll(AGDSEngine *engine) {
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end(); ++i)
		i->hide(engine);
}

} // End of namespace AGDS

