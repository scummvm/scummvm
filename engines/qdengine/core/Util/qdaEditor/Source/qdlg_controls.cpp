/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <commctrl.h>
#include <stdlib.h>

#include "qdlg_controls.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

void qdlg_status_string(const char* str);

/* --------------------------- DEFINITION SECTION --------------------------- */

void qdlgControl::update(void)
{
	int x,y,sx,sy,dsx,dsy;
	HWND hdlg = (HWND)hParent;

	RECT rc;
	GetClientRect(hdlg,&rc);
	dsx = rc.right - rc.left;
	dsy = rc.bottom - rc.top;

	if(size_x_ == QDLG_SIZE_SCALE)
		sx = dsx - (dlg_sx_ - sx_);
	else
		sx = sx_;

	switch(align_x_){
		case QDLG_ALIGN_LEFT:
			x = x_;
			break;
		case QDLG_ALIGN_RIGHT:
			x = dsx - sx - (dlg_sx_ - sx_ - x_);
			break;
		case QDLG_ALIGN_CENTER:
			x = (dsx - sx)/2;
			break;
	}

	if(size_y_ == QDLG_SIZE_SCALE)
		sy = dsy - (dlg_sy_ - sy_);
	else
		sy = sy_;

	switch(align_y_){
		case QDLG_ALIGN_LEFT:
			y = y_;
			break;
		case QDLG_ALIGN_RIGHT:
			y = dsy - sy - (dlg_sy_ - sy_ - y_);
			break;
		case QDLG_ALIGN_CENTER:
			y = (dsy - sy)/2;
			break;
	}

	SetWindowPos((HWND)hWnd,0,x,y,sx,sy,SWP_NOZORDER | SWP_NOCOPYBITS);

	x_cur_ = x;
	y_cur_ = y;

	sx_cur_ = sx;
	sy_cur_ = sy;
}

qdlgControl::qdlgControl(void* hw,int id,int ax,int ay,int sx,int sy,const char* str)
{
	hParent = hw;
	ctlID = id;

	align_x_ = ax;
	align_y_ = ay;

	size_x_ = sx;
	size_y_ = sy;

	RECT rc;
	HWND hdlg = (HWND)hParent;
	hWnd = GetDlgItem(hdlg,ctlID);

	GetWindowRect((HWND)hWnd,&rc);

	x_ = rc.left;
	y_ = rc.top;

	sx_ = sx_cur_ = rc.right - rc.left;
	sy_ = sy_cur_ = rc.bottom - rc.top;

	GetClientRect(hdlg,&rc);

	dlg_sx_ = rc.right - rc.left;
	dlg_sy_ = rc.bottom - rc.top;

	POINT pt = { x_,y_ };
	ScreenToClient(hdlg,&pt);

	x_ = x_cur_ = pt.x;
	y_ = y_cur_ = pt.y;

	if(str){
		comment_ = new char[strlen(str) + 1];
		strcpy(comment_,str);
	}
	else
		comment_ = 0;
}

qdlgControl::~qdlgControl(void)
{
	if(comment_) delete comment_;
}

