/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"
#include <shellapi.h>

#include "XUtil.h"

#include "gr_dispatcher.h"
#include "ar_button.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */

extern HWND hmainWnd;

/* --------------------------- PROTOTYPE SECTION ---------------------------- */

char* getIniKey(const char* fname,const char* section,const char* key);

/* --------------------------- DEFINITION SECTION --------------------------- */

HANDLE abtProcess = NULL;

int arButton::current_language_id_ = -1;

arButton::arButton(void)
{
	state = -1;
	need_redraw_ = true;

	type = ABT_URL;
	obj_name = cmd_line = NULL;
	obj_name_regvalue = NULL;

	cmd_show = -1;

	reg_key = NULL;

	check_after_exec = NULL;
	reg_exec_path_value = NULL;

	language_dependency_ = false;
	language_id_ = AR_LANGUAGE_NONE;

	is_enabled_ = true;
}

arButton::~arButton(void)
{
}

arButton::resource_container_t& arButton::resource_container()
{
	static resource_container_t rct;
	return rct;
}

void arButton::load_resources()
{
	resource_container_t::resource_list_t lst = resource_container().resource_list();
	std::for_each(lst.begin(),lst.end(),std::mem_fun(qdResource::load_resource));
}

void arButton::init(const char* ini_file,const char* section)
{
	char* fname = getIniKey(ini_file,section,"state0");
	if(strlen(fname))
		load_image(fname,0);

	fname = getIniKey(ini_file,section,"state1");
	if(strlen(fname))
		load_image(fname,1);

	Vect2i pos(0,0);

	pos.x = atoi(getIniKey(ini_file,section,"x0"));
	pos.y = atoi(getIniKey(ini_file,section,"y0"));

	states_[0].set_pos(pos);

	pos.x = atoi(getIniKey(ini_file,section,"x1"));
	pos.y = atoi(getIniKey(ini_file,section,"y1"));

	states_[1].set_pos(pos);

	XBuffer buf;
	int sz = atoi(getIniKey(ini_file,section,"url_num"));
	exec_objects_.reserve(sz);
	for(int i = 0; i < sz; i++){
		buf.init();
		buf < "url" <= i < "_language";
		int lng_id = atoi(getIniKey(ini_file,section,buf.c_str()));

		buf.init();
		buf < "url" <= i < "_string";

		exec_objects_.push_back(arButtonExecObject(lng_id,getIniKey(ini_file,section,buf.c_str())));
	}

	fname = getIniKey(ini_file,section,"url");
	if(strlen(fname)){
		type = ABT_URL;
		set_obj(fname);
	}
	else {
		fname = getIniKey(ini_file,section,"exec");

		if(strlen(fname)){
			if(stricmp(fname,"exit")){
				if(stricmp(fname,"language")){
					type = ABT_EXEC;

					set_obj(fname);
					fname = getIniKey(ini_file,section,"args");
					if(strlen(fname))
						set_cmdline(fname);
				}
				else {
					type = ABT_LANGUAGE;

					fname = getIniKey(ini_file,section,"args");
					if(strlen(fname))
						language_id_ = atoi(fname);
				}
			}
			else
				type = ABT_EXIT;
		}
		else {
			fname = getIniKey(ini_file,section,"exec_from_regval");
			if(strlen(fname)){
				type = ABT_EXEC;
				set_obj_regvalue(fname);
			}
		}
	}
	if(atoi(getIniKey(ini_file,section,"minimize")))
		cmd_show = 0;

	if(atoi(getIniKey(ini_file,section,"exit")))
		cmd_show = 1;

	fname = getIniKey(ini_file,section,"regkey");
	if(strlen(fname)) set_regkey(fname);

	fname = getIniKey(ini_file,section,"exec_path_regvalue");
	if(strlen(fname)) set_reg_exec_path(fname);

	if(atoi(getIniKey(ini_file,section,"regkey_check_on_startup"))){
		if(!check_regkey()) enable(0);
	}

	fname = getIniKey(ini_file,section,"check_after_exec");
	if(strlen(fname)) set_checkstr(fname);

	fname = getIniKey(ini_file,section,"language_dependency");
	if(atoi(fname))	language_dependency_ = true;
}

