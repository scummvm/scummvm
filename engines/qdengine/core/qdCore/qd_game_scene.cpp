/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_log_file.h"

#include "gr_dispatcher.h"
#include "xml_tag_buffer.h"
#include "mouse_input.h"
#include "plaympp_api.h"

#include "qdscr_parser.h"
#include "qd_minigame.h"
#include "qd_grid_zone.h"
#include "qd_music_track.h"
#include "qd_game_object_static.h"
#include "qd_game_object_animated.h"
#include "qd_game_object_moving.h"
#include "qd_game_object_mouse.h"
#include "qd_game_scene.h"
#include "qd_named_object_reference.h"
#include "qd_setup.h"

#include "qd_interface_button.h"
#include "qd_interface_screen.h"
#include "qd_game_dispatcher.h"

#include <stdio.h>
/* ----------------------------- STRUCT SECTION ----------------------------- */

struct qdGridZoneOrdering
{
	bool operator() (const qdGridZone* z0,const qdGridZone* z1){
		return z0 -> update_timer() < z1 -> update_timer();
	}
};

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

fpsCounter qdGameScene::fps_counter_ = fpsCounter(1000);
grScreenRegion qdGameScene::fps_region_ = grScreenRegion::EMPTY;
grScreenRegion qdGameScene::fps_region_last_ = grScreenRegion::EMPTY;
char qdGameScene::fps_string_[255];
std::vector<qdGameObject*> qdGameScene::visible_objects_;

qdGameScene::qdGameScene() : mouse_click_object_(NULL),
	mouse_right_click_object_(NULL),
	mouse_hover_object_(NULL),
#ifdef _QUEST_EDITOR
	active_object_(NULL),
#endif
	selected_object_(NULL),
	mouse_click_pos_(0,0),
	zone_update_count_(0)
#ifndef _QUEST_EDITOR
	, minigame_(NULL)
#endif // USE_NEW_PROJ
{
	set_loading_progress_callback(NULL);

	restart_minigame_on_load_ = false;

	autosave_slot_ = -1;
}

qdGameScene::~qdGameScene()
{
	grid_zones.clear();
}

void qdGameScene::init_objects_grid()
{
#ifndef _QUEST_EDITOR
	camera.drop_grid_attributes(sGridCell::CELL_OCCUPIED | sGridCell::CELL_PERSONAGE_OCCUPIED | sGridCell::CELL_SELECTED);

	for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io)
		(*io) -> save_grid_zone();
	
	for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io){
		if((*io) -> is_visible() && !(*io) -> check_flag(QD_OBJ_SCREEN_COORDS_FLAG))
			(*io) -> toggle_grid_zone();
	}
#endif
}

void qdGameScene::quant(float dt)
{
#ifndef _QUEST_EDITOR
	if(minigame_)
		minigame_ -> quant(dt);

	for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io)
		(*io) -> update_screen_pos();

	conditions_quant(dt);
#endif

	personages_quant();

#ifndef _QUEST_EDITOR
	follow_quant(dt);
	collision_quant();
#endif

	if(camera.quant(dt)){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> toggle_full_redraw();
	}

	qdGameDispatcherBase::quant(dt);

#ifndef _QUEST_EDITOR
	if(mouseDispatcher::instance() -> check_event(mouseDispatcher::EV_LEFT_DOWN) && selected_object_ && selected_object_ -> has_control_type(qdGameObjectMoving::CONTROL_MOUSE)){
		qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();
		if(dp && !dp -> check_flag(qdGameDispatcher::OBJECT_CLICK_FLAG | qdGameDispatcher::DIALOG_CLICK_FLAG) && selected_object_ -> can_move()){
			Vect3f pos = camera.get_cell_coords(camera.get_cell_index(mouse_click_pos_.x,mouse_click_pos_.y,false));

			selected_object_ -> set_queued_state(NULL);
			selected_object_ -> move(pos, false);

			// Для всех "следующих" нужно считать путь следования
			follow_pers_init(qdGameObjectMoving::FOLLOW_UPDATE_PATH);

			if (false == selected_object_->is_moving())
			{
				// Если активный не смог идти, но теоретически может, то он переходит в режим ожидания
				if (selected_object_->can_move())
					selected_object_->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
				else
					selected_object_->set_follow_condition(qdGameObjectMoving::FOLLOW_DONE);
			}
			else
				selected_object_->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);

			for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
				if (
					 ((*it) != selected_object_) && 
					 (*it)->has_control_type(qdGameObjectMoving::CONTROL_ACTIVE_CLICK_REACTING)
				   )
					(*it) -> move(pos, false);
		}
	}

	for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io){
		if(!(*io) -> check_flag(QD_OBJ_IS_IN_INVENTORY_FLAG))
			(*io) -> quant(dt);
	}

	update_mouse_cursor();

	if(selected_object_ && !selected_object_ -> is_visible()){
		for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it){
			if((*it) -> is_visible() && !(*it) -> check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)){
				set_active_personage(*it);
				break;
			}
		}
	}

	if(selected_object_) selected_object_ -> keyboard_move();
#else
	for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io){
		if(*io != active_object_)
			(*io) -> quant(0.0f);
	}

	if(active_object_)
		active_object_ -> quant(dt);
#endif

	mouse_click_object_ = mouse_right_click_object_ = NULL;
	mouse_hover_object_ = NULL;
}

void qdGameScene::redraw()
{
	if(!object_list().empty()){
#ifdef _QUEST_EDITOR
		personages_quant();
		init_visible_objects_list();
		for(std::vector<qdGameObject*>::reverse_iterator it = visible_objects_.rbegin(); it != visible_objects_.rend(); ++it){
			(*it) -> redraw();
		}
#else
		if(check_flag(CYCLE_X) || check_flag(CYCLE_Y)){
			const int sx = camera.get_scr_sx();
			const int sy = camera.get_scr_sy();

			int offs_x[8] = { -sx, -sx, 0, sx, sx, sx, 0, -sx };
			int offs_y[8] = { 0, -sy, -sy, -sy, 0, sy, sy, sy };

			switch(flags() & (CYCLE_X | CYCLE_Y)){
			case CYCLE_X:
				for(std::vector<qdGameObject*>::reverse_iterator it = visible_objects_.rbegin(); it != visible_objects_.rend(); ++it){
					for(int i = 0; i < 8; i += 4){
						Vect2i pos = (*it) -> screen_pos() + Vect2i(offs_x[i],offs_y[i]);
						Vect2i sz = (*it) -> screen_size();

						pos -= sz/2;

						if(grDispatcher::instance() -> is_rectangle_visible(pos.x,pos.y,sz.x,sz.y))
							(*it) -> redraw(offs_x[i],offs_y[i]);
					}
					(*it) -> redraw();
				}
				break;
			case CYCLE_Y:
				for(std::vector<qdGameObject*>::reverse_iterator it = visible_objects_.rbegin(); it != visible_objects_.rend(); ++it){
					for(int i = 2; i < 8; i += 4){
						Vect2i pos = (*it) -> screen_pos() + Vect2i(offs_x[i],offs_y[i]);
						Vect2i sz = (*it) -> screen_size();

						pos -= sz/2;

						if(grDispatcher::instance() -> is_rectangle_visible(pos.x,pos.y,sz.x,sz.y))
							(*it) -> redraw(offs_x[i],offs_y[i]);
					}
					(*it) -> redraw();
				}
				break;
			case CYCLE_X | CYCLE_Y:
				for(std::vector<qdGameObject*>::reverse_iterator it = visible_objects_.rbegin(); it != visible_objects_.rend(); ++it){
					for(int i = 0; i < 8; i++){
						Vect2i pos = (*it) -> screen_pos() + Vect2i(offs_x[i],offs_y[i]);
						Vect2i sz = (*it) -> screen_size();

						pos -= sz/2;

						if(grDispatcher::instance() -> is_rectangle_visible(pos.x,pos.y,sz.x,sz.y))
							(*it) -> redraw(offs_x[i],offs_y[i]);
					}
					(*it) -> redraw();
				}
				break;
			}
		}
		else {
			for(std::vector<qdGameObject*>::reverse_iterator it = visible_objects_.rbegin(); it != visible_objects_.rend(); ++it)
				(*it) -> redraw();
		}
#endif
	}

#ifdef _QUEST_EDITOR
	if(active_object_ && active_object_->named_object_type()!=QD_NAMED_OBJECT_STATIC_OBJ) {
		qdGameObjectAnimated* pobj = 
			static_cast<qdGameObjectAnimated*>(active_object_);
		if (pobj->named_object_type()==QD_NAMED_OBJECT_MOUSE_OBJ)
			pobj->redraw();
		if (pobj->has_bound()) 
			pobj->draw_bound();
	}
	camera.draw_grid();
#endif
}

