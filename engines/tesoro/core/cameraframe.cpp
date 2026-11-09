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

#include "tesoro/core/cameraframe.h"

#include "common/textconsole.h"

#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"

namespace Tesoro {

void CameraFrame::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers) {
	initAnimated(record, roomSortOffset, levelPlayers, 0);

	for (uint i = 0; i < 12; ++i)
		_targetName[i] = (char)record.dataByte(i);
	_targetName[12] = '\0';
	_count = (uint16)(record.dataByte(0xd) + 1);
	_flag = record.dataByte(0xe);
	_value = (uint16)(record.dataByte(0xf) | record.dataByte(0x10) << 8);
}

void CameraFrame::onAttach() {
	if (!_level || _anim.bankCount() == 0 || !_anim.player(0).bank()) {
		warning("CameraFrame: nothing to frame");
		return;
	}

	_target = _targetName[0] ? _level->findObject(nameKey64(_targetName)) : nullptr;

	frameCamera(_level->camera());
}

bool CameraFrame::frameCamera(Camera &camera) {
	if (_anim.bankCount() == 0 || !_anim.player(0).bank())
		return false;

	const AnimStep &step = _anim.player(0).bank()->step(0);
	const int32 left = step.hitX + _x;
	const int32 top = _y + step.hitY;
	camera.setBounds(left, top, step.hitW + left, top - step.hitH);
	camera.moveTo(_x, _y, 0, 0);
	return true;
}

} // End of namespace Tesoro
