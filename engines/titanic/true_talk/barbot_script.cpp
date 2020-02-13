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
#include "titanic/true_talk/barbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

static const int STATE_ARRAY_EN[7] = {
	51888, 51890, 51891, 51892, 51893, 51894, 51895
};
static const int STATE_ARRAY_DE[7] = {
	51903, 51905, 51906, 51907, 51908, 51909, 51910
};

static const uint ARRAY1_EN[23] = {
	0, 50033, 50044, 50045, 50046, 50047, 50048, 50049,
	50050, 50051, 50034, 50035, 50036, 50037, 50038, 50039,
	50040, 50041, 50042, 50043, 50411, 0
};
static const uint ARRAY1_DE[23] = {
	0, 50033, 50044, 50045, 50046, 50047, 50048, 50049, 50050,
	50051, 50034, 50035, 50036, 50037, 50038, 50039, 50040,
	50041, 50042, 50043, 50421, 0, 0
};

static const uint ARRAY2_EN[10] = {
	51899, 51900, 51901, 51902, 51903, 51904, 51905, 51906, 51907, 0
};
static const uint ARRAY2_DE[10] = {
	51914, 51915, 51916, 51917, 51918, 51919, 51920, 51921, 51922, 0
};

BarbotScript::BarbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {
	_state = 0;
	_arrIndex = 0;

	loadRanges("Ranges/Barbot");
	loadResponses("Responses/Barbot");
	setupSentences();
	_tagMappings.load("TagMap/Barbot");
	_quotes.load("Quotes/Barbot");
	_states.load("States/Barbot");
	_preResponses.load("PreResponses/Barbot");
}

void BarbotScript::setupSentences() {
	for (int idx = 28; idx < 35; ++idx)
		CTrueTalkManager::setFlags(idx, 0);
	setupDials(100, 100, 100);

	if (!_currentDialNum)
		_currentDialNum = 2;

	_mappings.load("Mappings/Barbot", 8);
	_entries.load("Sentences/Barbot");
	_entries2.load("Sentences/Barbot2");
	_words.load("Words/Barbot");
}

int BarbotScript::chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) {
	if (tag == MKTAG('D', 'N', 'A', '1') || tag == MKTAG('H', 'H', 'G', 'Q') ||
			tag == MKTAG('A', 'N', 'S', 'W') || tag == MKTAG('S', 'U', 'M', 'S')) {
		if (_state < 7) {
			addResponse(TRANSLATE(STATE_ARRAY_EN[_state++], STATE_ARRAY_DE[_state++]));
		} else {
			selectResponse(TRANSLATE(51896, 51911));
			setState(1);
			_state = 0;
		}

		applyResponse();
		return 2;

	} else if (tag == MKTAG('S', 'W', 'E', 'R')) {
		adjustDial(0, -18);
		adjustDial(1, -5);

		if (getRandomNumber(100) > 50) {
			addResponse(getDialogueId(getDialRegion(0) == 0 ? 250200 : 250062));
			applyResponse();
			return 2;
		}

	} else if (tag == MKTAG('B', 'A', 'R', 'K') && getRandomNumber(100) > 50) {
		selectResponse(250025);
		switch (getDialsBitset()) {
		case 4:
		case 6:
			addResponse(getDialogueId(250125));
			break;
		default:
			break;
		}

		applyResponse();
		return 2;

	} else if (tag == MKTAG('B', 'A', 'R', 'U') && getRandomNumber(100) > 50) {
		selectResponse(250025);
		switch (getDialsBitset()) {
		case 4:
		case 6:
			addResponse(getDialogueId(250112));
			break;
		default:
			break;
		}

		applyResponse();
		return 2;
	}

	if (tag == MKTAG('T', 'H', 'R', 'T') || tag == MKTAG('S', 'L', 'O', 'W') ||
			tag == MKTAG('S', 'E', 'X', '1') || tag == MKTAG('P', 'K', 'U', 'P')) {
		adjustDial(0, -7);
		adjustDial(1, -3);
	}

	return TTnpcScript::chooseResponse(roomScript, sentence, tag);
}

int BarbotScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	int dialogueId = 0;

	if (roomScript->_scriptId != 112)
		return 2;

	checkItems(roomScript, sentence);
	if (isState9()) {
		if (sentence->localWord("visioncenter") || sentence->localWord("brain") ||
				sentence->contains("vision") || sentence->contains("visual") ||
				sentence->contains("brain") || sentence->contains("crystal") ||
				sentence->contains("gesichtsmodul") || sentence->contains("sehmodul") ||
				sentence->contains("gesichtszentrum") || sentence->contains("hirn") ||
				sentence->contains("hirnstueck")
			) {
			if (CTrueTalkManager::getStateValue(2)) {
				addResponse(getDialogueId(251003));
				applyResponse();
				CTrueTalkManager::triggerAction(6, 0);
				return 2;
			}
		}

		if (sentence->contains("goldfish") || sentence->contains("goldfisch")) {
			addResponse(getDialogueId(250184));
			applyResponse();
			return 2;
		}

		dialogueId = TRANSLATE(ARRAY1_EN[getRandomNumber(20)], ARRAY1_DE[getRandomNumber(20)]);
		if (!TRANSLATE(ARRAY2_EN[_arrIndex], ARRAY2_DE[_arrIndex]))
			_arrIndex = 0;

		if (_arrIndex) {
			dialogueId = TRANSLATE(ARRAY2_EN[_arrIndex++], ARRAY2_DE[_arrIndex++]);
		} else if (getRandomNumber(100) > 35) {
			dialogueId = TRANSLATE(ARRAY2_EN[0], ARRAY2_DE[0]);
			_arrIndex = 1;
		} else if (getRandomNumber(100) > 60) {
			switch (sentence->_category) {
			case 2:
				dialogueId = TRANSLATE(51914, 51929);
				break;
			case 3:
				dialogueId = TRANSLATE(51911, 51926);
				break;
			case 4:
				dialogueId = TRANSLATE(51913, 51928);
				break;
			case 5:
				dialogueId = TRANSLATE(51912, 51927);
				break;
			case 6:
				dialogueId = TRANSLATE(51915, 51930);
				break;
			case 7:
				dialogueId = TRANSLATE(51909, 51924);
				break;
			default:
				break;
			}
		}

		addResponse(dialogueId);
		if (getRandomNumber(100) > 65)
			addResponse(getDialogueId(251250));
		applyResponse();
		return 2;
	}

	CTrueTalkManager::setFlags(29, getValue(29) - 1);
	CTrueTalkManager::setFlags(30, getValue(30) - 1);
	CTrueTalkManager::setFlags(31, getValue(31) - 1);
	CTrueTalkManager::setFlags(32, getValue(32) - 1);
	CTrueTalkManager::setFlags(33, getValue(33) - 1);
	CTrueTalkManager::setFlags(34, getValue(34) - 1);

	TTtreeResult treeResult[32];
	int val34 = getState();
	setState(0);

	int val2C = sentence->_category;
	bool flag = val2C == 11 || val2C == 13;
	bool flag2 = val2C == 12;

	if (!val34) {
		goto done;
	} else if (val34 > TRANSLATE(50357, 50366)) {
		goto done;
	} else if (val34 == TRANSLATE(50357, 50366)) {
		return applySentenceIds(TRANSLATE(50358, 50367), -1);
	}

	switch (val34) {
	case 1:
		if (flag)
			return applySentenceIds(TRANSLATE(51898, 51913), 2);
		if (flag2)
			return applySentenceIds(TRANSLATE(51897, 51912));
		break;
	case 2:
		if (flag)
			return applySentenceIds(TRANSLATE(51897, 51912));
		break;
	case 3:
		if (sentence->localWord("useless") || sentence->contains("useless") ||
				sentence->contains("hoffnungsloser fall"))
			return applySentenceIds(TRANSLATE(50824, 50837));
		break;
	case 4:
		if (flag)
			return applySentenceIds(getRandomBit() ?
				TRANSLATE(50512, 50522) : TRANSLATE(51642, 51657));
		else if (flag2)
			return applySentenceIds(getRandomBit() ?
				TRANSLATE(50511, 50521) : TRANSLATE(51643, 51658));
		break;
	case 5:
		if (flag)
			return applySentenceIds(TRANSLATE(50829, 50842), 6);
		if (flag2)
			return applySentenceIds(TRANSLATE(50828, 50841));
		break;
	case 6:
		if (flag)
			return applySentenceIds(TRANSLATE(50831, 50844));
		if (flag2)
			return applySentenceIds(TRANSLATE(50830, 50843));
		break;
	case 7:
		if (flag2 || sentence->contains("never") || sentence->contains("niemals") ||
				sentence->contains("nie"))
			return applySentenceIds(TRANSLATE(51553, 51568));
		if (flag || sentence->contains("nicest") || sentence->contains("schoenste"))
			return applySentenceIds(TRANSLATE(51554, 51569));
		break;
	case 8:
		if (flag)
			return applySentenceIds(TRANSLATE(50961, 50974));
		if (flag2)
			return applySentenceIds(TRANSLATE(50960, 50973));
		break;
	case 9:
		if (flag)
			return applySentenceIds(getDialogueId(251858));
		break;
	case 10:
		if (flag)
			return applySentenceIds(getDialogueId(251014));
		else if (flag2)
			return applySentenceIds(getDialogueId(251013));
		break;
	case 11:
		if (flag)
			return applySentenceIds(getDialogueId(251008));
		else if (flag2)
			return applySentenceIds(getDialogueId(251007));
		break;
	case 12:
		if (flag)
			return applySentenceIds(getDialogueId(250656));
		else if (flag2)
			return applySentenceIds(getDialogueId(250655));
		break;
	case 13:
		if (flag)
			return applySentenceIds(getDialogueId(250614));
		else if (flag2)
			return applySentenceIds(getDialogueId(250613));
		break;
	case 14:
		if (val2C == 6)
			return applySentenceIds(getDialogueId(250946));
		break;
	case 15:
		if (flag || sentence->contains("or") || sentence->contains("oder")) {
			return applySentenceIds(getDialogueId(250526), 16);
		} else {
			if (g_vm->_trueTalkManager->_quotesTree.search(
					sentence->_normalizedLine.c_str(), TREE_3, &treeResult[0], 0, nullptr) != -1) {
				uint newId = getDialogueId(250526);
				return applySentenceIds(newId, 16);
			}
		}
		break;
	case 17:
		if (flag) {
			return applySentenceIds(TRANSLATE(50382, 50391));
		} else if (flag2) {
			return applySentenceIds(TRANSLATE(51423, 51438));
		}
		// Intentional fall-through

	case 16:
		if (val2C == 7 || val2C == 10)
			return applySentenceIds(getDialogueId(250525));
		break;
	case 18:
		return applySentenceIds(getDialogueId(250589));
	case 19:
		return applySentenceIds(getDialogueId(250565), 20);
	case 20:
		if (flag)
			return applySentenceIds(TRANSLATE(50307, 50315));
		if (flag2)
			return applySentenceIds(TRANSLATE(50306, 50314));
		break;
	case 21:
		if (flag)
			return applySentenceIds(TRANSLATE(50359, 50368));
		if (flag2)
			return applySentenceIds(TRANSLATE(50357, 50366));
		break;
	case 23:
		if (val2C == 6 || val2C == 10)
			return applySentenceIds(getDialogueId(250551));
		break;
	case 24:
		if (sentence->contains("do not know")
				|| sentence->contains("no idea")
				|| sentence->contains("a clue")
				|| sentence->contains("keine ahnung")
				|| sentence->contains("weiss nicht")
				|| sentence->contains("keinen schimmer")) {
			return applySentenceIds(getDialogueId(250553));
		} else {
			return applySentenceIds(getDialogueId(250552));
		}
		break;
	case 25:
		if (flag || val2C == 10)
			applySentenceIds(getDialogueId(251899), 26);
		else if (flag2)
			return applySentenceIds(TRANSLATE(50215, 50223));
		break;
	case 26:
		if (g_vm->_trueTalkManager->_quotesTree.search(
				sentence->_normalizedLine.c_str(), TREE_3, &treeResult[0], 0, nullptr) != -1)
			return applySentenceIds(getDialogueId(251899), 26);
		break;

	case 27:
		if (flag)
			return applySentenceIds(getDialogueId(250766));
		else if (flag2)
			return applySentenceIds(getDialogueId(250764));
		break;
	case 28:
		return applySentenceIds(getDialogueId(250765));
	case 29:
		return applySentenceIds(getDialogueId(250652));
	case 30:
		return applySentenceIds(getDialogueId(250653));
	case 31:
		if (flag)
			return applySentenceIds(getDialogueId(250664));
		else if (flag2)
			return applySentenceIds(getDialogueId(250663));
		break;
	case 32:
		if (flag)
			return applySentenceIds(getDialogueId(250643));
		else if (flag2)
			return applySentenceIds(getDialogueId(250642));
		break;
	case 33:
		return applySentenceIds(TRANSLATE(50763, 50776));
	case 34:
		if (flag)
			return applySentenceIds(getDialogueId(251622));
		else if (flag2)
			return applySentenceIds(getDialogueId(251624));
		break;
	case 35:
		if (val2C == 6 || val2C == 10)
			return applySentenceIds(getDialogueId(251623));
		break;
	case 36:
		if (flag)
			return applySentenceIds(TRANSLATE(50335, 50344));
		if (flag2)
			return applySentenceIds(TRANSLATE(50334, 50343));
		break;
	case 37:
		if (flag)
			return applySentenceIds(TRANSLATE(50217, 50225));
		if (flag2)
			return applySentenceIds(TRANSLATE(50153, 50157));
		break;
	case 38:
		return applySentenceIds(getDialogueId(250637));
	case 39:
		return applySentenceIds(getDialogueId(250638));
	case 40:
		return applySentenceIds(getDialogueId(250639));
	case 41:
		return applySentenceIds(getDialogueId(250640));
	case 42:
		if (flag)
			return applySentenceIds(getDialogueId(250676));
		else if (flag2)
			return applySentenceIds(getDialogueId(250673));
		break;
	case 43:
		if (flag)
			return applySentenceIds(TRANSLATE(50416, 50426), -1);
		if (flag2)
			return applySentenceIds(TRANSLATE(50415, 50425), -1);
		break;
	case 44:
		if (flag)
			return applySentenceIds(getDialogueId(250468));
		else if (flag2)
			return applySentenceIds(getDialogueId(250413));

		if (val2C == 6 || val2C == 10)
			return applySentenceIds(getDialogueId(251649));
		break;
	case 45:
		if (sentence->localWord("summer")
				|| sentence->contains("summer")
				|| sentence->localWord("autumn")
				|| sentence->contains("autumn")
				|| sentence->contains("herbst")) {
			return applySentenceIds(TRANSLATE(50743, 50755));
		} else if (sentence->localWord("winter") || sentence->contains("winter")) {
			return applySentenceIds(TRANSLATE(50696, 50708));
		} else {
			return applySentenceIds(TRANSLATE(50225, 50233));
		}
		break;
	case 46:
		if (val2C == 7 || val2C == 10)
			return applySentenceIds(TRANSLATE(50698, 50710));
		break;
	case 47:
		if (flag || flag2 || val2C == 6)
			return applySentenceIds(TRANSLATE(50717, 50729));
		break;
	case 48:
		if (flag)
			return applySentenceIds(TRANSLATE(50710, 50722));
		if (flag2)
			return applySentenceIds(TRANSLATE(50225, 50233));
		break;
	case 49:
		if (sentence->localWord("scraliontis") || sentence->contains("scraliontis") ||
				sentence->contains("skraliontis"))
			return applySentenceIds(TRANSLATE(50711, 50723));
		if (sentence->localWord("brobostigon") || sentence->contains("brobostigon"))
			return applySentenceIds(TRANSLATE(50712, 50724));
		break;
	case 50:
		return applySentenceIds(TRANSLATE(50713, 50725));
	case 51:
		if (flag)
			return applySentenceIds(TRANSLATE(50715, 50727));
		if (flag2)
			return applySentenceIds(TRANSLATE(50714, 50726));
		break;
	case 52:
		if (sentence->localWord("note") || sentence->contains("note"))
			return applySentenceIds(TRANSLATE(50716, 50728));
		return  applySentenceIds(TRANSLATE(50210, 50218));
	case 53:
		return applySentenceIds(TRANSLATE(50210, 50218));
	case 54:
		if (getDialRegion(0) != 0) {
			if (val2C)
				return applySentenceIds(TRANSLATE(50174, 50178));
			else
				return applySentenceIds(TRANSLATE(50300, 50308));
		} else if (val2C == 7 || val2C == 10) {
			return applySentenceIds(TRANSLATE(50871, 50884));
		}
		break;
	case 55:
		if (flag)
			return applySentenceIds(TRANSLATE(50302, 50310));
		if (flag2)
			return applySentenceIds(TRANSLATE(50301, 50309));
		break;
	case 56:
		if (flag)
			return applySentenceIds(TRANSLATE(50304, 50312));
		if (flag2)
			return applySentenceIds(TRANSLATE(50303, 50311));
		break;
	case 57:
		if (sentence->localWord("mustard")
			|| sentence->contains("mustard")
			|| sentence->localWord("tomato")
			|| sentence->contains("tomato")
			|| sentence->contains("senf")
			|| sentence->contains("tomate"))
			return applySentenceIds(TRANSLATE(50320, 50329));
		if (sentence->localWord("sauce")
			|| sentence->localWord("puree")
			|| sentence->contains("sauce")
			|| sentence->contains("puree")
			|| sentence->contains("bird")
			|| sentence->contains("starling")
			|| sentence->contains("sosse")
			|| sentence->contains("pueree")
			|| sentence->contains("vogel")
			|| sentence->contains("staren")) {
			applySentenceIds(TRANSLATE(50321, 50330));
			CTrueTalkManager::triggerAction(30, 0);
			return 2;
		}

		return applySentenceIds(TRANSLATE(50320, 50329));
	case 58:
		if (val2C == 6 || val2C == 10)
			return applySentenceIds(TRANSLATE(50880, 50893));
		break;
	case 59:
		if (flag) {
			if (addRandomResponse(true)) {
				setState(59);
				return 2;
			}
		} else if (flag2) {
			return applySentenceIds(getDialogueId(251754));
		}
		break;
	case 60:
		if (flag && addRandomResponse(true)) {
			setState(59);
			return 2;
		} else if (flag2 || val2C == 7 || val2C == 10) {
			return applySentenceIds(getDialogueId(251712));
		}
		break;
	case 61:
		if (val2C == 3) {
			if (sentence->localWord("loop"))
				return applySentenceIds(getDialogueId(250269));
			else if (sentence->localWord("do"))
				return applySentenceIds(getDialogueId(250270));
		} else if (val2C == 7) {
			return applySentenceIds(getDialogueId(250270));
		} else if (flag) {
			return applySentenceIds(getDialogueId(250270));
		}

		return applySentenceIds(getDialogueId(250272));
	case 62:
		if (flag
				|| (val2C == 3 && sentence->localWord("do"))
				|| val2C == 7
				|| sentence->localWord("help"))
			return applySentenceIds(getDialogueId(250270));

		return applySentenceIds(getDialogueId(2570272));
	case 63:
		if (flag
				|| (val2C == 3 || sentence->localWord("do"))
				|| val2C == 7
				|| sentence->localWord("help"))
			return applySentenceIds(getDialogueId(250271));

		return applySentenceIds(getDialogueId(250272));
	case 64:
		if (flag || val2C == 3 || val2C == 8)
			return applySentenceIds(getDialogueId(250631));
		break;
	case 65:
		if (sentence->localWord("now") || sentence->localWord("soon"))
			return applySentenceIds(getDialogueId(250424));
		return applySentenceIds(getDialogueId(250506));
	case 66:
		if (flag || sentence->localWord("good") || sentence->localWord("well"))
			return applySentenceIds(getDialogueId(251027));
		return applySentenceIds(getDialogueId(251021));
	case 67:
		if (flag || val2C == 6 || val2C == 10) {
			setDial(0, getDialLevel(0, false) - 8);
			return applySentenceIds(getDialogueId(251589));
		}
		break;
	case 68:
		if (flag || val2C == 6 || val2C == 10) {
			setDial(0, getDialLevel(0, false) - 12);
			return applySentenceIds(getDialogueId(251590));
		}
		break;
	case 69:
		if (flag || val2C == 6 || val2C == 10) {
			setDial(0, getDialLevel(0, false) - 25);
			return applySentenceIds(getDialogueId(251591));
		}
		break;
	default:
		break;
	}

