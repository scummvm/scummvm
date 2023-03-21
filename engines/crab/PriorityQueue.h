#pragma once

#include <vector>
#include <deque>
#include <algorithm>

//! \brief The open heap used by all cost-based search algorithms.
//!
//! This class template is basically a thin wrapper on top of both the <code>std::deque</code>
//! class template and the <a href="http://www.sgi.com/tech/stl/">STL</a> heap operations.
template <typename Node>
class PriorityQueue
{
	std::deque<Node*> open;
	bool(*compare)(Node const*, Node const*);

public:
	//! \brief Constructs a new <code>%PriorityQueue</code> that heap-sorts nodes
	//! using the specified comparator.
	explicit PriorityQueue(bool(*)(Node const*, Node const*));

	//! \brief Returns <code>true</code> if the heap contains no nodes,
	//! <code>false</code> otherwise.
	bool empty() const;

	//! \brief Removes all nodes from the heap.
	//!
	//! \post
	//!   - <code>empty()</code>
	void clear();

	//! \brief Returns the number of nodes currently in the heap.
	std::size_t size() const;

	//! \brief Pushes the specified node onto the heap.
	//!
	//! The heap will maintain the ordering of its nodes during this operation.
	//!
	//! \post
	//!   - <code>! empty()</code>
	void push(Node* node);

	//! \brief Returns the least costly node in the heap.
	//!
	//! \pre
	//!   - <code>! empty()</code>
	Node* front() const;

	//! \brief Removes the least costly node from the heap.
	//!
	//! The heap will maintain the ordering of its nodes during this operation.
	//!
	//! \pre
	//!   - <code>! empty()</code>
	void pop();

	//! \brief Removes all instances of the specified node from the heap.
	void remove(Node const* node);

	//! \brief Enumerates all nodes in the heap so far.
	//!
	//! \param   sorted  the container to which each node will be added.
	//!
	//! \pre
	//!   - There must be no <code>NULL</code> pointers in the heap.
	//! \post
	//!   - All nodes should be sorted by this heap's comparator.
	void enumerate(std::vector<Node const*>& sorted) const;
};

template <typename Node>
PriorityQueue<Node>::PriorityQueue(bool(*c)(Node const*, Node const*)) : open(), compare(c)
{
}

template <typename Node>
bool PriorityQueue<Node>::empty() const
{
	return open.empty();
}

template <typename Node>
void PriorityQueue<Node>::clear()
{
	open.clear();
}

template <typename Node>
std::size_t PriorityQueue<Node>::size() const
{
	return open.size();
}

template <typename Node>
void PriorityQueue<Node>::push(Node* node)
{
	open.insert(std::upper_bound(open.begin(), open.end(), node, compare), node);
}

template <typename Node>
Node* PriorityQueue<Node>::front() const
{
	return open.back();
}

template <typename Node>
void PriorityQueue<Node>::pop()
{
	open.pop_back();
}

template <typename Node>
void PriorityQueue<Node>::remove(Node const* node)
{
	open.erase(std::remove(open.begin(), open.end(), node), open.end());
}

template <typename Node>
void PriorityQueue<Node>::enumerate(std::vector<Node const*>& sorted) const
{
	sorted.resize(open.size());
	std::copy(open.begin(), open.end(), sorted.begin());
}
