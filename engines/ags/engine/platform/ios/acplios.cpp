//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "core/platform.h"

#if AGS_PLATFORM_OS_IOS

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h> 
#include <ctype.h>

#include <allegro.h>
#include "platform/base/agsplatformdriver.h"
#include "ac/runtime_defines.h"
#include "main/config.h"
#include "plugin/agsplugin.h"
#include "util/string_utils.h"

using namespace AGS::Common;

#define IOS_CONFIG_FILENAME "ios.cfg"

extern char* ios_document_directory;

bool ReadConfiguration(char* filename, bool read_everything);
void ResetConfiguration();

//int psp_return_to_menu = 1;
int psp_ignore_acsetup_cfg_file = 1;
int psp_clear_cache_on_room_change = 0;
int psp_rotation = 0;
int psp_config_enabled = 0;
char psp_translation[100];
char* psp_translations[100];

// Mouse option from Allegro.
extern int config_mouse_control_mode;


// Graphic options from the Allegro library.
extern int psp_gfx_scaling;
extern int psp_gfx_smoothing;


// Audio options from the Allegro library.
unsigned int psp_audio_samplerate = 44100;
int psp_audio_enabled = 1;
volatile int psp_audio_multithreaded = 1;
int psp_audio_cachesize = 10;
int psp_midi_enabled = 1;
int psp_midi_preload_patches = 0;

int psp_video_framedrop = 0;

int psp_gfx_renderer = 0;
int psp_gfx_super_sampling = 0;
int psp_gfx_smooth_sprites = 0;

int psp_debug_write_to_logcat = 0;

int config_mouse_longclick = 0;

extern int display_fps;
extern int want_exit;
extern void PauseGame();
extern void UnPauseGame();
extern int main(int argc,char*argv[]);

char psp_game_file_name[256];
char* psp_game_file_name_pointer = psp_game_file_name;

bool psp_load_latest_savegame = false;
extern char saveGameDirectory[260];
extern const char *loadSaveGameOnStartup;
char lastSaveGameName[200];

bool reset_configuration = false;

const int CONFIG_IGNORE_ACSETUP = 0;
const int CONFIG_CLEAR_CACHE = 1;
const int CONFIG_AUDIO_RATE = 2;
const int CONFIG_AUDIO_ENABLED = 3;
const int CONFIG_AUDIO_THREADED = 4;
const int CONFIG_AUDIO_CACHESIZE = 5;
const int CONFIG_MIDI_ENABLED = 6;
const int CONFIG_MIDI_PRELOAD = 7;
const int CONFIG_VIDEO_FRAMEDROP = 8;
const int CONFIG_GFX_RENDERER = 9;
const int CONFIG_GFX_SMOOTHING = 10;
const int CONFIG_GFX_SCALING = 11;
const int CONFIG_GFX_SS = 12;
const int CONFIG_ROTATION = 13;
const int CONFIG_ENABLED = 14;
const int CONFIG_DEBUG_FPS = 15;
const int CONFIG_GFX_SMOOTH_SPRITES = 16;
const int CONFIG_TRANSLATION = 17;
const int CONFIG_DEBUG_LOGCAT = 18;
const int CONFIG_MOUSE_METHOD = 19;
const int CONFIG_MOUSE_LONGCLICK = 20;



struct AGSIOS : AGSPlatformDriver {

  virtual int  CDPlayerCommand(int cmdd, int datt);
  virtual void Delay(int millis);
  virtual void DisplayAlert(const char*, ...);
  virtual const char *GetAppOutputDirectory();
  virtual unsigned long GetDiskFreeSpaceMB();
  virtual const char* GetNoMouseErrorString();
  virtual bool IsBackendResponsibleForMouseScaling() { return true; }
  virtual eScriptSystemOSID GetSystemOSID();
  virtual int  InitializeCDPlayer();
  virtual void PostAllegroExit();
  virtual void SetGameWindowIcon();
  virtual void ShutdownCDPlayer();
};



bool readConfigFile(char* directory)
{
  chdir(directory);

  ResetConfiguration();

  return ReadConfiguration(IOS_CONFIG_FILENAME, true);
}