bool qdGameScene::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	bool result = false;

#ifdef _QUEST_EDITOR
	init_visible_objects_list();
#endif

	camera.cycle_coords(x,y);
	mouse_click_pos_ = camera.scr2plane(Vect2s(x,y));

	switch(ev){
		case mouseDispatcher::EV_MOUSE_MOVE:
			if(qdInterfaceDispatcher* dp = qdInterfaceDispatcher::get_dispatcher()){
				if(dp -> is_mouse_hover()){
					result = false;
					break;
				}
			}
			for(std::vector<qdGameObject*>::iterator io = visible_objects_.begin(); io != visible_objects_.end(); ++io){
				if(!(*io) -> check_flag(QD_OBJ_DISABLE_MOUSE_FLAG) && (*io) -> named_object_type() != QD_NAMED_OBJECT_STATIC_OBJ){
					if((*io) -> hit(x,y)){
						mouse_hover_object_ = *io;
						break;
					}
				}
			}
			break;
		case mouseDispatcher::EV_LEFT_DOWN:
		case mouseDispatcher::EV_RIGHT_DOWN:
			if(qdInterfaceDispatcher* dp = qdInterfaceDispatcher::get_dispatcher()){
				if(dp -> is_mouse_hover()){
					result = false;
					break;
				}
			}
			qdGameObject* pObj = get_hitted_obj(x, y);
			if (NULL != pObj)
			{
				result = pObj -> mouse_handler(x,y,ev);
				if(ev == mouseDispatcher::EV_LEFT_DOWN)
					mouse_click_object_ = pObj;
				else
					mouse_right_click_object_ = pObj;
			}
			break;
	}

	return result;
}

qdGameObject* qdGameScene::get_hitted_obj(int x, int y)
{
	for(std::vector<qdGameObject*>::iterator io = visible_objects_.begin(); io != visible_objects_.end(); ++io)
	{
		if(!(*io) -> check_flag(QD_OBJ_DISABLE_MOUSE_FLAG) && (*io) -> named_object_type() != QD_NAMED_OBJECT_STATIC_OBJ)
			if((*io) -> hit(x,y))
				return (*io);
	}
	return NULL;
}

void qdGameScene::load_script(const xml::tag* p)
{
	load_conditions_script(p);
	qdGameDispatcherBase::load_script_body(p);

	qdGameObject* obj;
	qdGridZone* grz;
	qdMusicTrack* trk;

//#ifndef _QUEST_EDITOR
	personages_.clear();
	int personages_count = 0;
//#endif

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_NAME:
			set_name(it -> data());
			break;
		case QDSCR_MINIGAME_GAME_NAME:
			set_minigame_name(it -> data());
			break;
		case QDSCR_SCENE_RESTART_MINIGAME:
			toggle_restart_minigame_on_load(xml::tag_buffer(*it).get_int() != 0);
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_CD:
			set_CD_info(xml::tag_buffer(*it).get_uint());
			break;
		case QDSCR_STATIC_OBJECT:
			obj = new qdGameObjectStatic;
			obj -> load_script(&*it);
			add_object(obj);
			break;
		case QDSCR_ANIMATED_OBJECT:
			obj = new qdGameObjectAnimated;
			obj -> load_script(&*it);
			add_object(obj);
			break;
		case QDSCR_MOVING_OBJECT:
			obj = new qdGameObjectMoving;
			obj -> load_script(&*it);
			add_object(obj);
//#ifndef _QUEST_EDITOR
			personages_count++;
//#endif
			break;
		case QDSCR_CAMERA:
			camera.load_script(&*it);
			break;
		case QDSCR_GRID_ZONE:
			grz = new qdGridZone;
			grz -> load_script(&*it);
			add_grid_zone(grz);
			break;
		case QDSCR_MUSIC_TRACK:
			trk = new qdMusicTrack;
			trk -> load_script(&*it);
			add_music_track(trk);
			break;
		case QDSCR_SCENE_SAVE_SLOT:
			xml::tag_buffer(*it) > autosave_slot_;
			break;
		case QDSCR_INTERFACE_SCREEN:
			if(const xml::tag* name_tag = it -> search_subtag(QDSCR_NAME))
				set_interface_screen_name(name_tag -> data());
			break;
		}
	}

#ifndef _QUEST_EDITOR
	visible_objects_.reserve(object_list().size());
#endif

	personages_.reserve(personages_count);

	for(qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it){
		if((*it) -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
			personages_.push_back(static_cast<qdGameObjectMoving*>(*it));
	}

	camera.set_cycle(check_flag(CYCLE_X),check_flag(CYCLE_Y));
}


bool qdGameScene::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<scene name=\"" < qdscr_XML_string(name()) < "\"";

	if(CD_info())
		fh < " cd=\"" <= CD_info() < "\"";

	if(flags())
		fh < " flags=\"" <= flags() < "\"";

	if(autosave_slot_ != -1)
		fh < " save_slot=\"" <= autosave_slot_ < "\"";

	if(restart_minigame_on_load_)
		fh < " restart_minigame=\"1\"";

	if(has_minigame())
		fh < " game_name=\"" < qdscr_XML_string(minigame_name()) < "\"";

	fh < ">\r\n";

	if(has_interface_screen()){
		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "<interface_screen name=\"" < qdscr_XML_string(interface_screen_name()) < "\"/>\r\n";
	}

	qdGameDispatcherBase::save_script_body(fh,indent);

	camera.save_script(fh,indent + 1);

	for(qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it)
		(*it) -> save_script(fh,indent + 1);

	for(qdGridZoneList::const_iterator it = grid_zone_list().begin(); it != grid_zone_list().end(); ++it)
		(*it) -> save_script(fh,indent + 1);

	for(qdMusicTrackList::const_iterator it = music_track_list().begin(); it != music_track_list().end(); ++it)
		(*it) -> save_script(fh,indent + 1);

	save_conditions_script(fh,indent);

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</scene>\r\n";

	return true;
}

int qdGameScene::load_resources()
{
	appLog::default_log() << appLog::default_log().time_string() << " Загрузка сцены \"" << name() << "\"\r\n";

	int total_size = get_resources_size();
	set_resources_size(total_size);

	qdGameDispatcher* dsp = qd_get_game_dispatcher();
	if(dsp)
		set_loading_progress_callback(dsp -> get_scene_loading_progress_callback(),dsp -> get_scene_loading_progress_data());

	int size = qdGameDispatcherBase::load_resources();

	qdGameObjectList::const_iterator io;
	FOR_EACH(object_list(),io){
		(*io) -> load_resources();
		show_loading_progress(1);
		size ++;
	}

	set_resources_size(0);
	fps_counter_.reset();

	return size;
}

void qdGameScene::free_resources()
{
	if(qdGameDispatcher* dp = qd_get_game_dispatcher()){
		if(dp->current_music() && !dp->current_music()->check_flag(QD_MUSIC_TRACK_DISABLE_SWITCH_OFF))
			dp -> stop_music();
	}

	qdGameObjectList::const_iterator io;
	FOR_EACH(object_list(),io)
		(*io) -> free_resources();

	qdGameDispatcherBase::free_resources();
}

