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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/system.h"

#include "video/video_decoder.h"
#include "video/mpegps_decoder.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_parser.h"

#include "qdengine/qdcore/util/plaympp_api.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_textdb.h"
#include "qdengine/qdcore/qd_sound.h"
#include "qdengine/qdcore/qd_music_track.h"
#include "qdengine/qdcore/qd_video.h"
#include "qdengine/qdcore/qd_grid_zone.h"
#include "qdengine/qdcore/qd_music_track.h"
#include "qdengine/qdcore/qd_animation_set.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_object_mouse.h"
#include "qdengine/qdcore/qd_game_object_moving.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_trigger_chain.h"
#include "qdengine/qdcore/qd_named_object_reference.h"
#include "qdengine/qdcore/qd_named_object_indexer.h"
#include "qdengine/qdcore/qd_minigame.h"
#include "qdengine/qdcore/qd_game_end.h"
#include "qdengine/qdcore/qd_counter.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_file_manager.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/gr_font.h"
#include "qdengine/system/sound/snd_dispatcher.h"
#include "qdengine/system/input/keyboard_input.h"

namespace QDEngine {

qdGameDispatcher *qdGameDispatcher::_dispatcher = NULL;

qdGameDispatcher *qd_get_game_dispatcher() {
	return qdGameDispatcher::get_dispatcher();
}

void qd_set_game_dispatcher(qdGameDispatcher *p) {
	qdGameDispatcher::set_dispatcher(p);
}

bool qd_keyboard_handler(Common::KeyCode vkey, bool event) {
	if (qdGameDispatcher *p = qd_get_game_dispatcher())
		return p->keyboard_handler(vkey, event);

	return false;
}

bool qd_char_input_handler(int input) {
	if (qdInterfaceDispatcher *dp = qdInterfaceDispatcher::get_dispatcher())
		return dp->char_input_handler(input);

	return false;
}

qdGameDispatcher::qdGameDispatcher() : _is_paused(false),
	_cur_scene(NULL),
	_scene_loading_progress_data(NULL),
	_scene_loading_progress_fnc(NULL),
	_cur_inventory(NULL),
	_cur_video(NULL),
	_next_scene(NULL),
	_cur_music_track(NULL),
	_cur_interface_music_track(NULL),
	_scene_saved(false),
	_mouse_click_state(NULL),
	_mouse_click_obj(NULL),
	_game_end(NULL) {
	_timer = 0;
	_default_font = 0;

	_hall_of_fame_size = 0;

	_resource_compression = 0;

	_fade_timer = 0.f;
	_fade_duration = 0.1f;

	_autosave_slot = 0;

	_interface_music_mode = false;

	_dialog_states.reserve(16);
	_dialog_states_last.reserve(16);

	_enable_file_packages = false;

	debugC(1, kDebugTemp, "Setting up mouse...");
	_mouse_obj = new qdGameObjectMouse;
	_mouse_obj->set_owner(this);

	_mouse_animation = new qdAnimation;

	_mouse_cursor_pos = Vect2f(0, 0);

	qdAnimationFrame *p = new qdAnimationFrame;
	p->set_file(Common::Path("Resource/Cursors/default.tga"));

	_mouse_animation->add_frame(p);

	debugC(1, kDebugTemp, "Mouse_set_animation");
	_mouse_obj->set_animation(_mouse_animation);
	debugC(1, kDebugTemp, "Mouse_set_animation_over");

	if (!_dispatcher) {
		keyboardDispatcher::instance()->set_handler(qd_keyboard_handler);
		grDispatcher::set_input_handler(qd_char_input_handler);
		set_dispatcher(this);
	}

}

qdGameDispatcher::~qdGameDispatcher() {
	free_resources();
	delete _mouse_obj;
	delete _mouse_animation;

	_trigger_chains.clear();

	if (_dispatcher == this)
		set_dispatcher(NULL);
}

void qdGameDispatcher::update_time() {
	_timer = g_system->getMillis();
}

void qdGameDispatcher::quant() {
	debugC(9, kDebugQuant, "qdGameDispatcher::quant()");

	if (check_flag(SKIP_REDRAW_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Skipping redraw...");
		drop_flag(SKIP_REDRAW_FLAG);
		toggle_full_redraw();
	}
	int idt = ConfMan.getInt("logic_period");

	if (!_scene_saved && _cur_scene && _cur_scene->autosave_slot() != -1) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Autosaving...");
		g_engine->saveGameState(_cur_scene->autosave_slot(), "Autosave", true);
	}

	if (check_flag(SAVE_GAME_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Saving game...");
		g_engine->saveGameState(_autosave_slot, "Autosave", true);
		drop_flag(SAVE_GAME_FLAG);
	}
	if (check_flag(LOAD_GAME_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Loading game...");
		g_engine->loadGameState(_autosave_slot);
		drop_flag(LOAD_GAME_FLAG);
	}

	_scene_saved = true;

	quant(float(idt) / 1000.0f);

	_timer += idt;

	if (!is_paused() && _next_scene) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Loading next scene...");
		select_scene(_next_scene);
		set_next_scene(NULL);
		quant(0.0f);
		quant(0.0f);
	}

	if (check_flag(MAIN_MENU_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Main menu...");
		drop_flag(MAIN_MENU_FLAG);
		toggle_main_menu(true);
	}

	if (_game_end) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Game end...");
		end_game(_game_end);
		_game_end = NULL;
	}
}

void qdGameDispatcher::quant(float dt) {
	debugC(9, kDebugQuant, "qdGameDispatcher::quant(%f)", dt);
	if (sndDispatcher *snd = sndDispatcher::get_dispatcher()) {
		snd->quant();
	}

	_mouse_obj->set_pos(Vect3f(mouseDispatcher::instance()->mouse_x(), mouseDispatcher::instance()->mouse_y(), 0));

	_mouse_cursor_pos.x = mouseDispatcher::instance()->mouse_x() + _mouse_obj->screen_pos_offset().x;
	_mouse_cursor_pos.y = mouseDispatcher::instance()->mouse_y() + _mouse_obj->screen_pos_offset().y;

	_mouse_obj->update_screen_pos();
	_mouse_obj->quant(dt);

	mouseDispatcher::instance()->toggle_event(mouseDispatcher::EV_MOUSE_MOVE);
	for (int i = mouseDispatcher::first_event_ID(); i <= mouseDispatcher::last_event_ID(); i++) {
		mouseDispatcher::mouseEvent ev = static_cast<mouseDispatcher::mouseEvent>(i);
		if (mouseDispatcher::instance()->check_event(ev)) {
			if (mouse_handler(_mouse_cursor_pos.x, _mouse_cursor_pos.y, ev))
				mouseDispatcher::instance()->clear_event(ev);
		}
	}

	if (_cur_music_track && !_interface_music_mode && !is_video_playing()) {
		if (!mpegPlayer::instance().is_playing())
			_cur_music_track = NULL;
	}

	if (_interface_music_mode && _cur_interface_music_track) {
		if (!mpegPlayer::instance().is_playing())
			_cur_interface_music_track = NULL;
	}

	if (!is_paused() || check_flag(NEXT_FRAME_FLAG)) {
		debugC(9, kDebugQuant, "qdGameDispatcher::quant() Quanting...");
		qdGameDispatcherBase::quant(dt);

		if (_cur_scene)
			_cur_scene->init_objects_grid();

		for (auto &it: trigger_chain_list()) {
			it->quant(dt);
		}

		for (qdCounterList::const_iterator it = counter_list().begin(); it != counter_list().end(); ++it)
			(*it)->quant();

		_interface_dispatcher.quant(dt);

		if (_cur_scene) {
			debugC(9, kDebugQuant, "qdGameDispatcher::quant() Checking Cur Scene...");
			_cur_scene->quant(dt);
		}

		for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it)
			(*it)->objects_quant(dt);

		if (_dialog_states != _dialog_states_last) {
			for (dialog_states_container_t::iterator it = _dialog_states_last.begin(); it != _dialog_states_last.end(); ++it)
				_screen_texts.clear_texts((*it)->owner());

			for (dialog_states_container_t::iterator it = _dialog_states.begin(); it != _dialog_states.end(); ++it) {
				_screen_texts.add_text(qdGameDispatcher::TEXT_SET_DIALOGS, qdScreenText((*it)->short_text(), (*it)->text_format(true), Vect2i(0, 0), (*it)));
			}
		}

		if (check_flag(FADE_IN_FLAG | FADE_OUT_FLAG)) {
			_fade_timer += dt;
			if (_fade_timer >= _fade_duration && !check_flag(FADE_OUT_FLAG)) {
				_fade_timer = _fade_duration;
				drop_flag(FADE_IN_FLAG | FADE_OUT_FLAG);
			}

			toggle_full_redraw();
		}

		_dialog_states_last = _dialog_states;
		_dialog_states.clear();
		drop_flag(NEXT_FRAME_FLAG);
	} else {
		if (is_video_playing()) {
			if (is_video_finished()) {
				close_video();
			} else {
				continueVideo();
			}
		}

		if (_interface_dispatcher.is_active()) {
			_interface_dispatcher.quant(dt);
		}
	}

	mouseDispatcher::instance()->clear_events();
	drop_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG);
	_mouse_click_obj = NULL;
	_mouse_click_state = NULL;

	if (check_flag(CLICK_FAILED_FLAG)) set_flag(CLICK_WAS_FAILED_FLAG);
	else drop_flag(CLICK_WAS_FAILED_FLAG);

	if (check_flag(OBJECT_CLICK_FAILED_FLAG)) set_flag(OBJECT_CLICK_WAS_FAILED_FLAG);
	else drop_flag(OBJECT_CLICK_WAS_FAILED_FLAG);

	drop_flag(CLICK_FAILED_FLAG | OBJECT_CLICK_FAILED_FLAG);
}

