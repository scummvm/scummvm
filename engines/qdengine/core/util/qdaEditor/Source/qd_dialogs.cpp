/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

//#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>

#include "..\resource.h"

#include "gr_dispatcher.h"
#include "gr_tile_animation.h"

#include "qda_editor.h"

#include "qdlg_controls.h"

#include "qd_dialogs.h"
#include "qd_setup.h"
#include "qdscr_parser.h"

#include "ResourceDispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */

extern ResourceDispatcher resD;

/* --------------------------- PROTOTYPE SECTION ---------------------------- */

void qdlg_init(HWND hdlg);

void qdlg_scales_dlg_init(HWND hdlg);
void qdlg_scales_dlg_finit(HWND hdlg);
void qdlg_reset_scales_list(HWND hdlg);

/* --------------------------- DEFINITION SECTION --------------------------- */

HWND hDlg = NULL;
HWND hStatus = NULL;

qdlgControlList qdlg_controls;

BOOL APIENTRY qdlg_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int val,wmID;
	switch(msg){
		case WM_TIMER:
			resD.quant();
			break;
		case WM_SIZE:
			qdlg_resize(hdlg);
			return 0;
		case WM_INITDIALOG:
			qdlg_init(hdlg);
			SetTimer(hdlg,1,60,NULL);
			return 1;
		case WM_COMMAND:
			wmID = LOWORD(wParam);
			switch(wmID){
				case ID_FILE_NEW_ANIMATION:
					qda_ED -> new_animation(hdlg);
					return 1;
				case ID_FILE_OPEN_ANIMATION:
					if(qda_ED -> save_changes(hdlg))
						qda_ED -> open_animation(hdlg);
					return 1;
				case ID_VIEW_SHOW_BORDER:
					qda_ED -> toggle_border(hdlg);
					return 1;
				case ID_VIEW_SHOW_GRID:
					qda_ED -> toggle_grid(hdlg);
					return 1;
				case ID_VIEW_OPTIONS:
					DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_OPTIONS_DLG),hdlg,qdlg_options_proc);
					qda_ED -> redraw();
					return 1;
				case ID_FILE_SAVE_ANIMATION:
					qda_ED -> save_animation(hdlg);
					return 1;
				case ID_FILE_SAVE_AS_ANIMATION:
					qda_ED -> save_animation_as(hdlg);
					return 1;
				case ID_FILE_SAVE_FRAMES:
					qda_ED -> save_frames(hdlg);
					return 1;
				case ID_FILE_SAVE_AVI:
					qda_ED -> save_avi(hdlg);
					return 1;
				case IDC_PLAY:
					qda_ED -> set_edit_mode(QDA_PLAY);
					return 1;
				case IDC_STOP:
					qda_ED -> set_edit_mode(QDA_STOP);
					return 1;
				case IDC_NEXT:
				case ID_ANIMATION_GOTO_NEXT_FRAME:
					qda_ED -> change_frame(1);
					return 1;
				case IDC_PREV:
				case ID_ANIMATION_GOTO_PREV_FRAME:
					qda_ED -> change_frame(-1);
					return 1;
				case IDC_BEGIN:
				case ID_ANIMATION_GOTO_BEG:
					qda_ED -> change_frame(-qda_ED -> animation() -> num_frames());
					return 1;
				case IDC_END:
				case ID_ANIMATION_GOTO_END:
					qda_ED -> change_frame(qda_ED -> animation() -> num_frames());
					return 1;
				case ID_ANIMATION_DELETE_FRAMES:
					qda_ED -> delete_frames(hdlg);
					return 1;
				case ID_ANIMATION_INSERT_FRAMES:
					qda_ED -> insert_frames(hdlg);
					return 1;
				case ID_ANIMATION_INSERT_FRAMES_DIR:
					qda_ED -> insert_frames_dir(hdlg);
					return 1;
				case ID_ANIMATION_UNSELECT:
					SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_CLEARSEL,(WPARAM)TRUE,(LPARAM)0);
					return 1;
				case ID_ANIMATION_FRAME_PROPERTIES:
				case IDC_FRAME_PROPS:
					if(qda_ED -> animation() -> get_cur_frame()){
						if(qda_ED -> frame_properties_dlg(hdlg,IDD_FRAME_PROPERTIES_DLG,qda_ED -> animation() -> get_cur_frame())){
							qda_ED -> animation() -> init_size();
							qda_ED -> toggle_changed(hdlg,1);
							qda_ED -> show_cur_frame_info(hdlg);
						}
					}
					return 1;
				case ID_ANIMATION_SELECTED_FRAMES_PROPERTIES:
					qda_ED -> selection_properties(hdlg);
					return 1;
				case ID_ANIMATION_PROPERTIES:
				case IDC_ANIMATION_PROPS:
					DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_ANIMATION_PROPERTIES_DLG),hdlg,qdlg_animation_properties_proc);
					return 1;
				case ID_ANIMATION_SET_SEL_BEG:
					val = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETPOS,(WPARAM)0,(LPARAM)0);
					SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_SETSELSTART,(WPARAM)TRUE,(LPARAM)val);
					return 1;
				case ID_ANIMATION_SET_SEL_END:
					val = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETPOS,(WPARAM)0,(LPARAM)0);
					SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_SETSELEND,(WPARAM)TRUE,(LPARAM)val);
					return 1;
				case ID_EDIT_CROP_FRAMES:
					qda_ED -> remove_edges(hdlg);
					return 1;
				case ID_ANIMATION_INVERT_TIME:
					qda_ED -> reverse_frames(hdlg);
					return 1;
				case ID_ANIMATION_SCALES:
					DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_SCALES_DLG),hdlg,qdlg_scales_proc);
					return 1;
				case ID_ANIMATION_TILE_COMPRESS:
					DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_TILE_COMPRESS_DLG),hdlg,qdlg_compression_proc);
					return 1;
				case IDOK:
				case IDCANCEL:
				case ID_FILE_EXIT:
					if(!qda_ED -> save_changes(hdlg))
						return 1;

					hDlg = 0;

					qda_ED -> save_options(hdlg);

					KillTimer(hdlg,1);
					EndDialog(hdlg,1);
					PostQuitMessage(0);
					return 1;
			}
			break;
		case WM_HSCROLL:
			if((HWND)lParam == GetDlgItem(hdlg,IDC_FRAME_SLIDER))
				qda_ED -> update_cur_frame(hdlg);
			break;
		case WM_CLOSE:
			if(!qda_ED -> save_changes(hdlg))
				return 1;

			hDlg = 0;

			qda_ED -> save_options(hdlg);

			KillTimer(hdlg,1);
			EndDialog(hdlg,0);
			PostQuitMessage(0);
			return 1;
	}
	return 0;
}