void qdGameScene::debug_redraw()
{
	if(qdGameConfig::get_config().show_fps())
		grDispatcher::instance() -> DrawText(10,10,grDispatcher::instance() -> make_rgb888(255,255,255),fps_string_);
	
#ifdef __QD_DEBUG_ENABLE__
	if(qdGameConfig::get_config().debug_draw()){												  
		if(selected_object_){
			static char buffer[256];
			sprintf(buffer,"%.1f %.1f %.1f, %.1f",selected_object_ -> R().x,selected_object_ -> R().y,selected_object_ -> R().z, R2G(selected_object_->direction_angle()));
			grDispatcher::instance() -> DrawText(10,30,grDispatcher::instance() -> make_rgb888(255,255,255),buffer);
			float z = camera.global2camera_coord(selected_object_ -> R()).z;
			sprintf(buffer,"D: %.2f",z);
			grDispatcher::instance() -> DrawText(10,50,grDispatcher::instance() -> make_rgb888(255,255,255),buffer);

			if(selected_object_ -> get_cur_state() && selected_object_ -> get_cur_state() -> name())
			grDispatcher::instance() -> DrawText(10,70,grDispatcher::instance() -> make_rgb888(255,255,255),selected_object_ -> get_cur_state() -> name());

			sprintf(buffer,"%d %d",camera.get_scr_center_x(),camera.get_scr_center_y());
			grDispatcher::instance() -> DrawText(10,90,grDispatcher::instance() -> make_rgb888(255,255,255),buffer);
/*
			sprintf(buffer,"%d %d",mouseDispatcher::instance()->mouse_x(),mouseDispatcher::instance()->mouse_y());
			grDispatcher::instance() -> DrawText(10,130,grDispatcher::instance() -> make_rgb888(255,255,255),buffer);
*/
		}

		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
			if(dp->current_music()){
				grDispatcher::instance() -> DrawText(10,130,grDispatcher::instance() -> make_rgb888(255,255,255), dp->current_music()->file_name());
			}
		}
/*
		const cl = 0x202020;
		
		Vect2s v0,v1;
		v0 = qdCamera::current_camera() -> global2scr(Vect3f(-300,-300,0));
		v1 = qdCamera::current_camera() -> global2scr(Vect3f(300,300,0));
		grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,cl,2);

		v0 = qdCamera::current_camera() -> global2scr(Vect3f(-300,300,0));
		v1 = qdCamera::current_camera() -> global2scr(Vect3f(300,-300,0));
		grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,cl,2);
*/
		if(qdGameConfig::get_config().debug_show_grid())
			camera.draw_grid();

		for(std::vector<qdGameObject*>::reverse_iterator it = visible_objects_.rbegin(); it != visible_objects_.rend(); ++it)
			(*it) -> debug_redraw();
	}
#endif
}

int qdGameScene::get_resources_size()
{
	return object_list().size() + qdGameDispatcherBase::get_resources_size();
}

bool qdGameScene::activate()
{
	appLog::default_log() << "активация сцены " << name() << "\r\n";

	camera.quant(0.0f);

	// При активации сцены все объекты следования переводим в нормальное состояние
	follow_pers_init(qdGameObjectMoving::FOLLOW_DONE);

#ifndef _QUEST_EDITOR
	for(qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it){
		(*it) -> init_grid_zone();
		if(qdGameObjectAnimated* p = dynamic_cast<qdGameObjectAnimated*>(*it))
			p -> set_states_owner();
	}

	qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();

	for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it){
		if(qdGameObjectState* p = dp -> get_walk_state((*it) -> name()))
			(*it) -> set_last_walk_state(p);
	}
#else
	split_global_objects();
	merge_global_objects();
#endif

	if(!get_active_personage()){
		for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io){
			if((*io) -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ && !(*io) -> check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)){
				set_active_personage(static_cast<qdGameObjectMoving*>(*io));
				break;
			}
		}
	}

	for(qdGridZoneList::const_iterator iz = grid_zone_list().begin(); iz != grid_zone_list().end(); ++iz)
		(*iz) -> set_state((*iz) -> state());

	init_visible_objects_list();

	return true;
}

bool qdGameScene::deactivate()
{
#ifndef _QUEST_EDITOR
	if(minigame_)
		minigame_ -> end();
#endif

	return true;
}

bool qdGameScene::merge_global_objects(qdGameObject* obj)
{
	qdGameDispatcher* gd = qd_get_game_dispatcher();
	if(!gd) return false;

	if(!obj){
		qdGameObjectList::const_iterator it;
		FOR_EACH(object_list(),it){
			if((*it) -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ){
				qdGameObject* p = gd -> get_global_object((*it) -> name());
				if(p && p -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ){
					static_cast<qdGameObjectMoving*>(*it) -> merge(static_cast<qdGameObjectMoving*>(p));
				}
			}
		}
	}
	else {
		qdGameObjectList::const_iterator it;
		FOR_EACH(object_list(),it){
			if((*it) -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ){
				if((*it) -> name() && obj -> name() && !strcmp((*it) -> name(),obj -> name())){
					static_cast<qdGameObjectMoving*>(*it) -> merge(static_cast<qdGameObjectMoving*>(obj));
				}
			}
		}
	}

	return true;
}

bool qdGameScene::split_global_objects(qdGameObject* obj)
{
	qdGameDispatcher* gd = qd_get_game_dispatcher();
	if(!gd) return false;

	if(!obj){
		qdGameObjectList::const_iterator it;
		FOR_EACH(object_list(),it){
			if((*it) -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ){
				qdGameObject* p = gd -> get_global_object((*it) -> name());
				if(p && p -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ){
					static_cast<qdGameObjectMoving*>(*it) -> split(static_cast<qdGameObjectMoving*>(p));
				}
			}
		}
	}
	else {
		qdGameObjectList::const_iterator it;
		FOR_EACH(object_list(),it){
			if((*it) -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ){
				if((*it) -> name() && obj -> name() && !strcmp((*it) -> name(),obj -> name())){
					static_cast<qdGameObjectMoving*>(*it) -> split(static_cast<qdGameObjectMoving*>(obj));
				}
			}
		}
	}

	return true;
}

struct qdObjectOrdering 
{
	bool operator ()(const qdGameObject* p0,const qdGameObject* p1){
		return p0 -> screen_depth() < p1 -> screen_depth();
	}
};

bool qdGameScene::init_visible_objects_list()
{
	visible_objects_.clear();

	qdGameObjectList::const_iterator it;
	FOR_EACH(object_list(),it){
		qdGameObject* p = *it;
		(*it) -> update_screen_pos();
		if((*it) -> is_visible() && !(*it) -> check_flag(QD_OBJ_SCREEN_COORDS_FLAG))
			visible_objects_.push_back(*it);
	}

	std::sort(visible_objects_.begin(),visible_objects_.end(),qdObjectOrdering());

	return true;
}

bool qdGameScene::add_object(qdGameObject* p)
{
	if(objects.add_object(p)){ 
		p -> set_owner(this); 
#ifdef _QUEST_EDITOR
		reload_personage_list();
#endif
		return true; 
	} 
	return false; 
}

bool qdGameScene::rename_object(qdGameObject* p,const char* name)
{
	return objects.rename_object(p,name);
}

bool qdGameScene::remove_object(const char* name)
{ 

	if (objects.remove_object(name))
	{
#ifdef _QUEST_EDITOR
		reload_personage_list();
#endif
		return true; 
	}
	return false;
}

bool qdGameScene::remove_object(qdGameObject* p)
{ 
	if (objects.remove_object(p))
	{
#ifdef _QUEST_EDITOR
		reload_personage_list();
#endif
		return true; 
	}
	return false;
}

qdGameObject* qdGameScene::get_object(const char* name)
{ 
	return objects.get_object(name); 
}

bool qdGameScene::is_object_in_list(const char* name)
{
	return objects.is_in_list(name);
}

bool qdGameScene::is_object_in_list(qdGameObject* p)
{
	return objects.is_in_list(p);
}

bool qdGameScene::add_grid_zone(qdGridZone* p)
{ 
	if(grid_zones.add_object(p)){
		p -> set_owner(this);
		return true;
	}

	return false;
}

bool qdGameScene::rename_grid_zone(qdGridZone* p,const char* name)
{
	return grid_zones.rename_object(p,name);
}

bool qdGameScene::remove_grid_zone(const char* name)
{ 
	return grid_zones.remove_object(name); 
}

bool qdGameScene::remove_grid_zone(qdGridZone* p)
{ 
	return grid_zones.remove_object(p); 
}

qdGridZone* qdGameScene::get_grid_zone(const char* name)
{ 
	return grid_zones.get_object(name); 
}

bool qdGameScene::is_grid_zone_in_list(const char* name)
{ 
	return grid_zones.is_in_list(name); 
}

bool qdGameScene::is_grid_zone_in_list(qdGridZone* p)
{
	return grid_zones.is_in_list(p); 
}