done:
	// Adjust primary dial
	setState(0);
	if (sentence->get58() != 5) {
		adjustDial(0, sentence->get58() * 4 - 20);
	} else if (getDialLevel(0, false) > 65) {
		adjustDial(0, -2 - getRandomNumber(7));
	} else if (getDialLevel(0, false) < 35) {
		adjustDial(0, 2 + getRandomNumber(7));
	}

	updateCurrentDial(true);

	if (sentence->contains("goldfish") || sentence->contains("goldfisch")) {
		addResponse(250184);
	} else if ((sentence->localWord("puree") || sentence->localWord("pureed"))
		&& sentence->localWord("parrot")) {
		addResponse(250021);
	} else if (sentence->localWord("starling")) {
		addResponse(250024);
	} else {
		if (getRandomNumber(100) > 95 && getDialRegion(2) == 0) {
			addResponse(getDialogueId(250210));
		}

		if (processEntries(&_entries, _entryCount, roomScript, sentence) == 2)
			return 2;
		if (processEntries(_defaultEntries, 0, roomScript, sentence) != 2
				&& !defaultProcess(roomScript, sentence)) {
			int dval = 0;
			flag = getRandomNumber(100) > 50;
			int val;

			switch (_field2C) {
			case 2:
				val = getValue(29);
				if (val < 16)
					val += 4;
				if (val < 9) {
					val = val / 2;
					dval = 250081 + (flag ? 0 : 267);
				}
				CTrueTalkManager::setFlags(29, val);
				break;

			case 3:
				val = getValue(30);
				if (val < 16)
					val += 4;
				if (val < 9) {
					val = val / 2;
					dval = 250081 + (flag ? 0 : 243);
				}
				CTrueTalkManager::setFlags(30, val);
				break;

			case 4:
				val = getValue(31);
				if (val < 16)
					val += 4;
				if (val < 9) {
					val = val / 2;
					dval = 250081 + (flag ? 0 : 256);
				}
				CTrueTalkManager::setFlags(31, val);
				break;

			case 5:
				val = getValue(32);
				if (val < 16)
					val += 4;
				if (val < 9) {
					val = val / 2;
					dval = 250081 + (flag ? 0 : 251);
				}
				CTrueTalkManager::setFlags(32, val);
				break;

			case 6:
				val = getValue(33);
				if (val < 16)
					val += 4;
				if (val < 9) {
					val = val / 2;
					dval = 250081 + (flag ? 0 : 273);
				}
				CTrueTalkManager::setFlags(33, val);
				break;

			case 7:
				val = getValue(34);
				if (val < 16)
					val += 4;
				if (val < 9) {
					val = val / 2;
					dval = 250081 + (flag ? 0 : 236);
				}
				CTrueTalkManager::setFlags(34, val);
				break;


			case 11:
				addResponse(getDialogueId(250463));
				applyResponse();
				return 2;

			case 12:
				addResponse(getDialogueId(250455));
				applyResponse();
				return 2;

			case 13:
				addResponse(getDialogueId(250447));
				applyResponse();
				return 2;

			case 19:
				return applySentenceIds(getDialogueId(getDialRegion(0) ? 250062 : 250200));

			default:
				break;
			}

			if (dval) {
				adjustDial(0, -9);
				adjustDial(1, -2);

				if (dval != 250081) {
					selectResponse(250286);
					selectResponse(250296);
					selectResponse(250307);
					applyResponse();
					return 2;
				}
			} else if (processEntries(&_entries2, 0, roomScript, sentence) == 2) {
				return 2;
			}

			addResponse(getDialogueId(250082 + (getRandomNumber(100) <= 89 ? 128 : 0)));
		}
	}

	applyResponse();
	return 2;
}

