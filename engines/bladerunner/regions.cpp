#include "bladerunner/regions.h"

namespace BladeRunner {

Regions::Regions() {
	_enabled = true;
	_regions = new Region[10];
	clear();
}

Regions::~Regions() {
	delete[] _regions;
}

void BladeRunner::Regions::clear() {
	for (int i = 0; i < 10; ++i)
		remove(i);
}

bool Regions::add(int index, Common::Rect rect, int type) {
	if (index < 0 || index >= 10)
		return false;

	if (_regions[index]._present)
		return false;

	_regions[index]._rectangle = rect;
	_regions[index]._type = type;
	_regions[index]._present = 1;

	return true;
}

bool Regions::remove(int index) {
	if (index < 0 || index >= 10)
		return false;

	_regions[index]._rectangle = Common::Rect(-1, -1, -1, -1);
	_regions[index]._type = -1;
	_regions[index]._present = 0;

	return true;
}

int Regions::getTypeAtXY(int x, int y) {
	int index = getRegionAtXY(x, y);

	if (index == -1)
		return -1;

	return _regions[index]._type;
}

int Regions::getRegionAtXY(int x, int y) {
	if (!_enabled)
		return -1;

	for (int i = 0; i != 10; ++i) {
		if (!_regions[i]._present)
			continue;

		// Common::Rect::contains is exclusive of right and bottom but
		// Blade Runner wants inclusive, so we adjust the edges.
		// TODO: Roll our own rect class?
		Common::Rect r = _regions[i]._rectangle;
		r.right++;
		r.bottom++;

		if (r.contains(x, y))
			return i;
	}

	return -1;
}

void Regions::setEnabled(bool enabled) {
	_enabled = enabled;
}

void Regions::enable() {
	_enabled = true;
}

} // End of namespace BladeRunner
