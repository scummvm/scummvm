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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/xml_tag.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/qdcore/qd_game_dispatcher.h"
#include "qdengine/core/qdcore/qd_named_object_indexer.h"
#include "qdengine/core/qdcore/qd_condition_object_reference.h"


namespace QDEngine {


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdConditionObjectReference::qdConditionObjectReference() : object_(NULL) {
}

qdConditionObjectReference::qdConditionObjectReference(const qdConditionObjectReference &ref) :
#ifdef _QUEST_EDITOR
	object_reference_(ref.object_reference_),
#endif
	object_(ref.object_) {
}

qdConditionObjectReference::~qdConditionObjectReference() {
}

qdConditionObjectReference &qdConditionObjectReference::operator = (const qdConditionObjectReference &ref) {
	if (this == &ref) return *this;

#ifdef _QUEST_EDITOR
	object_reference_ = ref.object_reference_;
#endif

	object_ = ref.object_;

	return *this;
}

void qdConditionObjectReference::set_object(const qdNamedObject *p) {
	object_ = p;
}

bool qdConditionObjectReference::find_object() {
#ifdef _QUEST_EDITOR
	if (object_reference_.is_empty())
		return false;

	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
		object_ = dp -> get_named_object(&object_reference_);

		if (object_)
			return true;
	}
#endif
	return false;
}

bool qdConditionObjectReference::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it) {
		switch (it -> ID()) {
		case QDSCR_NAMED_OBJECT: {
#ifndef _QUEST_EDITOR
			qdNamedObjectReference &ref = qdNamedObjectIndexer::instance().add_reference((qdNamedObject *&)object_);
			ref.load_script(&*it);
#else
			object_reference_.load_script(&*it);
#endif
		}
		break;
		}
	}

	return true;
}

bool qdConditionObjectReference::save_script(XStream &fh, int indent, int id) const {
	for (int i = 0; i < indent; i++) fh < "\t";
	fh < "<condition_object ID=\"" <= id < "\">\r\n";

	if (object_) {
		qdNamedObjectReference ref(object_);
		ref.save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) fh < "\t";
	fh < "</condition_object>\r\n";

	return true;
}
} // namespace QDEngine