HKEY arButton::open_regkey(const char* regkey_name)
{
	int i;
	const char* subkey_name;
	HKEY key = NULL,subkey;

	if(!regkey_name) return 0;

	static char* key_names[] = { "HKCR\\", "HKCU\\", "HKLM\\", "HKU\\" };
	static HKEY keys[] = { HKEY_CLASSES_ROOT, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_USERS };

	for(i = 0; i < 4; i ++){
		if(!strnicmp(regkey_name,key_names[i],strlen(key_names[i]))){
			key = keys[i];
			subkey_name = regkey_name + strlen(key_names[i]);
			break;
		}
	}

	if(!key) return 0;

	if(RegOpenKeyEx(key,subkey_name,0,KEY_READ,&subkey) != ERROR_SUCCESS)
		return 0;

	return subkey;
}

int arButton::check_regkey(void)
{
	HKEY key = open_regkey(reg_key);

	if(!key)
		return 0;

	RegCloseKey(key);
	return 1;
}

int arButton::click(void)
{
	SHELLEXECUTEINFO inf;
	DWORD exit_code;
	static char str[MAX_PATH * 2];
	char* p0,*p1;

	switch(type){
		case ABT_URL:
			if(!exec_objects_.empty()){
				exec_objects_container_t::const_iterator it = std::find(exec_objects_.begin(),exec_objects_.end(),current_language_id_);
				if(it != exec_objects_.end()){
					ShellExecute(hmainWnd,NULL,it -> exec_string(),NULL,NULL,SW_SHOWMAXIMIZED);
					break;
				}
			}

			if(obj_name)
				ShellExecute(hmainWnd,NULL,obj_name,NULL,NULL,SW_SHOWMAXIMIZED);
			break;
		case ABT_EXEC:
			if(!obj_name && !obj_name_regvalue)
				return 0;

			if(abtProcess){
				if(GetExitCodeProcess(abtProcess,&exit_code)){
					if(exit_code == STILL_ACTIVE)
						return 0;
				}
			}

			inf.cbSize = sizeof(SHELLEXECUTEINFO);
			inf.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
			inf.hwnd = hmainWnd;
			inf.lpVerb = NULL;

			if(obj_name){
				inf.lpFile = obj_name;
				inf.lpParameters = cmd_line;
				if(reg_exec_path_value && get_exec_path(str,MAX_PATH * 2))
					inf.lpDirectory = str;
				else
					inf.lpDirectory = NULL;
			}
			else {
				if(get_exec_obj(str,MAX_PATH * 2) && split_cmdline(str,p0,p1)){
					inf.lpFile = p0;
					inf.lpParameters = p1;
					inf.lpDirectory = NULL;
				}
				else
					return 0;
			}

			inf.nShow = SW_SHOWDEFAULT;

			ShellExecuteEx(&inf);

			abtProcess = inf.hProcess;

			break;
		case ABT_EXIT:
			PostQuitMessage(0);
			break;
	}

	switch(cmd_show){
		case 0:
			ShowWindow(hmainWnd,SW_MINIMIZE);
			break;
		case 1:
			PostQuitMessage(0);
			break;
		case -1:
			break;
	}

	return 1;
}

void arButton::load_image(const char* fname,int state)
{
	if(states_[state].has_animation())
		resource_container().remove_resource(states_[state].animation_name(),this);

	states_[state].set_animation_name(fname);
	states_[state].set_animation(dynamic_cast<const qdAnimation*>(resource_container().add_resource(states_[state].animation_name(),this)));
}

void arButton::redraw()
{
	animation_.redraw(states_[state].pos().x + animation_.size_x()/2,states_[state].pos().y + animation_.size_y()/2,0);
	need_redraw_ = false;
}

void arButtonDispatcher::init(const char* ini_file)
{
	const char* fname = getIniKey(ini_file,"settings","background");
	background_ = dynamic_cast<const qdAnimation*>(arButton::resource_container().add_resource(fname,NULL));

	int sz = atoi(getIniKey(ini_file,"settings","buttons"));
	btList.resize(sz);


	XBuffer name;
	for(int i = 0; i < sz; i ++){
		name.init();
		name < "button" <= i;
		btList[i].set_ID(i);
		btList[i].init(ini_file,name);
	}

	arButton::load_resources();

	for(int i = 0; i < sz; i ++){
		if(btList[i].is_enabled())
			btList[i].set_state(0);
	}

	sz = atoi(getIniKey(ini_file,"settings","languages"));
	languages_.resize(sz);
	for(int i = 0; i < sz; i ++){
		name.init();
		name < "language" <= i;
		languages_[i].init(ini_file,name);
	}
}

