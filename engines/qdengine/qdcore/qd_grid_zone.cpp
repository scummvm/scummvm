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

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_object.h"
#include "qdengine/qdcore/qd_grid_zone.h"

namespace QDEngine {

qdGridZone::qdGridZone() : qdContour(CONTOUR_POLYGON),
	_height(0),
	_state(false),
	_initial_state(false),
	_state_on(true),
	_state_off(false),
	_update_timer(0),
	_shadow_alpha(QD_NO_SHADOW_ALPHA),
	_shadow_color(0) {
	_state_on.set_owner(this);
	_state_off.set_owner(this);
}

qdGridZone::qdGridZone(const qdGridZone &gz) : qdNamedObject(gz), qdContour(gz),
	_state(gz._state),
	_initial_state(gz._initial_state),
	_height(gz._height),
	_state_on(gz._state_on),
	_state_off(gz._state_off),
	_update_timer(gz._update_timer),
	_shadow_alpha(gz._shadow_alpha),
	_shadow_color(gz._shadow_color) {
}

qdGridZone::~qdGridZone() {
}

qdGridZone &qdGridZone::operator = (const qdGridZone &gz) {
	if (this == &gz) return *this;

	*static_cast<qdNamedObject *>(this) = gz;
	*static_cast<qdContour *>(this) = gz;

	_state = gz._state;
	_initial_state = gz._initial_state;

	_height = gz._height;

	_state_on = gz._state_on;
	_state_off = gz._state_off;
	_update_timer = gz._update_timer;

	_shadow_alpha = gz._shadow_alpha;
	_shadow_color = gz._shadow_color;

	return *this;
}

bool qdGridZone::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_STATE:
			_initial_state = _state = (xml::tag_buffer(*it).get_int()) ? true : false;
			break;
		case QDSCR_GRID_ZONE_STATE:
			if (const xml::tag * tg = it->search_subtag(QDSCR_STATE)) {
				if (xml::tag_buffer(*tg).get_int())
					_state_on.load_script(&*it);
				else
					_state_off.load_script(&*it);
			}
			break;
		case QDSCR_GRID_ZONE_HEIGHT:
			xml::tag_buffer(*it) > _height;
			break;
		case QDSCR_GRID_ZONE_CONTOUR:
		case QDSCR_CONTOUR_POLYGON:
			qdContour::load_script(&*it);
			break;
		case QDSCR_GRID_ZONE_SHADOW_COLOR:
			xml::tag_buffer(*it) > _shadow_color;
			break;
		case QDSCR_GRID_ZONE_SHADOW_ALPHA:
			xml::tag_buffer(*it) > _shadow_alpha;
			break;
		}
	}

	return true;
}

bool qdGridZone::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<grid_zone name=\"%s\" grid_zone_height=\"%d\"", qdscr_XML_string(name()), _height));

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (_state) {
		fh.writeString(" state=\"1\"");
	} else {
		fh.writeString(" state=\"0\"");
	}

	if (_shadow_color) {
		fh.writeString(Common::String::format(" shadow_color=\"%d\"", _shadow_color));
	}

	if (_shadow_alpha != QD_NO_SHADOW_ALPHA) {
		fh.writeString(Common::String::format(" shadow_alpha=\"%d\"", _shadow_alpha));
	}

	fh.writeString(">\r\n");

	_state_on.save_script(fh, indent + 1);
	_state_off.save_script(fh, indent + 1);

	if (contour_size()) {
		qdContour::save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</grid_zone>\r\n");

	return true;
}

bool qdGridZone::set_height(int _h) {
	_height = _h;
	if (_state) {
		if (apply_zone()) {
			qdGameScene *sp = static_cast<qdGameScene *>(owner());

			_update_timer = sp->zone_update_count();
			sp->inc_zone_update_count();

			return true;
		} else
			return false;
	}

	return true;
}

bool qdGridZone::apply_zone() const {
	if (!owner() || owner()->named_object_type() != QD_NAMED_OBJECT_SCENE) return false;
	if (is_mask_empty()) return false;

	qdCamera *camera = static_cast<qdGameScene *>(owner())->get_camera();
	if (!camera) return false;

	Vect2s pos = mask_pos();
	pos.x -= mask_size().x / 2;
	pos.y -= mask_size().y / 2;

//	const byte* mask_ptr = mask_data();

	if (_state) {
		for (int y = 0; y < mask_size().y; y++) {
			for (int x = 0; x < mask_size().x; x++) {
				if (is_inside(pos + Vect2s(x, y))) {
//				if(*mask_ptr++){
					if (sGridCell * p = camera->get_cell(pos + Vect2s(x, y))) {
						p->make_walkable();
						p->set_height(_height);
					}
				}
			}
		}
	} else {
		for (int y = 0; y < mask_size().y; y++) {
			for (int x = 0; x < mask_size().x; x++) {
				if (is_inside(pos + Vect2s(x, y))) {
//				if(*mask_ptr++){
					if (sGridCell * p = camera->get_cell(pos + Vect2s(x, y))) {
						p->make_impassable();
						p->set_height(0);
					}
				}
			}
		}
	}

	return true;
}

