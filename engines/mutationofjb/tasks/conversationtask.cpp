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

#include "mutationofjb/assets.h"
#include "mutationofjb/conversationlinelist.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/gui.h"
#include "mutationofjb/script.h"
#include "mutationofjb/tasks/saytask.h"
#include "mutationofjb/tasks/taskmanager.h"
#include "mutationofjb/util.h"
#include "mutationofjb/widgets/conversationwidget.h"

#include "common/translation.h"

namespace MutationOfJB {

void ConversationTask::start() {
	setState(RUNNING);

	Game &game = getTaskManager()->getGame();
	ConversationWidget &widget = game.getGui().getConversationWidget();

	widget.setCallback(this);
	widget.setVisible(true);

	_currentLineIndex = 0;

	showChoicesOrPick();
}

void ConversationTask::update() {
	if (_sayTask) {
		if (_sayTask->getState() == Task::FINISHED) {
			getTaskManager()->removeTask(_sayTask);
			delete _sayTask;
			_sayTask = nullptr;

			switch (_substate) {
			case SAYING_NO_CHOICES:
				finish();
				break;
			case SAYING_CHOICE: {
				const ConversationLineList& responseList = getTaskManager()->getGame().getAssets().getResponseList();
				const ConversationLineList::Line *const line = responseList.getLine(_currentItem->_response);

				_sayTask = new SayTask(line->_speeches[0]._text, _convInfo._color);
				getTaskManager()->addTask(_sayTask);
				_substate = SAYING_RESPONSE;
				break;
			}
			case SAYING_RESPONSE: {
				startExtra();

				if (_substate != RUNNING_EXTRA)
				{
					gotoNextLine();
				}
				break;
			}
			default:
				break;
			}
		}
	}

	if (_innerExecCtx) {
		Command::ExecuteResult res = _innerExecCtx->runActiveCommand();
		if (res == Command::Finished) {
			delete _innerExecCtx;
			_innerExecCtx = nullptr;

			gotoNextLine();
		}
	}
}

void ConversationTask::onChoiceClicked(ConversationWidget *convWidget, int, uint32 data) {
	const ConversationInfo::Item &item = getCurrentLine()->_items[data];
	convWidget->clearChoices();

	const ConversationLineList& toSayList = getTaskManager()->getGame().getAssets().getToSayList();
	const ConversationLineList::Speech &speech = toSayList.getLine(item._choice)->_speeches[0];

	_sayTask = new SayTask(speech._text, _convInfo._color);
	getTaskManager()->addTask(_sayTask);
	_substate = SAYING_CHOICE;
	_currentItem = &item;

	if (!speech.isRepeating()) {
		getTaskManager()->getGame().getGameData().getCurrentScene()->addExhaustedChoice(_convInfo._context, data + 1, _currentLineIndex + 1);
	}
}

void ConversationTask::showChoicesOrPick() {
	Game &game = getTaskManager()->getGame();
	GameData &gameData = game.getGameData();
	Scene *const scene = gameData.getScene(_sceneId);

	Common::Array<uint32> itemsWithValidChoices;
	Common::Array<uint32> itemsWithValidResponses;
	Common::Array<uint32> itemsWithValidNext;

	/*
		Collect valid "to say" choices (not exhausted and not empty).
		Collect valid responses (not exhausted and not empty).
		If there are at least two visible choices, we show them.
		If there is just one visible choice, pick it automatically ONLY if this is not the first choice in this conversation.
		Otherwise we don't start the conversation.
		If there are no visible choices, automatically pick first valid response.
		If nothing above applies, don't start the conversation.
	*/

	const ConversationInfo::Line *const currentLine = getCurrentLine();
	for (ConversationInfo::Items::size_type i = 0; i < currentLine->_items.size(); ++i) {
		const ConversationInfo::Item &item = currentLine->_items[i];

		if (scene->isChoiceExhausted(_convInfo._context, (uint8) i + 1, (uint8) _currentLineIndex + 1)) {
			continue;
		}
		const uint8 choice = item._choice;
		const uint8 response = item._response;
		const uint8 next = item._nextLineIndex;

		if (choice != 0) {
			itemsWithValidChoices.push_back(i);
		}

		if (response != 0) {
			itemsWithValidResponses.push_back(i);
		}

		if (next != 0) {
			itemsWithValidNext.push_back(i);
		}
	}

	if (itemsWithValidChoices.size() > 1) {
		ConversationWidget &widget = game.getGui().getConversationWidget();
		const ConversationLineList& toSayList = game.getAssets().getToSayList();

		for (Common::Array<uint32>::size_type i = 0; i < itemsWithValidChoices.size() && i < ConversationWidget::CONVERSATION_MAX_CHOICES; ++i) {
			const ConversationInfo::Item &item = currentLine->_items[itemsWithValidChoices[i]];
			const ConversationLineList::Line *const line = toSayList.getLine(item._choice);
			const Common::String widgetText = toUpperCP895(line->_speeches[0]._text);
			widget.setChoice((int) i, widgetText, itemsWithValidChoices[i]);
		}
		_substate = IDLE;
		_currentItem = nullptr;

		_haveChoices = true;
	} else if (itemsWithValidChoices.size() == 1 && _haveChoices) {
		const ConversationLineList& toSayList = game.getAssets().getToSayList();
		const ConversationInfo::Item &item = currentLine->_items[itemsWithValidChoices.front()];
		const ConversationLineList::Line *const line = toSayList.getLine(item._choice);

		_sayTask = new SayTask(line->_speeches[0]._text, _convInfo._color);
		getTaskManager()->addTask(_sayTask);
		_substate = SAYING_CHOICE;
		_currentItem = &item;

		if (!line->_speeches[0].isRepeating()) {
			game.getGameData().getCurrentScene()->addExhaustedChoice(_convInfo._context, itemsWithValidChoices.front() + 1, _currentLineIndex + 1);
		}

		_haveChoices = true;
	} else if (!itemsWithValidResponses.empty() && _haveChoices) {
		const ConversationLineList& responseList = game.getAssets().getResponseList();
		const ConversationInfo::Item &item = currentLine->_items[itemsWithValidResponses.front()];
		const ConversationLineList::Line *const line = responseList.getLine(item._response);

		_sayTask = new SayTask(line->_speeches[0]._text, _convInfo._color);
		getTaskManager()->addTask(_sayTask);
		_substate = SAYING_RESPONSE;
		_currentItem = &item;

		_haveChoices = true;
	} else if (!itemsWithValidNext.empty() && _haveChoices) {
		_currentLineIndex = currentLine->_items[itemsWithValidNext.front()]._nextLineIndex - 1;
		showChoicesOrPick();
	} else {
		if (_haveChoices) {
			finish();
		} else {
			_sayTask = new SayTask("Nothing to talk about.", _convInfo._color); // TODO: This is hardcoded in executable. Load it.
			getTaskManager()->addTask(_sayTask);
			_substate = SAYING_NO_CHOICES;
			_currentItem = nullptr;
		}
	}
}

const ConversationInfo::Line *ConversationTask::getCurrentLine() const {
	return &_convInfo._lines[_currentLineIndex];
}

void ConversationTask::finish() {
	setState(FINISHED);

	Game &game = getTaskManager()->getGame();
	ConversationWidget &widget = game.getGui().getConversationWidget();
	widget.setVisible(false);
	game.getGui().markDirty(); // TODO: Handle automatically when changing visibility.
}

void ConversationTask::startExtra() {
	const ConversationLineList& responseList = getTaskManager()->getGame().getAssets().getResponseList();
	const ConversationLineList::Line *const line = responseList.getLine(_currentItem->_response);
	if (!line->_extra.empty()) {
		_innerExecCtx = new ScriptExecutionContext(getTaskManager()->getGame());
		Command *const extraCmd = _innerExecCtx->getExtra(line->_extra);
		if (extraCmd) {
			Command::ExecuteResult res = _innerExecCtx->startCommand(extraCmd);
			if (res == Command::InProgress) {
				_substate = RUNNING_EXTRA;
			} else {
				delete _innerExecCtx;
				_innerExecCtx = nullptr;
			}
		} else {
			warning(_("Extra '%s' not found"), line->_extra.c_str());
			delete _innerExecCtx;
			_innerExecCtx = nullptr;
		}
	}
}

void ConversationTask::gotoNextLine() {
	if (_currentItem->_nextLineIndex == 0) {
		finish();
	} else {
		_currentLineIndex = _currentItem->_nextLineIndex - 1;
		showChoicesOrPick();
	}
}

}
