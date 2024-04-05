#ifndef __QD_SETUP_H__
#define __QD_SETUP_H__

//! Настройки.
class qdGameConfig
{
public:
	qdGameConfig();
	~qdGameConfig(){ }

	int screen_sx() const { return screen_sx_; }
	int screen_sy() const { return screen_sy_; }

	int driver_ID() const { return driver_id_; }
	void set_driver_ID(int id){ driver_id_ = id; }

	void set_screen_size(int sx,int sy){ screen_sx_ = sx; screen_sy_ = sy; }

	int pixel_format() const;
	void set_pixel_format(int pf);

	int bits_per_pixel() const;
	void set_bits_per_pixel(int bpp);

	bool debug_draw() const { return debug_draw_; }
	void toggle_debug_draw(){ debug_draw_ = !debug_draw_; }
	bool debug_show_grid() const { return debug_show_grid_; }
	void toggle_show_grid(){ debug_show_grid_ = !debug_show_grid_; }

	bool force_full_redraw() const { return force_full_redraw_; }
	void toggle_full_redraw(){ force_full_redraw_ = !force_full_redraw_; }

	bool fullscreen() const { return fullscreen_; }
	void toggle_fullscreen(){ fullscreen_ = !fullscreen_; }

	const char* locale() const { return locale_.c_str(); }

	void load();
	void save();

	static qdGameConfig& get_config(){ return config_; }
	static void set_config(const qdGameConfig& s){ config_ = s; }

	static const char* const ini_name(){ return ini_name_; }

	bool triggers_debug() const { return triggers_debug_; }
	void toggle_triggers_debug(bool v){ triggers_debug_ = v; }

	bool show_fps() const { return show_fps_; }
	void toggle_fps(){ show_fps_ = !show_fps_; }

	bool is_sound_enabled() const { return enable_sound_; }
	void toggle_sound(bool state){ enable_sound_ = state; }
	unsigned int sound_volume() const { return sound_volume_; }
	void set_sound_volume(unsigned int vol){ sound_volume_ = vol; }
	bool update_sound_settings() const;
	
	bool is_music_enabled() const { return enable_music_; }
	void toggle_music(bool state){ enable_music_ = state; }
	unsigned int music_volume() const { return music_volume_; }
	void set_music_volume(unsigned int vol){ music_volume_ = vol; }
	bool update_music_settings() const;

	int logic_period() const { return logic_period_; }
	int logic_synchro_by_clock() const { return logic_synchro_by_clock_; }

	float game_speed() const { return game_speed_; }
	void set_game_speed(float speed){ game_speed_ = speed; }

	bool is_splash_enabled() const { return is_splash_enabled_; }
	int splash_time() const { return splash_time_; }

	bool is_profiler_enabled() const { return enable_profiler_; }
	void toggle_profiler(bool state) { enable_profiler_ = state; }
	const char* profiler_file() const { return profiler_file_.c_str(); }
	void set_profiler_file(const char* fname) { profiler_file_ = fname; }

	bool minigame_read_only_ini() const { return minigame_read_only_ini_; }

private:

	int bits_per_pixel_;
	bool fullscreen_;

	int driver_id_;

	int screen_sx_;
	int screen_sy_;

	bool enable_sound_;
	unsigned int sound_volume_;

	bool enable_music_;
	unsigned int music_volume_;

	bool debug_draw_;
	bool debug_show_grid_;

	bool triggers_debug_;
	bool show_fps_;
	bool force_full_redraw_;

	int logic_period_;
	int logic_synchro_by_clock_;
	float game_speed_;

	bool is_splash_enabled_;
	int splash_time_;

	bool enable_profiler_;
	std::string profiler_file_;

	std::string locale_;

	bool minigame_read_only_ini_;

	static qdGameConfig config_;
	static const char* const ini_name_;
};

char* getIniKey(const char* fname,const char* section,const char* key);
void putIniKey(const char* fname,const char* section,const char* key,int val);
void putIniKey(const char* fname,const char* section,const char* key,const char* val);
bool enumerateIniSections(const char* fname,std::list<std::string>& section_list);

#endif /* __QD_SETUP_H__ */

