/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_core.h"
#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "qd_game_dispatcher.h"
#include "qd_interface_save.h"
#include "qd_interface_screen.h"
#include "qd_interface_dispatcher.h"

#include "gr_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

bool qdInterfaceSave::save_mode_ = false;
int qdInterfaceSave::current_save_ID_ = 0;

qdInterfaceSave::qdInterfaceSave() : save_ID_(0),
	thumbnail_size_x_(0),
	thumbnail_size_y_(0),
	text_dx_(0),
	text_dy_(0)
{
	thumbnail_.set_owner(this);
	frame_.set_owner(this);

	isAutosaveSlot_ = false;

	save_ID_ = current_save_ID_++;
}

qdInterfaceSave::qdInterfaceSave(const qdInterfaceSave& sv) : qdInterfaceElement(sv),
	save_ID_(sv.save_ID_),
	thumbnail_size_x_(sv.thumbnail_size_x_),
	thumbnail_size_y_(sv.thumbnail_size_y_),
	text_dx_(sv.text_dx()),
	text_dy_(sv.text_dy()),
	isAutosaveSlot_(sv.isAutosaveSlot_)
{
	thumbnail_.set_owner(this);
	thumbnail_ = sv.thumbnail_;

	frame_.set_owner(this);
	frame_ = sv.frame_;
}

qdInterfaceSave::~qdInterfaceSave()
{
	thumbnail_.unregister_resources();
	frame_.unregister_resources();
}

qdInterfaceSave& qdInterfaceSave::operator = (const qdInterfaceSave& sv)
{
	if(this == &sv) return *this;

	*static_cast<qdInterfaceElement*>(this) = sv;

	save_ID_ = sv.save_ID_;

	thumbnail_size_x_ = sv.thumbnail_size_x_;
	thumbnail_size_y_ = sv.thumbnail_size_y_;

	isAutosaveSlot_ = sv.isAutosaveSlot_;

	text_dx_ = sv.text_dx();
	text_dy_ = sv.text_dy();

	thumbnail_ = sv.thumbnail_;
	frame_ = sv.frame_;

	return *this;
}

bool qdInterfaceSave::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	frame_.mouse_handler(x,y,ev);

	switch(ev){
	case mouseDispatcher::EV_LEFT_DOWN:
	case mouseDispatcher::EV_RIGHT_DOWN:
		if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
			clear_screen_region();

			if(save_mode_){
				if(isAutosaveSlot())
					return true;

				qdInterfaceDispatcher* ip = qdInterfaceDispatcher::get_dispatcher();
				if(ip){
					if(ip->has_save_title_screen()){
						ip->setModalScreenMode(qdInterfaceDispatcher::MODAL_SCREEN_SAVE_NAME_EDIT);
						ip->handle_event(qdInterfaceEvent::EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL, ip->save_title_screen_name(), this);
					}
					else if(ip->has_save_prompt_screen() && app_io::is_file_exist(save_file())){
						ip->setModalScreenMode(qdInterfaceDispatcher::MODAL_SCREEN_SAVE_OVERWRITE);
						ip->handle_event(qdInterfaceEvent::EVENT_SHOW_INTERFACE_SCREEN_AS_MODAL, ip->save_prompt_screen_name(), this);
					}
					else
					{
						perform_save();
						ip->handle_event(qdInterfaceEvent::EVENT_RESUME_GAME,NULL);
					}
				}

				return true;
			}
			else {
				dp -> load_game(save_ID_);
				if(qdInterfaceDispatcher* ip = qdInterfaceDispatcher::get_dispatcher())
					ip -> handle_event(qdInterfaceEvent::EVENT_RESUME_GAME,NULL);

				return true;
			}
		}
		break;
	}

	return false;
}

bool qdInterfaceSave::keyboard_handler(int vkey)
{
	return false;
}

