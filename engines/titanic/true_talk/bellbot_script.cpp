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
#include "titanic/true_talk/bellbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/core/node_item.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

uint BellbotScript::_oldId;

static const RoomDialogueId ROOM_DIALOGUE_IDS[] = {
	{ 100, 201442 },{ 101, 201417 },{ 107, 201491 },{ 108, 201421 },
	{ 109, 201437 },{ 110, 201431 },{ 111, 201457 },{ 112, 201411 },
	{ 113, 201424 },{ 114, 201464 },{ 115, 201407 },{ 116, 201468 },
	{ 117, 201447 },{ 122, 201491 },{ 123, 201299 },{ 124, 201479 },
	{ 125, 201480 },{ 126, 201476 },{ 127, 201483 },{ 128, 201399 },
	{ 129, 201400 },{ 130, 201387 },{ 131, 201395 },{ 132, 201388 },
	{ 0, 0 }
};

BellbotScript::BellbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0),
		_responseFlag(false), _room107First(false) {
	CTrueTalkManager::setFlags(25, 0);
	CTrueTalkManager::setFlags(24, 0);
	CTrueTalkManager::setFlags(40, 0);
	CTrueTalkManager::setFlags(26, 0);

	setupDials(0, 0, 0);
	_array[0] = 100;
	_array[1] = 0;

	loadRanges("Ranges/Bellbot");
	loadResponses("Responses/Bellbot", 4);
	setupSentences();
	_tagMappings.load("TagMap/Bellbot");
	_words.load("Words/Bellbot");
	_quotes.load("Quotes/Bellbot");
	_states.load("States/Bellbot");
	_preResponses.load("PreResponses/Bellbot");
	_phrases.load("Phrases/Bellbot");
}

void BellbotScript::setupSentences() {
	_mappings.load("Mappings/Bellbot", 1);
	_entries.load("Sentences/Bellbot");
	for (int idx = 1; idx < 20; ++idx)
		_sentences[idx].load(CString::format("Sentences/Bellbot/%d", idx));

	_responseFlag = false;
	_field68 = 0;
	_entryCount = 0;
}

int BellbotScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	int val24 = getValue(24);
	CTrueTalkManager::setFlags(24, 0);

	int result = preprocess(roomScript, sentence);
	if (result != 1)
		return 1;

	CTrueTalkManager::setFlags(23, 0);
	setState(0);
	if (getValue(1) <= 2)
		updateCurrentDial(1);

	if (g_language != Common::DE_DEU) {
		// Handle room specific sentences
		switch (roomScript->_scriptId) {
		case 101:
			if (getValue(2) == 1) {
				result = processEntries(&_sentences[11], 0, roomScript, sentence);
			}
			break;

		case 107:
			result = processEntries(&_sentences[5], 0, roomScript, sentence);
			break;

		case 108:
			result = processEntries(&_sentences[7], 0, roomScript, sentence);
			break;

		case 109:
			result = processEntries(&_sentences[13], 0, roomScript, sentence);
			break;

		case 110:
			result = processEntries(&_sentences[16], 0, roomScript, sentence);
			break;

		case 111:
			result = processEntries(&_sentences[10], 0, roomScript, sentence);
			break;

		case 112:
			result = processEntries(&_sentences[15], 0, roomScript, sentence);
			break;

		case 113:
			result = processEntries(&_sentences[9], 0, roomScript, sentence);
			break;

		case 114:
			result = processEntries(&_sentences[18], 0, roomScript, sentence);
			break;

		case 115:
			result = processEntries(&_sentences[12], 0, roomScript, sentence);
			break;

		case 116:
			result = processEntries(&_sentences[8], 0, roomScript, sentence);
			break;

		case 117:
			result = processEntries(&_sentences[6], 0, roomScript, sentence);
			break;

		case 123:
			result = processEntries(&_sentences[17], 0, roomScript, sentence);
			break;

		case 125:
			result = processEntries(&_sentences[14], 0, roomScript, sentence);
			break;

		case 131:
			if (getValue(26) == 0) {
				result = processEntries(&_sentences[getValue(6) ? 5 : 4], 0, roomScript, sentence);
			}
			break;

		default:
			break;
		}

		if (result == 2)
			return 2;
	}

	if (sentence->contains("pretend you summoned yourself") ||
		sentence->contains("pretend you just summoned yourself")) {
		if (scriptChanged(roomScript, 157) == 2)
			return 2;
	}

	if (sentence->localWord("television") || roomScript->_scriptId == 111) {
		if (sentence->localWord("drop") || sentence->localWord("throw")
			|| sentence->localWord("smash") || sentence->localWord("destroy")
			|| sentence->localWord("toss") || sentence->localWord("put")
			|| sentence->localWord("pitch") || sentence->localWord("heft")) {
			// You've instructed the Bellbot to go all Pete Townshend on a TV
			if (getValue(40) == 1) {
				// Won't smash
				addResponse(getDialogueId(201687));
				applyResponse();
				return 2;
			} else if (roomScript->_scriptId == 111) {
				// Within the Parrot Lobby
				addResponse(getDialogueId(200914));
				applyResponse();
				CTrueTalkManager::triggerAction(17, 0);
				CTrueTalkManager::setFlags(40, 1);
				return 2;
			} else {
				// In any other room other than the Parrot Lobby
				addResponse(getDialogueId(200710));
				addResponse(getDialogueId(201334));
				applyResponse();
				return 2;
			}
		}
	}

	if (sentence->contains("what should i do here")
			|| sentence->contains("what do i do here")
			|| sentence->contains("what shall i do in here")
			|| sentence->contains("what shall i do in this room")
			|| sentence->contains("what should i do in this room")
			|| sentence->contains("what am i supposed to do in here")
			|| sentence->contains("what should i do in here")
			|| sentence->contains("what do i do in this room")
			|| sentence->localWord("doidohere")
			|| sentence->contains("was soll ich denn hier tun")
			|| sentence->contains("was soll ich hier tun")
			|| sentence->contains("was gibt es hier zu tun")
			|| sentence->contains("was kann man denn hier machen")
			|| sentence->contains("was kann man denn hier tun")
			|| sentence->contains("was soll ich hier drin tun")
			|| sentence->contains("was soll ich hier")
			|| sentence->contains("wohin soll ich jetzt")
			|| sentence->contains("was ist das hier fuer ein raum")
			|| sentence->contains("was ist denn hier zu tun")
			|| sentence->contains("was kann man hier machen")
			|| sentence->contains("was soll ich jetzt machen")
			|| sentence->contains("was kommt jetzt")
			|| sentence->contains("was kommt nun")
			|| sentence->contains("wozu bin ich eigentlich hier")
			|| sentence->contains("wozu bin ich denn hier")) {
		if (addRoomDescription(roomScript)) {
			applyResponse();
			return 2;
		}
	}

	if (sentence->localWord("help")
			|| (g_language == Common::DE_DEU && sentence->contains("help"))
			|| sentence->contains("what now")
			|| sentence->contains("what next")
			|| sentence->contains("give me a hint")
			|| sentence->contains("i need a hint")
			|| sentence->contains("what should i be doing")
			|| sentence->contains("what do you reckon i should do now")
			|| sentence->contains("what shall i do")
			|| sentence->contains("what would you do")
			|| sentence->contains("what should i do")
			|| sentence->contains("what do i do")
			|| sentence->contains("was nun")
			|| sentence->contains("so und was kommt jetzt")
			|| sentence->contains("und jetzt")
			|| sentence->contains("einen hinweis")
			|| sentence->contains("einen tip")
			|| sentence->contains("ich bin verzweifelt")
			|| sentence->contains("bin ich auf der richtigen spur")
			|| sentence->contains("was soll ich jetzt anfangen")
			|| sentence->contains("wozu raetst du mir")
			|| sentence->contains("was muss ich jetzt")
			|| sentence->contains("was wuerdest du an meiner stelle")
			|| sentence->contains("was soll ich als naechstes tun")
			|| sentence->contains("was soll ich hier")) {
		if (getDialRegion(0) == 1) {
			randomResponse4(roomScript, getValue(1));
			applyResponse();
			return 2;
		} else {
			randomResponse3(roomScript, getValue(1));
		}
	}

	if (sentence->get58() > 6 && sentence->contains("please")) {
		addResponse(getDialogueId(200432));
		applyResponse();
		return 2;
	}

	if (checkCommonSentences(roomScript, sentence) == 2)
		return 2;

	// WORKAROUND: Skip processEntries call on unassigned sentence array

	// Standard sentence list
	if (processEntries(&_entries, _entryCount, roomScript, sentence) == 2)
		return 2;

	if (g_language != Common::DE_DEU) {
		if ((sentence->_category == 4 && sentence->localWord("am") && sentence->localWord("i"))
				|| (sentence->localWord("are") && sentence->localWord("we"))
				|| (sentence->_category == 3 && sentence->localWord("room")
						&& sentence->localWord("we") && sentence->localWord("in"))
				|| (sentence->_category == 3 && sentence->localWord("rom")
						&& sentence->localWord("is") && sentence->localWord("this"))
				) {
			uint id = getRangeValue(getRoomDialogueId(roomScript));
			addResponse(getDialogueId(id ? id : 201384));
			applyResponse();
			return 2;
		}

		if (getValue(1) >= 3) {
			result = processEntries(&_sentences[1], 0, roomScript, sentence);
		} else if (getValue(1) == 2) {
			result = processEntries(&_sentences[2], 0, roomScript, sentence);
		} else if (getValue(1) == 1) {
			result = processEntries(&_sentences[3], 0, roomScript, sentence);

			if (sentence->contains("shrinkbot")) {
				addResponse(getDialogueId(200583));
				applyResponse();
				return 2;
			}
		}
		if (result == 2)
			return 2;
	} else {
		if (getValue(1) == 1 && (sentence->localWord("shrinkbot") ||
				sentence->contains("psychobot"))) {
			addResponse(getDialogueId(200583));
			applyResponse();
			return 2;
		}
	}

	if ((g_language != Common::DE_DEU || getValue(40) == 0) &&
			(sentence->localWord("television") || sentence->localWord("tv")
			|| sentence->localWord("crush") || sentence->localWord("crushed")
			|| sentence->localWord("crushedtv"))) {
		if (roomScript->_scriptId == 111 || getRandomBit()) {
			addResponse(getDialogueId(getRandomBit() ? 200912 : 200913));
		} else {
			addResponse(getDialogueId(200710));
			addResponse(getDialogueId(201334));
		}

		applyResponse();
		return 2;
	}

	if (checkCommonWords(roomScript, sentence)) {
		applyResponse();
		setState(0);
		return 2;
	}

	if ((sentence->contains("my") || sentence->contains("mein"))
		&& (sentence->contains("where can i find")
			|| sentence->contains("where is")
			|| sentence->contains("wheres")
			|| sentence->contains("help me find")
			|| sentence->contains("what have you done with")
			|| sentence->contains("have you got")
			|| sentence->contains("id like")
			|| sentence->contains("i would like")
			|| sentence->contains("have you seen")
			|| sentence->contains("gibt es hier")
			|| sentence->contains("wo finde ich")
			|| sentence->contains("sind hier")
			|| sentence->contains("habt ihr")
			|| sentence->contains("gibt es")
			|| sentence->contains("wo sind")
			|| sentence->contains("wo ist")
			|| sentence->contains("wie komme ich")
			|| sentence->contains("wie erreicht man")
			|| sentence->contains("hast du")
			|| sentence->contains("ich moechte")
			|| sentence->contains("gib mir")
			|| sentence->contains("haettest du vielleicht")
			|| sentence->contains("ich haette gern")
			)) {
		addResponse(getDialogueId(200799));
		applyResponse();
		return 2;
	}

	uint tagId = g_vm->_trueTalkManager->_quotes.find(sentence->_normalizedLine);
	if (tagId && chooseResponse(roomScript, sentence, tagId) == 2)
		return 2;
	if (defaultProcess(roomScript, sentence))
		return 2;
	if (!processEntries(&_sentences[19], 0, roomScript, sentence))
		return 2;
	if (!processEntries(_defaultEntries, 0, roomScript, sentence))
		return 2;

	if (sentence->contains("42")) {
		addResponse(getDialogueId(200515));
		applyResponse();
		return 2;
	}

	CTrueTalkManager::setFlags(24, val24 + 1);
	if (getValue(24) > 3) {
		addResponse(getDialogueId(200200));
		applyResponse();
		return 2;
	}

	if (sentence->localWord("get")) {
		addResponse(getDialogueId(200475));
		applyResponse();
		return 2;
	}

	if (getRandomNumber(100) <= 75) {
		addResponse(getDialogueId(200060));
		applyResponse();
		return 2;
	}

	addResponse(getDialogueId(200140));
	addResponse(getDialogueId(getRandomBit() ? 200192 : 200157));
	addResponse(getDialogueId(200176));
	applyResponse();
	return 2;
}

