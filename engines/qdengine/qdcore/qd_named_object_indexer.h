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

#ifndef QDENGINE_QDCORE_QD_NAMED_OBJECT_INDEXER_H
#define QDENGINE_QDCORE_QD_NAMED_OBJECT_INDEXER_H

#include "qdengine/qdcore/qd_named_object_reference.h"


namespace QDEngine {

class qdNamedObjectIndexer {
public:
	qdNamedObjectIndexer();
	~qdNamedObjectIndexer();

	void resolve_references();
	qdNamedObjectReference &add_reference(qdNamedObject *&p);

	void clear();

	static qdNamedObjectIndexer &instance();

private:

	class qdNamedObjectReferenceLink {
	public:
		qdNamedObjectReferenceLink(qdNamedObject *&p) : _object(p) { }
		~qdNamedObjectReferenceLink() { }

		bool resolve();

		qdNamedObjectReference &reference() {
			return _reference;
		}

	private:

		qdNamedObjectReference _reference;
		qdNamedObject *&_object;
	};

	typedef Std::list<qdNamedObjectReferenceLink> link_container_t;
	link_container_t _links;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_NAMED_OBJECT_INDEXER_H
