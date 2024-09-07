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
#include "qdengine/qdengine.h"

#include "qdengine/parser/xml_tag_buffer.h"

#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_inventory.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_game_object_mouse.h"


namespace QDEngine {


qdInventory::qdInventory() : _need_redraw(false),
	_shadow_color(INV_DEFAULT_SHADOW_COLOR),
	_shadow_alpha(INV_DEFAULT_SHADOW_ALPHA),
	_additional_cells(0, 0) {
}

qdInventory::~qdInventory() {
	_cell_sets.clear();
}

void qdInventory::redraw(int offs_x, int offs_y, bool inactive_mode) const {
	qdInventoryCell::set_shadow(_shadow_color, _shadow_alpha);

	for (qdInventoryCellSetVector::const_iterator it = _cell_sets.begin(); it != _cell_sets.end(); ++it)
		it->redraw(offs_x, offs_y, inactive_mode);
}

void qdInventory::pre_redraw() const {
	for (qdInventoryCellSetVector::const_iterator it = _cell_sets.begin(); it != _cell_sets.end(); ++it)
		it->pre_redraw();

	if (_need_redraw) {
		if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
			for (qdInventoryCellSetVector::const_iterator it = _cell_sets.begin(); it != _cell_sets.end(); ++it) {
				dp->add_redraw_region(it->screen_region());
				dp->add_redraw_region(it->last_screen_region());
			}
		}
	}
}

void qdInventory::post_redraw() {
	toggle_redraw(false);

	for (qdInventoryCellSetVector::iterator it = _cell_sets.begin(); it != _cell_sets.end(); ++it)
		it->post_redraw();
}

bool qdInventory::put_object(qdGameObjectAnimated *p) {
	if (p->inventory_cell_index() != -1) {
		if (put_object(p, cell_position(p->inventory_cell_index())))
			return true;
	}

	for (auto  &it : _cell_sets) {
		if (it.put_object(p)) {
			p->set_inventory_cell_index(cell_index(p));
			p->set_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
			_need_redraw = true;
			return true;
		}
	}

	return false;
}

bool qdInventory::put_object(qdGameObjectAnimated *p, const Vect2s &pos) {
	for (auto &it : _cell_sets) {
		if (it.put_object(p, pos)) {
			p->set_inventory_cell_index(cell_index(p));
			p->set_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
			_need_redraw = true;
			return true;
		}
	}

	return false;
}

qdGameObjectAnimated *qdInventory::get_object(const Vect2s &pos) const {
	for (auto &it : _cell_sets) {
		if (it.hit(pos)) {
			qdGameObjectAnimated *p = it.get_object(pos);
			if (p) return p;
		}
	}

	return NULL;
}

bool qdInventory::remove_object(qdGameObjectAnimated *p) {
	for (auto &it : _cell_sets) {
		if (it.remove_object(p)) {
			p->drop_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
			_need_redraw = true;
			return true;
		}
	}

	return false;
}

bool qdInventory::is_object_in_list(const qdGameObjectAnimated *p) const {
	for (auto &it : _cell_sets) {
		if (it.is_object_in_list(p))
			return true;
	}

	return false;
}

bool qdInventory::load_script(const xml::tag *p) {
	qdInventoryCellSet set;

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_INVENTORY_CELL_SET:
			set.load_script(&*it);
			add_cell_set(set);
			break;
		case QDSCR_GRID_ZONE_SHADOW_COLOR:
			xml::tag_buffer(*it) > _shadow_color;
			break;
		case QDSCR_GRID_ZONE_SHADOW_ALPHA:
			xml::tag_buffer(*it) > _shadow_alpha;
			break;
		case QDSCR_INVENTORY_CELL_SET_ADDITIONAL_CELLS:
			xml::tag_buffer(*it) > _additional_cells.x > _additional_cells.y;
			break;
		}
	}

	return true;
}
bool qdInventory::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<inventory name=");

	if (name()) {
		fh.writeString(Common::String::format("\"%s\"", qdscr_XML_string(name())));
	} else {
		fh.writeString("\" \"");
	}

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}

	if (_shadow_color != INV_DEFAULT_SHADOW_COLOR) {
		fh.writeString(Common::String::format(" shadow_color=\"%u\"", _shadow_color));
	}

	if (_shadow_alpha != INV_DEFAULT_SHADOW_ALPHA) {
		fh.writeString(Common::String::format(" shadow_alpha=\"%d\"", _shadow_alpha));
	}

	fh.writeString(">\r\n");

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<inventory_cell_set_additional_cells>%d %d</inventory_cell_set_additional_cells>\r\n", _additional_cells.x, _additional_cells.y));

	for (auto &it : _cell_sets) {
		it.save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</inventory>\r\n");
	return true;
}

bool qdInventory::init(const qdInventoryCellTypeVector &tp) {
	bool result = true;
	for (auto &it : _cell_sets) {
		if (!it.init(tp)) {
			result = false;
		}
		it.set_additional_cells(_additional_cells);
	}
	return result;
}

