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

#include "common/debug.h"
#include "common/stream.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_named_object_reference.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

int qdNamedObjectReference::_objects_counter = 0;

qdNamedObjectReference::qdNamedObjectReference() {
	_objects_counter++;
}

qdNamedObjectReference::qdNamedObjectReference(int levels, const int *types, const char *const *names) {
	_object_types.reserve(levels);
	_object_names.reserve(levels);

	for (int i = 0; i < num_levels(); i++) {
		_object_names.push_back(names[i]);
		_object_types.push_back(types[i]);
	}

	_objects_counter++;
}

qdNamedObjectReference::qdNamedObjectReference(const qdNamedObjectReference &ref) : _object_types(ref._object_types),
	_object_names(ref._object_names) {
	_objects_counter++;
}

qdNamedObjectReference::qdNamedObjectReference(const qdNamedObject *p) {
	init(p);

	_objects_counter++;
}

qdNamedObjectReference::~qdNamedObjectReference() {
}

qdNamedObjectReference &qdNamedObjectReference::operator = (const qdNamedObjectReference &ref) {
	if (this == &ref) return *this;

	_object_types = ref._object_types;
	_object_names = ref._object_names;

	return *this;
}

bool qdNamedObjectReference::init(const qdNamedObject *p) {
	clear();

	int num_levels = 0;

	const qdNamedObject *obj = p;
	while (obj && obj->named_object_type() != QD_NAMED_OBJECT_DISPATCHER) {
		obj = obj->owner();
		num_levels ++;
	}

	_object_types.reserve(num_levels);
	_object_names.reserve(num_levels);

	for (int i = 0; i < num_levels; i++) {
		obj = p;
		for (int j = 0; j < num_levels - i - 1; j++) {
			obj = obj->owner();
		}
		if (obj->name()) {
			_object_names.push_back(obj->name());
			_object_types.push_back(obj->named_object_type());
		}
	}

	return true;
}

void qdNamedObjectReference::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);
		switch (it->ID()) {
		case QDSCR_SIZE:
			_object_types.reserve(xml::tag_buffer(*it).get_int());
			_object_names.reserve(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAME:
			_object_names.push_back(it->data());
			break;
		case QDSCR_TYPE:
			_object_types.push_back(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAMED_OBJECT_TYPES:
			_object_types.resize(it->data_size());
			_object_names.reserve(it->data_size());
			for (int i = 0; i < it->data_size(); i++)
				_object_types[i] = buf.get_int();
			break;
		}
	}
}

bool qdNamedObjectReference::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<named_object");

	fh.writeString(Common::String::format(" types=\"%d", num_levels()));
	for (int i = 0; i < num_levels(); i++) {
		if (debugChannelSet(-1, kDebugLog))
			fh.writeString(Common::String::format(" %s", objectType2str(_object_types[i])));
		else
			fh.writeString(Common::String::format(" %d", _object_types[i]));
	}
	fh.writeString("\"");
	fh.writeString(">\r\n");

	for (int j = 0; j < num_levels(); j++) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<name>%s</name>\r\n", qdscr_XML_string(_object_names[j].c_str())));
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</named_object>\r\n");
	return true;

}

bool qdNamedObjectReference::load_data(Common::SeekableReadStream &fh, int version) {
	debugC(5, kDebugSave, "      qdNamedObjectReference::load_data before: %d", (int)fh.pos());
	int nlevels = fh.readSint32LE();

	_object_types.resize(nlevels);
	_object_names.resize(nlevels);

	Common::String str;

	for (int i = 0; i < nlevels; i++) {
		int32 type = fh.readSint32LE();
		int32 nameLen = fh.readUint32LE();
		str = fh.readString('\0', nameLen);
		_object_types[i] = type;
		_object_names[i] = str.c_str();
	}

	debugC(5, kDebugSave, "      qdNamedObjectReference::load_data after: %d", (int)fh.pos());
	return true;
}

bool qdNamedObjectReference::save_data(Common::WriteStream &fh) const {
	debugC(5, kDebugSave, "      qdNamedObjectReference::save_data before: %d", (int)fh.pos());
	fh.writeSint32LE(num_levels());

	for (int i = 0; i < num_levels(); i++) {
		fh.writeSint32LE(_object_types[i]);
		fh.writeUint32LE(_object_names[i].size());
		fh.writeString(_object_names[i].c_str());
	}

	debugC(5, kDebugSave, "      qdNamedObjectReference::save_data after: %d", (int)fh.pos());
	return true;
}

qdNamedObject *qdNamedObjectReference::object() const {
	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher())
		return dp->get_named_object(this);

	return NULL;
}

} // namespace QDEngine
