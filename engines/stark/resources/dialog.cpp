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
#include "engines/stark/xrcreader.h"

namespace Stark {

Dialog::~Dialog() {
}

Dialog::Dialog(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_character(0),
				_hasAskAbout(0) {
	_type = TYPE;
}

void Dialog::readData(XRCReadStream *stream) {
	Resource::readData(stream);

	_hasAskAbout = stream->readUint32LE();
	_character = stream->readUint32LE();

	uint32 numTopics = stream->readUint32LE();
	for (uint32 i = 0; i < numTopics; i++) {
		Topic topic;
		topic._field_14 = stream->readUint32LE();

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
			reply._field_84 = stream->readUint32LE();
			reply._nextDialogIndex = stream->readUint32LE();
			reply._nextScriptReference = stream->readResourceReference();

			uint32 numLines = stream->readUint32LE();
			for (uint k = 0; k < numLines; k++) {
				Line line;
				line._field_0 = stream->readResourceReference();
				line._field_30 = stream->readResourceReference();
				reply._lines.push_back(line);
			}

			topic._replies.push_back(reply);
		}

		_topics.push_back(topic);
	}
}

void Dialog::printData() {
	Resource::printData();

	debug("character: %d", _character);
	debug("hasAskAbout: %d", _hasAskAbout);

	for (uint32 i = 0; i < _topics.size(); i++) {
		Topic &topic = _topics[i];
		debug("topic[%d].field_14: %d", i, topic._field_14);

		for (uint j = 0; j < topic._replies.size(); j++) {
			Reply reply = topic._replies[j];

			debug("topic[%d].reply[%d].conditionType: %d", i, j, reply._conditionType);
			debug("topic[%d].reply[%d].conditionReference: %s", i, j, reply._conditionReference.describe().c_str());
			debug("topic[%d].reply[%d].conditionScriptReference: %s", i, j, reply._conditionScriptReference.describe().c_str());
			debug("topic[%d].reply[%d].conditionReversed: %d", i, j, reply._conditionReversed);
			debug("topic[%d].reply[%d].minChapter: %d", i, j, reply._minChapter);
			debug("topic[%d].reply[%d].maxChapter: %d", i, j, reply._maxChapter);
			debug("topic[%d].reply[%d].field_84: %d", i, j, reply._field_84);
			debug("topic[%d].reply[%d].field_88: %d", i, j, reply._field_88);
			debug("topic[%d].reply[%d].nextScriptReference: %s", i, j, reply._nextScriptReference.describe().c_str());
			debug("topic[%d].reply[%d].nextDialogIndex: %d", i, j, reply._nextDialogIndex);

			for (uint k = 0; k < reply._lines.size(); k++) {
				Line line = reply._lines[k];
				debug("topic[%d].reply[%d].line[%d].field_0: %s", i, j, k, line._field_0.describe().c_str());
				debug("topic[%d].reply[%d].line[%d].field_30: %s", i, j, k, line._field_30.describe().c_str());
			}
		}
	}
}

} // End of namespace Stark