ScriptChangedResult BarbotScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	switch (id) {
	case 1:
	case 100:
		if (!isState9()) {
			selectResponse(250210);
			applyResponse();
		}

		adjustDial(0, getRandomBit() ? getRandomNumber(5) * 4 :
			-(int)getRandomNumber(5) * 4);
		break;

	case 3:
		if (isState9()) {
			selectResponse(250244);
			applyResponse();
			resetFlags();
		} else {
			if (!getValue(28) || !fn10(true)) {
				addResponse(getDialogueId(251627 + (getValue(28) ? -1034 : 0)));
				applyResponse();
			}

			CTrueTalkManager::setFlags(28, 1);
			resetFlags();
		}
		break;

	case 4:
		selectResponse(isState9() ? 250141 : 250140);
		applyResponse();
		adjustDial(2, getDialLevel(2, false) < 50 ? -15 - getRandomNumber(30) :
			15 + getRandomNumber(30));

		if (getDialRegion(1) != 0 && getRandomNumber(100) > 75)
			adjustDial(1, -35);
		break;

	case 143:
		addResponse(getDialogueId(isState9() ? 250577 : 250576));
		break;

	case 144:
		addResponse(getDialogueId(isState9() ? 250577 : 250584));
		break;

	case 145:
		if (isState9()) {
			addResponse(getDialogueId(250577));
			applyResponse();
		} else {
			setState(57);
		}
		break;

	case 146:
		addResponse(getDialogueId(isState9() ? 250577 : 250574));
		break;

	case 147:
		addResponse(getDialogueId(250579));
		break;

	default:
		break;
	}

	if (id >= 250000 && id <= 251900) {
		if (id > 250571) {
			if (id != 250575 && (id == 250586 || id == 251858 || !isState9())) {
				addResponse(getDialogueId(id));
				applyResponse();
			}
		} else if (id == 250571 || (id != 250244 && !isState9()) || isState9()) {
			addResponse(getDialogueId(id));
			applyResponse();
		} else {
			addResponse(getDialogueId(251018));
			applyResponse();
		}
	}

	return SCR_2;
}

