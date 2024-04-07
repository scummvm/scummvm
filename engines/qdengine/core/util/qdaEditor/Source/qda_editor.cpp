/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <commctrl.h>
#include <stdio.h>

#include "..\resource.h"

#include "gr_dispatcher.h"
#include "qda_editor.h"
#include "qd_dialogs.h"

#include "qd_animation_maker.h"

#include "AVIGenerator.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */

extern HINSTANCE xhInstance;
extern HWND hmainWnd;

/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdaEditor::qdaEditor(void)
{
	cur_frame_ = 0;

	edit_mode_ = QDA_STOP;

	timer_ = xclock();

	animation_ = new qdAnimation;
	animation_ -> set_flag(QD_ANIMATION_FLAG_CROP);
	animation_ -> set_flag(QD_ANIMATION_FLAG_COMPRESS);

	animation_file_name_ = NULL;

	default_frame_ = new qdAnimationFrame;
	default_frame_ -> set_length(0.0417f);

	show_border_ = 0;

	show_grid_ = 0;
	grid_step_ = 50;

	grid_col_ = border_col_ = RGB(128,128,128);
	fon_col_ = 0;

	strcpy(file_path_,".\\");
	strcpy(frames_path_,".\\");

	memset(custom_cols_,0,16 * sizeof(int));

	wnd_offset_ = Vect2i(0,0);
	
	changed_ = false;
	need_redraw_ = true;
}

qdaEditor::~qdaEditor(void)
{
	if(animation_) delete animation_;
}

void qdaEditor::load_animation(char* fname)
{
	if(animation_) delete animation_;

	animation_ = new qdAnimation;
	animation_ -> qda_load(fname);

	cur_frame_ = 0;

	set_animation_file_name(fname);

	wnd_offset_ = Vect2i(0,0);
}

void qdaEditor::quant(void)
{
	float dt;
	int idt;

	const float frame_tm = 0.025f;
	static float frame_tm_cur = 0.0f;

	idt = xclock() - timer_;
	if(idt > 300){ 
		timer_ += idt;
		return;
	}

	timer_ += idt;

	dt = float(idt) / 1000.0f;

	if(animation_){
		if(edit_mode_ == QDA_PLAY && animation_ -> is_finished())
			animation_ -> start();

		animation_ -> quant(dt);

		qdAnimationFrame* p = animation_ -> get_cur_frame();

		if(cur_frame_ != p){
			cur_frame_ = p;
			show_cur_frame_info(hmainWnd);
			need_redraw_ = true;
			redraw();
		}
	}
/*
	frame_tm_cur += dt;
	if(frame_tm_cur >= frame_tm){
		qdAnimationFrame* p = cur_frame_;

		cur_frame_ = animation_ -> get_cur_frame();

		if(cur_frame_ != p)
			show_cur_frame_info(hmainWnd);

		redraw();

		frame_tm_cur = 0.0f;
	}
*/
}

void qdaEditor::set_edit_mode(qda_editor_mode mode)
{
	if(!animation_) return;

	edit_mode_ = mode;

	switch(edit_mode_){
		case QDA_STOP:
			animation_ -> stop();
			break;
		case QDA_PLAY:
			animation_ -> resume();
			break;
	}
}

void qdaEditor::change_frame(int delta)
{
	int idx = animation_ -> get_cur_frame_number();
	int max_idx = animation_ -> num_frames();

	idx += delta;
	if(idx < 0) idx = 0;
	if(idx >= max_idx) idx = max_idx - 1;

	animation_ -> set_cur_frame(idx);
}

