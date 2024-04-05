/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "gr_dispatcher.h"

#include "qd_game_dispatcher.h"

#include "qd_setup.h"
#include "qd_interface_screen.h"
#include "qd_interface_text_window.h"
#include "qd_interface_slider.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

const float qdInterfaceCaretPeriod = 0.3f;

bool qdInterfaceTextWindow::caretVisible_ = false;
float qdInterfaceTextWindow::caretTimer_ = 0.f;

qdInterfaceTextWindow::qdInterfaceTextWindow() : text_size_(0,0),
	scrolling_speed_(0),
	text_set_(NULL),
	text_set_id_(qdGameDispatcher::TEXT_SET_DIALOGS),
	slider_(NULL),
	windowType_(WINDOW_DIALOGS),
	infoType_(INFO_NONE)
{
	textFormat_.toggle_global_depend(false);

	inputStringLimit_ = 0;
	scrolling_position_ = 0;
	text_set_position_ = 0;

	textVAlign_ = VALIGN_BOTTOM;

	background_color_ = 0;
	has_background_color_ = false;
	background_alpha_ = 0;

	isEditing_ = false;
	caretPose_ = -1;

	playerID_ = 0;

	border_background_.set_owner(this);
}

qdInterfaceTextWindow::qdInterfaceTextWindow(const qdInterfaceTextWindow& wnd) : qdInterfaceElement(wnd),
	text_size_(wnd.text_size_),
	scrolling_speed_(wnd.scrolling_speed_),
	text_set_(wnd.text_set_),
	text_set_id_(wnd.text_set_id_),
	slider_name_(wnd.slider_name_),
	slider_(wnd.slider_),
	windowType_(wnd.windowType_),
	infoType_(wnd.infoType_),
	playerID_(wnd.playerID_),
	inputString_(wnd.inputString_),
	inputStringBackup_(wnd.inputStringBackup_),
	inputStringLimit_(wnd.inputStringLimit_),
	textFormat_(wnd.textFormat_),
	textVAlign_(wnd.textVAlign_)
{
	scrolling_position_ = 0;
	text_set_position_ = 0;

	background_color_ = wnd.background_color_;
	has_background_color_ = wnd.has_background_color_;
	background_alpha_ = wnd.background_alpha_;

	isEditing_ = wnd.isEditing_;
	caretPose_ = wnd.caretPose_;

	border_background_.set_owner(this);
	border_background_ = wnd.border_background_;
}

qdInterfaceTextWindow& qdInterfaceTextWindow::operator = (const qdInterfaceTextWindow& wnd)
{
	if(this == &wnd) return *this;

	*static_cast<qdInterfaceElement*>(this) = wnd;

	text_size_ = wnd.text_size_;
	scrolling_speed_ = wnd.scrolling_speed_;
	text_set_ = wnd.text_set_;
	text_set_id_ = wnd.text_set_id_;
	slider_name_ = wnd.slider_name_;
	slider_ = wnd.slider_;

	windowType_ = wnd.windowType_;
	infoType_ = wnd.infoType_;
	playerID_ = wnd.playerID_;

	inputString_ = wnd.inputString_;
	inputStringBackup_ = wnd.inputStringBackup_;
	inputStringLimit_ = wnd.inputStringLimit_;
	textFormat_ = wnd.textFormat_;

	textVAlign_ = wnd.textVAlign_;

	isEditing_ = wnd.isEditing_;
	caretPose_ = wnd.caretPose_;

	border_background_ = wnd.border_background_;
	background_color_ = wnd.background_color_;

	return *this;
}

qdInterfaceTextWindow::~qdInterfaceTextWindow()
{
	border_background_.unregister_resources();
}

bool qdInterfaceTextWindow::mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev)
{
	if(!text_set_) return false;

	if(windowType_ == WINDOW_DIALOGS){
		if(ev == mouseDispatcher::EV_LEFT_DOWN){
			if(qdScreenText* p = text_set_ -> get_text(x,y)){
				if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
					dp -> set_flag(qdGameDispatcher::DIALOG_CLICK_FLAG);
					dp -> set_mouse_click_state(p -> owner());
				}
				return true;
			}
		}
		else if(ev == mouseDispatcher::EV_MOUSE_MOVE){
			text_set_ -> clear_hover_mode();
			if(qdScreenText* p = text_set_ -> get_text(x,y))
				p -> set_hover_mode(true);
		}
	}

	return false;
}

