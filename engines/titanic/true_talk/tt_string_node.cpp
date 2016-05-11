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

TTstringNode::~TTstringNode() {
	detach();
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

void TTstringNode::initialize(TTstringNode *oldNode) {
	_mode = oldNode->_mode;
	_field14 = oldNode->_field14;

	if (_string.isValid()) {
		_field1C = 0;
	} else {
		_field1C = 11;
		warning("TTstringNode::initialize has bad subobj");
	}

	delete oldNode;
}

TTstringNode *TTstringNode::scan(TTstringNode *start, const TTString &str, int mode) {
	for (; start; start = start->_pNext) {
		if (start->_mode == mode || (mode == 3 && start->_mode < 3)) {
			if (!strcmp(start->_string.c_str(), str.c_str()))
				start;
		}
	}

	return nullptr;
}

void TTstringNode::addNode(TTstringNode *newNode) {
	TTstringNode *tail = getTail();
	tail->_pNext = newNode;
	newNode->_pPrior = this;
}

void TTstringNode::detach() {
	if (_pPrior)
		_pPrior->_pNext = _pNext;

	if (_pNext)
		_pNext->_pPrior = _pPrior;
}

TTstringNode *TTstringNode::getTail() {
	if (_pNext == nullptr)
		return this;
	
	TTstringNode *node = _pNext;
	while (node->_pNext)
		node = node->_pNext;

	return node;
}

/*------------------------------------------------------------------------*/

TTsynonymNode::TTsynonymNode() : TTstringNode() {
}

TTsynonymNode::TTsynonymNode(const TTstringNode *src) {
	_string = src->_string;
	initialize(src->_mode);
	_field14 = src->_field14;
}

TTsynonymNode::TTsynonymNode(int mode, const char *str, int val2) :
		TTstringNode() {
	_string = str;
	initialize(mode);
	_field14 = val2;
}

TTsynonymNode *TTsynonymNode::copy(TTstringNode *src) {
	if (src->_field1C) {
		_field1C = 5;
		return this;
	} else {
		_field1C = 0;
		if (src == this)
			return this;

		_string = src->_string;
		TTsynonymNode *newNode = new TTsynonymNode(src);
		initialize(newNode);

		return this;
	}
}

} // End of namespace Titanic