void qdaEditor::delete_frames(HWND hdlg)
{
	if(!animation_) return;

	int start_idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETSELSTART,(WPARAM)0,(LPARAM)0);
	int end_idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETSELEND,(WPARAM)0,(LPARAM)0);

	int fl = 0;
	if((!start_idx && !end_idx) || end_idx < start_idx){
		int idx = animation_ -> get_cur_frame_number();
		if(idx != -1){
			const qdAnimationFrame* fp = animation_ -> get_cur_frame();
			float tm = - fp -> length() / 2.0f;

			fp = animation_ -> get_frame(idx + 1);
			if(fp) tm += fp -> length() / 2.0f;

			animation_ -> remove_frame(idx);

			animation_ -> set_time(animation_ -> cur_time() + tm); 

			fl = 1;
		}
	}
	else {
		animation_ -> remove_frame_range(start_idx,end_idx);
		fl = 1;
	}

	animation_ -> init_size();
	cur_frame_ = animation_ -> get_cur_frame();

	SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_CLEARSEL,(WPARAM)TRUE,(LPARAM)0);

	show_cur_frame_info(hdlg);

	if(fl){
		toggle_changed(hdlg,1);
		qdlg_resize(hmainWnd);
	}
}

void qdaEditor::show_frame_properties(HWND hdlg,qdAnimationFrame* p)
{
	static char str[256];

	sprintf(str,"%.4f",p -> length());
	SendDlgItemMessage(hdlg,IDC_LENGTH_EDIT,WM_SETTEXT,(WPARAM)0,(LPARAM)str);
/*
	if(p -> check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		SendDlgItemMessage(hdlg,IDC_FLIP_H_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_FLIP_H_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(p -> check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		SendDlgItemMessage(hdlg,IDC_FLIP_V_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_FLIP_V_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(p -> check_flag(QD_ANIMATION_FLAG_SUPPRESS_ALPHA))
		SendDlgItemMessage(hdlg,IDC_ALPHA_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_ALPHA_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(p -> check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		SendDlgItemMessage(hdlg,IDC_BLACK_FON_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_BLACK_FON_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(p -> check_flag(QD_ANIMATION_FLAG_CROP))
		SendDlgItemMessage(hdlg,IDC_CROP_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_CROP_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);
*/
}

void qdaEditor::get_frame_properties(HWND hdlg,qdAnimationFrame* p)
{
	static char str[256];

	SendDlgItemMessage(hdlg,IDC_LENGTH_EDIT,WM_GETTEXT,(WPARAM)256,(LPARAM)str);
	float val = atof(str);
	if(val < 0.01f) val = 0.01f;
	p -> set_length(val);
	toggle_changed(hmainWnd,1);
/*
	int flag = SendDlgItemMessage(hdlg,IDC_FLIP_H_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) p -> set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);
	else p -> drop_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);

	flag = SendDlgItemMessage(hdlg,IDC_FLIP_V_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) p -> set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);
	else p -> drop_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);

	flag = SendDlgItemMessage(hdlg,IDC_ALPHA_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) p -> set_flag(QD_ANIMATION_FLAG_SUPPRESS_ALPHA);
	else p -> drop_flag(QD_ANIMATION_FLAG_SUPPRESS_ALPHA);

	flag = SendDlgItemMessage(hdlg,IDC_BLACK_FON_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) p -> set_flag(QD_ANIMATION_FLAG_BLACK_FON);
	else p -> drop_flag(QD_ANIMATION_FLAG_BLACK_FON);

	flag = SendDlgItemMessage(hdlg,IDC_CROP_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED){ 
		p -> set_flag(QD_ANIMATION_FLAG_CROP);
		p -> crop();
	}
	else {
		p -> drop_flag(QD_ANIMATION_FLAG_CROP);
		if(!animation_ -> check_flag(QD_ANIMATION_FLAG_CROP))
			p -> undo_crop();
	}
*/
}

void qdaEditor::insert_frames(HWND hdlg)
{
	dlg_files_.clear();
	if(!qdlg_get_open_file_names(hdlg,frames_path_,"Targa (*.tga)\0*.tga\0","Добавить кадры",dlg_files_)) return;

	if(!frame_properties_dlg(hdlg,IDD_FRAME_PROPERTIES_DLG,default_frame_)) return;

	if(!dlg_files_.empty()){
		qdAnimationMaker mk;
		mk.set_default_frame_length(default_frame_ -> length());

		HWND hdlg_p = CreateDialog(xhInstance,MAKEINTRESOURCE(IDD_PROGRESS_DLG),hdlg,(DLGPROC)0);

		int pos = animation_ -> get_cur_frame_number();

		int i = 0;
		qdFileNameList::iterator it;
		FOR_EACH(dlg_files_,it){
			if(mk.insert_frame(animation_,it -> c_str(),pos,true)){
				toggle_changed(hdlg,1);
				animation_ -> set_cur_frame(pos);
				show_cur_frame_info(hdlg);
				qdlg_resize(hmainWnd);
		
				int percents = i++ * 100 / dlg_files_.size();
				qdlg_show_progress(percents,hdlg_p);

				strcpy(frames_path_,it -> c_str());
				qdlg_remove_fname(frames_path_);
			}
		}

		EndDialog(hdlg_p,0);
	}
}