#ifdef _QUEST_EDITOR
bool qdGameScene::insert_grid_zone(qdGridZone* p, const qdGridZone* before)
{
	if (grid_zones.insert_object(p, before))
	{
		p -> set_owner(this);
		return true;
	}
	return false;
}
#endif // _QUEST_EDITOR

bool qdGameScene::add_music_track(qdMusicTrack* p)
{
	if(music_tracks.add_object(p)){
		p -> set_owner(this);
		return true;
	}

	return false;
}

bool qdGameScene::rename_music_track(qdMusicTrack* p,const char* name)
{
	return music_tracks.rename_object(p,name);
}

bool qdGameScene::remove_music_track(const char* name)
{
	return music_tracks.remove_object(name); 
}

bool qdGameScene::remove_music_track(qdMusicTrack* p)
{
	return music_tracks.remove_object(p); 
}

qdMusicTrack* qdGameScene::get_music_track(const char* name)
{
	return music_tracks.get_object(name); 
}

bool qdGameScene::is_music_track_in_list(const char* name) const
{
	return music_tracks.is_in_list(name); 
}

bool qdGameScene::is_music_track_in_list(qdMusicTrack* p) const
{
	return music_tracks.is_in_list(p); 
}

bool qdGameScene::load_data(qdSaveStream& fh,int save_version)
{
	if(!qdConditionalObject::load_data(fh,save_version)) return false;

	if(!camera.load_data(fh,save_version)) return false;

	for(qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it){
		if(!(*it) -> load_data(fh,save_version))
			return false;
	}

	if(size_t sz = grid_zone_list().size()){
		for(qdGridZoneList::const_iterator it = grid_zone_list().begin(); it != grid_zone_list().end(); ++it){
		if(!(*it) -> load_data(fh,save_version))
			return false;
		}

		std::vector<qdGridZone*> zone_order;
		zone_order.reserve(sz);
		zone_order.insert(zone_order.end(),grid_zone_list().begin(),grid_zone_list().end());
		std::sort(zone_order.begin(),zone_order.end(),qdGridZoneOrdering());

		zone_update_count_ = 0;

		for(std::vector<qdGridZone*>::iterator it = zone_order.begin(); it != zone_order.end(); ++it)
			(*it) -> set_state((*it) -> state());
	}

	int fl;
	fh > fl;

	if(fl){
		qdNamedObjectReference ref;
		if(!ref.load_data(fh,save_version))
			return false;

		if(qdGameDispatcher* p = qd_get_game_dispatcher())
			selected_object_ = static_cast<qdGameObjectMoving*>(p -> get_named_object(&ref));

		if(!selected_object_) return false;

		selected_object_ -> toggle_selection(true);

		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			dp -> toggle_inventory(true);
	}
	else
		selected_object_ = NULL;

#ifndef _QUEST_EDITOR
	if(save_version >= 107){
		const int save_buf_sz = 64 * 1024;
		char save_buf[save_buf_sz];

		fh > fl;
		if(fl)
			fh.read(save_buf, fl);
		if(minigame_)
			minigame_->load_game(save_buf, fl, this);
	}
#endif

	return true;
}

bool qdGameScene::save_data(qdSaveStream& fh) const
{
	if(!qdConditionalObject::save_data(fh)) return false;

	if(!camera.save_data(fh)) return false;

	for(qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it){
		if(!(*it) -> save_data(fh))
			return false;
	}

	for(qdGridZoneList::const_iterator it = grid_zone_list().begin(); it != grid_zone_list().end(); ++it){
		if(!(*it) -> save_data(fh))
			return false;
	}

	if(selected_object_){
		fh < (int)1;
		qdNamedObjectReference ref(selected_object_);
		if(!ref.save_data(fh)) 
			return false;
	}
	else
		fh < (int)0;

#ifndef _QUEST_EDITOR
	if(minigame_){
		const int save_buf_sz = 64 * 1024;
		char save_buf[save_buf_sz];
		int size = minigame_->save_game(save_buf, save_buf_sz, const_cast<qdGameScene*>(this));
		fh < size;
		if(size)
			fh.write(save_buf, size);
	}
	else
		fh < (int)0;
#endif

	return true;
}

void qdGameScene::set_active_object(qdGameObjectAnimated* p)
{
	if(p -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
		set_active_personage(static_cast<qdGameObjectMoving*>(p));
}

void qdGameScene::set_active_personage(qdGameObjectMoving* p)
{
	if(p && !p -> is_visible())
		return;

	if(selected_object_)
		selected_object_ -> toggle_selection(false);

	selected_object_ = p;
	if (selected_object_)
		selected_object_ -> toggle_selection(true);

	camera.set_default_object(p);

#ifndef _QUEST_EDITOR

	if(p && p -> has_camera_mode()){
		camera.set_mode(p -> camera_mode(),p);
		camera.set_default_mode(p -> camera_mode());
	}

	if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
		dp -> toggle_inventory(true);
	}

	follow_pers_init(qdGameObjectMoving::FOLLOW_DONE); // При смене активного останавливаем следование
	for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it){
		if((*it) != p && !(*it) -> check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)){
			if((*it) -> check_flag(QD_OBJ_MOVING_FLAG)){
				(*it) -> set_queued_state(NULL);
				(*it) -> stop_movement();
			}
		}
	}
#endif
}

bool qdGameScene::init()
{
	if(!qdGameDispatcherBase::init()) return false;

	zone_update_count_ = 0;
	camera.init();

	selected_object_ = NULL;

	std::for_each(object_list().begin(), object_list().end(), 
		std::mem_fun(qdGameObject::init));

	std::for_each(grid_zone_list().begin(), grid_zone_list().end(), 
		std::mem_fun(qdGridZone::init));

	std::for_each(music_track_list().begin(), music_track_list().end(), 
				std::mem_fun(qdMusicTrack::init));

#ifndef _QUEST_EDITOR
	if(has_minigame() && !minigame_){
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
			minigame_ = dp -> get_minigame(minigame_name());

		if(minigame_)
			create_minigame_objects();
	}
#endif

	return true;
}

bool qdGameScene::change_active_personage(void)
{
#ifndef _QUEST_EDITOR
	if(selected_object_){
		personages_container_t::iterator it = std::find(personages_.begin(),personages_.end(),selected_object_);
		if(it == personages_.end()) return false;

		do {
			if(++it == personages_.end())
				it = personages_.begin();

			if(!(*it) -> check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)){
				if(*it != selected_object_)
					set_active_personage(*it);
				return true;
			}

		} while(*it != selected_object_);
	}
	else {
		for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it){
			if(!(*it) -> check_flag(QD_OBJ_NON_PLAYER_PERSONAGE_FLAG)){
				set_active_personage(*it);
				return true;
			}
		}
	}
#endif

	return false;
}

void qdGameScene::pre_redraw()
{
#ifndef _QUEST_EDITOR
	qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();
	if(!dp) return;

	init_visible_objects_list();

	if(!dp -> need_full_redraw()){
		if(qdGameConfig::get_config().show_fps()){
			const int sx = 80;
			const int sy = 20;
			dp -> add_redraw_region(grScreenRegion(10 + sx/2,10 + sy/2,sx,sy));
		}

		for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io){
			if(!(*io) -> check_flag(QD_OBJ_IS_IN_INVENTORY_FLAG) && (*io) -> need_redraw()){
				add_redraw_region((*io) -> last_screen_region());
				add_redraw_region((*io) -> screen_region());
			}
		}
		
		if(!fps_region_.is_empty())
			dp -> add_redraw_region(fps_region_);
		if(!fps_region_last_.is_empty())
			dp -> add_redraw_region(fps_region_last_);
	}

	if(qdGameConfig::get_config().show_fps()){
#ifdef __QD_DEBUG_ENABLE__
		static unsigned int memory_usage = 0;
		static int mem_update_counter = 20;
		if(++mem_update_counter >= 20) 
			memory_usage = app_memory_usage();
#endif

		if(fps_counter_.fps_value() > 0.0f)
#ifdef __QD_DEBUG_ENABLE__
			sprintf(fps_string_,"%.1f fps\nmemory: %.2f MB",fps_counter_.fps_value(),float(memory_usage) / 1024.0f / 1024.0f);
#else
			sprintf(fps_string_,"%.1f fps",fps_counter_.fps_value());
#endif
		else
			sprintf(fps_string_,"--");

		int sx = grDispatcher::instance() -> TextWidth(fps_string_);
		int sy = grDispatcher::instance() -> TextHeight(fps_string_);
		fps_region_ = grScreenRegion(10 + sx/2,10 + sy/2,sx,sy);
	}
	else
		fps_region_.clear();

	fps_counter_.quant();