ScriptChangedResult BellbotScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	if (!roomScript)
		return SCR_2;

	switch (id) {
	case 104:
		addResponse(getDialogueId(200617));
		applyResponse();
		break;

	case 105:
		addResponse(getDialogueId(200732));
		applyResponse();
		break;

	case 106:
		addResponse(getDialogueId(200733));
		applyResponse();
		break;

	case 107:
		addResponse(getDialogueId(200731));
		applyResponse();
		break;

	case 157:
		_responseFlag = true;
		break;

	case 158:
		CTrueTalkManager::setFlags(26, 1);
		break;

	case 3:
		if (_responseFlag) {
			if (randomResponse0(roomScript, id))
				return SCR_2;
		} else {
			addResponse(getDialogueId(201693));
			applyResponse();
		}

		_responseFlag = false;
		CTrueTalkManager::_v9 = 0;
		// Intentional fall-through
	default:
		if (roomScript->_scriptId == 115 && id == 103) {
			switch (getValue(4)) {
			case 0:
				addResponse(getDialogueId(200014));
				applyResponse();
				break;
			case 1:
			case 2:
				addResponse(getDialogueId(200011));
				applyResponse();
				break;
			case 3:
				addResponse(getDialogueId(200007));
				applyResponse();
				break;
			default:
				break;
			}
		}
		break;
	}

	return SCR_2;
}

int BellbotScript::handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
		uint tag1, uint tag2, uint remainder) {
	switch (tag2) {
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
	case MKTAG('W', 'W', 'E', 'B'):
		tag2 = MKTAG('E', 'N', 'T', 'N');
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
	case MKTAG('C', 'O', 'P', 'S'):
	case MKTAG('D', 'C', 'T', 'R'):
	case MKTAG('F', 'A', 'S', 'H'):
	case MKTAG('F', 'A', 'M', 'E'):
	case MKTAG('H', 'E', 'R', 'D'):
	case MKTAG('H', 'O', 'S', 'T'):
	case MKTAG('K', 'N', 'O', 'B'):
	case MKTAG('N', 'H', 'R', 'O'):
	case MKTAG('R', 'A', 'C', 'E'):
	case MKTAG('S', 'C', 'I', 'T'):
	case MKTAG('T', 'O', 'U', 'P'):
	case MKTAG('T', 'W', 'A', 'T'):
	case MKTAG('W', 'E', 'A', 'T'):
		tag2 = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 't'):
	case MKTAG('F', 'I', 'L', 'M'):
	case MKTAG('J', 'F', 'O', 'D'):
	case MKTAG('L', 'I', 'Q', 'D'):
		tag2 = MKTAG('F', 'O', 'O', 'D');
		break;
	case MKTAG('C', 'R', 'I', 'M'):
	case MKTAG('C', 'S', 'P', 'Y'):
	case MKTAG('D', 'R', 'U', 'G'):
		tag2 = MKTAG('V', 'B', 'A', 'D');
		break;
	case MKTAG('E', 'A', 'R', 'T'):
	case MKTAG('H', 'O', 'M', 'E'):
	case MKTAG('N', 'P', 'L', 'C'):
	case MKTAG('P', 'L', 'A', 'N'):
		tag2 = MKTAG('P', 'L', 'A', 'C');
		break;
	case MKTAG('F', 'A', 'U', 'N'):
	case MKTAG('F', 'I', 'S', 'H'):
	case MKTAG('F', 'L', 'O', 'R'):
		tag2 = MKTAG('N', 'A', 'T', 'R');
		break;
	case MKTAG('H', 'H', 'L', 'D'):
	case MKTAG('T', 'O', 'Y', 'S'):
	case MKTAG('W', 'E', 'A', 'P'):
		tag2 = MKTAG('M', 'A', 'C', 'H');
		break;
	case MKTAG('M', 'L', 'T', 'Y'):
	case MKTAG('P', 'G', 'R', 'P'):
	case MKTAG('P', 'T', 'I', 'C'):
		tag2 = MKTAG('G', 'R', 'U', 'P');
		break;
	case MKTAG('P', 'K', 'U', 'P'):
	case MKTAG('S', 'E', 'X', '1'):
	case MKTAG('S', 'W', 'E', 'R'):
		tag2 = MKTAG('R', 'U', 'D', 'E');
		break;
	case MKTAG('P', 'H', 'I', 'L'):
	case MKTAG('R', 'C', 'K', 'T'):
		tag2 = MKTAG('S', 'C', 'I', 'E');
		break;
	case MKTAG('T', 'R', 'A', '2'):
	case MKTAG('T', 'R', 'A', '3'):
		tag2 = MKTAG('T', 'R', 'A', 'V');
		break;
	default:
		break;
	}

	return TTnpcScript::handleQuote(roomScript, sentence, tag1, tag2, remainder);
}

int BellbotScript::updateState(uint oldId, uint newId, int index) {
	if (!getValue(25)) {
		newId = 202043 - (getValue(1) <= 2 ? 994 : 0);
		CTrueTalkManager::setFlags(25, 1);
	}

	if (oldId == _oldId && _rangeResetCtr >= 3) {
		TTscriptRange *range = findRange(oldId);
		if (range)
			range->_priorIndex = 0;

		_rangeResetCtr = 0;
		return getRangeValue(200370);
	}

	if (oldId != _oldId) {
		_oldId = oldId;
		_rangeResetCtr = 0;
	}

	if (oldId >= 201709 && oldId <= 201754) {
		addResponse(getDialogueId(201705));
		addResponse(getDialogueId(201706));
		newId = getRangeValue(201707);
	}

	if (newId == 202276)
		newId = addLocation();
	if (newId == 202275)
		newId = getStateDialogueId();

	if (getValue(1) >= 2) {
		if (newId == 200840 || newId == 200845 || newId == 200846 || newId == 200851) {
			if (getValue(1) == 2) {
				newId = 202047;
			} else {
				newId = getRangeValue(202848);
			}
		}
	}

	if (getValue(1) >= 3) {
		if (newId == 200841 || newId == 200842 || newId == 200843 ||
				newId == 200847 || newId == 200848 || newId == 200854) {
			newId = getRangeValue(202038);
		}
	}

	if (newId == 200264 && getValue(1) == 1)
		newId = 200267;
	if (newId == 202231 && getValue(1) == 1)
		newId = 200848;

	int v4 = getValue(4);
	if (newId == 200187 && v4) {
		return 200188;
	} else if (newId == 200188 && !v4) {
		return 200187;
	} else if (newId == 200014 && (v4 == 1 || v4 == 2)) {
		return 200011;
	} else if (newId == 200011 && !v4) {
		return 200014;
	}

	if (oldId == 200612) {
		CTrueTalkManager::setFlags(25, 2);
		CTrueTalkManager::setFlags(5, 1);
	}

	if (newId == 200423 || newId == 200424 || newId == 200425) {
		if (getValue(5)) {
			CTrueTalkManager::triggerAction(16, 0);
		} else {
			newId = 200611;
		}
	}

	if (oldId == 200261 && getRandomNumber(10) == 1) {
		if (getValue(1) >= 3)
			newId = getRangeValue(200283);
		else if (getValue(1) == 2)
			newId = getRangeValue(200279);
	}

	if (oldId == 200962) {
		if (getValue(1) == 2)
			return 200963;
		if (getValue(1) == 1)
			return 200964;
	}
	if (oldId == 200989 && getValue(1) <= 2)
		return 200990;

	if (oldId == 201760) {
		CGameManager *gameManager = g_vm->_trueTalkManager->getGameManager();
		CPetControl *pet = getPetControl(gameManager);

		if (pet) {
			bool canSummon = pet->canSummonBot("DoorBot");
			if (canSummon) {
				CTrueTalkManager::_v9 = 101;
				CTrueTalkManager::triggerAction(5, 0);
			} else {
				newId = 201857;
			}
		}
	}

	setValue23(newId);
	return newId;
}