void qdaEditor::insert_frames_dir(HWND hdlg)
{
	dlg_files_.clear();
	char* dir = qdlg_get_folder_name(hdlg,frames_path_,"Выбор каталога с кадрами");
	if(!strlen(dir)) return;

	if(!frame_properties_dlg(hdlg,IDD_FRAME_PROPERTIES_DLG,default_frame_)) return;

	qdAnimationMaker mk;
	mk.set_default_frame_length(default_frame_ -> length());

	HWND hdlg_p = CreateDialog(xhInstance,MAKEINTRESOURCE(IDD_PROGRESS_DLG),hdlg,(DLGPROC)0);
	mk.set_callback(qdlg_show_progress,hdlg_p);

	int pos = animation_ -> get_cur_frame_number();

	if(mk.insert_frames(animation_,dir,pos,true)){
		toggle_changed(hdlg,1);
		animation_ -> set_cur_frame(pos);
		show_cur_frame_info(hdlg);
		qdlg_resize(hmainWnd);

		XBuffer buf(frames_path_,MAX_PATH);
		buf < dir < "\\";
	}

	EndDialog(hdlg_p,0);
}

int qdaEditor::frame_properties_dlg(HWND hdlg,int dlg_id,qdAnimationFrame* p)
{
	dlg_frame_ = p;
	return DialogBox(xhInstance,MAKEINTRESOURCE(dlg_id),hdlg,qdlg_frame_properties_proc);
}

void qdaEditor::show_cur_frame_info(HWND hdlg)
{
	if(!animation_) return;

	qdAnimationFrame* fp = animation_ -> get_cur_frame();
	int idx = animation_ -> get_cur_frame_number();
	static char str[256];

	if(!fp){ 
		memset(str,0,256);
		SendDlgItemMessage(hdlg,IDC_FRAME_INFO,WM_SETTEXT,(WPARAM)0,(LPARAM)str);
		return;
	}

	sprintf(str," Кадр %d/%d (%.3f/%.3f сек)",idx + 1,animation_ -> num_frames(),fp -> start_time(),animation_ -> length());

	SendDlgItemMessage(hdlg,IDC_FRAME_INFO,WM_SETTEXT,(WPARAM)0,(LPARAM)str);

	SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_SETRANGE,(WPARAM)TRUE,(LPARAM)MAKELONG(0,animation_ -> num_frames() - 1));
	SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)idx);
}

void qdaEditor::update_cur_frame(HWND hdlg)
{
	if(!animation_) return;

	int idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETPOS,(WPARAM)0,(LPARAM)0);

	animation_ -> set_cur_frame(idx);
}

