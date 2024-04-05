#ifndef __QDLG_CONTROLS_H__
#define __QDLG_CONTROLS_H__

enum qdlgControAlign
{
	QDLG_ALIGN_LEFT,
	QDLG_ALIGN_RIGHT,
	QDLG_ALIGN_CENTER
};

enum qdlgControlSize
{
	QDLG_SIZE_ORIGINAL,
	QDLG_SIZE_SCALE
};

class qdlgControl
{
	int ctlID;
	void* hParent;
	void* hWnd;

	int align_x_;
	int align_y_;

	int size_x_;
	int size_y_;

	int x_;
	int y_;
	int sx_;
	int sy_;
	int dlg_sx_;
	int dlg_sy_;

	int x_cur_;
	int y_cur_;
	int sx_cur_;
	int sy_cur_;

	char* comment_;

public:

	void update(void);

	int x(void) const { return x_cur_; }
	int y(void) const { return y_cur_; }
	int sx(void) const { return sx_cur_; }
	int sy(void) const { return sy_cur_; }

	int control_ID(void) const { return ctlID; }

	char* comment(void){ return comment_; }

	int hit(int x,int y){ 
		if(x >= x_cur_ && x < x_cur_ + sx_cur_ && y >= y_cur_ && y < y_cur_ + sy_cur_) 
			return 1;
		return 0;
	}

	void* hwnd(void){ return hWnd; }

	qdlgControl(void* hdlg,int id,int ax,int ay,int sx,int sy,const char* str = 0);
	~qdlgControl(void);
};

typedef std::list<qdlgControl*> qdlgControlList;

#endif /* __QDLG_CONTROLS_H__ */