bool qdInterfaceSave::init(bool is_game_active)
{
#ifndef _QUEST_EDITOR
	if(!is_game_active && frame_.need_active_game())
		set_lock(true);
	else
		set_lock(false);

	if(app_io::is_file_exist(thumbnail_file()))
		thumbnail_.set_animation_file(thumbnail_file());

	if(app_io::is_file_exist(description_file())){
		XStream fh(description_file(), XS_IN);
		XBuffer buf(fh.size() + 1);
		buf.fill(0);
		fh.read(buf.buffer(), fh.size());
		fh.close();

		save_title_ = buf;
	}
	else
		save_title_ = "";
	
	set_state(&frame_);

	if(!save_mode_ && !app_io::is_file_exist(save_file())){
		if(is_visible()){
			hide();
			
			if(qdInterfaceScreen* sp = dynamic_cast<qdInterfaceScreen*>(owner()))
				sp -> build_visible_elements_list();
		}
	}
	else {
		if(!is_visible()){
			show();
			
			if(qdInterfaceScreen* sp = dynamic_cast<qdInterfaceScreen*>(owner()))
				sp -> build_visible_elements_list();
		}
	}
#else
	set_state(&frame_);
#endif // _QUEST_EDITOR

	return true;
}

bool qdInterfaceSave::redraw() const
{
	if (qdInterfaceDispatcher* pid = qdInterfaceDispatcher::get_dispatcher())
	{
		if(pid->need_save_screenshot())
			if(const qdAnimation* p = thumbnail_.animation())
				p -> redraw(r().x,r().y,0);

		std::string text;

		if(pid->need_show_save_title()){
			text = title();
			if(pid->need_show_save_time())
				text += " ";
		}

		grDispatcher* gr_disp = grDispatcher::instance();
		if(pid->need_show_save_time() && gr_disp){
#ifndef _QUEST_EDITOR
			FILETIME file_time;
			SYSTEMTIME global_time, time;
			std::string str = save_file();
			HANDLE hFile = CreateFile(str.c_str(),
				GENERIC_READ,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if (INVALID_HANDLE_VALUE != hFile &&
				GetFileTime(hFile, NULL, NULL, &file_time) &&
				FileTimeToSystemTime(&file_time, &global_time) &&
				SystemTimeToTzSpecificLocalTime(NULL, &global_time, &time))
			{
				XBuffer buf;
				if (time.wDay < 10) buf < "0";
				buf <= time.wDay < ".";
				if (time.wMonth < 10) buf < "0";
				buf <= time.wMonth < "." <= time.wYear < " ";
				if (time.wHour < 10) buf < "0";
				buf <= time.wHour < ":";
				if (time.wMinute < 10) buf < "0";
				buf <= time.wMinute;

				text += buf;
			}
			CloseHandle(hFile);
		}

		if(!text.empty()){
			qdGameDispatcher* game_disp = qdGameDispatcher::get_dispatcher();
			const grFont* font = NULL;
			if ((QD_FONT_TYPE_NONE != pid->save_font_type()) && game_disp)
				font = game_disp->find_font(pid->save_font_type());
			else
				font = gr_disp->get_default_font();

			int tx = r().x - size_x()/2 + text_dx();
			int ty = r().y - size_y()/2 + text_dy();

			gr_disp->DrawText(tx, ty, pid->save_font_color(), text.c_str(), 0, 0, font);
		}
#else

			if(qdInterfaceDispatcher* id = qdInterfaceDispatcher::get_dispatcher()){
				qdGameDispatcher* game_disp = qdGameDispatcher::get_dispatcher();
				const grFont* font = NULL;
				if ((QD_FONT_TYPE_NONE != pid->save_font_type()) && game_disp)
					font = game_disp->find_font(pid->save_font_type());
				else
					font = gr_disp->get_default_font();

				int tx = r().x - size_x()/2 + text_dx();
				int ty = r().y - size_y()/2 + text_dy();

				gr_disp->DrawText(tx, ty, id->save_font_color(), "30.01.2005 12:59", 0, 0, font);
			}
		}
#endif
	}

	return qdInterfaceElement::redraw();
}