void qdGameDispatcher::load_script(const xml::tag *p) {
	qdVideo *vid;
	qdTriggerChain *trc;
	qdGameObject *obj;
	qdGameScene *scn;
	qdCounter *cnt;

	bool text_fmt_flag = false;

	qdGameDispatcherBase::load_script_body(p);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_HALL_OF_FAME_SIZE:
			xml::tag_buffer(*it) > _hall_of_fame_size;
			break;
		case QDSCR_GAME_TITLE:
			_game_title = it->data();
			debug("> Game title: '%s', id: %d", transCyrillic(_game_title.c_str()), it->origID());
			break;
		case QDSCR_TEXT_DB:
			_texts_database = Common::Path(it->data(), '\\');
			debug("> Text db: '%s', id: %d", transCyrillic(_texts_database.toString()), it->origID());
			break;
		case QDSCR_CD_KEY:
			_cd_key = it->data();
			debug("> CD key: '%s', id: %d", transCyrillic(_cd_key.c_str()), it->origID());
			break;
		case QDSCR_STARTUP_SCENE:
			set_startup_scene(it->data());
			break;
		case QDSCR_RESOURCE_COMPRESSION:
			xml::tag_buffer(*it) > _resource_compression;
			debug("> Resource compression: '%d', id: %d", _resource_compression, it->origID());
			break;
		case QDSCR_CD:
			set_CD_info(xml::tag_buffer(*it).get_uint());
			break;
		case QDSCR_MOUSE_OBJECT:
			_mouse_obj->load_script(&*it);
			break;
		case QDSCR_COUNTER:
			cnt = new qdCounter;
			cnt->load_script(&*it);
			add_counter(cnt);
			break;
		case QDSCR_LOCATION: {
			for (xml::tag::subtag_iterator it1 = it->subtags_begin(); it1 != it->subtags_end(); ++it1) {
				// int sz = it->num_subtags();
				if (it1->ID() == QDSCR_SCENE) {
					scn = new qdGameScene;
					scn->load_script(&*it1);
					add_scene(scn);
				}
			}
		}
		break;
		case QDSCR_SCENE:
			scn = new qdGameScene;
			scn->load_script(&*it);
			add_scene(scn);
			break;
		case QDSCR_TRIGGER_CHAIN:
			trc = new qdTriggerChain;
			trc->load_script(&*it);
			add_trigger_chain(trc);
			break;
		case QDSCR_VIDEO:
			vid = new qdVideo;
			vid->load_script(&*it);
			add_video(vid);
			break;
		case QDSCR_SCREEN_SIZE: {
			int x, y;
			xml::tag_buffer(*it) > x > y;
			g_engine->_screenW = x;
			g_engine->_screenH = y;
		}
		break;
		case QDSCR_MOVING_OBJECT:
			obj = new qdGameObjectMoving;
			obj->load_script(&*it);
			add_global_object(obj);
			break;
		case QDSCR_INVENTORY_CELL_TYPE: {
			qdInventoryCellType tp;
			tp.load_script(&*it);
			add_inventory_cell_type(tp);
		}
		break;
		case QDSCR_FONT_INFO: {
			qdFontInfo *fi = new qdFontInfo();
			fi->load_script(&*it);
			add_font_info(fi);
		}
		break;
		case QDSCR_INVENTORY: {
			qdInventory *p1 = new qdInventory;
			p1->load_script(&*it);
			add_inventory(p1);
		}
		break;
		case QDSCR_MINIGAME: {
			qdMiniGame *p1 = new qdMiniGame;
			p1->load_script(&*it);
			add_minigame(p1);
		}
		break;
		case QDSCR_INTERFACE:
			_interface_dispatcher.load_script(&*it);
			break;
		case QDSCR_GAME_END: {
			qdGameEnd *p1 = new qdGameEnd;
			p1->load_script(&*it);
			add_game_end(p1);
		}
		break;
		case QDSCR_TEXT_SET: {
			qdScreenTextSet set;
			set.load_script(&*it);
			_screen_texts.add_text_set(set);
		}
		break;
		case QDSCR_DEFAULT_FONT:
			xml::tag_buffer(*it) > _default_font;
			break;
		case QDSCR_SCREEN_TEXT_FORMAT:
			qdScreenTextFormat frmt;
			frmt.load_script(&*it);
			frmt.toggle_global_depend(false);
			if (!text_fmt_flag) {
				text_fmt_flag = true;
				qdScreenTextFormat::set_global_text_format(frmt);
			} else
				qdScreenTextFormat::set_global_topic_format(frmt);
			break;
		}
	}

	merge_global_objects();

	if (_enable_file_packages) {
		qdFileManager::instance().init(CD_count());
	}

	load_hall_of_fame();

	qdNamedObjectIndexer::instance().resolve_references();
	qdNamedObjectIndexer::instance().clear();

	if (!_texts_database.empty()) {
		Common::SeekableReadStream *fh;

		if (qdFileManager::instance().open_file(&fh, _texts_database, false)) {
			qdTextDB::instance().load(fh);
			delete fh;
		} else {
			warning("Cannot open textsdbPath: '%s'", transCyrillic(_texts_database.toString().c_str()));
		}
	} else
		qdTextDB::instance().clear();

	init();
}

bool qdGameDispatcher::save_script(Common::SeekableWriteStream &fh) const {
	fh.writeString("<?xml version=\"1.0\" encoding=\"WINDOWS-1251\"?> \r\n");
	fh.writeString("<qd_script>\r\n");

	if (!_game_title.empty()) {
		fh.writeString(Common::String::format("\t<game_title>%s</game_title>\r\n", qdscr_XML_string(_game_title.c_str())));
	}

	if (_default_font) {
		fh.writeString(Common::String::format("\t<default_font>%d</default_font>\r\n", _default_font));
	}

	// Сохраняем глобальный формат до сохранения прочих объектов с форматом
	// текста чтобы он загрузился раньше прочих объектов с форматом текста
	// и все нормально проинициализировалось
	qdScreenTextFormat frmt = qdScreenTextFormat::global_text_format();
	frmt.toggle_global_depend(false); // Чтобы нормально сохранилось
	frmt.save_script(fh, 1);

	frmt = qdScreenTextFormat::global_topic_format();
	frmt.toggle_global_depend(false);
	frmt.save_script(fh, 1);

	qdGameDispatcherBase::save_script_body(fh);

	if (_hall_of_fame_size) {
		fh.writeString(Common::String::format("\t<hof_size>%d</hof_size>\r\n", _hall_of_fame_size));
	}

	if (has_startup_scene()) {
		fh.writeString(Common::String::format("\t<startup_scene>%s</startup_scene>\r\n", qdscr_XML_string(startup_scene())));
	}

	if (_resource_compression) {
		fh.writeString(Common::String::format("\t<compression>%d</compression>\r\n", _resource_compression));
	}

	if (CD_info()) {
		fh.writeString(Common::String::format("\t<cd>%d</cd>\r\n", CD_info()));
	}

	if (!_texts_database.empty()) {
		fh.writeString(Common::String::format("\t<text_db>%s</text_db>\r\n", qdscr_XML_string(_texts_database.toString('\\'))));
	}

	if (!_cd_key.empty()) {
		fh.writeString(Common::String::format("\t<cd_key>%s</cd_key>\r\n", qdscr_XML_string(_cd_key.c_str())));
	}

	fh.writeString(Common::String::format("\t<screen_size>%d %d</screen_size>\r\n", g_engine->_screenW, g_engine->_screenH));

	_screen_texts.save_script(fh, 1);

	_mouse_obj->save_script(fh, 1);

	for (auto &it : game_end_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : counter_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : scene_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : video_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : global_object_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : trigger_chain_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : _inventory_cell_types) {
		it.save_script(fh, 1);
	}

	for (auto &it : fonts_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : inventory_list()) {
		it->save_script(fh, 1);
	}

	for (auto &it : minigame_list()) {
		it->save_script(fh, 1);
	}

	_interface_dispatcher.save_script(fh, 1);

	fh.writeString("</qd_script>\r\n");
	return true;
}

bool qdGameDispatcher::save_script(const char *fname) const {
	Common::DumpFile df;
	df.open(Common::Path(fname));

	if (df.isOpen()) {
		save_script(df);
	} else {
		warning("Not able to open %s", fname);
	}

	df.close();

	return true;
}

void qdGameDispatcher::load_script(const char *fname) {
	xml::parser &pr = qdscr_XML_Parser();

	uint32 start_clock = g_system->getMillis();
	pr.parse_file(fname);
	uint32 end_clock =  g_system->getMillis();
	warning("Script parsing: %d ms", end_clock - start_clock);

	if (pr.is_script_binary()) {
		_enable_file_packages = true;
		qdFileManager::instance().enable_packages();
	}

	start_clock = g_system->getMillis();

	if (const xml::tag *tg = pr.root_tag().search_subtag(QDSCR_ROOT)) {
		load_script(tg);
	}

	end_clock = g_system->getMillis();
	warning("Script processing: %d ms", end_clock - start_clock);

	pr.clear();
}

bool qdGameDispatcher::select_scene(const char *s_name) {
	toggle_full_redraw();

	if (!s_name) {
		if (has_startup_scene())
			return select_scene(startup_scene());
		else
			return false;
	}

	if (qdGameScene *sp = get_scene(s_name))
		return select_scene(sp);

	return false;
}

qdSound *qdGameDispatcher::get_sound(const char *name) {
	qdSound *p = NULL;

	if (_cur_scene)
		p = _cur_scene->get_sound(name);

	if (p) return p;

	return qdGameDispatcherBase::get_sound(name);
}

qdAnimation *qdGameDispatcher::get_animation(const char *name) {
	qdAnimation *p = NULL;

	if (_cur_scene)
		p = _cur_scene->get_animation(name);

	if (p) return p;

	return qdGameDispatcherBase::get_animation(name);
}

qdAnimationSet *qdGameDispatcher::get_animation_set(const char *name) {
	qdAnimationSet *p = NULL;

	if (_cur_scene)
		p = _cur_scene->get_animation_set(name);

	if (p) return p;

	return qdGameDispatcherBase::get_animation_set(name);
}

void qdGameDispatcher::pre_redraw() {
	grDispatcher::instance()->clear_changes_mask();

	if (_cur_scene)
		_cur_scene->pre_redraw();

	_interface_dispatcher.pre_redraw();
	_mouse_obj->pre_redraw();
	_screen_texts.pre_redraw();

	if (!need_full_redraw()) {
		if (_cur_inventory) {
//			_cur_inventory->toggle_redraw(true);
			_cur_inventory->pre_redraw();
		}

		if (_cur_scene) {
			for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it) {
				if (*it != _cur_inventory && (*it)->check_flag(qdInventory::INV_VISIBLE_WHEN_INACTIVE) && _cur_scene->need_to_redraw_inventory((*it)->name())) {
//					(*it)->toggle_redraw(true);
					(*it)->pre_redraw();
				}
			}
		}
	} else
		add_redraw_region(grScreenRegion(grDispatcher::instance()->get_SizeX() / 2, grDispatcher::instance()->get_SizeY() / 2, grDispatcher::instance()->get_SizeX(), grDispatcher::instance()->get_SizeY()));

	grDispatcher::instance()->build_changed_regions();
}

void qdGameDispatcher::post_redraw() {
	if (_cur_scene)
		_cur_scene->post_redraw();

	_interface_dispatcher.post_redraw();
	_mouse_obj->post_redraw();
	_screen_texts.post_redraw();

	if (_cur_inventory)
		_cur_inventory->post_redraw();

	if (_cur_scene) {
		for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it) {
			if (*it != _cur_inventory && (*it)->check_flag(qdInventory::INV_VISIBLE_WHEN_INACTIVE) && _cur_scene->need_to_redraw_inventory((*it)->name()))
				(*it)->post_redraw();
		}
	}
}

//#define _GD_REDRAW_REGIONS_CHECK_