int BarbotScript::handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
		uint tag1, uint tag2, uint remainder) {
	switch (tag2) {
	case MKTAG('A', 'D', 'V', 'T'):
	case MKTAG('A', 'R', 'T', 'I'):
	case MKTAG('A', 'R', 'T', 'Y'):
	case MKTAG('B', 'R', 'N', 'D'):
	case MKTAG('C', 'O', 'M', 'D'):
	case MKTAG('D', 'N', 'C', 'E'):
	case MKTAG('H', 'B', 'B', 'Y'):
	case MKTAG('M', 'A', 'G', 'S'):
	case MKTAG('L', 'I', 'T', 'R'):
	case MKTAG('M', 'C', 'P', 'Y'):
	case MKTAG('M', 'I', 'N', 'S'):
	case MKTAG('M', 'U', 'S', 'I'):
	case MKTAG('N', 'I', 'K', 'E'):
	case MKTAG('S', 'F', 'S', 'F'):
	case MKTAG('S', 'O', 'A', 'P'):
	case MKTAG('S', 'O', 'N', 'G'):
	case MKTAG('S', 'P', 'R', 'T'):
	case MKTAG('T', 'E', 'A', 'M'):
	case MKTAG('T', 'U', 'S', 'H'):
	case MKTAG('W', 'W', 'E', 'B'):
		tag2 = MKTAG('E', 'N', 'T', 'N');
		break;
	case MKTAG('A', 'U', 'T', 'H'):
	case MKTAG('B', 'A', 'R', 'K'):
	case MKTAG('B', 'L', 'R', '1'):
	case MKTAG('B', 'L', 'P', '1'):
	case MKTAG('B', 'L', 'P', '2'):
	case MKTAG('B', 'L', 'P', '3'):
	case MKTAG('B', 'L', 'P', '4'):
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
		tag2 = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 'T'):
	case MKTAG('F', 'I', 'L', 'M'):
	case MKTAG('J', 'F', 'O', 'D'):
	case MKTAG('L', 'I', 'Q', 'D'):
		tag2 = MKTAG('F', 'O', 'O', 'D');
		break;
	case MKTAG('C', 'R', 'M', 'N'):
	case MKTAG('C', 'S', 'P', 'Y'):
	case MKTAG('U', 'B', 'A', 'D'):
		tag2 = MKTAG('V', 'B', 'A', 'D');
		break;
	case MKTAG('E', 'A', 'R', 'T'):
	case MKTAG('H', 'O', 'M', 'E'):
	case MKTAG('N', 'P', 'L', 'C'):
	case MKTAG('P', 'L', 'A', 'C'):
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
	case MKTAG('S', 'C', 'I', 'E'):
		tag2 = MKTAG('S', 'C', 'I', 'E');
		break;
	case MKTAG('T', 'R', 'A', '2'):
	case MKTAG('T', 'R', 'A', '3'):
		tag2 = MKTAG('T', 'R', 'A', 'V');
		break;
	default:
		break;
	}

	if (tag1 == 36) {
		switch (getValue(1)) {
		case 1:
			return setResponse(getDialogueId(220837), -1);
			break;
		case 2:
			return setResponse(getDialogueId(220849), -1);
		default:
			return setResponse(getDialogueId(220858), -1);
		}
	} else if (tag1 == 61 && getValue(1) > 2) {
		return setResponse(getDialogueId(222301), -1);
	}

	return TTnpcScript::handleQuote(roomScript, sentence, tag1, tag2, remainder);
}

