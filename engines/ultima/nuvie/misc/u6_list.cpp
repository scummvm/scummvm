/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_llist.h"

namespace Ultima {
namespace Nuvie {

void retainU6Link(U6Link *link) {
	if (link)
		link->ref_count++;
}

void releaseU6Link(U6Link *link) {
	if (link) {
		link->ref_count--;
		if (link->ref_count == 0)
			delete link;
	}
}

//use locally to cleanup the pointers when unlinking from list.
inline void deleteU6Link(U6Link *link) {

	if (link->ref_count == 1)
		delete link;
	else {
		link->ref_count--;
		link->data = nullptr;
		link->prev = link->next = nullptr;
	}
}

U6LList::U6LList() : head(nullptr), tail(nullptr) {
}

U6LList::~U6LList() {
	removeAll();
}

bool U6LList::add(void *data) {
	U6Link *link;

	link = new U6Link;
	if (link == nullptr)
		return false;

	if (tail == nullptr)
		head = tail = link;
	else {
		link->prev = tail;
		tail->next = link;
		tail = link;
	}

	link->data = data;

	return true;
}

bool U6LList::addAtPos(uint32 pos, void *data) {
	uint32 i;
	U6Link *link, *prev, *new_link;

	new_link = new U6Link;
	if (new_link == nullptr)
		return false;

	new_link->data = data;

	if (pos == 0 || head == nullptr) { // pos at head or list empty
		if (head != nullptr)
			head->prev = new_link;
		new_link->next = head;
		head = new_link;
		if (tail == nullptr)
			tail = head;
	} else {
		prev = nullptr;
		for (link = head, i = 0; link != nullptr && i < pos; i++) {
			prev = link;
			link = link->next;
		}

		if (prev == tail) {
			new_link->prev = tail;
			tail->next = new_link;
			tail = new_link;
		} else {
			new_link->prev = prev;
			new_link->next = prev->next;
			prev->next = new_link;
		}
	}

	return true;
}

uint32 U6LList::findPos(void *data) {
	U6Link *link;
	uint32 pos;

	for (pos = 0, link = start(); link != nullptr; link = link->next, pos++) {
		if (link->data == data)
			return pos;
	}

	return 0;
}

//replace the first instance of old with new.
bool U6LList::replace(void *old_data, void *new_data) {
	U6Link *link;

	for (link = start(); link != nullptr; link = link->next) {
		if (link->data == old_data) {
			link->data = new_data;
			return true;
		}
	}

	return false;
}

bool U6LList::remove(void *data) {
	U6Link *link;
	U6Link *prev;

	if (head == nullptr)
		return false;

	if (head->data == data) { // remove head
		link = head;
		head = head->next;
		if (head == nullptr) // empty list
			tail = nullptr;
		else
			head->prev = nullptr;

		deleteU6Link(link);

		return true;
	}

	for (link = prev = head; link != nullptr;) {
		if (link->data == data) {
			prev->next = link->next;

			if (link == tail)
				tail = prev;

			deleteU6Link(link);

			if (link != prev && prev->next) {
				link = prev->next;
				link->prev = prev;
			}

			return true;
		}

		prev = link;
		link = link->next;
	}

	return false;
}

bool U6LList::removeAll() {
	U6Link *tmp_link, *link;

	for (link = head; link != nullptr;) {
		tmp_link = link;
		link = link->next;

		deleteU6Link(tmp_link);
	}

	head = nullptr;
	tail = nullptr;

	return true;
}



uint32 U6LList::count() const {
	uint32 i;
	U6Link *link;

	for (i = 0, link = head; link != nullptr; link = link->next) {
		i++;
	}

	return i;
}

U6Link *U6LList::start() {
	return head;
}

U6Link *U6LList::end() {
	return tail;
}

const U6Link *U6LList::start() const {
	return head;
}

const U6Link *U6LList::end() const {
	return tail;
}

U6Link *U6LList::gotoPos(uint32 pos) {
	U6Link *link;
	uint32 i;

	for (link = head, i = 0; link != nullptr && i < pos; i++)
		link = link->next;

	return link;
}

} // End of namespace Nuvie
} // End of namespace Ultima
