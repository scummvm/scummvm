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

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/qdcore/qd_rnd.h"
#include "qdengine/qdcore/qd_condition.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

namespace Common {
class WriteStream;
}

namespace QDEngine {

bool qdCondition::_successful_click = false;
bool qdCondition::_successful_object_click = false;

qdCondition::qdCondition() : _type(CONDITION_FALSE), _is_inversed(false), _is_in_group(false) {
}

qdCondition::qdCondition(qdCondition::ConditionType tp) : _is_inversed(false), _is_in_group(false) {
	set_type(tp);
}

qdCondition::qdCondition(const qdCondition &cnd) : _type(cnd._type),
	_owner(cnd._owner),
	_data(cnd._data),
	_objects(cnd._objects),
	_is_inversed(cnd._is_inversed),
	_is_in_group(false) {
}

qdCondition &qdCondition::operator = (const qdCondition &cnd) {
	if (this == &cnd) return *this;

	_type = cnd._type;
	_owner = cnd._owner;

	_data = cnd._data;
	_objects = cnd._objects;

	_is_inversed = cnd._is_inversed;

	return *this;
}

qdCondition::~qdCondition() {
}

void qdCondition::set_type(ConditionType tp) {
	_type = tp;

	switch (_type) {
	case CONDITION_TRUE:
	case CONDITION_FALSE:
		break;
	case CONDITION_MOUSE_CLICK:
	case CONDITION_PERSONAGE_ACTIVE:
	case CONDITION_MOUSE_ZONE_CLICK:
		_data.resize(1);
		_objects.resize(1);
		init_data(0, qdConditionData::DATA_STRING);
		break;
	case CONDITION_MOUSE_OBJECT_CLICK:
		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_OBJECT_IN_ZONE:
		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_PERSONAGE_WALK_DIRECTION:
	case CONDITION_PERSONAGE_STATIC_DIRECTION:
		_data.resize(2);
		_objects.resize(1);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_FLOAT, 1);
		break;
	case CONDITION_TIMER:
		_data.resize(2);
		init_data(0, qdConditionData::DATA_FLOAT, 2);
		init_data(1, qdConditionData::DATA_INT, 2);
		break;
	case CONDITION_MOUSE_DIALOG_CLICK:
		break;
	case CONDITION_MINIGAME_STATE:
		_data.resize(2);
		_objects.resize(1);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_OBJECT_STATE:
	case CONDITION_OBJECT_PREV_STATE:
		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_OBJECT_NOT_IN_STATE:
		inverse();
		_type = CONDITION_OBJECT_STATE;

		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_MOUSE_OBJECT_ZONE_CLICK:
		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_OBJECT_STATE_WAS_ACTIVATED:
		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_OBJECT_STATE_WAS_NOT_ACTIVATED:
		inverse();
		_type = CONDITION_OBJECT_STATE_WAS_ACTIVATED;

		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_OBJECTS_DISTANCE:
		_data.resize(3);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		init_data(2, qdConditionData::DATA_FLOAT, 1);
		break;
	case CONDITION_OBJECT_STATE_WAITING:
		_data.resize(2);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		break;
	case CONDITION_OBJECT_STATE_ANIMATION_PHASE:
		_data.resize(3);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_STRING);
		init_data(1, qdConditionData::DATA_STRING);
		init_data(2, qdConditionData::DATA_FLOAT, 2);
		break;
	case CONDITION_STATE_TIME_GREATER_THAN_VALUE:
		_data.resize(1);
		_objects.resize(1);
		init_data(0, qdConditionData::DATA_FLOAT, 1);
		break;
	case CONDITION_STATE_TIME_GREATER_THAN_STATE_TIME:
		_objects.resize(2);
		break;
	case CONDITION_STATE_TIME_IN_INTERVAL:
		_data.resize(1);
		_objects.resize(1);
		init_data(0, qdConditionData::DATA_FLOAT, 2);
		break;
	case CONDITION_COUNTER_GREATER_THAN_VALUE:
	case CONDITION_COUNTER_LESS_THAN_VALUE:
		_data.resize(1);
		_objects.resize(1);
		init_data(0, qdConditionData::DATA_INT, 1);
		break;
	case CONDITION_COUNTER_GREATER_THAN_COUNTER:
		_objects.resize(2);
		break;
	case CONDITION_COUNTER_IN_INTERVAL:
		_data.resize(1);
		_objects.resize(1);
		init_data(0, qdConditionData::DATA_INT, 2);
		break;
	case CONDITION_OBJECT_ON_PERSONAGE_WAY:
		_data.resize(1);
		_objects.resize(2);
		init_data(0, qdConditionData::DATA_FLOAT, 1);
		break;
	case CONDITION_KEYPRESS:
		_data.resize(1);
		init_data(0, qdConditionData::DATA_INT, 1);
		break;
	case CONDITION_ANY_PERSONAGE_IN_ZONE:
		_objects.resize(1);
		break;
	case CONDITION_MOUSE_RIGHT_CLICK:
		_objects.resize(1);
		break;
	case CONDITION_MOUSE_RIGHT_OBJECT_CLICK:
		_objects.resize(2);
		break;
	case CONDITION_MOUSE_RIGHT_ZONE_CLICK:
		_objects.resize(1);
		break;
	case CONDITION_MOUSE_RIGHT_OBJECT_ZONE_CLICK:
		_objects.resize(2);
		break;
	case CONDITION_OBJECT_HIDDEN:
		_objects.resize(1);
		break;
	case CONDITION_MOUSE_HOVER:
		_objects.resize(1);
		break;
	case CONDITION_MOUSE_OBJECT_HOVER:
		_objects.resize(2);
		break;
	case CONDITION_MOUSE_HOVER_ZONE:
		_objects.resize(1);
		break;
	case CONDITION_MOUSE_OBJECT_HOVER_ZONE:
		_objects.resize(2);
		break;
	case CONDITION_MOUSE_CLICK_FAILED:
	case CONDITION_MOUSE_OBJECT_CLICK_FAILED:
	case CONDITION_MOUSE_CLICK_EVENT:
	case CONDITION_MOUSE_RIGHT_CLICK_EVENT:
		break;
	case CONDITION_MOUSE_OBJECT_CLICK_EVENT:
	case CONDITION_MOUSE_RIGHT_OBJECT_CLICK_EVENT:
	case CONDITION_MOUSE_STATE_PHRASE_CLICK:
		_objects.resize(1);
		break;
	case CONDITION_OBJECT_IS_CLOSER:
		_objects.resize(3);
		break;
	case CONDITION_ANIMATED_OBJECT_IDLE_GREATER_THAN_VALUE:
		_objects.resize(1);
		_data.resize(1);
		init_data(0, qdConditionData::DATA_INT, 1);
		break;
	case CONDITION_ANIMATED_OBJECTS_INTERSECTIONAL_BOUNDS:
		_objects.resize(2);
		break;
	}
}