#endif
}

void qdGameScene::post_redraw()
{
	for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io)
		(*io) -> post_redraw();

	fps_region_last_ = fps_region_;
}

qdConditionalObject::trigger_start_mode qdGameScene::trigger_start()
{
	if(qdGameDispatcher* dp = qd_get_game_dispatcher()){
		dp -> set_next_scene(this);

		if(check_flag(RESET_TRIGGERS_ON_LOAD))
			dp -> deactivate_scene_triggers(this);

		appLog::default_log() << appLog::default_log().time_string() << " Scene queued -> " << name() << "\r\n";

		return qdConditionalObject::TRIGGER_START_ACTIVATE;
	}

	return qdConditionalObject::TRIGGER_START_FAILED;
}

bool qdGameScene::is_active() const
{
	if(qdGameDispatcher* dp = qd_get_game_dispatcher())
		return (dp -> get_active_scene() == this);

	return false;
}

void qdGameScene::update_mouse_cursor()
{
	qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();
	if(!dp) return;

	if(dp -> mouse_object() -> object()){
		if(const qdGameObjectState* p = dp -> mouse_object() -> object() -> get_cur_state()){
			if(p -> has_mouse_cursor_ID()){
				dp -> mouse_object() -> set_cursor_state(p -> mouse_cursor_ID());
				return;
			}
		}
	}

	if(mouse_hover_object_){
		int cursor = qdGameObjectState::CURSOR_UNASSIGNED;
		qdGameObjectMouse::cursor_ID_t obj_cursor = qdGameObjectMouse::OBJECT_CURSOR;

		if(const qdGameObjectAnimated* obj = dynamic_cast<const qdGameObjectAnimated*>(mouse_hover_object_)){
			if(!obj -> get_cur_state() -> check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_INVENTORY))
				cursor = obj -> mouse_cursor_ID();
			else
				obj_cursor = qdGameObjectMouse::INVENTORY_OBJECT_CURSOR;
		}

		if(cursor == qdGameObjectState::CURSOR_UNASSIGNED)
			dp -> mouse_object() -> set_cursor(obj_cursor);
		else
			dp -> mouse_object() -> set_cursor_state(cursor);
	}
	else {
		qdGameObjectMouse::cursor_ID_t mouse_cursor = qdGameObjectMouse::DEFAULT_CURSOR;

		qdInterfaceDispatcher* ip = qdInterfaceDispatcher::get_dispatcher();
		if(ip && ip -> is_mouse_hover()){
			if(qdGameDispatcher* dsp = qdGameDispatcher::get_dispatcher())
				mouse_cursor = qdGameObjectMouse::INGAME_INTERFACE_CURSOR;
		}
		else {
			Vect2f r = camera.scr2plane(Vect2s(dp -> mouse_cursor_pos().x,dp -> mouse_cursor_pos().y));
			for(qdGridZoneList::const_iterator it = grid_zone_list().begin(); it != grid_zone_list().end(); ++it){
				if((*it) -> check_flag(qdGridZone::ZONE_EXIT_FLAG) && (*it) -> is_point_in_zone(r)){
					mouse_cursor = qdGameObjectMouse::ZONE_CURSOR;
					break;
				}
			}
		}

		dp -> mouse_object() -> set_cursor(mouse_cursor);
	}
}

// Пересечение прямоугольников с центрами в c* и с размерами gr*
bool inters3f(const Vect3f& c1, const Vect3f& sz1, const Vect3f& c2, const Vect3f& sz2)
{
	Vect3f a1, b1, a2, b2;
	a1 = c1 - sz1/2;
	b1 = a1 + sz1;
	a2 = c2 - sz2/2;
	b2 = a2 + sz2;
	
	if ((max(a1.x, a2.x) <= min(b1.x, b2.x)) &&
		(max(a1.y, a2.y) <= min(b1.y, b2.y)) &&
		(max(a1.z, a2.z) <= min(b1.z, b2.z)))
		return true;
	else return false;
}

bool inters2s(Vect2s c1, Vect2s sz1, Vect2s c2, Vect2s sz2)
{
	Vect2s a1, b1, a2, b2;
	
	a1 = c1 - sz1/2;
	b1 = a1 + sz1;
	a2 = c2 - sz2/2;
	b2 = a2 + sz2;
	b1.x--;
	b1.y--;
	b2.x--;
	b2.y--;
	
	if ((max(a1.x, a2.x) <= min(b1.x, b2.x)) &&
		(max(a1.y, a2.y) <= min(b1.y, b2.y)))
		return true;
	else return false;
}

