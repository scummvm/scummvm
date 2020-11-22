/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_AC_GAMESTATE_H
#define AGS_ENGINE_AC_GAMESTATE_H

#include <memory>
#include <vector>

#include "ac/characterinfo.h"
#include "ac/runtime_defines.h"
#include "game/roomstruct.h"
#include "game/viewport.h"
#include "media/audio/queuedaudioitem.h"
#include "util/geometry.h"
#include "util/string_types.h"
#include "util/string.h"
#include "ac/timer.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Bitmap;
class Stream;
typedef std::shared_ptr<Bitmap> PBitmap;
} // namespace Shared

namespace Engine {
struct RestoredData;
} // namespace Engine
} // namespace AGS
using namespace AGS; // FIXME later

struct ScriptViewport;
struct ScriptCamera;

#define GAME_STATE_RESERVED_INTS 5

// Savegame data format
enum GameStateSvgVersion {
	kGSSvgVersion_OldFormat = -1, // TODO: remove after old save support is dropped
	kGSSvgVersion_Initial = 0,
	kGSSvgVersion_350 = 1,
	kGSSvgVersion_3509 = 2,
	kGSSvgVersion_3510 = 3,
};


// Adding to this might need to modify AGSDEFNS.SH and AGSPLUGIN.H
struct GameState {
	int  score;      // player's current score
	int  usedmode;   // set by ProcessClick to last cursor mode used
	int  disabled_user_interface;  // >0 while in cutscene/etc
	int  gscript_timer;    // obsolete
	int  debug_mode;       // whether we're in debug mode
	int  globalvars[MAXGLOBALVARS];  // obsolete
	int  messagetime;      // time left for auto-remove messages
	int  usedinv;          // inventory item last used
	int  inv_top, inv_numdisp, obsolete_inv_numorder, inv_numinline;
	int  text_speed;       // how quickly text is removed
	int  sierra_inv_color; // background used to paint defualt inv window
	int  talkanim_speed;   // animation speed of talking anims
	int  inv_item_wid, inv_item_hit; // set by SetInvDimensions
	int  speech_text_shadow;         // colour of outline fonts (default black)
	int  swap_portrait_side;         // sierra-style speech swap sides
	int  speech_textwindow_gui;      // textwindow used for sierra-style speech
	int  follow_change_room_timer;   // delay before moving following characters into new room
	int  totalscore;           // maximum possible score
	int  skip_display;         // how the user can skip normal Display windows
	int  no_multiloop_repeat;  // for backwards compatibility
	int  roomscript_finished;  // on_call finished in room
	int  used_inv_on;          // inv item they clicked on
	int  no_textbg_when_voice; // no textwindow bgrnd when voice speech is used
	int  max_dialogoption_width; // max width of dialog options text window
	int  no_hicolor_fadein;      // fade out but instant in for hi-color
	int  bgspeech_game_speed;    // is background speech relative to game speed
	int  bgspeech_stay_on_display; // whether to remove bg speech when DisplaySpeech is used
	int  unfactor_speech_from_textlength; // remove "&10" when calculating time for text to stay
	int  mp3_loop_before_end;    // (UNUSED!) loop this time before end of track (ms)
	int  speech_music_drop;      // how much to drop music volume by when speech is played
	int  in_cutscene;            // we are between a StartCutscene and EndCutscene
	int  fast_forward;           // player has elected to skip cutscene
	int  room_width;      // width of current room
	int  room_height;     // height of current room
	// ** up to here is referenced in the plugin interface
	int  game_speed_modifier;
	int  score_sound;
	int  takeover_data;  // value passed to RunAGSGame in previous game
	int  replay_hotkey_unused;  // (UNUSED!) StartRecording: not supported
	int  dialog_options_x;
	int  dialog_options_y;
	int  narrator_speech;
	int  ambient_sounds_persist;
	int  lipsync_speed;
	int  close_mouth_speech_time; // stop speech animation at (messagetime - close_mouth_speech_time)
	// (this is designed to work in text-only mode)
	int  disable_antialiasing;
	int  text_speed_modifier;
	HorAlignment text_align;
	int  speech_bubble_width;
	int  min_dialogoption_width;
	int  disable_dialog_parser;
	int  anim_background_speed;  // the setting for this room
	int  top_bar_backcolor;
	int  top_bar_textcolor;
	int  top_bar_bordercolor;
	int  top_bar_borderwidth;
	int  top_bar_ypos;
	int  screenshot_width;
	int  screenshot_height;
	int  top_bar_font;
	HorAlignment speech_text_align;
	int  auto_use_walkto_points;
	int  inventory_greys_out;
	int  skip_speech_specific_key;
	int  abort_key;
	int  fade_to_red;
	int  fade_to_green;
	int  fade_to_blue;
	int  show_single_dialog_option;
	int  keep_screen_during_instant_transition;
	int  read_dialog_option_colour;
	int  stop_dialog_at_end;
	int  speech_portrait_placement; // speech portrait placement mode (automatic/custom)
	int  speech_portrait_x; // a speech portrait x offset from corresponding screen side
	int  speech_portrait_y; // a speech portrait y offset
	int  speech_display_post_time_ms; // keep speech text/portrait on screen after text/voice has finished playing;
	// no speech animation is supposed to be played at this time
	int  dialog_options_highlight_color; // The colour used for highlighted (hovered over) text in dialog options
	int  reserved[GAME_STATE_RESERVED_INTS];  // make sure if a future version adds a var, it doesn't mess anything up
	// ** up to here is referenced in the script "game." object
	long  randseed;    // random seed
	int   player_on_region;    // player's current region
	int   screen_is_faded_out; // the screen is currently black
	int   check_interaction_only;
	int   bg_frame, bg_anim_delay; // for animating backgrounds
	int   music_vol_was;  // before the volume drop
	short wait_counter;
	char  wait_skipped_by; // tells how last wait was skipped [not serialized]
	int   wait_skipped_by_data; // extended data telling how last wait was skipped [not serialized]
	short mboundx1, mboundx2, mboundy1, mboundy2;
	int   fade_effect;
	int   bg_frame_locked;
	int   globalscriptvars[MAXGSVALUES];
	int   cur_music_number, music_repeat;
	int   music_master_volume;
	int   digital_master_volume;
	char  walkable_areas_on[MAX_WALK_AREAS + 1];
	short screen_flipped;
	int   entered_at_x, entered_at_y, entered_edge;
	int   want_speech;
	int   cant_skip_speech;
	int   script_timers[MAX_TIMERS];
	int   sound_volume, speech_volume;
	int   normal_font, speech_font;
	char  key_skip_wait;
	int   swap_portrait_lastchar;
	int   swap_portrait_lastlastchar;
	int   separate_music_lib;
	int   in_conversation;
	int   screen_tint;
	int   num_parsed_words;
	short parsed_words[MAX_PARSED_WORDS];
	char  bad_parsed_word[100];
	int   raw_color;
	int   raw_modified[MAX_ROOM_BGFRAMES];
	Common::PBitmap raw_drawing_surface;
	short filenumbers[MAXSAVEGAMES];
	int   room_changes;
	int   mouse_cursor_hidden;
	int   silent_midi;
	int   silent_midi_channel;
	int   current_music_repeating;  // remember what the loop flag was when this music started
	unsigned long shakesc_delay;  // unsigned long to match loopcounter
	int   shakesc_amount, shakesc_length;
	int   rtint_red, rtint_green, rtint_blue, rtint_level, rtint_light;
	bool  rtint_enabled;
	int   end_cutscene_music;
	int   skip_until_char_stops;
	int   get_loc_name_last_time;
	int   get_loc_name_save_cursor;
	int   restore_cursor_mode_to;
	int   restore_cursor_image_to;
	short music_queue_size;
	short music_queue[MAX_QUEUED_MUSIC];
	short new_music_queue_size;
	short crossfading_out_channel;
	short crossfade_step;
	short crossfade_out_volume_per_step;
	short crossfade_initial_volume_out;
	short crossfading_in_channel;
	short crossfade_in_volume_per_step;
	short crossfade_final_volume_in;
	QueuedAudioItem new_music_queue[MAX_QUEUED_MUSIC];
	char  takeover_from[50];
	char  playmp3file_name[PLAYMP3FILE_MAX_FILENAME_LEN];
	char  globalstrings[MAXGLOBALSTRINGS][MAX_MAXSTRLEN];
	char  lastParserEntry[MAX_MAXSTRLEN];
	char  game_name[100];
	int   ground_level_areas_disabled;
	int   next_screen_transition;
	int   gamma_adjustment;
	short temporarily_turned_off_character;  // Hide Player Charactr ticked
	short inv_backwards_compatibility;
	int *gui_draw_order;
	std::vector<AGS::Shared::String> do_once_tokens;
	int   text_min_display_time_ms;
	int   ignore_user_input_after_text_timeout_ms;
	int   default_audio_type_volumes[MAX_AUDIO_TYPES];

