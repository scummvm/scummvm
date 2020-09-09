#include "agds/mouseMap.h"
#include "agds/region.h"

namespace AGDS {

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

void MouseMap::remove(int id) {
	for (MouseRegionsType::iterator i = _mouseRegions.begin(); i != _mouseRegions.end();) {
		MouseRegion &mouse = *i;
		if (mouse.id == id)
			i = _mouseRegions.erase(i);
		else
			++i;
	}
}


} // End of namespace AGDS