char* qdlg_get_file_name(HWND hparent,const char* path,const char* filter,const char* caption,int open_mode)
{
	const int path_len = _MAX_PATH;
	static char out_file[_MAX_FNAME];
	static char out_path[path_len];
	OPENFILENAME fname;

	memset(out_path,0,path_len);
	memset(&fname,0,sizeof(OPENFILENAME));

	out_path[0] = 0;

	fname.lStructSize = sizeof(OPENFILENAME);

	fname.lpstrFile = out_path;
	fname.nMaxFile = path_len;
	fname.lpstrFileTitle = out_file;
	fname.nMaxFileTitle = _MAX_FNAME;
	fname.lpstrTitle = caption;

	fname.lpstrFilter = filter;

	fname.hwndOwner = hparent;

	fname.lpstrInitialDir = path;
	fname.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

	if(open_mode)
		GetOpenFileName(&fname);
	else
		GetSaveFileName(&fname);

	return out_path;
}

int qdlg_get_open_file_names(HWND hparent,const char* path,const char* filter,const char* caption,qdFileNameList& flist)
{
	const int path_len = 16*1024;
	static char out_file[_MAX_PATH];
	static char out_path[path_len];
	OPENFILENAME fname;

	memset(out_path,0,path_len);
	memset(&fname,0,sizeof(OPENFILENAME));

	out_path[0] = 0;

	fname.lStructSize = sizeof(OPENFILENAME);

	fname.lpstrFile = out_path;
	fname.nMaxFile = path_len;
	fname.lpstrFileTitle = out_file;
	fname.nMaxFileTitle = _MAX_PATH;
	fname.lpstrTitle = caption;

	fname.lpstrFilter = filter;

	fname.hwndOwner = hparent;

	fname.lpstrInitialDir = path;
	fname.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if(!GetOpenFileName(&fname)) return 0;

	flist.clear();

	if(out_path[strlen(out_path) + 1]){
		char* p = out_path + strlen(out_path) + 1;
		while(strlen(p)){
			sprintf(out_file,"%s\\%s",out_path,p);
			flist.push_back(out_file);

			p += strlen(p) + 1;
		}
	}
	else
		flist.push_back(out_path);

	flist.sort();

	return 1;
}

