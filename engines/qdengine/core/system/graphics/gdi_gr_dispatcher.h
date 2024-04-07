#ifndef __GDI_GR_DISPATCHER_H__
#define __GDI_GR_DISPATCHER_H__

#include "gr_dispatcher.h"

class GDI_grDispatcher : public grDispatcher
{
public:
	GDI_grDispatcher();
	~GDI_grDispatcher();

	bool Flush(int x,int y,int sx,int sy);
	bool StretchFlush(int x_dest,int y_dest,int sx_dest,int sy_dest,int x_src,int y_src,int sx_src,int sy_src);

	bool init(int sx,int sy,grPixelFormat pixel_format,void* hwnd,bool fullscreen = false);

	bool InitScreen(int sx,int sy);
	bool Finit();

	bool is_in_fullscreen_mode() const { return false; }
	bool is_mode_supported(int sx,int sy,grPixelFormat pixel_format) const { return true; }
	bool is_mode_supported(grPixelFormat pixel_format) const { return true; }

	bool palette_mode() const { return palette_mode_; }
	bool set_palette(const char* pal,int start_col,int pal_size);

private:
	
	void* dibPtr;
	void* dibHandle;

	bool palette_mode_;
	LOGPALETTE* sys_palette_;
	HPALETTE sys_pal_handle_;
	LOGPALETTE* palette_;
	HPALETTE pal_handle_;
};

#endif /* __GDI_GR_DISPATCHER_H__ */

