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
#include "titanic/true_talk/tt_node.h"

namespace Titanic {

TTnode::TTnode() : _priorP(nullptr), _nextP(nullptr) {
}

TTnode::~TTnode() {
	detach();
}

void TTnode::addToTail(TTnode *newNode) {
	TTnode *tail = getTail();
	tail->_nextP = newNode;
	newNode->_priorP = this;
}

void TTnode::addToHead(TTnode *newNode) {
	TTnode *head = getHead();
	head->_priorP = newNode;
	newNode->_nextP = head;
}

void TTnode::detach() {
	if (_priorP)
		_priorP->_nextP = _nextP;

	if (_nextP)
		_nextP->_priorP = _priorP;
}

void TTnode::deleteSiblings() {
	// Detach current node from prior one, if there is one
	if (_priorP)
		_priorP->_nextP = nullptr;

	// Iterate through the linked chain of nodes, deleting each in turn
	while (_nextP) {
		TTnode *next = _nextP;
		_nextP = next->_nextP;
		delete next;
	}
}

TTnode *TTnode::getHead() {
	if (_priorP == nullptr)
		return this;

	TTnode *node = _priorP;
	while (node->_priorP)
		node = node->_priorP;

	return node;
}

TTnode *TTnode::getTail() {
	if (_nextP == nullptr)
		return this;

	TTnode *node = _nextP;
	while (node->_nextP)
		node = node->_nextP;

	return node;
}

} // End of namespace Titanic