void qdlg_set_current_directory(const char* file_name)
{
	char fpath[_MAX_PATH];
	_fullpath(fpath,file_name,_MAX_PATH);

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
   	char ext[_MAX_EXT];

	_splitpath(fpath,drive,dir,fname,ext);
	sprintf(fpath,"%s%s",drive,dir);

	SetCurrentDirectory(fpath);
}

void qdlg_change_ext(char* file_name,const char* ext)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
   	char old_ext[_MAX_EXT];

	_splitpath(file_name,drive,dir,fname,old_ext);
	sprintf(file_name,"%s%s%s%s",drive,dir,fname,ext);
}

void qdlg_remove_path(char* file_name)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
   	char ext[_MAX_EXT];

	_splitpath(file_name,drive,dir,fname,ext);
	sprintf(file_name,"%s%s",fname,ext);
}

void qdlg_remove_fname(char* file_name)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
   	char ext[_MAX_EXT];

	_splitpath(file_name,drive,dir,fname,ext);
	sprintf(file_name,"%s%s",drive,dir);
}

void qdlg_enable_control(HWND hdlg,int id,int enable)
{
	HWND hwnd = GetDlgItem(hdlg,id);
	if(hwnd)
		EnableWindow(hwnd,enable);
}

void qdlg_update_frame_info(HWND hdlg)
{
	if(!qda_ED -> animation()) return;

	int idx = SendDlgItemMessage(hdlg,IDC_FRAME_SLIDER,TBM_GETPOS,(WPARAM)0,(LPARAM)0);

	qda_ED -> animation() -> set_cur_frame(idx);
}

void qdlg_init(HWND hdlg)
{
	hDlg = hdlg;

	grDispatcher* gp = grDispatcher::instance();
	gp -> init(0,0,GR_RGB888,GetDlgItem(hdlg,IDC_GR_WINDOW));
	gp -> SetClip();
	gp -> SetClipMode(1);

	HMENU hmenu = GetMenu(hdlg);

	HICON ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_APP),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	assert(ic);
	SendMessage(hdlg,WM_SETICON,(WPARAM)ICON_BIG,(LPARAM)ic);
	SendMessage(hdlg,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_STOP),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_STOP,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_PLAY),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_PLAY,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_BEGIN),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_BEGIN,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_END),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_END,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_PREV),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_PREV,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_NEXT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_NEXT,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_FRAME),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_FRAME_PROPS,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	ic = (HICON)LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ANIMATION),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	SendDlgItemMessage(hdlg,IDC_ANIMATION_PROPS,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)ic);

	qdlgControl* p;
	p = new	qdlgControl(hdlg,IDC_STOP,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," Стоп");
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_PLAY,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," Старт");
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_BEGIN,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," На первый кадр");
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_PREV,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," На кадр назад");
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_NEXT,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," На кадр вперед");
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_END,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," На последний кадр");
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_FRAME_INFO,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL);
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_FRAME_SLIDER,QDLG_ALIGN_LEFT,QDLG_ALIGN_RIGHT,QDLG_SIZE_SCALE,QDLG_SIZE_ORIGINAL);
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_FRAME_PROPS,QDLG_ALIGN_RIGHT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," Свойства кадра");
	qdlg_controls.push_back(p);

	p = new	qdlgControl(hdlg,IDC_ANIMATION_PROPS,QDLG_ALIGN_RIGHT,QDLG_ALIGN_RIGHT,QDLG_SIZE_ORIGINAL,QDLG_SIZE_ORIGINAL," Свойства анимации");
	qdlg_controls.push_back(p);

	hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE,"",hdlg,83);

	qda_ED -> toggle_border(hdlg,0);
	qda_ED -> toggle_grid(hdlg,0);
	qda_ED -> toggle_changed(hdlg,0);
}