void qdGameDispatcher::redraw() {
	_mouse_obj->set_pos(Vect3f(mouseDispatcher::instance()->mouse_x(), mouseDispatcher::instance()->mouse_y(), 0));
	_mouse_obj->update_screen_pos();

	if (!check_flag(SKIP_REDRAW_FLAG)) {
		if (!is_video_playing()) {
			pre_redraw();
#ifndef _GD_REDRAW_REGIONS_CHECK_
			for (grDispatcher::region_iterator it = grDispatcher::instance()->changed_regions().begin(); it != grDispatcher::instance()->changed_regions().end(); ++it) {
				if (!it->is_empty())
					redraw(*it);
			}

			grDispatcher::instance()->flushChanges();
#else
			redraw(grScreenRegion(grDispatcher::instance()->get_SizeX() / 2, grDispatcher::instance()->get_SizeY() / 2, grDispatcher::instance()->get_SizeX(), grDispatcher::instance()->get_SizeY()));

			for (grDispatcher::region_iterator it = grDispatcher::instance()->changed_regions().begin(); it != grDispatcher::instance()->changed_regions().end(); ++it)
				grDispatcher::instance()->rectangle(it->min_x(), it->min_y(), it->size_x(), it->size_y(), 0xFFFFFF, 0, GR_OUTLINED);

			grDispatcher::instance()->flush();
#endif
		}
		if (!g_engine->_forceFullRedraw)
			drop_flag(FULLSCREEN_REDRAW_FLAG);
		post_redraw();
	}
}

void qdGameDispatcher::redraw(const grScreenRegion &reg) {
//	grDispatcher::instance()->setClip(reg.min_x() - 1,reg.min_y() - 1,reg.max_x() + 1,reg.max_y() + 1);
//	grDispatcher::instance()->erase(reg.min_x() - 1,reg.min_y() - 1,reg.size_x() + 2,reg.size_y() + 2,0);

	grDispatcher::instance()->setClip(reg.min_x(), reg.min_y(), reg.max_x(), reg.max_y());
	grDispatcher::instance()->erase(reg.min_x(), reg.min_y(), reg.size_x(), reg.size_y(), 0);

	if (!_interface_dispatcher.is_active()) {
		redraw_scene(true);
	} else {
		if (_interface_dispatcher.need_scene_redraw())
			redraw_scene(false);

		_interface_dispatcher.redraw();
	}

	debugC(1, kDebugTemp, "_mouse_obj->redraw()");
	_mouse_obj->redraw();

	grDispatcher::instance()->setClip();
}

void qdGameDispatcher::redraw_scene(bool draw_interface) {
	if (_cur_scene) {
		_cur_scene->redraw();

		if (draw_interface) {
			_interface_dispatcher.redraw();
			if (_cur_inventory) _cur_inventory->redraw();

			for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it) {
				if (*it != _cur_inventory && (*it)->check_flag(qdInventory::INV_VISIBLE_WHEN_INACTIVE) && _cur_scene->need_to_redraw_inventory((*it)->name()))
					(*it)->redraw(0, 0, true);
			}
		}

		_screen_texts.redraw();
		_cur_scene->debug_redraw();

		if (check_flag(FADE_IN_FLAG | FADE_OUT_FLAG)) {
			float phase = _fade_timer / _fade_duration;
			if (phase > 1.f) phase = 1.f;

			if (check_flag(FADE_OUT_FLAG))
				phase = 1.f - phase;

			grDispatcher::instance()->rectangleAlpha(0, 0,
					g_engine->_screenW, g_engine->_screenH,
			        0, round(phase * 255.f));
		}
	}
}

bool qdGameDispatcher::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	debugC(9, kDebugInput, "qdGameDispatcher::mouse_handler(%d, %d, %d)", x, y, ev);
	if ((ev == mouseDispatcher::EV_LEFT_DOWN || ev == mouseDispatcher::EV_RIGHT_DOWN) && _mouse_obj->object()) {
		set_flag(OBJECT_CLICK_FLAG);
		_mouse_click_obj = _mouse_obj->object();
	}

	if (!is_paused()) {
		if (_cur_inventory && _cur_inventory->mouse_handler(x, y, ev)) {
			debugC(3, kDebugInput, "qdGameDispatcher::mouse_handler(%d, %d, %d) Not paused...", x, y, ev);
			return true;
		}
	}

	if (_interface_dispatcher.mouse_handler(x, y, ev)) {
		debugC(9, kDebugInput, "qdGameDispatcher::mouse_handler(%d, %d, %d) Interface...", x, y, ev);
		mouseDispatcher::instance()->deactivate_event(ev);
		return true;
	}

	if (is_paused()) {
		if (is_video_playing() && (ev == mouseDispatcher::EV_LEFT_DOWN || ev == mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!_cur_video->check_flag(qdVideo::VID_DISABLE_INTERRUPT_FLAG)) {
				close_video();
				resume();
				return true;
			}
		}
		return false;
	}

	if (ev == mouseDispatcher::EV_LEFT_DOWN) {
		if (_mouse_click_obj)
			set_flag(OBJECT_CLICK_FAILED_FLAG);
		else
			set_flag(CLICK_FAILED_FLAG);
	}

	if (_cur_scene)
		return _cur_scene->mouse_handler(x, y, ev);

	return false;
}

int qdGameDispatcher::load_resources() {
	int size = 0;
	if (_mouse_obj->max_state())
		_mouse_obj->load_resources();
	else
		_mouse_animation->load_resources();

	if (_cur_scene) size += _cur_scene->load_resources();
	size += qdGameDispatcherBase::load_resources();

	for (auto &it : inventory_list()) {
		it->load_resources();
	}

	for (auto &icv : _inventory_cell_types) {
		icv.load_resources();
	}


	return size;
}

void qdGameDispatcher::free_resources() {
	_mouse_animation->free_resources();

	for (auto &icv : _inventory_cell_types) {
		icv.free_resources();
	}

	for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it)
		(*it)->free_resources();

	if (_cur_scene) _cur_scene->free_resources();

	qdGameDispatcherBase::free_resources();
}

int qdGameDispatcher::get_resources_size() {
	int size = 0;
	if (_cur_scene) size += _cur_scene->get_resources_size();
	size += qdGameDispatcherBase::get_resources_size();

	return size;
}

