/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_setup.h"
#include "gr_dispatcher.h"

#ifndef __QD_SYSLIB__
#include "snd_dispatcher.h"
#include "qd_game_scene.h"
#include "plaympp_api.h"
#endif

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

const char* const qdGameConfig::ini_name_ = "qd_game.ini";
qdGameConfig qdGameConfig::config_;

bool enumerateIniSections(const char* fname,std::list<std::string>& section_list)
{
	static char buf[4096];
	static char path[_MAX_PATH];

	if(_fullpath(path,fname,_MAX_PATH) == NULL)
		return false;

	int sz = GetPrivateProfileSectionNames(buf,4096,path);
	if(!sz) return false;

	char* str = buf;
	while(strlen(str)){
		section_list.push_back(str);
		str += strlen(str) + 1;
	}

	return true;
}

char* getIniKey(const char* fname,const char* section,const char* key)
{
	static char buf[256];
	static char path[_MAX_PATH];

	if(_fullpath(path,fname,_MAX_PATH) == NULL){
		*buf = 0;
	}
	else {
		if(!GetPrivateProfileString(section,key,NULL,buf,256,path))
			*buf = 0;
	}

	return buf;
}

void putIniKey(const char* fname,const char* section,const char* key,int val)
{
	static XBuffer str;

	str.init();
	str <= val;

	putIniKey(fname,section,key,str.c_str());
}

void putIniKey(const char* fname,const char* section,const char* key,const char* val)
{
	static char path[_MAX_PATH];

	if(_fullpath(path,fname,_MAX_PATH) == NULL) return; 

	WritePrivateProfileString(section,key,val,path);
}

qdGameConfig::qdGameConfig()
{
	screen_sx_ = 640;
	screen_sy_ = 480;

	bits_per_pixel_ = 2;
	debug_draw_ = false;
	debug_show_grid_ = false;
	fullscreen_ = true;
	triggers_debug_ = false;

	driver_id_ = 1;
	show_fps_ = false;
	force_full_redraw_ = false;

	enable_sound_ = true;
	sound_volume_ = 255;
	enable_music_ = true;
	music_volume_ = 255;

	logic_period_ = 25;
	logic_synchro_by_clock_ = 1;
	game_speed_ = 1.0f;

	is_splash_enabled_ = true;
	splash_time_ = 3000;

	enable_profiler_ = false;

	locale_ = "Russian";

	minigame_read_only_ini_ = false;
}

void qdGameConfig::set_pixel_format(int pf)
{
	switch(pf){
		case GR_ARGB1555:
			bits_per_pixel_ = 0;
			break;
		case GR_RGB565:
			bits_per_pixel_ = 1;
			break;
		case GR_RGB888:
			bits_per_pixel_ = 2;
			break;
		case GR_ARGB8888:
			bits_per_pixel_ = 3;
			break;
	}
}

int qdGameConfig::bits_per_pixel() const
{
	switch(bits_per_pixel_){
		case 0:
			return 15;
		case 1:
			return 16;
		case 2:
			return 24;
		case 3:
			return 32;
	}

	return 24;
}

void qdGameConfig::set_bits_per_pixel(int bpp)
{
	switch(bpp){
		case 15:
			bits_per_pixel_ = 0;
			break;
		case 16:
			bits_per_pixel_ = 1;
			break;
		case 24:
			bits_per_pixel_ = 2;
			break;
		case 32:
			bits_per_pixel_ = 3;
			break;
	}
}

int qdGameConfig::pixel_format() const
{
	switch(bits_per_pixel()){
		case 15:
			return GR_ARGB1555;
		case 16:
			return GR_RGB565;
		case 24:
			return GR_RGB888;
		case 32:
			return GR_ARGB8888;
	}

	return GR_RGB888;
}

void qdGameConfig::load()
{
	char* p = 0;
#ifndef _QUEST_EDITOR
	p = getIniKey(ini_name_,"graphics","color_depth");
	if(strlen(p)) bits_per_pixel_ = atoi(p);

	p = getIniKey(ini_name_,"graphics","fullscreen");
	if(strlen(p)) fullscreen_ = (atoi(p)) > 0;

	p = getIniKey(ini_name_,"graphics","driver");
	if(strlen(p)) driver_id_ = atoi(p);

	p = getIniKey(ini_name_,"game","logic_period");
	if(strlen(p)) logic_period_ = atoi(p);

	p = getIniKey(ini_name_,"game","synchro_by_clock");
	if(strlen(p)) logic_synchro_by_clock_ = atoi(p);

	p = getIniKey(ini_name_,"game","game_speed");
	if(strlen(p)) game_speed_ = atof(p);

	p = getIniKey(ini_name_,"game","locale");
	if(strlen(p)) locale_ = p;

#ifndef __QD_SYSLIB__
	p = getIniKey(ini_name_,"game","fps_period");
	if(strlen(p)) qdGameScene::fps_counter().set_period(atoi(p));
#endif

	if(atoi(getIniKey(ini_name_,"debug","enable_log")))
		appLog::default_log().enable();

	if(atoi(getIniKey(ini_name_,"debug","full_redraw")))
		force_full_redraw_ = true;

	if(atoi(getIniKey(ini_name_,"debug","enable_profiler")))
		enable_profiler_ = true;

	p = getIniKey(ini_name_,"debug","triggers_log_file");
	if (strlen(p)) profiler_file_ = p;

	if(atoi(getIniKey(ini_name_,"debug","triggers_debug")))
		triggers_debug_ = true;

	p = getIniKey(ini_name_,"sound","enable_sound");
	if(strlen(p)) enable_sound_ = (atoi(p) > 0);

	p = getIniKey(ini_name_,"sound","sound_volume");
	if(strlen(p)) sound_volume_ = atoi(p);

	p = getIniKey(ini_name_,"sound","enable_music");
	if(strlen(p)) enable_music_ = (atoi(p) > 0);

	p = getIniKey(ini_name_,"sound","music_volume");
	if(strlen(p)) music_volume_ = atoi(p);
#endif

	p = getIniKey(ini_name_, "minigame", "read_only_ini");
	if(strlen(p)) minigame_read_only_ini_ = (atoi(p) > 0);
}

void qdGameConfig::save()
{
#ifndef _QUEST_EDITOR
	putIniKey(ini_name_,"graphics","color_depth",bits_per_pixel_);
	putIniKey(ini_name_,"graphics","fullscreen",fullscreen_);
	putIniKey(ini_name_,"graphics","driver",driver_id_);

	putIniKey(ini_name_,"sound","enable_sound",enable_sound_);
	putIniKey(ini_name_,"sound","sound_volume",sound_volume_);
	putIniKey(ini_name_,"sound","enable_music",enable_music_);
	putIniKey(ini_name_,"sound","music_volume",music_volume_);
#endif
}

bool qdGameConfig::update_sound_settings() const
{
#ifndef __QD_SYSLIB__
	if(sndDispatcher* dp = sndDispatcher::get_dispatcher()){
		dp -> set_volume(sound_volume_);
		if(enable_sound_) dp -> enable();
		else dp -> disable();

		return true;
	}
#endif
	return false;
}

bool qdGameConfig::update_music_settings() const
{
#ifndef __QD_SYSLIB__
	mpegPlayer::instance().set_volume(music_volume_);
	if(enable_music_) mpegPlayer::instance().enable();
	else mpegPlayer::instance().disable();
#endif
	return true;
}