int BellbotScript::preResponse(uint id) {
	if (g_language == Common::DE_DEU && getDialRegion(0) == 0
			&& getRandomNumber(100) > 80)
		return 251250;

	int newId = _preResponses.find(id);

	if (g_language != Common::DE_DEU) {
		if (newId == 202277) {
			applyResponse();
			CTrueTalkManager::triggerAction(1, 0);
		}
		if (newId == 200769) {
			applyResponse();
			CTrueTalkManager::triggerAction(18, 0);
		}

		if (id == 21790)
			CTrueTalkManager::triggerAction(13, 0);
	}

	return newId;
}

int BellbotScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	if (g_language == Common::EN_ANY) {
		switch (val1) {
		case 1:
			addResponse(getDialogueId(*srcIdP));
			applyResponse();
			return 2;

		case 2:
			addResponse(getDialogueId(*srcIdP));
			addResponse(getDialogueId(getRandomNumber(2) == 1 ? 200192 : 200157));
			addResponse(getDialogueId(200176));
			applyResponse();
			return 2;

		case 21:
			if (CTrueTalkManager::getStateValue(7) == 0) {
				selectResponse(21372);
				applyResponse();
				return 2;
			}

			if (!sentence->localWord("broken") && !sentence->contains("broken") &&
				CTrueTalkManager::_currentNPC) {
				CNodeItem *node = CTrueTalkManager::_currentNPC->getNode();
				if (node) {
					CString nodeName = node->getName();
					if (nodeName.containsIgnoreCase("5") || nodeName.containsIgnoreCase("6")
						|| nodeName.containsIgnoreCase("7")) {
						CTrueTalkManager::triggerAction(29, 2);
						selectResponse(201571);
						applyResponse();
						return 2;
					}
				}
			}

			CTrueTalkManager::triggerAction(29, 1);
			selectResponse(201771);
			applyResponse();
			return 2;

		case 22:
			if (CTrueTalkManager::getStateValue(7) == 0) {
				selectResponse(21372);
				applyResponse();
				return 2;
			}

			if (!sentence->localWord("broken") && !sentence->contains("broken") &&
				CTrueTalkManager::_currentNPC) {
				CNodeItem *node = CTrueTalkManager::_currentNPC->getNode();
				if (node) {
					CString nodeName = node->getName();
					if (nodeName.containsIgnoreCase("5") || nodeName.containsIgnoreCase("6")
						|| nodeName.containsIgnoreCase("7")) {
						CTrueTalkManager::triggerAction(29, 2);
						selectResponse(201571);
						applyResponse();
						return 2;
					}
				}
			}

			CTrueTalkManager::triggerAction(29, 1);
			selectResponse(201771);
			applyResponse();
			return 2;

		case 23:
		case 24:
			if (CTrueTalkManager::getStateValue(7) == 0) {
				selectResponse(21372);
				applyResponse();
				return 2;
			}

			CTrueTalkManager::triggerAction(29, val1 == 23 ? 3 : 4);
			break;

		default:
			break;
		}
	} else {
		switch (val1) {
		case 5001:
		case 5021:
			return 0;

		case 5002:
			addResponse(getDialogueId(*srcIdP));
			addResponse(getDialogueId(200000));
			applyResponse();
			return 2;

		case 5003:
			addResponse(getDialogueId(*srcIdP));
			return 2;

		case 5022:
		case 5023:
			if (CTrueTalkManager::getStateValue(7)) {
				bool flag = true;

				if (!sentence->localWord("broken") && !sentence->contains("kaputt") &&
					!sentence->contains("im eimer") && !sentence->contains("funktioniert nicht") &&
					CTrueTalkManager::_currentNPC) {
					CNodeItem *node = CTrueTalkManager::_currentNPC->getNode();
					if (node) {
						CString nodeName = node->getName();
						if (nodeName.contains("5") || nodeName.contains("6") || nodeName.contains("7"))
							flag = false;
					}
				}

				if (flag) {
					CTrueTalkManager::triggerAction(29, 1);
					selectResponse(201771);
				} else {
					CTrueTalkManager::triggerAction(29, 2);
					selectResponse(201554);
				}
			} else {
				selectResponse(21378);
			}

			applyResponse();
			return 2;

		case 5024:
			if (CTrueTalkManager::getStateValue(7)) {
				CTrueTalkManager::triggerAction(29, 3);
				return 0;
			}

			selectResponse(21378);
			applyResponse();
			return 2;

		case 5025:
			if (CTrueTalkManager::getStateValue(7)) {
				CTrueTalkManager::triggerAction(29, 4);
				return 0;
			}

			selectResponse(21378);
			applyResponse();
			return 2;

		default:
			return TTnpcScript::doSentenceEntry(val1, srcIdP, roomScript, sentence);
		}
	}

	return 0;
}

bool BellbotScript::randomResponse(uint index) {
	if (getRandomNumber(100) > 10 || getRandomNumber(10) <= index)
		return 0;

	if (getRandomNumber(100) > 95) {
		deleteResponses();
		addResponse(getDialogueId(201695));
		applyResponse();
	} else {
		setResponseFromArray(index, 201696);
	}

	return true;
}

int BellbotScript::addLocation() {
	addResponse(getDialogueId(202228));
	int roomNum, floorNum, elevatorNum;
	getAssignedRoom(&roomNum, &floorNum, &elevatorNum);

	addResponse(getDialogueId(202071 + roomNum));
	addResponse(getDialogueId(201933 + floorNum));
	addResponse(getDialogueId(201916 + elevatorNum));

	return 200858;
}

int BellbotScript::getStateDialogueId() const {
	switch (getValue(1)) {
	case 1:
		return 201253;
	case 2:
		return 200282;
	default:
		return 201246;
	}
}

