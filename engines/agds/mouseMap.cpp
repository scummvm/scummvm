#include "agds/mouseMap.h"
#include "agds/agds.h"
#include "agds/region.h"
#include "agds/object.h"
#include <utility>

namespace AGDS {

int MouseMap::findFree() const {
	for(int i = 0, n = _mouseRegions.size(); i != n; ++i) {
		auto & region = _mouseRegions[i];
		if (!region)
			return i;
	}
	error("no mouse region available");
}


int MouseMap::add(MouseRegion area) {
	auto id = findFree();
	auto & region = _mouseRegions[id];
	region.reset(new MouseRegion(std::move(area)));
	region->id = id;
	return id;
}

MouseRegion *MouseMap::find(Common::Point pos) {
	if (_disabled)
		return nullptr;
	for (auto & region : _mouseRegions) {
		if (region && region->enabled && region->region->pointIn(pos))
			return region.get();
	}
	return nullptr;
}

MouseRegion *MouseMap::find(int id) {
	for (auto & region : _mouseRegions) {
		if (region && region->id == id)
			return region.get();
	}
	return nullptr;
}

void MouseMap::remove(AGDSEngine *engine, int id) {
	auto &region = _mouseRegions[id];
	if (!region) {
		warning("removing non-existent mouse region %d", id);
		return;
	}
	region->disable(engine);
	region.reset();
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
	for (auto & region : _mouseRegions)
		if (region)
			region->hide(engine);
}

} // End of namespace AGDS

