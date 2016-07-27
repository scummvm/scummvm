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

#include "common/textconsole.h"
#include "titanic/true_talk/deskbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

static const TThandleQuoteEntry QUOTES[] = {
	{ 0x00000008, 0x00000000, 0x0003ACD0 },
	{ 0x00000007, 0x00000000, 0x0003ACDC },
	{ 0x00000006, 0x00000000, 0x0003ABF9 },
	{ 0x00000005, 0x00000000, 0x0003AD04 },
	{ 0x00000001, 0x56424144, 0x0003AE27 },
	{ 0x00000001, 0x52554445, 0x0003AE27 },
	{ 0x00000001, 0x5052534E, 0x00041EB3 },
	{ 0x00000001, 0x464F4F44, 0x00041EB3 },
	{ 0x00000001, 0x00000032, 0x00041EB1 },
	{ 0x00000002, 0x56424144, 0x0003AE27 },
	{ 0x00000002, 0x52554445, 0x0003AE27 },
	{ 0x00000002, 0x5052534E, 0x00041EB3 },
	{ 0x00000002, 0x464F4F44, 0x00041EB3 },
	{ 0x00000002, 0x00000032, 0x00041EB1 },
	{ 0x00000003, 0x56424144, 0x0003AE0E },
	{ 0x00000003, 0x52554445, 0x0003AE0E },
	{ 0x00000003, 0x5052534E, 0x00041EB3 },
	{ 0x00000003, 0x464F4F44, 0x00041EB3 },
	{ 0x00000003, 0x00000032, 0x00041EB1 },
	{ 0x00000010, 0x54524156, 0x0003ACFE },
	{ 0x00000010, 0x0000003C, 0x00041EB0 },
	{ 0x00000011, 0x00000000, 0x0003ABF9 },
	{ 0x00000015, 0x00000032, 0x0003AC70 },
	{ 0x00000012, 0x00000042, 0x0003AC7E },
	{ 0x00000013, 0x00000021, 0x0003AC70 },
	{ 0x0000001D, 0x00000021, 0x0003AC09 },
	{ 0x00000014, 0x00000042, 0x0003AE07 },
	{ 0x0000001B, 0x00000000, 0x00041EB2 },
	{ 0x0000001E, 0x00000000, 0x0003ACC1 },
	{ 0x0000001F, 0x00000000, 0x0003AC3E },
	{ 0x0000000C, 0x00000000, 0x0003AE4C },
	{ 0x0000000D, 0x00000000, 0x0003AE4C },
	{ 0x0000000E, 0x00000000, 0x0003AE4B },
	{ 0x0000000F, 0x00000000, 0x0003AE4B },
	{ 0x00000020, 0x00000019, 0x0003AE24 },
	{ 0x00000021, 0x0000000F, 0x0003AE10 },
	{ 0x0000001C, 0x00000032, 0x00041EB0 },
	{ 0x00000023, 0x00000000, 0x0003AC46 },
	{ 0x00000024, 0x00000000, 0x0003AE1F },
	{ 0x00000025, 0x00000000, 0x0003AE14 },
	{ 0x00000026, 0x00000000, 0x0003AC7B },
	{ 0x00000030, 0x00000000, 0x0003AE3D },
	{ 0x00000027, 0x424F5953, 0x0003AE5D },
	{ 0x00000027, 0x4749524C, 0x0003AE5E },
	{ 0x00000027, 0x00000000, 0x0003AE5C },
	{ 0x00000028, 0x00000000, 0x0003AE5B },
	{ 0x00000029, 0x00000000, 0x0003AE58 },
	{ 0x0000002A, 0x00000000, 0x0003AE59 },
	{ 0x0000002B, 0x00000000, 0x0003AE5A },
	{ 0x0000002C, 0x00000000, 0x0003AE57 },
	{ 0x0000002D, 0x00000000, 0x0003AE5C },
	{ 0x0000002E, 0x424F5953, 0x0003AE5A },
	{ 0x0000002E, 0x4749524C, 0x0003AE5A },
	{ 0x0000002E, 0x00000000, 0x0003AE5A },
	{ 0x00000032, 0x00000000, 0x0003AE0E },
	{ 0x00000033, 0x00000000, 0x0003AE27 },
	{ 0x00000034, 0x00000000, 0x0003AE24 },
	{ 0x00000035, 0x00000000, 0x0003AE3E },
	{ 0x00000037, 0x00000000, 0x0003AE26 },
	{ 0x00000038, 0x00000000, 0x0003AEC0 },
	{ 0x00000039, 0x00000000, 0x0003AEC1 },
	{ 0x0000003A, 0x00000000, 0x0003AC7F },
	{ 0x0000003B, 0x00000000, 0x0003ADD5 },
	{ 0x0000003C, 0x00000000, 0x0003AEC5 },
	{ 0x0000003D, 0x00000000, 0x0003AEC9 },
	{ 0x0000003F, 0x00000000, 0x0003ABC5 },
	{ 0x0000003E, 0x00000000, 0x0003ABC5 },
	{ 0x00000040, 0x00000000, 0x0003AFB0 },
	{ 0x00000041, 0x00000000, 0x0003AFDC },
	{ 0x00000042, 0x00000000, 0x0003AFB5 },
	{ 0x00000043, 0x00000000, 0x0003AFDD },
	{ 0x00000044, 0x00000000, 0x0003AFDD },
	{ 0x00000045, 0x0000000A, 0x0003AC7E },
	{ 0x00000046, 0x00000000, 0x0003AF6E },
	{ 0x00000000, 0x00000000, 0x00000000 }
};

