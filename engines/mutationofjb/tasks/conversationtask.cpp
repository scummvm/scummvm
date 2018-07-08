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

#include "mutationofjb/tasks/conversationtask.h"
#include "mutationofjb/tasks/taskmanager.h"
#include "mutationofjb/assets.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/gui.h"
#include "mutationofjb/util.h"
#include "mutationofjb/widgets/conversationwidget.h"

namespace MutationOfJB {

void ConversationTask::start() {
	Game &game = getTaskManager()->getGame();
	ConversationWidget &widget = game.getGui().getConversationWidget();

	widget.setCallback(this);
	widget.setVisible(true);

	updateWidget();
}

void ConversationTask::update() {
}

void ConversationTask::onResponseClicked(ConversationWidget *, int response) {

	uint8 nextLineIndex = _convInfo._lines[_currentLine]._items[response]._nextLineIndex;
	if (nextLineIndex == 0) {
		setState(FINISHED);
		Game &game = getTaskManager()->getGame();
		ConversationWidget &widget = game.getGui().getConversationWidget();
		widget.setVisible(false);
		game.getGui().markDirty(); // TODO: Handle automatically when changing visibility.
		return;
	}

	_currentLine = nextLineIndex - 1;
	updateWidget();
}

void ConversationTask::updateWidget() {
	Game &game = getTaskManager()->getGame();
	ConversationWidget &widget = game.getGui().getConversationWidget();

	const ConversationLineList& toSayList = game.getAssets().getToSayList();

	const ConversationInfo::Line &convLine = _convInfo._lines[_currentLine];

	for (ConversationInfo::Items::size_type i = 0; i < convLine._items.size(); ++i) {
		Common::String widgetText;
		const uint8 question = convLine._items[i]._question;
		if (question != 0) {
			const ConversationLineList::Line *line = toSayList.getLine(convLine._items[i]._question);
			widgetText = toUpperCP895(line->_speeches[0]._text);
		}

		widget.setLine(i, widgetText);
	}
}

}