void qdGameScene::follow_pers_init(int follow_cond)
{
	for (personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
	{
		(*it)->ref_circuit_objs().clear();
		(*it)->set_follow_condition(follow_cond);
		if (qdGameObjectMoving::FOLLOW_DONE == follow_cond)
			(*it)->set_last_move_order((*it)->R());
	}
}

bool qdGameScene::follow_path_seek(qdGameObjectMoving* pObj, bool lock_target)
{
	if (qdGameObjectMoving::FOLLOW_UPDATE_PATH == pObj->follow_condition())
		selected_object_->set_grid_zone_attributes(sGridCell::CELL_SELECTED);

	return pObj->move(selected_object_->last_move_order(), lock_target);

	if (qdGameObjectMoving::FOLLOW_UPDATE_PATH == pObj->follow_condition())
		selected_object_->drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
}

/*
bool qdGameScene::follow_path_seek(qdGameObjectMoving* pObj, bool lock_target)
{
	Vect3f dest_pnt = selected_object_->last_move_order();
	if (pObj->has_control_type(
	      qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING))
	{
		dest_pnt.x += pObj->attach_shift().x;
		dest_pnt.y += pObj->attach_shift().y;
	};

	Vect3f dist_vec = dest_pnt - pObj->R();
	// Если точка, к которой нужно приблизиться достаточно близка, то идти к ней не нужно, но все путь найден удачно
	// (пустой путь)
	if (dist_vec.norm() <= pObj->follow_max_radius())
		return true;
	else
	{
		// Ищем точку, к которой сможем пойти (min)
		int lin  = pObj->follow_min_radius();
		int diag = round(lin * (0.7071067811)); // 0.7071067811 = sqrt(2)/2
		Vect2s test_pnt;
		Vect2s min;
		double min_dist = 1e100;
		Vect2s center;
		center.x = static_cast<short>(dest_pnt.x);
		center.y = static_cast<short>(dest_pnt.y);
		std::vector<Vect2s> pts_vec;
		for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
			{
				if ((0 == i) && (0 == j)) continue;
				if (min_dist < 1e100) break;

				test_pnt = center;
				// Для диагональных сумма кратна 2
				if (0 == ((i + j) % 2))
				{
					test_pnt.x += i*diag;
					test_pnt.y += j*diag;
				}
				else
				{
					test_pnt.x += i*lin;
					test_pnt.y += j*lin;
				};
				
				// Кладем все допустимые ячейки для подхода в массив (в порядки возростания длины)
				// Допустимой считаем ячейку, которая проходима. Выделенные и прочие
				// ячейки не учитываем, потому как они не постоянны - мы считаем, что к ним
				// можно бежать
				if (!qdCamera::current_camera()->check_grid_attributes(
					  test_pnt,
					  Vect2s(1,1),							   								  
			          sGridCell::CELL_IMPASSABLE)
				   )
				{
					Vect3f buf = qdCamera::current_camera()->get_cell_coords(test_pnt);
					Vect3f ins_pnt_dist = buf - pObj->R();
					bool is_ins = false;
					for (std::vector<Vect2s>::iterator ins_it = pts_vec.begin();
						 ins_it != pts_vec.end();
						 ++ins_it)
					{
						Vect3f cur_pnt_dist;
						cur_pnt_dist = qdCamera::current_camera()->get_cell_coords(*ins_it) - pObj->R();
						// Добавляем перед первой точкой, превосходящей расстоянием
						if (ins_pnt_dist.norm2() < cur_pnt_dist.norm2())
						{
							is_ins = true;
							pts_vec.insert(ins_it, test_pnt);
							break;
						}
					}
					if (false == is_ins) pts_vec.push_back(test_pnt);
				}
			};
		// Добавляем точку непосредственного подхода
		if (0 != pObj->follow_max_radius())
		{
			Vect3f dist = dest_pnt - pObj->R();
			float mul = 1 - pObj->follow_max_radius()/dist.norm();
			test_pnt.x = (dest_pnt.x - pObj->R().x)*mul + pObj->R().x;
			test_pnt.y = (dest_pnt.y - pObj->R().y)*mul + pObj->R().y;
			// В начало, как, очевидно, самую близкую
			pts_vec.insert(pts_vec.begin(), test_pnt);
		}
		// Пытаемся подойти ко всем точкам. При первой удачной выходим
		for (std::vector<Vect2s>::const_iterator it = pts_vec.begin();
		     it != pts_vec.end();
			 ++it)
		{
			if (true == pObj->move(Vect3f((*it).x, (*it).y, 0), lock_target))
				return true;
		}
	};

	return false; // Не удалось найти путь
}
*/

void qdGameScene::follow_implement_update_path()
{	
	for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
	{
		// Следование за активным персонажем, а так же следов. за точкой привязки к активному.
		// Ищем новую точку для следования, только если персонаж не пропускает сейчас активного
		if ((
			  (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
			  (*it)->has_control_type(
			    qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
			) &&
			(NULL != selected_object_) && 
			(*it != selected_object_) &&
			(qdGameObjectMoving::FOLLOW_UPDATE_PATH == (*it)->follow_condition()) &&
			(*it)->can_move()
		   )
		{
			Vect3f dist = selected_object_->R() - (*it)->R();
			// Если активный близко и движется, то никуда не идем (ждем пока отойдет)
			if ((selected_object_->is_moving()) && 
				(dist.norm() < (*it)->follow_min_radius())) continue;
			// Пытаемся найти путь, который будет идти непосредственно к цели (lock_target = true)
	        // иначе будет плохо следовать, довольствясь подоходом к краю препятствия

			if (follow_path_seek((*it), true))
				(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);
			else
				(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
		}
	};
}

void qdGameScene::follow_wakening()
{	
	// Возобновление следования если движущиеся и участвующие в следовании достаточно
	// удалились
	for(personages_container_t::iterator it1 = personages_.begin(); it1 != personages_.end(); ++it1)
		if ((
			  (selected_object_ == (*it1)) ||
			  (*it1)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
			  (*it1)->has_control_type(
				qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
			) &&
			(qdGameObjectMoving::FOLLOW_WAIT == (*it1)->follow_condition()))
		{
			Vect3f dist_vec;
			bool all_follow_moving_far = true;
			for (personages_container_t::iterator it2 = personages_.begin(); it2 != personages_.end(); ++it2)
				if ((
					  (selected_object_ == (*it2)) ||
					  (*it2)->has_control_type(
				  		  qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
					  (*it2)->has_control_type(
						  qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
					) &&
					(it1 != it2))
			{
				dist_vec = (*it1)->R() - (*it2)->R();
				// Считаем далеким, если расстояние между персонажами в два раза
				// превышает сумму их collision_radius.
				if (dist_vec.norm() < 2*((*it1)->collision_radius() + (*it2)->collision_radius()))
				{
					all_follow_moving_far = false;
					break;
				}
			}

			if (true == all_follow_moving_far)
			{
				bool any_move = false;
				if (((*it1) == selected_object_) && ((*it1)->move((*it1)->last_move_order(), false)))
					any_move = true;
				
				if ((*it1) != selected_object_)
				{
					dist_vec = selected_object_->R() - (*it1)->R();
					// Если достаточно далеко от следующего, то не учитываем занятое активным при поиске пути
					if (dist_vec.norm2() > sqr(selected_object_->collision_radius() + 10 + (*it1)->collision_radius()))
						selected_object_->set_grid_zone_attributes(sGridCell::CELL_SELECTED);

					follow_path_seek(*it1, false);

					selected_object_->drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
					any_move = true;
				}
				
				if (any_move)
					(*it1)->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);
				else
					(*it1)->set_follow_condition(qdGameObjectMoving::FOLLOW_FULL_STOP_WAIT);
			}
		}

	// Смотрим, все ли участники следования остановились
	bool is_all_stay = true;
	for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
		// Ждем пока остановятся ВСЕ персонажи
		if (/*
			(
			  (selected_object_ == (*it)) ||
			  (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
			  (*it)->has_control_type(
			    qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
			) &&
			*/
			(*it)->is_moving())
		{
			is_all_stay = false;
			break;
		}

	// Если все стоят, то одному из ждущих можно попытаться пройти к точке следования
	if (is_all_stay)
	{
		for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
			if ((
				  (selected_object_ == (*it)) ||
				  (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
				  (*it)->has_control_type(
					qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
				) &&
				(
				  (qdGameObjectMoving::FOLLOW_WAIT == (*it)->follow_condition()) ||
				  (qdGameObjectMoving::FOLLOW_FULL_STOP_WAIT == (*it)->follow_condition())
				) &&
				(*it)->can_move())
			{
				bool any_move = false;
				if (((*it) == selected_object_) && ((*it)->move((*it)->last_move_order(), false)))
					any_move = true;
				
				if ((*it) != selected_object_)
				{
					Vect3f dist_vec = selected_object_->R() - (*it)->R();
					// Если достаточно далеко от следующего, то не учитываем занятое активным при поиске пути
					if (dist_vec.norm2() > sqr(selected_object_->collision_radius() + 10 + (*it)->collision_radius()))
						selected_object_->set_grid_zone_attributes(sGridCell::CELL_SELECTED);

					follow_path_seek(*it, false);

					selected_object_->drop_grid_zone_attributes(sGridCell::CELL_SELECTED);
					any_move = true;
				}
				
				if (any_move)
				{
					// Если получилось идти, то сразу прерываем попытки, чтобы остальные ждущие остались на месте
					(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_MOVING);
					return; 
				}
			}
		// Дошли досюда. Значит никого двинуть не удалось. Значит безнадежно - все персонажи перестают ждать.
		follow_pers_init(qdGameObjectMoving::FOLLOW_DONE);
	}
}

void qdGameScene::follow_circuit(float dt)
{
	for (personages_container_t::iterator it1 = personages_.begin(); it1 != personages_.end(); ++it1)
	{				
		bool is_it1_follow = (selected_object_ == (*it1)) ||
			(*it1)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
			(*it1)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING);
		// не следующих и стоящих не обрабатываем как 
		if ((false == is_it1_follow) || (false == (*it1)->is_moving()) ||
			(qdGameObjectMoving::FOLLOW_MOVING != (*it1)->follow_condition()))
			continue; 

		Vect2s it1_cur_pos, it1_cur_grid, it1_next_pos, it1_next_grid;
		(*it1)->calc_cur_and_future_walk_grid(dt, it1_cur_pos, it1_cur_grid, it1_next_pos, it1_next_grid);

		for(personages_container_t::iterator it2 = personages_.begin(); it2 != personages_.end(); ++it2)
		{
			if ((*it1) == (*it2))
				continue;
			
			Vect2s it2_cur_pos, it2_cur_grid, it2_next_pos, it2_next_grid;
			(*it2)->calc_cur_and_future_walk_grid(dt, it2_cur_pos, it2_cur_grid, it2_next_pos, it2_next_grid);
			bool is_it2_follow = (selected_object_ == (*it2)) ||
				(*it2)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
				(*it2)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING);

			// Если на следующем шаге не пересекаются, то все в порядке
			if (!inters2s(it1_next_pos, it1_next_grid, it2_next_pos, it2_next_grid))
			{
				// Если it2 с которым НЕ пересекся текущий, которого обходим
				// то удаляем его из списка обходимых текущим
				for (std::vector<const qdGameObjectMoving*>::iterator cir_it = (*it1)->ref_circuit_objs().begin(); 
					 cir_it != (*it1)->ref_circuit_objs().end(); ++cir_it)
					if ((*cir_it) == (*it2))
					{
						(*it1)->ref_circuit_objs().erase(cir_it);
						break;
					}
				continue;
			}
			
			// Сначала пытаемся стопить второго для решения проблемы
			if (is_it2_follow && (*it2)->is_moving() &&
				(false == inters2s(it1_next_pos, it1_next_grid, it2_cur_pos, it2_cur_grid)))
			{
				(*it2)->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
				(*it2)->stop_movement();
				continue;
			}
			
			// Если it1 пытается обойти it2, то не учитываем их пересечение
			bool it2_is_circuit = false;
			for (std::vector<const qdGameObjectMoving*>::iterator cir_it = (*it1)->ref_circuit_objs().begin(); 
			     cir_it != (*it1)->ref_circuit_objs().end(); ++cir_it)
				if ((*cir_it) == (*it2))
				{
					it2_is_circuit = true;
					break;
				}
			if (it2_is_circuit) continue;

			// Пытаемся обойти второго, если он участвует в следовании или стоит.
			// Стопим второго при удача, а сами продолжаем обход.
			if ((*it1)->can_move() && (is_it2_follow || !(*it2)->is_moving()))
			{
				if (true == (*it1)->move((*it1)->last_move_order(), false))
				{
					(*it1)->ref_circuit_objs().push_back((*it2));
					(*it2)->set_follow_condition(qdGameObjectMoving::FOLLOW_WAIT);
					(*it2)->stop_movement();
					continue;
				}
			}
			
			// Ничего не помогло => стопим первого до поры, когда все движущиеся
			// остановяться
			(*it1)->set_follow_condition(qdGameObjectMoving::FOLLOW_FULL_STOP_WAIT);
			(*it1)->stop_movement();
		} // 2-for...
	} // 1-for...
}

void qdGameScene::follow_end_moving()
{
	if (NULL == selected_object_) return;
	for (personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
		if ((
			  (*it)->has_control_type(qdGameObjectMoving::CONTROL_FOLLOW_ACTIVE_PERSONAGE) ||
			  (*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_TO_ACTIVE_WITH_MOVING)
			) &&
			(qdGameObjectMoving::FOLLOW_MOVING == (*it)->follow_condition()) &&
			(selected_object_ != (*it)))
		{
			Vect3f dist = selected_object_->last_move_order() - (*it)->R();
			if (qdGameObjectMoving::FOLLOW_DONE == selected_object_->follow_condition())
				dist = selected_object_->R() - (*it)->R();

			if (dist.norm() <= (*it)->follow_min_radius())
			{
				(*it)->stop_movement();
				(*it)->set_follow_condition(qdGameObjectMoving::FOLLOW_DONE);
			}
		}
}

void qdGameScene::follow_quant(float dt)
{
	follow_implement_update_path();
	follow_wakening();
	follow_circuit(dt);
	follow_end_moving();
}

void qdGameScene::collision_quant()
{
#ifndef _QUEST_EDITOR
	Vect3f vec;
	for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
	{
		// Жесткая привязка с учетом направления привязывающего и без
		if (((*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL) ||
			 (*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITHOUT_DIR_REL))
			 &&
			(NULL != (*it)->attacher()))
		{
			Vect2s shift = (*it)->attach_shift();
			// Учитываем направление, если нужно
			if ((*it)->has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL))
			{
				Vect2s buf = shift;
				double ang = (*it)->attacher()->direction_angle();
				shift.x = round(buf.x*cos(ang) - buf.y*sin(ang));
				shift.y = round(buf.x*sin(ang) + buf.y*cos(ang));
			}

			Vect3f vec = (*it)->attacher()->R();
			vec.x += shift.x;
			vec.y += shift.y;

			(*it)->set_pos(vec);
		};
	}; // for(...)

	if((NULL == selected_object_) || !selected_object_->is_moving()) return;

	bool pflag = false;

	for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it){
		if(*it != selected_object_ && !(*it) -> has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITHOUT_DIR_REL) && !(*it) -> has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL) && (*it) -> can_move()){
			Vect3f dr = selected_object_ -> R() - (*it) -> R();

			float dist = selected_object_ -> collision_radius() + (*it) -> collision_radius();
			float angle = selected_object_ -> calc_direction_angle((*it) -> R());

			if(dr.norm() < dist){
				if(fabs(getDeltaAngle(angle,selected_object_ -> direction_angle())) < M_PI / 2.0f){
					if((*it) -> has_control_type(qdGameObjectMoving::CONTROL_COLLISION))
						(*it) -> set_movement_impulse(selected_object_ -> direction_angle());
				}
			}

			if(dr.norm() < dist){
				if(fabs(getDeltaAngle(angle,selected_object_ -> direction_angle())) < M_PI / 2.0f){
					if((*it) -> has_control_type(qdGameObjectMoving::CONTROL_AVOID_COLLISION))
						(*it) -> avoid_collision(selected_object_);
				}
			}

			if(!(*it) -> is_moving())
				pflag = true;
		}
	}

	if(pflag && selected_object_ -> has_control_type(qdGameObjectMoving::CONTROL_CLEAR_PATH)){
		selected_object_ -> set_path_attributes(sGridCell::CELL_PERSONAGE_PATH);
		for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it){
			if(!(*it) -> has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITHOUT_DIR_REL) && !(*it) -> has_control_type(qdGameObjectMoving::CONTROL_ATTACHMENT_WITH_DIR_REL)){
				if(*it != selected_object_ && (*it) -> can_move() && !(*it) -> is_moving() && (*it) -> check_grid_zone_attributes(sGridCell::CELL_PERSONAGE_PATH)){
					(*it) -> move_from_personage_path();
				}
			}
		}
		selected_object_ -> clear_path_attributes(sGridCell::CELL_PERSONAGE_PATH);
	}
	

	// Повторение движений активного персонажа
	for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it)
		if ((*it != selected_object_) && 
			(*it)->has_control_type(qdGameObjectMoving::CONTROL_REPEAT_ACTIVE_PERSONAGE_MOVEMENT) &&
			(*it)->can_move())
		{
			(*it) -> set_movement_impulse(selected_object_ -> direction_angle());
		};
#endif
}

