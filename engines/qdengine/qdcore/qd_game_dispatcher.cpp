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
#include "common/file.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/system.h"
#include "video/video_decoder.h"
#include "video/mpegps_decoder.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_precomp.h"
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
#include "qdengine/qdcore/qd_trigger_profiler.h"
#include "qdengine/qdcore/qd_named_object_reference.h"
#include "qdengine/qdcore/qd_named_object_indexer.h"
#include "qdengine/qdcore/qd_minigame.h"
#include "qdengine/qdcore/qd_game_end.h"
#include "qdengine/qdcore/qd_counter.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_element.h"
#include "qdengine/qdcore/qd_file_manager.h"
#include "qdengine/system/app_core.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/gr_font.h"
#include "qdengine/system/sound/snd_dispatcher.h"
#include "qdengine/system/input/keyboard_input.h"

namespace QDEngine {

qdGameDispatcher *qdGameDispatcher::dispatcher_ = NULL;

qdGameDispatcher *qd_get_game_dispatcher() {
	return qdGameDispatcher::get_dispatcher();
}

void qd_set_game_dispatcher(qdGameDispatcher *p) {
	qdGameDispatcher::set_dispatcher(p);
}

bool qd_keyboard_handler(int vkey, bool event) {
	if (qdGameDispatcher * p = qd_get_game_dispatcher())
		return p->keyboard_handler(vkey, event);

	return false;
}

bool qd_char_input_handler(int input) {
	if (qdInterfaceDispatcher * dp = qdInterfaceDispatcher::get_dispatcher())
		return dp->char_input_handler(input);

	return false;
}

qdGameDispatcher::qdGameDispatcher() : is_paused_(false),
	cur_scene_(NULL),
	scene_loading_progress_data_(NULL),
	scene_loading_progress_fnc_(NULL),
	cur_inventory_(NULL),
	cur_video_(NULL),
	next_scene_(NULL),
	cur_music_track_(NULL),
	cur_interface_music_track_(NULL),
	scene_saved_(false),
	mouse_click_state_(NULL),
	mouse_click_obj_(NULL),
	game_end_(NULL) {
	timer_ = 0;
	default_font_ = 0;

	hall_of_fame_size_ = 0;

	resource_compression_ = 0;

	fade_timer_ = 0.f;
	fade_duration_ = 0.1f;

	autosave_slot_ = 0;

	interface_music_mode_ = false;

	dialog_states_.reserve(16);
	dialog_states_last_.reserve(16);

	enable_file_packages_ = false;

	debugC(1, kDebugTemp, "Setting up mouse...");
	mouse_obj_ = new qdGameObjectMouse;
	mouse_obj_->set_owner(this);

	mouse_animation_ = new qdAnimation;

	mouse_cursor_pos_ = Vect2f(0, 0);

	qdAnimationFrame *p = new qdAnimationFrame;
	p->set_file("Resource\\Cursors\\default.tga");

	mouse_animation_->add_frame(p);

	debugC(1, kDebugTemp, "Mouse_set_animation");
	mouse_obj_->set_animation(mouse_animation_);
	debugC(1, kDebugTemp, "Mouse_set_animation_over");

	if (!dispatcher_) {
		keyboardDispatcher::instance()->set_handler(qd_keyboard_handler);
		grDispatcher::set_input_handler(qd_char_input_handler);
		set_dispatcher(this);
	}

#ifdef _QUEST_EDITOR
	set_flag(FULLSCREEN_REDRAW_FLAG);

#endif
}

qdGameDispatcher::~qdGameDispatcher() {
	free_resources();
	delete mouse_obj_;
	delete mouse_animation_;

	trigger_chains.clear();

	if (dispatcher_ == this)
		set_dispatcher(NULL);
}

void qdGameDispatcher::update_time() {
	timer_ = g_system->getMillis();
}

void qdGameDispatcher::quant() {
	debugC(9, kDebugQuant, "qdGameDispatcher::quant()");

	if (check_flag(SKIP_REDRAW_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Skipping redraw...");
		drop_flag(SKIP_REDRAW_FLAG);
		toggle_full_redraw();
	}
	int idt = qdGameConfig::get_config().logic_period();

	if (!scene_saved_ && cur_scene_ && cur_scene_->autosave_slot() != -1) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Autosaving...");
		save_game(cur_scene_->autosave_slot());
	}

	if (check_flag(SAVE_GAME_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Saving game...");
		save_game(autosave_slot_);
		drop_flag(SAVE_GAME_FLAG);
	}
	if (check_flag(LOAD_GAME_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Loading game...");
		load_game(autosave_slot_);
		drop_flag(LOAD_GAME_FLAG);
	}

	scene_saved_ = true;

	quant(float(idt) / 1000.0f);

	timer_ += idt;

	if (!is_paused() && next_scene_) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Loading next scene...");
		select_scene(next_scene_);
		set_next_scene(NULL);
		quant(0.0f);
		quant(0.0f);
	}

	if (check_flag(MAIN_MENU_FLAG)) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Main menu...");
		drop_flag(MAIN_MENU_FLAG);
		toggle_main_menu(true);
	}

	if (game_end_) {
		debugC(3, kDebugQuant, "qdGameDispatcher::quant() Game end...");
		end_game(game_end_);
		game_end_ = NULL;
	}
}

void qdGameDispatcher::quant(float dt) {
	debugC(9, kDebugQuant, "qdGameDispatcher::quant(%f)", dt);
	if (sndDispatcher * snd = sndDispatcher::get_dispatcher()) {
		snd->quant();
	}

#ifndef _QUEST_EDITOR
	mouse_obj_->set_pos(Vect3f(mouseDispatcher::instance()->mouse_x(), mouseDispatcher::instance()->mouse_y(), 0));
#endif

	mouse_cursor_pos_.x = mouseDispatcher::instance()->mouse_x() + mouse_obj_->screen_pos_offset().x;
	mouse_cursor_pos_.y = mouseDispatcher::instance()->mouse_y() + mouse_obj_->screen_pos_offset().y;

	mouse_obj_->update_screen_pos();
	mouse_obj_->quant(dt);

	mouseDispatcher::instance()->toggle_event(mouseDispatcher::EV_MOUSE_MOVE);
	for (int i = mouseDispatcher::first_event_ID(); i <= mouseDispatcher::last_event_ID(); i ++) {
		mouseDispatcher::mouseEvent ev = static_cast<mouseDispatcher::mouseEvent>(i);
		if (mouseDispatcher::instance()->check_event(ev)) {
			if (mouse_handler(mouse_cursor_pos_.x, mouse_cursor_pos_.y, ev))
				mouseDispatcher::instance()->clear_event(ev);
		}
	}

	if (cur_music_track_ && !interface_music_mode_ && !is_video_playing()) {
		if (!mpegPlayer::instance().is_playing())
			cur_music_track_ = NULL;
	}

	if (interface_music_mode_ && cur_interface_music_track_) {
		if (!mpegPlayer::instance().is_playing())
			cur_interface_music_track_ = NULL;
	}

	if (!is_paused() || check_flag(NEXT_FRAME_FLAG)) {
		debugC(9, kDebugQuant, "qdGameDispatcher::quant() Quanting...");
		qdGameDispatcherBase::quant(dt);

		if (cur_scene_)
			cur_scene_->init_objects_grid();

		for (auto &it: trigger_chain_list()) {
			it->quant(dt);
		}

		for (qdCounterList::const_iterator it = counter_list().begin(); it != counter_list().end(); ++it)
			(*it)->quant();

		interface_dispatcher_.quant(dt);

		if (cur_scene_) {
			debugC(9, kDebugQuant, "qdGameDispatcher::quant() Checking Cur Scene...");
			cur_scene_->quant(dt);
		}

		for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it)
			(*it)->objects_quant(dt);

		if (dialog_states_ != dialog_states_last_) {
			for (dialog_states_container_t::iterator it = dialog_states_last_.begin(); it != dialog_states_last_.end(); ++it)
				screen_texts.clear_texts((*it)->owner());

			for (dialog_states_container_t::iterator it = dialog_states_.begin(); it != dialog_states_.end(); ++it) {
				screen_texts.add_text(qdGameDispatcher::TEXT_SET_DIALOGS, qdScreenText((*it)->short_text(), (*it)->text_format(true), Vect2i(0, 0), (*it)));
			}
		}

		if (check_flag(FADE_IN_FLAG | FADE_OUT_FLAG)) {
			fade_timer_ += dt;
			if (fade_timer_ >= fade_duration_ && !check_flag(FADE_OUT_FLAG)) {
				fade_timer_ = fade_duration_;
				drop_flag(FADE_IN_FLAG | FADE_OUT_FLAG);
			}

			toggle_full_redraw();
		}

		dialog_states_last_ = dialog_states_;
		dialog_states_.clear();
		drop_flag(NEXT_FRAME_FLAG);
	} else {
		if (is_video_playing()) {
			if (is_video_finished()) {
				close_video();
			} else {
				continueVideo();
			}
		}

		if (interface_dispatcher_.is_active()) {
			interface_dispatcher_.quant(dt);
		}
	}