	// Dynamic custom property values for characters and items
	std::vector<AGS::Shared::StringIMap> charProps;
	AGS::Shared::StringIMap invProps[MAX_INV];

	// Dynamic speech state
	//
	// Tells whether there is a voice-over played during current speech
	bool  speech_has_voice;
	// Tells whether the voice was played in blocking mode;
	// atm blocking speech handles itself, and we only need to finalize
	// non-blocking voice speech during game update; speech refactor would be
	// required to get rid of this rule.
	bool  speech_voice_blocking;
	// Tells whether character speech stays on screen not animated for additional time
	bool  speech_in_post_state;

	int shake_screen_yoff; // y offset of the shaking screen


	GameState();
	// Free game resources
	void Free();

	//
	// Viewport and camera control.
	// Viewports are positioned in game screen coordinates, related to the "game size",
	// while cameras are positioned in room coordinates.
	//
	// Tells if the room viewport should be adjusted automatically each time a new room is loaded
	bool IsAutoRoomViewport() const;
	// Returns main viewport position on screen, this is the overall game view
	const Rect &GetMainViewport() const;
	// Returns UI viewport position on screen, this is the GUI layer
	const Rect &GetUIViewport() const;
	// Returns Room viewport object by it's main index
	PViewport  GetRoomViewport(int index) const;
	// Returns Room viewport object by index in z-order
	const std::vector<PViewport> &GetRoomViewportsZOrdered() const;
	// Finds room viewport at the given screen coordinates; returns nullptr if non found
	PViewport  GetRoomViewportAt(int x, int y) const;
	// Returns UI viewport position in absolute coordinates (with main viewport offset)
	Rect       GetUIViewportAbs() const;
	// Returns Room viewport position in absolute coordinates (with main viewport offset)
	Rect       GetRoomViewportAbs(int index) const;
	// Sets if the room viewport should be adjusted automatically each time a new room is loaded
	void SetAutoRoomViewport(bool on);
	// Main viewport defines the location of all things drawn and interactable on the game screen.
	// Other viewports are defined relative to the main viewports.
	void SetMainViewport(const Rect &viewport);
	// UI viewport is a formal dummy viewport for GUI and Overlays (like speech).
	void SetUIViewport(const Rect &viewport);
	// Applies all the pending changes to viewports and cameras;
	// NOTE: this function may be slow, thus recommended to be called only once
	// and during the main game update.
	void UpdateViewports();
	// Notifies game state that viewports need z-order resorting upon next update.
	void InvalidateViewportZOrder();
	// Returns room camera object chosen by index
	PCamera GetRoomCamera(int index) const;
	// Runs cameras behaviors
	void UpdateRoomCameras();
	// Converts room coordinates to the game screen coordinates through the room viewport
	// This group of functions always tries to pass a point through the **primary** room viewport
	// TODO: also support using arbitrary viewport (for multiple viewports).
	Point RoomToScreen(int roomx, int roomy);
	int  RoomToScreenX(int roomx);
	int  RoomToScreenY(int roomy);
	// Converts game screen coordinates to the room coordinates through the room viewport
	// This pair of functions tries to find if there is any viewport at the given coords and results
	// in failure if there is none.
	// TODO: find out if possible to refactor and get rid of "variadic" variants;
	// usually this depends on how the arguments are created (whether they are in "variadic" or true coords)
	VpPoint ScreenToRoom(int scrx, int scry);
	VpPoint ScreenToRoomDivDown(int scrx, int scry); // native "variadic" coords variant

