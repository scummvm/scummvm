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

#include "cryomni3d/dialogs_manager.h"

#include "common/debug.h"
#include "common/file.h"

namespace CryOmni3D {

DialogsManager::~DialogsManager() {
	delete[] _gtoBuffer;
}

void DialogsManager::loadGTO(const Common::String &gtoFileName) {
	Common::File gtoFile;
	if (!gtoFile.open(gtoFileName)) {
		error("Can't open GTO file '%s'", gtoFileName.c_str());
	}

	_labels.clear();
	_gtoEnd = nullptr;
	delete[] _gtoBuffer;
	_gtoBuffer = nullptr;

	uint gtoSize = gtoFile.size();
	_gtoBuffer = new char[gtoSize];
	gtoFile.read(_gtoBuffer, gtoSize);
	gtoFile.close();

	_gtoEnd = _gtoBuffer + gtoSize;

	populateLabels();
}

void DialogsManager::populateLabels() {
	/* Get labels count and populate the labels array */
	uint numLabels;
	const char *labelsP = strstr(_gtoBuffer, "LABELS=");
	if (labelsP) {
		labelsP += sizeof("LABELS=") - 1;
		for (; *labelsP == ' '; labelsP++) { }
		numLabels = atoi(labelsP);
	} else {
		numLabels = 0;
	}

	for (const char *labelP = _gtoBuffer; labelP != nullptr; labelP = nextLine(labelP)) {
		if (*labelP == ':') {
			/* Line starting with ':', it's a label */
			_labels.push_back(nextChar(labelP));
		}
	}

	if (_labels.size() != numLabels) {
		error("Bad labels number in GTO");
	}
}

const char *DialogsManager::findLabel(const char *label, const char **realLabel) const {
	uint labelLen = 0;
	/* Truncate input label */
	for (const char *labelP = label;
	        *labelP != '\0' &&
	        *labelP != ' ' &&
	        *labelP != '.' &&
	        *labelP != '\r'; labelP++, labelLen++) { }

	Common::Array<const char *>::const_iterator labelsIt;
	for (labelsIt = _labels.begin(); labelsIt != _labels.end(); labelsIt++) {
		if (!strncmp(*labelsIt, label, labelLen)) {
			break;
		}
	}

	if (labelsIt == _labels.end()) {
		error("Label not found");
	}

	if (realLabel) {
		*realLabel = *labelsIt;
	}
	return nextLine(*labelsIt);
}

Common::String DialogsManager::getLabelSound(const char *label) const {
	/* Remove starting : if any */
	if (*label == ':') {
		label++;
	}

	const char *labelEnd;
	for (labelEnd = label; *labelEnd >= '0' && *labelEnd <= 'Z'; labelEnd++) { }

	return Common::String(label, labelEnd);
}

const char *DialogsManager::findSequence(const char *sequence) const {
	uint sequenceLen = strlen(sequence);

	const char *lineP;
	for (lineP = _gtoBuffer; lineP != nullptr; lineP = nextLine(lineP)) {
		if (!strncmp(lineP, sequence, sequenceLen)) {
			/* Line starting with the sequence name */
			break;
		}
	}

	if (!lineP) {
		return nullptr;
	}

	/* Find next label */
	for (; lineP != nullptr && *lineP != ':'; lineP = nextLine(lineP)) { }

	/* Return the label name without it's ':' */
	return nextChar(lineP);
}

Common::String DialogsManager::findVideo(const char *data) const {
	data = previousMatch(data, ".FLC");
	if (data == nullptr) {
		return Common::String();
	}

	// Video name is without the extension
	const char *end = data;

	for (; data >= _gtoBuffer && *data != '\r'; data--) { }
	data++;

	if (data < _gtoBuffer || *data == '.') {
		return Common::String();
	}

	return Common::String(data, end);
}

Common::String DialogsManager::getText(const char *text) const {
	/* Skip '<' */
	text = nextChar(text);

	if (text == nullptr) {
		return Common::String();
	}

	const char *end;
	for (end = text; end < _gtoEnd && *end != '>'; end++) { }

	if (end == _gtoEnd) {
		return Common::String();
	}

	return Common::String(text, end);
}

void DialogsManager::reinitVariables() {
	for (Common::Array<DialogVariable>::iterator it = _dialogsVariables.begin();
	        it != _dialogsVariables.end(); it++) {
		it->value = 'N';
	}
}

const DialogsManager::DialogVariable &DialogsManager::find(const Common::String &name) const {
	for (Common::Array<DialogVariable>::const_iterator it = _dialogsVariables.begin();
	        it != _dialogsVariables.end(); it++) {
		if (it->name == name) {
			return *it;
		}
	}
	error("Can't find dialog variable %s", name.c_str());
}

DialogsManager::DialogVariable &DialogsManager::find(const Common::String &name) {
	for (Common::Array<DialogVariable>::iterator it = _dialogsVariables.begin();
	        it != _dialogsVariables.end(); it++) {
		if (it->name == name) {
			return *it;
		}
	}
	error("Can't find dialog variable %s", name.c_str());
}

const char *DialogsManager::nextLine(const char *currentPtr) const {
	for (; currentPtr < _gtoEnd && *currentPtr != '\r'; currentPtr++) { }

	/* Go after the \r */
	return nextChar(currentPtr);
}

const char *DialogsManager::nextChar(const char *currentPtr) const {
	if (currentPtr == nullptr || currentPtr < _gtoBuffer || currentPtr >= _gtoEnd) {
		return nullptr;
	}

	currentPtr++;

	if (currentPtr >= _gtoEnd) {
		return nullptr;
	} else {
		return currentPtr;
	}
}

const char *DialogsManager::previousMatch(const char *currentPtr, const char *str) const {
	if (currentPtr == nullptr || currentPtr >= _gtoEnd || currentPtr < _gtoBuffer) {
		return nullptr;
	}

	uint matchLen = strlen(str);
	for (; currentPtr >= _gtoBuffer; currentPtr--) {
		if (*currentPtr == str[0]) {
			if (!strncmp(currentPtr, str, matchLen)) {
				break;
			}
		}
	}

	if (currentPtr < _gtoBuffer) {
		return nullptr;
	} else {
		return currentPtr;
	}
}

bool DialogsManager::play(const Common::String &sequence, bool &slowStop) {
	const char *label = findSequence(sequence.c_str());

	if (!label) {
		error("Can't find sequence '%s' in GTO", sequence.c_str());
	}

	Common::String video = sequence;

	const char *text = findLabel(label);

	slowStop = false;
	bool playerLabel = !strncmp(label, "JOU", 3);
	bool didSomething = false;
	bool finished = false;
	/* Keep the gotoList outside the loop to avoid it being freed at the end of it and
	 * having label possibly pointing on free memory */
	Common::Array<DialogsManager::Goto> gotoList;
	while (!finished) {
		const char *actions;
		if (playerLabel) {
			/* If sequence begins with a player label go to action directly */
			playerLabel = false;
			actions = text;
			// Maybe a bug in original game, we should go to next line
		} else if (!strncmp(text, "<#>", 3)) {
			/* Text is empty: go to action directly */
			actions = nextLine(text);
		} else {
			/* Real text, play video */
			video = findVideo(text);
			Common::String properText = getText(text);
			Common::String sound = getLabelSound(label);
			Common::HashMap<Common::String, SubtitlesSettings>::const_iterator settingsIt =
			    _subtitlesSettings.find(video);
			if (settingsIt == _subtitlesSettings.end()) {
				settingsIt = _subtitlesSettings.find("default");
			}
			if (settingsIt == _subtitlesSettings.end()) {
				error("No video settings for %s", video.c_str());
			}
			playDialog(video, sound, properText, settingsIt->_value);
			didSomething = true;
			actions = nextLine(text);
		}
		gotoList = executeAfterPlayAndBuildGotoList(actions);
		Common::StringArray questions;
		bool endOfConversationFound = false;
		if (_ignoreNoEndOfConversation) {
			// Don't check if there is an end, so, there is one
			endOfConversationFound = true;
		}
		for (Common::Array<DialogsManager::Goto>::iterator it = gotoList.begin(); it != gotoList.end();
		        it++) {
			if (!endOfConversationFound && it->label.hasPrefix("JOU")) {
				// No need to get the real label here, we just need to know if the question ends up
				if (!executePlayerQuestion(it->text, true)) {
					endOfConversationFound = true;
				}
			}
			assert(it->text);
			const char *questionStart = it->text + 1;
			const char *questionEnd = questionStart;
			for (; *questionEnd != '>'; questionEnd++) { }
			questions.push_back(Common::String(questionStart, questionEnd));
		}
		uint eocInserted = uint(-1);
		if (!endOfConversationFound && questions.size() > 0) {
			eocInserted = questions.size();
			questions.push_back(_endOfConversationText);
		}
		if (questions.size() == 0) {
			// There are no choices, just quit with a pause to avoid abrupt ending
			slowStop = true;
			break;
		}

		if (gotoList[0].label.hasPrefix("JOU")) {
			// We must give a subject
			uint playerChoice = askPlayerQuestions(video, questions);
			didSomething = true;
			// -1 when shouldAbort
			if (playerChoice == uint(-1) || playerChoice == eocInserted) {
				break;
			}

			text = executePlayerQuestion(gotoList[playerChoice].text, false, &label);
			if (!text) {
				break;
			}
		} else if (gotoList[0].label.hasPrefix("MES")) {
			// Display a simple message
			const char *messageStart = gotoList[0].text + 1;
			const char *messageEnd = messageStart;
			for (; *messageEnd != '>'; messageEnd++) { }
			displayMessage(Common::String(messageStart, messageEnd));
			break;
		} else {
			// Unattended conversation: two NPC speak
			label = gotoList[0].label.c_str();
			text = gotoList[0].text;
		}
	}
	return didSomething;
}

Common::Array<DialogsManager::Goto> DialogsManager::executeAfterPlayAndBuildGotoList(
    const char *actions) {
	Common::Array<DialogsManager::Goto> gotos;

	for (; actions && *actions != ':'; actions = nextLine(actions)) {
		if (!strncmp(actions, "GOTO ", 5)) {
			buildGotoGoto(actions, gotos);
			break;
		} else if (!strncmp(actions, "IF ", 3)) {
			if (buildGotoIf(actions, gotos)) {
				break;
			}
		} else if (!strncmp(actions, "LET ", 4)) {
			executeLet(actions);
		} else if (!strncmp(actions, "SHOW ", 5)) {
			executeShow(actions);
		}
	}
	return gotos;
}

void DialogsManager::buildGotoGoto(const char *gotoLine, Common::Array<Goto> &gotos) {
	Common::String label;
	gotoLine = gotoLine + 5;
	while (true) {
		const char *labelEnd = gotoLine;
		for (labelEnd = gotoLine; *labelEnd >= '0' && *labelEnd <= 'Z'; labelEnd++) { }
		label = Common::String(gotoLine, labelEnd);

		if (label == "REM") {
			break;
		}

		// To build goto list, no need to get back the real label position
		const char *text = findLabel(label.c_str());
		gotos.push_back(Goto(label, text));

		if (*labelEnd == '.') {
			if (!strncmp(labelEnd, ".WAV", 4)) {
				labelEnd += 4;
			} else {
				debug("Problem with GOTO.WAV: '%s'", gotoLine);
			}
		}
		for (; *labelEnd == ' ' || *labelEnd == ','; labelEnd++) { }

		if (*labelEnd == '\r') {
			break;
		}

		// Next goto tag
		gotoLine = labelEnd;
	}
}

bool DialogsManager::buildGotoIf(const char *ifLine, Common::Array<Goto> &gotos) {
	ifLine += 3;

	bool finishedConditions = false;
	while (!finishedConditions) {
		const char *endVar = ifLine;
		const char *equalPos;
		// Find next '='
		for (; *endVar != '='; endVar++) { }
		equalPos = endVar;
		// Strip spaces at the end
		endVar--;
		for (; *endVar == ' '; endVar--) { }
		endVar++;
		Common::String variable(ifLine, endVar);

		const char *testValue = equalPos + 1;
		for (; *testValue == ' ' || *testValue == '\t'; testValue++) { }

		byte value = (*this)[variable];

		if (value != *testValue) {
			// IF is not taken, go to next line
			return false;
		}

		ifLine = testValue + 1;
		for (; *ifLine == ' ' || *ifLine == '\t'; ifLine++) { }

		if (!strncmp(ifLine, "AND IF ", 7)) {
			ifLine += 7;
		} else {
			finishedConditions = true;
		}
	}

	/* We are in the (implicit) THEN part of the IF
	 * ifLine points to the instruction */
	if (!strncmp(ifLine, "GOTO", 4)) {
		buildGotoGoto(ifLine, gotos);
	} else if (!strncmp(ifLine, "LET", 3)) {
		executeLet(ifLine);
	} else if (!strncmp(ifLine, "SHOW", 4)) {
		executeShow(ifLine);
	} else {
		debug("Invalid IF line: %s", ifLine);
		return false;
	}

	return true;
}

void DialogsManager::executeLet(const char *letLine) {
	letLine = letLine + 4;

	const char *endVar = letLine;
	const char *equalPos;
	// Find next '='
	for (; *endVar != '='; endVar++) { }
	equalPos = endVar;
	// Strip spaces at the end
	endVar--;
	for (; *endVar == ' '; endVar--) { }
	endVar++;
	Common::String variable(letLine, endVar);

	(*this)[variable] = equalPos[1];
}

void DialogsManager::executeShow(const char *showLine) {
	showLine = showLine + 5;

	const char *endShow = showLine;
	// Find next ')' and include it
	for (; *endShow != ')'; endShow++) { }
	endShow++;

	Common::String show(showLine, endShow);

	executeShow(show);
}

const char *DialogsManager::executePlayerQuestion(const char *text, bool dryRun,
        const char **realLabel) {
	// Go after the text
	const char *actions = nextLine(text);

	while (actions && *actions != ':') {
		if (!strncmp(actions, "IF ", 3)) {
			actions = parseIf(actions);
		} else if (!strncmp(actions, "LET ", 4)) {
			if (!dryRun) {
				executeLet(actions);
			}
			actions = nextLine(actions);
		} else if (!strncmp(actions, "GOTO ", 5)) {
			return findLabel(actions + 5, realLabel);
		} else {
			actions = nextLine(actions);
		}
	}

	// There were no GOTO, so it's the end of the conversation
	return nullptr;
}

const char *DialogsManager::parseIf(const char *ifLine) {
	ifLine += 3;

	bool finishedConditions = false;
	while (!finishedConditions) {
		const char *endVar = ifLine;
		const char *equalPos;
		// Find next '='
		for (; *endVar != '='; endVar++) { }
		equalPos = endVar;
		// Strip spaces at the end
		endVar--;
		for (; *endVar == ' '; endVar--) { }
		endVar++;
		Common::String variable(ifLine, endVar);

		const char *testValue = equalPos + 1;
		for (; *testValue == ' ' || *testValue == '\t'; testValue++) { }

		byte value = (*this)[variable];

		if (value != *testValue) {
			// IF is not taken, go to next line
			return nextLine(ifLine);
		}

		ifLine = testValue + 1;
		for (; *ifLine == ' ' || *ifLine == '\t'; ifLine++) { }

		if (!strncmp(ifLine, "AND IF ", 7)) {
			ifLine += 7;
		} else {
			finishedConditions = true;
		}
	}

	/* We are in the (implicit) THEN part of the IF
	 * ifLine points to the instruction */
	return ifLine;
}

void DialogsManager::registerSubtitlesSettings(const Common::String &videoName,
        const SubtitlesSettings &settings) {
	_subtitlesSettings[videoName] = settings;
}

} // End of namespace CryOmni3D