void qdaEditor::show_animation_properties(HWND hdlg)
{
	static char str[256];

	sprintf(str,"%.3f",animation_ -> length());
	SendDlgItemMessage(hdlg,IDC_LENGTH_EDIT,WM_SETTEXT,(WPARAM)0,(LPARAM)str);
/*
	if(animation_ -> check_flag(QD_ANIMATION_FLAG_LOOP))
		SendDlgItemMessage(hdlg,IDC_LOOP_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_LOOP_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);
*/
	if(animation_ -> check_flag(QD_ANIMATION_FLAG_COMPRESS))
		SendDlgItemMessage(hdlg,IDC_COMPRESS_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_COMPRESS_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);
/*
	if(animation_ -> check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		SendDlgItemMessage(hdlg,IDC_FLIP_H_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_FLIP_H_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(animation_ -> check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		SendDlgItemMessage(hdlg,IDC_FLIP_V_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_FLIP_V_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(animation_ -> check_flag(QD_ANIMATION_FLAG_SUPPRESS_ALPHA))
		SendDlgItemMessage(hdlg,IDC_ALPHA_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_ALPHA_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(animation_ -> check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		SendDlgItemMessage(hdlg,IDC_BLACK_FON_CHECK,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_BLACK_FON_CHECK,BM_SETCHECK,(WPARAM)0,(LPARAM)0);

	if(animation_ -> check_flag(QD_ANIMATION_FLAG_CROP))
		SendDlgItemMessage(hdlg,IDC_CROP_CHECK2,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
	else
		SendDlgItemMessage(hdlg,IDC_CROP_CHECK2,BM_SETCHECK,(WPARAM)0,(LPARAM)0);
*/
}

void qdaEditor::get_animation_properties(HWND hdlg)
{
	int flag;

	static char str[256];
	SendDlgItemMessage(hdlg,IDC_LENGTH_EDIT,WM_GETTEXT,(WPARAM)256,(LPARAM)str);

	float len = atof(str);
	if(fabs(len - animation_-> length()) > 0.001f){
		float scale = len / animation_-> length();
		for(int i = 0; i < animation_ -> num_frames(); i ++){
			if(qdAnimationFrame* p = animation_ -> get_frame(i))
				p -> set_length(p -> length() * scale);
		}

		animation_ -> init_size();
		animation_ -> set_time(0.0f);

		toggle_changed(hmainWnd,1);
		show_cur_frame_info(hmainWnd);
	}
/*
	flag = SendDlgItemMessage(hdlg,IDC_LOOP_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED){ 
		if(!animation_ -> check_flag(QD_ANIMATION_FLAG_LOOP)){
			animation_ -> set_flag(QD_ANIMATION_FLAG_LOOP);
			toggle_changed(hdlg,1);
		}
	}
	else {
		if(animation_ -> check_flag(QD_ANIMATION_FLAG_LOOP)){
			animation_ -> drop_flag(QD_ANIMATION_FLAG_LOOP);
			toggle_changed(hdlg,1);
		}
	}
*/
	flag = SendDlgItemMessage(hdlg,IDC_COMPRESS_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED){ 
		if(!animation_ -> check_flag(QD_ANIMATION_FLAG_COMPRESS)){
			if(animation_ -> check_flag(QD_ANIMATION_FLAG_CROP))
				animation_ -> crop();

			animation_ -> compress();
			toggle_changed(hmainWnd,1);
		}
	}
	else {
		if(animation_ -> check_flag(QD_ANIMATION_FLAG_COMPRESS)){
			animation_ -> uncompress();
			toggle_changed(hmainWnd,1);
		}
	}
/*
	flag = SendDlgItemMessage(hdlg,IDC_FLIP_H_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) animation_ -> set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);
	else animation_ -> drop_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);

	flag = SendDlgItemMessage(hdlg,IDC_FLIP_V_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) animation_ -> set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);
	else animation_ -> drop_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);

	flag = SendDlgItemMessage(hdlg,IDC_ALPHA_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) animation_ -> set_flag(QD_ANIMATION_FLAG_SUPPRESS_ALPHA);
	else animation_ -> drop_flag(QD_ANIMATION_FLAG_SUPPRESS_ALPHA);

	flag = SendDlgItemMessage(hdlg,IDC_BLACK_FON_CHECK,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED) animation_ -> set_flag(QD_ANIMATION_FLAG_BLACK_FON);
	else animation_ -> drop_flag(QD_ANIMATION_FLAG_BLACK_FON);

	flag = SendDlgItemMessage(hdlg,IDC_CROP_CHECK2,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
	if(flag & BST_CHECKED){ 
		animation_ -> set_flag(QD_ANIMATION_FLAG_CROP);
		animation_ -> crop();
	}
	else {
		animation_ -> drop_flag(QD_ANIMATION_FLAG_CROP);
		animation_ -> undo_crop();
	}
*/
}

