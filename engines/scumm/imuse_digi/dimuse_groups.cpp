/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/imuse_digi/dimuse_groups.h"

namespace Scumm {

IMuseDigiGroupsHandler::IMuseDigiGroupsHandler(IMuseDigital *engine) {
	_engine = engine;
}

IMuseDigiGroupsHandler::~IMuseDigiGroupsHandler() {}

int IMuseDigiGroupsHandler::init() {
	for (int i = 0; i < DIMUSE_MAX_GROUPS; i++) {
		_effVols[i] = 127;
		_vols[i] = 127;
	}
	return 0;
}

int IMuseDigiGroupsHandler::setGroupVol(int id, int volume) {
	int l;

	if (id >= DIMUSE_MAX_GROUPS) {
		return -5;
	}

	if (volume == -1) {
		return _vols[id];
	}

	if (volume > 127)
		return -5;

	if (id) {
		_vols[id] = volume;
		_effVols[id] = (_vols[0] * (volume + 1)) / 128;
	} else {
		_effVols[0] = volume;
		_vols[0] = volume;

		for (l = 1; l < DIMUSE_MAX_GROUPS; l++) {
			_effVols[l] = (volume * (_vols[l] + 1)) / 128;
		}
	}

	_engine->diMUSEUpdateGroupVolumes();
	return _vols[id];
}

int IMuseDigiGroupsHandler::getGroupVol(int id) {
	if (id >= DIMUSE_MAX_GROUPS) {
		return -5;
	}

	return _effVols[id];
}

} // End of namespace Scumm
