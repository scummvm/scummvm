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
#include "common/file.h"
#include "common/savefile.h"
#include "common/stream.h"

#include "engines/metaengine.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_interface_save.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_dispatcher.h"
#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

bool qdInterfaceSave::_save_mode = false;
int qdInterfaceSave::_current_save_ID = 0;

qdInterfaceSave::qdInterfaceSave() : _save_ID(0),
	_thumbnail_size_x(0),
	_thumbnail_size_y(0),
	_text_dx(0),
	_text_dy(0) {
	_thumbnail.set_owner(this);
	_frame.set_owner(this);

	_isAutosaveSlot = false;

	_save_ID = _current_save_ID++;
}

qdInterfaceSave::qdInterfaceSave(const qdInterfaceSave &sv) : qdInterfaceElement(sv),
	_save_ID(sv._save_ID),
	_thumbnail_size_x(sv._thumbnail_size_x),
	_thumbnail_size_y(sv._thumbnail_size_y),
	_text_dx(sv.text_dx()),
	_text_dy(sv.text_dy()),
	_isAutosaveSlot(sv._isAutosaveSlot) {
	_thumbnail.set_owner(this);
	_thumbnail = sv._thumbnail;

	_frame.set_owner(this);
	_frame = sv._frame;
}

qdInterfaceSave::~qdInterfaceSave() {
	_thumbnail.unregister_resources();
	_frame.unregister_resources();
}

qdInterfaceSave &qdInterfaceSave::operator = (const qdInterfaceSave &sv) {
	if (this == &sv) return *this;

	*static_cast<qdInterfaceElement *>(this) = sv;

	_save_ID = sv._save_ID;

	_thumbnail_size_x = sv._thumbnail_size_x;
	_thumbnail_size_y = sv._thumbnail_size_y;

	_isAutosaveSlot = sv._isAutosaveSlot;

	_text_dx = sv.text_dx();
	_text_dy = sv.text_dy();

	_thumbnail = sv._thumbnail;
	_frame = sv._frame;

	return *this;
}

bool qdInterfaceSave::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	_frame.mouse_handler(x, y, ev);
	debugC(9, kDebugInput, "qdInterfaceSave::mouse_handler(): ev = %d, x = %d, y = %d", ev, x, y);

	switch (ev) {
	case mouseDispatcher::EV_LEFT_DOWN:
	case mouseDispatcher::EV_RIGHT_DOWN:
		if (/*qdGameDispatcher *dp = */qdGameDispatcher::get_dispatcher()) {
			debugC(1, kDebugSave, "qdInterfaceSave::mouse_handler(): _save_mode = %d", _save_mode);
			clear_screen_region();

			if (_save_mode) {
				if (isAutosaveSlot())
					return true;

				qdInterfaceDispatcher *ip = qdInterfaceDispatcher::get_dispatcher();
				if (ip) {
					if (ip->has_save_title_screen()) {
						ip->setModalScreenMode(qdInterfaceDispatcher::MODAL_SCREEN_SAVE_NAME_EDIT);
						ip->handle_event(qdInterfaceEvent::EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL, ip->save_title_screen_name(), this);
					} else if (ip->has_save_prompt_screen() && g_engine->getSaveFileManager()->exists(g_engine->getSaveStateName(_save_ID))) {
						ip->setModalScreenMode(qdInterfaceDispatcher::MODAL_SCREEN_SAVE_OVERWRITE);
						ip->handle_event(qdInterfaceEvent::EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL, ip->save_prompt_screen_name(), this);
					} else {
						perform_save();
						ip->handle_event(qdInterfaceEvent::EVENT_RESUME_GAME, NULL);
					}
				}

				return true;
			} else {
				debugC(1, kDebugSave, "qdInterfaceSave::mouse_handler(): load_game() _save_ID = %d", _save_ID);
				g_engine->loadGameState(_save_ID);
				if (qdInterfaceDispatcher *ip = qdInterfaceDispatcher::get_dispatcher())
					ip->handle_event(qdInterfaceEvent::EVENT_RESUME_GAME, NULL);

				return true;
			}
		}
		break;
	default:
		break;
	}

	return false;
}

bool qdInterfaceSave::keyboard_handler(Common::KeyCode vkey) {
	return false;
}

bool qdInterfaceSave::init(bool is_game_active) {
	if (!is_game_active && _frame.need_active_game())
		set_lock(true);
	else
		set_lock(false);

	Common::String saveFileName(g_engine->getSaveStateName(_save_ID));
	bool fileExists = false;

	if (g_engine->getSaveFileManager()->exists(saveFileName)) {
		Common::InSaveFile *saveFile = g_engine->getSaveFileManager()->openForLoading(saveFileName);

		fileExists = true;

		ExtendedSavegameHeader header;

		if (MetaEngine::readSavegameHeader(saveFile, &header, true))
			_save_title =  header.description.c_str();

		delete saveFile;

		_thumbnail.set_animation_file(Common::Path(Common::String::format("scummvm/%s", saveFileName.c_str())));
	} else {
		_save_title = "";
	}

	set_state(&_frame);

	if (!_save_mode && !fileExists) {
		if (is_visible()) {
			debugC(3, kDebugInput, "qdInterfaceSave::init(): Hide %s", saveFileName.c_str());
			hide();

			if (qdInterfaceScreen *sp = dynamic_cast<qdInterfaceScreen * >(owner()))
				sp->build_visible_elements_list();
		}
	} else {
		if (!is_visible()) {
			show();

			if (qdInterfaceScreen *sp = dynamic_cast<qdInterfaceScreen * >(owner()))
				sp->build_visible_elements_list();
		}
	}

	return true;
}