bool arButtonDispatcher::init_language(const char* ini_file)
{
	char* str = getIniKey(ini_file,"settings","language_regkey");
	if(strlen(str)){
		if(HKEY key = arButton::open_regkey(str)){
			str = getIniKey(ini_file,"settings","language_regvalue");
			char lng_str[256];
			DWORD len = 256;

			int ret = RegQueryValueEx(key,str,NULL,NULL,(LPBYTE)lng_str,&len);
			RegCloseKey(key);

			if(ret == ERROR_SUCCESS)
				return set_language(lng_str);
		}
	}

	switch(PRIMARYLANGID(GetUserDefaultLangID())){
	case LANG_GERMAN:
		return set_language("german");
	case LANG_ITALIAN:
		return set_language("italian");
	case LANG_ENGLISH:
		return set_language("english");
	case LANG_SPANISH:
		return set_language("spanish");
	case LANG_FRENCH:
		return set_language("french");
	}

	return false;
}

bool arButtonDispatcher::hit(int x,int y)
{
	bool flag = false,redraw_flag = false;
	for(buttons_container_t::iterator it = btList.begin(); it != btList.end(); ++it){
		if(!flag && it -> hit(x,y)){
			if(it -> get_state() != -1){
				it -> set_state(1);
				flag = true;

				if(!redraw_flag)
					redraw_flag = it -> redraw_needed();
			}
		}
		else {
			if(it -> get_state() != -1)
				it -> set_state(0);

			if(!redraw_flag)
				redraw_flag = it -> redraw_needed();
		}
	}

	if(redraw_flag)
		redraw();

	return flag;
}

void arButtonDispatcher::click(int x,int y)
{
	for(buttons_container_t::iterator it = btList.begin(); it != btList.end(); ++it){
		if(it -> hit(x,y)){
			if(it -> get_state() != -1)
				if(it -> click()){
					if(abtProcess)
						curButton = &*it;

					if(it -> has_language_id())
						set_language(it -> language_id());
				}
			return;
		}
	}
}

void arButtonDispatcher::redraw()
{
	if(background_)
		background_ -> redraw(background_ -> size_x()/2,background_ -> size_y()/2,0);

	for(buttons_container_t::iterator it1 = btList.begin(); it1 != btList.end(); ++it1)
		it1 -> redraw();

	grDispatcher::instance() -> Flush();
}

arButtonDispatcher::arButtonDispatcher(void)
{
	curButton = NULL;
	current_language_ = -1;
}

arButtonDispatcher::~arButtonDispatcher(void)
{
	btList.clear();
}

void arButtonDispatcher::check_exec(void)
{
	DWORD exit_code;

	if(!abtProcess){ 
		curButton = NULL;
		return;
	}

	if(GetExitCodeProcess(abtProcess,&exit_code)){
		if(exit_code != STILL_ACTIVE){
			switch(curButton -> exec_show_mode()){
				case 0:
					ShowWindow(hmainWnd,SW_SHOWNORMAL);
					break;
				case -1:
					break;
			}
			if(curButton -> get_checkstr()){
				XBuffer check_str(curButton -> get_checkstr(),strlen(curButton -> get_checkstr()));
				while(check_str.tell() < check_str.size()){
					int id;
					check_str >= id;
					buttons_container_t::iterator it = std::find(btList.begin(),btList.end(),id);
					if(it != btList.end()){
						if(it -> check_regkey())
							it -> enable(true);
						else 
							it -> enable(false);
					}
				}
			}
			curButton = NULL;
			redraw();
		}
	}
	else
		curButton = NULL;
}

int arButton::get_exec_path(char* path,int path_len)
{
	int ret;
	HKEY key = open_regkey(reg_key);
	DWORD len = path_len;

	if(!key) return 0;

	ret = RegQueryValueEx(key,reg_exec_path_value,NULL,NULL,(LPBYTE)path,&len);
	RegCloseKey(key);

	return (ret == ERROR_SUCCESS);
}