qdNamedObject *qdGameDispatcher::get_named_object(const qdNamedObjectReference *ref) {
	qdNamedObject *p = nullptr;

	for (int i = 0; i < ref->num_levels(); i++) {
		debugC(9, kDebugLoad, "%i of %d: type: %s (%d)  p so far: %p", i, ref->num_levels() - 1, objectType2str(ref->object_type(i)), ref->object_type(i), (void *)p);

		switch (ref->object_type(i)) {
		case QD_NAMED_OBJECT_GENERIC:
			return nullptr;
		case QD_NAMED_OBJECT_TRIGGER_CHAIN:
			p = get_trigger_chain(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_SCALE_INFO:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_SCENE) {
				qdGameDispatcherBase *bp = static_cast<qdGameDispatcherBase *>(p);
				p = bp->get_scale_info(ref->object_name(i));
			} else
				p = get_scale_info(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_SOUND:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_SCENE) {
				qdGameDispatcherBase *bp = static_cast<qdGameDispatcherBase *>(p);
				p = bp->get_sound(ref->object_name(i));
			} else
				p = get_sound(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_ANIMATION:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_SCENE) {
				qdGameDispatcherBase *bp = static_cast<qdGameDispatcherBase *>(p);
				p = bp->get_animation(ref->object_name(i));
			} else
				p = get_animation(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_ANIMATION_SET:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_SCENE) {
				qdGameDispatcherBase *bp = static_cast<qdGameDispatcherBase *>(p);
				p = bp->get_animation_set(ref->object_name(i));
			} else
				p = get_animation_set(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_ANIMATION_INFO:
			return nullptr;
		case QD_NAMED_OBJECT_OBJ_STATE:
			if (p && (p->named_object_type() == QD_NAMED_OBJECT_ANIMATED_OBJ || p->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ || p->named_object_type() == QD_NAMED_OBJECT_MOUSE_OBJ)) {
				qdGameObjectAnimated *ap = static_cast<qdGameObjectAnimated *>(p);
				p = ap->get_state(ref->object_name(i));
			} else
				return nullptr;
			break;
		case QD_NAMED_OBJECT_STATIC_OBJ:
		case QD_NAMED_OBJECT_ANIMATED_OBJ:
		case QD_NAMED_OBJECT_MOVING_OBJ:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_SCENE) {
				qdGameScene *sp = static_cast<qdGameScene *>(p);
				p = sp->get_object(ref->object_name(i));
			} else
				p = get_global_object(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_GRID_ZONE:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_SCENE) {
				qdGameScene *sp = static_cast<qdGameScene *>(p);
				p = sp->get_grid_zone(ref->object_name(i));
			} else
				return nullptr;
			break;
		case QD_NAMED_OBJECT_GRID_ZONE_STATE:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_GRID_ZONE) {
				qdGridZone *zp = static_cast<qdGridZone *>(p);
				p = zp->get_state(ref->object_name(i));
			} else
				return nullptr;
			break;
		case QD_NAMED_OBJECT_MOUSE_OBJ:
			p = _mouse_obj;
			break;
		case QD_NAMED_OBJECT_SCENE:
			p = get_scene(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_VIDEO:
			p = get_video(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_LOCATION:
			break;
		case QD_NAMED_OBJECT_MINIGAME:
			p = get_minigame(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_MUSIC_TRACK:
			if (p && p->named_object_type() == QD_NAMED_OBJECT_SCENE) {
				qdGameScene *sp = static_cast<qdGameScene *>(p);
				p = sp->get_music_track(ref->object_name(i));
			} else
				return nullptr;
			break;
		case QD_NAMED_OBJECT_INVENTORY:
			p = get_inventory(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_GAME_END:
			p = get_game_end(ref->object_name(i));
			break;
		case QD_NAMED_OBJECT_COUNTER:
			p = get_counter(ref->object_name(i));
			break;
		}
	}

	return p;
}

bool qdGameDispatcher::init_triggers() {
	bool result = true;

	for (auto &it : trigger_chain_list()) {
		if (!it->init_elements())
			result = false;
#ifdef __QD_DEBUG_ENABLE__
		if (qdGameConfig::get_config().triggers_debug())
			it->init_debug_check();
#endif
	}

	return result;
}

bool qdGameDispatcher::reset_triggers() {
	for (qdTriggerChainList::const_iterator it = trigger_chain_list().begin(); it != trigger_chain_list().end(); ++it)
		(*it)->reset();

	return true;
}

bool qdGameDispatcher::check_condition(qdCondition *cnd) {
	switch (cnd->type()) {
	case qdCondition::CONDITION_TRUE:
		return true;
	case qdCondition::CONDITION_FALSE:
		return false;
	case qdCondition::CONDITION_MOUSE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG)) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdNamedObject *p = cnd->get_object(qdCondition::OBJECT_NAME);

			if (!p) {
				const char *object_name;
				if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
					if (cnd->owner())
						p = cnd->owner()->owner();
				} else
					p = get_object(object_name);

				if (!p) return false;
			}

			qdGameScene *sc = get_active_scene();
			if (!sc) return false;

			if (sc->mouse_click_object()) {
				if (p == sc->mouse_click_object())
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG)) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdNamedObject *p = cnd->get_object(qdCondition::OBJECT_NAME);
			if (!p) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc) return false;

			if (sc->mouse_right_click_object()) {
				if (p == sc->mouse_right_click_object())
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_OBJECT_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !_mouse_click_obj) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdNamedObject *p = cnd->get_object(qdCondition::OBJECT_NAME);
			if (!p) {
				const char *object_name;
				if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
					if (cnd->owner())
						p = cnd->owner()->owner();
				} else
					p = get_object(object_name);

				if (!p) return false;
			}

			qdGameScene *sc = get_active_scene();
			if (!sc) return false;

			if (p == sc->mouse_click_object()) {
				const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::MOUSE_OBJECT_NAME));
				if (!m_obj) {
					const char *object_name;
					if (!cnd->get_value(qdCondition::MOUSE_OBJECT_NAME, object_name) || !strlen(object_name))
						return false;

					m_obj = get_object(object_name);
					if (!m_obj) return false;
				}

				if (m_obj == _mouse_click_obj)
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_OBJECT_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !_mouse_click_obj) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdNamedObject *p = cnd->get_object(qdCondition::OBJECT_NAME);
			if (!p) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc) return false;

			if (p == sc->mouse_right_click_object()) {
				const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::MOUSE_OBJECT_NAME));
				if (m_obj == _mouse_click_obj)
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_OBJECT_IN_ZONE: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject *>(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (!obj->is_visible())
			return false;

		const qdGridZone *zone = NULL;
		if (const qdNamedObject *zone_obj = cnd->get_object(qdCondition::ZONE_NAME)) {
			if (zone_obj->named_object_type() != QD_NAMED_OBJECT_GRID_ZONE)
				return false;
			zone = dynamic_cast<const qdGridZone *>(zone_obj);
		} else {
			qdGameScene *sc = get_active_scene();
			if (!sc) return false;

			const char *zone_name;
			if (!cnd->get_value(qdCondition::ZONE_NAME, zone_name))
				return false;

			zone = sc->get_grid_zone(zone_name);
		}

		if (!zone) return false;
		return zone->is_object_in_zone(obj);
	}
	return false;
	case qdCondition::CONDITION_PERSONAGE_WALK_DIRECTION: {
		const qdGameObjectMoving *p = dynamic_cast<const qdGameObjectMoving *>(cnd->get_object(qdCondition::PERSONAGE_NAME));
		if (!p) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::PERSONAGE_NAME, object_name) || !strlen(object_name)) {
				if (!cnd->owner() || !cnd->owner()->owner()) return false;
				p = dynamic_cast<const qdGameObjectMoving *>(cnd->owner()->owner());
			} else
				p = dynamic_cast<const qdGameObjectMoving *>(get_object(object_name));

			if (!p) return false;
		}

		if (!p->is_visible())
			return false;

		float angle = 0.0f;
		if (!cnd->get_value(qdCondition::DIRECTION_ANGLE, angle)) return false;

		int dir = p->get_direction(p->direction_angle());

		if (!p->check_flag(QD_OBJ_MOVING_FLAG) || dir == -1 || dir != p->get_direction(angle))
			return false;

		return true;
		}
	case qdCondition::CONDITION_PERSONAGE_STATIC_DIRECTION: {
		const qdGameObjectMoving *p = dynamic_cast<const qdGameObjectMoving *>(cnd->get_object(qdCondition::PERSONAGE_NAME));
		if (!p) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::PERSONAGE_NAME, object_name) || !strlen(object_name)) {
				if (!cnd->owner() || !cnd->owner()->owner()) return false;
				p = dynamic_cast<const qdGameObjectMoving *>(cnd->owner()->owner());
			} else
				p = dynamic_cast<const qdGameObjectMoving *>(get_object(object_name));

			if (!p) return false;
		}

		if (!p->is_visible())
			return false;

		float angle = 0.0f;
		if (!cnd->get_value(qdCondition::DIRECTION_ANGLE, angle)) return false;

		int dir = p->get_direction(p->direction_angle());

		if (p->check_flag(QD_OBJ_MOVING_FLAG) || dir == -1 || dir != p->get_direction(angle))
			return false;

		return true;
		}
	case qdCondition::CONDITION_TIMER: {
		int state;
		if (!cnd->get_value(qdCondition::TIMER_RND, state, 1) || !state)
			return false;
		return true;
		}
	case qdCondition::CONDITION_MOUSE_DIALOG_CLICK: {
		if (!check_flag(DIALOG_CLICK_FLAG) || _mouse_click_obj)
			return false;
		if (cnd->owner() && cnd->owner() == _mouse_click_state)
			return true;
		return false;
		}
	case qdCondition::CONDITION_MINIGAME_STATE:
		return false;
	case qdCondition::CONDITION_OBJECT_STATE: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject *>(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated *p = dynamic_cast<const qdGameObjectAnimated *>(obj)) {
			if (!p->is_visible())
				return false;

			if (const qdGameObjectState *sp = dynamic_cast<const qdGameObjectState *>(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->is_state_active(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->is_state_active(state_name);
			}
		}
		return false;
		}
	case qdCondition::CONDITION_MOUSE_ZONE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG) || _mouse_click_obj) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object()) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));

			if (!zone) {
				const char *zone_name;
				if (!cnd->get_value(qdCondition::CLICK_ZONE_NAME, zone_name))
					return false;

				zone = sc->get_grid_zone(zone_name);
				if (!zone) return false;
			}

			return zone->is_point_in_zone(sc->mouse_click_pos());
		}
		return false;
	case qdCondition::CONDITION_MOUSE_OBJECT_ZONE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !_mouse_click_obj) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object()) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));

			if (!zone) {
				const char *zone_name;
				if (!cnd->get_value(qdCondition::CLICK_ZONE_NAME, zone_name))
					return false;

				zone = sc->get_grid_zone(zone_name);
				if (!zone) return false;
			}

			if (zone->is_point_in_zone(sc->mouse_click_pos())) {
				const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::MOUSE_OBJECT_NAME));
				if (!m_obj) {
					const char *object_name;
					if (!cnd->get_value(qdCondition::MOUSE_OBJECT_NAME, object_name) || !strlen(object_name))
						return false;

					m_obj = get_object(object_name);
					if (!m_obj) return false;
				}

				if (m_obj == _mouse_click_obj)
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_OBJECT_STATE_WAS_ACTIVATED: {
		const qdGameObject *obj = static_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = static_cast<const qdGameObject * >(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated *p = dynamic_cast<const qdGameObjectAnimated *>(obj)) {
			if (const qdGameObjectState *sp = static_cast<const qdGameObjectState *>(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->was_state_active(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->was_state_active(state_name);
			}
		}
		return false;
		}
	case qdCondition::CONDITION_OBJECTS_DISTANCE: {
		const char *object_name;

		const qdGameObject *obj1 = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj1) {
			if (cnd->get_value(qdCondition::OBJECT_NAME, object_name) && strlen(object_name))
				obj1 = get_object(object_name);
			if (!obj1) return false;
		}

		if (!obj1->is_visible())
			return false;

		const qdGameObject *obj2 = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT2_NAME));
		if (!obj2) {
			if (cnd->get_value(qdCondition::OBJECT2_NAME, object_name) && strlen(object_name))
				obj2 = get_object(object_name);
			if (!obj2) return false;
		}

		if (!obj2->is_visible())
			return false;

		float dist = 0.0f;
		if (!cnd->get_value(qdCondition::OBJECTS_DISTANCE, dist)) return false;

		Vect3f dr = obj2->R() - obj1->R();
		dr.z = 0.0f;

		if (dr.norm2() < dist * dist)
			return true;

		return false;
		}
	case qdCondition::CONDITION_PERSONAGE_ACTIVE:
		if (get_active_personage()) {
			const qdGameObjectMoving *p = dynamic_cast<const qdGameObjectMoving *>(cnd->get_object(qdCondition::PERSONAGE_NAME));
			if (!p) {
				const char *object_name;
				if (!cnd->get_value(qdCondition::PERSONAGE_NAME, object_name) || !strlen(object_name)) {
					if (!cnd->owner() || !cnd->owner()->owner()) return false;
					p = dynamic_cast<const qdGameObjectMoving *>(cnd->owner()->owner());
				} else
					p = dynamic_cast<const qdGameObjectMoving *>(get_object(object_name));

				if (!p) return false;
			}

			if (p == get_active_personage()) return true;
		}
		return false;
	case qdCondition::CONDITION_OBJECT_STATE_WAITING: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject *>(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated *p = dynamic_cast<const qdGameObjectAnimated *>(obj)) {
			if (!p->is_visible())
				return false;

			if (const qdGameObjectState *sp = dynamic_cast<const qdGameObjectState *>(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->is_state_waiting(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->is_state_waiting(state_name);
			}
		}
		return false;
		}
	case qdCondition::CONDITION_OBJECT_STATE_ANIMATION_PHASE: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject *>(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated *p = dynamic_cast<const qdGameObjectAnimated *>(obj)) {
			if (!p->is_visible())
				return false;

			const qdGameObjectState *sp = dynamic_cast<const qdGameObjectState *>(cnd->get_object(qdCondition::OBJECT_STATE_NAME));
			if (!sp) {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				sp = p->get_state(state_name);
			}

			if (!sp || !p->is_state_active(sp)) return false;

			float phase0, phase1;
			if (!cnd->get_value(qdCondition::ANIMATION_PHASE, phase0, 0))
				return false;
			if (!cnd->get_value(qdCondition::ANIMATION_PHASE, phase1, 1))
				return false;

			float phase = p->get_animation()->cur_time_rel();
			if (phase >= phase0 && phase <= phase1)
				return true;
		}
		return false;
		}
	case qdCondition::CONDITION_OBJECT_PREV_STATE: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject *>(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated *p = dynamic_cast<const qdGameObjectAnimated *>(obj)) {
			if (!p->is_visible())
				return false;

			if (const qdGameObjectState *sp = dynamic_cast<const qdGameObjectState *>(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->was_state_previous(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->was_state_previous(state_name);
			}
		}
		return false;
		}
	case qdCondition::CONDITION_STATE_TIME_GREATER_THAN_VALUE:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_OBJ_STATE) return false;
			const qdGameObjectState *sp = static_cast<const qdGameObjectState *>(p);

			if (!sp->is_active()) return false;

			float time;
			if (!cnd->get_value(0, time, 0))

				if (sp->cur_time() > time)
					return true;
		}
		return false;
	case qdCondition::CONDITION_STATE_TIME_GREATER_THAN_STATE_TIME:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_OBJ_STATE) return false;
			const qdGameObjectState *sp0 = static_cast<const qdGameObjectState *>(p);

			if (!sp0->is_active()) return false;

			p = cnd->get_object(1);
			if (p->named_object_type() != QD_NAMED_OBJECT_OBJ_STATE) return false;
			const qdGameObjectState *sp1 = static_cast<const qdGameObjectState *>(p);

			if (!sp1->is_active()) return false;

			return (sp0->cur_time() > sp1->cur_time());
		}
		return false;
	case qdCondition::CONDITION_STATE_TIME_IN_INTERVAL:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_OBJ_STATE) return false;
			const qdGameObjectState *sp = static_cast<const qdGameObjectState *>(p);

			if (!sp->is_active()) return false;

			float time0, time1;
			if (!cnd->get_value(0, time0, 0))
				return false;
			if (!cnd->get_value(0, time1, 1))
				return false;

			if (sp->cur_time() >= time0 && sp->cur_time() <= time1)
				return true;
		}
		return false;
	case qdCondition::CONDITION_COUNTER_GREATER_THAN_VALUE:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp = static_cast<const qdCounter *>(p);

			int value;
			if (!cnd->get_value(0, value))
				return false;

			return (cp->value() > value);
		}
		return false;
	case qdCondition::CONDITION_COUNTER_LESS_THAN_VALUE:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp = static_cast<const qdCounter *>(p);

			int value;
			if (!cnd->get_value(0, value))
				return false;

			return (cp->value() < value);
		}
		return false;
	case qdCondition::CONDITION_COUNTER_GREATER_THAN_COUNTER:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp0 = static_cast<const qdCounter *>(p);

			p = cnd->get_object(1);
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp1 = static_cast<const qdCounter *>(p);

			return (cp0->value() > cp1->value());
		}
		return false;
	case qdCondition::CONDITION_COUNTER_IN_INTERVAL:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp = static_cast<const qdCounter *>(p);

			int value0, value1;
			if (!cnd->get_value(0, value0, 0))
				return false;
			if (!cnd->get_value(0, value1, 1))
				return false;

			return (cp->value() >= value0 && cp->value() <= value1);
		}
		return false;
	case qdCondition::CONDITION_OBJECT_ON_PERSONAGE_WAY:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_MOVING_OBJ) return false;
			const qdGameObjectMoving *obj = static_cast<const qdGameObjectMoving *>(p);
			if (!obj->is_visible()) return false;

			p = cnd->get_object(1);
			if (!p) return false;
			const qdGameObject *obj1 = dynamic_cast<const qdGameObject *>(p);
			if (!obj1 || !obj1->is_visible()) return false;

			float dist = 0.0f;
			if (!cnd->get_value(0, dist)) return false;

			Vect3f dr = obj->R() - obj1->R();
			dr.z = 0.0f;

			if (dr.norm2() > dist * dist) return false;

			float angle = obj->calc_direction_angle(obj1->R());

			if (fabs(angle - obj->direction_angle()) < M_PI / 2.0f) return true;
		}
		return false;
	case qdCondition::CONDITION_KEYPRESS: {
		int vkey;
		if (!cnd->get_value(0, vkey)) return false;
		return keyboardDispatcher::instance()->is_pressed(vkey);
	}
	return false;
	case qdCondition::CONDITION_ANY_PERSONAGE_IN_ZONE:
		if (const qdNamedObject *p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_GRID_ZONE) return false;
			return static_cast<const qdGridZone *>(p)->is_any_personage_in_zone();
		}
		return false;
	case qdCondition::CONDITION_OBJECT_HIDDEN: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) return false;
		return !obj->is_visible();
		}
	case qdCondition::CONDITION_MOUSE_RIGHT_ZONE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG) || _mouse_click_obj) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object() || sc->mouse_right_click_object()) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));
			if (!zone)
				return false;

			return zone->is_point_in_zone(sc->mouse_click_pos());
		}
		return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_OBJECT_ZONE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !_mouse_click_obj) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object() || sc->mouse_right_click_object()) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));
			if (!zone) return false;

			if (zone->is_point_in_zone(sc->mouse_click_pos())) {
				const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::MOUSE_OBJECT_NAME));
				if (m_obj && m_obj == _mouse_click_obj)
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_HOVER:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN) && !mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			const qdNamedObject *p = cnd->get_object(qdCondition::OBJECT_NAME);

			if (!p) {
				if (cnd->owner())
					p = cnd->owner()->owner();

				if (!p) return false;
			}

			qdGameScene *sc = get_active_scene();
			if (!sc) return false;

			if (sc->mouse_hover_object()) {
				if (p == sc->mouse_hover_object())
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_OBJECT_HOVER:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN) && !mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			const qdNamedObject *p = cnd->get_object(qdCondition::OBJECT_NAME);

			if (!p) {
				if (cnd->owner())
					p = cnd->owner()->owner();

				if (!p) return false;
			}

			qdGameScene *sc = get_active_scene();
			if (!sc) return false;

			if (sc->mouse_hover_object()) {
				if (p == sc->mouse_hover_object()) {
					const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::MOUSE_OBJECT_NAME));
					if (m_obj && m_obj == _mouse_obj->object())
						return true;
				}
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_HOVER_ZONE:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN) && !mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (_mouse_obj->object()) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object()) return false;

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));
			if (!zone) return false;

			return zone->is_point_in_zone(sc->mouse_click_pos());
		}
		return false;
	case qdCondition::CONDITION_MOUSE_OBJECT_HOVER_ZONE:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN) && !mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!_mouse_obj->object()) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object()) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));
			if (!zone) return false;

			if (zone->is_point_in_zone(sc->mouse_click_pos())) {
				const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::MOUSE_OBJECT_NAME));
				if (m_obj && m_obj == _mouse_obj->object())
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_CLICK_FAILED:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			return check_flag(CLICK_WAS_FAILED_FLAG);
		}
		return false;
	case qdCondition::CONDITION_MOUSE_OBJECT_CLICK_FAILED:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			return check_flag(OBJECT_CLICK_WAS_FAILED_FLAG);
		}
		return false;
	case qdCondition::CONDITION_MOUSE_CLICK_EVENT:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG)) return false;

			return true;
		}
		return false;
	case qdCondition::CONDITION_MOUSE_OBJECT_CLICK_EVENT:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !_mouse_click_obj) return false;

			const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(0));
			return (!m_obj || m_obj == _mouse_click_obj);

			return true;
		}
		return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_CLICK_EVENT:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG)) return false;
			return true;
		}
		return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_OBJECT_CLICK_EVENT:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !_mouse_click_obj) return false;

			const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(0));
			return (!m_obj || m_obj == _mouse_click_obj);

			return true;
		}
		return false;
	case qdCondition::CONDITION_MOUSE_STATE_PHRASE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (check_flag(DIALOG_CLICK_FLAG) && !_mouse_click_obj) {
				const qdGameObjectState *p = dynamic_cast<const qdGameObjectState *>(cnd->get_object(0));
				if (!p) return false;

				return (p == _mouse_click_state);
			}
		}
		return false;
	case qdCondition::CONDITION_OBJECT_IS_CLOSER: {
		const qdGameObject *obj0 = dynamic_cast<const qdGameObject *>(cnd->get_object(0));
		if (!obj0) {
			if (cnd->owner())
				obj0 = dynamic_cast<const qdGameObject *>(cnd->owner()->owner());
		}
		if (!obj0) return false;

		const qdGameObject *obj1 = dynamic_cast<const qdGameObject *>(cnd->get_object(1));
		if (!obj1) return false;

		const qdGameObject *obj2 = dynamic_cast<const qdGameObject *>(cnd->get_object(2));
		if (!obj2) return false;

		Vect3f dr1 = obj1->R() - obj0->R();
		Vect3f dr2 = obj2->R() - obj0->R();

		dr1.z = dr2.z = 0;

		return (dr1.norm2() < dr2.norm2());
		}
	case qdCondition::CONDITION_ANIMATED_OBJECT_IDLE_GREATER_THAN_VALUE: {
		const qdGameObjectAnimated *anim_obj =
		    dynamic_cast<const qdGameObjectAnimated *>(cnd->get_object(0));

		if (NULL == anim_obj) return false;

		int value;
		if (!cnd->get_value(0, value))
			return false;

		return (anim_obj->idle_time() > value);
		}
	return false;
	case qdCondition::CONDITION_ANIMATED_OBJECTS_INTERSECTIONAL_BOUNDS: {
		const qdGameObjectAnimated *anim1 =
		    dynamic_cast<const qdGameObjectAnimated *>(cnd->get_object(0));
		const qdGameObjectAnimated *anim2 =
		    dynamic_cast<const qdGameObjectAnimated *>(cnd->get_object(1));

		if ((NULL == anim1) || (NULL == anim2))
			return false;

		return anim1->inters_with_bound(anim2->bound(), anim2->R());
		}
	default:
		break;
	}
	return false;
}

