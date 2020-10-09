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

#include "engines/stark/resources/dialog.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/item.h"
#include "engines/stark/resources/knowledge.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/speech.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

Dialog::~Dialog() {
}

Dialog::Dialog(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_character(0),
		_hasAskAbout(0) {
	_type = TYPE;
}

void Dialog::readData(Formats::XRCReadStream *stream) {
	Object::readData(stream);

	_hasAskAbout = stream->readUint32LE();
	_character = stream->readUint32LE();

	uint32 numTopics = stream->readUint32LE();
	for (uint32 i = 0; i < numTopics; i++) {
		Topic topic;
		topic._removeOnceDepleted = stream->readBool();

		uint32 numReplies = stream->readUint32LE();
		for (uint j = 0; j < numReplies; j++) {
			Reply reply;

			reply._conditionType = stream->readUint32LE();
			reply._conditionReference = stream->readResourceReference();
			reply._conditionScriptReference = stream->readResourceReference();
			reply._conditionReversed = stream->readUint32LE();
			reply._field_88 = stream->readUint32LE();
			reply._minChapter = stream->readUint32LE();
			reply._maxChapter = stream->readUint32LE();
			reply._noCaption = stream->readUint32LE();
			reply._nextDialogIndex = stream->readSint32LE();
			reply._nextScriptReference = stream->readResourceReference();

			uint32 numLines = stream->readUint32LE();
			for (uint k = 0; k < numLines; k++) {
				reply._lines.push_back(stream->readResourceReference());
				reply._lines.push_back(stream->readResourceReference());
			}

			topic._replies.push_back(reply);
		}

		_topics.push_back(topic);
	}
}

void Dialog::saveLoad(ResourceSerializer *serializer) {
	for (uint i = 0; i < _topics.size(); i++) {
		serializer->syncAsSint32LE(_topics[i]._currentReplyIndex);
	}
}

void Dialog::printData() {
	Object::printData();

	debug("character: %d", _character);
	debug("hasAskAbout: %d", _hasAskAbout);

	for (uint32 i = 0; i < _topics.size(); i++) {
		Topic &topic = _topics[i];
		debug("topic[%d].removeOnceDepleted: %d", i, topic._removeOnceDepleted);

		for (uint j = 0; j < topic._replies.size(); j++) {
			Reply reply = topic._replies[j];

			debug("topic[%d].reply[%d].conditionType: %d", i, j, reply._conditionType);
			debug("topic[%d].reply[%d].conditionReference: %s", i, j, reply._conditionReference.describe().c_str());
			debug("topic[%d].reply[%d].conditionScriptReference: %s", i, j, reply._conditionScriptReference.describe().c_str());
			debug("topic[%d].reply[%d].conditionReversed: %d", i, j, reply._conditionReversed);
			debug("topic[%d].reply[%d].minChapter: %d", i, j, reply._minChapter);
			debug("topic[%d].reply[%d].maxChapter: %d", i, j, reply._maxChapter);
			debug("topic[%d].reply[%d].noCaption: %d", i, j, reply._noCaption);
			debug("topic[%d].reply[%d].field_88: %d", i, j, reply._field_88);
			debug("topic[%d].reply[%d].nextScriptReference: %s", i, j, reply._nextScriptReference.describe().c_str());
			debug("topic[%d].reply[%d].nextDialogIndex: %d", i, j, reply._nextDialogIndex);

			for (uint k = 0; k < reply._lines.size(); k++) {
				debug("topic[%d].reply[%d].line[%d]: %s", i, j, k, reply._lines[k].describe().c_str());
			}
		}
	}
}

Dialog::TopicArray Dialog::listAvailableTopics() {
	Common::Array<Dialog::Topic *> topics;

	for (uint i = 0; i < _topics.size(); i++) {
		Topic *topic = &_topics[i];
		if (topic->getNextReplyIndex() < 0) {
			continue;
		}

		topics.push_back(topic);
	}

	return topics;
}

Dialog::Topic::Topic() :
		_removeOnceDepleted(true),
		_currentReplyIndex(-1) {
}

int32 Dialog::Topic::getNextReplyIndex() const {
	uint32 nextIndex = _currentReplyIndex + 1;

	if (nextIndex >= _replies.size()) {
		// No more replies ...
		if (_removeOnceDepleted || _replies.empty()) {
			// Don't show this topic
			return -1;
		} else {
			// Repeat the last reply
			nextIndex = _replies.size() - 1;
		}
	}

	uint32 currentChapter = StarkGlobal->getCurrentChapter();

	// Skip replies from previous chapters
	while (nextIndex < _replies.size() && _replies[nextIndex]._maxChapter < currentChapter) {
		nextIndex++;
	}

	if (nextIndex >= _replies.size()) {
		// No more replies ...
		if (_removeOnceDepleted || _replies.empty()) {
			// Don't show this topic
			return -1;
		} else {
			// Repeat the last reply
			nextIndex = _replies.size() - 1;
		}
	}

	// Chapter check
	const Reply &reply = _replies[nextIndex];
	if (currentChapter < reply._minChapter || currentChapter >= reply._maxChapter) {
		return -1;
	}

	return nextIndex;
}

Dialog::Reply *Dialog::Topic::startReply(uint32 index) {
	_currentReplyIndex = index;

	Reply *reply = &_replies[_currentReplyIndex];
	reply->start();

	return reply;
}

Dialog::Reply *Dialog::Topic::getReply(uint32 index) {
	return &_replies[index];
}

Common::String Dialog::Topic::getCaption() const {
	int32 replyIndex = getNextReplyIndex();
	if (replyIndex < 0) {
		error("Trying to obtain the caption of a depleted dialog topic.");
	}

	const Reply &reply = _replies[replyIndex];

	if (reply._lines.empty()) {
		error("Trying to obtain the caption of a reply with no lines.");
	}

	Speech *speech = reply._lines[0].resolve<Speech>();
	if (speech) {
		return speech->getPhrase();
	} else {
		return "No Caption";
	}
}


Dialog::Reply::Reply() :
		_conditionReversed(0),
		_field_88(0),
		_minChapter(0),
		_maxChapter(999),
		_conditionType(0),
		_noCaption(0),
		_nextDialogIndex(-1),
		_nextSpeechIndex(-1) {
}

void Dialog::Reply::start() {
	if (_noCaption) {
		_nextSpeechIndex = -1;
	} else {
		// Skip the first line when it is a caption
		_nextSpeechIndex = 0;
	}

	goToNextLine();
}

void Dialog::Reply::goToNextLine() {
	_nextSpeechIndex++;
	while ((uint32)_nextSpeechIndex < _lines.size() && _lines[_nextSpeechIndex].empty()) {
		_nextSpeechIndex++;
	}

	if ((uint32)_nextSpeechIndex >= _lines.size()) {
		_nextSpeechIndex = -2; // No more lines
	}
}

Speech *Dialog::Reply::getCurrentSpeech() {
	if (_nextSpeechIndex < 0) {
		return nullptr;
	}

	return _lines[_nextSpeechIndex].resolve<Speech>();
}

bool Dialog::Reply::checkCondition() const {
	bool result;

	switch (_conditionType) {
		case kConditionTypeAlways:
			result = true;
			break;
		case kConditionTypeNoOtherOptions:
			result = true; // Will be removed from to the options later if some other options are available
			break;
		case kConditionTypeHasItem: {
			Item *item = _conditionReference.resolve<Item>();
			result = item->isEnabled();
			break;
		}
		case kConditionTypeCheckValue4:
		case kConditionTypeCheckValue5: {
			Knowledge *condition = _conditionReference.resolve<Knowledge>();
			result = condition->getBooleanValue();
			break;
		}
		case kConditionTypeRunScriptCheckValue: {
			Script *conditionScript = _conditionScriptReference.resolve<Script>();
			conditionScript->execute(Resources::Script::kCallModeDialogAnswer);

			Knowledge *condition = _conditionReference.resolve<Knowledge>();
			result = condition->getBooleanValue();
			break;
		}
		default:
			warning("Unimplemented dialog reply condition %d", _conditionType);
			result = true;
			break;
	}

	if (_conditionReversed && (_conditionType == kConditionTypeHasItem
			|| _conditionType == kConditionTypeCheckValue4
			|| _conditionType == kConditionTypeCheckValue5
			|| _conditionType == kConditionTypeRunScriptCheckValue)) {
		result = !result;
	}

	return result;
}

bool Dialog::Reply::isLastOnly() const {
	return _conditionType == kConditionTypeNoOtherOptions;
}

Dialog *Dialog::getNextDialog(Dialog::Reply *reply) {
	if (reply->_nextDialogIndex < 0) {
		return nullptr;
	}

	return _parent->findChildWithIndex<Dialog>(reply->_nextDialogIndex);
}

Script *Dialog::getNextScript(Dialog::Reply *reply) {
	if (reply->_nextScriptReference.empty()) {
		return nullptr;
	}

	return reply->_nextScriptReference.resolve<Script>();
}

Common::String Dialog::getDiaryTitle() const {
	return _parent->getName();
}

int32 Dialog::getCharacter() const {
	return _character;
}

} // End of namespace Resources
} // End of namespace Stark