bool qdInterfaceTextWindow::keyboard_handler(int vkey)
{
	if(windowType_ == WINDOW_EDIT && isEditing_)
		return edit_input(vkey);

	return false;
}

bool qdInterfaceTextWindow::char_input_handler(int input)
{
	bool ret = __super::char_input_handler(input);

	if(windowType_ == WINDOW_EDIT && isEditing_){
		if(!inputStringLimit_ || inputString_.length() < inputStringLimit_){
			if(isprint(input) || input == '_' || input == '-' || input == ' '){
				inputString_.insert(caretPose_++, 1, input);
				return true;
			}
		}
	}

	return ret;
}

void qdInterfaceTextWindow::hover_clear()
{
	if(windowType_ == WINDOW_DIALOGS){
		if(text_set_)
			text_set_ -> clear_hover_mode();
	}
}

bool qdInterfaceTextWindow::init(bool is_game_active)
{
	set_state(&border_background_);

	if(windowType_ == WINDOW_DIALOGS){
		if(!text_set_)
			text_set_ = qdGameDispatcher::get_dispatcher() -> screen_texts_dispatcher().get_text_set(text_set_id_);

		if(!slider_){
			if(qdInterfaceScreen* p = static_cast<qdInterfaceScreen*>(owner()))
				slider_ = dynamic_cast<qdInterfaceSlider*>(p -> get_element(slider_name()));
		}

		if(text_set_){
			text_set_ -> set_max_text_width(text_size_.x);
			update_text_position();
		}
	}
	else {
#ifndef _QUEST_EDITOR
		qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher();

		switch(infoType_){
		case INFO_PLAYER_NAME:
			inputString_ = dp->hall_of_fame_player_name(playerID_);
			if(dp->is_hall_of_fame_updated(playerID_) && !isEditing_)
				edit_start();
			break;
		case INFO_PLAYER_SCORE:
			if(dp->hall_of_fame_player_score(playerID_)){
				XBuffer buf;
				buf <= dp->hall_of_fame_player_score(playerID_);
				inputString_ = buf;
			}
			else
				inputString_ = "";
			break;
		}
#endif
		if(windowType_ == WINDOW_TEXT)
			set_input_string(input_string());
	}
	return true;
}

bool qdInterfaceTextWindow::save_script_body(XStream& fh,int indent) const
{
	if(border_background_.has_animation()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<border_back>" < qdscr_XML_string(border_background_.animation_file()) < "</border_back>\r\n";
	}

	if(!slider_name_.empty()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<window_slider>" < qdscr_XML_string(slider_name_.c_str()) < "</window_slider>\r\n";
	}

	if(text_size_.x || text_size_.y){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<screen_size>" <= text_size_.x < " " <= text_size_.y < "</screen_size>\r\n";
	}

	if(background_color_){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<color>" <= background_color_ < "</color>\r\n";
	}

	if(has_background_color_){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<enable_background>1</enable_background>\r\n";
	}

	if(background_alpha_){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<alpha>" <= background_alpha_ < "</alpha>\r\n";
	}

	if(windowType_ != WINDOW_DIALOGS){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<window_type>" <= int(windowType_) < "</window_type>\r\n";
	}

	if(infoType_ != INFO_NONE){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<info_type>" <= int(infoType_) < "</info_type>\r\n";
	}

	if(playerID_){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<player_id>" <= playerID_ < "</player_id>\r\n";
	}

	if(!inputString_.empty()){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<text>" < qdscr_XML_string(inputString_.c_str()) < "</text>\r\n";
	}

	if(inputStringLimit_){
		for(int i = 0; i <= indent; i++) fh < "\t";
		fh < "<string_length>" <= inputStringLimit_ < "</string_length>\r\n";
	}

	for(int i = 0; i <= indent; i++) fh < "\t";
	fh < "<valign>" <= textVAlign_ < "</valign>\r\n";

	if(textFormat_ != qdScreenTextFormat::default_format())
		textFormat_.save_script(fh, indent + 1);

	return true;
}

