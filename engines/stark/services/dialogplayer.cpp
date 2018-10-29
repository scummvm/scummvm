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
#include "engines/stark/services/diary.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/resources/script.h"
#include "engines/stark/resources/speech.h"

namespace Stark {

DialogPlayer::DialogPlayer() :
		_currentDialog(nullptr),
		_currentReply(nullptr),
		_interruptedDialog(nullptr),
		_interruptedReply(nullptr),
		_speechReady(false),
		_singleSpeech(nullptr),
		_optionsAvailable(false) {
}

DialogPlayer::~DialogPlayer() {}

void DialogPlayer::run(Resources::Dialog *dialog) {
	reset();

	StarkUserInterface->setInteractive(false);

	if (!_currentDialog) {
		Common::String dialogTitle = dialog->getDiaryTitle();
		int32 characterId = dialog->getCharacter();
		Common::String characterName = StarkGlobal->getCharacterName(characterId);

		StarkDiary->openDialog(dialogTitle, characterName, characterId);
	}

	_currentDialog = dialog;
	buildOptions();
}

void DialogPlayer::playSingle(Resources::Speech *speech) {
	reset();

	_singleSpeech = speech;
	_speechReady = true;
}

bool DialogPlayer::isRunning() const {
	return _currentDialog != nullptr || _interruptedDialog != nullptr;
}

bool DialogPlayer::isSpeechReady() const {
	return _speechReady;
}

bool DialogPlayer::isSpeechReady(Resources::Speech *speech) const {
	return _speechReady && _singleSpeech == speech;
}

Resources::Speech *DialogPlayer::acquireReadySpeech() {
	assert(_speechReady);
	_speechReady = false;

	if (_singleSpeech) {
		return _singleSpeech;
	} else {
		return _currentReply->getCurrentSpeech();
	}
}

bool DialogPlayer::areOptionsAvailable() const {
	return _optionsAvailable;
}

Common::Array<DialogPlayer::Option> DialogPlayer::listOptions() const {
	return _options;
}

void DialogPlayer::removeLastOnlyOption() {
	int32 lastOnlyOptionIndex = -1;

	for (uint i = 0; i < _options.size(); i++) {
		Resources::Dialog::Topic *topic = _options[i]._topic;
		Resources::Dialog::Reply *reply = topic->getReply(_options[i]._replyIndex);
		if (reply->isLastOnly()) {
			lastOnlyOptionIndex = i;
			break;
		}
	}

	if (lastOnlyOptionIndex >= 0) {
		_options.remove_at(lastOnlyOptionIndex);
	}
}

void DialogPlayer::buildOptions() {
	Resources::Dialog::TopicArray availableTopics = _currentDialog->listAvailableTopics();

	for (uint i = 0; i < availableTopics.size(); i++) {
		Option option;

		option._type = kOptionTypeAsk;
		option._topic = availableTopics[i];
		option._caption = availableTopics[i]->getCaption();
		option._replyIndex = availableTopics[i]->getNextReplyIndex();

		Resources::Dialog::Reply *reply = availableTopics[i]->getReply(option._replyIndex);
		if (reply->checkCondition()) {
			_options.push_back(option);
		}
	}

	if (_options.size() > 1) {
		removeLastOnlyOption();
	}

	if (_options.size() == 1) {
		// Only one option, just run it
		selectOption(0);
	} else {
		_optionsAvailable = true;
	}
}

void DialogPlayer::selectOption(uint32 index) {
	_optionsAvailable = false;

	Option &option = _options[index];

	//TODO: Complete

	switch (option._type) {
	case kOptionTypeAsk: {
		Resources::Dialog::Topic *topic = option._topic;

		// Set the current reply
		_currentReply = topic->startReply(option._replyIndex);

		Resources::Speech *speech = _currentReply->getCurrentSpeech();
		if (speech) {
			StarkDiary->logSpeech(speech->getPhrase(), speech->getCharacterId());

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
	Resources::Script *nextScript = _currentDialog->getNextScript(_currentReply);
	Resources::Dialog *nextDialog = _currentDialog->getNextDialog(_currentReply);

	if (nextScript) {
		// Save the dialog player's state before running the script,
		// so that we can restore it when the script ends.
		// The script might run another dialog.
		saveToInterruptionSlot();

		nextScript->addReturnObject(_currentDialog);
		nextScript->execute(Resources::Script::kCallModeDialogCreateSelections);
	} else if (nextDialog) {
		run(nextDialog);
	} else {
		// Quit the dialog
		reset();
		StarkUserInterface->setInteractive(true);
	}
}

void DialogPlayer::reset() {
	if (_currentDialog) {
		StarkDiary->closeDialog();
	}

	_currentDialog = nullptr;
	_currentReply = nullptr;
	_singleSpeech = nullptr;
	_speechReady = false;
	_optionsAvailable = false;
	_options.clear();
}

void DialogPlayer::update() {
	if (_singleSpeech || !_currentDialog || !_currentReply) {
		return; // Nothing to do
	}

	//TODO: Complete

	Resources::Speech *speech = _currentReply->getCurrentSpeech();
	if (speech && _speechReady) {
		// A new line is already ready, no need to prepare another one
		return;
	}

	if (!speech || !speech->isPlaying()) {
		// A line has ended, play the next one
		_currentReply->goToNextLine();
		speech = _currentReply->getCurrentSpeech();
		if (speech) {
			StarkDiary->logSpeech(speech->getPhrase(), speech->getCharacterId());

			_speechReady = true;
		} else {
			onReplyEnd();
		}
	}
}

void DialogPlayer::resume(Resources::Dialog *dialog) {
	assert(_interruptedDialog == dialog);

	// Restore our state from before running the script
	restoreFromInterruptionSlot();

	Resources::Dialog *nextDialog = _currentDialog->getNextDialog(_currentReply);
	 if (nextDialog) {
		run(nextDialog);
	} else {
		// Quit the dialog
		reset();
		StarkUserInterface->setInteractive(true);
	}
}

void DialogPlayer::saveToInterruptionSlot() {
	_interruptedDialog = _currentDialog;
	_interruptedReply = _currentReply;
}

void DialogPlayer::restoreFromInterruptionSlot() {
	_currentDialog = _interruptedDialog;
	_currentReply = _interruptedReply;
	_interruptedDialog = nullptr;
	_interruptedReply = nullptr;
}
} // End of namespace Stark