void BellbotScript::setValue23(uint id) {
	uint val = 0;
	for (uint idx = 0; idx < _states.size() && !val; ++idx) {
		TTmapEntry &us = _states[idx];
		if (us._src == id)
			val = us._dest;
	}

	CTrueTalkManager::setFlags(23, val);
}

int BellbotScript::preprocess(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (!roomScript || !sentence)
		return true;

	bool applyFlag = false, stateFlag = true;
	switch (getValue(23)) {
	case 1:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200818));
			applyFlag = true;
		}
		if (sentence->_category == 12) {
			addResponse(getDialogueId(200817));
			applyFlag = true;
		}
		break;

	case 2:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200835));
			addResponse(getDialogueId(200830));
			applyFlag = true;
		} else if (sentence->_category == 12) {
			addResponse(getDialogueId(200834));
			addResponse(getDialogueId(200830));
			applyFlag = true;
		}
		break;

	case 3:
		if (sentence->_category >= 11 && sentence->_category <= 13) {
			addResponse(getDialogueId(200831));
			addResponse(getDialogueId(200833));
			applyFlag = true;
		}
		break;

	case 4:
		if (sentence->_category == 11) {
			addResponse(getDialogueId(200872));
			applyFlag = true;
		}
		if (sentence->_category == 12 || sentence->_category == 13) {
			addResponse(getDialogueId(200873));
			applyFlag = true;
		}
		break;

	case 5:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200492));
			applyFlag = true;
		}
		if (sentence->_category == 12) {
			addResponse(getDialogueId(200491));
			applyFlag = true;
		}
		break;

	case 6:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200496));
			applyResponse();
			setState(0);
			CTrueTalkManager::setFlags(23, 7);
			return 2;
		}
		if (sentence->_category == 12) {
			addResponse(getDialogueId(200127));
			applyFlag = true;
		}
		break;

	case 7:
		addResponse(getDialogueId(200504));
		addResponse(getDialogueId(200496));
		applyFlag = true;
		stateFlag = false;
		break;

	case 8:
		addResponse(getDialogueId(200494));
		applyFlag = true;
		stateFlag = false;
		break;

	case 9:
		addResponse(getDialogueId(sentence->localWord("guess") ? 200495 : 200493));
		applyFlag = true;
		break;

	case 10:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200317));
			applyResponse();
			setState(0);
			CTrueTalkManager::setFlags(23, 11);
			return 2;
		}

		addResponse(getDialogueId(sentence->_category == 12 ? 200316 : 200315));
		applyFlag = true;
		break;

	case 11:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200055));
		} else if (sentence->_category == 12) {
			addResponse(getDialogueId(200318));
		} else {
			addResponse(getDialogueId(200315));
		}

		applyFlag = true;
		break;

	case 12:
		if (sentence->_category == 6) {
			addResponse(getDialogueId(200259));
			applyFlag = true;
		}
		break;

	case 13:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200207));
			applyFlag = true;
		} else if (sentence->_category == 12) {
			addResponse(getDialogueId(200206));
			applyFlag = true;
		}
		break;

	case 14:
		if (sentence->_category == 6) {
			addResponse(getDialogueId(200349));
			applyFlag = true;
		}
		break;

	case 15:
		if (sentence->_category == 6) {
			addResponse(getDialogueId(200130));
			applyResponse();
			setState(0);
			CTrueTalkManager::setFlags(23, 16);
			return 2;
		}
		break;

	case 16:
		if (sentence->localWord("invented")) {
			addResponse(getDialogueId(200131));
			applyFlag = true;
		}
		break;

	case 17:
		if ((sentence->_category == 3 && sentence->localWord("code"))
				|| (sentence->localWord("which") && sentence->localWord("is"))
				|| sentence->localWord("remember")
				|| sentence->localWord("know")
		) {
			addResponse(getDialogueId(200044));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 19:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200223));
			applyFlag = true;
		}
		break;

	case 20:
		addResponse(getDialogueId(200254));
		applyFlag = true;
		break;

	case 21:
		if (sentence->localWord("hiker") && (sentence->contains("hug") ||
				sentence->contains("anhalter"))) {
			addResponse(getDialogueId(200379));
			applyFlag = true;
		}
		break;

	case 22:
		if (sentence->localWord("get") && sentence->localWord("it")) {
			addResponse(getDialogueId(200474));
			applyFlag = true;
		}
		break;

	case 23:
		addResponse(getDialogueId(sentence->localWord("long") ? 200870 : 200871));
		applyFlag = true;
		break;

	case 24:
		addResponse(getDialogueId(200793));
		applyFlag = true;
		stateFlag = false;
		break;

	case 25:
		if (sentence->localWord("parrot")) {
			addResponse(getDialogueId(200255));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 26:
		if (sentence->localWord("cage")) {
			addResponse(getDialogueId(200380));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 27:
		addResponse(getDialogueId(200347));
		applyFlag = true;
		stateFlag = false;
		break;

	case 28:
		if (sentence->localWord("perch")) {
			addResponse(getDialogueId(200242));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 29:
		if (sentence->localWord("brain") || sentence->localWord("titania")) {
			addResponse(getDialogueId(200392));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 30:
		if ((sentence->localWord("did") && sentence->localWord("not"))
				|| (sentence->localWord("would") && sentence->localWord("not"))
				|| (sentence->localWord("could") && sentence->localWord("not"))
				|| sentence->localWord("tried")) {
			addResponse(getDialogueId(200416));
			applyFlag = true;
		}
		break;

	case 31:
		addResponse(getDialogueId(sentence->_category == 11 ? 200810 : 200811));
		applyFlag = true;
		break;

	case 32:
		addResponse(getDialogueId(sentence->_category == 11 ? 200810 : 200812));
		applyFlag = true;
		break;

	case 33:
		addResponse(getDialogueId(200822));
		applyFlag = true;
		break;

	case 34:
		addResponse(getDialogueId(200824));
		applyFlag = true;
		break;

	case 35:
		if (sentence->_category == 3 && sentence->localWord("it")
				&& (sentence->localWord("for") || sentence->localWord("do"))) {
			addResponse(getDialogueId(200768));
			applyFlag = true;
		}
		break;

	case 36:
		if (sentence->_category == 11) {
			CTrueTalkManager::triggerAction(14, 0);
			addResponse(getDialogueId(200761));
			applyFlag = true;
		}
		break;

	case 37:
		addResponse(getDialogueId(200630));
		applyFlag = true;
		break;

	case 38:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(200631));
			applyFlag = true;
		}
		break;

	case 39:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200632));
			stateFlag = false;
		} else {
			addResponse(getDialogueId(200633));
		}
		applyFlag = true;
		break;

	case 40:
		addResponse(getDialogueId(200633));
		applyFlag = true;
		break;

	case 41:
		addResponse(getDialogueId(sentence->contains("42") ? 200139 : 200627));
		applyFlag = true;
		break;

	case 42:
		if ((sentence->localWord("carry") && sentence->localWord("on"))
				|| (sentence->localWord("go") && sentence->localWord("on"))
				|| sentence->localWord("more")
				|| sentence->localWord("going")
				|| sentence->localWord("elaborate")
				|| sentence->localWord("suspicious")
				|| sentence->localWord("they")) {
			addResponse(getDialogueId(200642));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 43:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200643));
			applyFlag = true;
		}
		break;

	case 44:
case44:
		if (better(sentence, 200615, 200613)) {
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 45:
		if (sentence->contains("surprise")) {
			addResponse(getDialogueId(200614));
			applyFlag = true;
			stateFlag = false;
			break;
		}
		goto case44;

	case 46:
		if (sentence->contains("good")) {
			addResponse(getDialogueId(200616));
			applyFlag = true;
			stateFlag = false;
			break;
		}
		goto case44;

	case 47:
		if (sentence->_category == 12)
			addResponse(getDialogueId(200368));
		addResponse(getDialogueId(200366));
		applyFlag = true;
		stateFlag = false;
		break;

	case 48:
		if ((sentence->localWord("carry") && sentence->localWord("on"))
				|| sentence->localWord("more")
				|| (sentence->localWord("go") && sentence->localWord("on"))
				|| sentence->localWord("going")
				|| sentence->localWord("yes")
				|| sentence->localWord("really")) {
			addResponse(getDialogueId(200367));
			applyFlag = true;
		}
		break;

	case 49:
		if (sentence->_category >= 11 && sentence->_category <= 13) {
			addResponse(getDialogueId(200407));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 50:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200408));
			stateFlag = false;
		} else {
			addResponse(getDialogueId(200409));
		}
		applyFlag = true;
		break;

	case 51:
		if ((sentence->localWord("no") && sentence->localWord("it")
				&& sentence->localWord("is") && sentence->localWord("not"))
				|| sentence->contains("yeah right")) {
			addResponse(getDialogueId(200636));
			applyFlag = true;
		}
		break;

	case 52:
		if (sentence->_category >= 11 && sentence->_category <= 13) {
			addResponse(getDialogueId(200872));
			applyFlag = true;
		}
		break;

	case 53:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(200525));
			applyFlag = true;
		} else if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200526));
			applyFlag = true;
		}
		break;

	case 54:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(200095));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 55:
		if (sentence->_category == 6) {
			addResponse(getDialogueId(200112));
			applyFlag = true;
		}
		break;

	case 56:
		if (sentence->localWord("sure")
			|| (sentence->localWord("nothing") && sentence->localWord("else"))) {
			addResponse(getDialogueId(200649));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 57:
		if (sentence->localWord("bad")
				|| (sentence->localWord("not") && sentence->localWord("good"))) {
			addResponse(getDialogueId(200654));
		} else {
			addResponse(getDialogueId(200655));
			stateFlag = false;
		}
		applyFlag = true;
		break;

	case 58:
		if (sentence->localWord("more")
				|| (sentence->localWord("go") && sentence->localWord("on"))
				|| (sentence->_category == 11 && sentence->localWord("really"))) {
			addResponse(getDialogueId(200650));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 59:
		if (!sentence->localWord("shutup")) {
			addResponse(getDialogueId(200651));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 60:
		if (sentence->_category == 3 && sentence->localWord("they") && sentence->localWord("do")) {
			addResponse(getDialogueId(200652));
			applyFlag = true;
stateFlag = false;
		}
		break;

	case 61:
		if ((sentence->localWord("that") && sentence->localWord("all"))
			|| (sentence->localWord("anything") && sentence->localWord("else"))) {
			addResponse(getDialogueId(200653));
			applyFlag = true;
		}
		break;

	case 62:
		if (sentence->localWord("meant") || sentence->localWord("woman")) {
			addResponse(getDialogueId(200743));
			applyFlag = true;
		}
		break;

	case 63:
		addResponse(getDialogueId(200208));
		applyFlag = true;
		break;

	case 64:
		if (sentence->localWord("rowbot")) {
			addResponse(getDialogueId(200052));
			applyFlag = true;
		}
		break;

	case 65:
		if (sentence->localWord("sorry")) {
			addResponse(getDialogueId(200056));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 66:
		if (sentence->localWord("sorry")) {
			addResponse(getDialogueId(200057));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 67:
		if (sentence->localWord("sorry")) {
			addResponse(getDialogueId(200055));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 68:
		if ((sentence->localWord("i") && (sentence->localWord("care") ||
				sentence->localWord("do")))
				|| sentence->localWord("me")) {
			addResponse(getDialogueId(201006));
			applyFlag = true;
		}
		break;

	case 69:
		if ((sentence->localWord("what") && sentence->localWord("happen"))
			|| sentence->localWord("filigon")) {
			addResponse(getDialogueId(201011));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 70:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(201012));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 71:
		if (sentence->localWord("why")) {
			addResponse(getDialogueId(201013));
			applyFlag = true;
		}
		break;

	case 72:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(200921));
			applyFlag = true;
		} else if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(200920));
			applyFlag = true;
		}
		break;

	case 73:
		if (sentence->localWord("mood") || sentence->contains("stimmung") ||
				sentence->contains("laune") || sentence->contains("verfassung")) {
			if (charId() == 7 || charId() == 5) {
				addResponse(getDialogueId(201021));
				applyFlag = true;
				stateFlag = false;
			}
		}
		break;

	case 74:
		if (sentence->_category == 6) {
			addResponse(getDialogueId(201022));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 75:
		if (sentence->_category == 3) {
			if (sentence->localWord("that") || sentence->localWord("worb")) {
				addResponse(getDialogueId(201802));
				applyFlag = true;
			}
		}
		break;

	case 76:
		if (sentence->_category == 2 && (sentence->localWord("that") || sentence->localWord("gat"))) {
			addResponse(getDialogueId(201034));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 77:
		if (sentence->_category == 4 || sentence->_category == 3) {
			if (sentence->localWord("that") || sentence->localWord("blerontis")) {
				addResponse(getDialogueId(201035));
				applyFlag = true;
			}
		}
		break;

	case 78:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(201034));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->_category == 11) {
			addResponse(getDialogueId(201040));
			applyFlag = true;
		} else if ((sentence->localWord("not") && sentence->localWord("remember"))
				|| sentence->localWord("forgot")) {
			addResponse(getDialogueId(201041));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why")) {
			addResponse(getDialogueId(201042));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 79:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(201052));
			CTrueTalkManager::triggerAction(14, 0);
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->_category == 12) {
			addResponse(getDialogueId(202119));
			addResponse(getDialogueId(200256));
			applyFlag = true;
		}
		break;

	case 80:
		if ((!sentence->localWord("what") && sentence->localWord("how"))
				|| sentence->localWord("about")
				|| sentence->localWord("you")) {
			if (sentence->_category != 3 && sentence->_category != 4 && sentence->_category != 7) {
				addResponse(getDialogueId(201694));
				applyFlag = true;
				stateFlag = false;
			}
		} else {
			addResponse(getDialogueId(201135));
			applyFlag = true;
		}
		break;

	case 81:
		if ((!sentence->localWord("what") && !sentence->localWord("how"))
				|| !sentence->localWord("about")
				|| !sentence->localWord("you")) {
			if (!sentence->localWord("and") || !sentence->localWord("yourself"))
				break;
		}
		addResponse(getDialogueId(201135));
		applyFlag = true;
		break;

	case 82:
		if ((sentence->_category == 3 && sentence->localWord("mean"))
				|| sentence->localWord("surf")
				|| (sentence->localWord("what") && sentence->localWord("talk")
					&& sentence->localWord("about"))) {
			addResponse(getDialogueId(201694));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 83:
		if (sentence->_category != 3 && sentence->_category != 4 && sentence->_category != 7) {
			addResponse(getDialogueId(201083));
			applyFlag = true;
		}
		break;

	case 84:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(202119));

			switch (getValue(1)) {
			case 1:
				addResponse(getDialogueId(202024));
				applyFlag = true;
				break;
			case 2:
				addResponse(getDialogueId(201812));
				applyFlag = true;
				stateFlag = false;
				break;
			default:
				break;
			}
		} else if (sentence->_category == 11) {
			addResponse(getDialogueId(201060));
			addResponse(getDialogueId(201079));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 85:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(201814));
			applyFlag = true;
		}
		if (sentence->_category == 12) {
			addResponse(getDialogueId(201813));
			applyFlag = true;
		}
		break;

	case 86:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(202109));
			applyFlag = true;
		}
		if (sentence->_category == 12) {
			addResponse(getDialogueId(202108));
			applyFlag = true;
		}
		break;

	case 87:
		if (better(sentence, 201993, 200720)) {
			applyFlag = true;
		}
		break;

	case 88:
		if (sentence->_category == 6 || 
				(g_language != Common::DE_DEU && sentence->contains("upside down"))) {
			addResponse(getDialogueId(202142));
			applyFlag = true;
		}
		break;

	case 89:
		if (sentence->_category == 2) {
			addResponse(getDialogueId(200739));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 90:
		if (sentence->contains("like") && (sentence->contains("slug") || sentence->contains("snail"))) {
			addResponse(getDialogueId(201029));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->contains("slime") || sentence->localWord("what")) {
			addResponse(getDialogueId(201220));
			applyFlag = true;
			stateFlag = false;
		}

	default:
		break;
	}

	if (applyFlag)
		applyResponse();
	if (stateFlag) {
		setState(0);
		CTrueTalkManager::setFlags(23, 0);
	}

	return applyFlag ? 2 : 1;
}