bool qdGameDispatcher::play_video(const char *vid_name) {
	qdVideo *p = get_video(vid_name);
	if (p) return play_video(p);

	return false;
}

bool qdGameDispatcher::play_video(qdVideo *p) {
	if (!_video_player.open_file(find_file(p->file_name(), *p)))
		return false;

	if (!p->check_flag(qdVideo::VID_ENABLE_MUSIC)) {
		mpegPlayer::instance().pause();
	} else {
		warning("STUB: qdGameDispatcher::play_video(): music is enabled");
	}

	_cur_video = p;

	if (p->check_flag(qdVideo::VID_FULLSCREEN_FLAG)) {
		_video_player.set_window(0, 0, g_engine->_screenW, g_engine->_screenH);
	} else {
		int sx, sy;
		_video_player.get_movie_size(sx, sy);

		if (p->check_flag(qdVideo::VID_CENTER_FLAG)) {
			int x = (g_engine->_screenW - sx) >> 1;
			int y = (g_engine->_screenH - sy) >> 1;

			_video_player.set_window(x, y, sx, sy);
		} else
			_video_player.set_window(p->position().x, p->position().y, sx, sy);
	}

	return _video_player.play();
}

bool qdGameDispatcher::pause_video() {
	if (!_cur_video) return false;
	return false;
}

bool qdGameDispatcher::stop_video() {
	if (!_cur_video) return false;
	return _video_player.stop();
}

bool qdGameDispatcher::close_video() {
	if (!_cur_video)
		return false;

	_video_player.stop();
	_video_player.close_file();

	if (check_flag(INTRO_MODE_FLAG)) {
		qdVideoList::const_iterator it = Common::find(video_list().begin(), video_list().end(), _cur_video);
		if (it != video_list().end()) ++it;
		for (; it != video_list().end(); ++it) {
			if ((*it)->is_intro_movie()) {
				play_video(*it);
				return true;
			}
		}

		drop_flag(INTRO_MODE_FLAG);
	}

	if (mpegPlayer::instance().is_enabled())
		mpegPlayer::instance().resume();

	_cur_video = NULL;

	if (sndDispatcher *sp = sndDispatcher::get_dispatcher())
		sp->resume_sounds();

	if (!_interface_dispatcher.is_active())
		resume();

	set_flag(SKIP_REDRAW_FLAG);

	return true;
}

bool qdGameDispatcher::is_video_finished() {
	if (!_cur_video) return false;
	return _video_player.is_playback_finished();
}

void qdGameDispatcher::continueVideo() {
	if (!_cur_video) return;
	_video_player.quant();
}

bool qdGameDispatcher::merge_global_objects(qdGameObject *obj) {
	for (auto &is : scene_list()) {
		is->merge_global_objects(obj);
	}

	return true;
}

