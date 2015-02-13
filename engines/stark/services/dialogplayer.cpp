/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/services/dialogplayer.h"

#include "engines/stark/resources/dialog.h"
#include "engines/stark/resources/speech.h"

namespace Stark {

DialogPlayer::DialogPlayer() :
		_currentDialog(nullptr),
		_currentReply(nullptr),
		_speechReady(false) {
}

DialogPlayer::~DialogPlayer() {
}

void DialogPlayer::run(Resources::Dialog *dialog) {
	reset();

	_currentDialog = dialog;
	buildOptions();
}

bool DialogPlayer::isRunning() {
	return _currentDialog != nullptr;
}

void DialogPlayer::buildOptions() {
	Common::Array<Resources::Dialog::Topic *> availableTopics = _currentDialog->listAvailableTopics();

	// TODO: This is very minimal, complete

	for (uint i = 0; i < availableTopics.size(); i++) {
		Option option;

		option._type = kOptionTypeAsk;
		option._topic = availableTopics[i];
		option._caption = availableTopics[i]->getCaption();
		option._replyIndex = availableTopics[i]->getNextReplyIndex();

		_options.push_back(option);
	}

	if (_options.size() == 1) {
		// Only one option, just run it
		selectOption(0);
	}
}

void DialogPlayer::selectOption(uint32 index) {
	Option &option = _options[index];

	//TODO: Complete

	switch (option._type) {
	case kOptionTypeAsk: {
		Resources::Dialog::Topic *topic = option._topic;

		// Set the current reply
		_currentReply = topic->startReply(index);

		Resources::Speech *speech = _currentReply->getCurrentSpeech();
		if (speech) {
			_speechReady = true;
		} else {
			onReplyEnd();
		}
		break;
	}
	default:
		error("Unhandled option type %d", option._type);
	}
}

void DialogPlayer::onReplyEnd() {
	Resources::Dialog *nextDialog = _currentDialog->getNextDialog(_currentReply);

	//TODO: Complete

	if (nextDialog) {
		run(nextDialog);
	} else {
		// Quit the dialog
		reset();
	}
}

void DialogPlayer::reset() {
	_currentDialog = nullptr;
	_currentReply = nullptr;
	_speechReady = false;
	_options.clear();
}

void DialogPlayer::update() {
	if (!_currentDialog || !_currentReply) {
		return; // Nothing to do
	}

	//TODO: Complete / Refactor

	Resources::Speech *speech = _currentReply->getCurrentSpeech();
	if (speech && _speechReady) {
		// A new line can be played
		speech->playSound();
		_speechReady = false;
		return;
	}

	if (!speech || !speech->isPlaying()) {
		// A line has ended, play the next one
		_currentReply->goToNextLine();
		speech = _currentReply->getCurrentSpeech();
		if (speech) {
			_speechReady = true;
		} else {
			onReplyEnd();
		}
	}
}

} // End of namespace Stark
