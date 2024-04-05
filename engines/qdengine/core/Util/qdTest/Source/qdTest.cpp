
#include "qd_precomp.h"

#include "gdi_gr_dispatcher.h"
#include "qd_animation.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	grDispatcher::sys_init();

	GDI_grDispatcher* gr_D = new GDI_grDispatcher;

	grPixelFormat mode = GR_RGB565;
//	mode = GR_ARGB1555;
///	mode = GR_RGB888;
//	mode = GR_ARGB8888;
	if(!grDispatcher::instance() -> init(800,600,mode,0,false))
		return 1;

	grDispatcher::instance()->Flush();

	grDispatcher::instance()->set_default_font(grDispatcher::instance()->load_font("font00"));
/*
	qdSprite spr;
	spr.load("test.tga");
	spr.compress();
	spr.convert(mode);

	const int test_count = 10000;
	int start_time = xclock();

	for(int i = 0; i < test_count; i++)
		spr.redraw(200, 300, 0);

	int total_time = xclock() - start_time;

	XBuffer str;
	str < "draw_time = " <= total_time;

	grDispatcher::instance()->DrawAlignedText(10,10,0,0,0xFFFFFF,str);

	grDispatcher::instance()->Flush();
*/

	qdAnimation anm;
	anm.qda_load("test.qda");
//	anm.convert(mode);

	qdSprite spr;
	spr.load("test.tga");
//	spr.convert(mode);

	int flip_mode = 0;
//	flip_mode |= GR_FLIP_HORIZONTAL;
//	flip_mode |= GR_FLIP_VERTICAL;

	float angle = G2R(45.f);

	const int test_count = 10000;
	int start_time = xclock();
//	for(int i = 0; i < test_count; i++)
//		spr.redraw_rot(200, 300, 0, angle, Vect2f(0.7f, 0.5f), flip_mode);
//		spr.redraw_rot(200, 300, 0, angle, Vect2f(0.7f, 0.5f), flip_mode);
//		spr.redraw(200, 300, 0, flip_mode);
//		anm.redraw(200, 300, 0, flip_mode);

	int total_time = xclock() - start_time;

	XBuffer str;
	str < "draw_time = " <= total_time;
//	grDispatcher::instance()->DrawAlignedText(10,10,0,0,0xFFFFFF,str);

	const unsigned mask_color = grDispatcher::instance()->make_rgb888(255, 255, 255);
	const int mask_alpha = 160;

	grDispatcher::instance()->Fill(0xFFFFFF);

//	spr.redraw(200, 300, 0, flip_mode);
	anm.redraw(200, 300, 0, flip_mode);
//	anm.draw_mask(200, 300, 0, mask_color, mask_alpha, flip_mode);

	anm.redraw_rot(600, 300, 0, 0.f, flip_mode);
//	anm.draw_mask_rot(600, 300, 0, 0.f, mask_color, mask_alpha, flip_mode);

//	anm.redraw_rot(200, 300, 0, angle, flip_mode);
//	anm.draw_mask_rot(200, 300, 0, angle, mask_color, mask_alpha, flip_mode);
//	anm.redraw_rot(600, 300, 0, angle, Vect2f(0.7f, 0.7f), flip_mode ^ GR_FLIP_VERTICAL);
//	anm.draw_mask_rot(600, 300, 0, angle, mask_color, mask_alpha, Vect2f(0.7f, 0.7f), flip_mode ^ GR_FLIP_VERTICAL);
//	spr.redraw_rot(200, 300, 0, angle, flip_mode);

	grDispatcher::instance()->Flush();

	MSG msg;
	while(GetMessage(&msg,NULL,0,0)){ 
		switch(msg.message){
			case WM_KEYDOWN:
				if(msg.wParam == VK_LEFT){
					angle = cycleAngle(angle + G2R(1.f));
					grDispatcher::instance()->Fill(0);
					anm.redraw_rot(200, 300, 0, angle, flip_mode);
					anm.draw_mask_rot(200, 300, 0, angle, mask_color, mask_alpha, flip_mode);
					anm.redraw_rot(600, 300, 0, angle, Vect2f(0.7f, 0.7f), flip_mode ^ GR_FLIP_VERTICAL);
					anm.draw_mask_rot(600, 300, 0, angle, mask_color, mask_alpha, Vect2f(0.7f, 0.7f), flip_mode ^ GR_FLIP_VERTICAL);
					grDispatcher::instance()->Flush();
				}
				else if(msg.wParam == VK_RIGHT){
					angle = cycleAngle(angle - G2R(1.f));
					grDispatcher::instance()->Fill(0);
					anm.redraw_rot(200, 300, 0, angle, flip_mode);
					anm.draw_mask_rot(200, 300, 0, angle, mask_color, mask_alpha, flip_mode);
					anm.redraw_rot(600, 300, 0, angle, Vect2f(0.7f, 0.7f), flip_mode ^ GR_FLIP_VERTICAL);
					anm.draw_mask_rot(600, 300, 0, angle, mask_color, mask_alpha, Vect2f(0.7f, 0.7f), flip_mode ^ GR_FLIP_VERTICAL);
					grDispatcher::instance()->Flush();
				}
				else if(msg.wParam == VK_ESCAPE)
					PostQuitMessage(0);
				break;
		}
		DispatchMessage(&msg); 
	} 

	delete gr_D;
	grDispatcher::sys_finit();

	return 0;
}
