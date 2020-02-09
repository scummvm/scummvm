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

#ifndef MUTATIONOFJB_SAYTASK_H
#define MUTATIONOFJB_SAYTASK_H

#include "mutationofjb/tasks/task.h"

#include "mutationofjb/timer.h"

#include "common/rect.h"
#include "common/str.h"

namespace MutationOfJB {

class SayTask : public Task {
public:
	SayTask(const Common::String &toSay, uint8 color);

	void start() override;
	void update() override;
	void stop() override;

private:
	void drawSubtitle(const Common::String &text, int16 talkX, int16 talkY, uint8 color);
	void finish();

	Common::String _toSay;
	uint8 _color;
	Timer _timer;
	Common::Rect _boundingBox;
};

}

#endif
