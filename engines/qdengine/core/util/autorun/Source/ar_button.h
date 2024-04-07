#ifndef __AR_BUTTON_H__
#define __AR_BUTTON_H__

#include "qd_animation.h"
#include "qd_resource_container.h"

class arLanguageInfo
{
public:
	arLanguageInfo(){ };
	~arLanguageInfo(){ };

	const char* name() const { return name_.c_str(); }
	void set_name(const char* name){ name_ = name; }

	const char* resources_folder() const { return resources_folder_.c_str(); }
	void set_resources_folder(const char* folder){ resources_folder_ = folder; }

	void init(const char* ini_file,const char* section);

private:

	std::string name_;
	std::string resources_folder_;
};

enum arButtonType
{
	ABT_URL,
	ABT_EXEC,
	ABT_EXIT,
	ABT_LANGUAGE
};

class arButtonState
{
public:
	arButtonState() : pos_(0,0), animation_(NULL){ }
	~arButtonState(){ }

	const Vect2i& pos() const { return pos_; }
	void set_pos(const Vect2i& pos){ pos_ = pos; }

	bool has_animation() const { return !animation_name_.empty(); }
	const char* animation_name() const { return animation_name_.c_str(); }
	void set_animation_name(const char* name){ animation_name_ = name; }

	const qdAnimation* animation() const { return animation_; }
	void set_animation(const qdAnimation* p){ animation_ = p; }

private:

	Vect2i pos_;

	std::string animation_name_;
	const qdAnimation* animation_;
};

class arButtonExecObject
{
public:
	arButtonExecObject(int id,const char* str) : language_id_(id), exec_string_(str) { }
	~arButtonExecObject(){ }

	bool operator == (int lng_id) const { return (language_id_ == lng_id); }

	const char* exec_string() const { return exec_string_.c_str(); }

private:

	std::string exec_string_;
	int language_id_;
};

const int AR_LANGUAGE_NONE = -1;

class arButton 
{
public:
	arButton(void);
	~arButton(void);

	bool operator == (int id) const { return ID == id; }

	void set_ID(int id){ ID = id; }

	int exec_show_mode(void) const { return cmd_show; }
	bool redraw_needed(void) const { return need_redraw_; }

	void load_image(const char* fname,int state);
	void init(const char* ini_file,const char* section);

	void redraw();

	void set_state(int st,bool forced = false);
	int get_state(void) const { return state; }

	void set_type(int tp){ type = tp; }
	int get_type(void) const { return type; }

	void set_obj(const char* p){ if(obj_name) free(obj_name); obj_name = strdup(p); }
	char* get_obj(void){ return obj_name; }

	void set_obj_regvalue(const char* p){ if(obj_name_regvalue) free(obj_name_regvalue); obj_name_regvalue = strdup(p); }

	void set_cmdline(const char* p){ if(cmd_line) free(cmd_line); cmd_line = strdup(p); }
	char* get_cmdline(void){ return cmd_line; }

	void set_regkey(const char* p){ if(reg_key) free(reg_key); reg_key = strdup(p); }
	char* get_regkey(void){ return reg_key; }

	void set_reg_exec_path(const char* p){ if(reg_exec_path_value) free(reg_exec_path_value); reg_exec_path_value = strdup(p); }

	void set_checkstr(const char* p){ if(check_after_exec) free(check_after_exec); check_after_exec = strdup(p); }
	char* get_checkstr(void){ return check_after_exec; }

	int check_regkey(void);
	static HKEY open_regkey(const char* regkey_name);

	int get_exec_path(char* path,int path_len);
	int get_exec_obj(char* str,int str_len);

	void enable(bool st = true){ is_enabled_ = st; if(!st) set_state(-1); else set_state(0); }
	bool is_enabled() const { return is_enabled_; }

	int size_x(int state) const { if(states_[state].animation()) return states_[state].animation() -> size_x(); return 0; }
	int size_y(int state) const { if(states_[state].animation()) return states_[state].animation() -> size_y(); return 0; }

	bool hit(int x,int y) const;
	int click(void);

	static void load_resources();

	bool language_dependency() const { return language_dependency_; }
	void set_language_dependency(bool state){ language_dependency_ = state; }

	int language_id() const { return language_id_; }
	void set_language_id(int id){ language_id_ = id; }

	bool has_language_id() const { return (language_id_ != AR_LANGUAGE_NONE); }

	void change_animation_folder(const char* folder);

	static void set_current_language_id(int id){ current_language_id_ = id; }

	typedef qdResourceContainer<arButton> resource_container_t;
	static resource_container_t& resource_container();

private:

	int ID;

	int type;
	char* obj_name;
	char* obj_name_regvalue;

	char* cmd_line;
	int cmd_show;

	char* reg_key;
	char* reg_exec_path_value;

	char* check_after_exec;

	bool is_enabled_;

	int state;
	bool need_redraw_;

	bool language_dependency_;

	int language_id_;

	typedef std::vector<arButtonExecObject> exec_objects_container_t;
	exec_objects_container_t exec_objects_;

	arButtonState states_[2];

	qdAnimation animation_;

	static int current_language_id_;

	int split_cmdline(char* str,char*& exec,char*& args);
};

class arButtonDispatcher
{
public:
	arButtonDispatcher(void);
	~arButtonDispatcher(void);

	void init(const char* ini_file);
	bool init_language(const char* ini_file);

	bool hit(int x,int y);
	void click(int x,int y);
	void redraw();

	int need_check(void){ if(curButton) return 1; return 0; }
	void check_exec(void);

	const qdAnimation* background() const { return background_; }

	bool set_language(int language_id);
	bool set_language(const char* language_name);

private:

	typedef std::vector<arLanguageInfo> languages_container_t;
	languages_container_t languages_;
	int current_language_;

	typedef std::vector<arButton> buttons_container_t;
	buttons_container_t btList;

	const qdAnimation* background_;

	arButton* curButton;
};

#endif /* __AR_BUTTON_H__ */