BOOL APIENTRY qdlg_frame_properties_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int wmID;
//	static char str[256];

	switch(msg){
		case WM_INITDIALOG:
			qda_ED -> show_frame_properties(hdlg,qda_ED -> dlg_frame());
/*
			SendDlgItemMessage(hdlg,IDC_LENGTH_SPIN,UDM_SETBUDDY,(WPARAM)GetDlgItem(hdlg,IDC_LENGTH_EDIT),(LPARAM)0);
			SendDlgItemMessage(hdlg,IDC_LENGTH_SPIN,UDM_SETRANGE,(WPARAM)0,(LPARAM)MAKELONG(10000,0));
*/
			return 1;
		case WM_COMMAND:
			wmID = LOWORD(wParam);
			switch(wmID){
				case IDC_BROWSE:
					qdlg_get_open_file_names(hdlg,"","Targa (*.tga)\0*.tga\0","Добавить кадры",qda_ED -> dlg_files_list());
					break;
				case IDOK:
					qda_ED -> get_frame_properties(hdlg,qda_ED -> dlg_frame());
					EndDialog(hdlg,1);
					return 1;
				case IDCANCEL:
					EndDialog(hdlg,0);
					return 1;
			}
			break;
/*
		case WM_VSCROLL:
			if((HWND)lParam == GetDlgItem(hdlg,IDC_LENGTH_SPIN)){
				if(LOWORD(wParam) == SB_THUMBPOSITION){
					float val = float(HIWORD(wParam))/100.0f;
					sprintf(str,"%.3f",val);
					SendDlgItemMessage(hdlg,IDC_LENGTH_EDIT,WM_SETTEXT,(WPARAM)0,(LPARAM)str);
				}
			}
			break;
*/
	}
	return 0;
}

BOOL APIENTRY qdlg_animation_properties_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int wmID;

	switch(msg){
		case WM_INITDIALOG:
			qda_ED -> show_animation_properties(hdlg);
			return 1;
		case WM_COMMAND:
			wmID = LOWORD(wParam);
			switch(wmID){
				case IDOK:
					qda_ED -> get_animation_properties(hdlg);
					EndDialog(hdlg,1);
					return 1;
				case IDCANCEL:
					EndDialog(hdlg,0);
					return 1;
			}
			break;
	}
	return 0;
}

void qdlg_resize(HWND hdlg)
{
	qdlgControlList::iterator it,slider_ctl;
	FOR_EACH(qdlg_controls,it){
		(*it) -> update();
		if((*it) -> control_ID() == IDC_FRAME_SLIDER)
			slider_ctl = it;
	}

	RECT rt;
	GetClientRect(hdlg,&rt);
	int sx = rt.right - rt.left;
	int sy = rt.bottom - rt.top;

	int gr_sx = sx - 20;
	int gr_sy = (*slider_ctl) -> y() - 20;

	int wsx,wsy;
	if(!qda_ED -> animation() -> is_empty()){
		wsx = (qda_ED -> animation() -> size_x() + 4 < gr_sx) ? qda_ED -> animation() -> size_x() + 4 : gr_sx;
		wsy = (qda_ED -> animation() -> size_y() + 4 < gr_sy) ? qda_ED -> animation() -> size_y() + 4 : gr_sy;
	}
	else {
		wsx = gr_sx;
		wsy = gr_sy;
	}

	if(wsx <= 1) wsx = 1;
	if(wsy <= 1) wsy = 1;

	grDispatcher::instance() -> init(wsx,wsy,GR_RGB888,GetDlgItem(hdlg,IDC_GR_WINDOW));

	int x = 10 + (gr_sx - wsx) / 2;
	int y = 10 + (gr_sy - wsy) / 2;

	SetWindowPos(GetDlgItem(hdlg,IDC_GR_WINDOW),0,x,y,wsx,wsy,SWP_NOZORDER | SWP_NOCOPYBITS);

	if(hStatus)
		SetWindowPos(hStatus,NULL,0,sy - 20,sx,20,SWP_NOZORDER);

	qda_ED -> redraw();
}

