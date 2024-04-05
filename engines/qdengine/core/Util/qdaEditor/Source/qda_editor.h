#ifndef __QDA_EDITOR_H__
#define __QDA_EDITOR_H__

#include "qd_animation.h"

enum qda_editor_mode
{
	QDA_STOP,
	QDA_PLAY,
	QDA_PAUSE
};

class qdaEditor
{
	int timer_;
	qdAnimation* animation_;
	qdAnimationFrame* cur_frame_;

	qdAnimationFrame* default_frame_;

	qdAnimationFrame* dlg_frame_;
	qdFileNameList dlg_files_;

	char* animation_file_name_;

	qda_editor_mode	edit_mode_;

	int show_grid_;
	int grid_step_;
	int grid_col_;

	int show_border_;
	int border_col_;

	int custom_cols_[16];

	int fon_col_;

	bool changed_;
	bool need_redraw_;

	char file_path_[MAX_PATH];
	char frames_path_[MAX_PATH];

	Vect2i wnd_offset_;

public:

	bool need_redraw() const { return need_redraw_; }

	void move_window(int dx,int dy){ wnd_offset_.x += dx; wnd_offset_.y += dy; }

	void save_options(HWND hdlg);
	void load_options(HWND hdlg);

	int grid_color(void) const { return grid_col_; }
	void set_grid_color(int col){ grid_col_ = col; }

	int border_color(void) const { return border_col_; }
	void set_border_color(int col){ border_col_ = col; }

	int fon_color(void) const { return fon_col_; }
	void set_fon_color(int col){ fon_col_ = col; }

	int* custom_colors(void){ return custom_cols_; }

	void toggle_changed(HWND hdlg,bool state);
	int save_changes(HWND hdlg);

	void toggle_grid(HWND hdlg,int state = -1);
	void draw_grid(void);

	void toggle_border(HWND hdlg,int state = -1);
	void draw_border(void);

	void new_animation(HWND hdlg);

	void set_animation_file_name(const char* fname);
	char* animation_file_name(void){ return animation_file_name_; }

	void open_animation(HWND hdlg);
	void save_animation(HWND hdlg);
	void save_animation_as(HWND hdlg);
	void save_frames(HWND hdlg);
	void save_avi(HWND hdlg);

	bool remove_edges(HWND hdlg);

	qda_editor_mode	edit_mode(void) const { return edit_mode_; }
	void set_edit_mode(qda_editor_mode mode);

	void change_frame(int delta);
	void delete_frames(HWND hdlg);

	void insert_frames(HWND hdlg);
	void insert_frames_dir(HWND hdlg);

	void reverse_frames(HWND hdlg);

	qdAnimation* animation(void){ return animation_; }
	void load_animation(char* fname);

	qdAnimationFrame* cur_frame(void){ if(animation_) return animation_ -> get_cur_frame(); return 0; }
	qdAnimationFrame* default_frame(void){ return default_frame_; }

	qdAnimationFrame* dlg_frame(void){ return dlg_frame_; }
	qdFileNameList& dlg_files_list(void){ return dlg_files_; }

	void show_cur_frame_info(HWND hdlg);
	void update_cur_frame(HWND hdlg);

	void selection_properties(HWND hdlg);

	int frame_properties_dlg(HWND hdlg,int dlg_id,qdAnimationFrame* p);
	void show_frame_properties(HWND hdlg,qdAnimationFrame* p);
	void get_frame_properties(HWND hdlg,qdAnimationFrame* p);

	void show_animation_properties(HWND hdlg);
	void get_animation_properties(HWND hdlg);

	void show_options(HWND hdlg);
	void get_options(HWND hdlg);

	void convert2avi(qdAnimation& anm, const char* avi_file_name, HWND hdlg);

	void quant(void);
	void redraw(void);

	qdaEditor(void);
	~qdaEditor(void);
};

extern qdaEditor* qda_ED;

#endif /* __QDA_EDITOR_H__ */

