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
#include "titanic/true_talk/tt_string_node.h"

namespace Titanic {

TTstringNode::TTstringNode() : TTnode(), _file(HANDLE_STDIN),
		_mode(0), _field1C(0) {
}

void TTstringNode::initialize(int mode) {
	_mode = mode;
	_file = HANDLE_STDIN;

	if (_string.isValid()) {
		_field1C = 0;
	} else {
		_field1C = 11;
		warning("TTstringNode::initialize has bad subobj");
	}
}

void TTstringNode::initialize(TTstringNode *oldNode) {
	_mode = oldNode->_mode;
	_file = oldNode->_file;

	if (_string.isValid()) {
		_field1C = 0;
	} else {
		_field1C = 11;
		warning("TTstringNode::initialize has bad subobj");
	}

	delete oldNode;
}

TTstringNode *TTstringNode::findByName(const TTstring &str, VocabMode mode) {
	for (TTstringNode *nodeP = this; nodeP; nodeP = dynamic_cast<TTstringNode *>(nodeP->_nextP)) {
		if (nodeP->_mode == mode || (mode == VOCAB_MODE_EN && nodeP->_mode < 3)) {
			if (nodeP->_string == str)
				return nodeP;
		}
	}

	return nullptr;
}

} // End of namespace Titanic