DeskbotScript::DeskbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0) {
	CTrueTalkManager::setFlags(18, 0);
	CTrueTalkManager::setFlags(19, 0);
	CTrueTalkManager::setFlags(20, 0);
	CTrueTalkManager::setFlags(21, 0);
	CTrueTalkManager::setFlags(22, 0);

	setupDials(0, 0, 0);
	_array[0] = 100;
	if (_currentDialNum == 1)
		_currentDialNum = 0;

	loadRanges("Ranges/Deskbot");
	loadResponses("Responses/Deskbot", 4);
	setupSentences();
	_tagMappings.load("TagMap/Deskbot");
	_words.load("Words/Deskbot");
}

void DeskbotScript::setupSentences() {
	_mappings.load("Mappings/Deskbot", 4);
	_entries.load("Sentences/Deskbot");
	_dialValues[0] = _dialValues[1] = 0;
	_field68 = 0;
	_entryCount = 0;
}

int DeskbotScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	// TODO
	return 0;
}

ScriptChangedResult DeskbotScript::scriptChanged(TTscriptBase *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

int DeskbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		uint val, uint tagId, uint remainder) {
	switch (tagId) {
	case MKTAG('A', 'D', 'V', 'T'):
	case MKTAG('A', 'R', 'T', 'I'):
	case MKTAG('A', 'R', 'T', 'Y'):
	case MKTAG('B', 'R', 'N', 'D'):
	case MKTAG('C', 'O', 'M', 'D'):
	case MKTAG('D', 'N', 'C', 'E'):
	case MKTAG('H', 'B', 'B', 'Y'):
	case MKTAG('L', 'I', 'T', 'R'):
	case MKTAG('M', 'A', 'G', 'S'):
	case MKTAG('M', 'C', 'P', 'Y'):
	case MKTAG('M', 'I', 'N', 'S'):
	case MKTAG('M', 'U', 'S', 'I'):
	case MKTAG('N', 'I', 'K', 'E'):
	case MKTAG('S', 'F', 'S', 'F'):
	case MKTAG('S', 'O', 'A', 'P'):
	case MKTAG('S', 'O', 'N', 'G'):
	case MKTAG('S', 'P', 'R', 'T'):
	case MKTAG('T', 'E', 'A', 'M'):
	case MKTAG('T', 'V', 'S', 'H'):
		tagId = MKTAG('E', 'N', 'T', 'N');
		break;
	case MKTAG('A', 'C', 'T', 'R'):
	case MKTAG('A', 'C', 'T', 'S'):
	case MKTAG('A', 'U', 'T', 'H'):
	case MKTAG('B', 'A', 'R', 'K'):
	case MKTAG('B', 'A', 'R', 'U'):
	case MKTAG('B', 'L', 'F', '1'):
	case MKTAG('B', 'L', 'F', '2'):
	case MKTAG('B', 'L', 'P', '1'):
	case MKTAG('B', 'L', 'P', '2'):
	case MKTAG('B', 'L', 'P', '3'):
	case MKTAG('B', 'L', 'P', '4'):
	case MKTAG('B', 'L', 'R', '1'):
	case MKTAG('B', 'L', 'R', '2'):
	case MKTAG('B', 'L', 'T', '1'):
	case MKTAG('B', 'L', 'T', '2'):
	case MKTAG('B', 'L', 'T', '3'):
	case MKTAG('B', 'L', 'T', '4'):
	case MKTAG('B', 'L', 'T', '5'):
	case MKTAG('B', 'O', 'Y', 'S'):
	case MKTAG('C', 'O', 'P', 'S'):
	case MKTAG('D', 'C', 'T', 'R'):
	case MKTAG('F', 'A', 'M', 'E'):
	case MKTAG('F', 'A', 'S', 'H'):
	case MKTAG('G', 'I', 'R', 'L'):
	case MKTAG('H', 'E', 'R', 'O'):
	case MKTAG('H', 'O', 'S', 'T'):
	case MKTAG('K', 'N', 'O', 'B'):
	case MKTAG('N', 'H', 'R', 'O'):
	case MKTAG('R', 'A', 'C', 'E'):
	case MKTAG('S', 'C', 'I', 'T'):
	case MKTAG('T', 'D', 'V', 'P'):
	case MKTAG('T', 'W', 'A', 'T'):
	case MKTAG('W', 'E', 'A', 'T'):
	case MKTAG('W', 'W', 'E', 'B'):
		tagId = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 'T'):
	case MKTAG('F', 'I', 'L', 'M'):
	case MKTAG('J', 'F', 'O', 'D'):
	case MKTAG('L', 'I', 'Q', 'D'):
		tagId = MKTAG('F', 'O', 'O', 'D');
		break;
	case MKTAG('C', 'R', 'I', 'M'):
	case MKTAG('C', 'S', 'P', 'Y'):
	case MKTAG('D', 'R', 'U', 'G'):
		tagId = MKTAG('V', 'B', 'A', 'D');
		break;
	case MKTAG('E', 'A', 'R', 'T'):
	case MKTAG('H', 'O', 'M', 'E'):
	case MKTAG('N', 'P', 'L', 'C'):
	case MKTAG('P', 'L', 'A', 'N'):
		tagId = MKTAG('P', 'L', 'A', 'C');
		break;
	case MKTAG('F', 'A', 'U', 'N'):
	case MKTAG('F', 'I', 'S', 'H'):
	case MKTAG('F', 'L', 'O', 'R'):
		tagId = MKTAG('N', 'A', 'T', 'R');
		break;
	case MKTAG('H', 'H', 'L', 'D'):
	case MKTAG('T', 'O', 'Y', 'S'):
	case MKTAG('W', 'E', 'A', 'P'):
		tagId = MKTAG('M', 'A', 'C', 'H');
		break;
	case MKTAG('M', 'L', 'T', 'Y'):
	case MKTAG('P', 'G', 'R', 'P'):
	case MKTAG('P', 'T', 'I', 'C'):
		tagId = MKTAG('G', 'R', 'U', 'P');
		break;
	case MKTAG('P', 'K', 'U', 'P'):
	case MKTAG('S', 'E', 'X', '1'):
	case MKTAG('S', 'W', 'E', 'R'):
		tagId = MKTAG('R', 'U', 'D', 'E');
		break;
	case MKTAG('P', 'H', 'I', 'L'):
	case MKTAG('R', 'C', 'K', 'T'):
		tagId = MKTAG('S', 'C', 'I', 'E');
		break;
	case MKTAG('T', 'R', 'A', '2'):
	case MKTAG('T', 'R', 'A', '3'):
		tagId = MKTAG('T', 'R', 'A', 'V');
		break;
	default:
		break;
	}
	
	int loopCounter = 0;
	for (const TThandleQuoteEntry *qe = QUOTES; qe->_index && loopCounter < 2; ++qe) {
		if (!qe->_index) {
			// End of list; start at beginning again
			++loopCounter;
			qe = QUOTES;
		}

		if (qe->_index == val && (
			(tagId == 0 && loopCounter == 2) ||
			(qe->_tagId < MKTAG('A', 'A', 'A', 'A')) ||
			(qe->_tagId == tagId)
			)) {
			uint foundTagId = qe->_tagId;
			if (foundTagId > 0 && foundTagId < 100) {
				if (!tagId)
					foundTagId >>= 1;
				if (getRandomNumber(100) < foundTagId)
					return 1;
			}

			uint dialogueId = qe->_dialogueId;
			if (dialogueId >= 270000 && dialogueId <= 275000) {
				dialogueId -= 270000;
				if (dialogueId >= 3)
					error("Invalid dialogue index in BarbotScript");
				const int RANDOM_LIMITS[3] = { 30, 50, 70 };
				int rangeLimit = RANDOM_LIMITS[dialogueId];
				int dialRegion = getDialRegion(0);

				if (dialRegion != 1) {
					rangeLimit = MAX(rangeLimit - 20, 20);
				}

				dialogueId = (((int)remainder + 25) % 100) >= rangeLimit ? 241191 : 241166;
			}

			addResponse(getDialogueId(dialogueId));
			applyResponse();
			return 2;
		}
	}

	return 1;
}

int DeskbotScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc22(int id) const {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc23() const {
	warning("TODO");
	return 0;
}

int DeskbotScript::proc25(int val1, const int *srcIdP, TTroomScript *roomScript, TTsentence *sentence) {
	uint id;

	switch (val1) {
	case 1:
		id = *srcIdP;
		if (id == 240431 || id == 240432) {
			switch (getValue(1)) {
			case 1:
				id = 240336;
				break;
			case 2:
				id = addAssignedRoomDialogue();
				break;
			case 3:
				if (getValue(3) == 1) {
					if (id == 240431)
						id = 240432;
				}
				else {
					if (id == 240432)
						id = 240431;
				}
			default:
				break;
			}

			addResponse(getDialogueId(id));
			applyResponse();
			return 2;
		}
		break;

	case 2:
		if (getValue(1) == 1)
			return true;
		break;

	default:
		break;
	}
	
	return 0;
}

void DeskbotScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

int DeskbotScript::proc36(int id) const {
	warning("TODO");
	return 0;
}

bool DeskbotScript::isDial0Medium() const {
	return getDialRegion(0) == 1;
}

bool DeskbotScript::isDial0Low() const {
	return getDialRegion(0) == 0;
}

bool DeskbotScript::isDial1Medium() const {
	return getDialRegion(1) == 1;
}

bool DeskbotScript::isDial1Low() const {
	return getDialRegion(1) == 0;
}

int DeskbotScript::addAssignedRoomDialogue() {
	if (isDial1Medium()) {
		addResponse(getDialogueId(240407));
		addResponse(getDialogueId(241510));
		CTrueTalkManager::setFlags(1, 1);
		CTrueTalkManager::triggerAction(19, 1);

		int roomNum, floorNum, elevatorNum;
		getAssignedRoom(&roomNum, &floorNum, &elevatorNum);

		addResponse(getDialogueId(241317 + roomNum));
		addResponse(getDialogueId(241271 + floorNum));
		addResponse(getDialogueId(241511));
		addResponse(getDialogueId(241313 + elevatorNum));

		return 241512;
	} else {
		return 240567;
	}
}

} // End of namespace Titanic
