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

#include "mutationofjb/tasks/saytask.h"

#include "mutationofjb/tasks/taskmanager.h"
#include "mutationofjb/assets.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/room.h"

#include "graphics/managed_surface.h"
#include "graphics/screen.h"

namespace MutationOfJB {

SayTask::SayTask(const Common::String &toSay, uint8 color) : _toSay(toSay), _color(color), _timer(1000) {}

void SayTask::start() {

	getTaskManager()->getGame().getAssets().getSpeechFont().drawString(_toSay, _color, 0, 0, getTaskManager()->getGame().getScreen());
	_timer.start();
	setState(RUNNING);
}

void SayTask::update() {
	_timer.update();

	if (_timer.isFnished()) {
		getTaskManager()->getGame().getRoom().redraw(); // TODO: Only redraw the area occupied by the text.
		setState(FINISHED);
		return;
	}
}

}