bool BellbotScript::better(const TTsentence *sentence, uint id1, uint id2) {
	if (sentence->contains("good") || sentence->localWord("better")) {
		addResponse(getDialogueId(id1));
	} else if (sentence->localWord("bad")) {
		addResponse(getDialogueId(id2));
	} else {
		return false;
	}

	return true;
}

bool BellbotScript::randomResponse0(const TTroomScript *roomScript, uint id) {
	bool dr0 = getDialRegion(0) == 1;
	uint newId = getValue(1);

	if (getValue(25) == 0) {
		CTrueTalkManager::setFlags(25, 1);
		if (getValue(1) > 2) {
			addResponse(getDialogueId(202043));
			applyResponse();
			return true;
		}
	}

	bool result = dr0 ? randomResponse1(roomScript, newId) :
		randomResponse2(roomScript, newId);
	if (result)
		CTrueTalkManager::triggerAction(1, 0);

	return true;
}

bool BellbotScript::randomResponse1(const TTroomScript *roomScript, uint id) {
	if (getRandomNumber(100) < 10) {
		addResponse(getDialogueId(201978));
		applyResponse();
	} else {
		if (getRandomNumber(100) < 50)
			addResponse(getDialogueId(202259));

		randomResponse3(roomScript, id);
		applyResponse();
	}

	return false;
}

bool BellbotScript::randomResponse2(const TTroomScript *roomScript, uint id) {
	if (getRandomNumber(100) < 5) {
		addResponse(getDialogueId(202262));
		applyResponse();
	} else {
		if (getRandomNumber(100) < 40)
			addResponse(getDialogueId(202258));

		randomResponse4(roomScript, id);
		applyResponse();
	}

	return false;
}

void BellbotScript::randomResponse3(const TTroomScript *roomScript, uint id) {
	bool result = false;
	if (roomScript && getRandomNumber(100) < 50)
		result = addRoomDescription(roomScript);

	if (result)
		return;
	if (getRandomNumber(100) >= 50) {
		addResponse(getDialogueId(202262));
		return;
	}

	if (id <= 2) {
		if (getRandomNumber(100) < 50) {
			addResponse(getDialogueId(202266));
			return;
		} else if (id == 2) {
			addResponse(getDialogueId(202264));
			return;
		}
	}

	addResponse(getDialogueId(id == 1 ? 202265 : 202263));
}

void BellbotScript::randomResponse4(const TTroomScript *roomScript, uint id) {
	if (getRandomNumber(100) < 4 && id <= 2) {
		addResponse(getDialogueId(202268));
	} else {
		addResponse(getDialogueId(202267));
	}
}

int BellbotScript::checkCommonSentences(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (!roomScript || !sentence)
		return 1;

	uint val1 = getValue(1);
	for (uint idx = 0; idx < _phrases.size(); ++idx) {
		TTcommonPhrase &cp = _phrases[idx];

		if (cp._roomNum != 0 && cp._roomNum != roomScript->_scriptId)
			continue;
		if (cp._val1 != 0 && cp._val1 != val1 && (cp._val1 == 3 || val1 != 4))
			continue;
		if (!sentence->contains(cp._str.c_str()))
			continue;

		addResponse(getDialogueId(cp._dialogueId));
		applyResponse();
		return 2;
	}

	return 0;
}