bool qdInterfaceTextWindow::load_script_body(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
		case QDSCR_TEXT_WINDOW_BORDER_BACK:
			set_border_background_file(it -> data());
			break;
		case QDSCR_TEXT_WINDOW_SLIDER:
			set_slider_name(it -> data());
			break;
		case QDSCR_SCREEN_SIZE:
			xml::tag_buffer(*it) > text_size_.x > text_size_.y;
			break;
		case QDSCR_COLOR:
			xml::tag_buffer(*it) > background_color_;
			break;
		case QDSCR_ALPHA:
			xml::tag_buffer(*it) > background_alpha_;
			break;
		case QDSCR_ENABLE_BACKGROUND:
			has_background_color_ = xml::tag_buffer(*it).get_int() != 0;
			break;
		case QDSCR_TEXT_WINDOW_TYPE:
			windowType_ = WindowType(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_WINDOW_INFO_TYPE:
			infoType_ = InfoType(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_WINDOW_PLAYER_ID:
			xml::tag_buffer(*it) > playerID_;
			break;
		case QDSCR_SCREEN_TEXT_FORMAT:
			textFormat_.load_script(&*it);
			textFormat_.toggle_global_depend(false);
			break;
		case QDSCR_TEXT_WINDOW_MAX_STRING_LENGTH:
			xml::tag_buffer(*it) > inputStringLimit_;
			break;
		case QDSCR_TEXT:
			inputString_ = it -> data();
			break;
		case QDSCR_VALIGN:
			textVAlign_ = TextVAlign(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdInterfaceTextWindow::redraw() const
{
	qdInterfaceElement::redraw();

	if(windowType_ == WINDOW_DIALOGS){
		if(text_set_){
			int l_clip,t_clip,r_clip,b_clip;
			grDispatcher::instance() -> GetClip(l_clip,t_clip,r_clip,b_clip);

			Vect2i ar = r();
			grDispatcher::instance() -> LimitClip(ar.x - text_size_.x/2,ar.y - text_size_.y/2,ar.x + text_size_.x/2,ar.y + text_size_.y/2);

			if(has_background_color_){
				Vect2i text_r = text_set_->screen_pos();
				Vect2i text_sz = text_set_->screen_size();

				if(!background_alpha_)
					grDispatcher::instance()->Rectangle(ar.x - text_size_.x/2,text_r.y - text_sz.y/2,text_size_.x,text_sz.y,background_color_,background_color_,GR_FILLED);
				else
					grDispatcher::instance()->RectangleAlpha(ar.x - text_size_.x/2,text_r.y - text_sz.y/2,text_size_.x,text_sz.y,background_color_,background_alpha_);
			}

			text_set_ -> redraw();

			grDispatcher::instance() -> SetClip(l_clip,t_clip,r_clip,b_clip);

			if(qdGameConfig::get_config().debug_draw())
				grDispatcher::instance() -> Rectangle(ar.x - text_size_.x/2,ar.y - text_size_.y/2,text_size_.x,text_size_.y,0xFFFFFF,0,GR_OUTLINED,3);
		}
	}
	else if(windowType_ == WINDOW_EDIT || windowType_ == WINDOW_TEXT){
		if(has_background_color_){
			Vect2i ar = r();
			if(!background_alpha_)
				grDispatcher::instance()->Rectangle(ar.x - text_size_.x/2,ar.y - text_size_.y/2,text_size_.x,text_size_.y,background_color_,background_color_,GR_FILLED);
			else
				grDispatcher::instance()->RectangleAlpha(ar.x - text_size_.x/2,ar.y - text_size_.y/2,text_size_.x,text_size_.y,background_color_,background_alpha_);
		}

		text_redraw();
	}

#ifdef _QUEST_EDITOR
	Vect2i leftTop = r();
	Vect2i rightBottom = r();
	Vect2i const half(text_size().x>>1, text_size().y>>1);

	leftTop -= half;
	rightBottom += half;
	grDispatcher * const gd = grDispatcher::instance();
	gd->Line(leftTop.x, leftTop.y, leftTop.x, rightBottom.y, 0, 4,true);
	gd->Line(leftTop.x, leftTop.y,rightBottom.x, leftTop.y,  0, 4,true);
	gd->Line(rightBottom.x,rightBottom.y,leftTop.x,rightBottom.y, 0, 4,true);
	gd->Line(rightBottom.x, rightBottom.y, rightBottom.x,leftTop.y,  0, 4,true);
#endif // _QUEST_EDITOR
	return true;
}

bool qdInterfaceTextWindow::need_redraw() const
{
	if(qdInterfaceElement::need_redraw())
		return true;

	if(windowType_ == WINDOW_DIALOGS){
		if(text_set_ && text_set_ -> need_redraw())
			return true;

		if(slider_ && slider_ -> need_redraw())
			return true;
	}
	else if(windowType_ == WINDOW_EDIT || windowType_ == WINDOW_TEXT)
		return true;

	return false;
}

bool qdInterfaceTextWindow::quant(float dt)
{
	qdInterfaceElement::quant(dt);

	if(isEditing_){
		caretTimer_ -= dt;
		if(caretTimer_ < 0.f){
			caretVisible_ = !caretVisible_;
			caretTimer_ = qdInterfaceCaretPeriod;
		}
	}

	if(windowType_ == WINDOW_DIALOGS){
		if(is_visible() && text_set_ && text_set_ -> was_changed())
			update_text_position();

#ifndef _QUEST_EDITOR
		if(text_set_){
			if(fabs(scrolling_position_) > FLT_EPS){
				float delta = scrolling_speed_ * dt;
				if(fabs(scrolling_position_) > delta)
					scrolling_position_ += (scrolling_position_ > 0) ? -delta : delta;
				else 
					scrolling_position_ = 0;
		
				Vect2i pos = text_set_ -> screen_pos();
				pos.y = text_set_position_ + round(scrolling_position_);
				text_set_ -> set_screen_pos(pos);
			}

			if(text_set_ -> is_empty()){
				if(is_visible()){
					hide();
					if(qdInterfaceScreen* sp = dynamic_cast<qdInterfaceScreen*>(owner()))
						sp -> build_visible_elements_list();

					qdGameDispatcher::get_dispatcher() -> toggle_full_redraw();
				}
			}
			else {
				if(!is_visible()){
					show();
					if(qdInterfaceScreen* sp = dynamic_cast<qdInterfaceScreen*>(owner()))
						sp -> build_visible_elements_list();
				}
			}

			if(slider_){
				if(!is_visible() || text_size_.y > text_set_ -> screen_size().y){
					if(slider_ -> is_visible()){
						slider_ -> hide();

						if(qdInterfaceScreen* sp = dynamic_cast<qdInterfaceScreen*>(owner()))
							sp -> build_visible_elements_list();

						qdGameDispatcher::get_dispatcher() -> toggle_full_redraw();
					}
				}
				else {
					if(!slider_ -> is_visible()){
						slider_ -> show();

						if(qdInterfaceScreen* sp = dynamic_cast<qdInterfaceScreen*>(owner()))
							sp -> build_visible_elements_list();
					}
				}
			}
		}
#endif // _QUEST_EDITOR
	}

	return true;
}

void qdInterfaceTextWindow::update_text_position()
{
	if(text_set_){
		Vect2f rr = r() - text_size()/2 + text_set_->screen_size()/2;
		switch(textVAlign_){
			case VALIGN_CENTER:
				rr.y += (text_size().y - text_set_->screen_size().y)/2;
				break;
			case VALIGN_BOTTOM:
				rr.y += text_size().y - text_set_->screen_size().y;
				break;
		}
		text_set_->set_screen_pos(rr);
		text_set_position_ = rr.y;

		set_scrolling(text_set_->new_texts_height());
		text_set_->clear_new_texts_height();

		text_set_->toggle_changed(false);
	}
}

bool qdInterfaceTextWindow::hit_test(int x,int y) const
{
	x -= r().x;
	y -= r().y;

	if(x >= -size_x()/2 && x < size_x()/2 && y >= -size_y()/2 && y < size_y()/2)
		return true;

	return false;
}

grScreenRegion qdInterfaceTextWindow::screen_region() const
{
	return grScreenRegion(r().x,r().y,size_x(),size_y());
}

int qdInterfaceTextWindow::size_x() const
{
	return (qdInterfaceElement::size_x() > text_size_.x) ? qdInterfaceElement::size_x() : text_size_.x;
}

int qdInterfaceTextWindow::size_y() const
{
	return (qdInterfaceElement::size_y() > text_size_.y) ? qdInterfaceElement::size_y() : text_size_.y;
}

void qdInterfaceTextWindow::set_scrolling(int y_delta)
{
	if(scrolling_speed_ > FLT_EPS)
		scrolling_position_ = y_delta;
	else
		scrolling_position_ = 0;
}

void qdInterfaceTextWindow::text_redraw() const
{
	Vect2i ar = r() - text_size_/2;

	unsigned col = textFormat_.color();

	const grFont* font = qdGameDispatcher::get_dispatcher()->
		                   find_font(textFormat_.font_type());

	if(windowType_ == WINDOW_EDIT && isEditing_){
		int sz0 = grDispatcher::instance()->TextWidth("|", 0, font);
		int sz1 = grDispatcher::instance()->TextWidth(inputString_.c_str(), 0, font);
		int x0 = ar.x;
		switch(textFormat_.alignment()){
		case qdScreenTextFormat::ALIGN_LEFT:
			break;
		case qdScreenTextFormat::ALIGN_CENTER:
			x0 += (text_size_.x - sz0 - sz1)/2;
			break;
		case qdScreenTextFormat::ALIGN_RIGHT:
			x0 += text_size_.x - sz0 - sz1;
			break;
		}

		std::string str = inputString_.substr(0, caretPose_);
		if(!str.empty()){
			grDispatcher::instance() -> DrawAlignedText(x0,ar.y,text_size_.x,text_size_.y,col,str.c_str(),GR_ALIGN_LEFT,0,0,font);
			x0 += grDispatcher::instance()->TextWidth(str.c_str(), 0, font);
		}
		if(caretVisible_)
			grDispatcher::instance() -> DrawAlignedText(x0,ar.y,text_size_.x,text_size_.y,col,"|",GR_ALIGN_LEFT,0,0,font);
		x0 += grDispatcher::instance()->TextWidth("|", 0, font);

		str = inputString_.substr(caretPose_, std::string::npos);
		if(!str.empty())
			grDispatcher::instance() -> DrawAlignedText(x0,ar.y,text_size_.x,text_size_.y,col,str.c_str(),GR_ALIGN_LEFT,0,0,font);
	}
	else {
		if(windowType_ == WINDOW_TEXT)
			grDispatcher::instance() -> DrawParsedText(ar.x,ar.y,text_size_.x,text_size_.y,col,&parser_,grTextAlign(textFormat_.alignment()),font);
		else
			grDispatcher::instance() -> DrawAlignedText(ar.x,ar.y,text_size_.x,text_size_.y,col,inputString_.c_str(),grTextAlign(textFormat_.alignment()),0,0,font);
	}
}

bool qdInterfaceTextWindow::edit_input(int vkey)
{
	if(isEditing_){
		switch(vkey){
		case VK_ESCAPE:
			if(!edit_done(true))
				return false;
			return true;
		
		case VK_RETURN:
			if(!edit_done(false))
				return false;
			return true;

		case VK_LEFT:
			if(caretPose_ > 0)
				--caretPose_;
			return true;

		case VK_HOME:
			caretPose_ = 0;
			return true;

		case VK_END:
			caretPose_ = inputString_.size();
			return true;

		case VK_RIGHT:
			if(caretPose_ < inputString_.size())
				++caretPose_;
			return true;

		case VK_BACK:
			if(caretPose_ > 0 && caretPose_ <= inputString_.size())
				inputString_.erase(--caretPose_, 1);
			return true;

		case VK_DELETE:
			if(caretPose_ >= 0 && caretPose_ < inputString_.size())
				inputString_.erase(caretPose_, 1);

			return true;

		}

		return true;
	}

	return false;
}

void qdInterfaceTextWindow::set_input_string(const char* str)
{
	inputString_ = str;

	if(windowType_ == WINDOW_TEXT){
		parser_.setFont(qdGameDispatcher::get_dispatcher()->find_font(textFormat_.font_type()));
		parser_.parseString(inputString_.c_str(), textFormat_.color());
	}
}

bool qdInterfaceTextWindow::edit_start()
{
	if(!isEditing_){
		isEditing_ = true;
		inputStringBackup_ = inputString_;
		caretPose_ = inputString_.size();
		return true;
	}

	return false;
}

bool qdInterfaceTextWindow::edit_done(bool cancel)
{
	if(isEditing_){
		bool end_edit = true;

		switch(infoType_){
		case INFO_NONE:
			if(qdInterfaceDispatcher* dp = qdInterfaceDispatcher::get_dispatcher())
				dp->handle_event(!cancel ? qdInterfaceEvent::EVENT_MODAL_OK : qdInterfaceEvent::EVENT_MODAL_CANCEL, 0, this);
			break;
		case INFO_PLAYER_NAME:
			if(cancel || inputString_.empty()){
				end_edit = false;
			}
			else {
				if(qdGameDispatcher* dp = qdGameDispatcher::get_dispatcher()){
					dp->set_hall_of_fame_player_name(playerID_, inputString_.c_str());
					dp->save_hall_of_fame();
				}
			}
			break;
		}

		if(end_edit){
			isEditing_ = false;

			if(cancel)
				inputString_ = inputStringBackup_;
		}

		return true;
	}

	return false;
}