void qdlg_status_string(const char* str)
{
	if(hStatus){
		if(str)
			SendMessage(hStatus,SB_SETTEXT,(WPARAM)0,(LPARAM)str);
		else
			SendMessage(hStatus,SB_SETTEXT,(WPARAM)0,(LPARAM)"");
	}
}

void qdlg_mouse_move(HWND hwnd,int x,int y,int flags)
{
	bool fl = false;
	qdlgControlList::iterator it;
	FOR_EACH(qdlg_controls,it){
		if((*it) -> hwnd() == hwnd){ 
			qdlg_status_string((*it) -> comment());
			fl = true;
		}
	}

	if(!fl) qdlg_status_string(0);

	if(hwnd == (HWND)grDispatcher::instance() -> Get_hWnd()){
		static int mx = -1;
		static int my = -1;

		if(mx != -1 && flags & MK_LBUTTON){
			int dx = x - mx;
			int dy = y - my;

			if(dx || dy){
				qda_ED -> move_window(dx,dy);
				qda_ED -> redraw();
			}
		}

		mx = x;
		my = y;
	}
}

int qdlg_warning(HWND hdlg,const char* question)
{
	return MessageBox(hdlg,question,"qdaEditor",MB_YESNOCANCEL | MB_ICONWARNING);
}

int qdlg_choose_color(HWND hdlg,int& color,int* def_colors)
{
	CHOOSECOLOR cc;

	memset(&cc,0,sizeof(CHOOSECOLOR));

	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hdlg;
	cc.lpCustColors = (unsigned long*)def_colors;
	cc.rgbResult = color;
	cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

	if(ChooseColor(&cc)){
		color = cc.rgbResult;
		return 1;
	}
	return 0;
}

BOOL APIENTRY qdlg_options_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int wmID;

	LPDRAWITEMSTRUCT dis;

	static HBRUSH hb_grid = 0;
	static HBRUSH hb_border = 0;
	static HBRUSH hb_fon = 0;

	static int grid_color,border_color,fon_color;

	switch(msg){
		case WM_DRAWITEM:
			dis = (LPDRAWITEMSTRUCT)lParam;
			if(dis -> CtlID == IDC_BORDER_COLOR)
				FillRect(dis -> hDC,&dis -> rcItem,hb_border);
			if(dis -> CtlID == IDC_GRID_COLOR)
				FillRect(dis -> hDC,&dis -> rcItem,hb_grid);
			if(dis -> CtlID == IDC_FON_COLOR)
				FillRect(dis -> hDC,&dis -> rcItem,hb_fon);
			return 1;
		case WM_INITDIALOG:
			qda_ED -> show_options(hdlg);

			border_color = qda_ED -> border_color();
			grid_color = qda_ED -> grid_color();
			fon_color = qda_ED -> fon_color();

			hb_border = CreateSolidBrush(border_color);
			hb_grid = CreateSolidBrush(grid_color);
			hb_fon = CreateSolidBrush(fon_color);
			return 1;
		case WM_COMMAND:
			wmID = LOWORD(wParam);
			switch(wmID){
				case IDC_BORDER_COLOR:
					if(qdlg_choose_color(hdlg,border_color,qda_ED -> custom_colors())){
						DeleteObject(hb_border);
						hb_border = CreateSolidBrush(border_color);
						InvalidateRect(GetDlgItem(hdlg,IDC_BORDER_COLOR),0,0);
					}
					return 0;
				case IDC_GRID_COLOR:
					if(qdlg_choose_color(hdlg,grid_color,qda_ED -> custom_colors())){
						DeleteObject(hb_grid);
						hb_grid = CreateSolidBrush(grid_color);
						InvalidateRect(GetDlgItem(hdlg,IDC_GRID_COLOR),0,0);
					}
					return 0;
				case IDC_FON_COLOR:
					if(qdlg_choose_color(hdlg,fon_color,qda_ED -> custom_colors())){
						DeleteObject(hb_fon);
						hb_fon = CreateSolidBrush(fon_color);
						InvalidateRect(GetDlgItem(hdlg,IDC_FON_COLOR),0,0);
					}
					return 0;
				case IDC_GRID_STEP:
					return 0;
				case IDOK:
					DeleteObject(hb_border);
					DeleteObject(hb_grid);
					DeleteObject(hb_fon);

					qda_ED -> set_border_color(border_color);
					qda_ED -> set_grid_color(grid_color);
					qda_ED -> set_fon_color(fon_color);

					qda_ED -> get_options(hdlg);
					EndDialog(hdlg,1);
					return 0;
				case IDCANCEL:
					DeleteObject(hb_border);
					DeleteObject(hb_grid);
					DeleteObject(hb_fon);
					EndDialog(hdlg,0);
					return 0;
			}
			break;
	}
	return 0;
}

