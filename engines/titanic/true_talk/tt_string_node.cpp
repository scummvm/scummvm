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

TTstringNode::TTstringNode() : _pPrior(nullptr), _pNext(nullptr),
		_field14(0), _mode(0), _field1C(0) {
}

void TTstringNode::initialize(int mode) {
	_mode = mode;
	_field14 = 0;

	if (_string.isValid()) {
		_field1C = 0;
	} else {
		_field1C = 11;
		warning("TTstringNode::initialize has bad subobj");
	}
}

void TTstringNode::addNode(TTstringNode *newNode) {
	TTstringNode *tail = getTail();
	tail->_pNext = newNode;
	newNode->_pPrior = this;
}

TTstringNode *TTstringNode::getTail() const {
	if (_pNext == nullptr)
		return nullptr;
	
	TTstringNode *node = _pNext;
	while (node->_pNext)
		node = node->_pNext;

	return node;
}

/*------------------------------------------------------------------------*/

TTsynonymNode::TTsynonymNode() : TTstringNode() {
}

TTsynonymNode::TTsynonymNode(int mode, const char *str, int val2) :
		TTstringNode() {
	_string = str;
	initialize(mode);
	_field14 = val2;
}


} // End of namespace Titanic