grScreenRegion qdInterfaceSave::screen_region() const
{
	grScreenRegion reg0 = qdInterfaceElement::screen_region();
	grScreenRegion reg1(r().x,r().y,thumbnail_size_x_,thumbnail_size_y_);

	reg0 += reg1;

	return reg0;
}

int qdInterfaceSave::size_x() const
{
	int x = thumbnail_size_x_;
	if(const qdAnimation* p = frame_.animation()){
		if(x < p -> size_x())
			x = p -> size_x();
	}

	return x;
}

int qdInterfaceSave::size_y() const
{
	int y = thumbnail_size_y_;
	if(const qdAnimation* p = frame_.animation()){
		if(y < p -> size_y())
			y = p -> size_y();
	}

	return y;
}

bool qdInterfaceSave::quant(float dt)
{
	qdInterfaceElement::quant(dt);

	frame_.quant(dt);

	return true;
}

bool qdInterfaceSave::hit_test(int x,int y) const
{
	if(qdInterfaceElement::hit_test(x,y)) return true;

	x -= r().x;
	y -= r().y;

	bool result = false;

	if(x >= -thumbnail_size_x_/2 && x < thumbnail_size_x_/2 && y >= -thumbnail_size_y_/2 && y < thumbnail_size_y_/2)
		result = true;

	return result;
}

bool qdInterfaceSave::perform_save()
{
	bool is_ok = true;
	if (qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher())
	{
		is_ok &= dp -> save_game(save_ID_);

		if(!save_title_.empty()){
			XStream fh(description_file(), XS_OUT);
			fh.write(save_title_.c_str(), strlen(save_title_.c_str()));
			fh.close();
		}

		is_ok &= dp -> game_screenshot(qdGameDispatcher::get_save_name(save_ID_, qdGameDispatcher::SAVE_THUMBNAIL),thumbnail_size_x_,thumbnail_size_y_);
		is_ok &= init(true);
		return is_ok;
	}
	return false;
}

bool qdInterfaceSave::save_script_body(XStream& fh,int indent) const
{
	if(!frame_.save_script(fh,indent)) return false;

	for(int i = 0; i <= indent; i++) fh < "\t";
	fh < "<ID>" <= save_ID_ < "</ID>\r\n";

	if(thumbnail_size_x_ || thumbnail_size_y_){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<thumbnail_size>" <= thumbnail_size_x_ < " " <= thumbnail_size_y_ < "</thumbnail_size>\r\n";
	}
	if(text_dx_ || text_dy_){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<text_shift>" <= text_dx_ < " " <= text_dy_ < "</text_shift>\r\n";
	}
	if(isAutosaveSlot()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<is_autosave>1</is_autosave>\r\n";
	}

	return true;
}

bool qdInterfaceSave::load_script_body(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_INTERFACE_ELEMENT_STATE:
			if(!frame_.load_script(&*it)) return false;
			break;
		case QDSCR_INTERFACE_THUMBNAIL_SIZE:
			xml::tag_buffer(*it) > thumbnail_size_x_ > thumbnail_size_y_;
			break;
		case QDSCR_INTERFACE_TEXT_SHIFT:
			xml::tag_buffer(*it) > text_dx_ > text_dy_;
			break;
		case QDSCR_ID:
			xml::tag_buffer(*it) > save_ID_;
			break;
		case QDSCR_INTERFACE_SAVE_IS_AUTOSAVE:
			setAutosaveSlot(xml::tag_buffer(*it).get_int() != 0);
			break;
		}
	}

	return true;
}

const char* qdInterfaceSave::save_file() const
{
	return qdGameDispatcher::get_save_name(save_ID_);
}

const char* qdInterfaceSave::thumbnail_file() const
{
	return qdGameDispatcher::get_save_name(save_ID_, qdGameDispatcher::SAVE_THUMBNAIL);
}

const char* qdInterfaceSave::description_file() const
{
	return qdGameDispatcher::get_save_name(save_ID_, qdGameDispatcher::SAVE_DESCRIPTION);
}