bool writeConfigFile()
{
  FILE* config = fopen(IOS_CONFIG_FILENAME, "wb");
  if (config)
  {
    fprintf(config, "[misc]\n");
    fprintf(config, "config_enabled = %d\n", psp_config_enabled);
    fprintf(config, "rotation = %d\n", psp_rotation);
    fprintf(config, "translation = %s\n", psp_translation);

    fprintf(config, "[controls]\n");
    fprintf(config, "mouse_method = %d\n", config_mouse_control_mode);
    fprintf(config, "mouse_longclick = %d\n", config_mouse_longclick);
	
    fprintf(config, "[compatibility]\n");
    fprintf(config, "clear_cache_on_room_change = %d\n", psp_clear_cache_on_room_change);

    fprintf(config, "[sound]\n");
    fprintf(config, "samplerate = %d\n", psp_audio_samplerate );
    fprintf(config, "enabled = %d\n", psp_audio_enabled);
    fprintf(config, "threaded = %d\n", psp_audio_multithreaded);
    fprintf(config, "cache_size = %d\n", psp_audio_cachesize);
    
    fprintf(config, "[midi]\n");
    fprintf(config, "enabled = %d\n", psp_midi_enabled);
    fprintf(config, "preload_patches = %d\n", psp_midi_preload_patches);

    fprintf(config, "[video]\n");
    fprintf(config, "framedrop = %d\n", psp_video_framedrop);

    fprintf(config, "[graphics]\n");
    fprintf(config, "renderer = %d\n", psp_gfx_renderer);
    fprintf(config, "smoothing = %d\n", psp_gfx_smoothing);
    fprintf(config, "scaling = %d\n", psp_gfx_scaling);
    fprintf(config, "super_sampling = %d\n", psp_gfx_super_sampling);
    fprintf(config, "smooth_sprites = %d\n", psp_gfx_smooth_sprites);

    fprintf(config, "[debug]\n");
    fprintf(config, "show_fps = %d\n", (display_fps == 2) ? 1 : 0);
    fprintf(config, "logging = %d\n", psp_debug_write_to_logcat);

    fclose(config);

    return true;
  }

  return false;
}


int readIntConfigValue(int id)
{
  switch (id)
  {
    case CONFIG_IGNORE_ACSETUP:
      return psp_ignore_acsetup_cfg_file;
      break;
    case CONFIG_CLEAR_CACHE:
      return psp_clear_cache_on_room_change;
      break;
    case CONFIG_AUDIO_RATE:
      return psp_audio_samplerate;
      break;
    case CONFIG_AUDIO_ENABLED:
      return psp_audio_enabled;
      break;
    case CONFIG_AUDIO_THREADED:
      return psp_audio_multithreaded;
      break;
    case CONFIG_AUDIO_CACHESIZE:
      return psp_audio_cachesize;
      break;
    case CONFIG_MIDI_ENABLED:
      return psp_midi_enabled;
      break;
    case CONFIG_MIDI_PRELOAD:
      return psp_midi_preload_patches;
      break;
    case CONFIG_VIDEO_FRAMEDROP:
      return psp_video_framedrop;
      break;
    case CONFIG_GFX_RENDERER:
      return psp_gfx_renderer;
      break;
    case CONFIG_GFX_SMOOTHING:
      return psp_gfx_smoothing;
      break;
    case CONFIG_GFX_SCALING:
      return psp_gfx_scaling;
      break;
    case CONFIG_GFX_SS:
      return psp_gfx_super_sampling;
      break;
    case CONFIG_GFX_SMOOTH_SPRITES:
      return psp_gfx_smooth_sprites;
      break;
    case CONFIG_ROTATION:
      return psp_rotation;
      break;
    case CONFIG_ENABLED:
      return psp_config_enabled;
      break;
    case CONFIG_DEBUG_FPS:
      return (display_fps == 2) ? 1 : 0;
      break;
    case CONFIG_DEBUG_LOGCAT:
      return psp_debug_write_to_logcat;
      break;
    case CONFIG_MOUSE_METHOD:
      return config_mouse_control_mode;
      break;
    case CONFIG_MOUSE_LONGCLICK:
      return config_mouse_longclick;
      break;
    default:
      return 0;
      break;
  }
}


char* readStringConfigValue(int id)
{
  switch (id)
  {
    case CONFIG_TRANSLATION:
      return &psp_translation[0];
      break;
  }
}