bool qdCondition::put_value(int idx, const char *str) {
	assert(idx >= 0 && idx < (int)_data.size());
	return _data[idx].put_string(str);
}

bool qdCondition::put_value(int idx, int val, int val_index) {
	assert(idx >= 0 && idx < (int)_data.size());
	return _data[idx].put_int(val, val_index);
}

bool qdCondition::put_value(int idx, float val, int val_index) {
	assert(idx >= 0 && idx < (int)_data.size());
	return _data[idx].put_float(val, val_index);
}

bool qdCondition::get_value(int idx, const char *&str) const {
	assert(idx >= 0 && idx < (int)_data.size());

	if (_data[idx].get_string()) {
		str = _data[idx].get_string();
		return true;
	}

	return false;
}

bool qdCondition::get_value(int idx, int &val, int val_index) const {
	assert(idx >= 0 && idx < (int)_data.size());
	val = _data[idx].get_int(val_index);

	return true;
}

bool qdCondition::get_value(int idx, float &val, int val_index) const {
	assert(idx >= 0 && idx < (int)_data.size());
	val = _data[idx].get_float(val_index);

	return true;
}

bool qdCondition::load_script(const xml::tag *p) {
	int data_idx = 0;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_CONDITION_DATA_INT:
		case QDSCR_CONDITION_DATA_FLOAT:
		case QDSCR_CONDITION_DATA_STRING:
			if (data_idx < (int)_data.size())
				_data[data_idx++].load_script(&*it);
			break;
		case QDSCR_CONDITION_INVERSE:
			if (xml::tag_buffer(*it).get_int())
				inverse(true);
			else
				inverse(false);
			break;
		case QDSCR_CONDITION_OBJECT:
			if (const xml::tag *tp = it->search_subtag(QDSCR_ID)) {
				int object_idx = xml::tag_buffer(*tp).get_int();

				if (object_idx >= 0 && object_idx < (int)_objects.size())
					_objects[object_idx].load_script(&*it);
			}
			break;
		}
	}
	return true;
}