	mouseDispatcher::instance()->clear_events();
	drop_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG);
	mouse_click_obj_ = NULL;
	mouse_click_state_ = NULL;

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
			xml::tag_buffer(*it) > hall_of_fame_size_;
			break;
		case QDSCR_GAME_TITLE:
			game_title_ = it->data();
			debug("> Game title: '%s', id: %d", transCyrillic(game_title_.c_str()), it->origID());
			break;
		case QDSCR_TEXT_DB:
			texts_database_ = it->data();
			debug("> Text db: '%s', id: %d", transCyrillic(texts_database_.c_str()), it->origID());
			break;
		case QDSCR_CD_KEY:
			cd_key_ = it->data();
			debug("> CD key: '%s', id: %d", transCyrillic(cd_key_.c_str()), it->origID());
			break;
		case QDSCR_STARTUP_SCENE:
			set_startup_scene(it->data());
			break;
		case QDSCR_RESOURCE_COMPRESSION:
			xml::tag_buffer(*it) > resource_compression_;
			debug("> Resource compression: '%d', id: %d", resource_compression_, it->origID());
			break;
		case QDSCR_CD:
			set_CD_info(xml::tag_buffer(*it).get_uint());
			break;
		case QDSCR_MOUSE_OBJECT:
			mouse_obj_->load_script(&*it);
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
			qdGameConfig::get_config().set_screen_size(x, y);
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
			interface_dispatcher_.load_script(&*it);
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
			screen_texts.add_text_set(set);
		}
		break;
		case QDSCR_DEFAULT_FONT:
			xml::tag_buffer(*it) > default_font_;
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

#ifndef _QUEST_EDITOR
	if (enable_file_packages_) {
		qdFileManager::instance().init(CD_count());
		startup_check();
	}

	load_hall_of_fame();

	qdNamedObjectIndexer::instance().resolve_references();
	qdNamedObjectIndexer::instance().clear();
#endif

	if (!texts_database_.empty()) {
		Common::SeekableReadStream *fh;
		Common::Path textsdbPath(texts_database_.c_str(), '\\');

		if (qdFileManager::instance().open_file(&fh, textsdbPath.toString().c_str(), false)) {
			qdTextDB::instance().load(fh);
		} else {
			warning("Cannot open textsdbPath: '%s'", transCyrillic(textsdbPath.toString().c_str()));
		}

		delete fh;
	} else
		qdTextDB::instance().clear();

	init();
}

bool qdGameDispatcher::save_script(Common::SeekableWriteStream &fh) const {
	fh.writeString("<?xml version=\"1.0\" encoding=\"WINDOWS-1251\"?> \r\n");
	fh.writeString("<qd_script>\r\n");

	if (!game_title_.empty()) {
		fh.writeString(Common::String::format("\t<game_title>%s</game_title>\r\n", qdscr_XML_string(game_title_.c_str())));
	}

	if (default_font_) {
		fh.writeString(Common::String::format("\t<default_font>%d</default_font>\r\n", default_font_));
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

	if (hall_of_fame_size_) {
		fh.writeString(Common::String::format("\t<hof_size>%d</hof_size>\r\n", hall_of_fame_size_));
	}

	if (has_startup_scene()) {
		fh.writeString(Common::String::format("\t<startup_scene>%s</startup_scene>\r\n", qdscr_XML_string(startup_scene())));
	}

	if (resource_compression_) {
		fh.writeString(Common::String::format("\t<compression>%d</compression>\r\n", resource_compression_));
	}

	if (CD_info()) {
		fh.writeString(Common::String::format("\t<cd>%d</cd>\r\n", CD_info()));
	}

	if (!texts_database_.empty()) {
		fh.writeString(Common::String::format("\t<text_db>%s</text_db>\r\n", qdscr_XML_string(texts_database_.c_str())));
	}

	if (!cd_key_.empty()) {
		fh.writeString(Common::String::format("\t<cd_key>%s</cd_key>\r\n", qdscr_XML_string(cd_key_.c_str())));
	}

	fh.writeString(Common::String::format("\t<screen_size>%d %d</screen_size>\r\n", qdGameConfig::get_config().screen_sx(), qdGameConfig::get_config().screen_sy()));

	screen_texts.save_script(fh, 1);

	mouse_obj_->save_script(fh, 1);

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

	for (auto &it : inventory_cell_types_) {
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

	interface_dispatcher_.save_script(fh, 1);

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

	unsigned int start_clock = g_system->getMillis();
	pr.parse_file(fname);
	unsigned int end_clock =  g_system->getMillis();
	warning("Script parsing: %d ms", end_clock - start_clock);

	if (pr.is_script_binary()) {
		enable_file_packages_ = true;
		qdFileManager::instance().enable_packages();
	}

	start_clock = g_system->getMillis();

	if (const xml::tag * tg = pr.root_tag().search_subtag(QDSCR_ROOT)) {
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

	if (cur_scene_)
		p = cur_scene_->get_sound(name);

	if (p) return p;

	return qdGameDispatcherBase::get_sound(name);
}

qdAnimation *qdGameDispatcher::get_animation(const char *name) {
	qdAnimation *p = NULL;

	if (cur_scene_)
		p = cur_scene_->get_animation(name);

	if (p) return p;

	return qdGameDispatcherBase::get_animation(name);
}

qdAnimationSet *qdGameDispatcher::get_animation_set(const char *name) {
	qdAnimationSet *p = NULL;

	if (cur_scene_)
		p = cur_scene_->get_animation_set(name);

	if (p) return p;

	return qdGameDispatcherBase::get_animation_set(name);
}

void qdGameDispatcher::pre_redraw() {
	grDispatcher::instance()->clear_changes_mask();

	if (cur_scene_)
		cur_scene_->pre_redraw();

	interface_dispatcher_.pre_redraw();
	mouse_obj_->pre_redraw();
	screen_texts.pre_redraw();

	if (!need_full_redraw()) {
		if (cur_inventory_) {
//			cur_inventory_->toggle_redraw(true);
			cur_inventory_->pre_redraw();
		}

		if (cur_scene_) {
			for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it) {
				if (*it != cur_inventory_ && (*it)->check_flag(qdInventory::INV_VISIBLE_WHEN_INACTIVE) && cur_scene_->need_to_redraw_inventory((*it)->name())) {
//					(*it)->toggle_redraw(true);
					(*it)->pre_redraw();
				}
			}
		}
	} else
		add_redraw_region(grScreenRegion(grDispatcher::instance()->Get_SizeX() / 2, grDispatcher::instance()->Get_SizeY() / 2, grDispatcher::instance()->Get_SizeX(), grDispatcher::instance()->Get_SizeY()));

	grDispatcher::instance()->build_changed_regions();
}

void qdGameDispatcher::post_redraw() {
	if (cur_scene_)
		cur_scene_->post_redraw();

	interface_dispatcher_.post_redraw();
	mouse_obj_->post_redraw();
	screen_texts.post_redraw();

	if (cur_inventory_)
		cur_inventory_->post_redraw();

	if (cur_scene_) {
		for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it) {
			if (*it != cur_inventory_ && (*it)->check_flag(qdInventory::INV_VISIBLE_WHEN_INACTIVE) && cur_scene_->need_to_redraw_inventory((*it)->name()))
				(*it)->post_redraw();
		}
	}
}

//#define _GD_REDRAW_REGIONS_CHECK_

void qdGameDispatcher::redraw() {
#ifndef _QUEST_EDITOR
	mouse_obj_->set_pos(Vect3f(mouseDispatcher::instance()->mouse_x(), mouseDispatcher::instance()->mouse_y(), 0));
	mouse_obj_->update_screen_pos();
#endif

	if (!check_flag(SKIP_REDRAW_FLAG)) {
		if (!is_video_playing()) {
			pre_redraw();
#ifndef _GD_REDRAW_REGIONS_CHECK_
			for (grDispatcher::region_iterator it = grDispatcher::instance()->changed_regions().begin(); it != grDispatcher::instance()->changed_regions().end(); ++it) {
				if (!it->is_empty())
					redraw(*it);
			}

			grDispatcher::instance()->FlushChanges();
#else
			redraw(grScreenRegion(grDispatcher::instance()->Get_SizeX() / 2, grDispatcher::instance()->Get_SizeY() / 2, grDispatcher::instance()->Get_SizeX(), grDispatcher::instance()->Get_SizeY()));

			for (grDispatcher::region_iterator it = grDispatcher::instance()->changed_regions().begin(); it != grDispatcher::instance()->changed_regions().end(); ++it)
				grDispatcher::instance()->Rectangle(it->min_x(), it->min_y(), it->size_x(), it->size_y(), 0xFFFFFF, 0, GR_OUTLINED);

			grDispatcher::instance()->Flush();
#endif
		}
#ifndef _QUEST_EDITOR
		if (!qdGameConfig::get_config().force_full_redraw())
			drop_flag(FULLSCREEN_REDRAW_FLAG);
#endif
		post_redraw();
	}
}

void qdGameDispatcher::redraw(const grScreenRegion &reg) {
//	grDispatcher::instance()->SetClip(reg.min_x() - 1,reg.min_y() - 1,reg.max_x() + 1,reg.max_y() + 1);
//	grDispatcher::instance()->Erase(reg.min_x() - 1,reg.min_y() - 1,reg.size_x() + 2,reg.size_y() + 2,0);

	grDispatcher::instance()->SetClip(reg.min_x(), reg.min_y(), reg.max_x(), reg.max_y());
	grDispatcher::instance()->Erase(reg.min_x(), reg.min_y(), reg.size_x(), reg.size_y(), 0);

	if (!interface_dispatcher_.is_active()) {
		redraw_scene(true);
	} else {
		if (interface_dispatcher_.need_scene_redraw())
			redraw_scene(false);

		interface_dispatcher_.redraw();
	}

	debugC(1, kDebugTemp, "mouse_obj_->redraw()");
	mouse_obj_->redraw();

	grDispatcher::instance()->SetClip();
}

void qdGameDispatcher::redraw_scene(bool draw_interface) {
	if (cur_scene_) {
		cur_scene_->redraw();

		if (draw_interface) {
			interface_dispatcher_.redraw();
			if (cur_inventory_) cur_inventory_->redraw();

			for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it) {
				if (*it != cur_inventory_ && (*it)->check_flag(qdInventory::INV_VISIBLE_WHEN_INACTIVE) && cur_scene_->need_to_redraw_inventory((*it)->name()))
					(*it)->redraw(0, 0, true);
			}
		}

		screen_texts.redraw();
#ifndef _QUEST_EDITOR
		cur_scene_->debug_redraw();

		if (check_flag(FADE_IN_FLAG | FADE_OUT_FLAG)) {
			float phase = fade_timer_ / fade_duration_;
			if (phase > 1.f) phase = 1.f;

			if (check_flag(FADE_OUT_FLAG))
				phase = 1.f - phase;

			grDispatcher::instance()->RectangleAlpha(0, 0,
			        qdGameConfig::get_config().screen_sx(),
			        qdGameConfig::get_config().screen_sy(),
			        0, round(phase * 255.f));
		}
#endif
	}
}

