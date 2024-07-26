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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/debug.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/qdcore/qd_named_object_indexer.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

qdNamedObjectIndexer::qdNamedObjectIndexer() {
}

qdNamedObjectIndexer::~qdNamedObjectIndexer() {
}

qdNamedObjectIndexer &qdNamedObjectIndexer::instance() {
	static qdNamedObjectIndexer idx;
	return idx;
}

bool qdNamedObjectIndexer::qdNamedObjectReferenceLink::resolve() {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
		object_ = dp->get_named_object(&reference_);
		if (!object_) {
			debugC(3, kDebugLog, "qdNamedObjectReferenceLink::resolve() failed\n%s", reference_.toString().c_str());
		} else
			return true;
	}

	return false;
}

qdNamedObjectReference &qdNamedObjectIndexer::add_reference(qdNamedObject *&p) {
	links_.push_back(qdNamedObjectReferenceLink(p));
	return links_.back().reference();
}

void qdNamedObjectIndexer::resolve_references() {
	for (link_container_t::iterator it = links_.begin(); it != links_.end(); ++it)
		it->resolve();
}

void qdNamedObjectIndexer::clear() {
#ifdef _QD_DEBUG_ENABLE_
	debugC(3, kDebugLog, "qdNamedObjectIndexer::links count - %d", links_.size());
#endif

	links_.clear();
}
} // namespace QDEngine
