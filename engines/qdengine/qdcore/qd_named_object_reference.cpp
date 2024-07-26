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
#include "common/stream.h"

#include <string>
#include "qdengine/qd_precomp.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_named_object_reference.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

int qdNamedObjectReference::objects_counter_ = 0;

qdNamedObjectReference::qdNamedObjectReference() {
	objects_counter_++;
}

qdNamedObjectReference::qdNamedObjectReference(int levels, const int *types, const char *const *names) {
	object_types_.reserve(levels);
	object_names_.reserve(levels);

	for (int i = 0; i < num_levels(); i ++) {
		object_names_.push_back(names[i]);
		object_types_.push_back(types[i]);
	}

	objects_counter_++;
}

qdNamedObjectReference::qdNamedObjectReference(const qdNamedObjectReference &ref) : object_types_(ref.object_types_),
	object_names_(ref.object_names_) {
	objects_counter_++;
}

qdNamedObjectReference::qdNamedObjectReference(const qdNamedObject *p) {
	init(p);

	objects_counter_++;
}

qdNamedObjectReference::~qdNamedObjectReference() {
}

qdNamedObjectReference &qdNamedObjectReference::operator = (const qdNamedObjectReference &ref) {
	if (this == &ref) return *this;

	object_types_ = ref.object_types_;
	object_names_ = ref.object_names_;

	return *this;
}

bool qdNamedObjectReference::init(const qdNamedObject *p) {
	clear();

	int num_levels = 0;

	const qdNamedObject *obj = p;
	while (obj && obj->named_object_type() != QD_NAMED_OBJECT_DISPATCHER) {
#ifdef _QUEST_EDITOR
		obj = obj->ref_owner();
#else
		obj = obj->owner();
#endif
		num_levels ++;
	}

	object_types_.reserve(num_levels);
	object_names_.reserve(num_levels);

	for (int i = 0; i < num_levels; i ++) {
		obj = p;
		for (int j = 0; j < num_levels - i - 1; j ++) {
#ifdef _QUEST_EDITOR
			obj = obj->ref_owner();
#else
			obj = obj->owner();
#endif
		}
		if (obj->name()) {
			object_names_.push_back(obj->name());
			object_types_.push_back(obj->named_object_type());
		}
	}

	return true;
}

void qdNamedObjectReference::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);
		switch (it->ID()) {
		case QDSCR_SIZE:
			object_types_.reserve(xml::tag_buffer(*it).get_int());
			object_names_.reserve(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAME:
			object_names_.push_back(it->data());
			break;
		case QDSCR_TYPE:
			object_types_.push_back(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAMED_OBJECT_TYPES:
			object_types_.resize(it->data_size());
			object_names_.reserve(it->data_size());
			for (int i = 0; i < it->data_size(); i++)
				object_types_[i] = buf.get_int();
			break;
		}
	}
}

bool qdNamedObjectReference::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<named_object");

	fh.writeString(Common::String::format(" types=\"%d", num_levels()));
	for (int i = 0; i < num_levels(); i++) {
		fh.writeString(Common::String::format(" %d", object_types_[i]));
	}
	fh.writeString("\"");
	fh.writeString(">\r\n");

	for (int j = 0; j < num_levels(); j ++) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<name>%s</name>\r\n", qdscr_XML_string(object_names_[j].c_str())));
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</named_object>\r\n");
	return true;

}

bool qdNamedObjectReference::load_data(Common::SeekableReadStream &fh, int version) {
	debugC(5, kDebugSave, "      qdNamedObjectReference::load_data before: %ld", fh.pos());
	int nlevels = fh.readSint32LE();

	object_types_.resize(nlevels);
	object_names_.resize(nlevels);

	Common::String str;

	for (int i = 0; i < nlevels; i ++) {
		int32 type = fh.readSint32LE();
		int32 nameLen = fh.readUint32LE();
		str = fh.readString('\0', nameLen);
		object_types_[i] = type;
		object_names_[i] = str.c_str();
	}

	debugC(5, kDebugSave, "      qdNamedObjectReference::load_data after: %ld", fh.pos());
	return true;
}

bool qdNamedObjectReference::save_data(Common::SeekableWriteStream &fh) const {
	debugC(5, kDebugSave, "      qdNamedObjectReference::save_data before: %ld", fh.pos());
	fh.writeSint32LE(num_levels());

	for (int i = 0; i < num_levels(); i ++) {
		fh.writeSint32LE(object_types_[i]);
		fh.writeUint32LE(object_names_[i].size());
		fh.writeString(object_names_[i].c_str());
	}

	debugC(5, kDebugSave, "      qdNamedObjectReference::save_data after: %ld", fh.pos());
	return true;
}

qdNamedObject *qdNamedObjectReference::object() const {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher())
		return dp->get_named_object(this);

	return NULL;
}

} // namespace QDEngine