	// Makes sure primary viewport and camera are created and linked together
	void CreatePrimaryViewportAndCamera();
	// Creates new room viewport
	PViewport CreateRoomViewport();
	// Register camera in the managed system; optionally links to existing handle
	ScriptViewport *RegisterRoomViewport(int index, int32_t handle = 0);
	// Deletes existing room viewport
	void DeleteRoomViewport(int index);
	// Get number of room viewports
	int GetRoomViewportCount() const;
	// Creates new room camera
	PCamera CreateRoomCamera();
	// Register camera in the managed system; optionally links to existing handle
	ScriptCamera *RegisterRoomCamera(int index, int32_t handle = 0);
	// Deletes existing room camera
	void DeleteRoomCamera(int index);
	// Get number of room cameras
	int GetRoomCameraCount() const;
	// Gets script viewport reference; does NOT increment refcount
	// because script interpreter does this when acquiring managed pointer.
	ScriptViewport *GetScriptViewport(int index);
	// Gets script camera reference; does NOT increment refcount
	// because script interpreter does this when acquiring managed pointer.
	ScriptCamera *GetScriptCamera(int index);

	//
	// User input management
	//
	// Tells if game should ignore user input right now. Note that some of the parent states
	// may not ignore it at the same time, such as cutscene state, which may still be skipped
	// with a key press or a mouse button.
	bool IsIgnoringInput() const;
	// Sets ignore input state, for the given time; if there's one already, chooses max timeout
	void SetIgnoreInput(int timeout_ms);
	// Clears ignore input state
	void ClearIgnoreInput();

