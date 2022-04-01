/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
// AGS Runtime header
//
//=============================================================================

#ifndef AGS_ENGINE_AC_GAME_H
#define AGS_ENGINE_AC_GAME_H

#include "ags/engine/ac/dynobj/script_view_frame.h"
#include "ags/engine/main/game_file.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define RAGMODE_PRESERVEGLOBALINT 1
#define RAGMODE_LOADNOW 0x8000000  // just to make sure it's non-zero

// Game parameter constants for backward-compatibility functions
#define GP_SPRITEWIDTH   1
#define GP_SPRITEHEIGHT  2
#define GP_NUMLOOPS      3
#define GP_NUMFRAMES     4
#define GP_ISRUNNEXTLOOP 5
#define GP_FRAMESPEED    6
#define GP_FRAMEIMAGE    7
#define GP_FRAMESOUND    8
#define GP_NUMGUIS       9
#define GP_NUMOBJECTS    10
#define GP_NUMCHARACTERS 11
#define GP_NUMINVITEMS   12
#define GP_ISFRAMEFLIPPED 13

enum CutsceneSkipStyle {
	kSkipSceneUndefined = 0,
	eSkipSceneEscOnly = 1,
	eSkipSceneAnyKey = 2,
	eSkipSceneMouse = 3,
	eSkipSceneKeyMouse = 4,
	eSkipSceneEscOrRMB = 5,
	eSkipSceneScriptOnly = 6
};

//=============================================================================
// Audio
//=============================================================================
#define VOL_CHANGEEXISTING   1678
#define VOL_SETFUTUREDEFAULT 1679
#define VOL_BOTH             1680

void Game_StopAudio(int audioType);
int  Game_IsAudioPlaying(int audioType);
void Game_SetAudioTypeSpeechVolumeDrop(int audioType, int volumeDrop);
void Game_SetAudioTypeVolume(int audioType, int volume, int changeType);

int Game_GetMODPattern();

//=============================================================================
// ---
//=============================================================================
int Game_GetDialogCount();

// Sets a default save directory, based on platform driver settings and user config
void SetDefaultSaveDirectory();
// Sets a new save directory within the save parent; copies "restart" slot if available
int Game_SetSaveGameDirectory(const char *newFolder);
const char *Game_GetSaveSlotDescription(int slnum);

const char *Game_GetGlobalStrings(int index);

int Game_GetInventoryItemCount();
int Game_GetFontCount();
int Game_GetMouseCursorCount();
int Game_GetCharacterCount();
int Game_GetGUICount();
int Game_GetViewCount();
int Game_GetUseNativeCoordinates();
int Game_GetSpriteWidth(int spriteNum);
int Game_GetSpriteHeight(int spriteNum);
int Game_GetLoopCountForView(int viewNumber);
int Game_GetRunNextSettingForLoop(int viewNumber, int loopNumber);
int Game_GetFrameCountForLoop(int viewNumber, int loopNumber);
ScriptViewFrame *Game_GetViewFrame(int viewNumber, int loopNumber, int frame);
int Game_DoOnceOnly(const char *token);

int  Game_GetTextReadingSpeed();
void Game_SetTextReadingSpeed(int newTextSpeed);
int  Game_GetMinimumTextDisplayTimeMs();
void Game_SetMinimumTextDisplayTimeMs(int newTextMinTime);
int  Game_GetIgnoreUserInputAfterTextTimeoutMs();
void Game_SetIgnoreUserInputAfterTextTimeoutMs(int newValueMs);
const char *Game_GetFileName();
const char *Game_GetName();
void Game_SetName(const char *newName);

int Game_GetSkippingCutscene();
int Game_GetInSkippableCutscene();

int Game_GetColorFromRGB(int red, int grn, int blu);
const char *Game_InputBox(const char *msg);
const char *Game_GetLocationName(int x, int y);

const char *Game_GetGlobalMessages(int index);

int Game_GetSpeechFont();
int Game_GetNormalFont();

const char *Game_GetTranslationFilename();
int Game_ChangeTranslation(const char *newFilename);
const char *Game_GetSpeechVoxFilename();
bool Game_ChangeSpeechVox(const char *newFilename);

//=============================================================================

void set_debug_mode(bool on);
void set_game_speed(int new_fps);
void setup_for_dialog();
void restore_after_dialog();
Shared::String get_save_game_directory();
Shared::String get_save_game_suffix();
void set_save_game_suffix(const Shared::String &suffix);
// Returns full path to the save for the given slot number
Shared::String get_save_game_path(int slotNum);
void restore_game_dialog();
void save_game_dialog();
void free_do_once_tokens();
// Free all the memory associated with the game
void unload_game_file();
void save_game(int slotn, const char *descript);
bool read_savedgame_description(const Shared::String &savedgame, Shared::String &description);
bool read_savedgame_screenshot(const Shared::String &savedgame, int &want_shot);
// Tries to restore saved game and displays an error on failure; if the error occurred
// too late, when the game data was already overwritten, shuts engine down.
bool try_restore_save(int slot);
bool try_restore_save(const Shared::String &path, int slot);
void serialize_bitmap(const Shared::Bitmap *thispic, Shared::Stream *out);
// On Windows we could just use IIDFromString but this is platform-independent
void convert_guid_from_text_to_binary(const char *guidText, unsigned char *buffer);
Shared::Bitmap *read_serialized_bitmap(Shared::Stream *in);
void skip_serialized_bitmap(Shared::Stream *in);
long write_screen_shot_for_vista(Shared::Stream *out, Shared::Bitmap *screenshot);

bool is_in_cutscene();
CutsceneSkipStyle get_cutscene_skipstyle();
void start_skipping_cutscene();
bool check_skip_cutscene_keypress(int kgn);
bool check_skip_cutscene_mclick(int mbut);
void initialize_skippable_cutscene();
void stop_fast_forwarding();

int __GetLocationType(int xxx, int yyy, int allowHotspot0);

// Called whenever game looses input focus
void display_switch_out();
// Called whenever game gets input focus
void display_switch_in();
// Called when the game looses input focus and must suspend
void display_switch_out_suspend();
// Called when the game gets input focus and should resume
void display_switch_in_resume();

void replace_tokens(const char *srcmes, char *destm, int maxlen = 99999);
const char *get_global_message(int msnum);
void get_message_text(int msnum, char *buffer, char giveErr = 1);

bool unserialize_audio_script_object(int index, const char *objectType, const char *serializedData, int dataSize);

// Notifies the game objects that certain sprite was updated.
// This make them update their render states, caches, and so on.
void game_sprite_updated(int sprnum);
// Notifies the game objects that certain sprite was deleted.
// Those which used that sprite will reset to dummy sprite 0, update their render states and caches.
void game_sprite_deleted(int sprnum);


extern void set_loop_counter(unsigned int new_counter);

} // namespace AGS3

#endif
