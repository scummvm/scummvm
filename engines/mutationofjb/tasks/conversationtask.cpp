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
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/gamescreen.h"
#include "mutationofjb/script.h"
#include "mutationofjb/tasks/saytask.h"
#include "mutationofjb/tasks/sequentialtask.h"
#include "mutationofjb/tasks/taskmanager.h"
#include "mutationofjb/util.h"
#include "mutationofjb/widgets/conversationwidget.h"

namespace MutationOfJB {

void ConversationTask::start() {
	setState(RUNNING);

	Game &game = getTaskManager()->getGame();
	game.getGameScreen().showConversationWidget(true);
	ConversationWidget &widget = game.getGameScreen().getConversationWidget();
	widget.setCallback(this);

	_currentGroupIndex = 0;

	showChoicesOrPick();
}

void ConversationTask::update() {
	if (_sayTask) {
		if (_sayTask->getState() == Task::FINISHED) {
			_sayTask.reset();

			switch (_substate) {
			case SAYING_NO_QUESTIONS:
				finish();
				break;
			case SAYING_QUESTION: {
				const ConversationLineList &responseList = getTaskManager()->getGame().getAssets().getResponseList();
				const ConversationLineList::Line *const line = responseList.getLine(_currentItem->_response);

				_substate = SAYING_RESPONSE;
				createSayTasks(line);
				getTaskManager()->startTask(_sayTask);
				break;
			}
			case SAYING_RESPONSE: {
				startExtra();

				if (_substate != RUNNING_EXTRA) {
					gotoNextGroup();
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

			gotoNextGroup();
		}
	}
}

void ConversationTask::onChoiceClicked(ConversationWidget *convWidget, int, uint32 data) {
	const ConversationInfo::Item &item = getCurrentGroup()[data];
	convWidget->clearChoices();

	const ConversationLineList &toSayList = getTaskManager()->getGame().getAssets().getToSayList();
	const ConversationLineList::Line *line = toSayList.getLine(item._question);

	_substate = SAYING_QUESTION;
	createSayTasks(line);
	getTaskManager()->startTask(_sayTask);
	_currentItem = &item;

	if (!line->_speeches[0].isRepeating()) {
		getTaskManager()->getGame().getGameData().getCurrentScene()->addExhaustedConvItem(_convInfo._context, data + 1, _currentGroupIndex + 1);
	}
}

void ConversationTask::showChoicesOrPick() {
	Game &game = getTaskManager()->getGame();
	GameData &gameData = game.getGameData();
	Scene *const scene = gameData.getScene(_sceneId);
	if (!scene) {
		return;
	}

	Common::Array<uint32> itemsWithValidQuestions;
	Common::Array<uint32> itemsWithValidResponses;
	Common::Array<uint32> itemsWithValidNext;

	/*
		Collect valid questions (not exhausted and not empty).
		Collect valid responses (not exhausted and not empty).
		If there are at least two visible questions, we show them.
		If there is just one visible question, pick it automatically ONLY if this is not the first question in this conversation.
		Otherwise we don't start the conversation.
		If there are no visible questions, automatically pick the first valid response.
		If nothing above applies, don't start the conversation.
	*/

	const ConversationInfo::ItemGroup &currentGroup = getCurrentGroup();
	for (ConversationInfo::ItemGroup::size_type i = 0; i < currentGroup.size(); ++i) {
		const ConversationInfo::Item &item = currentGroup[i];

		if (scene->isConvItemExhausted(_convInfo._context, static_cast<uint8>(i + 1), static_cast<uint8>(_currentGroupIndex + 1))) {
			continue;
		}
		const uint8 toSay = item._question;
		const uint8 response = item._response;
		const uint8 next = item._nextGroupIndex;

		if (toSay != 0) {
			itemsWithValidQuestions.push_back(i);
		}

		if (response != 0) {
			itemsWithValidResponses.push_back(i);
		}

		if (next != 0) {
			itemsWithValidNext.push_back(i);
		}
	}

	if (itemsWithValidQuestions.size() > 1) {
		ConversationWidget &widget = game.getGameScreen().getConversationWidget();
		const ConversationLineList &toSayList = game.getAssets().getToSayList();

		for (Common::Array<uint32>::size_type i = 0; i < itemsWithValidQuestions.size() && i < ConversationWidget::CONVERSATION_MAX_CHOICES; ++i) {
			const ConversationInfo::Item &item = currentGroup[itemsWithValidQuestions[i]];
			const ConversationLineList::Line *const line = toSayList.getLine(item._question);
			const Common::String widgetText = toUpperCP895(line->_speeches[0]._text);
			widget.setChoice(static_cast<int>(i), widgetText, itemsWithValidQuestions[i]);
		}
		_substate = IDLE;
		_currentItem = nullptr;

		_haveChoices = true;
	} else if (itemsWithValidQuestions.size() == 1 && _haveChoices) {
		const ConversationLineList &toSayList = game.getAssets().getToSayList();
		const ConversationInfo::Item &item = currentGroup[itemsWithValidQuestions.front()];
		const ConversationLineList::Line *const line = toSayList.getLine(item._question);

		_substate = SAYING_QUESTION;
		createSayTasks(line);
		getTaskManager()->startTask(_sayTask);
		_currentItem = &item;

		if (!line->_speeches[0].isRepeating()) {
			game.getGameData().getCurrentScene()->addExhaustedConvItem(_convInfo._context, itemsWithValidQuestions.front() + 1, _currentGroupIndex + 1);
		}

		_haveChoices = true;
	} else if (!itemsWithValidResponses.empty() && _haveChoices) {
		const ConversationLineList &responseList = game.getAssets().getResponseList();
		const ConversationInfo::Item &item = currentGroup[itemsWithValidResponses.front()];
		const ConversationLineList::Line *const line = responseList.getLine(item._response);

		_substate = SAYING_RESPONSE;
		createSayTasks(line);
		getTaskManager()->startTask(_sayTask);
		_currentItem = &item;

		_haveChoices = true;
	} else if (!itemsWithValidNext.empty() && _haveChoices) {
		_currentGroupIndex = currentGroup[itemsWithValidNext.front()]._nextGroupIndex - 1;
		showChoicesOrPick();
	} else {
		if (_haveChoices) {
			finish();
		} else {
			_sayTask = TaskPtr(new SayTask("Nothing to talk about.", _convInfo._color)); // TODO: This is hardcoded in executable. Load it.
			getTaskManager()->startTask(_sayTask);
			_substate = SAYING_NO_QUESTIONS;
			_currentItem = nullptr;
		}
	}
}

const ConversationInfo::ItemGroup &ConversationTask::getCurrentGroup() const {
	assert(_currentGroupIndex < _convInfo._itemGroups.size());
	return _convInfo._itemGroups[_currentGroupIndex];
}

void ConversationTask::finish() {
	setState(FINISHED);

	Game &game = getTaskManager()->getGame();
	game.getGameScreen().showConversationWidget(false);
	ConversationWidget &widget = game.getGameScreen().getConversationWidget();
	widget.setCallback(nullptr);
}

void ConversationTask::startExtra() {
	const ConversationLineList &responseList = getTaskManager()->getGame().getAssets().getResponseList();
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
			warning("Extra '%s' not found", line->_extra.c_str());
			delete _innerExecCtx;
			_innerExecCtx = nullptr;
		}
	}
}

void ConversationTask::gotoNextGroup() {
	if (_currentItem->_nextGroupIndex == 0) {
		finish();
	} else {
		_currentGroupIndex = _currentItem->_nextGroupIndex - 1;
		showChoicesOrPick();
	}
}

void ConversationTask::createSayTasks(const ConversationLineList::Line *line) {
	if (line->_speeches.size() == 1) {
		const ConversationLineList::Speech &speech = line->_speeches[0];
		_sayTask = TaskPtr(new SayTask(speech._text, getSpeechColor(speech)));
	} else {
		TaskPtrs tasks;
		for (ConversationLineList::Speeches::const_iterator it = line->_speeches.begin(); it != line->_speeches.end(); ++it) {
			tasks.push_back(TaskPtr(new SayTask(it->_text, getSpeechColor(*it))));
		}
		_sayTask = TaskPtr(new SequentialTask(tasks));
	}
}

uint8 ConversationTask::getSpeechColor(const ConversationLineList::Speech &speech) {
	uint8 color = WHITE;
	if (_substate == SAYING_RESPONSE) {
		color = _convInfo._color;
		if (_mode == TalkCommand::RAY_AND_BUTTLEG_MODE) {
			if (speech.isFirstSpeaker()) {
				color = GREEN;
			} else if (speech.isSecondSpeaker()) {
				color = LIGHTBLUE;
			}
		}
	}
	return color;
}

}