void setIntConfigValue(int id, int value)
{
  switch (id)
  {
    case CONFIG_IGNORE_ACSETUP:
      psp_ignore_acsetup_cfg_file = value;
      break;
    case CONFIG_CLEAR_CACHE:
      psp_clear_cache_on_room_change = value;
      break;
    case CONFIG_AUDIO_RATE:
      psp_audio_samplerate = value;
      break;
    case CONFIG_AUDIO_ENABLED:
      psp_audio_enabled = value;
      break;
    case CONFIG_AUDIO_THREADED:
      psp_audio_multithreaded = value;
      break;
    case CONFIG_AUDIO_CACHESIZE:
      psp_audio_cachesize = value;
      break;
    case CONFIG_MIDI_ENABLED:
      psp_midi_enabled = value;
      break;
    case CONFIG_MIDI_PRELOAD:
      psp_midi_preload_patches = value;
      break;
    case CONFIG_VIDEO_FRAMEDROP:
      psp_video_framedrop = value;
      break;
    case CONFIG_GFX_RENDERER:
      psp_gfx_renderer = value;
      break;
    case CONFIG_GFX_SMOOTHING:
      psp_gfx_smoothing = value;
      break;
    case CONFIG_GFX_SCALING:
      psp_gfx_scaling = value;
      break;
    case CONFIG_GFX_SS:
      psp_gfx_super_sampling = value;
      break;
    case CONFIG_GFX_SMOOTH_SPRITES:
      psp_gfx_smooth_sprites = value;
      break;
    case CONFIG_ROTATION:
      psp_rotation = value;
      break;
    case CONFIG_ENABLED:
      psp_config_enabled = value;
      break;
    case CONFIG_DEBUG_FPS:
      display_fps = (value == 1) ? 2 : 0;
      break;
    case CONFIG_DEBUG_LOGCAT:
      psp_debug_write_to_logcat = value;
      break;
    case CONFIG_MOUSE_METHOD:
      config_mouse_control_mode = value;
      break;
    case CONFIG_MOUSE_LONGCLICK:
      config_mouse_longclick = value;
      break;
    default:
      break;
  }
}


void setStringConfigValue(int id, char* value)
{
  switch (id)
  {
    case CONFIG_TRANSLATION:
      strcpy(psp_translation, value);
      break;
    default:
      break;
  }
}

/*
int getAvailableTranslations(char* translations)
{
  int i = 0;
  int length;
  DIR* dir;
  struct dirent* entry;
  char buffer[200];

  dir = opendir(".");
  if (dir)
  {
    while ((entry = readdir(dir)) != 0)
    {
      length = strlen(entry->d_name);
      if (length > 4)
      {
        if (ags_stricmp(&entry->d_name[length - 4], ".tra") == 0)
        {
          memset(buffer, 0, 200);
          strncpy(buffer, entry->d_name, length - 4);
          psp_translations[i] = (char*)malloc(strlen(buffer) + 1);
          strcpy(psp_translations[i], buffer);
          env->SetObjectArrayElement(translations, i, env->NewStringUTF(&buffer[0]));
          i++;
        }
      }
    }
    closedir(dir);
  }

  return i;
}
*/




void selectLatestSavegame()
{
  DIR* dir;
  struct dirent* entry;
  struct stat statBuffer;
  char buffer[200];
  time_t lastTime = 0;

  dir = opendir(saveGameDirectory);

  if (dir)
  {
    while ((entry = readdir(dir)) != 0)
    {
      if (ags_strnicmp(entry->d_name, "agssave", 7) == 0)
      {
        if (ags_stricmp(entry->d_name, "agssave.999") != 0)
        {
          strcpy(buffer, saveGameDirectory);
          strcat(buffer, entry->d_name);
          stat(buffer, &statBuffer);
          if (statBuffer.st_mtime > lastTime)
          {
            strcpy(lastSaveGameName, buffer);
            loadSaveGameOnStartup = lastSaveGameName;
            lastTime = statBuffer.st_mtime;
          }
        }
      }
    }
    closedir(dir);
  }
}


int ReadInteger(int* variable, const ConfigTree &cfg, char* section, char* name, int minimum, int maximum, int default_value)
{
  if (reset_configuration)
  {
    *variable = default_value;
    return 0;
  }

  int temp = INIreadint(cfg, section, name);

  if (temp == -1)
    return 0;

  if ((temp < minimum) || (temp > maximum))
    temp = default_value;

  *variable = temp;

  return 1;
}



int ReadString(char* variable, const ConfigTree &cfg, char* section, char* name, char* default_value)
{
  if (reset_configuration)
  {
    strcpy(variable, default_value);
    return 0;
  }

  String temp;
  if (!INIreaditem(cfg, section, name, temp))
    temp = default_value;

  strcpy(variable, temp);

  return 1;
}



void ResetConfiguration()
{
  reset_configuration = true;

  ReadConfiguration(IOS_CONFIG_FILENAME, true);

  reset_configuration = false;
}