char qdlg_pathBuf[MAX_PATH];
int CALLBACK qdlg_browse_folder_proc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];

	switch(uMsg) {
		case BFFM_INITIALIZED: 
			SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)qdlg_pathBuf);
			break;
		case BFFM_SELCHANGED: 
			if(SHGetPathFromIDList((LPITEMIDLIST)lp,szDir))
				SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
			break;
		default:
			break;
         }

         return 0;
}

char* qdlg_get_folder_name(HWND hdlg,const char* start_path,const char* title)
{
	BROWSEINFO bi;
	char* p;
        static char szDir[MAX_PATH];
        LPITEMIDLIST pidl;
        LPMALLOC pMalloc;

	*szDir = 0;

	if(!start_path || !strlen(start_path)) start_path = ".\\";

	GetFullPathName(start_path,MAX_PATH,qdlg_pathBuf,&p);
	if(p) *p = 0;

        if(SUCCEEDED(SHGetMalloc(&pMalloc))){
		ZeroMemory(&bi,sizeof(bi));
		bi.hwndOwner = hdlg;
		bi.pszDisplayName = 0;
		bi.pidlRoot = 0;
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
		bi.lpfn = qdlg_browse_folder_proc;
		bi.lpszTitle = title;

		pidl = SHBrowseForFolder(&bi);
		if(pidl)
			SHGetPathFromIDList(pidl,szDir);
		else 
			*szDir = 0;

		pMalloc -> Free(pidl);
		pMalloc -> Release();
	}
	return szDir;
}

void qdlg_show_progress(int percents,void* p)
{
	HWND hdlg = (HWND)p;
	SendDlgItemMessage(hdlg,IDC_PROGRESS,PBM_SETPOS,(WPARAM)percents,(LPARAM)0);
	ShowWindow(hdlg,SW_SHOWNORMAL);
	UpdateWindow(hdlg);
}

void qdlg_scales_dlg_init(HWND hdlg)
{
	SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_RESETCONTENT,WPARAM(0),LPARAM(0));

	char str[256];
	const std::vector<float>& scales = qda_ED -> animation() -> scales();

	for(int i = 0; i < scales.size(); i++){
		sprintf(str,"%.3f",scales[i]);
		SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_ADDSTRING,WPARAM(0),LPARAM(str));
	}
}

void qdlg_scales_dlg_finit(HWND hdlg)
{
	qda_ED -> animation() -> clear_scales();

	char str[256];
	int count = SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_GETCOUNT,WPARAM(0),LPARAM(0));
	for(int i = 0; i < count; i++){
		int len = SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_GETTEXTLEN,WPARAM(i),LPARAM(0));
		if(len < 256){
			SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_GETTEXT,WPARAM(i),LPARAM(str));
			float value = atof(str);
			qda_ED -> animation() -> add_scale(value);
		}
	}
}

void qdlg_reset_scales_list(HWND hdlg)
{
	int count = SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_GETCOUNT,WPARAM(0),LPARAM(0));
	if(!count)
		return;

	std::vector<float> scales;
	scales.reserve(count);

	char str[256];
	for(int i = 0; i < count; i++){
		int len = SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_GETTEXTLEN,WPARAM(i),LPARAM(0));
		if(len < 256){
			SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_GETTEXT,WPARAM(i),LPARAM(str));
			float value = atof(str);
			std::vector<float>::const_iterator it = std::find(scales.begin(),scales.end(),value);
			if(it == scales.end())
				scales.push_back(value);
		}
	}

	SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_RESETCONTENT,WPARAM(0),LPARAM(0));

	for(int i = 0; i < scales.size(); i++){
		sprintf(str,"%.3f",scales[i]);
		SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_ADDSTRING,WPARAM(0),LPARAM(str));
	}
}