void qdaEditor::open_animation(HWND hdlg)
{
	char* p = qdlg_get_file_name(hdlg,file_path_,"Анимация (*.qda)\0*.qda\0","Открыть анимацию",1);
	if(!strlen(p)) return;

	if(animation_)
		delete animation_;

	cur_frame_ = 0;

	animation_ = new qdAnimation;
	animation_ -> qda_load(p);

	set_animation_file_name(p);

	wnd_offset_ = Vect2i(0,0);

	qdlg_resize(hdlg);

	toggle_changed(hdlg,0);
}

void qdaEditor::set_animation_file_name(const char* fname)
{
	if(animation_file_name_){ 
		delete animation_file_name_;
		animation_file_name_ = 0;
	}

	if(fname){
		animation_file_name_ = new char[strlen(fname) + 1];
		strcpy(animation_file_name_,fname);

		strcpy(file_path_,fname);
		qdlg_remove_fname(file_path_);
	}
}

void qdaEditor::save_animation(HWND hdlg)
{
	if(!animation_file_name_)
		save_animation_as(hdlg);
	else {
		animation_ -> create_scaled_frames();
		animation_ -> qda_save(animation_file_name_);
		toggle_changed(hdlg,0);
	}
}

void qdaEditor::save_frames(HWND hdlg)
{
	XBuffer fname;

	qdAnimation anm = *animation_;

	anm.uncompress();
	anm.undo_crop();

	for(int i = 0; i < anm.num_frames(); i++){
		fname.init();
		fname < "frame";

		if(i < 100) fname < "0";
		if(i < 10) fname < "0";

		fname <= i < ".tga";

		if(qdAnimationFrame* p = anm.get_frame(i))
			p -> save(fname.c_str());
	}
}

void qdaEditor::save_avi(HWND hdlg)
{
	XBuffer fname;

	qdAnimation anm = *animation_;

	anm.uncompress();
	anm.undo_crop();

	if(!animation_file_name_){
		char* p = qdlg_get_file_name(hdlg,file_path_,"Анимация (*.avi)\0*.avi\0","Сохранить анимацию",0);
		if(!strlen(p)) return;

		qdlg_change_ext(p,".qda");

		set_animation_file_name(p);
	}

	char path[MAX_PATH];
	strcpy(path, animation_file_name_);
	qdlg_change_ext(path,".avi");

	convert2avi(anm, path, hdlg);
}

void qdaEditor::save_animation_as(HWND hdlg)
{
	char* p = qdlg_get_file_name(hdlg,file_path_,"Анимация (*.qda)\0*.qda\0","Сохранить анимацию",0);
	if(!strlen(p)) return;

	qdlg_change_ext(p,".qda");

	set_animation_file_name(p);
	animation_ -> qda_save(animation_file_name_);
	toggle_changed(hdlg,0);
}

void qdaEditor::toggle_grid(HWND hdlg,int state)
{
	if(state == -1)
		show_grid_ ^= 1;
	else
		show_grid_ = state;

	HMENU hmenu = GetMenu(hdlg);

	if(show_grid_)
		CheckMenuItem(hmenu,ID_VIEW_SHOW_GRID,MF_CHECKED);
	else 
		CheckMenuItem(hmenu,ID_VIEW_SHOW_GRID,MF_UNCHECKED);

	redraw();
}

void qdaEditor::toggle_border(HWND hdlg,int state)
{
	if(state == -1)
		show_border_ ^= 1;
	else
		show_border_ = state;

	HMENU hmenu = GetMenu(hdlg);

	if(show_border_)
		CheckMenuItem(hmenu,ID_VIEW_SHOW_BORDER,MF_CHECKED);
	else 
		CheckMenuItem(hmenu,ID_VIEW_SHOW_BORDER,MF_UNCHECKED);

	redraw();
}