void qdGameScene::add_redraw_region(const grScreenRegion& reg) const
{
	qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();

	dp -> add_redraw_region(reg);

	if(check_flag(CYCLE_X)){
		grScreenRegion reg1 = reg;

		reg1.move(-camera.get_scr_sx(),0);
		dp -> add_redraw_region(reg1);

		reg1.move(camera.get_scr_sx() * 2,0);
		dp -> add_redraw_region(reg1);
	}

	if(check_flag(CYCLE_Y)){
		grScreenRegion reg1 = reg;

		reg1.move(0,-camera.get_scr_sy());
		dp -> add_redraw_region(reg1);

		reg1.move(0,camera.get_scr_sy() * 2);
		dp -> add_redraw_region(reg1);
	}

	if(check_flag(CYCLE_X) && check_flag(CYCLE_Y)){
		grScreenRegion reg1 = reg;

		reg1.move(-camera.get_scr_sx(),-camera.get_scr_sy());
		dp -> add_redraw_region(reg1);

		reg1.move(camera.get_scr_sx() * 2,0);
		dp -> add_redraw_region(reg1);

		reg1.move(0,camera.get_scr_sy() * 2);
		dp -> add_redraw_region(reg1);

		reg1.move(-camera.get_scr_sx() * 2,0);
		dp -> add_redraw_region(reg1);
	}
}

bool qdGameScene::is_any_personage_in_zone(const qdGridZone* p) const
{
#ifndef _QUEST_EDITOR
	for(personages_container_t::const_iterator it = personages_.begin(); it != personages_.end(); ++it){
		if(p -> is_object_in_zone(*it))
			return true;
	}
#endif
	return false;
}

bool qdGameScene::set_personage_button(qdInterfaceButton* p)
{
	bool ret = false;
#ifndef _QUEST_EDITOR
	for(personages_container_t::const_iterator it = personages_.begin(); it != personages_.end(); ++it){
		if(p -> has_event(qdInterfaceEvent::EVENT_ACTIVATE_PERSONAGE,(*it) -> name())){
			(*it) -> set_button(p);
			ret = true;
		}
	}
#endif
	return ret;
}

