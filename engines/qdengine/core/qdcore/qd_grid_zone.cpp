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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/stream.h"
#include "qdengine/qdengine.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/qdcore/qd_game_scene.h"
#include "qdengine/core/qdcore/qd_game_object.h"
#include "qdengine/core/qdcore/qd_grid_zone.h"

namespace QDEngine {



/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdGridZone::qdGridZone() : qdContour(CONTOUR_POLYGON),
	height_(0),
	state_(false),
	initial_state_(false),
	state_on_(true),
	state_off_(false),
	update_timer_(0),
	shadow_alpha_(QD_NO_SHADOW_ALPHA),
	shadow_color_(0) {
	state_on_.set_owner(this);
	state_off_.set_owner(this);
}

qdGridZone::qdGridZone(const qdGridZone &gz) : qdNamedObject(gz), qdContour(gz),
	state_(gz.state_),
	initial_state_(gz.initial_state_),
	height_(gz.height_),
	state_on_(gz.state_on_),
	state_off_(gz.state_off_),
	update_timer_(gz.update_timer_),
	shadow_alpha_(gz.shadow_alpha_),
	shadow_color_(gz.shadow_color_) {
}

qdGridZone::~qdGridZone() {
};

qdGridZone &qdGridZone::operator = (const qdGridZone &gz) {
	if (this == &gz) return *this;

	*static_cast<qdNamedObject *>(this) = gz;
	*static_cast<qdContour *>(this) = gz;

	state_ = gz.state_;
	initial_state_ = gz.initial_state_;

	height_ = gz.height_;

	state_on_ = gz.state_on_;
	state_off_ = gz.state_off_;
	update_timer_ = gz.update_timer_;

	shadow_alpha_ = gz.shadow_alpha_;
	shadow_color_ = gz.shadow_color_;

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
			initial_state_ = state_ = (xml::tag_buffer(*it).get_int()) ? true : false;
			break;
		case QDSCR_GRID_ZONE_STATE:
			if (const xml::tag * tg = it->search_subtag(QDSCR_STATE)) {
				if (xml::tag_buffer(*tg).get_int())
					state_on_.load_script(&*it);
				else
					state_off_.load_script(&*it);
			}
			break;
		case QDSCR_GRID_ZONE_HEIGHT:
			xml::tag_buffer(*it) > height_;
			break;
		case QDSCR_GRID_ZONE_CONTOUR:
		case QDSCR_CONTOUR_POLYGON:
			qdContour::load_script(&*it);
			break;
		case QDSCR_GRID_ZONE_SHADOW_COLOR:
			xml::tag_buffer(*it) > shadow_color_;
			break;
		case QDSCR_GRID_ZONE_SHADOW_ALPHA:
			xml::tag_buffer(*it) > shadow_alpha_;
			break;
		}
	}

	return true;
}

bool qdGridZone::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<grid_zone name=\"%s\" grid_zone_height=\"%d\"", qdscr_XML_string(name()), height_));

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (state_) {
		fh.writeString(" state=\"1\"");
	} else {
		fh.writeString(" state=\"0\"");
	}

	if (shadow_color_) {
		fh.writeString(Common::String::format(" shadow_color=\"%d\"", shadow_color_));
	}

	if (shadow_alpha_ != QD_NO_SHADOW_ALPHA) {
		fh.writeString(Common::String::format(" shadow_alpha=\"%d\"", shadow_alpha_));
	}

	fh.writeString(">\r\n");

	state_on_.save_script(fh, indent + 1);
	state_off_.save_script(fh, indent + 1);

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
	height_ = _h;
	if (state_) {
		if (apply_zone()) {
			qdGameScene *sp = static_cast<qdGameScene *>(owner());

			update_timer_ = sp->zone_update_count();
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

//	const unsigned char* mask_ptr = mask_data();

	if (state_) {
		for (int y = 0; y < mask_size().y; y ++) {
			for (int x = 0; x < mask_size().x; x ++) {
				if (is_inside(pos + Vect2s(x, y))) {
//				if(*mask_ptr++){
					if (sGridCell * p = camera->get_cell(pos + Vect2s(x, y))) {
						p->make_walkable();
						p->set_height(height_);
					}
				}
			}
		}
	} else {
		for (int y = 0; y < mask_size().y; y ++) {
			for (int x = 0; x < mask_size().x; x ++) {
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
	state_ = st;

	if (apply_zone()) {
		debugC(3, kDebugLog, "[%d] zone condition: %s %s", g_system->getMillis(), transCyrillic(name()), (st) ? "on" : "off");

		qdGameScene *sp = static_cast<qdGameScene *>(owner());

		update_timer_ = sp->zone_update_count();
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

//	const unsigned char* mask_ptr = mask_data();

	if (bSelect) {
		for (int y = 0; y < mask_size().y; y ++) {
			for (int x = 0; x < mask_size().x; x ++) {
				if (is_inside(pos + Vect2s(x, y))) {
//				if(*mask_ptr++){
					if (sGridCell * p = camera->get_cell(pos + Vect2s(x, y)))
						p->select();
				}
			}
		}
	} else {
		for (int y = 0; y < mask_size().y; y ++) {
			for (int x = 0; x < mask_size().x; x ++) {
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
	if (!strcmp(state_name, state_on_.name())) return &state_on_;

	return &state_off_;
}

bool qdGridZone::load_data(Common::SeekableReadStream &fh, int saveVersion) {
	if (!qdNamedObject::load_data(fh, saveVersion)) {
		return false;
	}

	char st = fh.readByte();
	update_timer_ = fh.readSint32LE();

	state_ = (st) ? true : false;
	return true;
}

bool qdGridZone::load_data(qdSaveStream &fh, int save_version) {
	if (!qdNamedObject::load_data(fh, save_version)) return false;

	char st;
	fh > st > update_timer_;

	state_ = (st) ? true : false;

	return true;
}

bool qdGridZone::save_data(Common::SeekableWriteStream &fh) const {
	if (!qdNamedObject::save_data(fh)) {
		return false;
	}

	fh.writeByte(state_);
	fh.writeSint32LE(update_timer_);

	return true;
}

bool qdGridZone::save_data(qdSaveStream &fh) const {
	if (!qdNamedObject::save_data(fh)) return false;

	fh < char(state_) < update_timer_;

	return true;
}

bool qdGridZone::init() {
#ifdef _QUEST_EDITOR
	state_on_.init();
	state_off_.init();
#endif//_QUEST_EDITOR
	return set_state(initial_state_);
}

bool qdGridZone::is_any_personage_in_zone() const {
	if (!owner() || owner()->named_object_type() != QD_NAMED_OBJECT_SCENE) return false;

	const qdGameScene *p = static_cast<const qdGameScene *>(owner());
	return p->is_any_personage_in_zone(this);
}
} // namespace QDEngine
