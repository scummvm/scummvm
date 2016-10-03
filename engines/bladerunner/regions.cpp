/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