BOOL APIENTRY qdlg_scales_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int wmID,idx;

	switch(msg){
		case WM_INITDIALOG:
			qdlg_scales_dlg_init(hdlg);
			return 1;
		case WM_COMMAND:
			wmID = LOWORD(wParam);
			switch(wmID){
				case IDOK:
					EndDialog(hdlg,1);
					qdlg_scales_dlg_finit(hdlg);
					return 1;
				case IDC_ADD_SCALE: {
						char str[256];
						SendDlgItemMessage(hdlg,IDC_SCALE_EDIT,WM_GETTEXT,WPARAM(256),LPARAM(str));
						float val = atof(str);
						if(val < 0.01f) val = 0.01f;
						else if(val > 100.0f) val = 100.0f;
						if(fabs(val - 1.0f) > 0.001f){
							sprintf(str,"%.3f",val);
							SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_ADDSTRING,WPARAM(0),LPARAM(str));
							qdlg_reset_scales_list(hdlg);
						}
					}
					return 1;
				case IDC_REMOVE_SCALE:
					idx = SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_GETCURSEL,WPARAM(0),LPARAM(0));
					if(idx != LB_ERR)
						SendDlgItemMessage(hdlg,IDC_SCALES_LIST,LB_DELETESTRING,WPARAM(idx),LPARAM(0));
					return 1;
				case IDCANCEL:
					EndDialog(hdlg,0);
					return 1;
			}
			break;
	}
	return 0;
}

BOOL APIENTRY qdlg_compression_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int wmID;

	static grTileCompressionMethod compression = TILE_COMPRESS_LZ77;

	switch(msg){
		case WM_INITDIALOG:
			SetDlgItemInt(hdlg, IDC_EDIT_TOLERANCE, grTileSprite::comprasionTolerance(), TRUE);
			switch(compression){
			case TILE_UNCOMPRESSED:
				CheckDlgButton(hdlg, IDC_RADIO_UNCOMPRESSED, BST_CHECKED);
				break;
			case TILE_COMPRESS_RLE:
				CheckDlgButton(hdlg, IDC_RADIO_RLE, BST_CHECKED);
				break;
			case TILE_COMPRESS_LZ77:
				CheckDlgButton(hdlg, IDC_RADIO_LZ77, BST_CHECKED);
				break;
			}
			return 1;
		case WM_COMMAND:
			wmID = LOWORD(wParam);
			switch(wmID){
				case IDOK:
					if(HWND hdlg_p = CreateDialog(GetModuleHandle(0),MAKEINTRESOURCE(IDD_PROGRESS_DLG),hdlg,(DLGPROC)0)){
						int tolerance = clamp(GetDlgItemInt(hdlg, IDC_EDIT_TOLERANCE, 0, TRUE), 0, 255);

						if(IsDlgButtonChecked(hdlg, IDC_RADIO_UNCOMPRESSED) & BST_CHECKED)
							compression = TILE_UNCOMPRESSED;
						else if(IsDlgButtonChecked(hdlg, IDC_RADIO_RLE) & BST_CHECKED)
							compression = TILE_COMPRESS_RLE;
						else if(IsDlgButtonChecked(hdlg, IDC_RADIO_LZ77) & BST_CHECKED)
							compression = TILE_COMPRESS_LZ77;

						grTileAnimation::setProgressHandler(qdlg_show_progress, hdlg_p);
						qda_ED->animation()->tileCompress(compression, tolerance);
						qda_ED->toggle_changed(hDlg,1);
						grTileAnimation::setProgressHandler(0, 0);

						EndDialog(hdlg_p,0);
					}
					EndDialog(hdlg,1);
					return 1;
				case IDCANCEL:
					EndDialog(hdlg,0);
					return 1;
			}
			break;
	}
	return 0;
}