bool ReadConfiguration(char* filename, bool read_everything)
{
  ConfigTree cfg;
  if (IniUtil::Read(filename, cfg) || reset_configuration)
  {
//    ReadInteger((int*)&psp_disable_powersaving, "misc", "disable_power_saving", 0, 1, 1);

//    ReadInteger((int*)&psp_return_to_menu, "misc", "return_to_menu", 0, 1, 1);

    ReadString(&psp_translation[0], cfg, "misc", "translation", "default");

    ReadInteger((int*)&psp_config_enabled, cfg, "misc", "config_enabled", 0, 1, 0);
    if (!psp_config_enabled && !read_everything)
      return true;

    ReadInteger(&psp_debug_write_to_logcat, cfg, "debug", "logging", 0, 1, 0);
    ReadInteger(&display_fps, cfg, "debug", "show_fps", 0, 1, 0);
    if (display_fps == 1)
      display_fps = 2;

    ReadInteger((int*)&psp_rotation, cfg, "misc", "rotation", 0, 2, 0);

//    ReadInteger((int*)&psp_ignore_acsetup_cfg_file, "compatibility", "ignore_acsetup_cfg_file", 0, 1, 0);
    ReadInteger((int*)&psp_clear_cache_on_room_change, cfg, "compatibility", "clear_cache_on_room_change", 0, 1, 0);

    ReadInteger((int*)&psp_audio_samplerate, cfg, "sound", "samplerate", 0, 44100, 44100);
    ReadInteger((int*)&psp_audio_enabled, cfg, "sound", "enabled", 0, 1, 1);
    ReadInteger((int*)&psp_audio_multithreaded, cfg, "sound", "threaded", 0, 1, 1);
    ReadInteger((int*)&psp_audio_cachesize, cfg, "sound", "cache_size", 1, 50, 10);

    ReadInteger((int*)&psp_midi_enabled, cfg, "midi", "enabled", 0, 1, 1);
    ReadInteger((int*)&psp_midi_preload_patches, cfg, "midi", "preload_patches", 0, 1, 0);

    ReadInteger((int*)&psp_video_framedrop, cfg, "video", "framedrop", 0, 1, 0);

    ReadInteger((int*)&psp_gfx_renderer, cfg, "graphics", "renderer", 0, 2, 0);
    ReadInteger((int*)&psp_gfx_smoothing, cfg, "graphics", "smoothing", 0, 1, 1);
    ReadInteger((int*)&psp_gfx_scaling, cfg, "graphics", "scaling", 0, 2, 1);
    ReadInteger((int*)&psp_gfx_super_sampling, cfg, "graphics", "super_sampling", 0, 1, 0);
    ReadInteger((int*)&psp_gfx_smooth_sprites, cfg, "graphics", "smooth_sprites", 0, 1, 0);

    ReadInteger((int*)&config_mouse_control_mode, cfg, "controls", "mouse_method", 0, 1, 0);
    ReadInteger((int*)&config_mouse_longclick, cfg, "controls", "mouse_longclick", 0, 1, 1);

    return true;
  }

  return false;
}



extern void ios_show_message_box(char* buffer);
volatile int ios_wait_for_ui = 0;


void startEngine(char* filename, char* directory, int loadLastSave)
{
  strcpy(psp_game_file_name, filename);

  // Get the base directory (usually "/sdcard/ags").
  chdir(directory);

  // Reset configuration.
  ResetConfiguration();

  // Read general configuration.
  ReadConfiguration(IOS_CONFIG_FILENAME, true);

  // Get the games path.
  char path[256];
  strcpy(path, psp_game_file_name);
  int lastindex = strlen(path) - 1;
  while (path[lastindex] != '/')
  {
    path[lastindex] = 0;
    lastindex--;
  }
  chdir(path);
  
  setenv("ULTRADIR", "..", 1);

  // Read game specific configuration.
  ReadConfiguration(IOS_CONFIG_FILENAME, false);

  psp_load_latest_savegame = loadLastSave;

  // Start the engine main function.
  main(1, &psp_game_file_name_pointer);
  
  // Explicitly quit here, otherwise the app will hang forever.
  exit(0);
}




int AGSIOS::CDPlayerCommand(int cmdd, int datt) {
  return 0;//cd_player_control(cmdd, datt);
}



void AGSIOS::DisplayAlert(const char *text, ...) {
  char displbuf[2000];
  va_list ap;
  va_start(ap, text);
  vsprintf(displbuf, text, ap);
  va_end(ap);
  printf("%s", displbuf);
  ios_show_message_box(displbuf);
  
  while (ios_wait_for_ui)
    usleep(200);
}

void AGSIOS::Delay(int millis) {
  usleep(millis);
}

unsigned long AGSIOS::GetDiskFreeSpaceMB() {
  // placeholder
  return 100;
}

const char* AGSIOS::GetNoMouseErrorString() {
  return "This game requires a mouse. You need to configure and setup your mouse to play this game.\n";
}

eScriptSystemOSID AGSIOS::GetSystemOSID() {
  return eOS_iOS;
}

int AGSIOS::InitializeCDPlayer() {
  return 0;//cd_player_init();
}

void AGSIOS::PostAllegroExit() {
  // do nothing
}

void AGSIOS::SetGameWindowIcon() {
  // do nothing
}



void AGSIOS::ShutdownCDPlayer() {
  //cd_exit();
}

const char *AGSIOS::GetAppOutputDirectory()
{
  return ios_document_directory;
}

AGSPlatformDriver* AGSPlatformDriver::GetDriver() {
  if (instance == NULL)
    instance = new AGSIOS();
  return instance;
}

#endif