void qdaEditor::draw_border(void)
{
	if(!cur_frame_) return;

	int x,y;

	grDispatcher* gp = grDispatcher::instance();
	x = wnd_offset_.x + gp -> Get_SizeX()/2 - cur_frame_ -> size_x()/2;
	y = wnd_offset_.y + gp -> Get_SizeY()/2 - cur_frame_ -> size_y()/2;

	gp -> Rectangle(x,y,cur_frame_ -> size_x(),cur_frame_ -> size_y(),border_col_,0,GR_OUTLINED,1);

	if(cur_frame_ -> picture_size_x() != cur_frame_ -> size_x() || cur_frame_ -> picture_size_y() != cur_frame_ -> size_y()){
		int xx = cur_frame_ -> picture_x();
		int yy = cur_frame_ -> picture_y();

		if(animation_ -> check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			xx = cur_frame_ -> size_x() - xx - cur_frame_ -> picture_size_x();
		if(animation_ -> check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			yy = cur_frame_ -> size_y() - yy - cur_frame_ -> picture_size_y();

		x = wnd_offset_.x + gp -> Get_SizeX()/2 - cur_frame_ -> size_x()/2 + xx;
		y = wnd_offset_.y + gp -> Get_SizeY()/2 - cur_frame_ -> size_y()/2 + yy;

		gp -> Rectangle(x,y,cur_frame_ -> picture_size_x(),cur_frame_ -> picture_size_y(),border_col_,0,GR_OUTLINED,1);
	}
}

void qdaEditor::draw_grid(void)
{
	grDispatcher* gp = grDispatcher::instance();

	for(int x = 0; x < gp -> Get_SizeX(); x += grid_step_)
		gp -> Line(x,0,x,gp -> Get_SizeY(),grid_col_,4);
	for(int y = 0; y < gp -> Get_SizeY(); y += grid_step_)
		gp -> Line(0,y,gp -> Get_SizeX(),y,grid_col_,4);
}

void qdaEditor::redraw(void)
{
	grDispatcher::instance() -> Fill(fon_col_);

	animation_ -> redraw(grDispatcher::instance() -> Get_SizeX()/2 + wnd_offset_.x,grDispatcher::instance() -> Get_SizeY()/2 + wnd_offset_.y,0);
	if(show_border_)
		draw_border();

	if(show_grid_)
		draw_grid();

	grDispatcher::instance() -> Flush();

	need_redraw_ = false;
}

void qdaEditor::selection_properties(HWND hdlg)
{
	int fl = 0;
	int start_idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETSELSTART,(WPARAM)0,(LPARAM)0);
	int end_idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETSELEND,(WPARAM)0,(LPARAM)0);

	if(start_idx || end_idx){
		int i;
		qdAnimationFrame* p,*p0 = animation_ -> get_frame(start_idx);
		if(p0 && frame_properties_dlg(hdlg,IDD_FRAME_PROPERTIES_DLG,p0)){
			animation_ -> init_size();

			for(i = start_idx + 1; i <= end_idx; i ++){
				p = animation_ -> get_frame(i);

				if(p){
					p -> set_length(p0 -> length());

					animation_ -> init_size();

					fl = 1;
				}
			}
		}
	}

	if(fl){
		toggle_changed(hdlg,1);
		show_cur_frame_info(hdlg);
	}
}

void qdaEditor::toggle_changed(HWND hdlg,bool state)
{
	changed_ = state;

	static XBuffer wnd_title;
	static char path[MAX_PATH];

	wnd_title.init();
	wnd_title < "qdaEditor - [";

	if(animation_file_name_){
		strcpy(path,animation_file_name_);
		qdlg_remove_path(path);

		wnd_title < path;
	}
	else
		wnd_title < "новая анимация";

	if(changed_)
		wnd_title < " *";

	wnd_title < "]";

	SendMessage(hdlg,WM_SETTEXT,(WPARAM)0,(LPARAM)wnd_title.c_str());
}

int qdaEditor::save_changes(HWND hdlg)
{
	if(!changed_) return 1;

	static XBuffer question;
	static char path[MAX_PATH];

	question.init();
	question < "Сохранить изменения";

	if(animation_file_name_){
		strcpy(path,animation_file_name_);
		qdlg_remove_path(path);

		question < " в " < path < "?";
	}
	else 
		question < "?";

	int ret = qdlg_warning(hdlg,question);

	if(ret == IDCANCEL) return 0;

	if(ret == IDYES)
		save_animation(hdlg);

	return 1;
}

