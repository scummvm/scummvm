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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_PRIORITYQUEUE_H
#define CRAB_PRIORITYQUEUE_H

#include "crab/common_header.h"

namespace Crab {

//! \brief The open heap used by all cost-based search algorithms.
//!
//! This class template is basically a thin wrapper on top of both the <code>std::deque</code>
//! class template and the <a href="http://www.sgi.com/tech/stl/">STL</a> heap operations.
template<typename Node>
class PriorityQueue {
	Common::Array<Node *> _open;
	bool (*compare)(Node const *, Node const *);

public:
	//! \brief Constructs a new <code>%PriorityQueue</code> that heap-sorts nodes
	//! using the specified comparator.
	explicit PriorityQueue(bool (*)(Node const *, Node const *));

	//! \brief Returns <code>true</code> if the heap contains no nodes,
	//! <code>false</code> otherwise.
	bool empty() const;

	//! \brief Removes all nodes from the heap.
	//!
	//! \post
	//!   - <code>empty()</code>
	void clear();

	//! \brief Returns the number of nodes currently in the heap.
	size_t size() const;

	//! \brief Pushes the specified node onto the heap.
	//!
	//! The heap will maintain the ordering of its nodes during this operation.
	//!
	//! \post
	//!   - <code>! empty()</code>
	void push(Node *node);

	//! \brief Returns the least costly node in the heap.
	//!
	//! \pre
	//!   - <code>! empty()</code>
	Node *front() const;

	//! \brief Removes the least costly node from the heap.
	//!
	//! The heap will maintain the ordering of its nodes during this operation.
	//!
	//! \pre
	//!   - <code>! empty()</code>
	void pop();

	//! \brief Removes all instances of the specified node from the heap.
	void remove(Node const *node);

	//! \brief Enumerates all nodes in the heap so far.
	//!
	//! \param   sorted  the container to which each node will be added.
	//!
	//! \pre
	//!   - There must be no <code>NULL</code> pointers in the heap.
	//! \post
	//!   - All nodes should be sorted by this heap's comparator.
	void enumerate(Common::Array<Node const *> &sorted) const;
};

template<typename Node>
PriorityQueue<Node>::PriorityQueue(bool (*c)(Node const *, Node const *)) : _open(), compare(c) {
}

template<typename Node>
bool PriorityQueue<Node>::empty() const {
	return _open.empty();
}

template<typename Node>
void PriorityQueue<Node>::clear() {
	_open.clear();
}

template<typename Node>
size_t PriorityQueue<Node>::size() const {
	return _open.size();
}

template<typename Node>
void PriorityQueue<Node>::push(Node *node) {
	_open.insert(Common::upperBound(_open.begin(), _open.end(), node, compare), node);
}

template<typename Node>
Node *PriorityQueue<Node>::front() const {
	return _open.back();
}

template<typename Node>
void PriorityQueue<Node>::pop() {
	_open.pop_back();
}

template<typename Node>
void PriorityQueue<Node>::remove(Node const *node) {
	_open.erase(Common::remove(_open.begin(), _open.end(), node), _open.end());
}

template<typename Node>
void PriorityQueue<Node>::enumerate(Common::Array<Node const *> &sorted) const {
	sorted.resize(_open.size());
	Common::copy(_open.begin(), _open.end(), sorted.begin());
}

} // End of namespace Crab

#endif // CRAB_PRIORITYQUEUE_H