	//
	// Voice speech management
	//
	// Tells if there's a blocking voice speech playing right now
	bool IsBlockingVoiceSpeech() const;
	// Tells whether we have to finalize voice speech when stopping or reusing the channel
	bool IsNonBlockingVoiceSpeech() const;
	// Speech helpers
	bool ShouldPlayVoiceSpeech() const;

	//
	// Serialization
	//
	void ReadQueuedAudioItems_Aligned(Common::Stream *in);
	void ReadCustomProperties_v340(Common::Stream *in);
	void WriteCustomProperties_v340(Common::Stream *out) const;
	void ReadFromSavegame(Common::Stream *in, GameStateSvgVersion svg_ver, AGS::Engine::RestoredData &r_data);
	void WriteForSavegame(Common::Stream *out) const;
	void FreeProperties();
	void FreeViewportsAndCameras();

private:
	VpPoint ScreenToRoomImpl(int scrx, int scry, int view_index, bool clip_viewport, bool convert_cam_to_data);
	void UpdateRoomCamera(int index);

	// Defines if the room viewport should be adjusted to the room size automatically.
	bool _isAutoRoomViewport;
	// Main viewport defines the rectangle of the drawn and interactable area
	// in the most basic case it will be equal to the game size.
	Viewport _mainViewport;
	// UI viewport defines the render and interaction rectangle of game's UI.
	Viewport _uiViewport;
	// Room viewports define place on screen where the room camera's
	// contents are drawn.
	std::vector<PViewport> _roomViewports;
	// Vector of viewports sorted in z-order.
	std::vector<PViewport> _roomViewportsSorted;
	// Cameras defines the position of a "looking eye" inside the room.
	std::vector<PCamera> _roomCameras;
	// Script viewports and cameras are references to real data export to
	// user script. They became invalidated as the actual object gets
	// destroyed, but are kept in memory to prevent script errors.
	std::vector<std::pair<ScriptViewport *, int32_t>> _scViewportRefs;
	std::vector<std::pair<ScriptCamera *, int32_t>> _scCameraRefs;

	// Tells that the main viewport's position has changed since last game update
	bool  _mainViewportHasChanged;
	// Tells that room viewports need z-order resort
	bool  _roomViewportZOrderChanged;

	AGS_Clock::time_point _ignoreUserInputUntilTime;
};

// Converts legacy alignment type used in script API
HorAlignment ConvertLegacyScriptAlignment(LegacyScriptAlignment align);
// Reads legacy alignment type from the value set in script depending on the
// current Script API level. This is made to make it possible to change
// Alignment constants in the Script API and still support old version.
HorAlignment ReadScriptAlignment(int32_t align);

extern GameState play;

} // namespace AGS3

#endif