bool qdGameDispatcher::update_walk_state(const char *object_name, qdGameObjectState *p) {
	if (qdGameObject *obj = get_global_object(object_name)) {
		if (obj->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			static_cast<qdGameObjectMoving *>(obj)->set_last_walk_state(p);
	}

	return false;
}

qdGameObjectState *qdGameDispatcher::get_walk_state(const char *object_name) {
	if (qdGameObject *obj = get_global_object(object_name)) {
		if (obj->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			return static_cast<qdGameObjectMoving *>(obj)->last_walk_state();
	}

	return NULL;
}

bool qdGameDispatcher::split_global_objects(qdGameObject *obj) {
	for (auto &is : scene_list()) {
		is->split_global_objects(obj);
	}

	return true;
}

bool qdGameDispatcher::init_inventories() {
	bool result = true;
	for (auto &it : inventory_list()) {
		if (!it->init(_inventory_cell_types)) {
			result = false;
		}
	}
	return result;
}

bool qdGameDispatcher::toggle_inventory(bool state) {
	toggle_full_redraw();

	drop_mouse_object();

	if (state) {
		qdGameObjectMoving *p = get_active_personage();
		if (p && strlen(p->inventory_name())) {
			_cur_inventory = get_inventory(p->inventory_name());
			if (_cur_inventory) {
				update_ingame_interface();
				return true;
			}
		}
	}

	_cur_inventory = NULL;
	update_ingame_interface();
	return true;
}

bool qdGameDispatcher::drop_mouse_object() {
	if (_mouse_obj->object()) {
		/*      if(!_cur_inventory){
		            if(!toggle_inventory(true))
		                return false;
		        }*/

		if (!_cur_inventory)
			return false;

		qdGameObjectAnimated *obj = _mouse_obj->object();
		_mouse_obj->take_object(NULL);
		_cur_inventory->put_object(obj);
	}

	return true;
}

bool qdGameDispatcher::put_to_inventory(qdGameObjectAnimated *p) {
	if (is_in_inventory(p)) return false;

	qdInventory *inv = NULL;

	if (!p->has_inventory_name()) {
		qdGameObjectMoving *pe = get_active_personage();
		if (!pe || !pe->has_inventory_name()) return false;
		inv = get_inventory(pe->inventory_name());
	} else
		inv = get_inventory(p->inventory_name());

	if (inv && inv->put_object(p)) {
		if (qdGameObjectState *sp = p->get_inventory_state())
			p->set_state(sp);

		if (!inv->check_flag(qdInventory::INV_DONT_OPEN_AFTER_TAKE)) {
			if (!_cur_inventory) toggle_inventory(true);

			if (inv->check_flag(qdInventory::INV_TAKE_TO_MOUSE)) {
				if (_cur_inventory == inv) {
					if (_mouse_obj->object()) {
						qdGameObjectAnimated *obj = _mouse_obj->object();
						_mouse_obj->take_object(NULL);
						_cur_inventory->put_object(obj);
					}
					_cur_inventory->remove_object(p);
					_mouse_obj->take_object(p);
				}
			}
		}
		toggle_full_redraw();
		return true;
	}

	return false;
}

bool qdGameDispatcher::is_in_inventory(const qdGameObjectAnimated *p) const {
	for (auto &it : inventory_list()) {
		if (it->is_object_in_list(p)) return true;
	}

	return false;
}

bool qdGameDispatcher::remove_from_inventory(qdGameObjectAnimated *p) {
	if (_mouse_obj->object() == p) {
		_mouse_obj->take_object(NULL);
		p->drop_flag(QD_OBJ_IS_IN_INVENTORY_FLAG);
		return true;
	}

	for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it) {
		if ((*it)->is_object_in_list(p)) {
			(*it)->remove_object(p);
			return true;
		}
	}

	return false;
}

bool qdGameDispatcher::rename_inventory(qdInventory *p, const char *name) {
	return _inventories.rename_object(p, name);
}

bool qdGameDispatcher::add_video(qdVideo *p, qdVideo const *before) {
		if (_videos.add_object(p)) {
			p->set_owner(this);
			return true;
		}

	return false;
}

bool qdGameDispatcher::is_video_in_list(const char *name) {
	return _videos.is_in_list(name);
}

bool qdGameDispatcher::is_video_in_list(qdVideo *p) {
	return _videos.is_in_list(p);
}

bool qdGameDispatcher::remove_video(const char *name) {
	return _videos.remove_object(name);
}

bool qdGameDispatcher::remove_video(qdVideo *p) {
	return _videos.remove_object(p);
}

bool qdGameDispatcher::rename_video(qdVideo *p, const char *name) {
	return _videos.rename_object(p, name);
}

qdVideo *qdGameDispatcher::get_video(const char *name) {
	return _videos.get_object(name);
}

bool qdGameDispatcher::select_scene(qdGameScene *sp, bool resources_flag) {
	int tm = g_system->getMillis();

	toggle_full_redraw();

	_screen_texts.clear_texts();

	if (!sp || get_active_scene() != sp) {
			debugC(3, kDebugQuant, "qdGameDispatcher::select_scene() Stop sound");
		if (sndDispatcher *p = sndDispatcher::get_dispatcher())
			p->stop_sounds();
	}

	if (sp) {
		debugC(3, kDebugQuant, "qdGameDispatcher::select_scene() request_file_package");
		request_file_package(*sp);
	}

	drop_mouse_object();
	toggle_inventory(true);

	if (_cur_scene) {
		if (_cur_scene != sp)
			_cur_scene->free_resources();

		_cur_scene->deactivate();
	}

	_scene_saved = false;

	_cur_scene = sp;
	qdCamera::set_current_camera(NULL);

	toggle_inventory(true);

	debug("select_scene('%s', %d)", sp ? (const char *)transCyrillic(sp->name()) : "<no name>", resources_flag);

	if (_cur_scene) {
		debugC(3, kDebugQuant, "qdGameDispatcher::select_scene() set_current_camera");
		qdCamera::set_current_camera(_cur_scene->get_camera());
		_cur_scene->activate();

		if (resources_flag)
			_cur_scene->load_resources();

		update_ingame_interface();
		_cur_scene->start_minigame();
		_interface_dispatcher.update_personage_buttons();
	}

	if (resources_flag) {
		if (_mouse_obj->max_state()) {
			_mouse_obj->free_resources();
			_mouse_obj->load_resources();
		} else
			_mouse_animation->load_resources();

		for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it)
			(*it)->load_resources();
	}

	tm = g_system->getMillis() - tm;
	if (_cur_scene)
		debugC(1, kDebugLoad, "Scene loading \"%s\" %d ms", transCyrillic(_cur_scene->name()), tm);

	return true;
}

qdGameObject *qdGameDispatcher::get_object(const char *name) {
	if (_cur_scene)
		return _cur_scene->get_object(name);

	return NULL;
}

qdGameObjectMoving *qdGameDispatcher::get_active_personage() {
	if (_cur_scene) return _cur_scene->get_active_personage();

	return NULL;
}

qdScaleInfo *qdGameDispatcher::get_scale_info(const char *p) {
	if (_cur_scene) {
		qdScaleInfo *si = _cur_scene->get_scale_info(p);
		if (si) return si;
	}

	return qdGameDispatcherBase::get_scale_info(p);
}