bool qdGameDispatcher::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	if ((ev == mouseDispatcher::EV_LEFT_DOWN || ev == mouseDispatcher::EV_RIGHT_DOWN) && mouse_obj_->object()) {
		set_flag(OBJECT_CLICK_FLAG);
		mouse_click_obj_ = mouse_obj_->object();
	}

	if (!is_paused()) {
		if (cur_inventory_ && cur_inventory_->mouse_handler(x, y, ev))
			return true;
	}

	if (interface_dispatcher_.mouse_handler(x, y, ev)) {
		mouseDispatcher::instance()->deactivate_event(ev);
		return true;
	}

	if (is_paused()) {
		if (is_video_playing() && (ev == mouseDispatcher::EV_LEFT_DOWN || ev == mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!cur_video_->check_flag(qdVideo::VID_DISABLE_INTERRUPT_FLAG)) {
				close_video();
				resume();
				return true;
			}
		}
		return false;
	}

	if (ev == mouseDispatcher::EV_LEFT_DOWN) {
		if (mouse_click_obj_)
			set_flag(OBJECT_CLICK_FAILED_FLAG);
		else
			set_flag(CLICK_FAILED_FLAG);
	}

	if (cur_scene_)
		return cur_scene_->mouse_handler(x, y, ev);

	return false;
}

int qdGameDispatcher::load_resources() {
	int size = 0;
	if (mouse_obj_->max_state())
		mouse_obj_->load_resources();
	else
		mouse_animation_->load_resources();

	if (cur_scene_) size += cur_scene_->load_resources();
	size += qdGameDispatcherBase::load_resources();

	for (auto &it : inventory_list()) {
		it->load_resources();
	}

	for (auto &icv : inventory_cell_types_) {
		icv.load_resources();
	}


	return size;
}

void qdGameDispatcher::free_resources() {
	mouse_animation_->free_resources();

	for (auto &icv : inventory_cell_types_) {
		icv.free_resources();
	}

	for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it)
		(*it)->free_resources();

	if (cur_scene_) cur_scene_->free_resources();

	qdGameDispatcherBase::free_resources();
}

int qdGameDispatcher::get_resources_size() {
	int size = 0;
	if (cur_scene_) size += cur_scene_->get_resources_size();
	size += qdGameDispatcherBase::get_resources_size();

	return size;
}

