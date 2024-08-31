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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_named_object_indexer.h"
#include "qdengine/qdcore/qd_condition_object_reference.h"


namespace QDEngine {

qdConditionObjectReference::qdConditionObjectReference() : _object(NULL) {
}

qdConditionObjectReference::qdConditionObjectReference(const qdConditionObjectReference &ref) :
	_object(ref._object) {
}

qdConditionObjectReference::~qdConditionObjectReference() {
}

qdConditionObjectReference &qdConditionObjectReference::operator = (const qdConditionObjectReference &ref) {
	if (this == &ref) return *this;

	_object = ref._object;

	return *this;
}

void qdConditionObjectReference::set_object(qdNamedObject *p) {
	_object = p;
}

bool qdConditionObjectReference::find_object() {
	return false;
}

bool qdConditionObjectReference::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAMED_OBJECT: {
			qdNamedObjectReference &ref = qdNamedObjectIndexer::instance().add_reference((qdNamedObject *&)_object);
			ref.load_script(&*it);
		}
		break;
		}
	}

	return true;
}

bool qdConditionObjectReference::save_script(Common::WriteStream &fh, int indent, int id) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<condition_object ID=\"%d\">\r\n", id));
	if (_object) {
		qdNamedObjectReference ref(_object);
		ref.save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</condition_object>\r\n");

	return true;
}

} // namespace QDEngine