bool qdInventory::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	if (ev == mouseDispatcher::EV_LEFT_DOWN) {
		if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
			bool ret_val = false;
			bool obj_flag = false;
			qdGameObjectAnimated *obj = get_object(Vect2s(x, y));

			qdGameObjectMouse *mp = dp->mouse_object();
			if (mp->object()) {
				if (!obj) {
					if (put_object(mp->object(), mp->screen_pos())) {
						mp->take_object(NULL);
						ret_val = true;
						obj_flag = true;
					}
				}
			} else {
				if (obj) {
					remove_object(obj);
					mp->take_object(obj);
					ret_val = true;
					obj_flag = true;
				}
			}
			if (obj) {
				if (qdGameScene * sp = dp->get_active_scene()) {
					sp->set_mouse_click_object(obj);
					ret_val = true;
				}
			}

			if (!obj_flag && check_flag(INV_PUT_OBJECT_AFTER_CLICK) && mp->object()) {
				if (put_object(mp->object()))
					mp->take_object(NULL);
			}

			return ret_val;
		}
	} else if (ev == mouseDispatcher::EV_RIGHT_DOWN) {
		if (check_flag(INV_ENABLE_RIGHT_CLICK)) {
			if (qdGameDispatcher * dp = qd_get_game_dispatcher()) {
				qdGameObjectMouse *mp = dp->mouse_object();
				if (mp->object()) {
					if (put_object(mp->object())) {
						mp->take_object(NULL);
						return true;
					}
				}
			}
		}
		if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
			if (qdGameObjectAnimated * obj = get_object(Vect2s(x, y))) {
				if (qdGameScene * sp = dp->get_active_scene()) {
					sp->set_mouse_right_click_object(obj);
					return true;
				}
			}
		}
	} else if (ev == mouseDispatcher::EV_MOUSE_MOVE) {
		qdGameObjectAnimated *obj = get_object(Vect2s(x, y));

		if (obj) {
			if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
				if (qdGameScene * sp = dp->get_active_scene()) {
					sp->set_mouse_hover_object(obj);
				}
			}
		}

		for (auto &it : _cell_sets) {
			it.set_mouse_hover_object(obj);
		}
	}

	return false;
}

void qdInventory::remove_cell_set(int idx) {
	assert(-1 < idx && idx < static_cast<int>(_cell_sets.size()));

	_cell_sets.erase(_cell_sets.begin() + idx);
}

bool qdInventory::load_resources() {
	debugC(4, kDebugLoad, "qdInventory::load_resources(), %u cells", _cell_sets.size());

	for (auto &it : _cell_sets)
		it.load_resources();

	return true;
}

bool qdInventory::free_resources() {
	for (auto &it : _cell_sets) {
		it.free_resources();
	}

	return true;
}

bool qdInventory::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(3, kDebugSave, "  qdInventory::load_data before: %d", (int)fh.pos());
	for (auto &it : _cell_sets) {
		if (!it.load_data(fh, save_version))
			return false;
	}
	debugC(3, kDebugSave, "  qdInventory::load_data after: %d", (int)fh.pos());

	debug_log();

	return true;
}

bool qdInventory::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdInventory::save_data before: %d", (int)fh.pos());
	for (auto &it : _cell_sets) {
		if (!it.save_data(fh)) {
			return false;
		}
	}
	debugC(3, kDebugSave, "  qdInventory::save_data after: %d", (int)fh.pos());

	return true;
}

int qdInventory::cell_index(const qdGameObjectAnimated *obj) const {
	int index = 0;
	for (auto &it : _cell_sets) {
		int idx = it.cell_index(obj);
		if (idx != -1) {
			return index + idx;
		}
		else {
			index += it.num_cells();
		}
	}

	return -1;
}

Vect2s qdInventory::cell_position(int cell_idx) const {
	for (auto &it : _cell_sets) {
		if (cell_idx < it.num_cells())
			return it.cell_position(cell_idx);

		cell_idx -= it.num_cells();
	}

	return Vect2s(0, 0);
}

void qdInventory::objects_quant(float dt) {
	for (auto &it : _cell_sets) {
		it.objects_quant(dt);
	}
}

void qdInventory::scroll_left() {
	for (auto &it : _cell_sets) {
		it.scroll_left();
	}
	toggle_redraw(true);
}

void qdInventory::scroll_right() {
	for (auto &it : _cell_sets) {
		it.scroll_right();
	}
	toggle_redraw(true);
}

void qdInventory::scroll_up() {
	for (auto &it : _cell_sets) {
		it.scroll_up();
	}
	toggle_redraw(true);
}

void qdInventory::scroll_down() {
	for (auto &it : _cell_sets) {
		it.scroll_down();
	}
	toggle_redraw(true);
}

void qdInventory::debug_log() const {
#ifdef _DEBUG
	debugCN(3, kDebugLog, "Inventory ");
	if (name())
		debugCN(3, kDebugLog, "%s", transCyrillic(name()));

	debugC(3, kDebugLog, "--------------");

	for (auto &it : _cell_sets) {
		it.debug_log();
	}

	debugC(3, kDebugLog, "--------------");
#endif
}
} // namespace QDEngine