bool BellbotScript::checkCommonWords(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (!roomScript || !sentence)
		return 0;
	CTrueTalkManager::setFlags(23, 0);
	if (sentence->_category != 4)
		return 0;

	if (sentence->localWord("garage")) {
		addResponse(getDialogueId(200874));
	} else if (sentence->localWord("parrotfoodshop")) {
		addResponse(getDialogueId(200821));
	} else if (sentence->localWord("sgt") && sentence->localWord("restaurant")) {
		addResponse(getDialogueId(200857));
	} else if (sentence->localWord("firstclass") && sentence->localWord("restaurant")) {
		addResponse(getDialogueId(200839));
	} else if (sentence->localWord("restaurant")) {
		addResponse(getDialogueId(getValue(1) == 1 ? 200839 : 200857));
	} else if (getValue(1) == 1 && sentence->localWord("canal") && sentence->localWord("firstclass")) {
		addResponse(getDialogueId(200846));
	} else if (getValue(1) == 2 && sentence->localWord("canal") && sentence->localWord("secondclass")) {
		addResponse(getDialogueId(200847));
	} else if (sentence->localWord("canal")) {
		addResponse(getDialogueId(getValue(1) == 1 ? 200846 : 200847));
	} else if (sentence->localWord("firstclass") &&
			(sentence->localWord("stateroom") || sentence->localWord("room"))) {
		addResponse(getDialogueId(getValue(1) == 1 ? 200840 : 200306));
	} else if (sentence->localWord("secondclass") && sentence->localWord("stateroom") && sentence->localWord("room")) {
		addResponse(getDialogueId(getValue(1) < 3 ? 202231 : 200306));
	} else if (sentence->localWord("stateroom") || sentence->contains("my room")) {
		addResponse(getDialogueId(202231));
	} else if (sentence->localWord("firstclass")) {
		addResponse(getDialogueId(200840));
	} else if (sentence->localWord("secondclass")) {
		addResponse(getDialogueId(200841));
	} else if (sentence->localWord("thirdclass")) {
		addResponse(getDialogueId(202231));
	} else if (sentence->localWord("arboretum")) {
		addResponse(getDialogueId(200842));
	} else if (sentence->localWord("bar")) {
		addResponse(getDialogueId(200843));
	} else if (sentence->localWord("bottomofwell")) {
		addResponse(getDialogueId(200860));
	} else if (sentence->localWord("topwell") || sentence->localWord("well")) {
		addResponse(getDialogueId(200861));
	} else if (sentence->localWord("bridge")) {
		addResponse(getDialogueId(202213));
	} else if (sentence->localWord("creatorroom")) {
		addResponse(getDialogueId(200848));
	} else if (sentence->localWord("servicelift")) {
		addResponse(getDialogueId(200855));
	} else if (sentence->localWord("lift")) {
		addResponse(getDialogueId(202256));
	} else if (sentence->localWord("bilgeroom")) {
		addResponse(getDialogueId(202255));
	} else if (sentence->localWord("musicroom")) {
		addResponse(getDialogueId(200851));
	} else if (sentence->localWord("parrotlobby")) {
		addResponse(getDialogueId(200852));
	} else if (sentence->localWord("parrot") &&
			(sentence->localWord("room") || sentence->localWord("lobby"))) {
		addResponse(getDialogueId(200852));
	} else if (sentence->localWord("promenade")) {
		addResponse(getDialogueId(200853));
	} else if (sentence->localWord("sculpture") || sentence->localWord("sculptureroom")
				|| sentence->localWord("statue")) {
		addResponse(getDialogueId(200854));
	} else if (sentence->localWord("lounge")) {
		addResponse(getDialogueId(200856));
	} else if (sentence->localWord("titania")) {
		if (sentence->localWord("room")) {
			addResponse(getDialogueId(200859));
		} else if (sentence->localWord("nose")) {
			addResponse(getDialogueId(200703));
		} else if (sentence->localWord("mouth")) {
			addResponse(getDialogueId(200702));
		} else if (sentence->localWord("eyes")) {
			addResponse(getDialogueId(200701));
		} else if (sentence->localWord("ear")) {
			addResponse(getDialogueId(200698));
		} else if (sentence->localWord("brain")) {
			addResponse(getDialogueId(200693));
		} else {
			addResponse(getDialogueId(200686));
		}
	} else if (sentence->localWord("embarklobby")
			|| sentence->localWord("lobby")) {
		addResponse(getDialogueId(200850));
	} else if (sentence->localWord("pellerator")) {
		addResponse(getDialogueId(200862));
	} else if (sentence->localWord("servicelift")
			|| (sentence->localWord("service") && sentence->localWord("elevator"))) {
		addResponse(getDialogueId(200855));
	} else if (sentence->localWord("elevator")) {
		addResponse(getDialogueId(202256));
	} else if (sentence->localWord("now")) {
		addResponse(getDialogueId(200788));
	} else if (sentence->localWord("room")) {
		addResponse(getDialogueId(200311));
	} else {
		return false;
	}

	return true;
}

uint BellbotScript::getRoomDialogueId(const TTroomScript *roomScript) {
	if (!roomScript)
		return 0;

	for (int idx = 0; ROOM_DIALOGUE_IDS[idx]._roomNum; ++idx) {
		if (ROOM_DIALOGUE_IDS[idx]._roomNum == roomScript->_scriptId)
			return ROOM_DIALOGUE_IDS[idx]._dialogueId;
	}

	return 0;
}

bool BellbotScript::addRoomDescription(const TTroomScript *roomScript) {
	if (!roomScript)
		return false;

	switch (roomScript->_scriptId) {
	case 101:
		addResponse(getDialogueId(getValue(2) == 1 ? 201835 : 201832));
		break;
	case 107:
		if (_room107First) {
			addResponse(getDialogueId(202162));
		} else {
			addResponse(getDialogueId(202162));
			_room107First = true;
		}
		break;
	case 108:
		addResponse(getDialogueId(201844));
		break;
	case 109:
		addResponse(getDialogueId(200303));
		break;
	case 110:
		addResponse(getDialogueId(202257));
		break;
	case 111:
		addResponse(getDialogueId(202056));
		break;
	case 112:
		addResponse(getDialogueId(201828));
		break;
	case 113:
		addResponse(getDialogueId(201859));
		break;
	case 114:
		addResponse(getDialogueId(202052));
		break;
	case 115:
		addResponse(getDialogueId(202004));
		break;
	case 116:
		addResponse(getDialogueId(202092));
		break;
	case 117:
		addResponse(getDialogueId(202027));
		break;
	case 124:
		addResponse(getDialogueId(202110));
		break;
	case 125:
		addResponse(getDialogueId(202103));
		break;
	case 126:
		addResponse(getDialogueId(202116));
		break;
	case 127:
		addResponse(getDialogueId(202111));
		break;
	case 128:
		addResponse(getDialogueId(201815));
		break;
	case 129:
		addResponse(getDialogueId(201816));
		break;
	case 131:
		addResponse(getDialogueId(201930));
		break;
	case 132:
		addResponse(getDialogueId(201924));
		break;
	default:
		return false;
	}

	return true;
}

} // End of namespace Titanic
