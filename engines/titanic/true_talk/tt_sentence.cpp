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

#include "titanic/true_talk/tt_sentence.h"
#include "titanic/true_talk/tt_concept.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

TTsentenceConcept::~TTsentenceConcept() {
	for (int idx = 0; idx <= 5; ++idx)
		delete _concepts[idx];
}

TTsentenceConcept *TTsentenceConcept::addSibling() {
	if (_nextP != nullptr)
		// This should never happen
		return nullptr;

	TTsentenceConcept *nextP = new TTsentenceConcept();
	_nextP = nextP;
	return nextP;
}

/*------------------------------------------------------------------------*/

TTsentence::TTsentence(int inputCtr, const TTstring &line, CScriptHandler *owner,
		TTroomScript *roomScript, TTnpcScript *npcScript) :
		_owner(owner), _category(1), _inputCtr(inputCtr), _field34(0),
		_field38(0), _initialLine(line), _nodesP(nullptr), _roomScript(roomScript),
		_npcScript(npcScript), _field58(5), _field5C(5) {
	_status = _initialLine.isValid() && _normalizedLine.isValid() ? SS_11: SS_VALID;
}

TTsentence::TTsentence(const TTsentence *src) : _sentenceConcept(src->_sentenceConcept),
		_initialLine(src->_initialLine), _normalizedLine(src->_normalizedLine) {
	copyFrom(*src);
}

TTsentence::~TTsentence() {
	_sentenceConcept.deleteSiblings();

	if (_nodesP) {
		_nodesP->deleteSiblings();
		delete _nodesP;
	}
}

void TTsentence::copyFrom(const TTsentence &src) {
	if (!src.getStatus())
		_status = SS_5;
	else if (!src._initialLine.isValid() || !src._normalizedLine.isValid())
		_status = SS_11;
	else
		_status = SS_VALID;

	_inputCtr = src._inputCtr;
	_owner = src._owner;
	_roomScript = src._roomScript;
	_npcScript = src._npcScript;
	_field58 = src._field58;
	_field5C = src._field5C;
	_field34 = src._field34;
	_field38 = src._field38;
	_category = src._category;
	_nodesP = nullptr;

	if (src._nodesP) {
		// Source has processed nodes, so duplicate them
		for (TTsentenceNode *node = src._nodesP; node;
				node = dynamic_cast<TTsentenceNode *>(node->_nextP)) {
			TTsentenceNode *newNode = new TTsentenceNode(node->_wordP);
			if (_nodesP)
				_nodesP->addToTail(newNode);
			else
				_nodesP = newNode;
		}
	}
}

int TTsentence::storeVocabHit(TTword *word) {
	if (!word)
		return 0;

	TTsentenceNode *node = new TTsentenceNode(word);
	if (_nodesP) {
		_nodesP->addToTail(node);
	} else {
		_nodesP = node;
	}

	return 0;
}

bool TTsentence::fn1(const CString &str, int wordId1, const CString &str1, const CString &str2,
		const CString &str3, int wordId2, int val1, int val2, const TTconceptNode *node) const {
	if (!node)
		node = &_sentenceConcept;

	if (!node)
		return false;
	if (val1 && !is18(val1, node))
		return false;
	if (!str.empty() && !fn2(0, str, node))
		return false;
	if (wordId1 && !fn4(1, wordId1, node))
		return false;
	if (!str1.empty() && !fn2(2, str1, node))
		return false;
	if (!str2.empty() && !fn2(3, str2, node))
		return false;
	if (!str3.empty() && !fn2(4, str3, node))
		return false;
	if (wordId2 && !fn4(5, wordId2, node))
		return false;
	if (val2 && !is1C(val2, node))
		return false;

	return true;
}

bool TTsentence::fn3(const CString &str1, const CString &str2, const CString &str3,
		const CString &str4, const CString &str5, const CString &str6,
		int val1, int val2, const TTconceptNode *node) const {
	if (!node)
		node = &_sentenceConcept;

	if (val1 && !is18(val1, node))
		return false;
	if (!str1.empty() && !fn2(0, str1, node))
		return false;
	if (!str2.empty() && !fn2(1, str2, node))
		return false;
	if (!str3.empty() && !fn2(2, str3, node))
		return false;
	if (!str4.empty() && !fn2(3, str4, node))
		return false;
	if (!str5.empty() && !fn2(4, str5, node))
		return false;
	if (!str6.empty() && !fn2(5, str6, node))
		return false;
	if (!val2 && !is1C(val2, node))
		return false;

	return true;
}