int arButton::get_exec_obj(char* str,int str_len)
{
	int ret;
	HKEY key = open_regkey(reg_key);
	DWORD len = str_len;

	if(!key) return 0;

	ret = RegQueryValueEx(key,obj_name_regvalue,NULL,NULL,(LPBYTE)str,&len);
	RegCloseKey(key);

	return (ret == ERROR_SUCCESS);
}

int arButton::split_cmdline(char* str,char*& exec,char*& args)
{
	int i,sz = strlen(str);

	if(str[sz - 1] == '\"'){
		for(i = sz - 2; i >= 0; i --){
			if(str[i] == '\"'){
				str[i - 1] = 0;

				exec = str;
				args = str + i;

				return 1;
			}
		}
	}
	else {
		for(i = 0; i < sz; i ++){
			if(str[i] == ' '){
				//if(i > 5 && !strnicmp(str + i - 4,".exe",4)){
					str[i] = 0;

					exec = str;
					args = str + i + 1;

					return 1;
				//}
			}
		}
	}
	return 0;
}

bool arButton::hit(int x,int y) const
{
	if(!is_enabled()) return false;

	if(!state){
		if(x >= states_[0].pos().x && x < states_[0].pos().x + size_x(0) && y >= states_[0].pos().y && y < states_[0].pos().y + size_y(0)){
			if(states_[0].animation()){
				int x1 = x - states_[0].pos().x - size_x(0)/2;
				int y1 = y - states_[0].pos().y - size_y(0)/2;

				return states_[0].animation() -> hit(x1,y1);
			}
			else
				return true;
		}
	}

	if(x >= states_[1].pos().x && x < states_[1].pos().x + size_x(1) && y >= states_[1].pos().y && y < states_[1].pos().y + size_y(1)){
		if(states_[1].animation()){
			int x1 = x - states_[1].pos().x - size_x(1)/2;
			int y1 = y - states_[1].pos().y - size_y(1)/2;

			return states_[1].animation() -> hit(x1,y1);
		}
	}

	return false; 
}

void arButton::set_state(int st,bool forced)
{
	if(state != st || forced){
		state = st;
		need_redraw_ = true;

		if(st != -1 && states_[st].animation())
			states_[st].animation() -> create_reference(&animation_);
		else
			animation_.clear();

		animation_.start();
	}
}

void arButton::change_animation_folder(const char* folder)
{
	char drive[_MAX_DRIVE];
   	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
   	char ext[_MAX_EXT];

	static XBuffer name_buf(_MAX_PATH);

	if(states_[0].has_animation()){
		_splitpath(states_[0].animation_name(),drive,dir,fname,ext);

		name_buf.init();
		name_buf < drive < folder < "\\" < fname < ext;

		load_image(name_buf.c_str(),0);
	}

	if(states_[1].has_animation()){
		_splitpath(states_[1].animation_name(),drive,dir,fname,ext);

		name_buf.init();
		name_buf < drive < folder < "\\" < fname < ext;

		load_image(name_buf.c_str(),1);
	}
}

bool arButtonDispatcher::set_language(int language_id)
{
	if(language_id < 0 || language_id >= languages_.size())
		return false;
	if(language_id == current_language_)
		return true;

	current_language_ = language_id;
	arButton::set_current_language_id(language_id);

	const char* folder = languages_[current_language_].resources_folder();

	for(buttons_container_t::iterator it = btList.begin(); it != btList.end(); ++it){
		if(it -> language_dependency())
			it -> change_animation_folder(folder);
	}

	arButton::load_resources();

	for(buttons_container_t::iterator it = btList.begin(); it != btList.end(); ++it){
		if(it -> language_dependency())
			it -> set_state(it -> get_state(),true);
	}

	redraw();

	return true;
}

bool arButtonDispatcher::set_language(const char* language_name)
{
	for(languages_container_t::const_iterator it = languages_.begin(); it != languages_.end(); ++it){
		if(!stricmp(it -> name(),language_name)){
			return set_language(it - languages_.begin());
		}
	}

	return false;
}

void arLanguageInfo::init(const char* ini_file,const char* section)
{
	set_name(getIniKey(ini_file,section,"name"));
	set_resources_folder(getIniKey(ini_file,section,"resources"));
}

