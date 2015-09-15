#include "bladerunner/regions.h"

namespace BladeRunner {
	Regions::Regions() {
		_enabled = true;
		_regions = new Region[10];
	}

	Regions::~Regions() {
		delete[] _regions;
	}

	bool Regions::add(int index, Common::Rect rect, int type) {
		if (index <= 0 || index >= 10)
			return false;

		if (_regions[index]._present)
			return false;

		_regions[index]._rectangle = rect;
		_regions[index]._type = type;
		_regions[index]._present = 1;

		return true;
	}

	bool Regions::remove(int index) {
		if (index <= 0 || index >= 10)
			return false;

		_regions[index]._rectangle = Common::Rect(-1, -1, -1, -1);
		_regions[index]._type = -1;
		_regions[index]._present = 0;

		return true;
	}

	void BladeRunner::Regions::clear() {
		int i;
		for (i = 0; i < 10; i++)
			remove(i);
	}

	void Regions::setEnabled(bool enabled) {
		_enabled = enabled;
	}
}