#ifndef __APP_CORE_H__
#define __APP_CORE_H__

// Main window handle
void* appGetHandle();
void appSetHandle(void* hwnd);

unsigned app_memory_usage();

typedef void (*SetFunc)(const char*);
typedef const char* (*GetFunc)(void);

// Macroses
#define QD_ADJUST_TO_REL_FILE_MEMBER(res_dir, get_file, set_file, can_overwrite, ret) {\
		if (false == app_io::relat_path(get_file())){							\
			std::string str = res_dir + app_io::strip_path(get_file());			\
			if (!can_overwrite && app_io::dupe_resolve_file_copy(str, get_file()))\
				set_file(str.c_str());											\
			else if (can_overwrite) {											\
				std::string msg = "Program attempt copy file:\n";				\
				msg += get_file(), msg += "\n\n";								\
				msg += "to: \n";												\
				msg += str, msg += "\n\n";										\
				msg += "But file already exist. Overwrite?";					\
				if (!app_io::is_file_exist(str.c_str()) ||						\
					IDYES == MessageBox(NULL, msg.c_str(), "Message",			\
										MB_YESNO | MB_TASKMODAL))				\
				{																\
					if (app_io::copy_file(str.c_str(), get_file()))				\
						set_file(str.c_str());									\
					else ret = false;											\
				}																\
				else if (app_io::dupe_resolve_file_copy(str, get_file()))		\
					set_file(str.c_str());										\
				else ret = false;												\
			}																	\
			else ret = false;													\
		}																		\
	}

namespace app_io {

bool is_file_exist(const char* file_name);
bool is_directory_exist(const char* dir_name);

bool set_current_directory(const char* file_name);
const char* strip_path(const char* file_name);
const char* strip_file_name(const char* path);
const char* get_ext(const char* file_name);
const char* change_ext(const char* file_name, const char* new_ext);

bool create_directory(const char* path);
bool remove_directory(const char* path);
bool full_remove_directory(const char* path);

unsigned file_size(const char* file_name);

bool copy_file(const char* target,const char* source);
bool copy_file_list(const std::list<std::string>& file_list,const char* target_dir,const char* file_extension);
//! При успешном копировании в target помещается путь, по которому скопировали
bool dupe_resolve_file_copy(std::string& target, const char* source);
std::string path_to_file_name(const std::string& str);

bool relat_path(const char* path);
void adjust_dir_end_slash(std::string& str);
};

#endif /* __APP_CORE_H__ */