void qdaEditor::new_animation(HWND hdlg)
{
	if(!save_changes(hdlg)) return;

	delete animation_;
	animation_ = new qdAnimation;
	animation_ -> set_flag(QD_ANIMATION_FLAG_CROP);
	animation_ -> set_flag(QD_ANIMATION_FLAG_COMPRESS);

	set_animation_file_name(0);

	toggle_changed(hdlg,0);
}

void qdaEditor::show_options(HWND hdlg)
{
	static XBuffer buf;

	buf.init();
	buf <= grid_step_;
	SendDlgItemMessage(hdlg,IDC_GRID_STEP,WM_SETTEXT,(WPARAM)0,(LPARAM)buf.c_str());
}

void qdaEditor::get_options(HWND hdlg)
{
	static char buf[256];
	SendDlgItemMessage(hdlg,IDC_GRID_STEP,WM_GETTEXT,(WPARAM)256,(LPARAM)buf);

	grid_step_ = atoi(buf);
	if(grid_step_ < 5) grid_step_ = 5;
}

void qdaEditor::save_options(HWND hdlg)
{
	char path[MAX_PATH];
	strcpy(path,_pgmptr);

	qdlg_remove_fname(path);

	XBuffer ini_name(MAX_PATH);
	ini_name < path < "qda_editor.ini";

	static XBuffer buf;

	buf.init();
	buf <= border_col_;
	WritePrivateProfileString("view","frame_border_color",buf,ini_name.c_str());

	buf.init();
	buf <= grid_col_;
	WritePrivateProfileString("view","grid_color",buf,ini_name.c_str());

	buf.init();
	buf <= fon_col_;
	WritePrivateProfileString("view","fon_color",buf,ini_name.c_str());

	buf.init();
	buf <= show_border_;
	WritePrivateProfileString("view","show_frame_border",buf,ini_name.c_str());

	buf.init();
	buf <= show_grid_;
	WritePrivateProfileString("view","show_grid",buf,ini_name.c_str());

	buf.init();
	buf <= grid_step_;
	WritePrivateProfileString("view","grid_step",buf,ini_name.c_str());

	buf.init();
	buf < "\"" < file_path_ < "\"";
	WritePrivateProfileString("paths","file_path",buf,ini_name.c_str());

	buf.init();
	buf < "\"" < frames_path_ < "\"";
	WritePrivateProfileString("paths","frames_path",buf,ini_name.c_str());
}

void qdaEditor::load_options(HWND hdlg)
{
	char path[MAX_PATH];
	strcpy(path,_pgmptr);

	qdlg_remove_fname(path);

	XBuffer ini_name(MAX_PATH);
	ini_name < path < "qda_editor.ini";

	border_col_ = GetPrivateProfileInt("view","frame_border_color",RGB(128,128,128),ini_name.c_str());
	grid_col_ = GetPrivateProfileInt("view","grid_color",RGB(128,128,128),ini_name.c_str());
	fon_col_ = GetPrivateProfileInt("view","fon_color",0,ini_name.c_str());

	toggle_border(hdlg,GetPrivateProfileInt("view","show_frame_border",0,ini_name.c_str()));
	toggle_grid(hdlg,GetPrivateProfileInt("view","show_grid",0,ini_name.c_str()));
	grid_step_ = GetPrivateProfileInt("view","grid_step",50,ini_name.c_str());

	GetPrivateProfileString("paths","file_path",".\\",file_path_,MAX_PATH,ini_name.c_str());
	GetPrivateProfileString("paths","frames_path",".\\",frames_path_,MAX_PATH,ini_name.c_str());
}

bool qdaEditor::remove_edges(HWND hdlg)
{
	if(MessageBox(hdlg,"Удалить пустые края кадров?","qdaEditor",MB_YESNO | MB_ICONWARNING) != IDYES)
		return false;

	Vect2i v = animation_ -> remove_edges();
	
	if(v.x || v.y){
		toggle_changed(hdlg,1); 
		qdlg_resize(hdlg);
		show_cur_frame_info(hdlg);
		return true;
	}
	return false;
}