bool qdCondition::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	if (debugChannelSet(-1, kDebugLog)) {
		fh.writeString(Common::String::format("<condition type=\"%s\"", type2str(_type)));
	} else {
		fh.writeString(Common::String::format("<condition type=\"%d\"", _type));
	}

	if (is_inversed()) {
		fh.writeString(" condition_inverse=\"1\"");
	}

	fh.writeString(">\r\n");

	for (auto &it : _data) {
		it.save_script(fh, indent + 1);
	}

	for (uint i = 0; i < _objects.size(); i++) {
		if (_objects[i].object())
			_objects[i].save_script(fh, indent + 1, i);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("</condition>\r\n");
	return true;
}

void qdCondition::quant(float dt) {
	debugC(9, kDebugQuant, "qdCondition::quant(%f)", dt);
	if (_type == CONDITION_TIMER) {
		float period, timer;
		if (!get_value(TIMER_PERIOD, period, 0)) return;
		if (!get_value(TIMER_PERIOD, timer, 1)) return;

		timer += dt;

		put_value(TIMER_PERIOD, timer, 1);

		if (timer >= period) {
			debugC(3, kDebugQuant, "qdCondition::quant() timer >= period");
			timer -= period;
			put_value(TIMER_PERIOD, timer, 1);

			int rnd;
			if (!get_value(TIMER_RND, rnd)) return;

			int state = 1;
			if (rnd && qd_rnd(100 - rnd))
				state = 0;

			put_value(TIMER_RND, state, 1);
		} else
			put_value(TIMER_RND, 0, 1);
	}
}

bool qdCondition::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(5, kDebugSave, "      qdCondition::load_data(): before: %d", (int)fh.pos());
	if (_type == CONDITION_TIMER) {
		int state;
		float timer;

		timer = fh.readFloatLE();
		state = fh.readSint32LE();

		if (!put_value(TIMER_PERIOD, timer, 1)) return false;
		if (!put_value(TIMER_RND, state, 1)) return false;
	}

	debugC(5, kDebugSave, "      qdCondition::load_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdCondition::save_data(Common::WriteStream &fh) const {
	debugC(5, kDebugSave, "      qdCondition::save_data(): before: %d", (int)fh.pos());
	if (_type == CONDITION_TIMER) {
		float timer;
		if (!get_value(TIMER_PERIOD, timer, 1)) {
			return false;
		}

		int state;
		if (!get_value(TIMER_RND, state, 1)) {
			return false;
		}

		fh.writeFloatLE(timer);
		fh.writeSint32LE(state);
	}

	debugC(5, kDebugSave, "      qdCondition::save_data(): after: %d", (int)fh.pos());
	return true;
}

bool qdCondition::check() {
	bool result = false;
	if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
		if (dp->check_condition(this))
			result = !_is_inversed;
		else
			result = _is_inversed;
	}

	if (result) {
		if (is_click_condition())
			_successful_click = true;
		else if (is_object_click_condition())
			_successful_object_click = true;
	}

	return result;
}

bool qdCondition::put_object(int idx, qdNamedObject *obj) {
	assert(idx >= 0 && idx < (int)_objects.size());
	_objects[idx].set_object(obj);
	return true;
}