bool qdGameDispatcher::add_trigger_chain(qdTriggerChain *p) {
	if (_trigger_chains.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::remove_trigger_chain(const char *name) {
	return _trigger_chains.remove_object(name);
}

qdTriggerChain *qdGameDispatcher::get_trigger_chain(const char *name) {
	return _trigger_chains.get_object(name);
}

bool qdGameDispatcher::remove_trigger_chain(qdTriggerChain *p) {
	return _trigger_chains.remove_object(p);
}

bool qdGameDispatcher::is_trigger_chain_in_list(const char *name) {
	return _trigger_chains.is_in_list(name);
}

bool qdGameDispatcher::is_trigger_chain_in_list(qdTriggerChain *p) {
	return _trigger_chains.is_in_list(p);
}

bool qdGameDispatcher::rename_trigger_chain(qdTriggerChain *p, const char *name) {
	return _trigger_chains.rename_object(p, name);
}

bool qdGameDispatcher::add_global_object(qdGameObject *p) {
	if (_global_objects.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_global_object(qdGameObject *p, const char *name) {
	return _global_objects.rename_object(p, name);
}

bool qdGameDispatcher::remove_global_object(const char *name) {
	return _global_objects.remove_object(name);
}

qdGameObject *qdGameDispatcher::get_global_object(const char *name) {
	return _global_objects.get_object(name);
}

bool qdGameDispatcher::remove_global_object(qdGameObject *p) {
	return _global_objects.remove_object(p);
}

bool qdGameDispatcher::is_global_object_in_list(const char *name) {
	return _global_objects.is_in_list(name);
}

bool qdGameDispatcher::is_global_object_in_list(qdGameObject *p) {
	return _global_objects.is_in_list(p);
}

bool qdGameDispatcher::add_minigame(qdMiniGame *p) {
	if (_minigames.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_minigame(qdMiniGame *p, const char *name) {
	return _minigames.rename_object(p, name);
}

bool qdGameDispatcher::remove_minigame(const char *name) {
	return _minigames.remove_object(name);
}

qdMiniGame *qdGameDispatcher::get_minigame(const char *name) {
	return _minigames.get_object(name);
}

bool qdGameDispatcher::remove_minigame(qdMiniGame *p) {
	return _minigames.remove_object(p);
}

bool qdGameDispatcher::is_minigame_in_list(const char *name) {
	return _minigames.is_in_list(name);
}

bool qdGameDispatcher::is_minigame_in_list(qdMiniGame *p) {
	return _minigames.is_in_list(p);
}

bool qdGameDispatcher::keyboard_handler(Common::KeyCode vkey, bool event) {
	if (is_paused()) {
		if (event) {
			switch (vkey) {
			case Common::KEYCODE_p:
				resume();
				return true;
			case Common::KEYCODE_SPACE:
				set_flag(NEXT_FRAME_FLAG);
				return true;
			default:
				break;
			}
		}

		if (is_video_playing() && event) {
			if (!_cur_video->check_flag(qdVideo::VID_DISABLE_INTERRUPT_FLAG)) {
				close_video();
				return true;
			}
		}

		if (_interface_dispatcher.is_active() && event)
			return _interface_dispatcher.keyboard_handler(vkey);

		return false;
	}

	if (event) {
		if (_interface_dispatcher.keyboard_handler(vkey))
			return true;

		switch (vkey) {
		case Common::KEYCODE_ESCAPE:
			if (is_main_menu_exit_enabled())
				return toggle_main_menu(true);
			break;
		case Common::KEYCODE_SPACE:
			if (qdGameScene *sp = get_active_scene()) {
				if (!sp->check_flag(qdGameScene::DISABLE_KEYBOARD_PERSONAGE_SWITCH))
					sp->change_active_personage();
				return true;
			}
			break;
		case Common::KEYCODE_F9:
			g_engine->_debugDraw = !g_engine->_debugDraw;
			toggle_full_redraw();
			return true;
		case Common::KEYCODE_F8:
			g_engine->_debugDrawGrid = !g_engine->_debugDrawGrid;
			toggle_full_redraw();

			qdCamera::current_camera()->dump_grid("qd_grid.txt");
			warning("Grid dumped");
			return true;
		case Common::KEYCODE_p:
			pause();
			return true;
#ifdef __QD_DEBUG_ENABLE__
		case Common::KEYCODE_F10:
			write_resource_stats("memory_usage.html");
			return true;
		case Common::KEYCODE_F5:
			pause();
			g_engine->saveGameState(_autosave_slot, "Autosave", true);
			resume();
			return true;
		case Common::KEYCODE_F6:
			g_engine->loadGameState(_autosave_slot);
			return true;
#endif
		default:
			break;
		}
	}

	return false;
}

bool qdGameDispatcher::load_save(Common::SeekableReadStream *fh) {
	if (sndDispatcher *p = sndDispatcher::get_dispatcher()) {
		p->stop_sounds();
		p->pause();
	}

	pause();

	free_resources();

	int32 save_version;
	save_version = fh->readSint32LE();

	qdNamedObjectReference ref;

	if (!ref.load_data(*fh, save_version)) return false;

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): active_scene %ld", fh->pos());
	qdGameScene *cur_scene_ptr = static_cast<qdGameScene *>(get_named_object(&ref));
	select_scene(0, false);

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): music %ld", fh->pos());
	if (!ref.load_data(*fh, save_version)) return false;
	if (qdMusicTrack *p = static_cast<qdMusicTrack *>(get_named_object(&ref))) {
		_cur_music_track = 0;
		play_music_track(p);
	}

	int32 flag = fh->readSint32LE();
	if (flag)
		toggle_inventory(true);
	else
		toggle_inventory(false);

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): object_list 1 %ld", fh->pos());
	uint32 size = fh->readUint32LE();
	if (size != global_object_list().size()) return false;

	for (auto &it : global_object_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): counter_list %ld", fh->pos());
	size = fh->readUint32LE();
	if (size != counter_list().size()) return false;

	for (auto &it : counter_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): scene_list %ld", fh->pos());
	size = fh->readUint32LE();
	if (size != scene_list().size()) return false;

	debugC(3, kDebugLog, "Scene list size: %u pos: %ld", scene_list().size(), fh->pos());
	for (auto &it : scene_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	debugC(3, kDebugLog, "Global object list size: %u pos: %ld", global_object_list().size(), fh->pos());

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): object_list 2 %ld", fh->pos());
	size = fh->readSint32LE();
	if (size != global_object_list().size()) return false;

	for (auto &it : global_object_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): trigger_chain_list %ld", fh->pos());
	size = fh->readSint32LE();
	if (size != trigger_chain_list().size()) return false;

	for (auto &it : trigger_chain_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): inventory_list %ld", fh->pos());
	size = fh->readSint32LE();
	if (size != inventory_list().size()) return false;

	for (auto &it : inventory_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): mouse_obj %ld", fh->pos());
	if (save_version >= 10)
		_mouse_obj->load_data(*fh, save_version);

	debugC(2, kDebugSave, "qdGameDispatcher::load_save(): TOTAL SIZE %ld", fh->pos());

	if (cur_scene_ptr)
		select_scene(cur_scene_ptr, false);

	load_resources();

	if (sndDispatcher *p = sndDispatcher::get_dispatcher())
		p->resume();

	_interface_dispatcher.update_personage_buttons();
	resume();

	_scene_saved = true;

	return true;
}

bool qdGameDispatcher::save_save(Common::WriteStream *fh) const {
	const int32 save_version = 107;
	fh->writeUint32LE(save_version);

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): active_scene %ld", fh->pos());
	if (get_active_scene()) {
		qdNamedObjectReference ref(get_active_scene());
		if (!ref.save_data(*fh)) {
			return false;
		}
	} else {
		qdNamedObjectReference ref;
		if (!ref.save_data(*fh)) {
			return false;
		}
	}

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): music %ld", fh->pos());
	if (_cur_music_track) {
		qdNamedObjectReference ref(_cur_music_track);
		if (!ref.save_data(*fh)) {
			return false;
		}
	} else {
		qdNamedObjectReference ref;
		if (!ref.save_data(*fh)) {
			return false;
		}
	}

	if (_cur_inventory)
		fh->writeSint32LE(1);
	else
		fh->writeSint32LE(0);

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): object_list 1 %ld", fh->pos());
	fh->writeUint32LE(global_object_list().size());
	for (auto &it : global_object_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): counter_list %ld", fh->pos());
	fh->writeUint32LE(counter_list().size());
	for (auto &it : counter_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): scene_list %ld", fh->pos());
	fh->writeUint32LE(scene_list().size());
	debugC(3, kDebugLog, "Scene list size: %u pos: %ld", scene_list().size(), fh->pos());
	for (auto &it : scene_list()) {
		if (!it->save_data(*fh))
			return false;
	}

	debugC(3, kDebugLog, "Global object list size: %u pos: %ld", global_object_list().size(), fh->pos());

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): object_list 2 %ld", fh->pos());
	fh->writeUint32LE(global_object_list().size());
	for (auto &it : global_object_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): trigger_chain_list %ld", fh->pos());
	fh->writeUint32LE(trigger_chain_list().size());
	for (auto &it : trigger_chain_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): inventory_list %ld", fh->pos());
	fh->writeUint32LE(inventory_list().size());
	for (auto &it : inventory_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): mouse_obj %ld", fh->pos());
	_mouse_obj->save_data(*fh);

	debugC(2, kDebugSave, "qdGameDispatcher::save_save(): TOTAL SIZE %ld", fh->pos());

	return true;
}

bool qdGameDispatcher::play_music_track(const qdMusicTrack *p, bool interface_mode) {
	debugC(3, kDebugLog, "[%d] music start->%s", g_system->getMillis(), transCyrillic(p->file_name().toString()));

	if (!interface_mode) {
		if (p->check_flag(QD_MUSIC_TRACK_DISABLE_RESTART) && _cur_music_track == p)
			return true;

		_cur_music_track = p;
		_cur_interface_music_track = NULL;
	} else {
		if (_cur_interface_music_track == p)
			return true;

		_cur_interface_music_track = p;
	}

	Common::Path fname;

	if (_cur_scene)
		fname = find_file(p->file_name(), *_cur_scene);
	else
		fname = find_file(p->file_name(), *this);

	_interface_music_mode = interface_mode;

	debugC(3, kDebugLoad, "qdGameDispatcher::play_music_track() %s", transCyrillic(fname.toString()));

	return mpegPlayer::instance().play(fname, p->is_cycled(), p->volume());
}

bool qdGameDispatcher::stop_music() {
	debugC(3, kDebugLog, "[%d] music stop", g_system->getMillis());

	if (_interface_music_mode)
		_cur_interface_music_track = NULL;
	else
		_cur_music_track = NULL;

	return mpegPlayer::instance().stop();
}

void qdGameDispatcher::pause() {
	_is_paused = true;

	if (sndDispatcher *p = sndDispatcher::get_dispatcher())
		p->pause_sounds();
}

void qdGameDispatcher::resume() {
	update_time();
	_is_paused = false;

	if (sndDispatcher *p = sndDispatcher::get_dispatcher())
		p->resume_sounds();
}

void qdGameDispatcher::set_dispatcher(qdGameDispatcher *p) {
	_dispatcher = p;

	if (p)
		qdInterfaceDispatcher::set_dispatcher(&p->_interface_dispatcher);
	else
		qdInterfaceDispatcher::set_dispatcher(NULL);
}

bool qdGameDispatcher::toggle_main_menu(bool state, const char *screen_name) {
	toggle_full_redraw();

	if (state) {
		if (_interface_dispatcher.has_main_menu() || screen_name) {
			/*          if(_cur_inventory){
			                if(_mouse_obj->object()){
			                    qdGameObjectAnimated* obj = _mouse_obj->object();
			                    _mouse_obj->take_object(NULL);
			                    _cur_inventory->put_object(obj);
			                }
			            }*/

			_mouse_obj->set_cursor(qdGameObjectMouse::MAIN_MENU_CURSOR);

			if (!screen_name)
				screen_name = _interface_dispatcher.main_menu_screen_name();

			_interface_dispatcher.select_screen(screen_name);
			_interface_dispatcher.activate();
			pause();
			return true;
		}
	} else {
		update_ingame_interface();

		_interface_dispatcher.deactivate();
		_interface_dispatcher.update_personage_buttons();

		if (_interface_music_mode) {
			debugC(3, kDebugQuant, "qdGameDispatcher::toggle_main_menu() _interface_music_mode");
			if (_cur_music_track) {
				const qdMusicTrack *tp = _cur_music_track;
				_cur_music_track = 0;
				play_music_track(tp);
			} else
				stop_music();
		}

		resume();
		return true;
	}

	return false;
}

bool qdGameDispatcher::is_main_menu_exit_enabled() const {
	return !_cur_scene || !_cur_scene->check_flag(qdGameScene::DISABLE_MAIN_MENU);
}

bool qdGameDispatcher::end_game(const qdGameEnd *p) {
	restart();

	const char *screen_name = (p->has_interface_screen()) ? p->interface_screen() : NULL;

	if (screen_name)
		_interface_dispatcher.toggle_end_game_mode(true);

	return toggle_main_menu(true, screen_name);
}

bool qdGameDispatcher::add_redraw_region(const grScreenRegion &reg) {
	return grDispatcher::instance()->invalidate_region(reg);
}

bool qdGameDispatcher::init() {
	if (sndDispatcher *sdp = sndDispatcher::get_dispatcher())
		sdp->stop_sounds();

	if (!_screen_texts.get_text_set(TEXT_SET_DIALOGS)) {
		qdScreenTextSet set;
		set.set_ID(TEXT_SET_DIALOGS);
		set.set_screen_pos(Vect2i(g_engine->_screenW / 2, g_engine->_screenH / 2));
		set.set_screen_size(Vect2i(g_engine->_screenW, g_engine->_screenH - g_engine->_screenH / 4));

		_screen_texts.add_text_set(set);
	}

	init_triggers();
	init_inventories();

	select_scene(NULL, false);
	for (qdGameSceneList::const_iterator it = scene_list().begin(); it != scene_list().end(); ++it)
		(*it)->init();

	for (qdMiniGameList::const_iterator it = minigame_list().begin(); it != minigame_list().end(); ++it)
		(*it)->init();

	for (qdVideoList::const_iterator it = video_list().begin(); it != video_list().end(); ++it)
		(*it)->init();

	for (qdCounterList::const_iterator it = counter_list().begin(); it != counter_list().end(); ++it)
		(*it)->init();

	for (qdGameObjectList::const_iterator it = global_object_list().begin(); it != global_object_list().end(); ++it)
		(*it)->init();

	//! Грузим шрифты, заданные в qdGameDispatcher::qdFontInfoList
	for (Std::list<qdFontInfo *>::const_iterator it = _fonts.get_list().begin();
	        it != _fonts.get_list().end(); ++it)
		(*it)->load_font();

	_cur_video = NULL;
	_cur_inventory = NULL;
	_next_scene = NULL;
	_cur_music_track = NULL;

	return true;
}

