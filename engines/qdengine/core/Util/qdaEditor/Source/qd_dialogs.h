#ifndef __QD_DIALOGS_H__
#define __QD_DIALOGS_H__

BOOL APIENTRY qdlg_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL APIENTRY qdlg_options_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL APIENTRY qdlg_frame_properties_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL APIENTRY qdlg_animation_properties_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL APIENTRY qdlg_progress_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL APIENTRY qdlg_scales_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL APIENTRY qdlg_compression_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);

void qdlg_change_ext(char* file_name,const char* ext);
void qdlg_remove_path(char* file_name);
void qdlg_remove_fname(char* file_name);
void qdlg_set_current_directory(const char* file_name);
char* qdlg_get_file_name(HWND hparent,const char* path,const char* filter,const char* caption,int open_mode);
int qdlg_get_open_file_names(HWND hparent,const char* path,const char* filter,const char* caption,qdFileNameList& flist);
char* qdlg_get_folder_name(HWND hdlg,const char* start_path,const char* title = 0);
void qdlg_show_progress(int percents,void* p);

void qdlg_status_string(const char* str);

void qdlg_enable_control(HWND hdlg,int id,int enable);
void qdlg_resize(HWND hdlg);

void qdlg_mouse_move(HWND hwnd,int x,int y,int flags);

int qdlg_warning(HWND hdlg,const char* question);

int qdlg_choose_color(HWND hdlg,int& color,int* def_colors);

#endif /* __QD_DIALOGS_H__ */