bool qdInterfaceSave::redraw() const {
	//warning("STUB: qdInterfaceSave::redraw()");
	if (qdInterfaceDispatcher *pid = qdInterfaceDispatcher::get_dispatcher()) {
		if (pid->need_save_screenshot())
			if (const qdAnimation *p = _thumbnail.animation())
				p->redraw(r().x, r().y, 0);

		Common::String text;

		if (pid->need_show_save_title()) {
			text = title();
			if (pid->need_show_save_time())
				text += " ";
		}

		grDispatcher *gr_disp = grDispatcher::instance();
		if (!text.empty()) {
			qdGameDispatcher *game_disp = qdGameDispatcher::get_dispatcher();
			const grFont *font = NULL;
			if ((QD_FONT_TYPE_NONE != pid->save_font_type()) && game_disp)
				font = game_disp->find_font(pid->save_font_type());
			else
				font = gr_disp->get_default_font();

			int tx = r().x - size_x() / 2 + text_dx();
			int ty = r().y - size_y() / 2 + text_dy();

			gr_disp->drawText(tx, ty, pid->save_font_color(), text.c_str(), 0, 0, font);
		}
	}

	return qdInterfaceElement::redraw();
}

grScreenRegion qdInterfaceSave::screen_region() const {
	grScreenRegion reg0 = qdInterfaceElement::screen_region();
	grScreenRegion reg1(r().x, r().y, _thumbnail_size_x, _thumbnail_size_y);

	reg0 += reg1;

	return reg0;
}

int qdInterfaceSave::size_x() const {
	int x = _thumbnail_size_x;
	if (const qdAnimation *p = _frame.animation()) {
		if (x < p->size_x())
			x = p->size_x();
	}

	return x;
}

int qdInterfaceSave::size_y() const {
	int y = _thumbnail_size_y;
	if (const qdAnimation *p = _frame.animation()) {
		if (y < p->size_y())
			y = p->size_y();
	}

	return y;
}

bool qdInterfaceSave::quant(float dt) {
	qdInterfaceElement::quant(dt);

	_frame.quant(dt);

	return true;
}

bool qdInterfaceSave::hit_test(int x, int y) const {
	if (qdInterfaceElement::hit_test(x, y)) return true;

	x -= r().x;
	y -= r().y;

	bool result = false;

	if (x >= -_thumbnail_size_x / 2 && x < _thumbnail_size_x / 2 && y >= -_thumbnail_size_y / 2 && y < _thumbnail_size_y / 2)
		result = true;

	return result;
}

bool qdInterfaceSave::perform_save() {
	bool is_ok = true;
	if (qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher()) {
		debugC(1, kDebugSave, "qdInterfaceSave::perform_save(): _save_ID = %d", _save_ID);
		is_ok &= (g_engine->saveGameState(_save_ID, _save_title.c_str(), dp->is_autosave_slot(_save_ID)).getCode() == Common::kNoError);

		debugC(1, kDebugSave, "qdInterfaceSave::perform_save(): is_ok = %d", is_ok);

		is_ok &= init(true);
		return is_ok;
	}
	return false;
}

bool qdInterfaceSave::save_script_body(Common::WriteStream &fh, int indent) const {
	if (!_frame.save_script(fh, indent)) {
		return false;
	}

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<ID>%d</ID>\r\n", _save_ID));

	if (_thumbnail_size_x || _thumbnail_size_y) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<thumbnail_size>%d %d</thumbnail_size>\r\n", _thumbnail_size_x, _thumbnail_size_y));
	}

	if (_text_dx || _text_dy) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<text_shift>%d %d</text_shift>\r\n", _text_dx, _text_dy));
	}

	if (isAutosaveSlot()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("<is_autosave>1</is_autosave>\r\n");
	}

	return true;
}

bool qdInterfaceSave::load_script_body(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_INTERFACE_ELEMENT_STATE:
			if (!_frame.load_script(&*it)) return false;
			break;
		case QDSCR_INTERFACE_THUMBNAIL_SIZE:
			xml::tag_buffer(*it) > _thumbnail_size_x > _thumbnail_size_y;

			g_engine->_thumbSizeX = _thumbnail_size_x;
			g_engine->_thumbSizeY = _thumbnail_size_y;
			break;
		case QDSCR_INTERFACE_TEXT_SHIFT:
			xml::tag_buffer(*it) > _text_dx > _text_dy;
			break;
		case QDSCR_ID:
			xml::tag_buffer(*it) > _save_ID;
			break;
		case QDSCR_INTERFACE_SAVE_IS_AUTOSAVE:
			setAutosaveSlot(xml::tag_buffer(*it).get_int() != 0);
			break;
		}
	}

	return true;
}

} // namespace QDEngine