bool qdGameDispatcher::game_screenshot(Graphics::Surface &thumb) const {
	int w = g_engine->_screenW;
	int h = g_engine->_screenH;

	thumb.create(w, h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	if (qdGameScene *sp = get_active_scene()) {
		qdSprite sprite(w, h, GR_RGB565);

		sp->redraw();

		uint16 col;
		for (int i = 0; i < h; i++) {
			uint16 *dst = (uint16 *)thumb.getBasePtr(0, i);
			for (int j = 0; j < w; j++) {
				grDispatcher::instance()->getPixel(j, i, col);
				*dst = col;
				dst++;
			}
		}

		return true;
	}

	return false;
}

bool qdGameDispatcher::restart() {
//	free_resources();

	init();
	reset_triggers();

	toggle_full_redraw();

	select_scene(NULL, false);

	resume();

	return true;
}

bool qdGameDispatcher::add_game_end(qdGameEnd *p) {
	if (_game_ends.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_game_end(qdGameEnd *p, const char *name) {
	return _game_ends.rename_object(p, name);
}

bool qdGameDispatcher::remove_game_end(qdGameEnd *p) {
	return _game_ends.remove_object(p);
}

qdGameEnd *qdGameDispatcher::get_game_end(const char *name) {
	return _game_ends.get_object(name);
}

bool qdGameDispatcher::is_game_end_in_list(const char *name) {
	return _game_ends.is_in_list(name);
}

bool qdGameDispatcher::is_game_end_in_list(qdGameEnd *p) {
	return _game_ends.is_in_list(p);
}

bool qdGameDispatcher::is_on_mouse(const qdGameObjectAnimated *p) const {
	return (_mouse_obj->object() == p);
}

bool qdGameDispatcher::add_scene(qdGameScene *p) {
	if (_scenes.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_scene(qdGameScene *p, const char *name) {
	return _scenes.rename_object(p, name);
}

bool qdGameDispatcher::remove_scene(qdGameScene *p) {
	return _scenes.remove_object(p);
}

qdGameScene *qdGameDispatcher::get_scene(const char *name) {
	return _scenes.get_object(name);
}

bool qdGameDispatcher::is_scene_in_list(const char *name) {
	return _scenes.is_in_list(name);
}

bool qdGameDispatcher::is_scene_in_list(const qdGameScene *p) {
	return _scenes.is_in_list(p);
}

bool qdGameDispatcher::add_counter(qdCounter *p) {
	if (_counters.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_counter(qdCounter *p, const char *name) {
	return _counters.rename_object(p, name);
}

bool qdGameDispatcher::remove_counter(qdCounter *p) {
	return _counters.remove_object(p);
}

qdCounter *qdGameDispatcher::get_counter(const char *name) {
	return _counters.get_object(name);
}

bool qdGameDispatcher::is_counter_in_list(const char *name) {
	return _counters.is_in_list(name);
}

bool qdGameDispatcher::is_counter_in_list(qdCounter *p) {
	return _counters.is_in_list(p);
}

bool qdGameDispatcher::start_intro_videos() {
	for (auto &it : video_list()) {
		if (it->is_intro_movie()) {
			if (play_video(it)) {
				set_flag(INTRO_MODE_FLAG);
				return true;
			}
		}
	}

	return false;
}

int qdGameDispatcher::CD_count() const {
	int cnt = 1;
	for (int i = 1; i < 32; i++) {
		if (is_on_CD(i))
			cnt = i + 1;
	}

	return cnt;
}

void qdGameDispatcher::request_file_package(const qdFileOwner &file_owner) const {
	if (!_enable_file_packages) return;

	if (qdFileManager::instance().is_package_available(file_owner))
		return;

	error("Requested file package is not avaliable");
}

Common::Path qdGameDispatcher::find_file(const Common::Path file_name, const qdFileOwner &file_owner) const {
	debugC(4, kDebugLoad, "qdGameDispatcher::find_file(%s)", file_name.toString().c_str());

	return file_name;
}

grFont *qdGameDispatcher::create_font(int font_idx) {
	grFont *p = new grFont;

	Common::String fname;

	fname = Common::String::format("Resource/Fonts/font%02d.tga", font_idx);

	Common::SeekableReadStream *fh;

	if (qdFileManager::instance().open_file(&fh, fname.c_str(), false)) {
		if (p->load_alpha(fh)) {
			delete fh;

			fname = Common::String::format("Resource/Fonts/font%02d.idx", font_idx);

			if (qdFileManager::instance().open_file(&fh, fname.c_str(), false))
				p->load_index(fh);

			delete fh;
		}
	}

	return p;
}

void qdGameDispatcher::free_font(grFont *fnt) {
	delete fnt;
}

bool qdGameDispatcher::add_dialog_state(qdGameObjectState *p) {
	dialog_states_container_t::const_iterator it = Common::find(_dialog_states.begin(), _dialog_states.end(), p);
	if (it != _dialog_states.end()) return false;

	_dialog_states.push_back(p);

	return true;
}

bool qdGameDispatcher::activate_trigger_links(const qdNamedObject *p) {
	for (qdTriggerChainList::const_iterator it = trigger_chain_list().begin(); it != trigger_chain_list().end(); ++it)
		(*it)->activate_links(p);

	return true;
}

// Поиск шрифтов
qdFontInfo *qdGameDispatcher::find_font_info(int type) {
	for (qdFontInfoList::const_iterator it = fonts_list().begin(); it != fonts_list().end(); it++)
		if ((*it)->type() == type)
			return (*it);
	return NULL;
}

const qdFontInfo *qdGameDispatcher::find_font_info(int type) const {
	for (qdFontInfoList::const_iterator it = fonts_list().begin(); it != fonts_list().end(); it++)
		if ((*it)->type() == type)
			return (*it);
	return NULL;
}

const grFont *qdGameDispatcher::find_font(int type) const {
	const qdFontInfo *pFI = find_font_info(type);
	if (NULL != pFI)
		return pFI->font();
	else
		return NULL;
}


// Операции со шрифтами
bool qdGameDispatcher::add_font_info(qdFontInfo *fi) {
	// проверяем уникальность идентификатора, вставляемого типа
	if (NULL == find_font_info(fi->type())) {
		_fonts.add_object(fi);
		return true;
	} else return false;
}

bool qdGameDispatcher::rename_font_info(qdFontInfo *fi, char const *name) {
	return _fonts.rename_object(fi, name);
}

bool qdGameDispatcher::remove_font_info(qdFontInfo *fi) {
	return _fonts.remove_object(fi);
}

const qdFontInfo *qdGameDispatcher::get_font_info(int type) const {
	if (type == QD_FONT_TYPE_NONE)
		type = _default_font;

	for (qdFontInfoList::const_iterator it = fonts_list().begin(); it != fonts_list().end(); it++)
		if ((*it)->type() == type)
			return (*it);
	return NULL;
}

int qdGameDispatcher::get_unique_font_info_type() const {
	if (fonts_list().empty()) return 0;
	return fonts_list().back()->type() + 1;
}

bool qdGameDispatcher::set_font_info(const qdFontInfo &fi) {
	qdFontInfo *res = find_font_info(fi.type());
	if (NULL != res) {
		*res = fi;
		return true;
	}
	return false;
}

bool qdGameDispatcher::deactivate_scene_triggers(const qdGameScene *p) {
	for (qdTriggerChainList::const_iterator it = trigger_chain_list().begin(); it != trigger_chain_list().end(); ++it)
		(*it)->deactivate_object_triggers(p);

	return true;
}

bool qdGameDispatcher::set_fade(bool fade_in, float duration) {
	if (duration < 1.f / 40.f)
		duration = 1.f / 40.f;

	drop_flag(FADE_IN_FLAG | FADE_OUT_FLAG);

	if (fade_in)
		set_flag(FADE_IN_FLAG);
	else
		set_flag(FADE_OUT_FLAG);

	_fade_timer = 0.f;
	_fade_duration = duration;

	return true;
}

bool qdGameDispatcher::write_resource_stats(const char *file_name) const {
	return true;
}

bool qdGameDispatcher::update_ingame_interface() {
	if (_cur_scene && _cur_scene->has_interface_screen()) {
		debugC(3, kDebugQuant, "qdGameDispatcher::update_ingame_interface() update_ingame_interface");
		return _interface_dispatcher.select_screen(_cur_scene->interface_screen_name());
	} else
		return _interface_dispatcher.select_ingame_screen(_cur_inventory != 0);
}

const char *qdGameDispatcher::hall_of_fame_player_name(int place) const {
	if (place >= 0 && place < _hall_of_fame_size)
		return _hall_of_fame[place]._player_name.c_str();

	return "";
}

void qdGameDispatcher::set_hall_of_fame_player_name(int place, const char *name) {
	if (place >= 0 && place < _hall_of_fame_size) {
		_hall_of_fame[place]._player_name = name;
		_hall_of_fame[place]._updated = false;
	}
}

int qdGameDispatcher::hall_of_fame_player_score(int place) const {
	if (place >= 0 && place < _hall_of_fame_size)
		return _hall_of_fame[place]._score;

	return 0;
}

bool qdGameDispatcher::is_hall_of_fame_updated(int place) const {
	if (place >= 0 && place < _hall_of_fame_size)
		return _hall_of_fame[place]._updated;

	return false;
}

bool qdGameDispatcher::load_hall_of_fame() {
	if (!_hall_of_fame_size)
		return false;

	_hall_of_fame.clear();
	_hall_of_fame.resize(_hall_of_fame_size);

	Common::File fh;
	if (fh.open(Common::Path("Resource/hof.dat"))) {
		char buf[1024];
		for (int i = 0; i < _hall_of_fame_size; i++) {
			fh.readLine(buf, 1024);
			_hall_of_fame[i]._player_name = buf;
			fh.readLine(buf, 1024);
			_hall_of_fame[i]._score = atoi(buf);
		}

		return true;
	}

	return false;
}

bool qdGameDispatcher::save_hall_of_fame() const {
	if (!_hall_of_fame_size)
		return false;

	Common::DumpFile fh;
	if (fh.open(Common::Path("Resource/hof.dat"))) {
		for (int i = 0; i < _hall_of_fame_size; i++) {
			fh.writeString(Common::String::format("%s\r\n", _hall_of_fame[i]._player_name.c_str()));
			fh.writeString(Common::String::format("%d\r\n", _hall_of_fame[i]._score));
		}
		return true;
	}

	return false;
}

bool qdGameDispatcher::add_hall_of_fame_entry(int score) {
	if (!_hall_of_fame_size)
		return false;

	for (int i = 0; i < _hall_of_fame_size; i++) {
		if (score > _hall_of_fame[i]._score) {
			for (int j = _hall_of_fame_size - 1; j > i; j--)
				_hall_of_fame[j] = _hall_of_fame[j - 1];
			_hall_of_fame[i]._score = score;
			_hall_of_fame[i]._player_name = "";
			_hall_of_fame[i]._updated = true;

			return true;
		}
	}

	return false;
}

bool qdGameDispatcher::update_hall_of_fame_names() {
	if (!_hall_of_fame_size)
		return false;

	const qdInterfaceDispatcher::screen_list_t &list = _interface_dispatcher.screen_list();
	for (qdInterfaceDispatcher::screen_list_t::const_iterator it = list.begin(); it != list.end(); ++it) {
		const qdInterfaceScreen::element_list_t &el_list = (*it)->element_list();
		for (qdInterfaceScreen::element_list_t::const_iterator it1 = el_list.begin(); it1 != el_list.end(); ++it1) {
		}
	}

	return true;
}

} // namespace QDEngine