int BarbotScript::updateState(uint oldId, uint newId, int index) {
	if (newId == 250538) {
		CTrueTalkManager::triggerAction(28, 0);
		return 250538;
	}
	if (newId == 251704) {
		return 251701 + (_field7C ? 3 : 0);
	}

	for (uint idx = 0; idx < _states.size(); ++idx) {
		const TTupdateState &us = _states[idx];
		if (us._newId == newId) {
			if ((us._dialBits & 1) && !getDialRegion(0))
				continue;
			if ((us._dialBits & 2) && getDialRegion(0))
				continue;
			if ((us._dialBits & 4) && !getDialRegion(1))
				continue;
			if ((us._dialBits & 8) && getDialRegion(1))
				continue;
			if ((us._dialBits & 0x10) && !getDialRegion(2))
				continue;
			if ((us._dialBits & 0x20) && getDialRegion(2))
				continue;

			setState(us._newValue);
			break;
		}
	}

	return newId;
}

int BarbotScript::preResponse(uint id) {
	if (getDialRegion(0) == 0 && getRandomNumber(100) > 80)
		return 251250;

	return _preResponses.find(id);
}

uint BarbotScript::getDialsBitset() const {
	uint bits = 0;
	if (!getDialRegion(0))
		bits = 1;
	if (!getDialRegion(1))
		bits |= 2;
	if (getDialRegion(2))
		bits |= 4;

	return bits;
}

int BarbotScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	uint id = 0;

	int index = val1;
	if (g_language == Common::DE_DEU && !(val1 >= 512 && val1 <= 515))
		index -= 1000;

	switch (index) {
	case 2:
		if (getValue(1) != 1)
			return 1;
		break;

	case 3:
		if (getValue(1) != 2)
			return 1;
		break;

	case 4:
		if (getValue(1) != 3)
			return 1;
		break;

	case 5:
		if (getValue(1) == 3)
			return 1;
		break;

	case 6:
		if (sentence->contains("do not") || sentence->contains("have no") ||
			sentence->contains("got no"))
			return 1;
		break;

	case 7:
		if (!sentence->contains(TRANSLATE("do not", "idem")) && !sentence->contains(TRANSLATE("have no", "habe kein")) &&
				!sentence->contains("got no"))
			return 1;
		break;

	case 8:
		if (sentence->_field38 == 2)
			return 1;
		break;

	case 9: {
		uint val = CTrueTalkManager::getStateValue(3);
		bool lemonFlag = (val & 1) != 0;
		bool puretFlag = (val & 4) != 0;
		bool tvFlag = (val & 8) != 0;

		if (puretFlag) {
			if (!lemonFlag) {
				id = tvFlag ? 50369 : 250085;
				break;
			} else if (!tvFlag) {
				id = 250627;
			}
		} else {
			if (lemonFlag) {
				id = tvFlag ? 50367 : 50365;
			} else if (tvFlag) {
				id = 50370;
			}
		}
		break;
	}

	case 10: {
		uint val = CTrueTalkManager::getStateValue(3);
		bool lemonFlag = (val & 1) != 0;
		bool puretFlag = (val & 4) != 0;
		bool tvFlag = (val & 8) != 0;

		if (lemonFlag && puretFlag && tvFlag) {
			addResponse(getDialogueId(251027));
			applyResponse();
			CTrueTalkManager::triggerAction(7, 0);
			return 2;
		} else {
			if (getDialRegion(1) == 1) {
				if (*srcIdP != 251650)
					id = 251651;
			} else {
				addResponse(getDialRegion(0) != 0 ? 51444 : 51530);
				applyResponse();
				return 2;
			}
		}
		break;
	}

	case 11:
		if (CTrueTalkManager::getStateValue(2) != 0) {
			CTrueTalkManager::triggerAction(6, 0);
			id = 251003;
		}
		break;

	case 12:
		if (getDialRegion(1) == 0) {
			addResponse(getDialogueId(251871));
			applyResponse();
			return 2;
		} else if (getRandomNumber(100) > 25 && addRandomResponse(false)) {
			return 2;
		}
		break;

	case 512:
		if (getValue(4) != 1)
			id = 250738;
		break;

	case 513:
		if (getValue(4) != 2)
			id = 250738;
		break;

	case 514:
		if (getValue(4) != 3)
			id = 250738;
		break;

	case 515:
		if (getValue(4) != 0)
			id = 250738;
		break;

	default:
		if (g_language == Common::DE_DEU)
			return TTnpcScript::doSentenceEntry(val1, srcIdP, roomScript, sentence);
		break;
	}

	if (id) {
		addResponse(getDialogueId(id));
		applyResponse();
		return 2;
	}

	return 0;
}

void BarbotScript::setDialRegion(int dialNum, int region) {
	TTnpcScript::setDialRegion(dialNum, region);
	selectResponse(250365);
	applyResponse();
}

void BarbotScript::adjustDial(int dialNum, int amount) {
	int level = CLIP(getDialLevel(dialNum) + amount, 0, 100);
	setDial(dialNum, level);
}

bool BarbotScript::isState9() const {
	return CTrueTalkManager::getStateValue(9) != 0;
}

int BarbotScript::applySentenceIds(int dialogueId, int v34) {
	addResponse(dialogueId);
	applyResponse();

	if (v34 != -1) {
		setState(v34);
	} else {
		for (uint idx = 0; idx < _mappings.size(); ++idx) {
			const TTscriptMapping &m = _mappings[idx];
			for (int vidx = 0; vidx < _mappings._valuesPerMapping; ++vidx) {
				if (m._values[vidx] == (uint)dialogueId) {
					updateState(m._id, m._id, vidx);
					break;
				}
			}
		}
	}

	return -2;
}

int BarbotScript::setResponse(int dialogueId, int state) {
	addResponse(dialogueId);
	applyResponse();

	if (state != -1)
		setState(state);
	return 2;
}

} // End of namespace Titanic