qdNamedObject *qdGameDispatcher::get_named_object(const qdNamedObjectReference *ref) {
	qdNamedObject *p = nullptr;

	for (int i = 0; i < ref->num_levels(); i ++) {
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
			p = mouse_obj_;
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

#ifdef __QD_TRIGGER_PROFILER__
	qdTriggerProfiler::instance().set_read_only(false);
#endif

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
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !mouse_click_obj_) return false;

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

				if (m_obj == mouse_click_obj_)
					return true;
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_OBJECT_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !mouse_click_obj_) return false;

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
				if (m_obj == mouse_click_obj_)
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
					obj = dynamic_cast<const qdGameObject * >(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (!obj->is_visible())
			return false;

		const qdGridZone *zone = NULL;
		if (const qdNamedObject * zone_obj = cnd->get_object(qdCondition::ZONE_NAME)) {
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
	return false;
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
	return false;
	case qdCondition::CONDITION_TIMER: {
		int state;
		if (!cnd->get_value(qdCondition::TIMER_RND, state, 1) || !state) return false;
		return true;
	}
	return false;
	case qdCondition::CONDITION_MOUSE_DIALOG_CLICK: {
		if (!check_flag(DIALOG_CLICK_FLAG) || mouse_click_obj_) return false;
		if (cnd->owner() && cnd->owner() == mouse_click_state_)
			return true;
	}
	return false;
	case qdCondition::CONDITION_MINIGAME_STATE:
		return false;
	case qdCondition::CONDITION_OBJECT_STATE: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject * >(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated * p = dynamic_cast<const qdGameObjectAnimated * >(obj)) {
			if (!p->is_visible())
				return false;

			if (const qdGameObjectState * sp = dynamic_cast<const qdGameObjectState * >(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->is_state_active(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->is_state_active(state_name);
			}
		}
	}
	return false;
	case qdCondition::CONDITION_MOUSE_ZONE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG) || mouse_click_obj_) return false;

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
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !mouse_click_obj_) return false;

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

				if (m_obj == mouse_click_obj_)
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

		if (const qdGameObjectAnimated * p = dynamic_cast<const qdGameObjectAnimated * >(obj)) {
			if (const qdGameObjectState * sp = static_cast<const qdGameObjectState * >(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->was_state_active(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->was_state_active(state_name);
			}
		}
	}
	return false;
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

		if (dr.norm2() < dist * dist) return true;
	}
	return false;
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
					obj = dynamic_cast<const qdGameObject * >(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated * p = dynamic_cast<const qdGameObjectAnimated * >(obj)) {
			if (!p->is_visible())
				return false;

			if (const qdGameObjectState * sp = dynamic_cast<const qdGameObjectState * >(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->is_state_waiting(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->is_state_waiting(state_name);
			}
		}
	}
	return false;
	case qdCondition::CONDITION_OBJECT_STATE_ANIMATION_PHASE: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject * >(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated * p = dynamic_cast<const qdGameObjectAnimated * >(obj)) {
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
	}
	return false;
	case qdCondition::CONDITION_OBJECT_PREV_STATE: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) {
			const char *object_name;
			if (!cnd->get_value(qdCondition::OBJECT_NAME, object_name) || !strlen(object_name)) {
				if (cnd->owner())
					obj = dynamic_cast<const qdGameObject * >(cnd->owner()->owner());
			} else
				obj = get_object(object_name);

			if (!obj) return false;
		}

		if (const qdGameObjectAnimated * p = dynamic_cast<const qdGameObjectAnimated * >(obj)) {
			if (!p->is_visible())
				return false;

			if (const qdGameObjectState * sp = dynamic_cast<const qdGameObjectState * >(cnd->get_object(qdCondition::OBJECT_STATE_NAME))) {
				return p->was_state_previous(sp);
			} else {
				const char *state_name;
				if (!cnd->get_value(qdCondition::OBJECT_STATE_NAME, state_name) || !strlen(state_name))
					return false;

				return p->was_state_previous(state_name);
			}
		}
	}
	return false;
	case qdCondition::CONDITION_STATE_TIME_GREATER_THAN_VALUE:
		if (const qdNamedObject * p = cnd->get_object(0)) {
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
		if (const qdNamedObject * p = cnd->get_object(0)) {
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
		if (const qdNamedObject * p = cnd->get_object(0)) {
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
		if (const qdNamedObject * p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp = static_cast<const qdCounter *>(p);

			int value;
			if (!cnd->get_value(0, value))
				return false;

			return (cp->value() > value);
		}
		return false;
	case qdCondition::CONDITION_COUNTER_LESS_THAN_VALUE:
		if (const qdNamedObject * p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp = static_cast<const qdCounter *>(p);

			int value;
			if (!cnd->get_value(0, value))
				return false;

			return (cp->value() < value);
		}
		return false;
	case qdCondition::CONDITION_COUNTER_GREATER_THAN_COUNTER:
		if (const qdNamedObject * p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp0 = static_cast<const qdCounter *>(p);

			p = cnd->get_object(1);
			if (p->named_object_type() != QD_NAMED_OBJECT_COUNTER) return false;
			const qdCounter *cp1 = static_cast<const qdCounter *>(p);

			return (cp0->value() > cp1->value());
		}
		return false;
	case qdCondition::CONDITION_COUNTER_IN_INTERVAL:
		if (const qdNamedObject * p = cnd->get_object(0)) {
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
		if (const qdNamedObject * p = cnd->get_object(0)) {
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
		if (const qdNamedObject * p = cnd->get_object(0)) {
			if (p->named_object_type() != QD_NAMED_OBJECT_GRID_ZONE) return false;
			return static_cast<const qdGridZone *>(p)->is_any_personage_in_zone();
		}
		return false;
	case qdCondition::CONDITION_OBJECT_HIDDEN: {
		const qdGameObject *obj = dynamic_cast<const qdGameObject *>(cnd->get_object(qdCondition::OBJECT_NAME));
		if (!obj) return false;
		return !obj->is_visible();
	}
	return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_ZONE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (check_flag(OBJECT_CLICK_FLAG | DIALOG_CLICK_FLAG) || mouse_click_obj_) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object() || sc->mouse_right_click_object()) return false;

			if (cnd->owner()) {
				qdNamedObject *p = cnd->owner()->owner(QD_NAMED_OBJECT_MOVING_OBJ);
				if (p && p != get_active_personage())
					return false;
			}

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));
			if (!zone) return false;

			return zone->is_point_in_zone(sc->mouse_click_pos());
		}
		return false;
	case qdCondition::CONDITION_MOUSE_RIGHT_OBJECT_ZONE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !mouse_click_obj_) return false;

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
				if (m_obj && m_obj == mouse_click_obj_)
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
					if (m_obj && m_obj == mouse_obj_->object())
						return true;
				}
			}
		}
		return false;
	case qdCondition::CONDITION_MOUSE_HOVER_ZONE:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN) && !mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (mouse_obj_->object()) return false;

			qdGameScene *sc = get_active_scene();
			if (!sc || sc->mouse_click_object()) return false;

			const qdGridZone *zone = dynamic_cast<const qdGridZone *>(cnd->get_object(qdCondition::CLICK_ZONE_NAME));
			if (!zone) return false;

			return zone->is_point_in_zone(sc->mouse_click_pos());
		}
		return false;
	case qdCondition::CONDITION_MOUSE_OBJECT_HOVER_ZONE:
		if (!mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN) && !mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_RIGHT_DOWN)) {
			if (!mouse_obj_->object()) return false;

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
				if (m_obj && m_obj == mouse_obj_->object())
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
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !mouse_click_obj_) return false;

			const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(0));
			return (!m_obj || m_obj == mouse_click_obj_);

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
			if (!check_flag(OBJECT_CLICK_FLAG) || check_flag(DIALOG_CLICK_FLAG) || !mouse_click_obj_) return false;

			const qdGameObject *m_obj = dynamic_cast<const qdGameObject *>(cnd->get_object(0));
			return (!m_obj || m_obj == mouse_click_obj_);

			return true;
		}
		return false;
	case qdCondition::CONDITION_MOUSE_STATE_PHRASE_CLICK:
		if (mouseDispatcher::instance()->is_event_active(mouseDispatcher::EV_LEFT_DOWN)) {
			if (check_flag(DIALOG_CLICK_FLAG) && !mouse_click_obj_) {
				const qdGameObjectState *p = dynamic_cast<const qdGameObjectState *>(cnd->get_object(0));
				if (!p) return false;

				return (p == mouse_click_state_);
			}
		}
		return false;
	case qdCondition::CONDITION_OBJECT_IS_CLOSER: {
		const qdGameObject *obj0 = dynamic_cast<const qdGameObject *>(cnd->get_object(0));
		if (!obj0) {
			if (cnd->owner())
				obj0 = dynamic_cast<const qdGameObject * >(cnd->owner()->owner());
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
	return false;
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
	return false;
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
	if (!video_player_.open_file(find_file(p->file_name(), *p)))
		return false;

	cur_video_ = p;

	return video_player_.play();
}

bool qdGameDispatcher::pause_video() {
	if (!cur_video_) return false;
	return false;
}

bool qdGameDispatcher::stop_video() {
	if (!cur_video_) return false;
	return video_player_.stop();
}

bool qdGameDispatcher::close_video() {
	if (!cur_video_)
		return false;

	video_player_.stop();
	video_player_.close_file();

	if (check_flag(INTRO_MODE_FLAG)) {
		qdVideoList::const_iterator it = std::find(video_list().begin(), video_list().end(), cur_video_);
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

	cur_video_ = NULL;

	if (sndDispatcher * sp = sndDispatcher::get_dispatcher())
		sp->resume_sounds();

	if (!interface_dispatcher_.is_active())
		resume();

#ifndef _QUEST_EDITOR
	set_flag(SKIP_REDRAW_FLAG);
#endif

	return true;
}

bool qdGameDispatcher::is_video_finished() {
	if (!cur_video_) return false;
	return video_player_.is_playback_finished();
}

void qdGameDispatcher::continueVideo() {
	if (!cur_video_) return;
	video_player_.play();
}

bool qdGameDispatcher::merge_global_objects(qdGameObject *obj) {
#ifndef _QUEST_EDITOR
	for (auto &is : scene_list()) {
		is->merge_global_objects(obj);
	}
#else
	qdGameScene *const activeScene = get_active_scene();
	qdGameSceneList::const_iterator is = scene_list().begin(),
	                en = scene_list().end();
	for (; is != en; ++is) {
		if (*is != activeScene)
			(*is)->merge_global_objects(obj);
	}
	if (activeScene)
		activeScene->merge_global_objects(obj);
#endif // _QUEST_EDITOR

	return true;
}

#ifndef _QUEST_EDITOR
bool qdGameDispatcher::update_walk_state(const char *object_name, qdGameObjectState *p) {
	if (qdGameObject * obj = get_global_object(object_name)) {
		if (obj->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			static_cast<qdGameObjectMoving * >(obj)->set_last_walk_state(p);
	}

	return false;
}

qdGameObjectState *qdGameDispatcher::get_walk_state(const char *object_name) {
	if (qdGameObject * obj = get_global_object(object_name)) {
		if (obj->named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			return static_cast<qdGameObjectMoving * >(obj)->last_walk_state();
	}

	return NULL;
}
#endif // _QUEST_EDITOR

bool qdGameDispatcher::split_global_objects(qdGameObject *obj) {
	for (auto &is : scene_list()) {
		is->split_global_objects(obj);
	}

	return true;
}

bool qdGameDispatcher::init_inventories() {
	bool result = true;
	for (auto &it : inventory_list()) {
		if (!it->init(inventory_cell_types_)) {
			result = false;
		}
	}
	return result;
}

bool qdGameDispatcher::toggle_inventory(bool state) {
#ifndef _QUEST_EDITOR
	toggle_full_redraw();

	drop_mouse_object();

	if (state) {
		qdGameObjectMoving *p = get_active_personage();
		if (p && strlen(p->inventory_name())) {
			cur_inventory_ = get_inventory(p->inventory_name());
			if (cur_inventory_) {
				update_ingame_interface();
				return true;
			}
		}
	}

	cur_inventory_ = NULL;
	update_ingame_interface();
#endif
	return true;
}

bool qdGameDispatcher::drop_mouse_object() {
	if (mouse_obj_->object()) {
		/*      if(!cur_inventory_){
		            if(!toggle_inventory(true))
		                return false;
		        }*/

		if (!cur_inventory_)
			return false;

		qdGameObjectAnimated *obj = mouse_obj_->object();
		mouse_obj_->take_object(NULL);
		cur_inventory_->put_object(obj);
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
		if (qdGameObjectState * sp = p->get_inventory_state())
			p->set_state(sp);

		if (!inv->check_flag(qdInventory::INV_DONT_OPEN_AFTER_TAKE)) {
			if (!cur_inventory_) toggle_inventory(true);

			if (inv->check_flag(qdInventory::INV_TAKE_TO_MOUSE)) {
				if (cur_inventory_ == inv) {
					if (mouse_obj_->object()) {
						qdGameObjectAnimated *obj = mouse_obj_->object();
						mouse_obj_->take_object(NULL);
						cur_inventory_->put_object(obj);
					}
					cur_inventory_->remove_object(p);
					mouse_obj_->take_object(p);
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
	if (mouse_obj_->object() == p) {
		mouse_obj_->take_object(NULL);
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
	return inventories.rename_object(p, name);
}

bool qdGameDispatcher::add_video(qdVideo *p, qdVideo const *before) {
#ifdef _QUEST_EDITOR
	if (before) {
		if (videos.insert_object(p, before)) {
			p->set_owner(this);
			return true;
		}
	} else //!!!!!!!!
#endif // _QUEST_EDITOR
		if (videos.add_object(p)) {
			p->set_owner(this);
			return true;
		}

	return false;
}

bool qdGameDispatcher::is_video_in_list(const char *name) {
	return videos.is_in_list(name);
}

bool qdGameDispatcher::is_video_in_list(qdVideo *p) {
	return videos.is_in_list(p);
}

bool qdGameDispatcher::remove_video(const char *name) {
	return videos.remove_object(name);
}

bool qdGameDispatcher::remove_video(qdVideo *p) {
	return videos.remove_object(p);
}

bool qdGameDispatcher::rename_video(qdVideo *p, const char *name) {
	return videos.rename_object(p, name);
}

qdVideo *qdGameDispatcher::get_video(const char *name) {
	return videos.get_object(name);
}

bool qdGameDispatcher::select_scene(qdGameScene *sp, bool resources_flag) {
	int tm = g_system->getMillis();

	toggle_full_redraw();

	screen_texts.clear_texts();

	if (!sp || get_active_scene() != sp) {
			debugC(3, kDebugQuant, "qdGameDispatcher::select_scene() Stop sound");
		if (sndDispatcher * p = sndDispatcher::get_dispatcher())
			p->stop_sounds();
	}

	if (sp) {
		debugC(3, kDebugQuant, "qdGameDispatcher::select_scene() request_file_package");
		request_file_package(*sp);
	}

	drop_mouse_object();
	toggle_inventory(true);

	if (cur_scene_) {
		if (cur_scene_ != sp)
			cur_scene_->free_resources();

		cur_scene_->deactivate();
	}

	scene_saved_ = false;

	cur_scene_ = sp;
	qdCamera::set_current_camera(NULL);

	toggle_inventory(true);

	debug("select_scene('%s', %d)", sp ? (const char *)transCyrillic(sp->name()) : "<no name>", resources_flag);

	if (cur_scene_) {
		debugC(3, kDebugQuant, "qdGameDispatcher::select_scene() set_current_camera");
		qdCamera::set_current_camera(cur_scene_->get_camera());
		cur_scene_->activate();

		if (resources_flag)
			cur_scene_->load_resources();

		update_ingame_interface();
		cur_scene_->start_minigame();
		interface_dispatcher_.update_personage_buttons();
	}

	if (resources_flag) {
		if (mouse_obj_->max_state()) {
			mouse_obj_->free_resources();
			mouse_obj_->load_resources();
		} else
			mouse_animation_->load_resources();

		for (qdInventoryList::const_iterator it = inventory_list().begin(); it != inventory_list().end(); ++it)
			(*it)->load_resources();
	}

	tm = g_system->getMillis() - tm;
	if (cur_scene_)
		debugC(1, kDebugLoad, "Scene loading \"%s\" %d ms", transCyrillic(cur_scene_->name()), tm);

	return true;
}

qdGameObject *qdGameDispatcher::get_object(const char *name) {
	if (cur_scene_)
		return cur_scene_->get_object(name);

	return NULL;
}

qdGameObjectMoving *qdGameDispatcher::get_active_personage() {
	if (cur_scene_) return cur_scene_->get_active_personage();

	return NULL;
}

qdScaleInfo *qdGameDispatcher::get_scale_info(const char *p) {
	if (cur_scene_) {
		qdScaleInfo *si = cur_scene_->get_scale_info(p);
		if (si) return si;
	}

	return qdGameDispatcherBase::get_scale_info(p);
}

bool qdGameDispatcher::add_trigger_chain(qdTriggerChain *p) {
	if (trigger_chains.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::remove_trigger_chain(const char *name) {
	return trigger_chains.remove_object(name);
}

qdTriggerChain *qdGameDispatcher::get_trigger_chain(const char *name) {
	return trigger_chains.get_object(name);
}

bool qdGameDispatcher::remove_trigger_chain(qdTriggerChain *p) {
	return trigger_chains.remove_object(p);
}

bool qdGameDispatcher::is_trigger_chain_in_list(const char *name) {
	return trigger_chains.is_in_list(name);
}

bool qdGameDispatcher::is_trigger_chain_in_list(qdTriggerChain *p) {
	return trigger_chains.is_in_list(p);
}

bool qdGameDispatcher::rename_trigger_chain(qdTriggerChain *p, const char *name) {
	return trigger_chains.rename_object(p, name);
}

bool qdGameDispatcher::add_global_object(qdGameObject *p) {
	if (global_objects.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_global_object(qdGameObject *p, const char *name) {
	return global_objects.rename_object(p, name);
}

bool qdGameDispatcher::remove_global_object(const char *name) {
	return global_objects.remove_object(name);
}

qdGameObject *qdGameDispatcher::get_global_object(const char *name) {
	return global_objects.get_object(name);
}

bool qdGameDispatcher::remove_global_object(qdGameObject *p) {
	return global_objects.remove_object(p);
}

bool qdGameDispatcher::is_global_object_in_list(const char *name) {
	return global_objects.is_in_list(name);
}

bool qdGameDispatcher::is_global_object_in_list(qdGameObject *p) {
	return global_objects.is_in_list(p);
}

bool qdGameDispatcher::add_minigame(qdMiniGame *p) {
	if (minigames.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_minigame(qdMiniGame *p, const char *name) {
	return minigames.rename_object(p, name);
}

bool qdGameDispatcher::remove_minigame(const char *name) {
	return minigames.remove_object(name);
}

qdMiniGame *qdGameDispatcher::get_minigame(const char *name) {
	return minigames.get_object(name);
}

bool qdGameDispatcher::remove_minigame(qdMiniGame *p) {
	return minigames.remove_object(p);
}

bool qdGameDispatcher::is_minigame_in_list(const char *name) {
	return minigames.is_in_list(name);
}

bool qdGameDispatcher::is_minigame_in_list(qdMiniGame *p) {
	return minigames.is_in_list(p);
}

bool qdGameDispatcher::keyboard_handler(int vkey, bool event) {
	if (is_paused()) {
#ifdef __QD_DEBUG_ENABLE__
		if (event) {
			switch (vkey) {
			case VK_PAUSE:
				resume();
				return true;
			case VK_SPACE:
				set_flag(NEXT_FRAME_FLAG);
				return true;
			}
		}
#endif

		if (is_video_playing() && event) {
			if (!cur_video_->check_flag(qdVideo::VID_DISABLE_INTERRUPT_FLAG)) {
				close_video();
				return true;
			}
		}

		if (interface_dispatcher_.is_active() && event)
			return interface_dispatcher_.keyboard_handler(vkey);

		return false;
	}

	if (event) {
		if (interface_dispatcher_.keyboard_handler(vkey))
			return true;

		switch (vkey) {
		case VK_ESCAPE:
			if (is_main_menu_exit_enabled())
				return toggle_main_menu(true);
			break;
		case VK_SPACE:
			if (qdGameScene * sp = get_active_scene()) {
				if (!sp->check_flag(qdGameScene::DISABLE_KEYBOARD_PERSONAGE_SWITCH))
					sp->change_active_personage();
				return true;
			}
			break;
#ifdef __QD_DEBUG_ENABLE__
		case VK_F9:
			qdGameConfig::get_config().toggle_debug_draw();
			if (qdGameConfig::get_config().debug_draw()) {
				if (!qdGameConfig::get_config().force_full_redraw())
					qdGameConfig::get_config().toggle_full_redraw();
			} else {
				if (qdGameConfig::get_config().force_full_redraw())
					qdGameConfig::get_config().toggle_full_redraw();
			}
			toggle_full_redraw();
			return true;
		case VK_F10:
			write_resource_stats("memory_usage.html");
			return true;
		case VK_F5:
			pause();
			save_game(0);
			resume();
			return true;
		case VK_F6:
			load_game(0);
			return true;
		case VK_PAUSE:
			pause();
			return true;
#endif
		}
	}

	return false;
}

bool qdGameDispatcher::load_save(const char *fname) {
	debugC(3, kDebugLog, "[%d] Save loading %s", g_system->getMillis(), transCyrillic(fname));

	if (sndDispatcher * p = sndDispatcher::get_dispatcher()) {
		p->stop_sounds();
		p->pause();
	}

	pause();

	free_resources();

	Common::SeekableReadStream *fh;
	fh = g_engine->_savefileMan->openForLoading(Common::Path(fname, '\\').toString());
	int save_version;
	save_version = fh->readSint32LE();

	qdNamedObjectReference ref;

	if (!ref.load_data(*fh, save_version)) return false;

	qdGameScene *cur_scene_ptr = static_cast<qdGameScene *>(get_named_object(&ref));
	select_scene(0, false);

	if (!ref.load_data(*fh, save_version)) return false;
	if (qdMusicTrack * p = static_cast<qdMusicTrack * >(get_named_object(&ref))) {
		cur_music_track_ = 0;
		play_music_track(p);
	}

	int flag = fh->readSint32LE();
	if (flag)
		toggle_inventory(true);
	else
		toggle_inventory(false);

	int size = fh->readSint32LE();
	if (size != global_object_list().size()) return false;

	for (auto &it : global_object_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	size = fh->readSint32LE();
	if (size != counter_list().size()) return false;

	for (auto &it : counter_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	size = fh->readSint32LE();
	if (size != scene_list().size()) return false;

	for (auto &it : scene_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	size = fh->readSint32LE();
	if (size != global_object_list().size()) return false;

	for (auto &it : global_object_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	size = fh->readSint32LE();
	if (size != trigger_chain_list().size()) return false;

	for (auto &it : trigger_chain_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	size = fh->readSint32LE();
	if (size != inventory_list().size()) return false;

	for (auto &it : inventory_list()) {
		if (!it->load_data(*fh, save_version))
			return false;
	}

	if (save_version >= 10)
		mouse_obj_->load_data(*fh, save_version);

	if (cur_scene_ptr)
		select_scene(cur_scene_ptr, false);

	load_resources();

	if (sndDispatcher * p = sndDispatcher::get_dispatcher())
		p->resume();

	interface_dispatcher_.update_personage_buttons();
	resume();

	return true;
}

bool qdGameDispatcher::save_save(const char *fname) const {
	Common::OutSaveFile *fh;
	Common::Path fpath(fname, '\\');
	fh = g_engine->_savefileMan->openForSaving(fpath.toString());

	const int save_version = 107;
	fh->writeUint32LE(save_version);

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

	if (cur_music_track_) {
		qdNamedObjectReference ref(cur_music_track_);
		if (!ref.save_data(*fh)) {
			return false;
		}
	} else {
		qdNamedObjectReference ref;
		if (!ref.save_data(*fh)) {
			return false;
		}
	}

	if (cur_inventory_)
		fh->writeSint32LE(1);
	else
		fh->writeSint32LE(0);

	fh->writeUint32LE(global_object_list().size());
	for (auto &it : global_object_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	fh->writeUint32LE(counter_list().size());
	for (auto &it : counter_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	fh->writeUint32LE(scene_list().size());
	for (auto &it : scene_list()) {
		if (!it->save_data(*fh))
			return false;
	}

	fh->writeUint32LE(global_object_list().size());
	for (auto &it : global_object_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	fh->writeUint32LE(trigger_chain_list().size());
	for (auto &it : trigger_chain_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	fh->writeUint32LE(inventory_list().size());
	for (auto &it : inventory_list()) {
		if (!it->save_data(*fh)) {
			return false;
		}
	}

	mouse_obj_->save_data(*fh);

	delete fh;

	return true;
}

bool qdGameDispatcher::play_music_track(const qdMusicTrack *p, bool interface_mode) {
	debugC(3, kDebugLog, "[%d] music start->%s", g_system->getMillis(), transCyrillic(p->file_name()));

	if (!interface_mode) {
		if (p->check_flag(QD_MUSIC_TRACK_DISABLE_RESTART) && cur_music_track_ == p)
			return true;

		cur_music_track_ = p;
		cur_interface_music_track_ = NULL;
	} else {
		if (cur_interface_music_track_ == p)
			return true;

		cur_interface_music_track_ = p;
	}

	const char *file_name = NULL;

	if (cur_scene_)
		file_name = find_file(p->file_name(), *cur_scene_);
	else
		file_name = find_file(p->file_name(), *this);

	interface_music_mode_ = interface_mode;

	return mpegPlayer::instance().play(file_name, p->is_cycled(), p->volume());
}

bool qdGameDispatcher::stop_music() {
	debugC(3, kDebugLog, "[%d] music stop", g_system->getMillis());

	if (interface_music_mode_)
		cur_interface_music_track_ = NULL;
	else
		cur_music_track_ = NULL;

	return mpegPlayer::instance().stop();
}

void qdGameDispatcher::pause() {
	is_paused_ = true;

	if (sndDispatcher * p = sndDispatcher::get_dispatcher())
		p->pause_sounds();
}

void qdGameDispatcher::resume() {
	update_time();
	is_paused_ = false;

	if (sndDispatcher * p = sndDispatcher::get_dispatcher())
		p->resume_sounds();
}

void qdGameDispatcher::set_dispatcher(qdGameDispatcher *p) {
	dispatcher_ = p;

	if (p)
		qdInterfaceDispatcher::set_dispatcher(&p->interface_dispatcher_);
	else
		qdInterfaceDispatcher::set_dispatcher(NULL);
}

bool qdGameDispatcher::toggle_main_menu(bool state, const char *screen_name) {
	toggle_full_redraw();

	if (state) {
		if (interface_dispatcher_.has_main_menu() || screen_name) {
			/*          if(cur_inventory_){
			                if(mouse_obj_->object()){
			                    qdGameObjectAnimated* obj = mouse_obj_->object();
			                    mouse_obj_->take_object(NULL);
			                    cur_inventory_->put_object(obj);
			                }
			            }*/

			mouse_obj_->set_cursor(qdGameObjectMouse::MAIN_MENU_CURSOR);

			if (!screen_name)
				screen_name = interface_dispatcher_.main_menu_screen_name();

			interface_dispatcher_.select_screen(screen_name);
			interface_dispatcher_.activate();
			pause();
			return true;
		}
	} else {
		update_ingame_interface();

		interface_dispatcher_.deactivate();
		interface_dispatcher_.update_personage_buttons();

		if (interface_music_mode_) {
			debugC(3, kDebugQuant, "qdGameDispatcher::toggle_main_menu() interface_music_mode_");
			if (cur_music_track_) {
				const qdMusicTrack *tp = cur_music_track_;
				cur_music_track_ = 0;
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
	return !cur_scene_ || !cur_scene_->check_flag(qdGameScene::DISABLE_MAIN_MENU);
}

bool qdGameDispatcher::end_game(const qdGameEnd *p) {
	restart();

	const char *screen_name = (p->has_interface_screen()) ? p->interface_screen() : NULL;

	if (screen_name)
		interface_dispatcher_.toggle_end_game_mode(true);

	return toggle_main_menu(true, screen_name);
}

bool qdGameDispatcher::add_redraw_region(const grScreenRegion &reg) {
	return grDispatcher::instance()->invalidate_region(reg);
}

bool qdGameDispatcher::init() {
#ifdef _QUEST_EDITOR
	load_resources();
#endif

	if (sndDispatcher * sdp = sndDispatcher::get_dispatcher())
		sdp->stop_sounds();

	if (!screen_texts.get_text_set(TEXT_SET_DIALOGS)) {
		qdScreenTextSet set;
		set.set_ID(TEXT_SET_DIALOGS);
		set.set_screen_pos(Vect2i(qdGameConfig::get_config().screen_sx() / 2, qdGameConfig::get_config().screen_sy() / 2));
		set.set_screen_size(Vect2i(qdGameConfig::get_config().screen_sx(), qdGameConfig::get_config().screen_sy() - qdGameConfig::get_config().screen_sy() / 4));

		screen_texts.add_text_set(set);
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
	for (std::list<qdFontInfo * >::const_iterator it = fonts_.get_list().begin();
	        it != fonts_.get_list().end(); ++it)
		(*it)->load_font();

	cur_video_ = NULL;
	cur_inventory_ = NULL;
	next_scene_ = NULL;
	cur_music_track_ = NULL;

	return true;
}

bool qdGameDispatcher::load_game(int slot_id) {
	const char *save_name = get_save_name(slot_id);

	if (app_io::is_file_exist(save_name)) {
		if (!load_save(save_name))
			error("Save game format error");
	}

	scene_saved_ = true;

	return false;
}

bool qdGameDispatcher::save_game(int slot_id) const {
	if (!get_active_scene()) return false;

	debugC(3, kDebugSave, "qdGameDispatcher::save_game(%d): filename: %s", slot_id, get_save_name(slot_id));
	return save_save(get_save_name(slot_id));
}

const char *qdGameDispatcher::get_save_name(int slot_id, SaveFileType file_type) {
	static XBuffer file_name(_MAX_PATH);

	file_name.init();
	file_name < "Saves\\save";
	if (slot_id < 10) file_name < "0";
	file_name <= slot_id;

	switch (file_type) {
	case SAVE_FILE:
		file_name < ".sav";
		break;
	case SAVE_THUMBNAIL:
		file_name < ".tga";
		break;
	case SAVE_DESCRIPTION:
		file_name < ".txt";
		break;
	}

	return file_name.c_str();
}

bool qdGameDispatcher::game_screenshot(const char *file_name, int sx, int sy) const {
	if (qdGameScene * sp = get_active_scene()) {
		qdSprite sprite(qdGameConfig::get_config().screen_sx(),
		                qdGameConfig::get_config().screen_sy(),
		                GR_RGB888);

		sp->redraw();

		unsigned int r, g, b;
		for (int i = 0; i < qdGameConfig::get_config().screen_sy(); i++)
			for (int j = 0; j < qdGameConfig::get_config().screen_sx(); j++) {
				grDispatcher::instance()->GetPixel(j, i, r, g, b);
				sprite.put_pixel(j, i, r, g, b);
			}

		sprite.scale(static_cast<float>(sx) / qdGameConfig::get_config().screen_sx(),
		             static_cast<float>(sy) / qdGameConfig::get_config().screen_sy());
		sprite.save(file_name);
		return true;
	}
	return false;
	/*
	if(qdGameScene* sp = get_active_scene()){
	    static std::vector<char> screen_buf;
	    if(screen_buf.size() < sx * sy * 3)
	        screen_buf.resize(sx * sy * 3);

	    sp->redraw();

	    int dx = (qdGameConfig::get_config().screen_sx() << 16) / sx;
	    int dy = (qdGameConfig::get_config().screen_sy() << 16) / sy;
	    int fy = (1 << 15);

	    char* p = &*screen_buf.begin();

	    for(int y = 0; y < sy; y++){
	        int y0 = (fy >> 16);
	        fy += dy;
	        int fx = (1 << 15);
	        for(int x = 0; x < sx; x++){
	            unsigned r,g,b;
	            grDispatcher::instance()->GetPixel(fx >> 16,y0,r,g,b);

	            p[2] = r;
	            p[1] = g;
	            p[0] = b;

	            p += 3;

	            fx += dx;
	        }
	    }

	    static unsigned char header[18];

	    memset(header,0,18);
	    header[2] = 2;

	    header[13] = (sx >> 8) & 0xFF;
	    header[12] = sx & 0xFF;

	    header[15] = (sy >> 8) & 0xFF;
	    header[14] = sy & 0xFF;

	    header[16] = 24;
	    header[17] = 0x20;

	    fh.write(header,18);

	    fh.write(&*screen_buf.begin(),sx * sy * 3);

	    fh.close();
	}
	return false;
	*/
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
	if (game_ends_.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_game_end(qdGameEnd *p, const char *name) {
	return game_ends_.rename_object(p, name);
}

bool qdGameDispatcher::remove_game_end(qdGameEnd *p) {
	return game_ends_.remove_object(p);
}

qdGameEnd *qdGameDispatcher::get_game_end(const char *name) {
	return game_ends_.get_object(name);
}

bool qdGameDispatcher::is_game_end_in_list(const char *name) {
	return game_ends_.is_in_list(name);
}

bool qdGameDispatcher::is_game_end_in_list(qdGameEnd *p) {
	return game_ends_.is_in_list(p);
}

bool qdGameDispatcher::is_on_mouse(const qdGameObjectAnimated *p) const {
	return (mouse_obj_->object() == p);
}

bool qdGameDispatcher::add_scene(qdGameScene *p) {
	if (scenes_.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_scene(qdGameScene *p, const char *name) {
	return scenes_.rename_object(p, name);
}

bool qdGameDispatcher::remove_scene(qdGameScene *p) {
	return scenes_.remove_object(p);
}

qdGameScene *qdGameDispatcher::get_scene(const char *name) {
	return scenes_.get_object(name);
}

bool qdGameDispatcher::is_scene_in_list(const char *name) {
	return scenes_.is_in_list(name);
}

bool qdGameDispatcher::is_scene_in_list(const qdGameScene *p) {
	return scenes_.is_in_list(p);
}

bool qdGameDispatcher::add_counter(qdCounter *p) {
	if (counters_.add_object(p)) {
		p->set_owner(this);
		return true;
	}

	return false;
}

bool qdGameDispatcher::rename_counter(qdCounter *p, const char *name) {
	return counters_.rename_object(p, name);
}

bool qdGameDispatcher::remove_counter(qdCounter *p) {
	return counters_.remove_object(p);
}

qdCounter *qdGameDispatcher::get_counter(const char *name) {
	return counters_.get_object(name);
}

bool qdGameDispatcher::is_counter_in_list(const char *name) {
	return counters_.is_in_list(name);
}

bool qdGameDispatcher::is_counter_in_list(qdCounter *p) {
	return counters_.is_in_list(p);
}

void qdGameDispatcher::scan_files(qdLoadingProgressFnc progress_fnc, void *context_ptr) {
	int size = video_list().size() + scene_list().size() + 1;
	int progress = 0;

	calc_files_size();

	progress++;
	int percents = progress * 100 / size;
	(*progress_fnc)(percents, context_ptr);

	for (qdVideoList::const_iterator it = video_list().begin(); it != video_list().end(); ++it) {
		(*it)->calc_files_size();

		progress++;
		percents = progress * 100 / size;
		(*progress_fnc)(percents, context_ptr);
	}

	for (qdGameSceneList::const_iterator it = scene_list().begin(); it != scene_list().end(); ++it) {
		(*it)->calc_files_size();

		progress++;
		percents = progress * 100 / size;
		(*progress_fnc)(percents, context_ptr);
	}
}

bool qdGameDispatcher::get_files_list(qdFileNameList &files_to_copy, qdFileNameList &files_to_pack) const {
	files_to_pack.push_back("Resource/Cursors/default.tga");
	files_to_pack.push_back("Resource/Fonts/font00.idx");
	files_to_pack.push_back("Resource/Fonts/font00.tga");

	if (!texts_database_.empty())
		files_to_pack.push_back(texts_database());

	for (qdMiniGameList::const_iterator it = minigame_list().begin(); it != minigame_list().end(); ++it)
		(*it)->get_files_list(files_to_copy, files_to_pack);

	for (qdSoundList::const_iterator it = sound_list().begin(); it != sound_list().end(); ++it)
		files_to_pack.push_back((*it)->file_name());

	for (qdAnimationList::const_iterator it = animation_list().begin(); it != animation_list().end(); ++it)
		files_to_pack.push_back((*it)->qda_file());

	for (qdInventoryCellTypeVector::const_iterator it = inventory_cell_types().begin(); it != inventory_cell_types().end(); ++it)
		files_to_pack.push_back(it->sprite_file());

	interface_dispatcher_.get_file_list(files_to_copy, files_to_pack);

	for (qdFontInfoList::const_iterator it = fonts_list().begin();
	        it != fonts_list().end(); ++it) {
		files_to_pack.push_back((*it)->font_file_name()); // tga
		files_to_pack.push_back(app_io::change_ext((*it)->font_file_name(), ".idx"));
	}

	if (hall_of_fame_size_)
		files_to_copy.push_back("Resource/hof.dat");

	return true;
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

void qdGameDispatcher::request_CD(const qdFileOwner &file_owner) const {
	int cd_id = -1;
	for (int i = 0; i < CD_count(); i++) {
		if (file_owner.is_on_CD(i)) {
			if (qdFileManager::instance().is_CD_available(i))
				return;

			if (cd_id == -1)
				cd_id = i;
		}
	}

	if (cd_id == -1)
		return;

	mpegPlayer::instance().stop();

	while (1) {
		switch (MessageBox(NULL, cd_request_string(cd_id), game_title(), MB_OKCANCEL | MB_ICONEXCLAMATION)) {
		case IDOK:
			if (qdFileManager::instance().scan_drives(&file_owner)) {
				qdFileManager::instance().set_last_CD_id(cd_id);
				qdFileManager::instance().update_packages();
				return;
			}
			break;
		case IDCANCEL:
			exit(1);
		}
	}
}

void qdGameDispatcher::request_CD(int cd_id) const {
	if (qdFileManager::instance().scan_drives(cd_id))
		return;

	mpegPlayer::instance().stop();

	while (1) {
		switch (MessageBox(NULL, cd_request_string(cd_id), game_title(), MB_OKCANCEL | MB_ICONEXCLAMATION)) {
		case IDOK:
			if (qdFileManager::instance().scan_drives(cd_id)) {
				qdFileManager::instance().update_packages();
				return;
			}
			break;
		case IDCANCEL:
			exit(1);
		}
	}
}

const char *qdGameDispatcher::cd_request_string(int cd_id) const {
	static XBuffer str;
	str.init();

	str < "Вставьте CD";

	if (CD_count() > 1)
		str < " " <= cd_id + 1;

	return str.c_str();
}

void qdGameDispatcher::request_file_package(const qdFileOwner &file_owner) const {
	if (!enable_file_packages_) return;

	if (qdFileManager::instance().is_package_available(file_owner))
		return;

	request_CD(file_owner);
}

void qdGameDispatcher::startup_check() const {
	if (!enable_file_packages_) return;

	request_file_package(*this);

	const char *p = (cd_key_.empty()) ? NULL : cd_key_.c_str();

	while (!qdFileManager::instance().check_drives(p))
		request_CD(*this);
}

const char *qdGameDispatcher::find_file(const char *file_name, const qdFileOwner &file_owner) const {
	debugC(4, kDebugLoad, "qdGameDispatcher::find_file(%s)", file_name);

	if (enable_file_packages_ && !app_io::is_file_exist(file_name)) {
		request_CD(file_owner);

		static XBuffer fname(MAX_PATH);
		fname.init();

		fname < qdFileManager::instance().CD_path(file_owner) < file_name;

		return fname.c_str();
	}

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
	dialog_states_container_t::const_iterator it = std::find(dialog_states_.begin(), dialog_states_.end(), p);
	if (it != dialog_states_.end()) return false;

	dialog_states_.push_back(p);

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
		fonts_.add_object(fi);
		return true;
	} else return false;
}

bool qdGameDispatcher::rename_font_info(qdFontInfo *fi, char const *name) {
	return fonts_.rename_object(fi, name);
}

bool qdGameDispatcher::remove_font_info(qdFontInfo *fi) {
	return fonts_.remove_object(fi);
}

const qdFontInfo *qdGameDispatcher::get_font_info(int type) const {
	if (type == QD_FONT_TYPE_NONE)
		type = default_font_;

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
#ifndef _QUEST_EDITOR
	if (duration < 1.f / 40.f)
		duration = 1.f / 40.f;

	drop_flag(FADE_IN_FLAG | FADE_OUT_FLAG);

	if (fade_in)
		set_flag(FADE_IN_FLAG);
	else
		set_flag(FADE_OUT_FLAG);

	fade_timer_ = 0.f;
	fade_duration_ = duration;

#endif
	return true;
}

bool qdGameDispatcher::write_resource_stats(const char *file_name) const {
	return true;
}

bool qdGameDispatcher::update_ingame_interface() {
	if (cur_scene_ && cur_scene_->has_interface_screen()) {
		debugC(3, kDebugQuant, "qdGameDispatcher::update_ingame_interface() update_ingame_interface");
		return interface_dispatcher_.select_screen(cur_scene_->interface_screen_name());
	} else
		return interface_dispatcher_.select_ingame_screen(cur_inventory_ != 0);
}

const char *qdGameDispatcher::hall_of_fame_player_name(int place) const {
	if (place >= 0 && place < hall_of_fame_size_)
		return hall_of_fame_[place].player_name_.c_str();

	return "";
}

void qdGameDispatcher::set_hall_of_fame_player_name(int place, const char *name) {
	if (place >= 0 && place < hall_of_fame_size_) {
		hall_of_fame_[place].player_name_ = name;
		hall_of_fame_[place].updated_ = false;
	}
}

int qdGameDispatcher::hall_of_fame_player_score(int place) const {
	if (place >= 0 && place < hall_of_fame_size_)
		return hall_of_fame_[place].score_;

	return 0;
}

bool qdGameDispatcher::is_hall_of_fame_updated(int place) const {
	if (place >= 0 && place < hall_of_fame_size_)
		return hall_of_fame_[place].updated_;

	return false;
}

bool qdGameDispatcher::load_hall_of_fame() {
	if (!hall_of_fame_size_)
		return false;

	hall_of_fame_.clear();
	hall_of_fame_.resize(hall_of_fame_size_);

	Common::File fh;
	if (fh.open(Common::Path("Resource/hof.dat"))) {
		char buf[1024];
		for (int i = 0; i < hall_of_fame_size_; i++) {
			fh.readLine(buf, 1024);
			hall_of_fame_[i].player_name_ = buf;
			fh.readLine(buf, 1024);
			hall_of_fame_[i].score_ = atoi(buf);
		}

		return true;
	}

	return false;
}

bool qdGameDispatcher::save_hall_of_fame() const {
	if (!hall_of_fame_size_)
		return false;

	Common::DumpFile fh;
	if (fh.open(Common::Path("Resource/hof.dat"))) {
		for (int i = 0; i < hall_of_fame_size_; i++) {
			fh.writeString(Common::String::format("%s\r\n", hall_of_fame_[i].player_name_.c_str()));
			fh.writeString(Common::String::format("%d\r\n", hall_of_fame_[i].score_));
		}
		return true;
	}

	return false;
}

bool qdGameDispatcher::add_hall_of_fame_entry(int score) {
	if (!hall_of_fame_size_)
		return false;

	for (int i = 0; i < hall_of_fame_size_; i++) {
		if (score > hall_of_fame_[i].score_) {
			for (int j = hall_of_fame_size_ - 1; j > i; j--)
				hall_of_fame_[j] = hall_of_fame_[j - 1];
			hall_of_fame_[i].score_ = score;
			hall_of_fame_[i].player_name_ = "";
			hall_of_fame_[i].updated_ = true;

			return true;
		}
	}

	return false;
}

bool qdGameDispatcher::update_hall_of_fame_names() {
	if (!hall_of_fame_size_)
		return false;

	const qdInterfaceDispatcher::screen_list_t &list = interface_dispatcher_.screen_list();
	for (qdInterfaceDispatcher::screen_list_t::const_iterator it = list.begin(); it != list.end(); ++it) {
		const qdInterfaceScreen::element_list_t &el_list = (*it)->element_list();
		for (qdInterfaceScreen::element_list_t::const_iterator it1 = el_list.begin(); it1 != el_list.end(); ++it1) {
		}
	}

	return true;
}

} // namespace QDEngine
