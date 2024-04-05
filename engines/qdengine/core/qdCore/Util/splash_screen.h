#ifndef __SPLASH_SCREEN_H__
#define __SPLASH_SCREEN_H__

class SplashScreen
{
public:
	SplashScreen() : splash_hwnd_(NULL), bitmap_handle_(NULL), start_time_(0) { }
	~SplashScreen(){ destroy(); }

	bool create(int bitmap_resid);
	bool create(const char* bitmap_file);

	bool set_mask(int mask_resid);
	bool set_mask(const char* mask_file);

	bool destroy();

	void show();
	void hide();

	void wait(int time);

private:

	int start_time_;

	void* splash_hwnd_;
	void* bitmap_handle_;

	bool create_window();
	void apply_mask(void* mask_handle);
};

#endif // __SPLASH_SCREEN_H__