bool qdGameScene::adjust_files_paths(const char* copy_dir, const char* pack_dir, bool can_overwrite)
{
	std::string copy_corr_dir = copy_dir;
	app_io::adjust_dir_end_slash(copy_corr_dir);
	std::string pack_corr_dir = pack_dir;
	app_io::adjust_dir_end_slash(pack_corr_dir);

	bool all_ok = true;
	for(qdMusicTrackList::const_iterator it = music_track_list().begin(); it != music_track_list().end(); ++it)
		QD_ADJUST_TO_REL_FILE_MEMBER(copy_corr_dir, (*it)->file_name, (*it)->set_file_name, can_overwrite, all_ok);
	
	for(qdSoundList::const_iterator it = sound_list().begin(); it != sound_list().end(); ++it)
		QD_ADJUST_TO_REL_FILE_MEMBER(pack_corr_dir, (*it)->file_name, (*it)->set_file_name, can_overwrite, all_ok);

	for(qdAnimationList::const_iterator it = animation_list().begin(); it != animation_list().end(); ++it)
		QD_ADJUST_TO_REL_FILE_MEMBER(pack_corr_dir, (*it)->qda_file, (*it)->qda_set_file, can_overwrite, all_ok);

	for(qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it){
		if((*it) -> named_object_type() == QD_NAMED_OBJECT_STATIC_OBJ)
		{
			qdGameObjectStatic* obj = static_cast<qdGameObjectStatic*>(*it);
			if(obj -> get_sprite() -> file())
				QD_ADJUST_TO_REL_FILE_MEMBER(pack_corr_dir, 
				  obj -> get_sprite() -> file, 
				  obj -> get_sprite() -> set_file, 
				  can_overwrite, 
				  all_ok);	
			std::string str = obj -> get_sprite() -> file();
			str = str;
		}
	}

	return all_ok;
}

bool qdGameScene::get_files_list(qdFileNameList& files_to_copy,qdFileNameList& files_to_pack) const
{
	for(qdMusicTrackList::const_iterator it = music_track_list().begin(); it != music_track_list().end(); ++it)
		files_to_copy.push_back((*it) -> file_name());

	for(qdSoundList::const_iterator it = sound_list().begin(); it != sound_list().end(); ++it)
		files_to_pack.push_back((*it) -> file_name());

	for(qdAnimationList::const_iterator it = animation_list().begin(); it != animation_list().end(); ++it)
		files_to_pack.push_back((*it) -> qda_file());

	for(qdGameObjectList::const_iterator it = object_list().begin(); it != object_list().end(); ++it){
		if((*it) -> named_object_type() == QD_NAMED_OBJECT_STATIC_OBJ){
			qdGameObjectStatic* obj = static_cast<qdGameObjectStatic*>(*it);
			if(obj -> get_sprite() -> file())
				files_to_pack.push_back(obj -> get_sprite() -> file());
		}
	}

	return true;
}


void qdGameScene::personages_quant()
{
#ifndef _QUEST_EDITOR
	for(personages_container_t::const_iterator it = personages_.begin(); it != personages_.end(); ++it){
		if((*it) -> button()){
			if((*it) -> is_visible()){
				if(!(*it) -> button() -> is_visible()){
					(*it) -> button() -> show();
					if(qdInterfaceScreen* sp = dynamic_cast<qdInterfaceScreen*>((*it) -> button() -> owner()))
						sp -> build_visible_elements_list();
				}

				if(get_active_personage() == *it)
					(*it) -> button() -> activate_state(1);
				else
					(*it) -> button() -> activate_state(0);
			}
			else
				(*it) -> button() -> hide();
		}
	}

	if(check_flag(CYCLE_X | CYCLE_Y)){
		for(personages_container_t::iterator it = personages_.begin(); it != personages_.end(); ++it){
			Vect3f r = (*it) -> R();
			r.z = 0.0f;

			Vect2s scr_r = camera.plane2rscr(r);
			Vect2s scr_r0 = scr_r;

			if(check_flag(CYCLE_X)){
				if(scr_r.x > camera.get_scr_sx()/2 + camera.get_scr_offset().x)
					scr_r.x -= camera.get_scr_sx();
				else if(scr_r.x < -camera.get_scr_sx()/2 + camera.get_scr_offset().x)
					scr_r.x += camera.get_scr_sx();
			}
			if(check_flag(CYCLE_Y)){
				if(scr_r.y > camera.get_scr_sy()/2 + camera.get_scr_offset().y)
					scr_r.y -= camera.get_scr_sy();
				else if(scr_r.y < -camera.get_scr_sy()/2 + camera.get_scr_offset().y)
					scr_r.y += camera.get_scr_sy();
			}

			if(!(scr_r0 == scr_r)){
				r = camera.rscr2plane(scr_r);
				(*it) -> set_pos(r);
				(*it) -> adjust_z();
			}
		}
	}
#endif

	for(personages_container_t::iterator itp = personages_.begin(); itp != personages_.end(); ++itp)
		(*itp) -> clear_shadow();

	for(qdGridZoneList::const_iterator it = grid_zone_list().begin(); it != grid_zone_list().end(); ++it){
		if((*it) -> has_shadow()){
			for(personages_container_t::iterator itp = personages_.begin(); itp != personages_.end(); ++itp){
				if((*it) -> is_object_in_zone(*itp))
					(*itp) -> set_shadow((*it) -> shadow_color(),(*it) -> shadow_alpha());
			}
		}
	}
}

bool qdGameScene::need_to_redraw_inventory(const char* inventory_name) const
{
#ifndef _QUEST_EDITOR
	for(personages_container_t::const_iterator it = personages_.begin(); it != personages_.end(); ++it){
		if(!strcmp((*it) -> inventory_name(),inventory_name))
			return true;
	}
#endif
	return false;
}
#ifdef _QUEST_EDITOR
void qdGameScene::reload_personage_list()
{
	personages_.clear();
	personages_.reserve(16);

	for(qdGameObjectList::const_iterator it = object_list().begin(); 
		it != object_list().end(); ++it){
			if((*it) -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ)
				personages_.push_back(static_cast<qdGameObjectMoving*>(*it));
		}
}
#endif // _QUEST_EDITOR

#ifndef _QUEST_EDITOR
void qdGameScene::create_minigame_objects()
{
	static XBuffer name_buf(1024);
	for(qdMiniGame::config_container_t::const_iterator it = minigame_ -> config().begin(); it != minigame_ -> config().end(); ++it){
		if(it -> data_type() == qdMinigameConfigParameter::PRM_DATA_OBJECT){
			if(const qdGameObject* obj = get_object(it -> data_string())){
				for(int i = 0; i < it -> data_count(); i++){
					name_buf.init();
					name_buf < obj -> name();
					int j = 1000;
					while(j > 1){
						if(i < j) name_buf < "0";
						j /= 10;
					}
					name_buf <= i;

					qdGameObject* new_obj = NULL;
					switch(obj -> named_object_type()){
					case QD_NAMED_OBJECT_ANIMATED_OBJ:
						new_obj = new qdGameObjectAnimated(*static_cast<const qdGameObjectAnimated*>(obj));
						break;
					case QD_NAMED_OBJECT_MOVING_OBJ:
						new_obj = new qdGameObjectMoving(*static_cast<const qdGameObjectMoving*>(obj));
						break;
					}

					if(new_obj){
						new_obj -> set_name(name_buf.c_str());
						add_object(new_obj);
					}
				}
			}
		}
	}
}
#endif // _QUEST_EDITOR

bool qdGameScene::set_camera_mode(const qdCameraMode& mode,qdGameObjectAnimated* object)
{
	if(!camera.can_change_mode())
		return false;

	if(object && object -> named_object_type() == QD_NAMED_OBJECT_MOVING_OBJ && object != selected_object_)
		return false;

	camera.set_mode(mode,object);
	return true;
}

#ifdef __QD_DEBUG_ENABLE__
bool qdGameScene::get_resources_info(qdResourceInfoContainer& infos) const
{
	qdGameDispatcherBase::get_resources_info(infos);

	for(qdGameObjectList::const_iterator io = object_list().begin(); io != object_list().end(); ++io){
		if((*io) -> named_object_type() == QD_NAMED_OBJECT_STATIC_OBJ){
			const qdSprite* sp = static_cast<const qdGameObjectStatic*>(*io) -> get_sprite();
			if(sp -> resource_data_size())
				infos.push_back(qdResourceInfo(sp,*io));
		}
	}

	return true;
}
#endif

#ifndef _QUEST_EDITOR
void qdGameScene::start_minigame()
{
	if(minigame_)
		minigame_ -> start();
}
#endif