void qdaEditor::reverse_frames(HWND hdlg)
{
	int start_idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETSELSTART,(WPARAM)0,(LPARAM)0);
	int end_idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETSELEND,(WPARAM)0,(LPARAM)0);

	bool changed = false;
	if((!start_idx && !end_idx) || end_idx < start_idx){
		start_idx = 0;
		end_idx = animation_ -> num_frames() - 1;
	}

	if(end_idx > start_idx){
		if(animation_ -> reverse_frame_range(start_idx,end_idx))
			changed = true;
	}

//	SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_CLEARSEL,(WPARAM)TRUE,(LPARAM)0);

	if(changed){
		show_cur_frame_info(hdlg);
		toggle_changed(hdlg,1);
		qdlg_resize(hdlg);
	}
}

void qdaEditor::convert2avi(qdAnimation& anm, const char* avi_file_name, HWND hdlg)
{
	const qdAnimationFrame* frame = anm.get_frame(0);
	if(!frame) return;

	int bytes_per_pixel = 3;
	if(frame->format() == GR_ARGB8888)
		bytes_per_pixel = 4;

	int sx = anm.size_x();
	int sy = anm.size_y();

	if(sx % 4) sx += 4 - (sx % 4);
	if(sy % 4) sy += 4 - (sy % 4);

	int dsx = sx * 3;

	BITMAPINFO bi;
	memset(&bi, 0, sizeof(BITMAPINFO));
	bi.bmiHeader.biCompression = BI_RGB;

	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biClrUsed = 0;

	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = sx;
	bi.bmiHeader.biHeight = sy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biSizeImage = dsx * sy;
	bi.bmiHeader.biClrImportant = 0;

	unsigned char* buffer = new unsigned char[dsx * sy];

	CAVIGenerator avi_generator(avi_file_name, &bi.bmiHeader, 30);
	avi_generator.InitEngine();

	HWND hdlg_p = CreateDialog(xhInstance,MAKEINTRESOURCE(IDD_PROGRESS_DLG),hdlg,(DLGPROC)0);

	float frame_length = 1.0f / 30.0f;
	float time = 0.0f;
	while(time < anm.length()){
		anm.set_time(time);

		int percents = round(100.0f * time / anm.length());
		qdlg_show_progress(percents,hdlg_p);

		if(const qdAnimationFrame* p = anm.get_cur_frame()){
			assert(p->size_x() <= anm.size_x() && p->size_y() <= anm.size_y());
			memset(buffer, 0, dsx * sy);
			unsigned char* buf = buffer + (anm.size_x() - p->size_x())/2*3 + (anm.size_y() - p->size_y())/2*dsx;
			const unsigned char* frame_buf = p -> data() + p->size_x() * bytes_per_pixel * (p->size_y() - 1);
			for(int y = 0; y < p->size_y(); y++){
				if(bytes_per_pixel == 4){
					for(int x = 0; x < p->size_x(); x++){
						unsigned r,g,b,a;
						r = frame_buf[x * 4]; 
						g = frame_buf[x * 4 + 1]; 
						b = frame_buf[x * 4 + 2]; 
						a = 255 - frame_buf[x * 4 + 3];

						if(a){
							buf[x * 3] = (r << 8) / a; 
							buf[x * 3 + 1] = (g << 8) / a;
							buf[x * 3 + 2] = (b << 8) / a;
						}
						else 
							buf[x * 3] = buf[x * 3 + 1] = buf[x * 3 + 2] = 0;
					}
				}
				else {
					memcpy(buf, frame_buf, p->size_x() * bytes_per_pixel);
				}

				buf += dsx;
				frame_buf -= p->size_x() * bytes_per_pixel;
			}

			HRESULT hr = avi_generator.AddFrame(buffer);
			if(FAILED(hr))
				assert(0);
		}

		time += frame_length;
	}

	delete buffer;

	EndDialog(hdlg_p,0);

	avi_generator.ReleaseEngine();
}