bool TTsentence::fn2(int slotIndex, const TTstring &str, const TTconceptNode *node) const {
	if (!node)
		node = &_sentenceConcept;
	TTconcept *concept = getFrameSlot(slotIndex, node);

	if (!concept)
		return str == "isEmpty";

	bool abortFlag = false;
	switch (concept->_scriptType) {
	case 1:
		if (str == "thePlayer")
			abortFlag = 1;
		break;

	case 2:
		if (str == "targetNpc")
			abortFlag = 1;
		break;

	case 3:
		if (str == "otherNpc")
			abortFlag = 1;
		break;

	default:
		break;
	}

	TTstring conceptText = concept->getText();
	if (abortFlag || str == conceptText || concept->compareTo(str)) {
		delete concept;
		return true;
	}

	if (slotIndex == 1 && g_vm->_exeResources._owner->_concept4P) {
		if (str == g_vm->_exeResources._owner->_concept4P->getText() &&
				conceptText == "do")
			goto exit;
	}

	if (g_vm->_exeResources._owner->_concept2P && (slotIndex == 0 ||
			slotIndex == 3 || slotIndex == 4)) {
		if (str == g_vm->_exeResources._owner->_concept2P->getText() &&
				(conceptText == "it" || conceptText == "he" || conceptText == "she" ||
				conceptText == "him" || conceptText == "her" || conceptText == "them" ||
				conceptText == "they"))
			goto exit;
	}

	if (g_vm->_exeResources._owner->_concept1P && (slotIndex == 0 ||
		slotIndex == 2 || slotIndex == 3 || slotIndex == 4 || slotIndex == 5)) {
		if (str == g_vm->_exeResources._owner->_concept1P->getText() &&
			(conceptText == "it" || conceptText == "that" || conceptText == "he" ||
				conceptText == "she" || conceptText == "him" || conceptText == "her" ||
				conceptText == "them" || conceptText == "they" || conceptText == "those" ||
				conceptText == "1" || conceptText == "thing"))
			goto exit;
	}

	if (g_vm->_exeResources._owner->_concept1P && (slotIndex == 0 || slotIndex == 2)) {
		if (conceptText == "?" && str == g_vm->_exeResources._owner->_concept1P->getText()) {
			delete concept;
			concept = getFrameSlot(5, node);
			conceptText = concept->getText();

			if (conceptText == "it" || conceptText == "that" || conceptText == "he" ||
				conceptText == "she" || conceptText == "him" || conceptText == "her" ||
				conceptText == "them" || conceptText == "they" || conceptText == "those" ||
				conceptText == "1" || conceptText == "thing")
				abortFlag = true;
		}
	}

exit:
	delete concept;
	return abortFlag;
}

bool TTsentence::fn4(int mode, int wordId, const TTconceptNode *node) const {
	if (!node)
		node = &_sentenceConcept;

	switch (mode) {
	case 1:
		return node->_concept1P && getWordId(node->_concept1P) == wordId;

	case 5:
		return node->_concept5P && getWordId(node->_concept5P) == wordId;

	default:
		return false;
	}
}

TTconcept *TTsentence::getFrameEntry(int slotIndex, const TTconceptNode *conceptNode) const {
	if (!conceptNode)
		conceptNode = &_sentenceConcept;

	return conceptNode->_concepts[slotIndex];
}

TTconcept *TTsentence::getFrameSlot(int slotIndex, const TTconceptNode *conceptNode) const {
	TTconcept *newConcept = new TTconcept();
	TTconcept *concept = getFrameEntry(slotIndex, conceptNode);

	if (concept)
		newConcept->copyFrom(concept);

	if (!newConcept->isValid()) {
		delete newConcept;
		newConcept = nullptr;
	}

	return newConcept;
}

bool TTsentence::isFrameSlotClass(int slotIndex, WordClass wordClass, const TTconceptNode *conceptNode) const {
	TTconcept *concept = getFrameEntry(slotIndex, conceptNode);
	if (concept && concept->_wordP) {
		return concept->_wordP->isClass(wordClass);
	} else {
		return false;
	}
}

int TTsentence::is18(int val, const TTconceptNode *node) const {
	if (!node)
		node = &_sentenceConcept;
	return node->_field18 == val;
}

int TTsentence::is1C(int val, const TTconceptNode *node) const {
	if (!node)
		node = &_sentenceConcept;
	return node->_field1C == val;
}

bool TTsentence::isConcept34(int slotIndex, const TTconceptNode *node) const {
	TTconcept *concept = getFrameEntry(slotIndex, node);
	return concept && concept->getState();
}

bool TTsentence::localWord(const char *str) const {
	CScriptHandler &scriptHandler = *g_vm->_exeResources._owner;
	bool foundMatch = false;
	static const char *const ARTICLES_EN[11] = {
		"it", "that", "he", "she", "him", "her", "them", "they", "those", "1", "thing"
	};
	static const char *const ARTICLES_DE[9] = {
		"es", "das", "er", "ihn", "ihm", "ihnen", "diese", "man", "ding"
	};

	if (scriptHandler._concept1P) {
		TTstring s = scriptHandler._concept1P->getText();
		if (s == str)
			foundMatch = true;
	} else if (scriptHandler._concept2P) {
			TTstring s = scriptHandler._concept2P->getText();
			if (s == str)
				foundMatch = true;
	}

	VocabMode mode = g_vm->_exeResources.getVocabMode();
	bool result = false;

	for (TTsentenceNode *nodeP = _nodesP; nodeP && !result;
			nodeP = dynamic_cast<TTsentenceNode *>(nodeP->_nextP)) {
		TTsynonym syn;
		if (!nodeP->_wordP)
			continue;

		const TTstring wordStr = nodeP->_wordP->_text;
		if ((g_language == Common::DE_DEU || mode == VOCAB_MODE_EN) && wordStr == str) {
			result = true;
		} else if (nodeP->_wordP->findSynByName(str, &syn, mode)) {
			result = true;
		} else if (foundMatch) {
			result = false;
			for (int idx = 0; idx < TRANSLATE(11, 9) && !result; ++idx) {
				result = wordStr == TRANSLATE(ARTICLES_EN[idx], ARTICLES_DE[idx]);
			}
		}
	}

	return result;
}

bool TTsentence::contains(const char *str) const {
	return _initialLine.contains(str) || _normalizedLine.contains(str);
}

} // End of namespace Titanic
