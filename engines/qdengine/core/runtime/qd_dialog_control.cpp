/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <commctrl.h>

#include <algorithm>

#include "qd_setup.h"
#include "qd_dialog_control.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

bool qdlgOption::load_value() const
{
	char* str = getIniKey(ini_file_.c_str(),ini_section_.c_str(),ini_key_.c_str());
	if(strlen(str)){
		set_value(atoi(str));
		return true;
	}

	return false;
}

bool qdlgOption::save_value() const
{
	putIniKey(ini_file_.c_str(),ini_section_.c_str(),ini_key_.c_str(),value());
	return true;
}

bool qdlgOption::enable_control(bool status) const
{
	if(HWND hwnd = GetDlgItem(control_parent(),control_ID())){
		EnableWindow(hwnd,status);
		return true;
	}

	return false;
}

int qdlgOptionCheckbox::value() const
{
	if(!has_control()) return 0;
	int check_state = SendDlgItemMessage(control_parent(),control_ID(),BM_GETCHECK,(WPARAM)0,(LPARAM)0);

	if(check_state & BST_CHECKED) return 1;

	return 0;
}

void qdlgOptionCheckbox::set_value(int val) const
{
	if(!has_control()) return;
	
	int check_state = (val) ? BST_CHECKED : 0;
	SendDlgItemMessage(control_parent(),control_ID(),BM_SETCHECK,(WPARAM)check_state,(LPARAM)0);
}

int qdlgOptionSlider::value() const
{
	if(!has_control()) return 0;
	int val = SendDlgItemMessage(control_parent(),control_ID(),TBM_GETPOS,(WPARAM)0,(LPARAM)0);

	if(invert_value_){
		int val_min = SendDlgItemMessage(control_parent(),control_ID(),TBM_GETRANGEMIN,(WPARAM)0,(LPARAM)0);
		int val_max = SendDlgItemMessage(control_parent(),control_ID(),TBM_GETRANGEMAX,(WPARAM)0,(LPARAM)0);

		val = val_max - (val - val_min);
	}

	return val;
}

void qdlgOptionSlider::set_value(int val) const
{
	if(!has_control()) return;

	if(invert_value_){
		int val_min = SendDlgItemMessage(control_parent(),control_ID(),TBM_GETRANGEMIN,(WPARAM)0,(LPARAM)0);
		int val_max = SendDlgItemMessage(control_parent(),control_ID(),TBM_GETRANGEMAX,(WPARAM)0,(LPARAM)0);

		val = val_max - (val - val_min);
	}

	SendDlgItemMessage(control_parent(),control_ID(),TBM_SETPOS,(WPARAM)TRUE,(LPARAM)val);
}

void qdlgOptionSlider::set_value_range(int min,int max,bool invert_value)
{
	invert_value_ = invert_value;
	SendDlgItemMessage(control_parent(),control_ID(),TBM_SETRANGE,(WPARAM)TRUE,(LPARAM)MAKELONG(min,max));
}

void qdlgOptionDroplist::init_control() const
{
	int val = value();

	SendDlgItemMessage(control_parent(),control_ID(),CB_RESETCONTENT,(WPARAM)0,(LPARAM)0);

	for(int i = 0; i < items_.size(); i++){
		if(items_[i].is_enabled())
			SendDlgItemMessage(control_parent(),control_ID(),CB_ADDSTRING,(WPARAM)0,(LPARAM)items_[i].title());
	}

	set_value(val);
}

int qdlgOptionDroplist::value() const
{
	if(!has_control()) return default_value();

	int val = SendDlgItemMessage(control_parent(),control_ID(),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
	if(val == CB_ERR) return default_value();

	return items_[val].value();
}

void qdlgOptionDroplist::set_value(int val) const
{
	if(!has_control()) return;

	int idx = 0;
	for(item_container_t::const_iterator it = items_.begin(); it != items_.end(); ++it){
		if(it -> value() == val){
			SendDlgItemMessage(control_parent(),control_ID(),CB_SETCURSEL,(WPARAM)idx,(LPARAM)0);
			return;
		}

		if(it -> is_enabled())
			idx++;
	}
}

void qdlgOptionDroplist::add_item(const char* title,int value)
{
	items_.push_back(qdlgDroplistItem(title,value));
}

bool qdlgOptionDroplist::enable_item(int value)
{
	item_container_t::iterator it = std::find(items_.begin(),items_.end(),value);
	if(it != items_.end()){
		it -> enable();
		return true;
	}

	return false;
}

bool qdlgOptionDroplist::disable_item(int value)
{
	item_container_t::iterator it = std::find(items_.begin(),items_.end(),value);
	if(it != items_.end()){
		it -> disable();
		return true;
	}

	return false;
}

bool qdlgOptionDroplist::is_value_enabled(int value) const
{
	item_container_t::const_iterator it = std::find(items_.begin(),items_.end(),value);
	if(it != items_.end())
		return it -> is_enabled();

	return false;
}

int qdlgOptionDroplist::default_value() const
{
	for(int i = 0; i < items_.size(); i++){
		if(items_[i].is_enabled())
			return items_[i].value();
	}

	return 0;
}