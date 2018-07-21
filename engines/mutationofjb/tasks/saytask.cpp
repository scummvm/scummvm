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
#include "mutationofjb/util.h"

#include "graphics/managed_surface.h"
#include "graphics/screen.h"

namespace MutationOfJB {

SayTask::SayTask(const Common::String &toSay, uint8 color) : _toSay(toSay), _color(color), _timer(50 * toSay.size()) {}

void SayTask::start() {
	Game &game = getTaskManager()->getGame();
	if (game.getActiveSayTask()) {
		getTaskManager()->stopTask(game.getActiveSayTask());
	}
	game.setActiveSayTask(getTaskManager()->getTask(this));

	setState(RUNNING);
	drawSubtitle(_toSay, 160, 0, _color); // TODO: Respect PTALK and LTALK commands.
	_timer.start();
}

void SayTask::update() {
	_timer.update();

	if (_timer.isFinished()) {
		finish();
	}
}

void SayTask::stop() {
	if (getState() == RUNNING) {
		finish();
	}
}

void SayTask::drawSubtitle(const Common::String &text, int16 talkX, int16 talkY, uint8 color) {
	const int MAX_LINE_WIDTH = 250;

	const Font &font = getTaskManager()->getGame().getAssets().getSpeechFont();

	Common::Array<Common::String> lines;
	font.wordWrap(text, MAX_LINE_WIDTH, lines);

	// Get the x, y coordinates of the top center point of the text's bounding box
	// from the (rather strange) talk coordinates coming from scripts.
	int16 x = talkX;
	int16 y = talkY - (lines.size() - 1) * font.getLineHeight() - 15;

	// Clamp to screen edges.
	y = MAX<int16>(y, 3);
	int16 maxWidth = 0;
	for (uint i = 0; i < lines.size(); i++) {
		int16 lineWidth = font.getWidth(lines[i]);
		if (lineWidth > maxWidth) {
			maxWidth = lineWidth;
		}
		x = MAX<int16>(x, 3 + lineWidth / 2);
		x = MIN<int16>(x, 317 - lineWidth / 2);
	}

	// Draw lines.
	for (uint i = 0; i < lines.size(); i++) {
		font.drawString(lines[i], color, x - font.getWidth(lines[i]) / 2, y + i * font.getLineHeight(), getTaskManager()->getGame().getScreen());
	}

	// Remember the area occupied by the text.
	_boundingBox.top = x - maxWidth / 2;
	_boundingBox.left = y;
	_boundingBox.setWidth(maxWidth);
	_boundingBox.setHeight(lines.size() * font.getLineHeight());
}

void SayTask::finish() {
	getTaskManager()->getGame().getRoom().redraw(); // TODO: Only redraw the area occupied by the text.
	setState(FINISHED);

	Game &game = getTaskManager()->getGame();
	if (game.getActiveSayTask().get() == this) {
		game.setActiveSayTask(Common::SharedPtr<SayTask>());
	}
}

}