bool qdGridZone::set_state(bool st) {
	_state = st;

	if (apply_zone()) {
		debugC(3, kDebugLog, "[%d] zone condition: %s %s", g_system->getMillis(), transCyrillic(name()), (st) ? "on" : "off");

		qdGameScene *sp = static_cast<qdGameScene *>(owner());

		_update_timer = sp->zone_update_count();
		sp->inc_zone_update_count();

		return true;
	}

	return false;
}

bool qdGridZone::select(qdCamera *camera, bool bSelect) const {
	if (is_mask_empty())
		return false;

	Vect2s pos = mask_pos();
	pos.x -= mask_size().x / 2;
	pos.y -= mask_size().y / 2;

//	const byte* mask_ptr = mask_data();

	if (bSelect) {
		for (int y = 0; y < mask_size().y; y++) {
			for (int x = 0; x < mask_size().x; x++) {
				if (is_inside(pos + Vect2s(x, y))) {
//				if(*mask_ptr++){
					if (sGridCell * p = camera->get_cell(pos + Vect2s(x, y)))
						p->select();
				}
			}
		}
	} else {
		for (int y = 0; y < mask_size().y; y++) {
			for (int x = 0; x < mask_size().x; x++) {
				if (is_inside(pos + Vect2s(x, y))) {
//				if(*mask_ptr++){
					if (sGridCell * p = camera->get_cell(pos + Vect2s(x, y)))
						p->deselect();
				}
			}
		}
	}

	return true;
}

bool qdGridZone::select(bool bSelect) const {
	assert(owner() || owner()->named_object_type() == QD_NAMED_OBJECT_SCENE);

	qdNamedObject *obj = owner();
	qdCamera *camera = static_cast<qdGameScene *>(obj)->get_camera();
	if (!camera) return false;
	return select(camera, bSelect);
}

bool qdGridZone::is_object_in_zone(const qdGameObject *obj) const {
	if (!owner() || owner()->named_object_type() != QD_NAMED_OBJECT_SCENE || owner() != obj->owner()) return false;

	return is_point_in_zone(Vect2f(obj->R().x, obj->R().y));
}

bool qdGridZone::is_point_in_zone(const Vect2f &r) const {
	assert(owner());
	qdCamera *camera = static_cast<qdGameScene *>(owner())->get_camera();
	assert(camera);

	Vect2s v = camera->get_cell_index(r.x, r.y);
	if (v.x == -1) return false;

	return is_inside(v);
}

qdGridZoneState *qdGridZone::get_state(const char *state_name) {
	if (!strcmp(state_name, _state_on.name())) return &_state_on;

	return &_state_off;
}

bool qdGridZone::load_data(Common::SeekableReadStream &fh, int saveVersion) {
	debugC(3, kDebugSave, "  qdGridZone::load_data before: %ld", fh.pos());
	if (!qdNamedObject::load_data(fh, saveVersion)) {
		return false;
	}

	char st = fh.readByte();
	_update_timer = fh.readSint32LE();

	_state = (st) ? true : false;
	debugC(3, kDebugSave, "  qdGridZone::load_data after: %ld", fh.pos());
	return true;
}


bool qdGridZone::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdGridZone::save_data before: %ld", fh.pos());
	if (!qdNamedObject::save_data(fh)) {
		return false;
	}

	fh.writeByte(_state);
	fh.writeSint32LE(_update_timer);

	debugC(3, kDebugSave, "  qdGridZone::save_data after: %ld", fh.pos());
	return true;
}


bool qdGridZone::init() {
	return set_state(_initial_state);
}

bool qdGridZone::is_any_personage_in_zone() const {
	if (!owner() || owner()->named_object_type() != QD_NAMED_OBJECT_SCENE) return false;

	const qdGameScene *p = static_cast<const qdGameScene *>(owner());
	return p->is_any_personage_in_zone(this);
}

} // namespace QDEngine