const qdNamedObject *qdCondition::get_object(int idx) {
	if (idx >= 0 && idx < (int)_objects.size()) {
		if (!_objects[idx].object())
			_objects[idx].find_object();

		return _objects[idx].object();
	}

	return NULL;
}


bool qdCondition::init() {
	if (_type == CONDITION_TIMER) {
		if (!put_value(TIMER_PERIOD, 0.0f, 1)) return false;
		if (!put_value(TIMER_RND, 0, 1)) return false;
	}
	return true;
}

const char *types[] = {
	"CONDITION_TRUE",
	"CONDITION_FALSE",
	"CONDITION_MOUSE_CLICK",
	"CONDITION_MOUSE_OBJECT_CLICK",
	"CONDITION_OBJECT_IN_ZONE",
	"CONDITION_PERSONAGE_WALK_DIRECTION",
	"CONDITION_PERSONAGE_STATIC_DIRECTION",
	"CONDITION_TIMER",
	"CONDITION_MOUSE_DIALOG_CLICK",
	"CONDITION_MINIGAME_STATE",
	"CONDITION_OBJECT_STATE",
	"CONDITION_MOUSE_ZONE_CLICK",
	"CONDITION_MOUSE_OBJECT_ZONE_CLICK",
	"CONDITION_OBJECT_STATE_WAS_ACTIVATED",
	"CONDITION_OBJECT_STATE_WAS_NOT_ACTIVATED",
	"CONDITION_OBJECT_NOT_IN_STATE",
	"CONDITION_OBJECTS_DISTANCE",
	"CONDITION_PERSONAGE_ACTIVE",
	"CONDITION_OBJECT_STATE_WAITING",
	"CONDITION_OBJECT_STATE_ANIMATION_PHASE",
	"CONDITION_OBJECT_PREV_STATE",
	"CONDITION_STATE_TIME_GREATER_THAN_VALUE",
	"CONDITION_STATE_TIME_GREATER_THAN_STATE_TIME",
	"CONDITION_STATE_TIME_IN_INTERVAL",
	"CONDITION_COUNTER_GREATER_THAN_VALUE",
	"CONDITION_COUNTER_LESS_THAN_VALUE",
	"CONDITION_COUNTER_GREATER_THAN_COUNTER",
	"CONDITION_COUNTER_IN_INTERVAL",
	"CONDITION_OBJECT_ON_PERSONAGE_WAY",
	"CONDITION_KEYPRESS",
	"CONDITION_ANY_PERSONAGE_IN_ZONE",
	"CONDITION_MOUSE_RIGHT_CLICK",
	"CONDITION_MOUSE_RIGHT_OBJECT_CLICK",
	"CONDITION_MOUSE_RIGHT_ZONE_CLICK",
	"CONDITION_MOUSE_RIGHT_OBJECT_ZONE_CLICK",
	"CONDITION_OBJECT_HIDDEN",
	"CONDITION_MOUSE_HOVER",
	"CONDITION_MOUSE_OBJECT_HOVER",
	"CONDITION_MOUSE_HOVER_ZONE",
	"CONDITION_MOUSE_OBJECT_HOVER_ZONE",
	"CONDITION_MOUSE_CLICK_FAILED",
	"CONDITION_MOUSE_OBJECT_CLICK_FAILED",
	"CONDITION_MOUSE_CLICK_EVENT",
	"CONDITION_MOUSE_OBJECT_CLICK_EVENT",
	"CONDITION_MOUSE_RIGHT_CLICK_EVENT",
	"CONDITION_MOUSE_RIGHT_OBJECT_CLICK_EVENT",
	"CONDITION_MOUSE_STATE_PHRASE_CLICK",
	"CONDITION_OBJECT_IS_CLOSER",
	"CONDITION_ANIMATED_OBJECT_IDLE_GREATER_THAN_VALUE",
	"CONDITION_ANIMATED_OBJECTS_INTERSECTIONAL_BOUNDS",
};

const char *qdCondition::type2str(uint type) {
	if (type >= ARRAYSIZE(types))
		return "???";

	return types[type];
}

} // namespace QDEngine
