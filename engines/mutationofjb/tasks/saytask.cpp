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
	const int16 actualMaxWidth = font.wordWrapText(text, MAX_LINE_WIDTH, lines);

	// Get the x, y coordinates of the top center point of the text's bounding box
	// from the (rather strange) talk coordinates coming from scripts.
	int16 x = talkX;
	int16 y = talkY - (lines.size() - 1) * font.getFontHeight() - 15;

	// Clamp to screen edges.
	x = CLIP<int16>(x, 3 + actualMaxWidth / 2, 317 - actualMaxWidth / 2);
	y = MAX<int16>(y, 3);

	// Remember the area occupied by the text.
	_boundingBox.left = x - actualMaxWidth / 2;
	_boundingBox.top = y;
	_boundingBox.setWidth(actualMaxWidth);
	_boundingBox.setHeight(lines.size() * font.getFontHeight());

	// Draw lines.
	for (uint i = 0; i < lines.size(); i++) {
		font.drawString(&getTaskManager()->getGame().getScreen(), lines[i], _boundingBox.left, _boundingBox.top + i * font.getFontHeight(), _boundingBox.width(), color, Graphics::kTextAlignCenter);
	}
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
