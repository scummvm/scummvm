/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#define FORBIDDEN_SYMBOL_EXCEPTION_time

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/icb.h"
#include "engines/icb/options_manager_pc.h"
#include "engines/icb/movie_pc.h"
#include "engines/icb/cluster_manager_pc.h"
#include "engines/icb/keyboard.h"
#include "engines/icb/actor_view_pc.h"
#include "engines/icb/mission_functions.h"
#include "engines/icb/text_sprites.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/res_man.h"

#include "common/keyboard.h"
#include "common/system.h"
#include "common/events.h"
#include "common/textconsole.h"
#include "common/file.h"

namespace ICB {

#define OPTIONS_FONT_NAME "futura.pcfont"
#define MAX_BYTESIZE_OF_PROFILE_INFO (2 * 1024)
#define MAX_BYTESIZE_OF_CONTROL_INFO (512)

// Global options controller instance
OptionsManager *g_theOptionsManager;

// Temporary thumbnail direct draw surface
uint32 g_thumbSurfaceID = 0;

// Colours used for drawing
_rgb g_drawColour;
_rgb g_drawColourDark;
_rgb g_drawSelected;
_rgb g_drawSelectedDark;

#define BASE 0x100
#define BASE_DARK 0x101
#define SELECTED 0x200
#define SELECTED_DARK 0x201

// Coordinates for the actor drawn to the control configuration screen
#define CTRL_ACTOR_X -20
#define CTRL_ACTOR_Y -15
#define CTRL_ACTOR_Z -100

#define REFRESH_LIMITER 15000

// External declaration in header
_MOVIESLOT g_movieLibrary[TOTAL_NUMBER_OF_MOVIES];

// To limit slot access to within sensible range
uint32 g_largestValidSlotID = 0;

// So we can provide a sensible default slot selection by modified date
uint32 g_lastAccessedSlot = 0;

bool8 g_mainMenuLoadPlease = FALSE8;
bool8 g_resetToTitleScreen = FALSE8;
uint32 g_missionNumber = 0;

// Controlling variables for title screen timeout movies
uint32 g_titleScreenSecondsElapsed = 0;
uint32 g_titleScreenInitialCount = 0;
uint32 g_titleScreenAutoDelay = 60;

// Force screen full refresh
bool8 g_forceRefresh = FALSE8;
int32 g_skipBackgroundDrawOverFrames = 0;
bool8 g_videoOptionsCheat = FALSE8;

// Private for the title background movie handling
MovieManager *g_personalSequenceManager;

// Need this prototype
void Init_play_movie(const char *param0, bool8 param1);
uint32 GetFileSz(const char *path);

// Translation tweaks
_linked_data_file *LoadTranslatedFile(cstr session, cstr mission);

// Death text functions and defines
#define MAX_DEATH_TEXT 4
#define DEATH_TEXT_RETRYS 10

bool8 usedDeathText[MAX_DEATH_TEXT];

void InitDeathText() {
	for (int i = 0; i < MAX_DEATH_TEXT; i++) {
		usedDeathText[i] = 0;
	}
}

// Animation sequences for the control configuration screen
#define NUMBER_OF_CONTROLS 14
#define NUMBER_OF_ANIMS_PER_CONTROL 5

typedef struct {
	bool8 used;
	const char *pose;
	const char *anim;
	bool8 forwards;
	int32 repeats;

} ANIM_DESC;

ANIM_DESC cc_anim_sequences[NUMBER_OF_CONTROLS * NUMBER_OF_ANIMS_PER_CONTROL];

void InitialiseAnimSequences(void) {
	// Set all to defaults (unused)
	for (int i = 0; i < NUMBER_OF_CONTROLS * NUMBER_OF_ANIMS_PER_CONTROL; i++) {
		cc_anim_sequences[i].used = FALSE8;
		cc_anim_sequences[i].pose = NULL;
		cc_anim_sequences[i].anim = NULL;
		cc_anim_sequences[i].forwards = TRUE8;
		cc_anim_sequences[i].repeats = 0;
	}

	int control = 0;

	// Up
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "stand_to_walk";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "walk";
	cc_anim_sequences[control + 1].repeats = 3;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].anim = "walk_to_stand";
	cc_anim_sequences[control + 3].used = TRUE8;
	cc_anim_sequences[control + 3].anim = "stand";
	cc_anim_sequences[control + 3].repeats = 20;
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Crouch
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "stand";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].pose = "crouched";
	cc_anim_sequences[control + 1].anim = "stand_crouch_to_stand";
	cc_anim_sequences[control + 1].forwards = FALSE8;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].pose = "crouched";
	cc_anim_sequences[control + 2].anim = "stand";
	cc_anim_sequences[control + 2].repeats = 20;
	cc_anim_sequences[control + 3].used = TRUE8;
	cc_anim_sequences[control + 3].pose = "crouched";
	cc_anim_sequences[control + 3].anim = "stand_crouch_to_stand";
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Down
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "stand_to_walk";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "walk";
	cc_anim_sequences[control + 1].repeats = 3;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].anim = "walk_to_stand";
	cc_anim_sequences[control + 3].used = TRUE8;
	cc_anim_sequences[control + 3].anim = "step_backward";
	cc_anim_sequences[control + 3].repeats = 3;
	cc_anim_sequences[control + 4].used = TRUE8;
	cc_anim_sequences[control + 4].anim = "stand";
	cc_anim_sequences[control + 4].repeats = 20;
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Interact
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "pick_up_object_from_table";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "use_card_on_slot";
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Left
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "stand_to_walk";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "walk";
	cc_anim_sequences[control + 1].repeats = 3;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].anim = "walk_to_stand";
	cc_anim_sequences[control + 3].used = TRUE8;
	cc_anim_sequences[control + 3].anim = "sidestep_left";
	cc_anim_sequences[control + 3].repeats = 3;
	cc_anim_sequences[control + 4].used = TRUE8;
	cc_anim_sequences[control + 4].anim = "stand";
	cc_anim_sequences[control + 4].repeats = 20;
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Arm
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].pose = "gun";
	cc_anim_sequences[control].anim = "pull_out_weapon";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].pose = "gun";
	cc_anim_sequences[control + 1].anim = "stand";
	cc_anim_sequences[control + 1].repeats = 30;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].pose = "gun";
	cc_anim_sequences[control + 2].anim = "put_away_weapon";
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Right
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "stand_to_walk";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "walk";
	cc_anim_sequences[control + 1].repeats = 3;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].anim = "walk_to_stand";
	cc_anim_sequences[control + 3].used = TRUE8;
	cc_anim_sequences[control + 3].anim = "sidestep_left";
	cc_anim_sequences[control + 3].forwards = FALSE8;
	cc_anim_sequences[control + 3].repeats = 3;
	cc_anim_sequences[control + 4].used = TRUE8;
	cc_anim_sequences[control + 4].anim = "stand";
	cc_anim_sequences[control + 4].repeats = 20;
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Attack
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "low_strike";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].pose = "gun";
	cc_anim_sequences[control + 1].anim = "pull_out_weapon";
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].pose = "gun";
	cc_anim_sequences[control + 2].anim = "stand_and_shoot";
	cc_anim_sequences[control + 2].repeats = 3;
	cc_anim_sequences[control + 3].used = TRUE8;
	cc_anim_sequences[control + 3].pose = "gun";
	cc_anim_sequences[control + 3].anim = "put_away_weapon";
	cc_anim_sequences[control + 4].used = TRUE8;
	cc_anim_sequences[control + 4].anim = "stand";
	cc_anim_sequences[control + 4].repeats = 12;
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Run
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "stand_to_run";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "run";
	cc_anim_sequences[control + 1].repeats = 5;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].anim = "run_to_stand";
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Inventory
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "stand";
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Sidestep
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "sidestep_left";
	cc_anim_sequences[control].repeats = 3;
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "stand";
	cc_anim_sequences[control + 1].repeats = 10;
	cc_anim_sequences[control + 2].used = TRUE8;
	cc_anim_sequences[control + 2].anim = "step_backward";
	cc_anim_sequences[control + 2].repeats = 3;
	cc_anim_sequences[control + 3].used = TRUE8;
	cc_anim_sequences[control + 3].anim = "sidestep_left";
	cc_anim_sequences[control + 3].forwards = FALSE8;
	cc_anim_sequences[control + 3].repeats = 3;
	cc_anim_sequences[control + 4].used = TRUE8;
	cc_anim_sequences[control + 4].anim = "stand";
	cc_anim_sequences[control + 4].repeats = 30;
	control += NUMBER_OF_ANIMS_PER_CONTROL;
	// Remora
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "use_remora";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "use_remora";
	cc_anim_sequences[control + 1].forwards = FALSE8;
	control += NUMBER_OF_ANIMS_PER_CONTROL;

	// Empty slot here for another button on page1 (currently unused)
	control += NUMBER_OF_ANIMS_PER_CONTROL;

	// Pause
	cc_anim_sequences[control].used = TRUE8;
	cc_anim_sequences[control].anim = "shrug";
	cc_anim_sequences[control + 1].used = TRUE8;
	cc_anim_sequences[control + 1].anim = "stand";
	cc_anim_sequences[control + 1].repeats = 15;
}

// Debug timers
uint32 movieTime;
uint32 movieblitTime;
uint32 drawTime;

void ResetTitleScreenTimeout(void) {
	g_titleScreenSecondsElapsed = 0;
	g_titleScreenInitialCount = 0;
	g_titleScreenAutoDelay = 60;
}

void InitialiseGlobalColours() {
	g_drawColour.red = 46;
	g_drawColour.green = 87;
	g_drawColour.blue = 156;

	g_drawColourDark.red = 22;
	g_drawColourDark.green = 42;
	g_drawColourDark.blue = 76;

	g_drawSelected.red = 202;
	g_drawSelected.green = 0;
	g_drawSelected.blue = 0;

	g_drawSelectedDark.red = 90;
	g_drawSelectedDark.green = 0;
	g_drawSelectedDark.blue = 0;
}

void MakeFullSaveFilename(uint32 slot_id, char *buff) {
	// Construct full actual filename
	sprintf(buff, "saves/ICBgame%02d.index", slot_id);
}

void MakeFullThumbFilename(uint32 slot_id, char *buff) {
	// Construct full actual filename
	sprintf(buff, "saves/ICBgame%02d.thumb", slot_id);
}

void InitialiseMovieLibrary() {
	// The game intro is always visible
	g_movieLibrary[0].visible = TRUE8;

	// Zero all other flags
	for (uint32 i = 1; i < TOTAL_NUMBER_OF_MOVIES; i++) {
		g_movieLibrary[i].visible = FALSE8;
	}

	// Setup filename mappings (only done here)
	strcpy(g_movieLibrary[0].filename, "m01int2");
	strcpy(g_movieLibrary[1].filename, "m01intro");
	strcpy(g_movieLibrary[2].filename, "m01accel");
	strcpy(g_movieLibrary[3].filename, "m01cable");
	strcpy(g_movieLibrary[4].filename, "m01chasm");
	strcpy(g_movieLibrary[5].filename, "m01cut");
	strcpy(g_movieLibrary[6].filename, "m01robot");
	strcpy(g_movieLibrary[7].filename, "m01robt2");
	strcpy(g_movieLibrary[8].filename, "m01robt3");
	strcpy(g_movieLibrary[9].filename, "m01outro");

	strcpy(g_movieLibrary[10].filename, "m02tortu");
	strcpy(g_movieLibrary[11].filename, "m02intro");
	strcpy(g_movieLibrary[12].filename, "m02outro");

	strcpy(g_movieLibrary[13].filename, "m03tortu");
	strcpy(g_movieLibrary[14].filename, "m03intro");
	strcpy(g_movieLibrary[15].filename, "m03bomb");
	strcpy(g_movieLibrary[16].filename, "m03cord");
	strcpy(g_movieLibrary[17].filename, "m03shoot");
	strcpy(g_movieLibrary[18].filename, "m03c1sep");
	strcpy(g_movieLibrary[19].filename, "m03outro");

	strcpy(g_movieLibrary[20].filename, "m04tortu");
	strcpy(g_movieLibrary[21].filename, "m04intro");
	strcpy(g_movieLibrary[22].filename, "m04lift1");
	strcpy(g_movieLibrary[23].filename, "m04lift2");
	strcpy(g_movieLibrary[24].filename, "m04spec");
	strcpy(g_movieLibrary[25].filename, "m04zapp");
	strcpy(g_movieLibrary[26].filename, "m04tube");
	strcpy(g_movieLibrary[27].filename, "m04nag");
	strcpy(g_movieLibrary[28].filename, "m04outro");

	strcpy(g_movieLibrary[29].filename, "m05tortu");
	strcpy(g_movieLibrary[30].filename, "m05intro");
	strcpy(g_movieLibrary[31].filename, "m05outro");

	strcpy(g_movieLibrary[32].filename, "m07tortu");
	strcpy(g_movieLibrary[33].filename, "m07intro");
	strcpy(g_movieLibrary[34].filename, "m07sam");
	strcpy(g_movieLibrary[35].filename, "m07doors");
	strcpy(g_movieLibrary[36].filename, "m07outro");

	strcpy(g_movieLibrary[37].filename, "m08intro");
	strcpy(g_movieLibrary[38].filename, "m08betr");
	strcpy(g_movieLibrary[39].filename, "m08tortu");
	strcpy(g_movieLibrary[40].filename, "m08outro");

	strcpy(g_movieLibrary[41].filename, "m10intro");
	strcpy(g_movieLibrary[42].filename, "m10luk");
	strcpy(g_movieLibrary[43].filename, "m10exp");
	strcpy(g_movieLibrary[44].filename, "m10miss");
	strcpy(g_movieLibrary[45].filename, "m10coptr");
	strcpy(g_movieLibrary[46].filename, "m10outro");
}

void Movie_ID_to_name(uint32 id, char *buff) {
	if (id >= TOTAL_NUMBER_OF_MOVIES)
		Fatal_error("Movies information out-of-date!");

	strcpy(buff, g_movieLibrary[id].filename);
}

int32 Movie_name_to_ID(char *name) {
	for (uint32 i = 0; i < TOTAL_NUMBER_OF_MOVIES; i++) {
		// Check all movies in the library
		if (strcmp(name, g_movieLibrary[i].filename) == 0)
			return i;
	}

	// This movie isn't viewable from the title screen menu
	return -1;
}

void SaveThumbnail(uint32 slot_id) {
	Common::WriteStream *stream;
	char buff[128];

	MakeFullThumbFilename(slot_id, buff);

	stream = openDiskWriteStream(buff);

	if (!stream)
		Fatal_error("SaveThumbnail() failed to open a file");

	// First off, check the thumb surface is valid
	if (!g_thumbSurfaceID)
		Fatal_error("SaveThumbnail() cannot save a null surface");

	// Now lock the fucker
	// Lock the directdraw surface (working buffer)
	uint8 *surface_address = surface_manager->Lock_surface(g_thumbSurfaceID);
	uint32 pitch = surface_manager->Get_pitch(g_thumbSurfaceID);

	uint32 *u32surfPtr = (uint32 *)surface_address;
	// Now we need to write the 64 by 48 image data out
	for (uint32 i = 0; i < 48; i++) {
		for (uint32 j = 0; j < 64; j++) {
			stream->writeUint32LE(u32surfPtr[j]);
			if (stream->err())
				Fatal_error("SaveThumbnail() failed writing");
		}

		surface_address += pitch;
	}

	// Release it now
	surface_manager->Unlock_surface(g_thumbSurfaceID);

	delete stream;
}

void LoadThumbnail(uint32 slot_id, uint32 to_surface_id) {
	char buff[128];

	MakeFullThumbFilename(slot_id, buff);

	// If no file exists then do nothing
	if (!checkFileExists(buff)) // amode = 0
		return;

	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(buff);

	if (!stream)
		Fatal_error("LoadThumbnail() failed to open a file");

	// First off, check the thumb surface is valid
	if (!to_surface_id)
		Fatal_error("LoadThumbnail() cannot read to a null surface");

	// Now lock the fucker
	// Lock the directdraw surface (working buffer)
	uint8 *surface_address = surface_manager->Lock_surface(to_surface_id);
	uint32 pitch = surface_manager->Get_pitch(to_surface_id);

	// Now we need to read the 64 by 48 image data into the surface
	for (uint32 i = 0; i < 48; i++) {
		if (stream->read(surface_address, sizeof(uint32) * 64) != 64 * sizeof(uint32))
			Fatal_error("LoadThumbnail() failed reading");

		surface_address += pitch;
	}

	// Release it now
	surface_manager->Unlock_surface(to_surface_id);

	delete stream;
}

void LoadAMovieShot(uint32 slot_id, uint32 to_surface_id) {
	char thbFile[128];
	uint32 thbFileHash = NULL_HASH;
	char art2DCluster[MAXLEN_CLUSTER_URL];
	uint32 art2DClusterHash = NULL_HASH;

	// Make the correct filename for this pic when clustered up
	if (slot_id < 10)
		sprintf(thbFile, "images\\pc\\movie0%d.thb", slot_id);
	else
		sprintf(thbFile, "images\\pc\\movie%d.thb", slot_id);

	uint32 fo, fs;

	// Now see if it exists in the cluster
	if (!DoesClusterContainFile(pxVString("a\\2dart"), HashString(thbFile), fo, fs)) {
		// If no file exists then fill the surface with black
		surface_manager->Clear_surface(to_surface_id);
		return;
	}

	// Set this up for resman and open the thb file
	sprintf(art2DCluster, ICON_CLUSTER_PATH);
	uint8 *data = (uint8 *)rs1->Res_open(thbFile, thbFileHash, art2DCluster, art2DClusterHash);

	// First off, check the thumb surface is valid
	if (!to_surface_id)
		Fatal_error("LoadAMovieShot() cannot read to a null surface");

	// Now lock the fucker
	uint8 *surface_address = surface_manager->Lock_surface(to_surface_id);
	uint32 pitch = surface_manager->Get_pitch(to_surface_id);

	// Now we need to read the 100 by 56 image data into the surface
	for (uint32 i = 0; i < 56; i++) {
		for (uint32 j = 0; j < 100; j++) {
			*surface_address++ = *data++;
			*surface_address++ = *data++;
			*surface_address++ = *data++;
			*surface_address++ = *data++;
		}

		surface_address += (pitch - 400);
	}

	// Release it now
	surface_manager->Unlock_surface(to_surface_id);
}

OptionsManager::OptionsManager() {
	m_global_text = NULL; // Clear global text pointer

	InitialiseGlobalColours();

	m_inGame = FALSE8;
	m_useDirtyRects = FALSE8;
	m_gameover = FALSE8;
	m_haveControl = FALSE8;
	m_thatsEnoughTa = FALSE8;

	m_autoAnimating = 0;
	m_autoAnimating2 = 0;
	m_optionsBox.left = 0;
	m_optionsBox.right = 0;
	m_optionsBox.top = 0;
	m_optionsBox.bottom = 0;
	m_over_n_Frames = 0;
	m_grower = 0;
	m_lipLength = 0;
	m_box.left = m_box.right = m_box.top = m_box.bottom = 0;
	m_targetBox.left = m_targetBox.right = m_targetBox.top = m_targetBox.bottom = 0;
	m_interFrames = 0;
	m_widthIncrements = 0;
	m_bottomIncrements = 0;
	m_topIncrements = 0;
	m_warpDirection = TRUE8;

	memset(m_fontName, 0, ENGINE_STRING_LEN);
	m_font_file = NULL;
	m_fontPalette = NULL;
	m_fontHeight = 20;
	m_currentSprite = NULL;

	m_selectedShade.red = 202;
	m_selectedShade.green = 0;
	m_selectedShade.blue = 0;
	m_selectedShade.alpha = 0;

	// A useful rect to have
	m_fullscreen.left = 0;
	m_fullscreen.right = SCREEN_WIDTH;
	m_fullscreen.top = 0;
	m_fullscreen.bottom = SCREEN_DEPTH;

	m_global_text = NULL;

	// Explicitly set alpha:
	m_drawColour.alpha = 0;
	// Set default draw colour (nice pale blue)
	SetDrawColour(BASE);

	m_activeMenu = INGAME_TOP;

	memset(m_slots, 0, TOTAL_NUMBER_OF_GAME_SLOTS * sizeof(_SLOT *));
	m_slotOffset = 0;
	// m_movieOffset set further down

	m_M_TOP_selected = _NEWGAME;
	m_M_EXTRA_selected = MOVIES;
	m_M_MOVIE_selected = MOVIE01;
	m_M_PLAYSELECT_selected = M01;
	m_M_PROFILES_selected = CORD;

	m_IG_TOP_selected = CONTINUE;
	m_OPTION_selected = VIDEO_SETTINGS;
	m_AUDIO_selected = MUSIC_VOLUME;
	m_CONTROL_selected = DEVICE;
	m_GAMESLOT_selected = SLOT1;
	m_SAVECONFIRM_selected = YEY;
	m_QUIT_selected = YES;
	m_GAMEOVER_selected = RESTORE;

	// Initialise these to zero so we know if it's safe to kill them
	m_myScreenSurfaceID = 0;
	m_mySlotSurface1ID = 0;
	for (uint32 i = 0; i < 8; i++) {
		m_thumbSurfaceIDs[i] = 0;
		m_grayThumbSurfaceIDs[i] = 0;
	}
	memset(m_movieSurfaceIDs, 0, 24 * sizeof(uint32));
	memset(m_grayMovieSurfaceIDs, 0, 24 * sizeof(uint32));
	m_profileSurface = 0;

	m_movieRect.left = m_movieRect.right = m_movieRect.top = m_movieRect.bottom = 0;

	//m_colourKey = RGB(255, 0, 255); // WIN32
	warning("TODO: Use a pixelformat and SDL_MapRGB here, m_colourKey set by hand for now");
	m_colourKey = 0xFF00FF00;

	m_moveLimiter = FALSE8;
	m_alterLimiter = FALSE8;
	m_choiceLimiter = FALSE8;
	m_controlPage1 = FALSE8;
	m_controlAnimCursor = 0;

	m_move_sfx_channel = 0;
	m_choose_sfx_channel = 0;

	m_canSave = FALSE8;

	m_cursorPos = 0;
	memset(m_editBuffer, '\0', MAX_LABEL_LENGTH);
	memset(m_defaultSlotName, '\0', MAX_LABEL_LENGTH);
	m_editing = FALSE8;
	m_defaultWiper = FALSE8;
	m_emptySlotFlag = FALSE8;

	m_timePlayed = 0;

	m_paging = FALSE8;
	m_pageleft = FALSE8;
	// Rectangle enclosing the slot information (that animates when paging)
	m_slotBoundingRect.left = 86;   // 73
	m_slotBoundingRect.right = 553; // 550
	m_slotBoundingRect.top = 128;
	m_slotBoundingRect.bottom = 375;

	m_slotsFuckOffBy = 0;
	m_pageOn_from.left = m_pageOn_from.right = m_pageOn_from.bottom = m_pageOn_from.top = 0;
	m_pageOn_dest.left = m_pageOn_dest.right = m_pageOn_dest.bottom = m_pageOn_dest.top = 0;
	m_pageOff_from.left = m_pageOff_from.right = m_pageOff_from.bottom = m_pageOff_from.top = 0;
	m_pageOff_dest.left = m_pageOff_dest.right = m_pageOff_dest.bottom = m_pageOff_dest.top = 0;
	m_letJoystickQuitEdit = FALSE8;
	m_awaitingKeyPress = FALSE8;
	m_assignFlash = 0;

	// m_creditControl set further down
	// m_crediter handled in its constructor

	m_slideshowActive = FALSE8;
	m_slideLimiter = FALSE8;
	m_currentSlide = 0;
	m_slideWadger = 0;
	m_slideFillColour = 0;

	m_margin = 0;
	m_profileRect.left = m_profileRect.right = m_profileRect.bottom = m_profileRect.top = 0;
	m_profileScrollingOffset = 0;
	m_profileScrollingLine = 0;
	m_lastLineDisplayed = FALSE8;
	m_profileScrolling = 0;

	// Initialise movie library
	InitialiseMovieLibrary();

	m_movieOffset = 0;

	m_creditControl = FALSE8;

	InitDeathText();
}

OptionsManager::~OptionsManager() {
	// Just in case
	DestroySlots();
}

void OptionsManager::MakeAllSurfii() {
	uint32 i;

	if (surface_manager) {
		// Create full screen spare surface (used by bink or the faded game screen)
		m_myScreenSurfaceID = surface_manager->Create_new_surface("Title screen", SCREEN_WIDTH, SCREEN_DEPTH, SYSTEM);
		surface_manager->Set_transparent_colour_key(m_myScreenSurfaceID, m_colourKey);
		surface_manager->Fill_surface(m_myScreenSurfaceID, m_colourKey);
		// And one more surface for blit scrolling (initialise to transparent)
		m_mySlotSurface1ID = surface_manager->Create_new_surface("Slot scroller 1", SCREEN_WIDTH, SCREEN_DEPTH, SYSTEM);
		surface_manager->Set_transparent_colour_key(m_mySlotSurface1ID, m_colourKey);
		surface_manager->Fill_surface(m_mySlotSurface1ID, m_colourKey);

		// Setup a surface for the profile text
		m_profileSurface = surface_manager->Create_new_surface("Profile scroller", 285, 240, SYSTEM);
		surface_manager->Set_transparent_colour_key(m_profileSurface, 0);

		// Create slot thumbnail surfii
		for (i = 0; i < 8; i++) {
			m_thumbSurfaceIDs[i] = surface_manager->Create_new_surface(pxVString("Thumb %d", i), 64, 48, SYSTEM);
			m_grayThumbSurfaceIDs[i] = surface_manager->Create_new_surface(pxVString("Gray thumb %d", i), 64, 48, SYSTEM);
		}

		// Create movie thumbnail surfii
		for (i = 0; i < 24; i++) {
			m_movieSurfaceIDs[i] = surface_manager->Create_new_surface(pxVString("MovieLib thumb %d", i), 100, 56, SYSTEM);
			m_grayMovieSurfaceIDs[i] = surface_manager->Create_new_surface(pxVString("Gray MovieLib thumb", i), 100, 56, SYSTEM);
		}
	} else
		Fatal_error("OptionsManager::MakeAllSurfii() function called when surface_manager is NULL");
}

void OptionsManager::KillAllSurfii() {
	uint32 i;

	if (m_myScreenSurfaceID != 0) {
		surface_manager->Kill_surface(m_myScreenSurfaceID);
		m_myScreenSurfaceID = 0;
	}
	if (m_mySlotSurface1ID != 0) {
		surface_manager->Kill_surface(m_mySlotSurface1ID);
		m_mySlotSurface1ID = 0;
	}
	if (g_thumbSurfaceID != 0) {
		surface_manager->Kill_surface(g_thumbSurfaceID);
		g_thumbSurfaceID = 0;
	}
	if (m_profileSurface != 0) {
		surface_manager->Kill_surface(m_profileSurface);
		m_profileSurface = 0;
	}

	for (i = 0; i < 8; i++) {
		if (m_thumbSurfaceIDs[i] != 0) {
			surface_manager->Kill_surface(m_thumbSurfaceIDs[i]);
			m_thumbSurfaceIDs[i] = 0;
		}
		if (m_grayThumbSurfaceIDs[i] != 0) {
			surface_manager->Kill_surface(m_grayThumbSurfaceIDs[i]);
			m_grayThumbSurfaceIDs[i] = 0;
		}
	}

	for (i = 0; i < 24; i++) {
		if (m_movieSurfaceIDs[i] != 0) {
			surface_manager->Kill_surface(m_movieSurfaceIDs[i]);
			m_movieSurfaceIDs[i] = 0;
		}
		if (m_grayMovieSurfaceIDs[i] != 0) {
			surface_manager->Kill_surface(m_grayMovieSurfaceIDs[i]);
			m_grayMovieSurfaceIDs[i] = 0;
		}
	}
}

void OptionsManager::StartInGameOptions() {
	// Pauses the game and presents the in game options
	stub.Push_stub_mode(__pause_menu);

	if (g_theSpeechManager)
		g_theSpeechManager->PauseSpeech();

	if (g_theMusicManager)
		g_theMusicManager->StopMusic();

	InitialiseInGameOptions();
}

int GetDeathText() {
	int i;
	int t;

	// Have 10 attempts at finding a random one not visited recently
	i = 0;
	while (i < DEATH_TEXT_RETRYS) {
		t = (g_icb->getRandomSource()->getRandomNumber(MAX_DEATH_TEXT - 1));

		if (!usedDeathText[t]) {
			usedDeathText[t] = 1;
			return (t + 1);
		}

		i++;
	}

	// Now we may well have used them all so reinit
	InitDeathText();

	// Choose any of them and set it's flag
	t = (g_icb->getRandomSource()->getRandomNumber(MAX_DEATH_TEXT - 1));
	usedDeathText[t] = 1;

	return (t + 1);
}

void OptionsManager::StartGameOverOptions() {
	bool8 regularPlayerDeath = TRUE8;

	// Have we died under irregular circumstances?
	c_game_object *playerObj = (c_game_object *)MS->objects->Fetch_item_by_number(MS->player.Fetch_player_id());
	int32 state = playerObj->GetVariable("state");
	if (playerObj->GetIntegerVariable(state) == 2)
		regularPlayerDeath = FALSE8;

	InitialiseSounds();

	LoadGlobalTextFile();

	LoadBitmapFont();

	// Error check
	if (g_mission == NULL)
		Fatal_error("OptionsManager::StartGameOverOptions() needs to know what mission is running (ie Can't have mission == NULL)");

	// Figure out what mission is running
	const char *mn = g_mission->Fetch_tiny_mission_name();

	switch (mn[2]) {
	case '1':
		g_missionNumber = 1;
		break;
	case '2':
		g_missionNumber = 2;
		break;
	case '3':
		g_missionNumber = 3;
		break;
	case '4':
		g_missionNumber = 4;
		break;
	case '5':
		g_missionNumber = 5;
		break;
	case '7':
		g_missionNumber = 7;
		break;
	case '8':
		if (g_globalScriptVariables.GetVariable("mission9") == 0)
			g_missionNumber = 8;
		else
			g_missionNumber = 9;
		break;
	case '0':
		g_missionNumber = 10;
		break;

	default:
		Fatal_error("Couldn't resolve what mission we are currently running (how SHIT is that?) - sorry");
	}

	// So we know what logic to run
	m_inGame = TRUE8;
	m_gameover = TRUE8;

	// Menu to start with
	m_activeMenu = GAME_OVER;

	// Initially selected option (first on the list)
	m_GAMEOVER_selected = (GAMEOVER_CHOICES)0;

	m_thatsEnoughTa = FALSE8;

	// Get font height
	SetCharacterSprite('W');
	m_fontHeight = m_currentSprite->height;

	InitialiseSlots();

	// Set default draw colour
	SetDrawColour(BASE);

	MakeAllSurfii();

	if (regularPlayerDeath) {
		// Copy the screen so we can refresh
		surface_manager->Blit_surface_to_surface(working_buffer_id, m_myScreenSurfaceID, &m_fullscreen, &m_fullscreen);

		// Make the screen go red
		BloodScreen();
	} else {
		// Black screen
		surface_manager->Clear_surface(m_myScreenSurfaceID);
	}

	// Refresh our faded screen first of all
	surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);

	// Now play some death speech
	char deathSpeech[128];

	int ds = 5;

	if (g_missionNumber < 9)
		ds = GetDeathText();

	sprintf(deathSpeech, "player_death%d", ds);

	SayLineOfSpeech(HashString(deathSpeech));

	// Stop the engine sounds
	PauseSounds();

	// Take control
	m_haveControl = TRUE8;

	m_useDirtyRects = TRUE8;
}

void OptionsManager::CycleInGameOptionsLogic() {
	PollInput();

	// Quit
	if ((m_thatsEnoughTa) && (m_autoAnimating < 0)) {
		// Refresh entire screen
		surface_manager->Clear_surface(working_buffer_id);
		stub.Update_screen();

		// Free up any resources we have used
		KillAllSurfii();

		DestroySlots();

		// An extra pop needed to return control to gamescript
		if (g_resetToTitleScreen) {
			g_resetToTitleScreen = FALSE8;
			stub.Pop_stub_mode();
		} else {
			// Resume the engines sounds
			UnpauseSounds();
			g_theSpeechManager->ResumeSpeech();
		}

		// Then quit
		stub.Pop_stub_mode();
		m_haveControl = FALSE8;

		return;
	}

	// Refresh our faded screen first of all
	if (g_forceRefresh == TRUE8) {
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);
		g_forceRefresh = FALSE8;
	}

	if (!AnimateBracketsToBox(m_warpDirection))
		DrawInGameOptionsScreen();
}

void OptionsManager::ForceInGameScreenRefresh() { g_forceRefresh = TRUE8; }

void OptionsManager::CycleGameOverLogic() {
	// Leave input unpolled until speech finished
	if (g_theSpeechManager->IsPlaying() == FALSE8)
		PollInput();

	// Quit
	if (m_thatsEnoughTa) {
		// Free up any resources we have used
		KillAllSurfii();

		DestroySlots();

		// An extra pop needed to return control to gamescript
		if (g_resetToTitleScreen) {
			g_resetToTitleScreen = FALSE8;
			stub.Pop_stub_mode();
		}

		// Then quit
		stub.Pop_stub_mode();
		m_haveControl = FALSE8;

		// Specifically for the Restart Mission command but can't hurt anyway
		UnpauseSounds();

		return;
	}

	// Refresh our faded screen first of all
	surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);

	// Leave screen alone until speech finished
	if (g_theSpeechManager->IsPlaying() == FALSE8)
		DrawGameOverScreen();
}

void OptionsManager::SetDesiredOptionsBoxParameters(uint32 width, uint32 height, uint32 nFrames) {
	// Size of options box that we want to display centred in the screen
	m_optionsBox.left = (SCREEN_WIDTH / 2) - (width / 2);
	m_optionsBox.right = (SCREEN_WIDTH / 2) + (width / 2);
	m_optionsBox.top = (SCREEN_DEPTH / 2) - (height / 2) - 60;
	m_optionsBox.bottom = (SCREEN_DEPTH / 2) + (height / 2) - 60;

	// Set the number of frames to animate over
	m_over_n_Frames = nFrames;

	// Initial animating box dimensions (screen centre-point)
	m_box.left = SCREEN_WIDTH / 2;
	m_box.right = SCREEN_WIDTH / 2;
	m_box.top = (SCREEN_DEPTH / 2) - 60;
	m_box.bottom = (SCREEN_DEPTH / 2) - 60;
}

void OptionsManager::LoadTitleScreenMovie() {
	// Initialise background movie (looping)
	pxString filename;

	filename.Format("gmovies\\title.bik");
	filename.ConvertPath();
	// For maximum playback performance on the title screen play this movie directly from memory

	// Clean background res_man
	rs_bg->Res_purge_all();

	// Caculate memory needed for movie
	uint32 movieSize = GetFileSz(filename);
	if (movieSize == 0)
		Fatal_error("Couldn't get filesize of title movie");
#if 0
	// Get the memory
	uint32 moviehashID = 0xBEEF ;
	uint32 nullhash = NULL_HASH ;
	uint8 *mem = rs_bg->Res_alloc(moviehashID, filename, nullhash, movieSize) ;

	// Open the movie file and read it straight into memory
	Common::SeekableReadStream *movieStream = openDiskFileForBinaryStreamRead(filename.c_str()) ;
	if (movieStream == NULL)
		Fatal_error(pxVString("Failed to open movie file: %s for reading", (const char *)filename)) ;
	if (movieStream->read(mem, movieSize) != movieSize)
		Fatal_error("LoadTitleScreenMovie() failed to read from file") ;
	// Close the file
	delete movieStream;
#endif
	if (!g_personalSequenceManager->registerMovie(filename, FALSE8, TRUE8)) {
		Fatal_error(pxVString("Couldn't register the title screen movie: %s", (const char *)filename));
	}

	// Calculate movie blitting rectangle
	uint32 movieWidth = g_personalSequenceManager->getMovieWidth();
	uint32 movieHeight = g_personalSequenceManager->getMovieHeight();

	m_movieRect.left = 0;
	m_movieRect.top = 0;

	if (movieWidth != SCREEN_WIDTH) {
		m_movieRect.left = (SCREEN_WIDTH / 2) - (movieWidth / 2);
	}
	if (movieHeight != SCREEN_DEPTH) {
		m_movieRect.top = (SCREEN_DEPTH / 2) - (movieHeight / 2);
	}

	m_movieRect.right = m_movieRect.left + movieWidth;
	m_movieRect.bottom = m_movieRect.top + movieHeight;
}

void OptionsManager::UnloadTitleScreenMovie() {
	// Release bink from playing the background movie
	g_personalSequenceManager->kill();

	// Clean background res_man
	rs_bg->Res_purge_all();
}

void OptionsManager::StartMainOptions(void) {
	LoadBitmapFont();

	LoadGlobalTextFile();

	InitialiseSlots();

	// So we know what logic to run
	m_inGame = FALSE8;
	m_gameover = FALSE8;

	// Need to calculate bracket dimesions
	uint32 int32estWidth = 0;
	const char *msg = NULL;

	for (uint i = 0; i < NUMBER_OF_MAIN_TOP_CHOICES; i++) {
		switch (i) {
		case 0:
			msg = GetTextFromReference(HashString("opt_newgame"));
			break;
		case 1:
			msg = GetTextFromReference(HashString("opt_loadgame"));
			break;
		case 2:
			msg = GetTextFromReference(HashString("opt_options"));
			break;
		case 3:
			msg = GetTextFromReference(HashString("opt_extras"));
			break;

		case 4:
			msg = GetTextFromReference(HashString("opt_exitgame"));
			break;
		}

		uint32 width;
		width = CalculateStringWidth(msg);

		if (width > int32estWidth)
			int32estWidth = width;
	}

	// Get font height
	SetCharacterSprite('W');
	m_fontHeight = m_currentSprite->height;

	// Size of the bracketed box we want to make
	SetDesiredOptionsBoxParameters(int32estWidth + 30, 5 * (m_fontHeight + 10), 20);

	// Set default draw colour
	SetDrawColour(BASE);

	m_activeMenu = MAIN_TOP;

	// Initialise our animatable box to the correct size with lips extended
	m_box = m_optionsBox;
	m_lipLength = 10;
	m_interFrames = -1;

	LoadTitleScreenMovie();

	surface_manager->Clear_surface(working_buffer_id);

	// Get surfaces to use
	MakeAllSurfii();

	// Need to ensure that that global timer variable is set to zero
	g_globalScriptVariables.SetVariable("missionelapsedtime", 0);

	// Initial selection
	m_M_TOP_selected = _NEWGAME;

	// Whey-hey!
	LoadVisibleMovieShots();

	// New game or load from title screen (determines how to quit - ie continue gamescript or set mission)
	g_mainMenuLoadPlease = FALSE8;

	// The game intro is always visible
	g_movieLibrary[0].visible = TRUE8;

	InitialiseSounds();

	m_slideshowActive = FALSE8;

	m_thatsEnoughTa = FALSE8;

	// Take control
	m_haveControl = TRUE8;

	m_useDirtyRects = FALSE8;

	ResetTitleScreenTimeout();
}

void OptionsManager::InitialiseInGameOptions(void) {
	InitialiseSounds();
	PlayChosenFX();
	Poll_Sound_Engine();

	LoadGlobalTextFile();

	LoadBitmapFont();

	// So we know what logic to run
	m_inGame = TRUE8;
	m_gameover = FALSE8;

	// Menu to start with
	m_activeMenu = INGAME_TOP;

	// Initially selected option (first on the list)
	m_IG_TOP_selected = (IN_GAME_TOP_CHOICES)0;

	m_thatsEnoughTa = FALSE8;

	InitialiseSlots();

	// Find out what mission we are in and set default slot name to match
	const char *mn = g_mission->Fetch_tiny_mission_name();

	switch (mn[2]) {
	case '1':
		strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_investigatemine")), MAX_LABEL_LENGTH - 1);
		break;
	case '2':
		strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_securityhq")), MAX_LABEL_LENGTH - 1);
		break;
	case '3':
		strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_landtrain")), MAX_LABEL_LENGTH - 1);
		break;
	case '4':
		strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_containment")), MAX_LABEL_LENGTH - 1);
		break;
	case '5':
		strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_refinery")), MAX_LABEL_LENGTH - 1);
		break;
	case '7':
		strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_mainlandbase")), MAX_LABEL_LENGTH - 1);
		break;
	case '8':
		if (g_globalScriptVariables.GetVariable("mission9") == 0)
			strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_islandbase")), MAX_LABEL_LENGTH - 1);
		else
			strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_escape")), MAX_LABEL_LENGTH - 1);
		break;
	case '0':
		strncpy(m_defaultSlotName, GetTextFromReference(HashString("opt_submarine")), MAX_LABEL_LENGTH - 1);
		break;

	default:
		Fatal_error("Couldn't resolve what mission we are currently running (how SHIT is that?) - sorry");
	}

	// Ensure string is terminated (dem translations arg)
	m_defaultSlotName[MAX_LABEL_LENGTH - 1] = 0;

	// Get number of game ticks at ~12fps
	int ticks = g_globalScriptVariables.GetVariable("missionelapsedtime");

	// Convert to seconds played
	m_timePlayed = (uint32)((float)ticks / 12.0f);

	// Need to calculate bracket dimesions
	uint32 int32estWidth = 0;
	const char *msg = NULL;

	for (uint i = 0; i < NUMBER_OF_IN_GAME_TOP_CHOICES; i++) {
		switch (i) {
		case 0:
			msg = GetTextFromReference(HashString("opt_continue"));
			break;
		case 1:
			msg = GetTextFromReference(HashString("opt_savegame"));
			break;
		case 2:
			msg = GetTextFromReference(HashString("opt_loadgame"));
			break;
		case 3:
			msg = GetTextFromReference(HashString("opt_options"));
			break;
		case 4:
			msg = GetTextFromReference(HashString("opt_quit"));
			break;
		}

		uint32 width;
		width = CalculateStringWidth(msg);

		if (width > int32estWidth)
			int32estWidth = width;
	}

	// Get font height
	SetCharacterSprite('W');
	m_fontHeight = m_currentSprite->height;

	// Size of the bracketed box we want to make
	SetDesiredOptionsBoxParameters(int32estWidth + 30, NUMBER_OF_IN_GAME_TOP_CHOICES * (m_fontHeight + 10), 20);

	m_interFrames = -1;
	m_lipLength = 0;

	// Set default draw colour
	SetDrawColour(BASE);

	MakeAllSurfii();

	// Copy the faded screen so we can refresh
	surface_manager->Blit_surface_to_surface(working_buffer_id, m_myScreenSurfaceID, &m_fullscreen, &m_fullscreen);

	GrabThumbnailImage();
	DarkenScreen();

	// Update the working buffer with the faded game screen
	surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);

	// Is this a bad time to save the game
	m_canSave = MS->Can_save();

	// Stop the engine sounds
	PauseSounds();

	// Take control
	m_haveControl = TRUE8;

	m_useDirtyRects = TRUE8;
}

void OptionsManager::GrabThumbnailImage() {
	// Here we need to get a thumbnail image of the current screen
	g_thumbSurfaceID = surface_manager->Create_new_surface("Thumbnail", 64, 48, SYSTEM);

	// Byte size of a pixel (ie 32-bit)
	uint32 PIXELSIZE = 4;

	uint32 r_total;
	uint32 g_total;
	uint32 b_total;

	// The target surface to stored averaged pixel data
	uint8 *dest_address = surface_manager->Lock_surface(g_thumbSurfaceID);
	uint32 dest_pitch = surface_manager->Get_pitch(g_thumbSurfaceID);

	// For screen surface access
	uint8 *surface_address = surface_manager->Lock_surface(m_myScreenSurfaceID);
	uint32 pitch = surface_manager->Get_pitch(m_myScreenSurfaceID);

	uint32 blocksize = 10 * PIXELSIZE;

	// Look at 10x10 portions of the screen in turn
	for (uint32 row = 0; row < 48; row++) {

		for (uint32 col = 0; col < 64; col++) {
			r_total = 0;
			g_total = 0;
			b_total = 0;

			// Gather pixel information
			for (uint32 r = 0; r < 10; r++) {
				for (uint32 c = 0; c < 10; c++) {
					// Read pixel components to additive totals
					r_total += *surface_address++;
					g_total += *surface_address++;
					b_total += *surface_address++;
					surface_address++;
				}

				// Next line in this portion please
				surface_address += (pitch - blocksize);
			}

			// Cheers, now move the surface pointer to first pixel in next portion
			surface_address -= (pitch * 10);
			surface_address += blocksize;

			// Now record the average at the destination pointer
			*dest_address++ = (uint8)(r_total / 100);
			*dest_address++ = (uint8)(g_total / 100);
			*dest_address++ = (uint8)(b_total / 100);
			dest_address++;
		}

		// Need to move destination pointer to next line
		dest_address += (dest_pitch - (64 * PIXELSIZE));

		// Need to move source pointer to next line
		surface_address += ((pitch * 10) - (640 * PIXELSIZE));
	}

	surface_manager->Unlock_surface(m_myScreenSurfaceID);
	surface_manager->Unlock_surface(g_thumbSurfaceID);
}

void OptionsManager::DrawGameOverScreen() {
	const char *msg = NULL;

	switch (m_activeMenu) {
	case GAME_OVER:
		uint8 *ad;
		uint32 pitch;
		ad = surface_manager->Lock_surface(working_buffer_id);
		pitch = surface_manager->Get_pitch(working_buffer_id);

		// Game Over Dude
		msg = GetTextFromReference(HashString("opt_gameover"));
		DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

		msg = GetTextFromReference(HashString("opt_loadgame"));
		DisplayText(ad, pitch, msg, 0, 150, (m_GAMEOVER_selected == RESTORE) ? SELECTEDFONT : NORMALFONT, TRUE8);

		msg = GetTextFromReference(HashString("opt_restart"));
		DisplayText(ad, pitch, msg, 0, 170, (m_GAMEOVER_selected == RESTART) ? SELECTEDFONT : NORMALFONT, TRUE8);

		msg = GetTextFromReference(HashString("opt_quit"));
		DisplayText(ad, pitch, msg, 0, 190, (m_GAMEOVER_selected == FUCKTHAT) ? SELECTEDFONT : NORMALFONT, TRUE8);

		surface_manager->Unlock_surface(working_buffer_id);

		break;

	case DEAD_LOAD:
		DrawMainLoadScreen();

		break;

	case DEAD_QUIT:
		DrawQuitGameConfirmScreen();

		break;
	default:
		break;
	}
}

void OptionsManager::DrawInGameOptionsScreen() {
	bool8 animating;
	uint32 pitch;
	uint8 *surface_address;
	const char *msg = NULL;
	uint32 temp;
	pxString str;
	int32 icon_x, icon_y;

	// Are we trying to quit options
	if (m_thatsEnoughTa) {
		AnimateThoseBrackets(FALSE8);
	} else {
		switch (m_activeMenu) {
		case INGAME_TOP:
			animating = AnimateThoseBrackets(TRUE8);

			if (!animating) {
				// Lock the directdraw surface (working buffer)
				surface_address = surface_manager->Lock_surface(working_buffer_id);
				pitch = surface_manager->Get_pitch(working_buffer_id);

				// Calculate proper positions from lengths once font is defined properly
				msg = GetTextFromReference(HashString("opt_continue"));
				DisplayText(surface_address, pitch, msg, 0, 130, (m_IG_TOP_selected == CONTINUE) ? SELECTEDFONT : NORMALFONT, TRUE8);

				msg = GetTextFromReference(HashString("opt_savegame"));
				if (m_canSave)
					DisplayText(surface_address, pitch, msg, 0, 150, (m_IG_TOP_selected == SAVE_GAME) ? SELECTEDFONT : NORMALFONT, TRUE8);
				else
					DisplayText(surface_address, pitch, msg, 0, 150, PALEFONT, TRUE8);

				msg = GetTextFromReference(HashString("opt_loadgame"));
				DisplayText(surface_address, pitch, msg, 0, 170, (m_IG_TOP_selected == LOAD_GAME) ? SELECTEDFONT : NORMALFONT, TRUE8);

				msg = GetTextFromReference(HashString("opt_options"));
				DisplayText(surface_address, pitch, msg, 0, 190, (m_IG_TOP_selected == OPTIONS) ? SELECTEDFONT : NORMALFONT, TRUE8);

				msg = GetTextFromReference(HashString("opt_quit"));
				DisplayText(surface_address, pitch, msg, 0, 210, (m_IG_TOP_selected == QUIT) ? SELECTEDFONT : NORMALFONT, TRUE8);

				msg = GetTextFromReference(HashString("opt_gamepaused"));
				DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

				SetDrawColour(BASE_DARK);

				// Inner lining top
				Draw_vertical_line(m_box.left, m_box.top, m_lipLength, &m_drawColour, surface_address, pitch);
				Draw_vertical_line(m_box.right, m_box.top, m_lipLength, &m_drawColour, surface_address, pitch);
				Draw_horizontal_line(m_box.left, m_box.top, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);

				// Inner lining bottom
				Draw_vertical_line(m_box.left, m_box.bottom - m_lipLength, m_lipLength, &m_drawColour, surface_address, pitch);
				Draw_vertical_line(m_box.right, m_box.bottom - m_lipLength, m_lipLength + 1, &m_drawColour, surface_address, pitch);
				Draw_horizontal_line(m_box.left, m_box.bottom, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);

				SetDrawColour(BASE);

				// Outer lining top
				Draw_vertical_line(m_box.left - 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
				Draw_vertical_line(m_box.right + 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
				Draw_horizontal_line(m_box.left - 1, m_box.top - 1, m_box.right - m_box.left + 2, &m_drawColour, surface_address, pitch);

				// Out lining bottom
				Draw_vertical_line(m_box.left - 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
				Draw_vertical_line(m_box.right + 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
				Draw_horizontal_line(m_box.left, m_box.bottom + 1, m_box.right - m_box.left + 1, &m_drawColour, surface_address, pitch);

				// Unlock the working buffer
				surface_manager->Unlock_surface(working_buffer_id);
			}

			break;

		case INGAME_OPTIONS:

			DrawGameOptions();

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;

		case INGAME_VIDEO:

			DrawVideoSettings();

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;

		case INGAME_AUDIO:

			DrawAudioSettings();

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;

		case INGAME_CONTROLS:

			DrawControllerConfiguration();

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;

		case INGAME_SAVE:
			// Set default drawing colour
			SetDrawColour(BASE);

			// The slots themselves
			if (!m_paging) {
				// Regular
				DrawGameSlots(m_slotOffset);
			} else {
				// Animating
				AnimateSlotsPaging();
			}

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_savegame"));
			DisplayText(surface_address, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

			icon_x = 40;
			icon_y = (SCREEN_DEPTH / 2) - (m_fontHeight / 2);

			// Draw the non-selectable paging icons
			if (m_slotOffset != 0) {
				temp = CalculateStringWidth("<");
				DrawRectangle((bool8)(m_paging && m_pageleft), icon_x, icon_y, temp + 18, m_fontHeight - 2, surface_address, pitch);
				DisplayText(surface_address, pitch, "<", icon_x + 10, icon_y - 2, (m_paging && m_pageleft) ? SELECTEDFONT : NORMALFONT, FALSE8);
			}
			if (m_slotOffset < TOTAL_NUMBER_OF_GAME_SLOTS - NUMBER_OF_VISIBLE_GAME_SLOTS) {
				temp = CalculateStringWidth(">");
				icon_x = SCREEN_WIDTH - icon_x - temp - 18;
				DrawRectangle((bool8)(m_paging && !m_pageleft), icon_x, icon_y, temp + 18, m_fontHeight - 2, surface_address, pitch);
				DisplayText(surface_address, pitch, ">", icon_x + 10, icon_y - 2, (m_paging && !m_pageleft) ? SELECTEDFONT : NORMALFONT, FALSE8);
			}

			msg = GetTextFromReference(HashString("opt_back"));
			DisplayText(surface_address, pitch, msg, 0, 378, (m_GAMESLOT_selected == RETURN) ? SELECTEDFONT : NORMALFONT, TRUE8);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;

		case INGAME_LOAD:
			// Set default drawing colour
			SetDrawColour(BASE);

			// The slots themselves
			if (!m_paging) {
				// Regular
				DrawGameSlots(m_slotOffset);
			} else {
				// Animating
				AnimateSlotsPaging();
			}

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_loadgame"));
			DisplayText(surface_address, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

			icon_x = 40;
			icon_y = (SCREEN_DEPTH / 2) - (m_fontHeight / 2);

			// Draw the non-selectable paging icons
			if (m_slotOffset != 0) {
				temp = CalculateStringWidth("<");
				DrawRectangle((bool8)(m_paging && m_pageleft), icon_x, icon_y, temp + 18, m_fontHeight - 2, surface_address, pitch);
				DisplayText(surface_address, pitch, "<", icon_x + 10, icon_y - 2, (m_paging && m_pageleft) ? SELECTEDFONT : NORMALFONT, FALSE8);
			}
			if (m_slotOffset < TOTAL_NUMBER_OF_GAME_SLOTS - NUMBER_OF_VISIBLE_GAME_SLOTS) {
				temp = CalculateStringWidth(">");
				icon_x = SCREEN_WIDTH - icon_x - temp - 18;
				DrawRectangle((bool8)(m_paging && !m_pageleft), icon_x, icon_y, temp + 18, m_fontHeight - 2, surface_address, pitch);
				DisplayText(surface_address, pitch, ">", icon_x + 10, icon_y - 2, (m_paging && !m_pageleft) ? SELECTEDFONT : NORMALFONT, FALSE8);
			}

			msg = GetTextFromReference(HashString("opt_back"));
			DisplayText(surface_address, pitch, msg, 0, 378, (m_GAMESLOT_selected == RETURN) ? SELECTEDFONT : NORMALFONT, TRUE8);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;

		case INGAME_QUIT:
			DrawQuitGameConfirmScreen();

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;

		case INGAME_SAVECONFIRM:

			DrawOverwriteSaveConfirmScreen();

			// Lock the directdraw surface (working buffer)
			surface_address = surface_manager->Lock_surface(working_buffer_id);
			pitch = surface_manager->Get_pitch(working_buffer_id);

			msg = GetTextFromReference(HashString("opt_gamepaused"));
			DisplayText(surface_address, pitch, msg, 0, SCREEN_DEPTH - 50, NORMALFONT, TRUE8);

			// Unlock the working buffer
			surface_manager->Unlock_surface(working_buffer_id);

			break;
		default:
			break;
		}
	}

	// All done thanks
}

void OptionsManager::DrawQuitGameConfirmScreen(uint32 surface_id) {
	const char *msg = NULL;
	uint32 halfScreen = SCREEN_WIDTH / 2;
	uint32 temp;

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	if (m_activeMenu == MAIN_QUIT) {
		msg = GetTextFromReference(HashString("opt_exitgame"));
	} else {
		msg = GetTextFromReference(HashString("opt_quit"));
	}
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	msg = GetTextFromReference(HashString("opt_confirm"));
	DisplayText(ad, pitch, msg, 0, 140, NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_yes"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, halfScreen - temp - 10, 172, (bool8)(m_QUIT_selected == YES) ? SELECTEDFONT : NORMALFONT, FALSE8);

	DisplayText(ad, pitch, "/", 0, 172, NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_no"));
	DisplayText(ad, pitch, msg, halfScreen + 10, 172, (bool8)(m_QUIT_selected == NO) ? SELECTEDFONT : NORMALFONT, FALSE8);

	// Unlock the working buffer
	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawOverwriteSaveConfirmScreen(uint32 surface_id) {
	const char *msg = NULL;
	uint32 halfScreen = SCREEN_WIDTH / 2;
	uint32 temp;

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	msg = GetTextFromReference(HashString("questttl"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	msg = GetTextFromReference(HashString("okoverquest"));
	DisplayText(ad, pitch, msg, 0, 140, NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_yes"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, halfScreen - temp - 10, 172, (bool8)(m_SAVECONFIRM_selected == YEY) ? SELECTEDFONT : NORMALFONT, FALSE8);

	DisplayText(ad, pitch, "/", 0, 172, NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_no"));
	DisplayText(ad, pitch, msg, halfScreen + 10, 172, (bool8)(m_SAVECONFIRM_selected == NAY) ? SELECTEDFONT : NORMALFONT, FALSE8);

	// Unlock the working buffer
	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawMainOptionsScreen(uint32 surface_id) {
	uint32 pitch;
	uint8 *surface_address;
	const char *msg = NULL;

	// With bink compression updating only portions of the screen per cycle we
	// need to draw things that beint32 on top to another surface then blit with
	// transparency at the end of each cycle

	switch (m_activeMenu) {
	case MAIN_TOP: // Lock the directdraw surface (working buffer)
		surface_address = surface_manager->Lock_surface(surface_id);
		pitch = surface_manager->Get_pitch(surface_id);

		// Calculate proper positions from lengths once font is defined properly
		msg = GetTextFromReference(HashString("opt_newgame"));
		DisplayText(surface_address, pitch, msg, 0, 120, (m_M_TOP_selected == _NEWGAME) ? SELECTEDFONT : NORMALFONT, TRUE8);

		msg = GetTextFromReference(HashString("opt_loadgame"));
		DisplayText(surface_address, pitch, msg, 0, 140, (m_M_TOP_selected == _LOAD_GAME) ? SELECTEDFONT : NORMALFONT, TRUE8);

		msg = GetTextFromReference(HashString("opt_options"));
		DisplayText(surface_address, pitch, msg, 0, 160, (m_M_TOP_selected == _OPTIONS) ? SELECTEDFONT : NORMALFONT, TRUE8);

		msg = GetTextFromReference(HashString("opt_extras"));
#ifdef PC_DEMO
		DisplayText(surface_address, pitch, msg, 0, 180, PALEFONT, TRUE8);
#else
		DisplayText(surface_address, pitch, msg, 0, 180, (m_M_TOP_selected == _EXTRAS) ? SELECTEDFONT : NORMALFONT, TRUE8);
#endif

		msg = GetTextFromReference(HashString("opt_exitgame"));
		DisplayText(surface_address, pitch, msg, 0, 220, (m_M_TOP_selected == _EXIT_GAME) ? SELECTEDFONT : NORMALFONT, TRUE8);

		SetDrawColour(BASE_DARK);

		// Inner lining top
		Draw_vertical_line(m_box.left, m_box.top, m_lipLength, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right, m_box.top, m_lipLength, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.top, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);

		// Inner lining bottom
		Draw_vertical_line(m_box.left, m_box.bottom - m_lipLength, m_lipLength, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right, m_box.bottom - m_lipLength, m_lipLength + 1, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.bottom, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);

		SetDrawColour(BASE);

		// Outer lining top
		Draw_vertical_line(m_box.left - 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right + 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left - 1, m_box.top - 1, m_box.right - m_box.left + 2, &m_drawColour, surface_address, pitch);

		// Out lining bottom
		Draw_vertical_line(m_box.left - 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right + 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.bottom + 1, m_box.right - m_box.left + 1, &m_drawColour, surface_address, pitch);

		DisplayText(surface_address, pitch, "Copyright 2000 Revolution Software", 0, SCREEN_DEPTH - 100, PALEFONT, TRUE8);

		// Unlock the working buffer
		surface_manager->Unlock_surface(surface_id);

		break;

	case MAIN_LOAD:

		DrawMainLoadScreen();

		break;

	case MAIN_OPTIONS:

		DrawGameOptions();

		break;

	case MAIN_VIDEO:

		DrawVideoSettings();

		break;
	case MAIN_AUDIO:

		DrawAudioSettings();

		break;

	case MAIN_CONTROLS:

		DrawControllerConfiguration();

		break;

	case MAIN_EXTRAS:

		DrawExtrasScreen();

		break;

	case MAIN_MOVIES:

		DrawMovieScreen();

		break;

	case MAIN_PLAYSELECT:

		DrawPlaySelectScreen();

		break;

	case MAIN_PROFILES:

		DrawProfileSelectScreen();

		break;

	case MAIN_A_PROFILE:

		DrawProfileScreen();

		break;

	case MAIN_QUIT:

		DrawQuitGameConfirmScreen(surface_id);

		break;
	default:
		break;
	}
}

void OptionsManager::CycleMainOptionsLogic() {
	uint32 totalTime = GetMicroTimer();

	if (g_titleScreenInitialCount == 0) {
		g_titleScreenInitialCount = g_system->getMillis();
	}

	// Calculate seconds elapsed
	g_titleScreenSecondsElapsed = (g_system->getMillis() - g_titleScreenInitialCount) / 1000;

	// No demo movie for russian or polish thanks
	if (g_theClusterManager->GetLanguage() == T_RUSSIAN || g_theClusterManager->GetLanguage() == T_POLISH)
		g_titleScreenSecondsElapsed = 0;

	// Hack for dreamcatcher, don't play demo movie either
	if (g_theClusterManager->GetLanguage() == T_ENGLISH)
		g_titleScreenSecondsElapsed = 0;

	if (g_titleScreenSecondsElapsed >= g_titleScreenAutoDelay) {
		// Reset timer count
		g_titleScreenInitialCount = 0;

		// Half next delay length
		g_titleScreenAutoDelay = (uint32)(g_titleScreenAutoDelay / 2);
		// Catch lower limit
		if (g_titleScreenAutoDelay < 5)
			g_titleScreenAutoDelay = 5;

		// Bink can only handle one movie at a time
		UnloadTitleScreenMovie();

		// Reset menu to top-level
		m_activeMenu = MAIN_TOP;
		m_M_TOP_selected = _NEWGAME;
		m_box = m_optionsBox;
		m_lipLength = 10;
		m_interFrames = -1;
		m_slideshowActive = FALSE8;
		m_thatsEnoughTa = FALSE8;

		// Play movie here
		Init_play_movie("demo", FALSE8);

		// Go and play the movie
		return;
	}

	// Sometimes we need to shutdown the title screen movie so check if we need to reload it
	if (g_personalSequenceManager->busy() == FALSE8) {
		LoadTitleScreenMovie();
	}

	PollInput();

	// Quit
	if (m_thatsEnoughTa) {
		// Free up any resources we have used
		KillAllSurfii();

		DestroySlots();

		UnloadTitleScreenMovie();

		// Then quit

		if (!g_mainMenuLoadPlease) {
			// If we want a new game then just pop to gamescript
			stub.Pop_stub_mode();
		} else {
			// When loading a game this sets the right gamescript position
			stub.Set_current_stub_mode(__mission_and_console);
		}

		UnpauseSounds();

		m_haveControl = FALSE8;

		return;
	}

	if (m_slideshowActive) {
		DrawSlideShow();
	} else {
		if (g_skipBackgroundDrawOverFrames != 0)
			return;

		movieTime = GetMicroTimer();
		// Now we need to draw a frame of the title movie as our backdrop (to its own private buffer)
		g_personalSequenceManager->drawFrame(m_myScreenSurfaceID);
		movieTime = GetMicroTimer() - movieTime;

		movieblitTime = GetMicroTimer();

		// Now blit the movie frame without transparency to the working buffer
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_movieRect, &m_movieRect, 0);

		movieblitTime = GetMicroTimer() - movieblitTime;

		drawTime = GetMicroTimer();

		// And draw the options on top
		if (!AnimateBracketsToBox(m_warpDirection, working_buffer_id))
			DrawMainOptionsScreen(working_buffer_id);

		drawTime = GetMicroTimer() - drawTime;
	}

	totalTime = GetMicroTimer() - totalTime;
}

void OptionsManager::CycleLogic() {
	Poll_Sound_Engine();

	// Have we been asked to force refresh the background
	if (g_skipBackgroundDrawOverFrames > 0) {
		g_skipBackgroundDrawOverFrames--;
		return;
	}

	// Res_open every cycle to ensure it stays in memory
	LoadGlobalTextFile();

	if (m_gameover) {
		CycleGameOverLogic();
	} else {
		if (m_inGame)
			CycleInGameOptionsLogic();
		else
			CycleMainOptionsLogic();
	}
}

void OptionsManager::PlayMoveFX() {
	if (g_theFxManager) {
		g_theFxManager->Stop(m_move_sfx_channel);
		g_theFxManager->SetVolume(m_move_sfx_channel, GetSfxVolume());
		// Begin playback
		g_theFxManager->Play(m_move_sfx_channel);
	}
}

void OptionsManager::PlayChosenFX() {
	if (g_theFxManager) {
		g_theFxManager->Stop(m_choose_sfx_channel);
		g_theFxManager->SetVolume(m_choose_sfx_channel, GetSfxVolume());
		// Begin playback
		g_theFxManager->Play(m_choose_sfx_channel);
	}
}

void OptionsManager::InitialiseSounds() {
	if (g_theFxManager) {
		m_move_sfx_channel = 22;
		m_choose_sfx_channel = 23;

		// Unload samples if channels have one
		g_theFxManager->Unregister(m_move_sfx_channel);
		g_theFxManager->Unregister(m_choose_sfx_channel);

		uint32 b_offset, sz;

		if (!DoesClusterContainFile(pxVString("g\\samples.clu"), HashString("options_select.wav"), b_offset, sz))
			Fatal_error(pxVString("Couldn't find options_select.wav in global sample cluster"));

		// Pass sample name only if we're running from clusters
		g_theFxManager->Register(m_move_sfx_channel, "options_select.wav", 0, b_offset);

		if (!DoesClusterContainFile(pxVString("g\\samples.clu"), HashString("options_choose.wav"), b_offset, sz))
			Fatal_error(pxVString("Couldn't find options_choose.wav in global sample cluster"));

		// Pass sample name only if we're running from clusters
		g_theFxManager->Register(m_choose_sfx_channel, "options_choose.wav", 0, b_offset);
	}
}

void OptionsManager::MoveSelected(bool8 _down_) {
	if (m_activeMenu != MAIN_A_PROFILE) {
		if (m_moveLimiter)
			return;
	}

	m_moveLimiter = TRUE8;

	int32 currentlySelected;

	int demo = g_globalScriptVariables.GetVariable("demo");

	ResetTitleScreenTimeout();

	// Need to switch on the menu currently active
	switch (m_activeMenu) {
	case MAIN_TOP:
		currentlySelected = m_M_TOP_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;
#ifdef PC_DEMO
		if (currentlySelected == _EXTRAS) {
			if (_down_)
				currentlySelected++;
			else
				currentlySelected--;
		}
#endif

		if (currentlySelected < 0)
			m_M_TOP_selected = (MAIN_TOP_CHOICES)(NUMBER_OF_MAIN_TOP_CHOICES - 1);
		else
			m_M_TOP_selected = (MAIN_TOP_CHOICES)(currentlySelected % NUMBER_OF_MAIN_TOP_CHOICES);
		break;

	case DEAD_LOAD:
	case INGAME_SAVE:
	case INGAME_LOAD:
	case MAIN_LOAD:
		currentlySelected = m_GAMESLOT_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_GAMESLOT_selected = (GAMESLOT_CHOICES)(NUMBER_OF_GAMESLOT_CHOICES - 1);
		else
			m_GAMESLOT_selected = (GAMESLOT_CHOICES)(currentlySelected % NUMBER_OF_GAMESLOT_CHOICES);
		break;

	case MAIN_EXTRAS:
		currentlySelected = m_M_EXTRA_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (px.game_completed) {
			if (currentlySelected < 0)
				m_M_EXTRA_selected = (M_EXTRA_CHOICES)(NUMBER_OF_EXTRA_CHOICES - 1);
			else
				m_M_EXTRA_selected = (M_EXTRA_CHOICES)(currentlySelected % NUMBER_OF_EXTRA_CHOICES);
		} else {
			if (currentlySelected < 0)
				m_M_EXTRA_selected = (M_EXTRA_CHOICES)(NUMBER_OF_EXTRA_CHOICES - 1);
			else {
				if (_down_ && currentlySelected == SLIDESHOW)
					m_M_EXTRA_selected = CREDITS;
				else if (!_down_ && currentlySelected == PROFILES)
					m_M_EXTRA_selected = MOVIES;
				else
					m_M_EXTRA_selected = (M_EXTRA_CHOICES)(currentlySelected % NUMBER_OF_EXTRA_CHOICES);
			}
		}

		break;

	case MAIN_PLAYSELECT:
		currentlySelected = m_M_PLAYSELECT_selected;

	moveme:
		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		// Wraparound
		if (currentlySelected < 0)
			currentlySelected = NUMBER_OF_PLAYSELECT_CHOICES - 1;
		else
			currentlySelected = currentlySelected % NUMBER_OF_PLAYSELECT_CHOICES;

		if (demo) {
			if (currentlySelected != M01 && currentlySelected != M04 && currentlySelected != M07 && currentlySelected != CANCEL)
				goto moveme;
		}

		m_M_PLAYSELECT_selected = (M_PLAYSELECT_CHOICES)currentlySelected;

		break;

	case MAIN_MOVIES: // Can't move up or down from the paging icons
		if (m_M_MOVIE_selected == PAGELEFT || m_M_MOVIE_selected == PAGERIGHT)
			break;

		currentlySelected = m_M_MOVIE_selected;

		// Moving up
		if (!_down_) {
			// To move up from the top row
			if (currentlySelected < 4) {
				// Not allowed to move
				return;
			} else {
				// Moving up from the back button
				if (currentlySelected == NOTHANKS) {
					// Hardcode this behaviour
					currentlySelected = MOVIE10;
				} else {
					// Up one row by all other means
					currentlySelected -= 4;
				}
			}
		} else {
			// To move down from the back button
			if (currentlySelected == NOTHANKS) {
				// Not allowed to move
				return;
			} else {
				// Moving down from the bottom row
				if (currentlySelected > MOVIE08) {
					currentlySelected = NOTHANKS;
				} else {
					// Down one row by all other means
					currentlySelected += 4;
				}
			}
		}

		// As movie total is dynamic then we need to provide further code here to catch the arse end
		if (currentlySelected != NOTHANKS) {
			if (currentlySelected + m_movieOffset >= TOTAL_NUMBER_OF_MOVIES) {
				// Ban this illegal move
				currentlySelected = m_M_MOVIE_selected;
				return;
			}
		}

		// Avoid playing a sound if we haven't changed the selection
		if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)currentlySelected) {
			return;
		}

		// Assign new selection
		m_M_MOVIE_selected = (M_MOVIE_CHOICES)currentlySelected;

		break;

	case MAIN_PROFILES:
		currentlySelected = m_M_PROFILES_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_M_PROFILES_selected = (M_PROFILES_CHOICES)(M_NUMBER_OF_PROFILE_CHOICES - 1);
		else
			m_M_PROFILES_selected = (M_PROFILES_CHOICES)(currentlySelected % M_NUMBER_OF_PROFILE_CHOICES);
		break;

	case MAIN_A_PROFILE:
		if (_down_) {
			// Can we scroll down at all
			if (m_lastLineDisplayed)
				return;

			m_profileScrolling = 1;

			m_profileScrollingOffset += 2;

			if (m_profileScrollingOffset == 20) {
				// Down a line
				m_profileScrollingOffset = 0;
				m_profileScrollingLine++;
			}
		} else {
			// Can we scroll up at all
			if (m_profileScrollingLine == -1 && m_profileScrollingOffset == 0)
				return;

			m_profileScrolling = -1;

			m_profileScrollingOffset -= 2;

			if (m_profileScrollingOffset == -2) {
				// Up a line
				m_profileScrollingOffset = 18;
				m_profileScrollingLine--;
			}
		}
		return;

	case DEAD_QUIT:
	case INGAME_QUIT:
	case MAIN_QUIT:
		currentlySelected = m_QUIT_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_QUIT_selected = (QUIT_CHOICES)(NUMBER_OF_QUIT_CHOICES - 1);
		else
			m_QUIT_selected = (QUIT_CHOICES)(currentlySelected % NUMBER_OF_QUIT_CHOICES);
		break;

	case INGAME_SAVECONFIRM:
		currentlySelected = m_SAVECONFIRM_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_SAVECONFIRM_selected = (SAVECONFIRM_CHOICES)(NUMBER_OF_SAVECONFIRM_CHOICES - 1);
		else
			m_SAVECONFIRM_selected = (SAVECONFIRM_CHOICES)(currentlySelected % NUMBER_OF_SAVECONFIRM_CHOICES);
		break;

	case INGAME_TOP:
		currentlySelected = m_IG_TOP_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		// Is save game disabled
		if (m_canSave == FALSE8) {
			if (currentlySelected == SAVE_GAME) {
				if (_down_)
					currentlySelected++;
				else
					currentlySelected--;
			}
		}

		if (currentlySelected < 0)
			m_IG_TOP_selected = (IN_GAME_TOP_CHOICES)(NUMBER_OF_IN_GAME_TOP_CHOICES - 1);
		else
			m_IG_TOP_selected = (IN_GAME_TOP_CHOICES)(currentlySelected % NUMBER_OF_IN_GAME_TOP_CHOICES);
		break;

	case MAIN_OPTIONS:
	case INGAME_OPTIONS:
		currentlySelected = m_OPTION_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_OPTION_selected = (OPTION_CHOICES)(NUMBER_OPTION_CHOICES - 1);
		else
			m_OPTION_selected = (OPTION_CHOICES)(currentlySelected % NUMBER_OPTION_CHOICES);
		break;

	case MAIN_VIDEO:
	case INGAME_VIDEO:
		currentlySelected = m_VIDEO_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (g_videoOptionsCheat == FALSE8) {
			// Illegal selections without cheat
			if (currentlySelected == SHADOWS || currentlySelected == FRAMELIMITER) {
				if (_down_ == TRUE8)
					currentlySelected = LEAVE;
				else
					currentlySelected = SEMITRANS;
			}
		}

		if (currentlySelected < 0)
			m_VIDEO_selected = (VIDEO_CHOICES)(NUMBER_OF_VIDEO_CHOICES - 1);
		else
			m_VIDEO_selected = (VIDEO_CHOICES)(currentlySelected % NUMBER_OF_VIDEO_CHOICES);
		break;

	case MAIN_AUDIO:
	case INGAME_AUDIO:
		currentlySelected = m_AUDIO_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_AUDIO_selected = (AUDIO_CHOICES)(NUMBER_OF_AUDIO_CHOICES - 1);
		else
			m_AUDIO_selected = (AUDIO_CHOICES)(currentlySelected % NUMBER_OF_AUDIO_CHOICES);
		break;

	case MAIN_CONTROLS:
	case INGAME_CONTROLS:
		currentlySelected = m_CONTROL_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_CONTROL_selected = (CONTROL_CHOICES)(NUMBER_OF_CONTROL_CHOICES - 1);
		else
			m_CONTROL_selected = (CONTROL_CHOICES)(currentlySelected % NUMBER_OF_CONTROL_CHOICES);

		// Account for extra choice on second page
		if (m_controlPage1 && m_CONTROL_selected == PAUSE) {
			if (_down_)
				m_CONTROL_selected = (CONTROL_CHOICES)(PAUSE + 1);
			else
				m_CONTROL_selected = (CONTROL_CHOICES)(PAUSE - 1);
		}

		if (m_CONTROL_selected >= UP_CROUCH && m_CONTROL_selected <= PAUSE) {
			// Need to change to the next animation for this control
			int indexToNextAnim = (m_CONTROL_selected - 2) * 2;
			if (m_controlPage1 == FALSE8)
				indexToNextAnim++;
			indexToNextAnim *= NUMBER_OF_ANIMS_PER_CONTROL;

			// Reset to first animation in this control sequence as we've just changed selection
			m_controlAnimCursor = 0;

			// Do the dynamic change
			ChangeAnimPlaying(cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].pose, cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].anim,
			                  cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].forwards, cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].repeats,
			                  CTRL_ACTOR_X, CTRL_ACTOR_Y, CTRL_ACTOR_Z);
		} else
			ChangeAnimPlaying(NULL, "stand", TRUE8, 0, CTRL_ACTOR_X, CTRL_ACTOR_Y, CTRL_ACTOR_Z);

		break;

	case GAME_OVER:
		currentlySelected = m_GAMEOVER_selected;

		if (_down_)
			currentlySelected++;
		else
			currentlySelected--;

		if (currentlySelected < 0)
			m_GAMEOVER_selected = (GAMEOVER_CHOICES)(NUMBER_OF_GAMEOVER_CHOICES - 1);
		else
			m_GAMEOVER_selected = (GAMEOVER_CHOICES)(currentlySelected % NUMBER_OF_GAMEOVER_CHOICES);
		break;
	default:
		return;
	}

	PlayMoveFX();
	Poll_Sound_Engine();
}

void OptionsManager::AlterSelected(bool8 _right_) {
	uint32 currentlySelected;

	ResetTitleScreenTimeout();

	// Need to switch on the menu currently active
	switch (m_activeMenu) {
	case DEAD_QUIT:
	case INGAME_QUIT:
	case MAIN_QUIT:
		if (m_alterLimiter)
			break;

		m_alterLimiter = TRUE8;
		MoveSelected(TRUE8);
		break;

	case DEAD_LOAD:
	case INGAME_LOAD:
	case INGAME_SAVE:
	case MAIN_LOAD:
		if (m_alterLimiter)
			break;

		m_alterLimiter = TRUE8;

		if (m_paging)
			break;

		// Initialise blitting rectangles
		m_pageOn_from = m_slotBoundingRect;
		m_pageOn_dest = m_slotBoundingRect;
		m_pageOff_from = m_slotBoundingRect;
		m_pageOff_dest = m_slotBoundingRect;

		if (!_right_) {
			m_pageleft = TRUE8;
			m_pageOn_dest.left = m_pageOn_dest.right = 0;

			if (m_slotOffset > 0) {
				m_paging = TRUE8;

				LoadPagingThumbnails(m_slotOffset - NUMBER_OF_VISIBLE_GAME_SLOTS);
				LoadVisibleThumbnails();
			} else
				return;
		} else {
			m_pageleft = FALSE8;
			m_pageOn_dest.left = m_pageOn_dest.right = SCREEN_WIDTH - 1;

			if (m_slotOffset != (TOTAL_NUMBER_OF_GAME_SLOTS - NUMBER_OF_VISIBLE_GAME_SLOTS)) {
				m_paging = TRUE8;

				LoadPagingThumbnails(m_slotOffset + NUMBER_OF_VISIBLE_GAME_SLOTS);
				LoadVisibleThumbnails();
			} else
				return;
		}

		// Special case move
		PlayChosenFX();
		Poll_Sound_Engine();

		break;

	case INGAME_SAVECONFIRM:
		if (m_alterLimiter)
			break;

		m_alterLimiter = TRUE8;
		MoveSelected(TRUE8);
		break;

	case MAIN_MOVIES:
		if (m_alterLimiter)
			break;

		m_alterLimiter = TRUE8;

		// Can't move left or right from the back button
		if (m_M_MOVIE_selected == NOTHANKS)
			break;

		currentlySelected = m_M_MOVIE_selected;

		// Move right
		if (_right_) {
			if (currentlySelected == PAGERIGHT) {
				// Not allowed to move but might be nice if this actually paged here
				DoChoice();
				break;
			} else {
				if (currentlySelected == PAGELEFT) {
					currentlySelected = MOVIE05;
				} else {
					// Last column
					if (currentlySelected % 4 == 3) {
						// The icon is unavailable if we're on the last page
						if (m_movieOffset >= TOTAL_NUMBER_OF_MOVIES - M_NUMBER_OF_VISIBLE_MOVIE_SLOTS)
							break;

						// Select the paging icon and auto-page
						m_M_MOVIE_selected = PAGERIGHT;
						DoChoice();
						break;
					} else {
						currentlySelected++;
					}
				}
			}
		} else {
			if (currentlySelected == PAGELEFT) {
				// Not allowed to move but might be nice if this actually paged here
				DoChoice();
				break;
			} else {
				if (currentlySelected == PAGERIGHT) {
					currentlySelected = MOVIE08;
				} else {
					if (currentlySelected % 4 == 0) {
						// The icon is unavailable if we're on the first page
						if (m_movieOffset == 0)
							break;

						// Select the paging icon and auto-page
						m_M_MOVIE_selected = PAGELEFT;
						DoChoice();
						break;
					} else {
						currentlySelected--;
					}
				}
			}
		}

		// As movie total is dynamic then we need to provide further code here
		if (currentlySelected < NOTHANKS) {
			if (currentlySelected + m_movieOffset >= TOTAL_NUMBER_OF_MOVIES) {
				// Ban this illegal move
				currentlySelected = m_M_MOVIE_selected;
				break;
			}
		}

		// Special case move
		if (m_M_MOVIE_selected != (M_MOVIE_CHOICES)currentlySelected) {
			PlayMoveFX();
			Poll_Sound_Engine();
		}

		// Assign new selection
		m_M_MOVIE_selected = (M_MOVIE_CHOICES)currentlySelected;

		break;

	case MAIN_VIDEO:
	case INGAME_VIDEO:
		if (m_alterLimiter)
			break;

		m_alterLimiter = TRUE8;

		switch (m_VIDEO_selected) {
		case SUBTITLES:
			if (px.on_screen_text)
				px.on_screen_text = FALSE8;
			else
				px.on_screen_text = TRUE8;

			// Chosen noise please
			PlayChosenFX();
			Poll_Sound_Engine();
			return;

		case SEMITRANS:
			if (px.semitransparencies)
				px.semitransparencies = FALSE8;
			else
				px.semitransparencies = TRUE8;

			// Chosen noise please
			PlayChosenFX();
			Poll_Sound_Engine();
			return;

		case SHADOWS:
			if (_right_) {
				if (px.actorShadows == 3)
					px.actorShadows = -1;
				else
					px.actorShadows++;
			} else {
				if (px.actorShadows == -1)
					px.actorShadows = 3;
				else
					px.actorShadows--;
			}
			// Chosen noise please
			PlayChosenFX();
			Poll_Sound_Engine();
			return;

		case FRAMELIMITER:
			if (_right_) {
				if (stub.cycle_speed < 200)
					stub.cycle_speed += 10;
				else if (stub.cycle_speed < 951)
					stub.cycle_speed += 50;
			} else {
				if (stub.cycle_speed > 200)
					stub.cycle_speed -= 50;
				else if (stub.cycle_speed > 10)
					stub.cycle_speed -= 10;
			}
			PlayChosenFX();
			Poll_Sound_Engine();
			return;
		default:
			break;
		}
		break;

	case MAIN_AUDIO:
	case INGAME_AUDIO:
		switch (m_AUDIO_selected) {
		case MUSIC_VOLUME:
			if (_right_) {
				if (GetMusicVolume() < 128)
					SetMusicVolume(GetMusicVolume() + 1);
			} else {
				if (GetMusicVolume() > 0)
					SetMusicVolume(GetMusicVolume() - 1);
			}
			// Update it
			if (g_theMusicManager) {
				g_theMusicManager->SetMusicVolume(GetMusicVolume());

				g_personalSequenceManager->setVolume(GetMusicVolume());
				g_theSequenceManager->setVolume(GetMusicVolume());
			}
			break;
		case SPEECH_VOLUME:
			if (_right_) {
				if (GetSpeechVolume() < 128)
					SetSpeechVolume(GetSpeechVolume() + 1);
			} else {
				if (GetSpeechVolume() > 0)
					SetSpeechVolume(GetSpeechVolume() - 1);
			}
			// Update it
			if (g_theSpeechManager) {
				g_theSpeechManager->SetSpeechVolume(GetSpeechVolume());
			}
			break;
		case SFX_VOLUME:
			if (_right_) {
				if (GetSfxVolume() < 128)
					SetSfxVolume(GetSfxVolume() + 1);
			} else {
				if (GetSfxVolume() > 0)
					SetSfxVolume(GetSfxVolume() - 1);
			}
			break;
		default:
			break;
		}
		break;

	case MAIN_CONTROLS:
	case INGAME_CONTROLS:
		if (m_alterLimiter)
			break;

		m_alterLimiter = TRUE8;

		switch (m_CONTROL_selected) {
		case DEVICE:
			if (currentJoystick == NO_JOYSTICK) {
				SetDefaultJoystick();
				currentJoystick = attachedJoystick;
			} else {
				SetDefaultKeys();
				currentJoystick = NO_JOYSTICK;
				// Ensure we move to actor relative for the keyboard
				g_icb_session->player.Set_control_mode(ACTOR_RELATIVE);
			}
			break;

		case METHOD: // Force actor relative if we're using the keyboard
			if (currentJoystick == NO_JOYSTICK) {
				g_icb_session->player.Set_control_mode(ACTOR_RELATIVE);
			} else {
				if (g_icb_session->player.Get_control_mode() == SCREEN_RELATIVE) {
					g_icb_session->player.Set_control_mode(ACTOR_RELATIVE);
				} else {
					g_icb_session->player.Set_control_mode(SCREEN_RELATIVE);
				}
			}
			break;

		default:
			if (m_CONTROL_selected >= UP_CROUCH && m_CONTROL_selected <= PAUSE) {
				if (m_controlPage1)
					m_controlPage1 = FALSE8;
				else {
					m_controlPage1 = TRUE8;

					// Account for extra choice on page 2
					if (m_CONTROL_selected == PAUSE)
						m_CONTROL_selected = SIDESTEP_REMORA;
				}

				// Need to change to the next animation for this control
				int indexToNextAnim = (m_CONTROL_selected - 2) * 2;
				if (m_controlPage1 == FALSE8)
					indexToNextAnim++;
				indexToNextAnim *= NUMBER_OF_ANIMS_PER_CONTROL;

				// See if we have a valid animation left in this sequence or return to first animation
				if (cc_anim_sequences[indexToNextAnim + m_controlAnimCursor + 1].used && m_controlAnimCursor < 4)
					m_controlAnimCursor++;
				else
					m_controlAnimCursor = 0;

				// Do the dynamic change
				ChangeAnimPlaying(cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].pose, cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].anim,
				                  cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].forwards,
				                  cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].repeats, CTRL_ACTOR_X, CTRL_ACTOR_Y, CTRL_ACTOR_Z);
			} else
				return;

			break;
		}

		PlayChosenFX();
		Poll_Sound_Engine();
		break;
	default:
		break;
	}
}

void OptionsManager::MakeGrayScaleThumbnail(uint32 src_surface_id, uint32 dst_surface_id, uint32 w, uint32 h) {
	// The target surface
	uint8 *dst_address = surface_manager->Lock_surface(dst_surface_id);
	uint32 dst_pitch = surface_manager->Get_pitch(dst_surface_id);

	// The source surface
	uint8 *src_address = surface_manager->Lock_surface(src_surface_id);
	uint32 src_pitch = surface_manager->Get_pitch(src_surface_id);

	uint32 width = w * 4;

	// Loop over all pixels
	for (uint32 row = 0; row < h; row++) {
		for (uint32 col = 0; col < w; col++) {
			uint8 r = *src_address++;
			uint8 g = *src_address++;
			uint8 b = *src_address++;
			src_address++;

			uint8 gray = (uint8)((r + g + b) / 3);

			*dst_address++ = gray;
			*dst_address++ = gray;
			*dst_address++ = gray;
			dst_address++;
		}

		src_address += (src_pitch - width);
		dst_address += (dst_pitch - width);
	}

	surface_manager->Unlock_surface(src_surface_id);
	surface_manager->Unlock_surface(dst_surface_id);
}

void OptionsManager::LoadVisibleThumbnails() {
	// Load the four visible slot thumbnails to our first four slot surfaces
	LoadThumbnail(m_slotOffset + SLOT1, m_thumbSurfaceIDs[SLOT1]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[SLOT1], m_grayThumbSurfaceIDs[SLOT1]);
	LoadThumbnail(m_slotOffset + SLOT2, m_thumbSurfaceIDs[SLOT2]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[SLOT2], m_grayThumbSurfaceIDs[SLOT2]);
	LoadThumbnail(m_slotOffset + SLOT3, m_thumbSurfaceIDs[SLOT3]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[SLOT3], m_grayThumbSurfaceIDs[SLOT3]);
	LoadThumbnail(m_slotOffset + SLOT4, m_thumbSurfaceIDs[SLOT4]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[SLOT4], m_grayThumbSurfaceIDs[SLOT4]);
}

void OptionsManager::LoadVisibleMovieShots() {
	// Load all movie pics for the selected page
	LoadAMovieShot(m_movieOffset + MOVIE01, m_movieSurfaceIDs[MOVIE01]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE01], m_grayMovieSurfaceIDs[MOVIE01], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE02, m_movieSurfaceIDs[MOVIE02]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE02], m_grayMovieSurfaceIDs[MOVIE02], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE03, m_movieSurfaceIDs[MOVIE03]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE03], m_grayMovieSurfaceIDs[MOVIE03], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE04, m_movieSurfaceIDs[MOVIE04]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE04], m_grayMovieSurfaceIDs[MOVIE04], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE05, m_movieSurfaceIDs[MOVIE05]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE05], m_grayMovieSurfaceIDs[MOVIE05], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE06, m_movieSurfaceIDs[MOVIE06]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE06], m_grayMovieSurfaceIDs[MOVIE06], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE07, m_movieSurfaceIDs[MOVIE07]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE07], m_grayMovieSurfaceIDs[MOVIE07], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE08, m_movieSurfaceIDs[MOVIE08]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE08], m_grayMovieSurfaceIDs[MOVIE08], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE09, m_movieSurfaceIDs[MOVIE09]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE09], m_grayMovieSurfaceIDs[MOVIE09], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE10, m_movieSurfaceIDs[MOVIE10]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE10], m_grayMovieSurfaceIDs[MOVIE10], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE11, m_movieSurfaceIDs[MOVIE11]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE11], m_grayMovieSurfaceIDs[MOVIE11], 100, 56);
	LoadAMovieShot(m_movieOffset + MOVIE12, m_movieSurfaceIDs[MOVIE12]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[MOVIE12], m_grayMovieSurfaceIDs[MOVIE12], 100, 56);
}

void OptionsManager::LoadPagingThumbnails(uint32 slotOffset) {
	// Load the four visible slot thumbnails to our first four slot surfaces
	LoadThumbnail(slotOffset + SLOT1, m_thumbSurfaceIDs[4]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[4], m_grayThumbSurfaceIDs[4]);
	LoadThumbnail(slotOffset + SLOT2, m_thumbSurfaceIDs[5]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[5], m_grayThumbSurfaceIDs[5]);
	LoadThumbnail(slotOffset + SLOT3, m_thumbSurfaceIDs[6]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[6], m_grayThumbSurfaceIDs[6]);
	LoadThumbnail(slotOffset + SLOT4, m_thumbSurfaceIDs[7]);
	MakeGrayScaleThumbnail(m_thumbSurfaceIDs[7], m_grayThumbSurfaceIDs[7]);
}

void OptionsManager::LoadPagingMovieShots(uint32 slotOffset) {
	// Load all movie pics for the selected page
	LoadAMovieShot(slotOffset + MOVIE01, m_movieSurfaceIDs[12]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[12], m_grayMovieSurfaceIDs[12], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE02, m_movieSurfaceIDs[13]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[13], m_grayMovieSurfaceIDs[13], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE03, m_movieSurfaceIDs[14]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[14], m_grayMovieSurfaceIDs[14], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE04, m_movieSurfaceIDs[15]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[15], m_grayMovieSurfaceIDs[15], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE05, m_movieSurfaceIDs[16]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[16], m_grayMovieSurfaceIDs[16], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE06, m_movieSurfaceIDs[17]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[17], m_grayMovieSurfaceIDs[17], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE07, m_movieSurfaceIDs[18]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[18], m_grayMovieSurfaceIDs[18], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE08, m_movieSurfaceIDs[19]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[19], m_grayMovieSurfaceIDs[19], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE09, m_movieSurfaceIDs[20]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[20], m_grayMovieSurfaceIDs[20], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE10, m_movieSurfaceIDs[21]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[21], m_grayMovieSurfaceIDs[21], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE11, m_movieSurfaceIDs[22]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[22], m_grayMovieSurfaceIDs[22], 100, 56);
	LoadAMovieShot(slotOffset + MOVIE12, m_movieSurfaceIDs[23]);
	MakeGrayScaleThumbnail(m_movieSurfaceIDs[23], m_grayMovieSurfaceIDs[23], 100, 56);
}

void OptionsManager::OnEscapeKey() {
	// Simply select the correct menu choice and perform a simulated button press
	switch (m_activeMenu) {
	case MAIN_TOP:
		m_M_TOP_selected = _EXIT_GAME;
		break;
	case DEAD_LOAD:
	case INGAME_SAVE:
	case INGAME_LOAD:
	case MAIN_LOAD:
		m_GAMESLOT_selected = RETURN;
		break;
	case INGAME_OPTIONS:
	case MAIN_OPTIONS:
		m_OPTION_selected = BACK;
		break;
	case INGAME_VIDEO:
	case MAIN_VIDEO:
		m_VIDEO_selected = LEAVE;
		break;
	case INGAME_AUDIO:
	case MAIN_AUDIO:
		m_AUDIO_selected = DO_ONE;
		break;
	case INGAME_CONTROLS:
	case MAIN_CONTROLS:
		m_CONTROL_selected = DONE;
		break;
	case MAIN_EXTRAS:
		m_M_EXTRA_selected = ALLDONE;
		break;
	case MAIN_MOVIES:
		m_M_MOVIE_selected = NOTHANKS;
		break;
	case MAIN_PLAYSELECT:
		m_M_PLAYSELECT_selected = CANCEL;
		break;
	case MAIN_PROFILES:
		m_M_PROFILES_selected = RET;
		break;
	case MAIN_A_PROFILE:
		break;
	case INGAME_TOP:
		m_IG_TOP_selected = CONTINUE;
		break;
	case GAME_OVER:
		m_GAMEOVER_selected = FUCKTHAT;
		break;

	// Do nothing for (critical) menus not in the list ie quit confirm screens
	default:
		return;
	}

	DoChoice();
}

void OptionsManager::DoChoice() {
	if (m_choiceLimiter)
		return;

	m_choiceLimiter = TRUE8;

	LRECT r;
	char buff[128];

	ResetTitleScreenTimeout();

	// For play mission gamescript bookmarks
	const char *actualMissions[] = {g_m01, g_m02, g_m03, g_m04, g_m05, g_m07, g_m08, g_m09, g_m10};

	// Need to switch on the menu currently active
	switch (m_activeMenu) {
	case MAIN_TOP:
		switch (m_M_TOP_selected) {
		case _NEWGAME:
			m_thatsEnoughTa = TRUE8;
			break;
		case _LOAD_GAME:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_loadgame")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 7);
			m_activeMenu = MAIN_LOAD;

			// Display the page containing the last modified save slot and select it by default
			m_slotOffset = (g_lastAccessedSlot / NUMBER_OF_VISIBLE_GAME_SLOTS) * NUMBER_OF_VISIBLE_GAME_SLOTS;
			m_GAMESLOT_selected = (GAMESLOT_CHOICES)(g_lastAccessedSlot % NUMBER_OF_VISIBLE_GAME_SLOTS);

			LoadVisibleThumbnails();
			break;
		case _OPTIONS:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_options")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 8);
			m_activeMenu = MAIN_OPTIONS;
			m_OPTION_selected = VIDEO_SETTINGS;
			break;
		case _EXTRAS:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_extras")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 7);
			m_activeMenu = MAIN_EXTRAS;
			m_M_EXTRA_selected = MOVIES;
			break;

		case _EXIT_GAME:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_exitgame")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 7);
			m_activeMenu = MAIN_QUIT;
			m_QUIT_selected = NO;
			break;
		}
		break;

	case MAIN_EXTRAS:
		switch (m_M_EXTRA_selected) {
		case MOVIES:
			m_activeMenu = MAIN_MOVIES;
			m_M_MOVIE_selected = MOVIE01;
			LoadVisibleMovieShots();
			break;
		case SLIDESHOW:
			InitialiseSlideShow();
			break;
		case PLAYSELECT:
			m_activeMenu = MAIN_PLAYSELECT;
			m_M_PLAYSELECT_selected = M01;
			break;
		case PROFILES:
			m_activeMenu = MAIN_PROFILES;
			m_M_PROFILES_selected = CORD;
			break;
		case CREDITS:
			stub.Push_stub_mode(__credits);
			break;
		case ALLDONE:
			m_warpDirection = FALSE8;
			m_activeMenu = MAIN_TOP;
			m_M_TOP_selected = _EXTRAS;
			break;
		}
		break;

	case MAIN_PLAYSELECT:
		switch (m_M_PLAYSELECT_selected) {
		case M01:
		case M02:
		case M03:
		case M04:
		case M05:
		case M07:
		case M08:
		case M09:
		case M10:
			UnloadTitleScreenMovie();

			// Run to correct position in gamescript
			gs.Run_to_bookmark(actualMissions[m_M_PLAYSELECT_selected]);

			g_mainMenuLoadPlease = FALSE8;
			m_thatsEnoughTa = TRUE8;
			break;

		case CANCEL:
			m_activeMenu = MAIN_EXTRAS;
			m_M_EXTRA_selected = PLAYSELECT;
			break;
		}
		break;

	case DEAD_LOAD:
	case INGAME_LOAD:
	case MAIN_LOAD: // Refresh entire screen
		if (m_useDirtyRects)
			surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);

		switch (m_GAMESLOT_selected) {
		case SLOT1:
		case SLOT2:
		case SLOT3:
		case SLOT4:
			if (m_slots[m_slotOffset + m_GAMESLOT_selected] != NULL) {
				MakeFullSaveFilename(m_slotOffset + m_GAMESLOT_selected, buff);
				// LOAD THIS INDEX FILE:

				if (Load_game(buff) != __LOAD_OK)
					Fatal_error("Load game failed.");

				// Return to game
				m_autoAnimating = 0;
				m_thatsEnoughTa = TRUE8;

				if (m_activeMenu == MAIN_LOAD)
					g_mainMenuLoadPlease = TRUE8;
			}
			break;
		case RETURN:
			m_warpDirection = FALSE8;

			if (m_activeMenu == INGAME_LOAD) {
				m_activeMenu = INGAME_TOP;
				m_IG_TOP_selected = LOAD_GAME;
			} else if (m_activeMenu == MAIN_LOAD) {
				m_activeMenu = MAIN_TOP;
				m_M_TOP_selected = _LOAD_GAME;
			} else {
				m_activeMenu = GAME_OVER;
				m_GAMEOVER_selected = RESTORE;
			}

			break;
		}
		break;

	case MAIN_OPTIONS:
	case INGAME_OPTIONS:
		switch (m_OPTION_selected) {
		case VIDEO_SETTINGS:
			if (m_activeMenu == INGAME_OPTIONS) {
				// Refresh entire screen
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);
				m_activeMenu = INGAME_VIDEO;
			} else
				m_activeMenu = MAIN_VIDEO;
			m_VIDEO_selected = SUBTITLES;
			break;
		case AUDIO_SETTINGS:
			if (m_activeMenu == INGAME_OPTIONS) {
				// Refresh entire screen
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);
				m_activeMenu = INGAME_AUDIO;
			} else
				m_activeMenu = MAIN_AUDIO;
			m_AUDIO_selected = MUSIC_VOLUME;
			break;
		case CONTROLS:
			if (m_activeMenu == INGAME_OPTIONS) {
				// Refresh entire screen
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);
				m_activeMenu = INGAME_CONTROLS;
			} else
				m_activeMenu = MAIN_CONTROLS;
			m_CONTROL_selected = DEVICE;
			InitialiseControlsScreen();
			break;
		case BACK:
			m_warpDirection = FALSE8;
			if (m_activeMenu == INGAME_OPTIONS) {
				m_activeMenu = INGAME_TOP;
				m_IG_TOP_selected = OPTIONS;
			} else {
				m_activeMenu = MAIN_TOP;
				m_M_TOP_selected = _OPTIONS;
			}
			break;
		}
		break;

	case MAIN_VIDEO:
	case INGAME_VIDEO:
		switch (m_VIDEO_selected) {
		case LEAVE:
			if (m_activeMenu == MAIN_VIDEO)
				m_activeMenu = MAIN_OPTIONS;
			else {
				// Refresh entire screen
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);
				m_activeMenu = INGAME_OPTIONS;
			}
			m_OPTION_selected = VIDEO_SETTINGS;
			break;

		// No other option should play a sound
		default:
			return;
		}
		break;

	case MAIN_AUDIO:
	case INGAME_AUDIO:
		switch (m_AUDIO_selected) {
		case DO_ONE:
			if (m_activeMenu == MAIN_AUDIO)
				m_activeMenu = MAIN_OPTIONS;
			else {
				// Refresh entire screen
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);
				m_activeMenu = INGAME_OPTIONS;
			}
			m_OPTION_selected = AUDIO_SETTINGS;
			break;

		// No other option should play a sound
		default:
			return;
		}
		break;

	case MAIN_MOVIES:
		switch (m_M_MOVIE_selected) {
		case MOVIE01:
		case MOVIE02:
		case MOVIE03:
		case MOVIE04:
		case MOVIE05:
		case MOVIE06:
		case MOVIE07:
		case MOVIE08:
		case MOVIE09:
		case MOVIE10:
		case MOVIE11:
		case MOVIE12:
			UnloadTitleScreenMovie();
			PlayMovie(m_movieOffset + m_M_MOVIE_selected);
			// Mute chosen SFX on playing a movie
			return;

		case NOTHANKS:
			m_activeMenu = MAIN_EXTRAS;
			m_M_EXTRA_selected = MOVIES;
			break;

		case PAGELEFT:
			if (m_paging)
				break;

			// Initialise blitting rectangles
			m_pageOn_from = m_slotBoundingRect;
			m_pageOn_dest = m_slotBoundingRect;
			m_pageOff_from = m_slotBoundingRect;
			m_pageOff_dest = m_slotBoundingRect;

			m_pageleft = TRUE8;
			m_pageOn_dest.left = m_pageOn_dest.right = 0;

			if (m_movieOffset > 0) {
				m_paging = TRUE8;
				LoadPagingMovieShots(m_movieOffset - M_NUMBER_OF_VISIBLE_MOVIE_SLOTS);
				LoadVisibleMovieShots();
			}

			break;

		case PAGERIGHT:
			if (m_paging)
				break;

			// Initialise blitting rectangles
			m_pageOn_from = m_slotBoundingRect;
			m_pageOn_dest = m_slotBoundingRect;
			m_pageOff_from = m_slotBoundingRect;
			m_pageOff_dest = m_slotBoundingRect;

			m_pageleft = FALSE8;
			m_pageOn_dest.left = m_pageOn_dest.right = SCREEN_WIDTH - 1;

			if (m_movieOffset < (TOTAL_NUMBER_OF_MOVIES - M_NUMBER_OF_VISIBLE_MOVIE_SLOTS)) {
				m_paging = TRUE8;
				LoadPagingMovieShots(m_movieOffset + M_NUMBER_OF_VISIBLE_MOVIE_SLOTS);
				LoadVisibleMovieShots();
			}
			break;
		}
		break;

	case MAIN_PROFILES:
		switch (m_M_PROFILES_selected) {
		// Do the same thing for all profiles
		case CORD:
		case CHI:
		case GREGOR:
		case NAGAROV:
		case LUKYAN:
		case KEIFFER:
		case TOLSTOV:
		case ALEXANDRA:
		case OLIAKOV:
		case SPECTRE:
			m_activeMenu = MAIN_A_PROFILE;
			InitialiseAProfile();
			break;
		case RET:
			m_activeMenu = MAIN_EXTRAS;
			m_M_EXTRA_selected = PROFILES;
			break;
		}
		break;

	case MAIN_A_PROFILE: // Can only ever return from a profile screen
		m_activeMenu = MAIN_PROFILES;
		DrawWidescreenBorders();
		// This will auto remember which profile was chosen (to return to)
		break;

	case MAIN_QUIT:
		switch (m_QUIT_selected) {
		case NO:
			m_warpDirection = FALSE8;
			m_activeMenu = MAIN_TOP;
			m_M_TOP_selected = _EXIT_GAME;
			break;
		case YES: {
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			g_system->getEventManager()->pushEvent(event);
			break;
		}
		}
		break;

	case INGAME_TOP:
		switch (m_IG_TOP_selected) {
		case OPTIONS:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_options")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 8);
			m_activeMenu = INGAME_OPTIONS;
			m_OPTION_selected = VIDEO_SETTINGS;
			break;
		case SAVE_GAME:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_savegame")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 7);
			m_activeMenu = INGAME_SAVE;

			// Display the page containing the last modified save slot and select it by default
			m_slotOffset = (g_lastAccessedSlot / NUMBER_OF_VISIBLE_GAME_SLOTS) * NUMBER_OF_VISIBLE_GAME_SLOTS;
			m_GAMESLOT_selected = (GAMESLOT_CHOICES)(g_lastAccessedSlot % NUMBER_OF_VISIBLE_GAME_SLOTS);

			LoadVisibleThumbnails();
			break;
		case LOAD_GAME:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_loadgame")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 7);
			m_activeMenu = INGAME_LOAD;

			// Display the page containing the last modified save slot and select it by default
			m_slotOffset = (g_lastAccessedSlot / NUMBER_OF_VISIBLE_GAME_SLOTS) * NUMBER_OF_VISIBLE_GAME_SLOTS;
			m_GAMESLOT_selected = (GAMESLOT_CHOICES)(g_lastAccessedSlot % NUMBER_OF_VISIBLE_GAME_SLOTS);

			LoadVisibleThumbnails();
			break;
		case CONTINUE:
			SetDrawColour(BASE_DARK);
			m_thatsEnoughTa = TRUE8;
			break;
		case QUIT:
			GetCentredRectFotText(GetTextFromReference(HashString("opt_quit")), r, 80);
			SetTargetBox(r.left, r.right, r.top, r.bottom, 7);
			m_activeMenu = INGAME_QUIT;
			m_QUIT_selected = NO;
			break;
		}
		break;

	case MAIN_CONTROLS:
	case INGAME_CONTROLS:
		m_configLimiter = TRUE8;
		m_assignFlash = 0;
		switch (m_CONTROL_selected) {
		// Set the selected function to unassigned

		case UP_CROUCH:
			if (m_controlPage1) {
				up_key = 0;
				up_joy = 0xFF;
			} else {
				crouch_key = 0;
				crouch_button = 0xFF;
			}
			m_awaitingKeyPress = TRUE8;
			m_editing = TRUE8;
			Clear_DI_key_buffer();
			break;

		case DOWN_INTERACT:
			if (m_controlPage1) {
				down_key = 0;
				down_joy = 0xFF;
			} else {
				interact_key = 0;
				interact_button = 0xFF;
			}
			m_awaitingKeyPress = TRUE8;
			m_editing = TRUE8;
			Clear_DI_key_buffer();
			break;

		case LEFT_ARM:
			if (m_controlPage1) {
				left_key = 0;
				left_joy = 0xFF;
			} else {
				arm_key = 0;
				arm_button = 0xFF;
			}
			m_awaitingKeyPress = TRUE8;
			m_editing = TRUE8;
			Clear_DI_key_buffer();
			break;

		case RIGHT_ATTACK:
			if (m_controlPage1) {
				right_key = 0;
				right_joy = 0xFF;
			} else {
				fire_key = 0;
				fire_button = 0xFF;
			}
			m_awaitingKeyPress = TRUE8;
			m_editing = TRUE8;
			Clear_DI_key_buffer();
			break;

		case RUN_INVENTORY:
			if (m_controlPage1) {
				run_key = 0;
				run_button = 0xFF;
			} else {
				inventory_key = 0;
				inventory_button = 0xFF;
			}
			m_awaitingKeyPress = TRUE8;
			m_editing = TRUE8;
			Clear_DI_key_buffer();
			break;

		case SIDESTEP_REMORA:
			if (m_controlPage1) {
				sidestep_key = 0;
				sidestep_button = 0xFF;
			} else {
				remora_key = 0;
				remora_button = 0xFF;
			}
			m_awaitingKeyPress = TRUE8;
			m_editing = TRUE8;
			Clear_DI_key_buffer();
			break;

		case PAUSE: // Can only select this on page 2 by default
			pause_key = 0;
			pause_button = 0xFF;
			m_awaitingKeyPress = TRUE8;
			m_editing = TRUE8;
			Clear_DI_key_buffer();
			break;

		case DEFAULTS:
			if (currentJoystick == NO_JOYSTICK)
				SetDefaultKeys();
			else
				SetDefaultJoystick();
			break;

		case DONE:
			if (m_inGame) {
				// Refresh entire screen
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);
				m_activeMenu = INGAME_OPTIONS;
			} else {
				m_activeMenu = MAIN_OPTIONS;
			}
			m_OPTION_selected = CONTROLS;
			break;

		// Don't play sound on the remaining options
		default:
			return;
		}
		break;

	case INGAME_SAVE: // Refresh entire screen
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_fullscreen, &m_fullscreen);

		switch (m_GAMESLOT_selected) {
		case SLOT1:
		case SLOT2:
		case SLOT3:
		case SLOT4: // Only EditSlotLabel() can quit editing
			if (m_editing == TRUE8)
				return;
			// Set this flag so that EditSlotLabel() gets called for this slot
			m_editing = TRUE8;
			m_letJoystickQuitEdit = FALSE8;
			// If the slot contains data, copy the label into our editing buffer else use default name
			if (m_slots[m_slotOffset + m_GAMESLOT_selected] != NULL) {
				// Wish to edit an existing slot label
				strcpy(m_editBuffer, m_slots[m_slotOffset + m_GAMESLOT_selected]->label);
				// Record this ex-time played so we restore if if the action is cancelled
				m_emptySlotFlag = m_slots[m_slotOffset + m_GAMESLOT_selected]->secondsPlayed;
				// Overwrite with current timeplayed
				m_slots[m_slotOffset + m_GAMESLOT_selected]->secondsPlayed = m_timePlayed;
				m_defaultWiper = FALSE8;
			} else {
				// Need to create a new slot here
				m_slots[m_slotOffset + m_GAMESLOT_selected] = new _SLOT;
				m_slots[m_slotOffset + m_GAMESLOT_selected]->secondsPlayed = m_timePlayed;
				memset(m_slots[m_slotOffset + m_GAMESLOT_selected]->label, 0, MAX_LABEL_LENGTH);
				strcpy(m_editBuffer, m_defaultSlotName);
				m_defaultWiper = TRUE8;
				// By setting this to zero it can be used to identify this newly created slot on cencel (ie need a delete)
				m_emptySlotFlag = 0;
			}

			// This slot instantly becomes the most recently modified file so update default selection explicitly
			g_lastAccessedSlot = m_slotOffset + m_GAMESLOT_selected;

			// Get the current character position
			m_cursorPos = strlen(m_editBuffer);
			if (m_cursorPos >= MAX_LABEL_LENGTH - 1)
				m_cursorPos = MAX_LABEL_LENGTH - 2;
			Clear_key_buffer();
			break;

		case RETURN:
			m_warpDirection = FALSE8;
			m_activeMenu = INGAME_TOP;
			m_IG_TOP_selected = CONTINUE;
			break;
		}
		break;

	case INGAME_SAVECONFIRM:
		switch (m_SAVECONFIRM_selected) {
		case NAY:
			m_activeMenu = INGAME_SAVE;

			// Cancellation code
			if (m_emptySlotFlag == 0) {
				// Slot was previously empty so delete on cancellation
				delete m_slots[m_slotOffset + m_GAMESLOT_selected];
				m_slots[m_slotOffset + m_GAMESLOT_selected] = NULL;
			} else {
				// Just need to restore time played to cancel
				m_slots[m_slotOffset + m_GAMESLOT_selected]->secondsPlayed = m_emptySlotFlag;
			}

			memset(m_editBuffer, '\0', MAX_LABEL_LENGTH);
			m_choiceLimiter = TRUE8;
			ForceInGameScreenRefresh();
			break;

		case YEY:
			m_activeMenu = INGAME_SAVE;

			// Save over slot code

			// Remove cursor from the end of the buffer
			m_editBuffer[m_cursorPos] = '\0';

			// Then set the label to equal the buffer
			strcpy(m_slots[m_slotOffset + m_GAMESLOT_selected]->label, m_editBuffer);

			// Now actually save the game
			MakeFullSaveFilename(m_slotOffset + m_GAMESLOT_selected, buff);
			g_mission->Save_game_position(buff, m_slots[m_slotOffset + m_GAMESLOT_selected]->label, m_slots[m_slotOffset + m_GAMESLOT_selected]->secondsPlayed);
			// Aint32 with a thumbnail
			SaveThumbnail(m_slotOffset + m_GAMESLOT_selected);
			LoadVisibleThumbnails();

			memset(m_editBuffer, '\0', MAX_LABEL_LENGTH);
			m_GAMESLOT_selected = RETURN;
			m_choiceLimiter = TRUE8;
			ForceInGameScreenRefresh();
			break;
		}
		break;

	case INGAME_QUIT:
		switch (m_QUIT_selected) {
		case NO:
			m_warpDirection = FALSE8;
			m_activeMenu = INGAME_TOP;
			m_IG_TOP_selected = QUIT;
			break;
		case YES:
			m_thatsEnoughTa = TRUE8;
			m_autoAnimating = 0;
#ifndef PC_DEMO
			if (gs.Running_from_gamescript()) {
				// Reset
				gs.Restart_game_script();
				g_resetToTitleScreen = TRUE8;
			} else
#endif
			{
				// Shutdown if no gamescript
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				g_system->getEventManager()->pushEvent(event);
			}
			break;
		}
		break;

	case GAME_OVER:
		switch (m_GAMEOVER_selected) {
		case RESTORE:
			m_activeMenu = DEAD_LOAD;

			// Display the page containing the last modified save slot and select it by default
			m_slotOffset = (g_lastAccessedSlot / NUMBER_OF_VISIBLE_GAME_SLOTS) * NUMBER_OF_VISIBLE_GAME_SLOTS;
			m_GAMESLOT_selected = (GAMESLOT_CHOICES)(g_lastAccessedSlot % NUMBER_OF_VISIBLE_GAME_SLOTS);

			LoadVisibleThumbnails();
			break;
		case RESTART: // We need to restart the mission that we're currently on

			// Nearly had me this - Load mission includes a six!
			if (g_missionNumber > 5)
				g_missionNumber--;

			// Mission 8-9 special cases
			if (g_missionNumber == 7) {
				g_globalScriptVariables.SetVariable("mission9", 0);
			}
			if (g_missionNumber == 8) {
				g_globalScriptVariables.SetVariable("mission9", 1);
			}

			// Need to preserve global timer variable
			int32 tv;
			tv = g_globalScriptVariables.GetVariable("missionelapsedtime");

			// Use this function and then quit
			RestartMission();

			g_globalScriptVariables.SetVariable("missionelapsedtime", tv);

			m_thatsEnoughTa = TRUE8;
			break;

		case FUCKTHAT:
			m_activeMenu = DEAD_QUIT;
			m_QUIT_selected = NO;
			break;
		}
		break;

	case DEAD_QUIT:
		switch (m_QUIT_selected) {
		case NO:
			m_activeMenu = GAME_OVER;
			m_GAMEOVER_selected = FUCKTHAT;
			break;
		case YES:
			m_thatsEnoughTa = TRUE8;
			if (gs.Running_from_gamescript()) {
				// Reset
				gs.Restart_game_script();
				g_resetToTitleScreen = TRUE8;
			} else {
				// Shutdown if no gamescript
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				g_system->getEventManager()->pushEvent(event);
			}
			break;
		}
		break;

	default:
		return;
	}

	PlayChosenFX();
	Poll_Sound_Engine();
}

void OptionsManager::PlayMovie(uint32 id) {
	// Are we allowed to view this movie yet
	if (g_movieLibrary[id].visible == FALSE8)
		return;

	char buff[32];

	// Get the filename
	Movie_ID_to_name(id, buff);

	bool8 cancelret = TRUE8;

	// Ensure correct CD is in the drive here so we can provide a cancel option
	switch (buff[2]) {
	case '1':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION1);
		break;
	case '2':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION2);
		break;
	case '3':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION3);
		break;
	case '4':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION4);
		break;
	case '5':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION5);
		break;
	case '7':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION7);
		break;
	case '8':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION8);
		break;
	case '9':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION9);
		break;
	case '0':
		cancelret = g_theClusterManager->CheckDiscInsertedWithCancel(MISSION10);
		break;
	}

	// User cancelled so do nowt
	if (cancelret == TRUE8)
		return;

	// Register and play
	Init_play_movie(buff, FALSE8);

	// Don't draw the background this cycle
	g_skipBackgroundDrawOverFrames = 5;
}

bool8 OptionsManager::VerifyLabel() {
	// Do not allow empty labels as that's just silly
	if (m_cursorPos == 0)
		return FALSE8;

	// Could add further restrictions here to the label names ie pure whitespace
	// but the problem is how to inform the user that the label is invalid easily.

	// Another difficulty arises when the length exceeds expected limits but NOT
	// before the imposed 22 character limit is reached ie all capital w's...
	// Perhaps I need to watch this everytime a character is input to the label
	// and refuse any further input if the string exceeds the slot blitting box.

	return TRUE8;
}

void OptionsManager::EditSlotLabel() {
	char c;
	static int flash = 0;
	char buff[128];

	if (!Read_Joystick(0))
		m_letJoystickQuitEdit = TRUE8;

	int id = m_slotOffset + m_GAMESLOT_selected;

	// Quit on joystick too
	if ((Read_Joystick(0)) && m_letJoystickQuitEdit) {
		// Not allowed an empty name 'cos thats daft
		if (!VerifyLabel())
			return;

		// Construct full filename
		MakeFullSaveFilename(id, buff);

		// Now check to see if the file exists (overwrite used slot)
		if (checkFileExists(buff)) { // amode 0
			// Slot is in use so do confirm prompt screen
			m_activeMenu = INGAME_SAVECONFIRM;
			m_SAVECONFIRM_selected = NAY;
			m_editing = FALSE8;
			ForceInGameScreenRefresh();
			return;
		}

		// Slot empty so just save without prompting

		// Remove cursor from the end of the buffer
		m_editBuffer[m_cursorPos] = '\0';

		// Then set the label to equal the buffer
		strcpy(m_slots[id]->label, m_editBuffer);

		// Now actually save the game
		g_mission->Save_game_position(buff, m_slots[id]->label, m_slots[id]->secondsPlayed);
		// Aint32 with a thumbnail
		SaveThumbnail(id);
		LoadVisibleThumbnails();

		memset(m_editBuffer, '\0', MAX_LABEL_LENGTH);
		m_editing = FALSE8;
		m_GAMESLOT_selected = RETURN;
		m_choiceLimiter = TRUE8;
	}

	// User cancallation
	if (Read_DI_once_keys(Common::KEYCODE_ESCAPE)) {
		if (m_emptySlotFlag == 0) {
			// Slot was previously empty so delete on cancellation
			delete m_slots[id];
			m_slots[id] = NULL;
		} else {
			// Just need to restore time played to cancel
			m_slots[id]->secondsPlayed = m_emptySlotFlag;
		}

		memset(m_editBuffer, '\0', MAX_LABEL_LENGTH);
		m_editing = FALSE8;
		m_choiceLimiter = FALSE8;
	}

	if (KeyWaiting()) {
		ReadKey(&c);

		// Shall we quit
		if (c == Common::KEYCODE_RETURN) {
			// Not allowed an empty name 'cos thats daft
			if (!VerifyLabel())
				return;

			MakeFullSaveFilename(id, buff);

			// Now check to see if the file exists (overwrite used slot)
			if (checkFileExists(buff)) { // amode = 0
				// Slot is in use so do confirm prompt screen
				m_activeMenu = INGAME_SAVECONFIRM;
				m_SAVECONFIRM_selected = NAY;
				m_editing = FALSE8;
				ForceInGameScreenRefresh();
				return;
			}

			// Slot empty so just save without prompting

			// Remove cursor from the end of the buffer
			m_editBuffer[m_cursorPos] = '\0';

			// Then set the label to equal the buffer
			strcpy(m_slots[id]->label, m_editBuffer);

			// Now actually save the game
			g_mission->Save_game_position(buff, m_slots[id]->label, m_slots[id]->secondsPlayed);
			// Aint32 with a thumbnail
			SaveThumbnail(id);
			LoadVisibleThumbnails();

			memset(m_editBuffer, '\0', MAX_LABEL_LENGTH);
			m_editing = FALSE8;
			m_GAMESLOT_selected = RETURN;
		} else
		    // Delete key
		    if (c == Common::KEYCODE_BACKSPACE) {
			if (m_cursorPos > 0) {
				m_editBuffer[m_cursorPos] = '\0';
				m_cursorPos--;
				m_editBuffer[m_cursorPos] = '\0';
			}

			// Cancel this
			m_defaultWiper = FALSE8;
		} else if ((c < 32) || (c > 'z')) {
			// Ignore these keypresses
		} else {
			// I like this behaviour of overwriting the entire name on first key
			if (m_defaultWiper) {
				// Wipe contents
				memset(m_editBuffer, '\0', MAX_LABEL_LENGTH);
				m_cursorPos = 0;
				m_defaultWiper = FALSE8;
			}

			// Only add the character if we have room left
			if (m_cursorPos < MAX_LABEL_LENGTH - 2) {
				m_editBuffer[m_cursorPos++] = c;
			}
		}
	}

	// Cursor display
	flash++;
	if (flash < 7)
		m_editBuffer[m_cursorPos] = '_';
	else
		m_editBuffer[m_cursorPos] = ' ';

	// Cycling
	if (flash == 14)
		flash = 0;
}

void OptionsManager::InitialiseSlots() {
	char buff[128];

	// Set all slots by default to empty
	for (uint32 i = 0; i < TOTAL_NUMBER_OF_GAME_SLOTS; i++) {
		m_slots[i] = NULL;

		// Now look for any valid game files and set corresponding info

		// Get filename
		MakeFullSaveFilename(i, buff);

		// If no file exists then end iteration
		if (!checkFileExists(buff))
			continue;

		g_lastAccessedSlot = 0;

		// This slot is in use so record largest id
		g_largestValidSlotID = i;

		// Now we must get the label and time from the file and set accordingly
		Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(buff);

		if (stream == NULL)
			Fatal_error(pxVString("Failed to open save file: %s", buff));

		// Get storage for this slot
		m_slots[i] = new _SLOT;

		// Now access the first two pieces of data that we need for the slot data
		stream->read(m_slots[i]->label, MAX_LABEL_LENGTH);
		m_slots[i]->secondsPlayed = stream->readUint32LE();

		delete stream;
	}

	// Display the page containing the last modified save slot
	m_slotOffset = (g_lastAccessedSlot / NUMBER_OF_VISIBLE_GAME_SLOTS) * NUMBER_OF_VISIBLE_GAME_SLOTS;
	m_GAMESLOT_selected = (GAMESLOT_CHOICES)(g_lastAccessedSlot % NUMBER_OF_VISIBLE_GAME_SLOTS);
}

void OptionsManager::DestroySlots() {
	for (uint32 i = 0; i < TOTAL_NUMBER_OF_GAME_SLOTS; i++) {
		if (m_slots[i] != NULL) {
			delete m_slots[i];
			m_slots[i] = NULL;
		}
	}
}

void OptionsManager::DrawGameSlots(uint32 slotOffset, uint32 surface_id /* = working_buffer_id*/) {
	const char *msg = NULL;
	uint32 temp;
	pxString str;

	uint32 h = 130;
	uint32 id_wPos = 150;
	uint32 nm_wPos = 240;
	uint32 vspacing = 62;

	// Need refresh when editing
	if (m_editing)
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &m_slotBoundingRect, &m_slotBoundingRect);

	// Lock the directdraw surface (working buffer)
	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	// Loop through the visible slots on the screen and do the shit
	for (int32 vslot = 0; vslot < NUMBER_OF_VISIBLE_GAME_SLOTS; vslot++) {
		GAMESLOT_CHOICES slot = (GAMESLOT_CHOICES)vslot;

		// Display slot id
		str.Format("%d.", slotOffset + slot + 1);
		temp = CalculateStringWidth(str);
		DisplayText(ad, pitch, str, id_wPos - temp, h, (bool8)(m_GAMESLOT_selected == slot) ? SELECTEDFONT : NORMALFONT, FALSE8);

		// Now draw the bounding rectangle for a thumbnail image
		DrawRectangle((bool8)(m_GAMESLOT_selected == slot), id_wPos + 9, h - 1, 65, 49, ad, pitch);

		// Followed by label and time played if the slot is in use
		if (m_slots[slotOffset + slot] == NULL) {
			if ((m_editing) && (m_GAMESLOT_selected == slot)) {
				// Unlock the surface for the thumbnail blit
				surface_manager->Unlock_surface(surface_id);

				EditSlotLabel();

				// Lock it back up again and continue
				ad = surface_manager->Lock_surface(surface_id);
				pitch = surface_manager->Get_pitch(surface_id);

				DisplayText(ad, pitch, m_editBuffer, nm_wPos, h, (bool8)(m_GAMESLOT_selected == slot) ? SELECTEDFONT : NORMALFONT, FALSE8);
			} else {
				msg = GetTextFromReference(HashString("opt_empty"));
				DisplayText(ad, pitch, msg, nm_wPos, h, (bool8)(m_GAMESLOT_selected == slot) ? SELECTEDFONT : NORMALFONT, FALSE8);
			}

			DisplayText(ad, pitch, "00:00:00", nm_wPos, h + 18, (bool8)(m_GAMESLOT_selected == slot) ? SELECTEDFONT : NORMALFONT, FALSE8);
		} else {
			if ((m_editing) && (m_GAMESLOT_selected == slot)) {
				// Unlock the surface for the thumbnail blit
				surface_manager->Unlock_surface(surface_id);

				EditSlotLabel();

				// Lock it back up again and continue
				ad = surface_manager->Lock_surface(surface_id);
				pitch = surface_manager->Get_pitch(surface_id);

				DisplayText(ad, pitch, m_editBuffer, nm_wPos, h, (bool8)(m_GAMESLOT_selected == slot) ? SELECTEDFONT : NORMALFONT, FALSE8);
			} else {
				msg = m_slots[slotOffset + slot]->label;
				DisplayText(ad, pitch, msg, nm_wPos, h, (bool8)(m_GAMESLOT_selected == slot) ? SELECTEDFONT : NORMALFONT, FALSE8);

				// The thumbnail
				LRECT dest;
				dest.left = id_wPos + 10;
				dest.right = dest.left + 64;
				dest.top = h;
				dest.bottom = dest.top + 48;

				// Unlock the surface for the thumbnail blit
				surface_manager->Unlock_surface(surface_id);

				// Are we to draw current thumbnails or the preloaded next thumbnails (paging animation)
				if (slotOffset == m_slotOffset) {
					if (m_GAMESLOT_selected == slot)
						surface_manager->Blit_surface_to_surface(m_thumbSurfaceIDs[slot], surface_id, NULL, &dest);
					else
						surface_manager->Blit_surface_to_surface(m_grayThumbSurfaceIDs[slot], surface_id, NULL, &dest);
				} else {
					if (m_GAMESLOT_selected == slot)
						surface_manager->Blit_surface_to_surface(m_thumbSurfaceIDs[slot + 4], surface_id, NULL, &dest);
					else
						surface_manager->Blit_surface_to_surface(m_grayThumbSurfaceIDs[slot + 4], surface_id, NULL, &dest);
				}

				// Lock it back up again and continue
				ad = surface_manager->Lock_surface(surface_id);
				pitch = surface_manager->Get_pitch(surface_id);
			}

			uint32 sec = 0;
			if (m_slots[slotOffset + slot])
				sec = m_slots[slotOffset + slot]->secondsPlayed;
			uint32 min = sec / 60;
			uint32 hrs = min / 60;
			// Get residual seconds
			sec = sec % 60;
			str.Format("%d%d:%d%d:%d%d", (hrs < 10) ? 0 : (hrs / 10), (hrs % 10), ((min % 60) < 10) ? 0 : ((min % 60) / 10), (min % 60) % 10, (sec < 10) ? 0 : sec / 10,
			           (sec < 10) ? sec : sec % 10);
			DisplayText(ad, pitch, str, nm_wPos, h + 18, (bool8)(m_GAMESLOT_selected == slot) ? SELECTEDFONT : NORMALFONT, FALSE8);
		}

		h += vspacing;
	}

	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawMovieSlots(uint32 offset, uint32 surface_id /* = working_buffer_id*/) {
	const char *msg = NULL;
	LRECT dest;
	uint32 selectedMovie;
	uint32 selectedMovieSlot;

	// Now draw the movie slots as three rows of four and the size ratio is 32\5 to that of
	// original movies(rendered 640x356 pixels) giving thumnail dimensions of 100x56

	uint32 firstRowY = 130;

	uint8 *ad;
	uint32 pitch;

	for (uint32 row = 0; row < 3; row++) {
		uint32 firstColumnX = 89;

		selectedMovieSlot = (row * 4);
		selectedMovie = selectedMovieSlot + offset;

		// Check we don't exceed the total number of movies (which is dynamic at the moment)
		if (selectedMovie == TOTAL_NUMBER_OF_MOVIES)
			break;

		ad = surface_manager->Lock_surface(surface_id);
		pitch = surface_manager->Get_pitch(surface_id);

		DrawRectangle((bool8)(m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot), firstColumnX, firstRowY, 101, 57, ad, pitch);

		surface_manager->Unlock_surface(surface_id);

		// The piccy
		dest.left = firstColumnX + 1;
		dest.right = dest.left + 100;
		dest.top = firstRowY + 1;
		dest.bottom = dest.top + 56;
		if (g_movieLibrary[selectedMovie].visible) {
			if (offset == m_movieOffset) {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
			} else {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
			}
		}
		firstColumnX += 120;

		selectedMovie++;
		selectedMovieSlot++;

		// Check we don't exceed the total number of movies
		if (selectedMovie == TOTAL_NUMBER_OF_MOVIES)
			break;

		ad = surface_manager->Lock_surface(surface_id);
		pitch = surface_manager->Get_pitch(surface_id);

		DrawRectangle((bool8)(m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot), firstColumnX, firstRowY, 101, 57, ad, pitch);

		surface_manager->Unlock_surface(surface_id);

		// The piccy
		dest.left = firstColumnX + 1;
		dest.right = dest.left + 100;
		dest.top = firstRowY + 1;
		dest.bottom = dest.top + 56;
		if (g_movieLibrary[selectedMovie].visible) {
			if (offset == m_movieOffset) {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
			} else {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
			}
		}
		firstColumnX += 120;

		selectedMovie++;
		selectedMovieSlot++;

		// Check we don't exceed the total number of movies
		if (selectedMovie == TOTAL_NUMBER_OF_MOVIES)
			break;

		ad = surface_manager->Lock_surface(surface_id);
		pitch = surface_manager->Get_pitch(surface_id);

		DrawRectangle((bool8)(m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot), firstColumnX, firstRowY, 101, 57, ad, pitch);

		surface_manager->Unlock_surface(surface_id);

		// The piccy
		dest.left = firstColumnX + 1;
		dest.right = dest.left + 100;
		dest.top = firstRowY + 1;
		dest.bottom = dest.top + 56;
		if (g_movieLibrary[selectedMovie].visible) {
			if (offset == m_movieOffset) {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
			} else {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
			}
		}
		firstColumnX += 120;

		selectedMovie++;
		selectedMovieSlot++;

		// Check we don't exceed the total number of movies
		if (selectedMovie == TOTAL_NUMBER_OF_MOVIES)
			break;

		ad = surface_manager->Lock_surface(surface_id);
		pitch = surface_manager->Get_pitch(surface_id);

		DrawRectangle((bool8)(m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot), firstColumnX, firstRowY, 101, 57, ad, pitch);

		surface_manager->Unlock_surface(surface_id);

		// The piccy
		dest.left = firstColumnX + 1;
		dest.right = dest.left + 100;
		dest.top = firstRowY + 1;
		dest.bottom = dest.top + 56;
		if (g_movieLibrary[selectedMovie].visible) {
			if (offset == m_movieOffset) {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot], surface_id, NULL, &dest);
			} else {
				if (m_M_MOVIE_selected == (M_MOVIE_CHOICES)selectedMovieSlot)
					surface_manager->Blit_surface_to_surface(m_movieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
				else
					surface_manager->Blit_surface_to_surface(m_grayMovieSurfaceIDs[selectedMovieSlot + 12], surface_id, NULL, &dest);
			}
		}
		firstRowY += 75;
	}

	ad = surface_manager->Lock_surface(surface_id);
	pitch = surface_manager->Get_pitch(surface_id);

	// Display movies name if allowed
	if (m_M_MOVIE_selected < NOTHANKS) {
		uint32 selectMovie = m_M_MOVIE_selected + offset;

		// Is this movie availble
		if (g_movieLibrary[selectMovie].visible) {
			// Get the nice name to go with the nice piccy
			if (selectMovie < 10)
				msg = GetTextFromReference(HashString(pxVString("opt_movie0%d", selectMovie)));
			else
				msg = GetTextFromReference(HashString(pxVString("opt_movie%d", selectMovie)));

			if (msg)
				DisplayText(ad, pitch, msg, 0, 355, PALEFONT, TRUE8);
			else
				DisplayText(ad, pitch, "???", 0, 355, PALEFONT, TRUE8);
		} else {
			// Nope, you'll have to play the game more dude
			DisplayText(ad, pitch, "???", 0, 355, PALEFONT, TRUE8);
		}
	}

	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawMainLoadScreen(uint32 surface_id) {
	const char *msg = NULL;
	uint32 temp;
	pxString str;
	int32 icon_x = 60;
	int32 icon_y = (SCREEN_DEPTH / 2) - (m_fontHeight / 2);

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	SetDrawColour(BASE);
	msg = GetTextFromReference(HashString("opt_loadgame"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	// Draw the non-selectable paging icons
	if (m_slotOffset != 0) {
		temp = CalculateStringWidth("<");
		DrawRectangle((bool8)(m_paging && m_pageleft), icon_x, icon_y, temp + 18, m_fontHeight - 2, ad, pitch);
		DisplayText(ad, pitch, "<", icon_x + 10, icon_y - 2, (m_paging && m_pageleft) ? SELECTEDFONT : NORMALFONT, FALSE8);
	}

	if (m_slotOffset < TOTAL_NUMBER_OF_GAME_SLOTS - NUMBER_OF_VISIBLE_GAME_SLOTS) {
		temp = CalculateStringWidth(">");
		icon_x = SCREEN_WIDTH - icon_x - temp - 18;
		DrawRectangle((bool8)(m_paging && !m_pageleft), icon_x, icon_y, temp + 18, m_fontHeight - 2, ad, pitch);
		DisplayText(ad, pitch, ">", icon_x + 10, icon_y - 2, (m_paging && !m_pageleft) ? SELECTEDFONT : NORMALFONT, FALSE8);
	}

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, 378, (m_GAMESLOT_selected == RETURN) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(surface_id);

	// Now the slots themselves
	if (!m_paging) {
		// Regular
		DrawGameSlots(m_slotOffset, surface_id);
	} else {
		// Animating
		AnimateSlotsPaging();
	}
}

void OptionsManager::DrawMovieScreen(uint32 surface_id) {
	const char *msg = NULL;
	uint32 temp;
	int32 icon_x = 40;
	int32 icon_y = (SCREEN_DEPTH / 2) - (m_fontHeight / 2);

	// Draw the slots themselves
	if (!m_paging) {
		// Regular
		DrawMovieSlots(m_movieOffset, surface_id);
	} else {
		// Animating
		AnimateSlotsPaging();
	}

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	SetDrawColour(BASE);
	msg = GetTextFromReference(HashString("opt_movies"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	// Draw the selectable paging icons after the slots

	// Only draw the page left icon if we can page left
	if (m_movieOffset != 0) {
		temp = CalculateStringWidth("<");
		DrawRectangle((bool8)(m_M_MOVIE_selected == PAGELEFT), icon_x, icon_y, temp + 18, m_fontHeight - 2, ad, pitch);
		DisplayText(ad, pitch, "<", icon_x + 10, icon_y - 2, (m_M_MOVIE_selected == PAGELEFT) ? SELECTEDFONT : NORMALFONT, FALSE8);
	}
	// Only draw the page right icon if we can page right
	if (m_movieOffset < TOTAL_NUMBER_OF_MOVIES - M_NUMBER_OF_VISIBLE_MOVIE_SLOTS) {
		temp = CalculateStringWidth(">");
		icon_x = SCREEN_WIDTH - icon_x - temp - 18;
		DrawRectangle((bool8)(m_M_MOVIE_selected == PAGERIGHT), icon_x, icon_y, temp + 18, m_fontHeight - 2, ad, pitch);
		DisplayText(ad, pitch, ">", icon_x + 10, icon_y - 2, (m_M_MOVIE_selected == PAGERIGHT) ? SELECTEDFONT : NORMALFONT, FALSE8);
	}

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, 385, (m_M_MOVIE_selected == NOTHANKS) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawExtrasScreen(uint32 surface_id) {
	const char *msg = NULL;

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	SetDrawColour(BASE);
	msg = GetTextFromReference(HashString("opt_extras"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	msg = GetTextFromReference(HashString("opt_movies"));
	DisplayText(ad, pitch, msg, 0, 130, (m_M_EXTRA_selected == MOVIES) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = GetTextFromReference(HashString("opt_slideshow"));
	if (px.game_completed)
		DisplayText(ad, pitch, msg, 0, 150, (m_M_EXTRA_selected == SLIDESHOW) ? SELECTEDFONT : NORMALFONT, TRUE8);
	else
		DisplayText(ad, pitch, msg, 0, 150, PALEFONT, TRUE8);
	msg = GetTextFromReference(HashString("opt_playselect"));
	if (px.game_completed)
		DisplayText(ad, pitch, msg, 0, 170, (m_M_EXTRA_selected == PLAYSELECT) ? SELECTEDFONT : NORMALFONT, TRUE8);
	else
		DisplayText(ad, pitch, msg, 0, 170, PALEFONT, TRUE8);
	msg = GetTextFromReference(HashString("opt_profiles"));
	if (px.game_completed)
		DisplayText(ad, pitch, msg, 0, 190, (m_M_EXTRA_selected == PROFILES) ? SELECTEDFONT : NORMALFONT, TRUE8);
	else
		DisplayText(ad, pitch, msg, 0, 190, PALEFONT, TRUE8);
	msg = GetTextFromReference(HashString("opt_credits"));
	DisplayText(ad, pitch, msg, 0, 210, (m_M_EXTRA_selected == CREDITS) ? SELECTEDFONT : NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, 240, (m_M_EXTRA_selected == ALLDONE) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawWidescreenBorders() {
	Fill_rect(0, 0, SCREEN_WIDTH, 67, 0);
	Fill_rect(0, SCREEN_DEPTH - 67, SCREEN_WIDTH, SCREEN_DEPTH, 0);
}

void OptionsManager::DrawPlaySelectScreen(uint32 surface_id) {
	const char *msg = NULL;

	// Some of this is disabled for demos
	int demo = g_globalScriptVariables.GetVariable("demo");

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	SetDrawColour(BASE);
	msg = GetTextFromReference(HashString("opt_playselect"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	if (demo) {
		msg = GetTextFromReference(HashString("opt_investigatemine"));
		DisplayText(ad, pitch, msg, 0, 130, (m_M_PLAYSELECT_selected == M01) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_securityhq"));
		DisplayText(ad, pitch, msg, 0, 150, PALEFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_landtrain"));
		DisplayText(ad, pitch, msg, 0, 170, PALEFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_containment"));
		DisplayText(ad, pitch, msg, 0, 190, (m_M_PLAYSELECT_selected == M04) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_refinery"));
		DisplayText(ad, pitch, msg, 0, 210, PALEFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_mainlandbase"));
		DisplayText(ad, pitch, msg, 0, 230, (m_M_PLAYSELECT_selected == M07) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_islandbase"));
		DisplayText(ad, pitch, msg, 0, 250, PALEFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_escape"));
		DisplayText(ad, pitch, msg, 0, 270, PALEFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_submarine"));
		DisplayText(ad, pitch, msg, 0, 290, PALEFONT, TRUE8);
	} else {
		msg = GetTextFromReference(HashString("opt_investigatemine"));
		DisplayText(ad, pitch, msg, 0, 130, (m_M_PLAYSELECT_selected == M01) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_securityhq"));
		DisplayText(ad, pitch, msg, 0, 150, (m_M_PLAYSELECT_selected == M02) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_landtrain"));
		DisplayText(ad, pitch, msg, 0, 170, (m_M_PLAYSELECT_selected == M03) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_containment"));
		DisplayText(ad, pitch, msg, 0, 190, (m_M_PLAYSELECT_selected == M04) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_refinery"));
		DisplayText(ad, pitch, msg, 0, 210, (m_M_PLAYSELECT_selected == M05) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_mainlandbase"));
		DisplayText(ad, pitch, msg, 0, 230, (m_M_PLAYSELECT_selected == M07) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_islandbase"));
		DisplayText(ad, pitch, msg, 0, 250, (m_M_PLAYSELECT_selected == M08) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_escape"));
		DisplayText(ad, pitch, msg, 0, 270, (m_M_PLAYSELECT_selected == M09) ? SELECTEDFONT : NORMALFONT, TRUE8);
		msg = GetTextFromReference(HashString("opt_submarine"));
		DisplayText(ad, pitch, msg, 0, 290, (m_M_PLAYSELECT_selected == M10) ? SELECTEDFONT : NORMALFONT, TRUE8);
	}

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, 330, (m_M_PLAYSELECT_selected == CANCEL) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawProfileSelectScreen(uint32 surface_id) {
	const char *msg = NULL;

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	SetDrawColour(BASE);
	msg = GetTextFromReference(HashString("opt_profiles"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	msg = "Cord";
	DisplayText(ad, pitch, msg, 0, 130, (m_M_PROFILES_selected == CORD) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Chi";
	DisplayText(ad, pitch, msg, 0, 150, (m_M_PROFILES_selected == CHI) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Gregor";
	DisplayText(ad, pitch, msg, 0, 170, (m_M_PROFILES_selected == GREGOR) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Nagarov";
	DisplayText(ad, pitch, msg, 0, 190, (m_M_PROFILES_selected == NAGAROV) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Lukyan";
	DisplayText(ad, pitch, msg, 0, 210, (m_M_PROFILES_selected == LUKYAN) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Keiffer";
	DisplayText(ad, pitch, msg, 0, 230, (m_M_PROFILES_selected == KEIFFER) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Tolstov";
	DisplayText(ad, pitch, msg, 0, 250, (m_M_PROFILES_selected == TOLSTOV) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Alexandra";
	DisplayText(ad, pitch, msg, 0, 270, (m_M_PROFILES_selected == ALEXANDRA) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Oliakov";
	DisplayText(ad, pitch, msg, 0, 290, (m_M_PROFILES_selected == OLIAKOV) ? SELECTEDFONT : NORMALFONT, TRUE8);
	msg = "Spectre";
	DisplayText(ad, pitch, msg, 0, 310, (m_M_PROFILES_selected == SPECTRE) ? SELECTEDFONT : NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, 350, (m_M_PROFILES_selected == RET) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::InitialiseAProfile() {
	// Check for outfit select
	int outfit_cheat = 0;
	if (Read_DI_keys(Common::KEYCODE_1))
		outfit_cheat = 1;
	if (Read_DI_keys(Common::KEYCODE_2))
		outfit_cheat = 2;
	if (Read_DI_keys(Common::KEYCODE_3))
		outfit_cheat = 3;
	if (Read_DI_keys(Common::KEYCODE_4))
		outfit_cheat = 4;

	// Need to calculate printing margin
	m_margin = 0;
	const char *msg = NULL;

	// This ensures correct spacing for any translations
	for (uint i = 0; i < 5; i++) {
		switch (i) {
		case 0:
			msg = GetTextFromReference(HashString("prf_name"));
			break;
		case 1:
			msg = GetTextFromReference(HashString("prf_age"));
			break;
		case 2:
			msg = GetTextFromReference(HashString("prf_height"));
			break;
		case 3:
			msg = GetTextFromReference(HashString("prf_weight"));
			break;
		case 4:
			msg = GetTextFromReference(HashString("prf_profile"));
			break;
		}

		uint32 width;
		width = CalculateStringWidth(msg);

		if (width > m_margin)
			m_margin = width;
	}

	// Cool now add some spacing
	m_margin += 10;

	// Rectangle describing the profile text box
	m_profileRect.left = m_margin + 20;
	m_profileRect.right = m_profileRect.left + 285;
	m_profileRect.top = 188;
	m_profileRect.bottom = m_profileRect.top + 184;

	// Initialise scrolling offsets
	m_profileScrollingOffset = 0;
	m_profileScrollingLine = -1;

	// Setup the character drawing stuff here using m_M_PROFILES_selected ...
	const char *character_name = NULL;
	const char *outfit_name = "outfit";
	const char *anim_name = "walk";
	int16 rx = 0;
	int16 ry = 0;
	int16 rz = 0;

	switch (m_M_PROFILES_selected) {
	case CORD:
		character_name = "cord";
		if (outfit_cheat == 1)
			outfit_name = "flack_jacket";
		else if (outfit_cheat == 2)
			outfit_name = "arctic_wear";
		else if (outfit_cheat == 3)
			outfit_name = "tech_suit";
		else if (outfit_cheat == 4)
			outfit_name = "vest";
		else
			outfit_name = "casual_wear";
		rx = -55;
		ry = -23;
		rz = -348;
		break;
	case CHI:
		character_name = "chi";
		if (outfit_cheat == 1)
			outfit_name = "casual_wear";
		else if (outfit_cheat == 2)
			outfit_name = "menial_wear";
		else
			outfit_name = "arctic_wear";
		rx = -55;
		ry = -26;
		rz = -348;
		break;
	case GREGOR:
		character_name = "gregor";
		outfit_name = "combats";
		rx = -55;
		ry = -18;
		rz = -210;
		break;
	case NAGAROV:
		character_name = "nagarov";
		outfit_name = "uniform";
		rx = -55;
		ry = -20;
		rz = -308;
		break;
	case LUKYAN:
		character_name = "lukyan";
		outfit_name = "uniform";
		rx = -55;
		ry = -17;
		rz = -210;
		break;
	case KEIFFER:
		character_name = "keiffer";
		rx = -55;
		ry = -23;
		rz = -348;
		break;
	case TOLSTOV:
		character_name = "scientist_contact";
		outfit_name = "labcoat";
		rx = -55;
		ry = -20;
		rz = -338;
		break;
	case ALEXANDRA:
		character_name = "scientists_daughter";
		rx = -55;
		ry = -30;
		rz = -368;
		break;
	case OLIAKOV:
		character_name = "oliakov";
		rx = -55;
		ry = -17;
		rz = -280;
		break;
	case SPECTRE:
		character_name = "spectre";
		rx = -28;
		ry = -10;
		break;

	default:
		Fatal_error("Can't draw unknown character profile!");
	}

	InitActorView(character_name, outfit_name, "unarmed", anim_name, rx, ry, rz);

	// Ensure these flags are set for the polgon renderer
	_drawActor = 1;
	_drawPolys = 1;
	_drawTxture = 1;
	_drawBbox = 0;
	_drawWfrm = 0;
	_drawLit = 1;
}

void OptionsManager::DrawProfileScreen(uint32 surface_id) {
	// Timing code to lock the refresh rate at a constant
	uint32 t = GetMicroTimer();

	const char *msg = NULL;
	const char *prefix = NULL;
	uint32 temp;
	pxString label;

	// Need this mainly for rabview reasons
	DrawWidescreenBorders();

	// Draw the actor first up
	ActorViewDraw();

	// Whose profile are we drawing
	switch (m_M_PROFILES_selected) {
	case CORD:
		prefix = "prf_cord_";
		break;
	case CHI:
		prefix = "prf_chi_";
		break;
	case GREGOR:
		prefix = "prf_gregor_";
		break;
	case NAGAROV:
		prefix = "prf_nagarov_";
		break;
	case LUKYAN:
		prefix = "prf_lukyan_";
		break;
	case KEIFFER:
		prefix = "prf_keiffer_";
		break;
	case TOLSTOV:
		prefix = "prf_tolstov_";
		break;
	case ALEXANDRA:
		prefix = "prf_alexandra_";
		break;
	case OLIAKOV:
		prefix = "prf_oliakov_";
		break;
	case SPECTRE:
		prefix = "prf_spectre_";
		break;

	default:
		Fatal_error("Can't draw unknown profile!");
	}

	uint32 leftMargin = 20;

	// Now we need to parse the info string word by word writing to the screen until we
	// need a new line

	// Get the whole string
	label.Format("%s%s", prefix, "info");
	msg = GetTextFromReference(HashString(label));

	if (msg == NULL)
		msg = "Please update 'globals\\translations\\' files";

	// Get some storage from the stack
	uint8 theData[MAX_BYTESIZE_OF_PROFILE_INFO];
	// Zero out our memory
	memset(theData, 0, MAX_BYTESIZE_OF_PROFILE_INFO);
	// Make a personal copy
	memcpy(theData, msg, strlen(msg) + 1);
	// Get a pointer to our memory
	char *ptr = (char *)theData;

	// Split the text into words (overwrite spaces with terminators)
	int i = 0;
	uint32 numberOfWords = 1;
	while (ptr[i]) {
		// Found a space?
		if (ptr[i] == ' ') {
			// Watch for multiple spaces!
			do {
				ptr[i] = 0;
				i++;

			} while (ptr[i] == ' ');

			numberOfWords++;
		} else
			i++;
	}

	// Positional cursors initialised
	uint32 xp = 0;
	uint32 yp;
	if (m_profileScrollingLine != -1)
		yp = 20 - m_profileScrollingOffset;
	else
		yp = 40 - m_profileScrollingOffset;

	uint32 RIGHT_HAND_MARGIN = surface_manager->Get_width(m_profileSurface);
	uint32 BOTTOM_EDGE_LIMIT = surface_manager->Get_height(m_profileSurface) - 20;
	uint32 SPACE_PIXELWIDTH = 5;
	uint32 cur = 0;

	// Black is the colour key for this surface
	surface_manager->Clear_surface(m_profileSurface);

	uint8 *ad = surface_manager->Lock_surface(m_profileSurface);
	uint32 pitch = surface_manager->Get_pitch(m_profileSurface);

	int line;
	if (m_profileScrollingLine != -1)
		line = 0;
	else
		line = -1;

	// Now we're in a position to display it word by word
	for (uint32 w = 0; w < numberOfWords; w++) {
		// Safety check
		if (cur >= sizeof(theData))
			break;

		// Print a word
		if (line >= m_profileScrollingLine)
			DisplayText(ad, pitch, (const char *)(ptr + cur), xp, yp, NORMALFONT, FALSE8);

		// Move cursor(s)
		xp += CalculateStringWidth((const char *)(ptr + cur)) + SPACE_PIXELWIDTH;

		// Last word already printed so break
		if (w + 1 == numberOfWords) {
			// Do we need to scroll a little more to display the last line properly
			if (yp < BOTTOM_EDGE_LIMIT - 39)
				m_lastLineDisplayed = TRUE8;
			else
				m_lastLineDisplayed = FALSE8;

			break;
		}

		// Point to next word
		cur += strlen((const char *)(ptr + cur));
		while (!ptr[cur])
			cur++;

		// Calculate pixel length of NEXT word
		uint32 nextw = CalculateStringWidth((const char *)(ptr + cur));

		// Check for line ends
		if (xp + nextw > RIGHT_HAND_MARGIN) {
			// Need to go to next line
			if (line >= m_profileScrollingLine)
				yp += 20;

			xp = 0;

			line++;

			// Check we've some screen left
			if (yp > BOTTOM_EDGE_LIMIT) {
				// Can't draw anymore
				m_lastLineDisplayed = FALSE8;
				break;
			}
		}
	}

	// Now do the fadey bit
	FadeStrip(0, 28, RIGHT_HAND_MARGIN + 1, TRUE8, ad, pitch);
	FadeStrip(0, surface_manager->Get_height(m_profileSurface) - 43, RIGHT_HAND_MARGIN + 1, FALSE8, ad, pitch);

	surface_manager->Unlock_surface(m_profileSurface);

	LRECT srcr;
	srcr.left = 0;
	srcr.top = 28;
	srcr.right = m_profileRect.right - m_profileRect.left;
	srcr.bottom = srcr.top + m_profileRect.bottom - m_profileRect.top;

	// Now blit the sucker to it's house with transparency
	surface_manager->Blit_surface_to_surface(m_profileSurface, working_buffer_id, &srcr, &m_profileRect, DDBLT_KEYSRC);

	// Draw to the correct surface now
	ad = surface_manager->Lock_surface(surface_id);
	pitch = surface_manager->Get_pitch(surface_id);

	// Draw the headings
	msg = GetTextFromReference(HashString("prf_name"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, leftMargin + m_margin - temp - 10, 90, PALEFONT, FALSE8);
	msg = GetTextFromReference(HashString("prf_age"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, leftMargin + m_margin - temp - 10, 110, PALEFONT, FALSE8);
	msg = GetTextFromReference(HashString("prf_height"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, leftMargin + m_margin - temp - 10, 130, PALEFONT, FALSE8);
	msg = GetTextFromReference(HashString("prf_weight"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, leftMargin + m_margin - temp - 10, 150, PALEFONT, FALSE8);
	msg = GetTextFromReference(HashString("prf_profile"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, leftMargin + m_margin - temp - 10, 200, PALEFONT, FALSE8);

	// Now the vitals for this character (using margin value for safety)
	label.Format("%s%s", prefix, "name");
	msg = GetTextFromReference(HashString(label));
	DisplayText(ad, pitch, msg, leftMargin + m_margin, 90, NORMALFONT, FALSE8);
	label.Format("%s%s", prefix, "age");
	msg = GetTextFromReference(HashString(label));
	DisplayText(ad, pitch, msg, leftMargin + m_margin, 110, NORMALFONT, FALSE8);
	label.Format("%s%s", prefix, "height");
	msg = GetTextFromReference(HashString(label));
	DisplayText(ad, pitch, msg, leftMargin + m_margin, 130, NORMALFONT, FALSE8);
	label.Format("%s%s", prefix, "weight");
	msg = GetTextFromReference(HashString(label));
	DisplayText(ad, pitch, msg, leftMargin + m_margin, 150, NORMALFONT, FALSE8);

	// Draw the paging indicators
	uint32 tlx = leftMargin + m_margin - 25;
	uint32 tly = 338;
	if (!(m_profileScrollingLine == -1 && m_profileScrollingOffset == 0))
		DrawPageIndicator(tlx, tly, TRUE8, (bool8)(m_moveLimiter && m_profileScrolling < 0 ? TRUE8 : FALSE8), ad, pitch);
	if (m_lastLineDisplayed == FALSE8)
		DrawPageIndicator(tlx, tly + 12, FALSE8, (bool8)(m_moveLimiter && m_profileScrolling > 0 ? TRUE8 : FALSE8), ad, pitch);

	// This is permanently selected on these profile screens
	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, SCREEN_DEPTH - 90, SELECTEDFONT, TRUE8);

	surface_manager->Unlock_surface(surface_id);

	// Timing code to lock the refresh rate at a constant
	t = GetMicroTimer() - t;
	int32 r = 40000 - t;
	if (t < 40000) {
		g_system->delayMillis(r / 1000);
	}
}

void OptionsManager::FadeStrip(uint32 x, uint32 y, uint32 w, bool8 up, uint8 *ad, uint32 pitch) {
	uint8 subtractive[8];
	uint32 *safe_ad = (uint32 *)ad;

	uint8 initialFade = 0xE6; // 230
	uint8 finalFade = 0x05;   // 5

	// Move to starting pixel
	safe_ad += (y * (pitch / 4));
	safe_ad += x;
	uint8 *pixels = (uint8 *)safe_ad;

	if (up) {
		// Initial fade amount == 230
		subtractive[4] = subtractive[0] = initialFade;
		subtractive[5] = subtractive[1] = initialFade;
		subtractive[6] = subtractive[2] = initialFade;
		subtractive[7] = subtractive[3] = 0x00;
	} else {
		// Initial fade amount == 5
		subtractive[4] = subtractive[0] = finalFade;
		subtractive[5] = subtractive[1] = finalFade;
		subtractive[6] = subtractive[2] = finalFade;
		subtractive[7] = subtractive[3] = 0x00;
	}

#if 1
	for (uint lines = 0; lines < 15; lines++) {
		for (uint xPos = 0; xPos < w; xPos++) {
			// 32-bit BGRA pixel
			uint8 *pixel = &pixels[xPos * 4];
			// Subtract from RGB components
			for (int i = 0; i < 3; i++) {
				pixel[i] = MAX(0, pixel[i] - subtractive[i]);
			}
		}
		// Next line
		pixels += pitch;

		// Change fade amounts
		if (up) {
			subtractive[4] = subtractive[0] = (uint8)(initialFade - (lines * 15));
			subtractive[5] = subtractive[1] = (uint8)(initialFade - (lines * 15));
			subtractive[6] = subtractive[2] = (uint8)(initialFade - (lines * 15));
			subtractive[7] = subtractive[3] = 0x00;
		} else {
			subtractive[4] = subtractive[0] = (uint8)(finalFade + (lines * 15));
			subtractive[5] = subtractive[1] = (uint8)(finalFade + (lines * 15));
			subtractive[6] = subtractive[2] = (uint8)(finalFade + (lines * 15));
			subtractive[7] = subtractive[3] = 0x00;
		}
	}
#else
	int pixelPairs = w / 2;
	for (uint32 lines = 0; lines < 15; lines++) {
		_asm {
			lea  edi, subtractive   ; // Load the address of the blend colour block
			mov  ecx, pixelPairs    ; // Pixel Counter (2 pixels at a time mind)
			mov  esi, safe_ad       ; // Load the address of the pixels
			movq MM0, [edi]         ; // Put address of the blend colour block into MMX register


			subtractive_fade_loop:

			movq    MM1, [esi]              ; // Load 2 pixels
			psubusb MM1, MM0                ; // Do the subtract
			movq    [esi], MM1              ; // Store the result
			add     esi, 8                  ; // Move pixel pointer on
			dec     ecx                     ; // Reduce counter
			jne     subtractive_fade_loop   ; // On to the next 2 pixels

			EMMS    ; // Clear/Set MMX/FPU flag
		}

		safe_ad += (pitch / 4);

		// Change fade amounts
		if (up) {
			subtractive[4] = subtractive[0] = (uint8)(initialFade - (lines * 15));
			subtractive[5] = subtractive[1] = (uint8)(initialFade - (lines * 15));
			subtractive[6] = subtractive[2] = (uint8)(initialFade - (lines * 15));
			subtractive[7] = subtractive[3] = 0x00;
		} else {
			subtractive[4] = subtractive[0] = (uint8)(finalFade + (lines * 15));
			subtractive[5] = subtractive[1] = (uint8)(finalFade + (lines * 15));
			subtractive[6] = subtractive[2] = (uint8)(finalFade + (lines * 15));
			subtractive[7] = subtractive[3] = 0x00;
		}
	}
#endif
}

void OptionsManager::DrawPageIndicator(uint32 x, uint32 y, bool8 up, bool8 selected, uint8 *ad, uint32 pitch) {
	_rgb col;

	if (selected) {
		col.red = 202;
		col.green = 0;
		col.blue = 0;
	} else {
		col.red = 254;
		col.green = 254;
		col.blue = 254;
	}

	if (up) {
		Draw_horizontal_line(x + 7, y, 1, &col, ad, pitch);
		Draw_horizontal_line(x + 6, y + 1, 3, &col, ad, pitch);
		Draw_horizontal_line(x + 5, y + 2, 5, &col, ad, pitch);
		Draw_horizontal_line(x + 4, y + 3, 7, &col, ad, pitch);
		Draw_horizontal_line(x + 3, y + 4, 9, &col, ad, pitch);
		Draw_horizontal_line(x + 2, y + 5, 11, &col, ad, pitch);
		Draw_horizontal_line(x + 1, y + 6, 13, &col, ad, pitch);
	} else {
		Draw_horizontal_line(x + 7, y + 6, 1, &col, ad, pitch);
		Draw_horizontal_line(x + 6, y + 5, 3, &col, ad, pitch);
		Draw_horizontal_line(x + 5, y + 4, 5, &col, ad, pitch);
		Draw_horizontal_line(x + 4, y + 3, 7, &col, ad, pitch);
		Draw_horizontal_line(x + 3, y + 2, 9, &col, ad, pitch);
		Draw_horizontal_line(x + 2, y + 1, 11, &col, ad, pitch);
		Draw_horizontal_line(x + 1, y, 13, &col, ad, pitch);
	}
}

void OptionsManager::GetKeyAssignment() {
	uint32 keypressed = Get_DI_key_press();
	uint8 joystickpressed = GetJoystickButtonPress();
	uint8 joystickAxisPressed = GetJoystickAxisPress();

	// Change selected function using the enter key (so ensure this doesn't get immediately assigned)
	if ((keypressed == Common::KEYCODE_RETURN) && m_configLimiter) {
		// Now allowed to assign a key
		m_configLimiter = FALSE8;
		return;
	}

	if ((joystickpressed == 0) && m_configLimiter) {
		// Now allowed to assign a button
		m_configLimiter = FALSE8;
		// Hacky fuck fuck!
		g_system->delayMillis(200);
		return;
	}

	if (currentJoystick != NO_JOYSTICK) {
		// Are we assigning directional control on the joystick

		if (joystickAxisPressed != 0xFF) {
			if ((m_CONTROL_selected == UP_CROUCH || m_CONTROL_selected == DOWN_INTERACT || m_CONTROL_selected == LEFT_ARM || m_CONTROL_selected == RIGHT_ATTACK) &&
			    m_controlPage1) {
				// Check reassignment
				if (left_joy == joystickAxisPressed)
					left_joy = 0xFF;
				else if (right_joy == joystickAxisPressed)
					right_joy = 0xFF;
				else if (up_joy == joystickAxisPressed)
					up_joy = 0xFF;
				else if (down_joy == joystickAxisPressed)
					down_joy = 0xFF;

				switch (m_CONTROL_selected) {
				case UP_CROUCH:
					up_joy = joystickAxisPressed;
					break;
				case DOWN_INTERACT:
					down_joy = joystickAxisPressed;
					break;
				case LEFT_ARM:
					left_joy = joystickAxisPressed;
					break;
				case RIGHT_ATTACK:
					right_joy = joystickAxisPressed;
					break;

				// Only assign axes to directional control
				default:
					return;
				}

				// Done my shit thanks
				m_awaitingKeyPress = FALSE8;
				m_editing = FALSE8;
				m_configLimiter = TRUE8;

				g_system->delayMillis(200);
				return;
			}

			// Chances are sliders will always return some value so ignore and continue
		}

		// Check for button with priority when we have a joystick

		if (joystickpressed != 0xFF) {
			// Can't assign buttons to directional controls
			if ((m_CONTROL_selected == UP_CROUCH || m_CONTROL_selected == DOWN_INTERACT || m_CONTROL_selected == LEFT_ARM || m_CONTROL_selected == RIGHT_ATTACK) &&
			    m_controlPage1)
				return;

			if (GetButtonName(joystickpressed) == NULL)
				return;

			// Check reassignment
			if (sidestep_button == joystickpressed)
				sidestep_button = 0xFF;
			else if (run_button == joystickpressed)
				run_button = 0xFF;
			else if (crouch_button == joystickpressed)
				crouch_button = 0xFF;
			else if (interact_button == joystickpressed)
				interact_button = 0xFF;
			else if (arm_button == joystickpressed)
				arm_button = 0xFF;
			else if (fire_button == joystickpressed)
				fire_button = 0xFF;
			else if (inventory_button == joystickpressed)
				inventory_button = 0xFF;
			else if (remora_button == joystickpressed)
				remora_button = 0xFF;
			else if (pause_button == joystickpressed)
				pause_button = 0xFF;

			switch (m_CONTROL_selected) {
			case UP_CROUCH:
				crouch_button = joystickpressed;
				break;
			case DOWN_INTERACT:
				interact_button = joystickpressed;
				break;
			case LEFT_ARM:
				arm_button = joystickpressed;
				break;
			case RIGHT_ATTACK:
				fire_button = joystickpressed;
				break;
			case RUN_INVENTORY:
				if (m_controlPage1)
					run_button = joystickpressed;
				else
					inventory_button = joystickpressed;
				break;
			case SIDESTEP_REMORA:
				if (m_controlPage1)
					sidestep_button = joystickpressed;
				else
					remora_button = joystickpressed;
				break;
			case PAUSE:
				pause_button = joystickpressed;
				break;

			default:
				return;
			}

			// Done my shit thanks
			m_awaitingKeyPress = FALSE8;
			m_editing = FALSE8;
			m_configLimiter = TRUE8;

			g_system->delayMillis(200);
			return;
		}
	}

	if (keypressed != 0) {
		// Joystick control is selected so don't assign keys to directional control
		if ((m_CONTROL_selected == UP_CROUCH || m_CONTROL_selected == DOWN_INTERACT || m_CONTROL_selected == LEFT_ARM || m_CONTROL_selected == RIGHT_ATTACK) &&
		    m_controlPage1) {
			if (currentJoystick != NO_JOYSTICK)
				return;
		}

		// Ban the use of keys with no names
		if (GetKeyName(keypressed) == NULL)
			return;

		// Check the assignment hasn't already been used
		if (up_key == keypressed)
			up_key = 0;
		else if (down_key == keypressed)
			down_key = 0;
		else if (left_key == keypressed)
			left_key = 0;
		else if (right_key == keypressed)
			right_key = 0;
		else if (sidestep_key == keypressed)
			sidestep_key = 0;
		else if (run_key == keypressed)
			run_key = 0;
		else if (crouch_key == keypressed)
			crouch_key = 0;
		else if (interact_key == keypressed)
			interact_key = 0;
		else if (arm_key == keypressed)
			arm_key = 0;
		else if (fire_key == keypressed)
			fire_key = 0;
		else if (inventory_key == keypressed)
			inventory_key = 0;
		else if (remora_key == keypressed)
			remora_key = 0;
		else if (pause_key == keypressed)
			return;

		switch (m_CONTROL_selected) {
		case UP_CROUCH:
			if (m_controlPage1)
				up_key = keypressed;
			else
				crouch_key = keypressed;
			break;
		case DOWN_INTERACT:
			if (m_controlPage1)
				down_key = keypressed;
			else
				interact_key = keypressed;
			break;
		case LEFT_ARM:
			if (m_controlPage1)
				left_key = keypressed;
			else
				arm_key = keypressed;
			break;
		case RIGHT_ATTACK:
			if (m_controlPage1)
				right_key = keypressed;
			else
				fire_key = keypressed;
			break;
		case RUN_INVENTORY:
			if (m_controlPage1)
				run_key = keypressed;
			else
				inventory_key = keypressed;
			break;
		case SIDESTEP_REMORA:
			if (m_controlPage1)
				sidestep_key = keypressed;
			else
				remora_key = keypressed;
			break;
		case PAUSE:
			pause_key = keypressed;
			break;

		default:
			return;
		}

		// Done my shit thanks
		m_awaitingKeyPress = FALSE8;
		m_editing = FALSE8;
		m_configLimiter = TRUE8;

		g_system->delayMillis(200);
	}

	m_assignFlash++;
	if (m_assignFlash == 10)
		m_assignFlash = 0;
}

void OptionsManager::InitialiseControlsScreen() {
	const char *msg = NULL;

	InitialiseAnimSequences();
	m_controlAnimCursor = 0;

	// Initialise an actor model to draw
	InitActorView("cord", "flack_jacket", "unarmed", "stand", CTRL_ACTOR_X, CTRL_ACTOR_Y, CTRL_ACTOR_Z);

	// Ensure these flags are set for the polgon renderer
	_drawActor = 1;
	_drawPolys = 0;
	_drawTxture = 0;
	_drawBbox = 0;
	_drawWfrm = 1;
	_drawLit = 1;
	wfrmRed = 3;
	wfrmGreen = 204;
	wfrmBlue = 0;

	// Need to calculate printing margin
	m_margin = 0;
	uint margin = 0;
	// This ensures correct spacing for any translations (assuming this is the int32est heading thang)
	msg = GetTextFromReference(HashString("opt_controlmethod"));
	margin = CalculateStringWidth(msg);
	if (m_margin < margin)
		m_margin = margin;
	// This is the offending line in german version so add hack here
	msg = GetTextFromReference(HashString("opt_turnright"));
	margin = CalculateStringWidth(msg);
	if (m_margin < margin)
		m_margin = margin;

	// Cool now add some spacing
	m_margin += 15;

	m_controlPage1 = TRUE8;
}

void OptionsManager::DrawControllerConfiguration() {
	const char *msg = NULL;
	const char *info = NULL;
	uint32 halfScreen = SCREEN_WIDTH / 2;
	uint32 temp;
	pxString sentence;
	LRECT repairRect;

	// Only perform dirty rectangle refresh in-game (ie no movie)
	if (m_useDirtyRects) {
		// Calculate dirty rectangle
		repairRect.left = 10;
		repairRect.right = halfScreen + 130;
		repairRect.top = 130;
		repairRect.bottom = 340;

		// We need to refresh a rectangle only
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);

		// Should use the actor's bounding box really but as that is more aften than not incorrect...
		repairRect.left = halfScreen + 131;
		repairRect.right = SCREEN_WIDTH;
		repairRect.top = 79;
		repairRect.bottom = 420;

		// We need to refresh a rectangle only
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);
	}

	// Draw the actor first up (following sequence logic)
	if (ActorViewDraw() == ANIMATION_END) {
		// Sequenceas only valid for controls selected
		if (m_CONTROL_selected >= UP_CROUCH && m_CONTROL_selected <= PAUSE) {
			// Need to change to the next animation for this control
			int indexToNextAnim = (m_CONTROL_selected - 2) * 2;
			if (m_controlPage1 == FALSE8)
				indexToNextAnim++;
			indexToNextAnim *= NUMBER_OF_ANIMS_PER_CONTROL;

			// See if we have a valid animation left in this sequence or return to first animation
			if (cc_anim_sequences[indexToNextAnim + m_controlAnimCursor + 1].used && m_controlAnimCursor < 4)
				m_controlAnimCursor++;
			else
				m_controlAnimCursor = 0;

			// Do the dynamic change
			ChangeAnimPlaying(cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].pose, cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].anim,
			                  cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].forwards, cc_anim_sequences[indexToNextAnim + m_controlAnimCursor].repeats,
			                  CTRL_ACTOR_X, CTRL_ACTOR_Y, CTRL_ACTOR_Z);
		}
	}

	uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	msg = GetTextFromReference(HashString("opt_controls"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	msg = GetTextFromReference(HashString("opt_device"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, m_margin - temp, 130, (m_CONTROL_selected == DEVICE) ? SELECTEDFONT : NORMALFONT, FALSE8);
	if (currentJoystick == NO_JOYSTICK)
		msg = GetTextFromReference(HashString("opt_keyboard"));
	else {
		// Change this to use DirectX device names later (for joysticks only)
		msg = GetJoystickName();

		// This returns NULL if there's no joystick so we can force keyboard
		if (msg == NULL) {
			msg = GetTextFromReference(HashString("opt_keyboard"));
		}
	}
	DisplayText(ad, pitch, msg, m_margin + 5, 130, NORMALFONT, FALSE8);

	msg = GetTextFromReference(HashString("opt_controlmethod"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, m_margin - temp, 155, (m_CONTROL_selected == METHOD) ? SELECTEDFONT : NORMALFONT, FALSE8);
	if (g_icb_session->player.Get_control_mode() == SCREEN_RELATIVE)
		msg = GetTextFromReference(HashString("opt_screenrelative"));
	else
		msg = GetTextFromReference(HashString("opt_actorrelative"));
	DisplayText(ad, pitch, msg, m_margin + 5, 155, NORMALFONT, FALSE8);

	// Now do the info thing
	bool8 doinfo = TRUE8;

	// Whose profile are we drawing
	switch (m_CONTROL_selected) {
	case UP_CROUCH:
		if (m_controlPage1)
			info = "up";
		else
			info = "crouch";
		break;
	case DOWN_INTERACT:
		if (m_controlPage1)
			info = "down";
		else
			info = "interact";
		break;
	case LEFT_ARM:
		if (m_controlPage1)
			info = "left";
		else
			info = "arm";
		break;
	case RIGHT_ATTACK:
		if (m_controlPage1)
			info = "right";
		else
			info = "attack";
		break;
	case RUN_INVENTORY:
		if (m_controlPage1)
			info = "run";
		else
			info = "inventory";
		break;
	case SIDESTEP_REMORA:
		if (m_controlPage1)
			info = "sidestep";
		else
			info = "remora";
		break;
	case PAUSE:
		info = "pause";
		break;

	default:
		doinfo = FALSE8;
		break;
	}

	if (doinfo) {
		int32 LEFT_HAND_MARGIN = m_margin + 130;

		// Hack the italian screen more cos they use int32 key names
		if (g_theClusterManager->GetLanguage() == T_ITALIAN)
			LEFT_HAND_MARGIN += 25;
		else if (g_theClusterManager->GetLanguage() == T_SPANISH)
			LEFT_HAND_MARGIN += 60;
		else if (g_theClusterManager->GetLanguage() == T_RUSSIAN)
			LEFT_HAND_MARGIN += 45;
		else if (g_theClusterManager->GetLanguage() == T_POLISH)
			LEFT_HAND_MARGIN += 50;

		msg = GetTextFromReference(HashString("opt_info"));
		DisplayText(ad, pitch, msg, LEFT_HAND_MARGIN, 192, PALEFONT, FALSE8);

		// Now we need to parse the info string word by word writing to the screen until we
		// need a new line

		// Get the whole string
		sentence.Format("opt_%sinfo", info);
		msg = GetTextFromReference(HashString(sentence));
		if (msg == NULL)
			msg = "PLEASE UPDATE GLOBAL TEXT DATA";

		// Get some storage from the stack
		uint8 theData[MAX_BYTESIZE_OF_CONTROL_INFO];
		// Zero out our memory
		memset(theData, 0, MAX_BYTESIZE_OF_CONTROL_INFO);
		// Make a personal copy
		memcpy(theData, msg, strlen(msg) + 1);
		// Get a pointer to our memory
		char *ptr = (char *)theData;

		// Split the text into words (overwrite spaces with terminators)
		int i = 0;
		uint32 numberOfWords = 1;
		while (ptr[i]) {
			// Found a space?
			if (ptr[i] == ' ') {
				// Watch for multiple spaces!
				do {
					ptr[i] = 0;
					i++;

				} while (ptr[i] == ' ');

				numberOfWords++;
			} else
				i++;
		}

		// Positional cursors initialised
		uint32 xp = LEFT_HAND_MARGIN;
		uint32 yp = 212;

		uint32 RIGHT_HAND_MARGIN = xp + 170;
		uint32 BOTTOM_EDGE_LIMIT = SCREEN_DEPTH - 50;
		uint32 SPACE_PIXELWIDTH = 5;
		uint32 cur = 0;

		// Now we're in a position to display it word by word
		for (uint32 w = 0; w < numberOfWords; w++) {
			// Safety check
			if (cur >= sizeof(theData))
				break;

			DisplayText(ad, pitch, (const char *)(ptr + cur), xp, yp, PALEFONT, FALSE8);

			// Move cursor(s)
			xp += CalculateStringWidth((const char *)(ptr + cur)) + SPACE_PIXELWIDTH;

			// Last word already printed so break
			if (w + 1 == numberOfWords) {
				break;
			}

			// Point to next word
			cur += strlen((const char *)(ptr + cur));
			while (!ptr[cur])
				cur++;

			// Calculate pixel length of NEXT word
			uint32 nextw = CalculateStringWidth((const char *)(ptr + cur));

			// Check for line ends
			if (xp + nextw > RIGHT_HAND_MARGIN) {
				yp += 20;
				xp = LEFT_HAND_MARGIN;

				// Check we've some screen left
				if (yp > BOTTOM_EDGE_LIMIT) {
					break;
				}
			}
		}
	}

	msg = GetTextFromReference(HashString("opt_defaults"));
	DisplayText(ad, pitch, msg, m_margin + 5, 356, (m_CONTROL_selected == DEFAULTS) ? SELECTEDFONT : NORMALFONT, FALSE8);

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, m_margin + 5, 385, (bool8)(m_CONTROL_selected == DONE) ? SELECTEDFONT : NORMALFONT, FALSE8);

	surface_manager->Unlock_surface(working_buffer_id);

	DrawControls(working_buffer_id);
}

void OptionsManager::DrawControls(uint32 surface_id) {
	const char *msg = NULL;
	const char *msg2 = NULL;
	static int flash = 0;
	uint32 temp;
	pxString str;
	bool8 screenRelative = (bool8)(g_icb_session->player.Get_control_mode() == SCREEN_RELATIVE);

	// Define initial offsets here that all controls relate too
	uint32 h = 192;  // Screen height of first control
	uint32 spc = 21; // Vertical spacing

	// Adjust spacing for first page (as it has one less choice)
	if (m_controlPage1)
		spc = 24;

	// Are we trying to assign a key to a game function
	if (m_awaitingKeyPress)
		GetKeyAssignment();

	uint8 *ad = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	flash++;
	// Draw flashing page indicators
	if (flash < 7)
		DisplayText(ad, pitch, "<>", 10, h - 15, NORMALFONT, FALSE8);

	// Cycling
	if (flash == 14)
		flash = 0;

	if (m_controlPage1) {
		msg = screenRelative ? GetTextFromReference(HashString("opt_up")) : GetTextFromReference(HashString("opt_forwards"));
		temp = CalculateStringWidth(msg);

		if (currentJoystick != NO_JOYSTICK)
			msg2 = GetAxisName(up_joy);
		else
			msg2 = GetKeyName(up_key);
		if (msg2 == NULL)
			msg2 = "???";
	} else {
		msg = GetTextFromReference(HashString("opt_crouch"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (crouch_button != 0xFF))
			msg2 = GetButtonName(crouch_button);
		else
			msg2 = GetKeyName(crouch_key);
		if (msg2 == NULL)
			msg2 = "???";
	}

	DisplayText(ad, pitch, msg, m_margin - temp, h, (bool8)(m_CONTROL_selected == UP_CROUCH) ? SELECTEDFONT : PALEFONT, FALSE8);
	DisplayText(ad, pitch, msg2, m_margin + 5, h, (bool8)(m_CONTROL_selected == UP_CROUCH && m_assignFlash < 5) ? SELECTEDFONT : NORMALFONT, FALSE8);

	h += spc;

	if (m_controlPage1) {
		msg = screenRelative ? GetTextFromReference(HashString("opt_down")) : GetTextFromReference(HashString("opt_backwards"));
		temp = CalculateStringWidth(msg);

		if (currentJoystick != NO_JOYSTICK)
			msg2 = GetAxisName(down_joy);
		else
			msg2 = GetKeyName(down_key);
		if (msg2 == NULL)
			msg2 = "???";
	} else {
		msg = GetTextFromReference(HashString("opt_interact"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (interact_button != 0xFF))
			msg2 = GetButtonName(interact_button);
		else
			msg2 = GetKeyName(interact_key);
		if (msg2 == NULL)
			msg2 = "???";
	}

	DisplayText(ad, pitch, msg, m_margin - temp, h, (bool8)(m_CONTROL_selected == DOWN_INTERACT) ? SELECTEDFONT : PALEFONT, FALSE8);
	DisplayText(ad, pitch, msg2, m_margin + 5, h, (bool8)(m_CONTROL_selected == DOWN_INTERACT && m_assignFlash < 5) ? SELECTEDFONT : NORMALFONT, FALSE8);

	h += spc;

	if (m_controlPage1) {
		msg = screenRelative ? GetTextFromReference(HashString("opt_left")) : GetTextFromReference(HashString("opt_turnleft"));
		temp = CalculateStringWidth(msg);

		if (currentJoystick != NO_JOYSTICK)
			msg2 = GetAxisName(left_joy);
		else
			msg2 = GetKeyName(left_key);
		if (msg2 == NULL)
			msg2 = "???";
	} else {
		msg = GetTextFromReference(HashString("opt_arm"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (arm_button != 0xFF))
			msg2 = GetButtonName(arm_button);
		else
			msg2 = GetKeyName(arm_key);
		if (msg2 == NULL)
			msg2 = "???";
	}

	DisplayText(ad, pitch, msg, m_margin - temp, h, (bool8)(m_CONTROL_selected == LEFT_ARM) ? SELECTEDFONT : PALEFONT, FALSE8);
	DisplayText(ad, pitch, msg2, m_margin + 5, h, (bool8)(m_CONTROL_selected == LEFT_ARM && m_assignFlash < 5) ? SELECTEDFONT : NORMALFONT, FALSE8);

	h += spc;

	if (m_controlPage1) {
		msg = screenRelative ? GetTextFromReference(HashString("opt_right")) : GetTextFromReference(HashString("opt_turnright"));
		temp = CalculateStringWidth(msg);

		if (currentJoystick != NO_JOYSTICK)
			msg2 = GetAxisName(right_joy);
		else
			msg2 = GetKeyName(right_key);
		if (msg2 == NULL)
			msg2 = "???";
	} else {
		msg = GetTextFromReference(HashString("opt_attack"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (fire_button != 0xFF))
			msg2 = GetButtonName(fire_button);
		else
			msg2 = GetKeyName(fire_key);
		if (msg2 == NULL)
			msg2 = "???";
	}

	DisplayText(ad, pitch, msg, m_margin - temp, h, (bool8)(m_CONTROL_selected == RIGHT_ATTACK) ? SELECTEDFONT : PALEFONT, FALSE8);
	DisplayText(ad, pitch, msg2, m_margin + 5, h, (bool8)(m_CONTROL_selected == RIGHT_ATTACK && m_assignFlash < 5) ? SELECTEDFONT : NORMALFONT, FALSE8);

	h += spc;

	if (m_controlPage1) {
		msg = GetTextFromReference(HashString("opt_run"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (run_button != 0xFF))
			msg2 = GetButtonName(run_button);
		else
			msg2 = GetKeyName(run_key);
		if (msg2 == NULL)
			msg2 = "???";
	} else {
		msg = GetTextFromReference(HashString("opt_inventory"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (inventory_button != 0xFF))
			msg2 = GetButtonName(inventory_button);
		else
			msg2 = GetKeyName(inventory_key);
		if (msg2 == NULL)
			msg2 = "???";
	}

	DisplayText(ad, pitch, msg, m_margin - temp, h, (bool8)(m_CONTROL_selected == RUN_INVENTORY) ? SELECTEDFONT : PALEFONT, FALSE8);
	DisplayText(ad, pitch, msg2, m_margin + 5, h, (bool8)(m_CONTROL_selected == RUN_INVENTORY && m_assignFlash < 5) ? SELECTEDFONT : NORMALFONT, FALSE8);

	h += spc;

	if (m_controlPage1) {
		msg = GetTextFromReference(HashString("opt_sidestep"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (sidestep_button != 0xFF))
			msg2 = GetButtonName(sidestep_button);
		else
			msg2 = GetKeyName(sidestep_key);
		if (msg2 == NULL)
			msg2 = "???";
	} else {
		msg = GetTextFromReference(HashString("opt_remora"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (remora_button != 0xFF))
			msg2 = GetButtonName(remora_button);
		else
			msg2 = GetKeyName(remora_key);
		if (msg2 == NULL)
			msg2 = "???";
	}

	DisplayText(ad, pitch, msg, m_margin - temp, h, (bool8)(m_CONTROL_selected == SIDESTEP_REMORA) ? SELECTEDFONT : PALEFONT, FALSE8);
	DisplayText(ad, pitch, msg2, m_margin + 5, h, (bool8)(m_CONTROL_selected == SIDESTEP_REMORA && m_assignFlash < 5) ? SELECTEDFONT : NORMALFONT, FALSE8);

	h += spc;

	if (m_controlPage1 == FALSE8) {
		msg = GetTextFromReference(HashString("opt_pause"));
		temp = CalculateStringWidth(msg);

		if ((currentJoystick != NO_JOYSTICK) && (pause_button != 0xFF))
			msg2 = GetButtonName(pause_button);
		else
			msg2 = GetKeyName(pause_key);
		if (msg2 == NULL)
			msg2 = "???";

		DisplayText(ad, pitch, msg, m_margin - temp, h, (bool8)(m_CONTROL_selected == PAUSE) ? SELECTEDFONT : PALEFONT, FALSE8);
		DisplayText(ad, pitch, msg2, m_margin + 5, h, (bool8)(m_CONTROL_selected == PAUSE && m_assignFlash < 5) ? SELECTEDFONT : NORMALFONT, FALSE8);
	}

	surface_manager->Unlock_surface(surface_id);
}

void OptionsManager::DrawGameOptions() {
	const char *msg = NULL;

	uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	msg = GetTextFromReference(HashString("opt_options"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	msg = GetTextFromReference(HashString("opt_videosettings"));
	DisplayText(ad, pitch, msg, 0, 130, (m_OPTION_selected == VIDEO_SETTINGS) ? SELECTEDFONT : NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_audiosettings"));
	DisplayText(ad, pitch, msg, 0, 150, (m_OPTION_selected == AUDIO_SETTINGS) ? SELECTEDFONT : NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_controls"));
	DisplayText(ad, pitch, msg, 0, 170, (m_OPTION_selected == CONTROLS) ? SELECTEDFONT : NORMALFONT, TRUE8);

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, 205, (m_OPTION_selected == BACK) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(working_buffer_id);
}

void OptionsManager::DrawAudioSettings() {
	const char *msg = NULL;
	uint32 halfScreen = SCREEN_WIDTH / 2;
	uint32 temp;
	LRECT repairRect;

	// Need to blit a dirty rect
	if (m_useDirtyRects) {
		if (m_AUDIO_selected != DO_ONE) {
			repairRect.left = halfScreen + 2;
			repairRect.right = halfScreen + 128;
			repairRect.top = 142 + (m_AUDIO_selected * 35);
			repairRect.bottom = repairRect.top + 16;

			// We need to refresh a rectangle only
			surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);
		}
	}

	// Filled rectangles must be outside of a lock
	Fill_rect(halfScreen + 3, 143, halfScreen + 127, 157, 0x172B4E);
	if (GetMusicVolume() > 2)
		Fill_rect(halfScreen + 2, 142, halfScreen + GetMusicVolume(), 158, 0xFEFEFE);
	Fill_rect(halfScreen + 3, 178, halfScreen + 127, 192, 0x172B4E);
	if (GetSpeechVolume() > 2)
		Fill_rect(halfScreen + 2, 177, halfScreen + GetSpeechVolume(), 193, 0xFEFEFE);
	Fill_rect(halfScreen + 3, 213, halfScreen + 127, 227, 0x172B4E);
	if (GetSfxVolume() > 2)
		Fill_rect(halfScreen + 2, 212, halfScreen + GetSfxVolume(), 228, 0xFEFEFE);

	uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	msg = GetTextFromReference(HashString("opt_audiosettings"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	DrawRectangle((bool8)(m_AUDIO_selected == MUSIC_VOLUME), halfScreen, 140, 129, 19, ad, pitch);

	DrawRectangle((bool8)(m_AUDIO_selected == SPEECH_VOLUME), halfScreen, 175, 129, 19, ad, pitch);

	DrawRectangle((bool8)(m_AUDIO_selected == SFX_VOLUME), halfScreen, 210, 129, 19, ad, pitch);

	SetDrawColour(BASE);

	msg = GetTextFromReference(HashString("opt_musicvolume"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, halfScreen - temp - 10, 140, (m_AUDIO_selected == MUSIC_VOLUME) ? SELECTEDFONT : NORMALFONT, FALSE8);
	msg = GetTextFromReference(HashString("opt_speechvolume"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, halfScreen - temp - 10, 175, (m_AUDIO_selected == SPEECH_VOLUME) ? SELECTEDFONT : NORMALFONT, FALSE8);
	msg = GetTextFromReference(HashString("opt_sfxvolume"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, halfScreen - temp - 10, 210, (m_AUDIO_selected == SFX_VOLUME) ? SELECTEDFONT : NORMALFONT, FALSE8);

	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, 255, (m_AUDIO_selected == DO_ONE) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(working_buffer_id);
}

void OptionsManager::DrawVideoSettings() {
	const char *msg = NULL;
	uint32 halfScreen = SCREEN_WIDTH / 2;
	uint32 temp;
	LRECT repairRect;

	// Need to blit a dirty rect
	if (m_useDirtyRects) {
		if (m_VIDEO_selected != LEAVE) {
			repairRect.left = halfScreen;
			repairRect.right = SCREEN_WIDTH;
			repairRect.top = 130 + (m_VIDEO_selected * 20);
			repairRect.bottom = repairRect.top + 20;

			// We need to refresh a rectangle only
			surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);
		}
	}

	uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	msg = GetTextFromReference(HashString("opt_videosettings"));
	DisplayText(ad, pitch, msg, 0, 80, NORMALFONT, TRUE8, TRUE8);

	uint32 hite = 130;

	// Subtitles
	msg = GetTextFromReference(HashString("opt_subtitles"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, halfScreen - temp - 10, hite, (m_VIDEO_selected == SUBTITLES) ? SELECTEDFONT : NORMALFONT, FALSE8);
	if (px.on_screen_text)
		msg = GetTextFromReference(HashString("opt_on"));
	else
		msg = GetTextFromReference(HashString("opt_off"));
	DisplayText(ad, pitch, msg, halfScreen, hite, NORMALFONT, FALSE8);
	hite += 20;

	// Semi-transparency
	msg = GetTextFromReference(HashString("opt_semitransparency"));
	temp = CalculateStringWidth(msg);
	DisplayText(ad, pitch, msg, halfScreen - temp - 10, hite, (m_VIDEO_selected == SEMITRANS) ? SELECTEDFONT : NORMALFONT, FALSE8);
	if (px.semitransparencies)
		msg = GetTextFromReference(HashString("opt_on"));
	else
		msg = GetTextFromReference(HashString("opt_off"));
	DisplayText(ad, pitch, msg, halfScreen, hite, NORMALFONT, FALSE8);

	if (g_videoOptionsCheat == TRUE8) {
		hite += 20;

		// Shadows
		msg = GetTextFromReference(HashString("opt_shadows"));
		temp = CalculateStringWidth(msg);
		DisplayText(ad, pitch, msg, halfScreen - temp - 10, hite, (m_VIDEO_selected == SHADOWS) ? SELECTEDFONT : NORMALFONT, FALSE8);
		if (px.actorShadows == -1)
			msg = GetTextFromReference(HashString("opt_shadows_simple"));
		else if (px.actorShadows == 1)
			msg = GetTextFromReference(HashString("opt_shadows_1"));
		else if (px.actorShadows == 2)
			msg = GetTextFromReference(HashString("opt_shadows_2"));
		else if (px.actorShadows == 3)
			msg = GetTextFromReference(HashString("opt_shadows_3"));
		else
			msg = GetTextFromReference(HashString("opt_shadows_off"));
		DisplayText(ad, pitch, msg, halfScreen, hite, NORMALFONT, FALSE8);
		hite += 20;

		// Frame limiter
		msg = GetTextFromReference(HashString("opt_framelimiter"));
		temp = CalculateStringWidth(msg);
		DisplayText(ad, pitch, msg, halfScreen - temp - 10, hite, (m_VIDEO_selected == FRAMELIMITER) ? SELECTEDFONT : NORMALFONT, FALSE8);
		char msg2[6];
		sprintf(msg2, "%d%%", stub.cycle_speed);
		DisplayText(ad, pitch, msg2, halfScreen, hite, NORMALFONT, FALSE8);
	}

	hite += 45;
	// Back
	msg = GetTextFromReference(HashString("opt_back"));
	DisplayText(ad, pitch, msg, 0, hite, (m_VIDEO_selected == LEAVE) ? SELECTEDFONT : NORMALFONT, TRUE8);

	surface_manager->Unlock_surface(working_buffer_id);
}

void OptionsManager::AnimateSlotsPaging() {
	int boxWidth = m_slotBoundingRect.right - m_slotBoundingRect.left;
	int inc = 50;
	uint32 t = 0;
	LRECT repairRect;

	bool8 saveRestoreScreen = (bool8)(m_activeMenu != MAIN_MOVIES);

	// Only perform dirty rectangle refresh in-game (ie no movie)
	if (m_useDirtyRects) {
		// Time lock code
		t = GetMicroTimer();
		// Calculate dirty rectangle
		repairRect.left = 0;
		repairRect.right = SCREEN_WIDTH;
		repairRect.top = m_slotBoundingRect.top - 1;
		repairRect.bottom = m_slotBoundingRect.bottom;

		// We need to refresh a rectangle only
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);
	}

	// Clean the surfaces
	surface_manager->Fill_surface(m_mySlotSurface1ID, m_colourKey);

	if (m_pageleft) {
		// We want to decrement the slot offset

		// Should we slow the animation down
		if (m_pageOn_dest.right > (m_slotBoundingRect.right - 10)) {
			m_pageOn_dest.right += 1;
		} else
			m_pageOn_dest.right += inc;

		// Limit checking
		if (m_pageOn_dest.right > m_slotBoundingRect.right)
			m_pageOn_dest.right = m_slotBoundingRect.right;

		// Is the rect still partially hidden outside the screen
		if (m_pageOn_dest.right > boxWidth) {
			m_pageOn_dest.left = m_pageOn_dest.right - boxWidth;
			m_pageOn_from.left = m_slotBoundingRect.left;
		} else {
			m_pageOn_dest.left = 0;

			// Now set the source blit rectangle to match
			m_pageOn_from.left = m_slotBoundingRect.left + (boxWidth - m_pageOn_dest.right);
		}

		if (m_pageOn_from.left < m_pageOn_from.right) {
			// Which slots are we animating
			if (saveRestoreScreen) {
				// Draw the new slots to this blank surface
				DrawGameSlots(m_slotOffset - NUMBER_OF_VISIBLE_GAME_SLOTS, m_mySlotSurface1ID);
			} else {
				DrawMovieSlots(m_movieOffset - M_NUMBER_OF_VISIBLE_MOVIE_SLOTS, m_mySlotSurface1ID);
			}

			// Blit this surface the the screen with animating offsets and transparency
			surface_manager->Blit_surface_to_surface(m_mySlotSurface1ID, working_buffer_id, &m_pageOn_from, &m_pageOn_dest, DDBLT_KEYSRC);
		}

		// Now for the page that's fucking off the screen to the right
		surface_manager->Fill_surface(m_mySlotSurface1ID, m_colourKey);

		m_pageOff_dest.left += m_slotsFuckOffBy * inc;
		m_pageOff_dest.right = m_pageOff_dest.left + boxWidth;

		if (m_pageOff_dest.right > SCREEN_WIDTH - 1) {
			m_pageOff_dest.right = SCREEN_WIDTH - 1;
			m_pageOff_from.right = m_slotBoundingRect.left + ((SCREEN_WIDTH - 1) - m_pageOff_dest.left);
		}

		if (m_pageOff_from.left < m_pageOff_from.right) {
			// Which slots are we animating
			if (saveRestoreScreen) {
				// Draw the current slots to this blank surface
				DrawGameSlots(m_slotOffset, m_mySlotSurface1ID);
			} else {
				DrawMovieSlots(m_movieOffset, m_mySlotSurface1ID);
			}

			// Blit this surface the the screen with animating offsets and transparency
			surface_manager->Blit_surface_to_surface(m_mySlotSurface1ID, working_buffer_id, &m_pageOff_from, &m_pageOff_dest, DDBLT_KEYSRC);
		}

		// Are we finished
		if (m_pageOn_dest.right == m_slotBoundingRect.right) {
			// Stop animating and alter slot offset
			m_paging = FALSE8;
			m_slotsFuckOffBy = 0;

			if (saveRestoreScreen) {
				m_slotOffset -= NUMBER_OF_VISIBLE_GAME_SLOTS;

				LoadVisibleThumbnails();
			} else {
				m_movieOffset -= M_NUMBER_OF_VISIBLE_MOVIE_SLOTS;

				// Check we move the selection from the paging icon for the first page (which is now hidden)
				if (m_movieOffset == 0)
					m_M_MOVIE_selected = MOVIE05;

				LoadVisibleMovieShots();
			}

			return;
		}

		// Decrement counter
		m_slotsFuckOffBy++;
	} else {
		// We want to increment the slot offset

		// Should we slow the animation down
		if (m_pageOn_dest.left < (m_slotBoundingRect.left + 10)) {
			m_pageOn_dest.left -= 1;
		} else
			m_pageOn_dest.left -= inc;

		// Limit checking
		if (m_pageOn_dest.left < m_slotBoundingRect.left)
			m_pageOn_dest.left = m_slotBoundingRect.left;

		// Is the rect still partially hidden outside the screen
		if (m_pageOn_dest.left < (SCREEN_WIDTH - 1) - boxWidth) {
			m_pageOn_dest.right = m_pageOn_dest.left + boxWidth;
			m_pageOn_from.right = m_slotBoundingRect.right;
		} else {
			m_pageOn_dest.right = (SCREEN_WIDTH - 1);

			// Now set the source blit rectangle to match
			m_pageOn_from.right = m_slotBoundingRect.left + ((SCREEN_WIDTH - 1) - m_pageOn_dest.left);
		}

		if (m_pageOn_from.left < m_pageOn_from.right) {
			if (saveRestoreScreen) {
				// Draw the new slots to this blank surface
				DrawGameSlots(m_slotOffset + NUMBER_OF_VISIBLE_GAME_SLOTS, m_mySlotSurface1ID);
			} else {
				DrawMovieSlots(m_movieOffset + M_NUMBER_OF_VISIBLE_MOVIE_SLOTS, m_mySlotSurface1ID);
			}

			// Blit this surface the the screen with animating offsets and transparency
			surface_manager->Blit_surface_to_surface(m_mySlotSurface1ID, working_buffer_id, &m_pageOn_from, &m_pageOn_dest, DDBLT_KEYSRC);
		}

		// Now for the page that's fucking off the screen to the right
		surface_manager->Fill_surface(m_mySlotSurface1ID, m_colourKey);

		m_pageOff_dest.right -= m_slotsFuckOffBy * inc;
		m_pageOff_dest.left = m_pageOff_dest.right - boxWidth;

		if (m_pageOff_dest.left < 0) {
			m_pageOff_dest.left = 0;
			m_pageOff_from.left = m_slotBoundingRect.left + (boxWidth - m_pageOff_dest.right);
		}

		if (m_pageOff_from.left < m_pageOff_from.right) {
			if (saveRestoreScreen) {
				// Draw the current slots to this blank surface
				DrawGameSlots(m_slotOffset, m_mySlotSurface1ID);
			} else {
				DrawMovieSlots(m_movieOffset, m_mySlotSurface1ID);
			}

			// Blit this surface the the screen with animating offsets and transparency
			surface_manager->Blit_surface_to_surface(m_mySlotSurface1ID, working_buffer_id, &m_pageOff_from, &m_pageOff_dest, DDBLT_KEYSRC);
		}

		// Are we finished
		if (m_pageOn_dest.left == m_slotBoundingRect.left) {
			// Stop animating and alter slot offset
			m_paging = FALSE8;
			m_slotsFuckOffBy = 0;

			if (saveRestoreScreen) {
				m_slotOffset += NUMBER_OF_VISIBLE_GAME_SLOTS;

				LoadVisibleThumbnails();
			} else {
				m_movieOffset += M_NUMBER_OF_VISIBLE_MOVIE_SLOTS;

				// Is the selection valid on this new page
				if ((m_M_MOVIE_selected < NOTHANKS) && (m_M_MOVIE_selected + m_movieOffset >= TOTAL_NUMBER_OF_MOVIES))
					m_M_MOVIE_selected = (M_MOVIE_CHOICES)((TOTAL_NUMBER_OF_MOVIES % M_NUMBER_OF_VISIBLE_MOVIE_SLOTS) - 1);

				// Check we move the selection from the paging icon for the last page (which is now hidden)
				if (m_movieOffset >= TOTAL_NUMBER_OF_MOVIES - M_NUMBER_OF_VISIBLE_MOVIE_SLOTS)
					m_M_MOVIE_selected = MOVIE08;

				LoadVisibleMovieShots();
			}

			return;
		}

		// Decrement counter
		m_slotsFuckOffBy++;
	}

	if (m_useDirtyRects) {
		// Lock refresh time
		t = GetMicroTimer() - t;

		int32 r = REFRESH_LIMITER - t;
		if (t < REFRESH_LIMITER) {
			g_system->delayMillis(r / 1000);
		}
	}
}

void OptionsManager::SetTargetBox(uint32 x1, uint32 x2, uint32 y1, uint32 y2, uint32 nFrames) {
	m_targetBox.left = x1;
	m_targetBox.right = x2;
	m_targetBox.top = y1;
	m_targetBox.bottom = y2;

	m_interFrames = nFrames;
	m_warpDirection = TRUE8;
	m_autoAnimating2 = 0;

	int32 growBy = (m_targetBox.left - m_optionsBox.left);
	m_widthIncrements = (growBy / m_interFrames);
	if (m_widthIncrements == 0) {
		if (growBy < 0)
			m_widthIncrements = -1;
		else
			m_widthIncrements = 1;
	}

	growBy = (m_targetBox.top - m_optionsBox.top);
	m_topIncrements = (growBy / m_interFrames);
	if (m_topIncrements == 0) {
		if (growBy < 0)
			m_topIncrements = -1;
		else
			m_topIncrements = 1;
	}

	growBy = (m_targetBox.bottom - m_optionsBox.bottom);
	m_bottomIncrements = (growBy / m_interFrames);
	if (m_bottomIncrements == 0) {
		if (growBy < 0)
			m_bottomIncrements = -1;
		else
			m_bottomIncrements = 1;
	}
}

bool8 OptionsManager::AnimateBracketsToBox(bool8 forwards, uint32 surface_id) {
	uint32 pitch;
	uint8 *surface_address;
	uint32 t = 0;
	LRECT repairRect;

	// Check if we still need to animate the brackets
	if (forwards) {
		// Are we finished
		if (m_autoAnimating2 > m_interFrames) {
			m_interFrames = -1;
			return FALSE8;
		}
	} else {
		// Are we finished
		if (m_autoAnimating2 < 0) {
			m_interFrames = -1;
			return FALSE8;
		}
	}

	// Only perform dirty rectangle refresh in-game (ie no movie)
	if (m_useDirtyRects) {
		// Time lock code
		t = GetMicroTimer();
		// Calculate dirty rectangle (needs to be quite fat to cope with other languages)
		repairRect.left = m_targetBox.left - 50;
		repairRect.right = m_targetBox.right + 50;
		repairRect.top = m_targetBox.top - 1;
		repairRect.bottom = m_targetBox.bottom + m_optionsBox.bottom - m_optionsBox.top + 5;

		// We need to refresh a rectangle only
		surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);
	}

	// Lock the directdraw surface (working buffer)
	surface_address = surface_manager->Lock_surface(surface_id);
	pitch = surface_manager->Get_pitch(surface_id);

	// Vertical bracket lips are drawn from top to bottom
	Draw_vertical_line(m_box.left - 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
	Draw_vertical_line(m_box.right + 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
	Draw_vertical_line(m_box.left - 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
	Draw_vertical_line(m_box.right + 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
	Draw_horizontal_line(m_box.left, m_box.top - 1, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);
	Draw_horizontal_line(m_box.left, m_box.bottom + 1, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);

	// Unlock the working buffer
	surface_manager->Unlock_surface(surface_id);

	// Apply modifications with clipping
	if (forwards) {
		// Catch last frame
		if (m_autoAnimating2 == m_interFrames) {
			m_box.left = m_targetBox.left;
			m_box.right = m_targetBox.right;
			m_box.top = m_targetBox.top;
			m_box.bottom = m_targetBox.bottom;

			// Final refresh
			if (m_useDirtyRects)
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);
		} else {
			// Check we don't exceed our desired limits

			if (m_box.top + m_topIncrements < m_targetBox.top)
				m_box.top = m_targetBox.top;
			else
				m_box.top += m_topIncrements;

			if (m_box.bottom + m_bottomIncrements < m_targetBox.bottom)
				m_box.bottom = m_targetBox.bottom;
			else
				m_box.bottom += m_bottomIncrements;

			if (m_box.left + m_widthIncrements < m_targetBox.left)
				m_box.left = m_targetBox.left;
			else
				m_box.left += m_widthIncrements;

			if (m_box.right - m_widthIncrements > m_targetBox.right)
				m_box.right = m_targetBox.right;
			else
				m_box.right -= m_widthIncrements;
		}
	} else {
		// Catch last frame
		if (m_autoAnimating2 == 0) {
			m_box.left = m_optionsBox.left;
			m_box.right = m_optionsBox.right;
			m_box.top = m_optionsBox.top;
			m_box.bottom = m_optionsBox.bottom;

			// Final refresh
			if (m_useDirtyRects)
				surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);
		} else {
			// Check we don't exceed our desired limits

			if (m_box.top - m_topIncrements > m_optionsBox.top)
				m_box.top = m_optionsBox.top;
			else
				m_box.top -= m_topIncrements;

			if (m_box.bottom - m_bottomIncrements > m_optionsBox.bottom)
				m_box.bottom = m_optionsBox.bottom;
			else
				m_box.bottom -= m_bottomIncrements;

			if (m_box.left - m_widthIncrements > m_optionsBox.left)
				m_box.left = m_optionsBox.left;
			else
				m_box.left -= m_widthIncrements;

			if (m_box.right + m_widthIncrements < m_optionsBox.right)
				m_box.right = m_optionsBox.right;
			else
				m_box.right += m_widthIncrements;
		}
	}

	// Increment or decrement current frame counter dependent on direction
	if (forwards)
		m_autoAnimating2++;
	else
		m_autoAnimating2--;

	if (m_useDirtyRects) {
		// Lock refresh time
		t = GetMicroTimer() - t;

		int32 r = REFRESH_LIMITER - t;
		if (t < REFRESH_LIMITER) {
			g_system->delayMillis(r / 1000);
		}
	}

	return TRUE8;
}

bool8 OptionsManager::AnimateThoseBrackets(bool8 forwards) {
	uint32 pitch;
	uint8 *surface_address;

	// Check if we still need to animate the brackets opening or closing
	if (forwards) {
		// Are we finished opening
		if (m_autoAnimating > m_over_n_Frames)
			return FALSE8;
	} else {
		// Are we finished closing
		if (m_autoAnimating < 0)
			return FALSE8;
	}
	// Time lock code
	uint32 t = GetMicroTimer();

	LRECT repairRect;
	repairRect.left = m_optionsBox.left - 1;
	repairRect.right = m_optionsBox.right + 2;
	repairRect.top = m_optionsBox.top - 1;
	repairRect.bottom = m_optionsBox.bottom + 2;

	surface_manager->Blit_surface_to_surface(m_myScreenSurfaceID, working_buffer_id, &repairRect, &repairRect);

	int32 nWidthIterations = (m_over_n_Frames / 3);
	int32 nHeightIterations = m_over_n_Frames - nWidthIterations;

	// The animating line portion of the animation occurs over a proportion of the total frames
	if (m_autoAnimating < nWidthIterations) {
		// Lock the directdraw surface (working buffer)
		surface_address = surface_manager->Lock_surface(working_buffer_id);
		pitch = surface_manager->Get_pitch(working_buffer_id);

		// Draw the horizontal line
		Draw_horizontal_line(m_box.left, m_box.top, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.top - 1, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);

		// Unlock the working buffer
		surface_manager->Unlock_surface(working_buffer_id);

		// Calculate m_grower value dependent on frame
		if (m_autoAnimating == 0) {
			// To give slight pause when only a dot
			m_grower = 1;
		} else if (m_autoAnimating == nWidthIterations - 1) {
			// To ensure we reach the desired width on the last iteration
			m_grower = 15;
		} else if (m_autoAnimating > nWidthIterations - 3) {
			// To pause when we're near the end of the line
			m_grower = 0;
		} else {
			// Regular growth
			m_grower = ((SCREEN_WIDTH / 2) - m_optionsBox.left - 3) / (nWidthIterations - 4);
		}

		// Apply modifications with clipping
		if (forwards) {
			// Check we don't exceed our desired width limit
			if (m_box.left - m_grower < m_optionsBox.left)
				m_box.left = m_optionsBox.left;
			else
				m_box.left -= m_grower;

			if (m_box.right + m_grower > m_optionsBox.right)
				m_box.right = m_optionsBox.right;
			else
				m_box.right += m_grower;
		} else {
			// We are closing so shrink the width of our box by m_grower
			m_box.left += m_grower;
			m_box.right -= m_grower;

			// Catch any overlap and cap it with limits
			if (m_box.left > m_box.right) {
				m_box.left = SCREEN_WIDTH / 2;
				m_box.right = SCREEN_WIDTH / 2;
			}
		}
	} else {
		// Now grow or shrink the brackets

		// Lock the directdraw surface (working buffer)
		surface_address = surface_manager->Lock_surface(working_buffer_id);
		pitch = surface_manager->Get_pitch(working_buffer_id);

		// Vertical bracket lips are drawn from top to bottom
		Draw_vertical_line(m_box.left, m_box.top, m_lipLength, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.left - 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right + 1, m_box.top - 1, m_lipLength + 1, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.left, m_box.bottom - m_lipLength, m_lipLength, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.left - 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
		Draw_vertical_line(m_box.right + 1, m_box.bottom - m_lipLength, m_lipLength + 2, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.top, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.top - 1, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.bottom, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);
		Draw_horizontal_line(m_box.left, m_box.bottom + 1, m_box.right - m_box.left, &m_drawColour, surface_address, pitch);

		// Unlock the working buffer
		surface_manager->Unlock_surface(working_buffer_id);

		// Calculate m_grower value dependent on frame
		if (m_autoAnimating == nWidthIterations) {
			// To give slight pause
			m_grower = 1;
		} else if (m_autoAnimating == m_over_n_Frames - 1) {
			// To ensure we reach the desired width on the last iteration
			m_grower = 15;
		} else if (m_autoAnimating > m_over_n_Frames - 3) {
			// To pause when we're near the end of the line
			m_grower = 2;
		} else {
			// Regular growth
			m_grower = ((SCREEN_DEPTH / 2) - m_optionsBox.top - 3) / (nHeightIterations - 4);

			// Special spongey feel
			if (m_autoAnimating % 2 == 0)
				m_grower++;
		}

		// Apply modifications with clipping
		if (forwards) {
			// Check we don't exceed our desired height limit
			if (m_box.top - m_grower < m_optionsBox.top)
				m_box.top = m_optionsBox.top;
			else
				m_box.top -= m_grower;

			if (m_box.bottom + m_grower > m_optionsBox.bottom)
				m_box.bottom = m_optionsBox.bottom;
			else
				m_box.bottom += m_grower;
		} else {
			// We are closing so shrink the height of our box by m_grower
			m_box.top += m_grower;
			m_box.bottom -= m_grower;

			// Catch any overlap and cap it with limits
			if (m_box.top > m_box.bottom) {
				m_box.top = m_box.bottom = m_optionsBox.top + ((m_optionsBox.bottom - m_optionsBox.top) / 2);
			}
		}

		// Ensure the bracket lips are the correct size
		if (m_box.bottom - m_box.top < 10)
			m_lipLength = m_box.bottom - m_box.top;
		else
			m_lipLength = 10;
	}

	// Increment or decrement current frame counter dependent on direction
	if (forwards)
		m_autoAnimating++;
	else
		m_autoAnimating--;

	// Lock refresh time
	t = GetMicroTimer() - t;

	if (t < REFRESH_LIMITER) {
		int32 r = REFRESH_LIMITER - t;
		g_system->delayMillis(r / 1000);
	}

	return TRUE8;
}

void OptionsManager::DarkenScreen() {
	uint8 subtractive[8];
	uint8 fadeBy = 0x50;

	// Fade by table
	subtractive[4] = subtractive[0] = fadeBy;
	subtractive[5] = subtractive[1] = fadeBy;
	subtractive[6] = subtractive[2] = fadeBy;
	subtractive[7] = subtractive[3] = 0x00;

	// Lock the directdraw surface (working buffer)
	uint8 *pixels = (uint8 *)surface_manager->Lock_surface(m_myScreenSurfaceID);
	uint32 pitch = surface_manager->Get_pitch(m_myScreenSurfaceID);

	// Darken the screen
#if 1
	for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
		for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
			// 32-bit BGRA pixel
			uint8 *pixel = &pixels[xPos * 4];
			// Subtract from RGB components
			for (int i = 0; i < 3; i++) {
				pixel[i] = MAX(0, pixel[i] - subtractive[i]);
			}
		}
		// Next line
		pixels += pitch;
	}
#else
	_asm {
		lea  edi, subtractive   ; // Load the address of the blend colour block
		mov  esi, pixels        ; // Load the address of the pixels
		movq MM0, [edi]         ; // Put address of the blend colour block into MMX register
		sub  esi, 8
		mov  edx, SCREEN_DEPTH  ; // Number of line to darken
		mov  eax, pitch;
		mov  ebx, (SCREEN_WIDTH/2); // Pixel Counter (2 pixels at a time mind)

		om_subtractive_fade_row_loop:
		mov  ecx, ebx   ; // Pixel Counter (2 pixels at a time mind)

		om_subtractive_fade_loop:
		movq    MM1, [esi + ecx * 8]        ; // Load 2 pixels
		psubusb MM1, MM0                ; // Do the subtract
		movq    [esi + ecx * 8], MM1        ; // Store the result
		dec     ecx                     ; // Reduce counter
		jne     om_subtractive_fade_loop; // On to the next 2 pixels

		add esi, eax; // On to the next row
		dec edx;
		jne om_subtractive_fade_row_loop;
		EMMS    ; // Clear/Set MMX/FPU flag
	}
#endif

	// Unlock the working buffer
	surface_manager->Unlock_surface(m_myScreenSurfaceID);
}

void OptionsManager::BloodScreen() {
	uint8 subtractive[8];
	uint8 fadeBy = 0x32;

	// Fade by table
	subtractive[4] = subtractive[0] = 0xFF;   // No blue
	subtractive[5] = subtractive[1] = 0xFF;   // No green
	subtractive[6] = subtractive[2] = fadeBy; // Fade red component
	subtractive[7] = subtractive[3] = 0x00;   // Don't care about alpha

	// Lock the directdraw surface (working buffer)
	uint8 *pixels = (uint8 *)surface_manager->Lock_surface(m_myScreenSurfaceID);
	uint32 pitch = surface_manager->Get_pitch(m_myScreenSurfaceID);

	// Darken the screen
#if 1
	for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
		for (int xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
			// 32-bit BGRA pixel
			uint8 *pixel = &pixels[xPos * 4];
			// Subtract from RGB components
			for (int i = 0; i < 3; i++) {
				pixel[i] = MAX(0, pixel[i] - subtractive[i]);
			}
		}
		// Next line
		pixels += pitch;
	}
#else
	_asm {
		lea  edi, subtractive   ; // Load the address of the blend colour block
		mov  esi, pixels        ; // Load the address of the pixels
		movq MM0, [edi]         ; // Put address of the blend colour block into MMX register
		sub  esi, 8
		mov  edx, SCREEN_DEPTH  ; // Number of line to darken
		mov  eax, pitch;
		mov  ebx, (SCREEN_WIDTH/2); // Pixel Counter (2 pixels at a time mind)

		om_blood_fade_row_loop:
		mov  ecx, ebx   ; // Pixel Counter (2 pixels at a time mind)

		om_blood_fade_loop:
		movq    MM1, [esi + ecx * 8]        ; // Load 2 pixels
		psubusb MM1, MM0                ; // Do the subtract
		movq    [esi + ecx * 8], MM1        ; // Store the result
		dec     ecx                     ; // Reduce counter
		jne     om_blood_fade_loop; // On to the next 2 pixels

		add esi, eax; // On to the next row
		dec edx;
		jne om_blood_fade_row_loop;
		EMMS    ; // Clear/Set MMX/FPU flag
	}
#endif

	// Unlock the working buffer
	surface_manager->Unlock_surface(m_myScreenSurfaceID);
}

void OptionsManager::SetDrawColour(uint32 def) {
	switch (def) {
	case BASE:
		m_drawColour.red = g_drawColour.red;
		m_drawColour.blue = g_drawColour.blue;
		m_drawColour.green = g_drawColour.green;
		break;
	case BASE_DARK:
		m_drawColour.red = g_drawColourDark.red;
		m_drawColour.blue = g_drawColourDark.blue;
		m_drawColour.green = g_drawColourDark.green;
		break;
	case SELECTED:
		m_drawColour.red = g_drawSelected.red;
		m_drawColour.blue = g_drawSelected.blue;
		m_drawColour.green = g_drawSelected.green;
		break;
	case SELECTED_DARK:
		m_drawColour.red = g_drawSelectedDark.red;
		m_drawColour.blue = g_drawSelectedDark.blue;
		m_drawColour.green = g_drawSelectedDark.green;
		break;
	}
}

void OptionsManager::DrawRectangle(bool8 selected, uint32 x, uint32 y, uint32 width, uint32 height, uint8 *surface_address, uint32 pitch) {
	if (selected) {
		SetDrawColour(SELECTED);
	} else {
		SetDrawColour(BASE);
	}

	Draw_vertical_line(x - 1, y - 1, height + 2, &m_drawColour, surface_address, pitch);
	Draw_vertical_line(x + width, y - 1, height + 2, &m_drawColour, surface_address, pitch);
	Draw_horizontal_line(x, y - 1, width, &m_drawColour, surface_address, pitch);
	Draw_horizontal_line(x, y + height, width, &m_drawColour, surface_address, pitch);

	if (selected) {
		SetDrawColour(SELECTED_DARK);
	} else {
		SetDrawColour(BASE_DARK);
	}

	Draw_vertical_line(x, y, height, &m_drawColour, surface_address, pitch);
	Draw_vertical_line(x + width + 1, y - 1, height + 3, &m_drawColour, surface_address, pitch);
	Draw_horizontal_line(x, y, width, &m_drawColour, surface_address, pitch);
	Draw_horizontal_line(x - 1, y + height + 1, width + 2, &m_drawColour, surface_address, pitch);
}

const char *OptionsManager::GetTextFromReference(uint32 hashRef) {
	char *textLine = NULL;

	// Get the text via a label
	if (m_global_text)
		textLine = (char *)m_global_text->Try_fetch_item_by_hash(hashRef);

	if (!textLine) {
		// Try again with reloaded text file
		LoadGlobalTextFile();
		textLine = (char *)m_global_text->Try_fetch_item_by_hash(hashRef);
		if (!textLine)
			return "Missing text!";
	}

	// All options text is non-spoken so we can assume it begins with the & flag character
	textLine++;

	// To be a line number, there must be an open brace as the first string character.
	if (textLine[0] == TS_LINENO_OPEN) {
		int nLineLength = strlen((const char *)textLine);

		// Okay, we appear to have a legal line number.  Find the close brace for it.
		int nCloseBracePos = 1;
		while ((nCloseBracePos < nLineLength) && (textLine[nCloseBracePos] != TS_LINENO_CLOSE))
			++nCloseBracePos;

		// If we didn't find one then this is an error.
		if (nCloseBracePos == nLineLength)
			Fatal_error("Failed to find the end of the line number in [%s]", textLine);

		// Right we appear to have a present-and-correct line number.  To display it we don't have
		// to do anything special.  If the displaying of line numbers is turned off then we must skip
		// past the line number.
		if (!px.speechLineNumbers) {
			// Skip to first non-space after the line number.
			const char *pcTextLine = (const char *)(&textLine[nCloseBracePos + 1]);
			while ((*pcTextLine != '\0') && (*pcTextLine == ' '))
				++pcTextLine;

			// If we got to the end of the string then we have a line number with no text following it.
			if (*pcTextLine == '\0')
				Fatal_error("Found line number [%s] with no text", textLine);

			// Write the modified pointer back into the text block
			textLine = const_cast<char *>(pcTextLine);
		}
	}

	return ((const char *)textLine);
}

void OptionsManager::LoadBitmapFont() {
	sprintf(m_fontName, FONT_PATH, OPTIONS_FONT_NAME);
	uint32 hashedname = NULL_HASH;

	m_font_file = (_pxBitmap *)rs_font->Res_open(m_fontName, hashedname, font_cluster, font_cluster_hash);

	if (m_font_file->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", m_fontName, PC_BITMAP_SCHEMA, m_font_file->schema);

	m_fontPalette = (uint32 *)m_font_file->Fetch_palette_pointer();
}

void OptionsManager::LoadGlobalTextFile() {
	// Set this up for resman
	char globalClusterFile[MAXLEN_CLUSTER_URL];
	sprintf(globalClusterFile, GLOBAL_CLUSTER_PATH);
	uint32 globalClusterHash = NULL_HASH;

	char textFileName[100];
	uint32 buf_hash = NULL_HASH;

	// Has a language been specified
	sprintf(textFileName, GLOBAL_TEXT_FILE);

	// Special text loading code so the translators can test their stuff

	if (tt) {
		// Ok, translators mode has been activated
		// Only load the global text if it hasn't been loaded already
		if (m_global_text == NULL)
			m_global_text = LoadTranslatedFile("global", "global\\global\\");
	} else
		m_global_text = (_linked_data_file *)rs1->Res_open(textFileName, buf_hash, globalClusterFile, globalClusterHash);

	m_global_text = (_linked_data_file *)rs1->Res_open(textFileName, buf_hash, globalClusterFile, globalClusterHash);
}

bool8 OptionsManager::SetCharacterSprite(char c) {
	int index = (int)c - 32;
	if (index < 0)
		index += 256;

	// Catch ernoeous characters and make them apostrophes
	if ((uint)index >= m_font_file->Fetch_number_of_items())
		index = 7;

	m_currentSprite = m_font_file->Fetch_item_by_number(index);

	if (!m_currentSprite)
		return FALSE8;

	return TRUE8;
}

uint32 OptionsManager::CalculateStringWidth(const char *str) {
	if (!str)
		Fatal_error("Cannot calculate width of a NULL or empty string");

	uint noChars = strlen(str);
	int32 sentenceWidth = 0;

	// Loop through all characters to get sentence length
	for (uint i = 0; i < noChars; i++) {
		// Select current sprite
		SetCharacterSprite(str[i]);
		// Keep track of sentence width
		sentenceWidth += (m_currentSprite->width + 1);
	}

	return sentenceWidth;
}

void OptionsManager::DisplayText(uint8 *ad, uint32 pitch, const char *str, int32 x, int32 y, uint32 col, bool8 centredHorizontally, bool8 boxed) {
	static char errorText[] = "MISSING TEXT !";

	if (!str)
		str = errorText;

	// How many characters do we draw
	uint noChars = strlen(str);
	int32 sentenceWidth;
	int32 initialX;

	// Ensure font is loaded as this function can be used outside this module
	if (m_font_file == NULL) {
		LoadBitmapFont();
	}

	// Do we need to modify x pos before we start to draw
	if (centredHorizontally) {
		sentenceWidth = CalculateStringWidth(str);

		// Override x value with this new one thus centring horizontally
		initialX = (SCREEN_WIDTH / 2) - (sentenceWidth / 2);

		x = initialX;
	}

	for (uint i = 0; i < noChars; i++) {
		// Select current sprite
		SetCharacterSprite(str[i]);

		RenderCharacter(ad, pitch, x, y, col);

		x += (m_currentSprite->width + 1);
	}

	if (boxed) {
		LRECT r;
		GetCentredRectFotText(str, r, y);
		DrawRectangle(FALSE8, r.left, r.top, r.right - r.left, r.bottom - r.top, ad, pitch);
	}
}

void OptionsManager::GetCentredRectFotText(const char *str, LRECT &r, int32 height) {
	int32 sentenceWidth = CalculateStringWidth(str);

	r.left = (SCREEN_WIDTH / 2) - (sentenceWidth / 2) - 50;
	r.right = r.left + (sentenceWidth + 100);
	r.top = height - 3;
	r.bottom = height + m_fontHeight + 3;
}

// Macros to get colour values from 32-bit palette pointer
#define GET_R(X) (((X) >> 16) & 0xFF)
#define GET_G(X) (((X) >> 8) & 0xFF)
#define GET_B(X) ((X)&0xFF)

void OptionsManager::RenderCharacter(uint8 *ad, uint32 pitch, int32 nX, int32 nY, uint32 col) {
	uint8 *src;
	uint8 *write;
	uint32 x, y;
	uint32 width, height;

	src = m_currentSprite->data;
	height = m_currentSprite->height;
	width = m_currentSprite->width;

	// Modify the palette for our anti-aliased coloured font
	switch (col) {
	case SELECTEDFONT:
		m_fontPalette[3] = 0xCA0000;
		m_fontPalette[4] = 0x960000;
		m_fontPalette[5] = 0x640000;
		break;
	case PALEFONT:
		m_fontPalette[3] = 0x808080;
		m_fontPalette[4] = 0x6E6E6E;
		m_fontPalette[5] = 0x505050;
		break;

	// Main Font
	default:
		m_fontPalette[3] = 0xFEFEFE;
		m_fontPalette[4] = 0xC1C1C1;
		m_fontPalette[5] = 0x646464;
		break;

		// Drawing blue     46, 87, 156     0x2E579C
		//                  29, 56, 102     0x1D3866
		//                  22, 42, 76      0x162A4C

		// Darker blue      22, 42, 76      0x162A4C
		//                  14, 28, 51      0x0E1C33
		//                  7,  14, 25      0x070E19

		// Gray             128, 128, 128   0x808080
		//                  110, 110, 110   0x6E6E6E
		//                  80, 80, 80      0x505050

		// White            254, 254, 254   0xFEFEFE
		//                  193, 193, 193   0xC1C1C1
		//                  100, 100, 100   0x646464

		// Red              202, 0, 0       0xCA0000
		//                  150, 0, 0       0x960000
		//                  100, 0, 0       0x640000
	}

	// Move to first byte to begin drawing
	ad += (nY * pitch);
	ad += (nX * 4);

	for (y = 0; y < height; y++) {
		write = ad;

		for (x = 0; x < width; x++) {
			// Don't draw pixels with palette index 0 (transparency)
			if (*src) {
				*write++ = (uint8)GET_B(m_fontPalette[*src]); // b
				*write++ = (uint8)GET_G(m_fontPalette[*src]); // g
				*write++ = (uint8)GET_R(m_fontPalette[*src]); // r
				write++; // a
			} else {
				write += 4;
			}

			src++;
		}

		ad += pitch;
	}
}

void OptionsManager::UnlockMovies() {
	for (uint32 m = 0; m < TOTAL_NUMBER_OF_MOVIES; m++)
		g_movieLibrary[m].visible = TRUE8;
}

// On complete game
void DoSomeMagicStuff() {
	g_theOptionsManager->UnlockMovies();

	uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	g_theOptionsManager->DisplayText(ad, pitch, "Movies visible", 0, SCREEN_DEPTH - 30, SELECTEDFONT, TRUE8);

	surface_manager->Unlock_surface(working_buffer_id);

	surface_manager->Flip();

	g_system->delayMillis(1000);
}

// Magic input sequences
uint8 magic_unlockmovies[13] = {Common::KEYCODE_t, Common::KEYCODE_e, Common::KEYCODE_s, Common::KEYCODE_t, Common::KEYCODE_b, Common::KEYCODE_r, Common::KEYCODE_e,
                                Common::KEYCODE_a, Common::KEYCODE_k, Common::KEYCODE_d, Common::KEYCODE_o, Common::KEYCODE_w, Common::KEYCODE_n};

uint8 magic_fastmovies[6] = {Common::KEYCODE_s, Common::KEYCODE_p, Common::KEYCODE_e, Common::KEYCODE_e, Common::KEYCODE_d, Common::KEYCODE_y};

uint8 magic_slideshowextras[12] = {Common::KEYCODE_h, Common::KEYCODE_o, Common::KEYCODE_l, Common::KEYCODE_i, Common::KEYCODE_d, Common::KEYCODE_a,
                                   Common::KEYCODE_y, Common::KEYCODE_s, Common::KEYCODE_n, Common::KEYCODE_a, Common::KEYCODE_p, Common::KEYCODE_s};

uint8 magic_avcontrol[7] = {Common::KEYCODE_r, Common::KEYCODE_a, Common::KEYCODE_b, Common::KEYCODE_v, Common::KEYCODE_i, Common::KEYCODE_e, Common::KEYCODE_w};

uint8 magic_completeme[6] = {Common::KEYCODE_c, Common::KEYCODE_a, Common::KEYCODE_5, Common::KEYCODE_2, Common::KEYCODE_4, Common::KEYCODE_8};

uint8 magic_videocontrol[7] = {Common::KEYCODE_a, Common::KEYCODE_p, Common::KEYCODE_r, Common::KEYCODE_i, Common::KEYCODE_c, Common::KEYCODE_o, Common::KEYCODE_t};

uint32 magico = 0;

void OptionsManager::PollInput() {
	static uint32 counter = 0;

	if (!m_editing) {
		if (!m_slideshowActive) {
			// Escape acts as generic back button for the majority of screens
			if (Read_DI_once_keys(Common::KEYCODE_ESCAPE))
				OnEscapeKey();
		}

		// Selection (up down input)
		if (Read_DI_keys(Common::KEYCODE_DOWN) || Read_DI_keys(down_key) || Read_Joystick(Y_AXIS) > 100) {
			MoveSelected(TRUE8);
		} else if (Read_DI_keys(Common::KEYCODE_UP) || Read_DI_keys(up_key) || Read_Joystick(Y_AXIS) < -100) {
			MoveSelected(FALSE8);
		} else {
			m_moveLimiter = FALSE8;
		}

		// Choose command
		if (Read_DI_keys(Common::KEYCODE_RETURN) || Read_DI_keys(fire_key) || Read_DI_keys(interact_key) || Read_Joystick(0)) {
			DoChoice();
		} else {
			m_choiceLimiter = FALSE8;
		}

		// Alter current selection (left right input)
		if (Read_DI_keys(Common::KEYCODE_LEFT) || Read_DI_keys(left_key) || Read_Joystick(X_AXIS) < -100) {
			AlterSelected(FALSE8);
		} else if (Read_DI_keys(Common::KEYCODE_RIGHT) || Read_DI_keys(right_key) || Read_Joystick(X_AXIS) > 100) {
			AlterSelected(TRUE8);
		} else {
			m_alterLimiter = FALSE8;
		}

		// Just magic_unlockmovies
		if (m_activeMenu == MAIN_TOP) {
			if (magico >= 13) {
				magico = 0;
				DoSomeMagicStuff();

				DrawWidescreenBorders();
			}
			if (Read_DI_keys(magic_unlockmovies[magico])) {
				magico++;
				counter = 0;
			}
		} else if (m_activeMenu == MAIN_MOVIES) {
			if (magico >= 6) {
				magico = 0;
				g_theSequenceManager->setRate();

				uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
				uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

				g_theOptionsManager->DisplayText(ad, pitch, "Zoom!", 0, SCREEN_DEPTH - 30, SELECTEDFONT, TRUE8);

				surface_manager->Unlock_surface(working_buffer_id);

				surface_manager->Flip();

				g_system->delayMillis(1000);

				DrawWidescreenBorders();
			}
			if (Read_DI_keys(magic_fastmovies[magico])) {
				magico++;
				counter = 0;
			}
		} else if (m_activeMenu == MAIN_EXTRAS) {
			if (magico >= 6) {
				magico = 0;

				// Complete game cheat
				uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
				uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

				g_theOptionsManager->DisplayText(ad, pitch, "Extras unlocked", 0, SCREEN_DEPTH - 30, SELECTEDFONT, TRUE8);

				px.game_completed = TRUE8;

				surface_manager->Unlock_surface(working_buffer_id);

				surface_manager->Flip();

				g_system->delayMillis(1000);

				DrawWidescreenBorders();
			}
			if (Read_DI_keys(magic_completeme[magico])) {
				magico++;
				counter = 0;
			}
		} else if (m_activeMenu == MAIN_PROFILES) {
			if (magico >= 7) {
				magico = 0;

				g_av_userControlled = TRUE8;

				uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
				uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

				g_theOptionsManager->DisplayText(ad, pitch, "Rabview enabled", 0, SCREEN_DEPTH - 30, SELECTEDFONT, TRUE8);

				surface_manager->Unlock_surface(working_buffer_id);

				surface_manager->Flip();

				g_system->delayMillis(1000);

				DrawWidescreenBorders();
			}
			if (Read_DI_keys(magic_avcontrol[magico])) {
				magico++;
				counter = 0;
			}
		} else if (m_activeMenu == MAIN_VIDEO) {
			if (magico >= 7) {
				magico = 0;

				// Toggle extra video options available
				if (g_videoOptionsCheat == FALSE8)
					g_videoOptionsCheat = TRUE8;
				else
					g_videoOptionsCheat = FALSE8;

				uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
				uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

				g_theOptionsManager->DisplayText(ad, pitch, "OK", 0, SCREEN_DEPTH - 30, SELECTEDFONT, TRUE8);

				surface_manager->Unlock_surface(working_buffer_id);

				surface_manager->Flip();

				g_system->delayMillis(1000);

				DrawWidescreenBorders();
			}
			if (Read_DI_keys(magic_videocontrol[magico])) {
				magico++;
				counter = 0;
			}

			if (g_videoOptionsCheat == FALSE8) {
				// Illegal selections without cheat
				if (m_VIDEO_selected == SHADOWS)
					m_VIDEO_selected = SEMITRANS;
				if (m_VIDEO_selected == FRAMELIMITER)
					m_VIDEO_selected = LEAVE;
			}
		}
	}

	counter++;
	// Cycling
	if (counter == 20) {
		magico = 0;
		counter = 0;
	}
}

void OptionsManager::DoCredits() {
	if (m_creditControl == FALSE8) {
		// Make credits file name
		char textFileName[128];
		char movieFileName[128];

		sprintf(textFileName, "%s.crd", gamelanguage);
		sprintf(movieFileName, "gmovies\\title.bik");

		// Free the sequence manager
		UnloadTitleScreenMovie();
		m_crediter.Initialise(textFileName, movieFileName, TRUE8, TRUE8, 0);
		m_creditControl = TRUE8;
	} else {
		if (m_crediter.DoScreen() == 0) {
			// Reinstate the title screen movie
			LoadTitleScreenMovie();
			m_creditControl = FALSE8;
			stub.Pop_stub_mode();
		}
	}
}

void OptionsManager::InitialiseScrollingText(const char *textFileName, const char *movieFileName, int frameStart) {
	// Free the sequence manager
	UnloadTitleScreenMovie();

	// If this is a credits file then we need to attach the bink logo at the end
	bool8 appendLogo = FALSE8;
	if (strcmp(textFileName, "english.crd") == 0)
		appendLogo = TRUE8;
#ifdef PC_DEMO
	m_crediter.Initialise(textFileName, movieFileName, TRUE8, appendLogo, frameStart);
#else
	m_crediter.Initialise(textFileName, movieFileName, FALSE8, appendLogo, frameStart);
#endif
	m_creditControl = TRUE8;
}

void OptionsManager::DoScrollingText() {
	if (m_crediter.DoScreen() == 0) {
		m_creditControl = FALSE8;
		stub.Pop_stub_mode();
	}
}

bool8 IsAValidSlide(uint32 num, char *slideFile) {
	// Make the correct filename for this pic when clustered up
	if (num < 10)
		sprintf(slideFile, "images\\pc\\slide_0%d.bink", num);
	else
		sprintf(slideFile, "images\\pc\\slide_%d.bink", num);

	uint32 fo, fs;

	// Now see if it exists in the cluster
	if (!DoesClusterContainFile(pxVString("a\\2dart"), HashString(slideFile), fo, fs))
		return FALSE8;

	return TRUE8;
}

#define MAX_SLIDES 30

void OptionsManager::InitialiseSlideShow() {
	// Set to full screen
	m_pageOn_from.left = m_pageOn_from.top = 0;
	m_pageOn_from.right = SCREEN_WIDTH;
	m_pageOn_from.bottom = SCREEN_DEPTH;

	m_pageOff_from.left = m_pageOff_from.top = 0;
	m_pageOff_from.right = SCREEN_WIDTH;
	m_pageOff_from.bottom = SCREEN_DEPTH;

	m_pageOff_dest.left = 0;
	m_pageOff_dest.right = 0;
	m_pageOff_dest.top = 0;
	m_pageOff_dest.bottom = SCREEN_DEPTH;

	surface_manager->Clear_surface(m_mySlotSurface1ID);

	m_currentSlide = 0;
	m_slideshowActive = TRUE8;
	m_slideWadger = 0;
	m_slideFillColour = 0;
}

#define WADGE_INCREMENTS 30

void OptionsManager::DrawSlideShow() {
	char slideFile[128];

	// Quit
	if (Read_DI_once_keys(Common::KEYCODE_ESCAPE) || Read_Joystick_once(pause_button)) {
		m_slideshowActive = FALSE8;
		DrawWidescreenBorders();
		return;
	}

	// The swap slides routine
	if (m_slideWadger != 0) {
		// Decrementing slide
		if (m_slideWadger < 0) {
			// Are we done
			if (m_slideWadger == -1) {
				// Move to the next valid slide backwards
				if (m_currentSlide == 0)
					m_currentSlide = MAX_SLIDES;
				else
					m_currentSlide--;

				while (!IsAValidSlide(m_currentSlide, slideFile)) {
					m_currentSlide--;
				}
			} else {
				m_pageOn_from.right = SCREEN_WIDTH;
				m_pageOn_from.left = (WADGE_INCREMENTS + m_slideWadger) * 20;

				// Stretchy blit
				surface_manager->Blit_surface_to_surface(m_mySlotSurface1ID, working_buffer_id, &m_pageOn_from, NULL, 0);
			}

			m_slideWadger++;
		} else
		    // Incrementing slide
		    if (m_slideWadger > 0) {
			// Are we done
			if (m_slideWadger == 1) {
				// Do we have any more slides
				if (IsAValidSlide(m_currentSlide + 1, slideFile))
					m_currentSlide++;
				else
					m_currentSlide = 0;

				// Safety check
				if (m_currentSlide > MAX_SLIDES)
					Fatal_error("Slideshow all confused - hit AndyB");
			} else {
				m_pageOn_from.left = 0;
				m_pageOn_from.right = SCREEN_WIDTH - ((WADGE_INCREMENTS - m_slideWadger) * 20);

				// Stretchy blit
				surface_manager->Blit_surface_to_surface(m_mySlotSurface1ID, working_buffer_id, &m_pageOn_from, NULL, 0);
			}

			m_slideWadger--;
		}

		// Could do an effect on the working buffer here
	} else {
		// Alter current visible slide (on left right input)
		if (Read_DI_keys(Common::KEYCODE_LEFT) || Read_DI_keys(left_key) || Read_Joystick(X_AXIS) < -100) {
			if (!m_slideLimiter) {
				m_slideLimiter = TRUE8;

				// Caught by the swap slide routine above
				m_slideWadger = -WADGE_INCREMENTS;
			}
		} else if (Read_DI_keys(Common::KEYCODE_RIGHT) || Read_DI_keys(right_key) || Read_Joystick(X_AXIS) > 100) {
			if (!m_slideLimiter) {
				m_slideLimiter = TRUE8;

				// Caught by the swap slide routine above
				m_slideWadger = WADGE_INCREMENTS;
			}
		} else {
			m_slideLimiter = FALSE8;
		}

		// Clean the screen first off
		surface_manager->Fill_surface(m_mySlotSurface1ID, m_slideFillColour);

		uint32 slideFileHash = NULL_HASH;
		char art2DCluster[MAXLEN_CLUSTER_URL];
		uint32 art2DClusterHash = NULL_HASH;

		if (!IsAValidSlide(m_currentSlide, slideFile))
			Fatal_error("Trying to display a non-existant slide image!");

		// Set this up for resman and open the thb file
		sprintf(art2DCluster, ICON_CLUSTER_PATH);

		uint8 *slideptr = rs1->Res_open(slideFile, slideFileHash, art2DCluster, art2DClusterHash);

		// This slide is bink compressed
		HBINK binkHandle = BinkOpen((const char *)slideptr, BINKFROMMEMORY | BINKNOSKIP);

		if (binkHandle == NULL)
			Fatal_error("BinkOpen Failed with %s", BinkGetError());

		// Verify image dimensions
		if (binkHandle->Width > SCREEN_WIDTH || binkHandle->Height > SCREEN_DEPTH)
			Fatal_error("Slide image is too large to fit screen!");

		// Let bink do it stuff
		BinkDoFrame(binkHandle);

		// Lock the buffers now so bink has somewhere ot put it's data
		void *surface = (void *)surface_manager->Lock_surface(m_mySlotSurface1ID);
		uint32 pitch = surface_manager->Get_pitch(m_mySlotSurface1ID);

		// Go Bink go ...
		uint32 binkFlags = BINKNOSKIP;
		if (surface_manager->Get_BytesPP(m_mySlotSurface1ID) == 3) {
			binkFlags |= BINKSURFACE24;
		} else if (surface_manager->Get_BytesPP(m_mySlotSurface1ID) == 4) {
			binkFlags |= BINKSURFACE32;
		}

		// Screen coordinates
		uint32 m_x = 0;
		uint32 m_y = 0;

		// Centre of the screen please
		if (binkHandle->Width != SCREEN_WIDTH) {
			m_x = (SCREEN_WIDTH / 2) - (binkHandle->Width / 2);
		}
		if (binkHandle->Height != SCREEN_DEPTH) {
			m_y = (SCREEN_DEPTH / 2) - (binkHandle->Height / 2);
		}

		BinkCopyToBuffer(binkHandle, surface, pitch, SCREEN_DEPTH, m_x, m_y, binkFlags);

		// Get the first pixel colour
		m_slideFillColour = *((int32 *)surface + m_x + (m_y * pitch));

		surface_manager->Unlock_surface(m_mySlotSurface1ID);

		BinkClose(binkHandle);

		// Update the screen
		surface_manager->Blit_surface_to_surface(m_mySlotSurface1ID, working_buffer_id, NULL, NULL, 0);

		// Now ensure the slide surroundings are the correct colour at this cycle

		// If the slide has width less than SCREEN_WIDTH
		if (m_x != 0) {
			Fill_rect(0, 0, m_x, SCREEN_DEPTH, m_slideFillColour);
			Fill_rect(SCREEN_WIDTH - m_x, 0, SCREEN_WIDTH, SCREEN_DEPTH, m_slideFillColour);
		}
		// If the slide has height less than SCREEN_DEPTH
		if (m_y != 0) {
			Fill_rect(m_x, 0, SCREEN_WIDTH - m_x, m_y, m_slideFillColour);
			Fill_rect(m_x, SCREEN_DEPTH - m_y, SCREEN_WIDTH - m_x, SCREEN_DEPTH, m_slideFillColour);
		}
	}

	uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	// Print a helpful message
	const char *msg = GetTextFromReference(HashString("opt_slideshowmessage"));
	DisplayText(ad, pitch, msg, 10, SCREEN_DEPTH - 10 - m_fontHeight, PALEFONT, FALSE8);

	surface_manager->Unlock_surface(working_buffer_id);
}

// Generic symbols in the text file
#define TITLE_LINE_SYMBOL '*'
#define CENTRE_LINE_SYMBOL '!'
// Version specific symbols in the text file
#define IGNORE_LINE_SYMBOL '+'
#define OKAY_LINE_SYMBOL '-'

// Defines
#define CREDIT_LINE_SPACING 20
#define CREDITS_PER_SCREEN ((480 / CREDIT_LINE_SPACING) + 1)

// Line types
#define IGNORE_LINE 0x00
#define CENTRED_LINE 0x01
#define TITLE_LINE 0x02
#define PERSON_LINE 0x03
#define PC_LINE 0x04
#define EMPTY_LINE 0x05

void LoadLogo(uint32 to_surface_id) {
	char thbFile[128];
	uint32 thbFileHash = NULL_HASH;
	char art2DCluster[MAXLEN_CLUSTER_URL];
	uint32 art2DClusterHash = NULL_HASH;

	// Make the correct filename for this pic when clustered up
	sprintf(thbFile, "images\\pc\\binklogo.thb");

	// Set this up for resman and open the thb file
	sprintf(art2DCluster, ICON_CLUSTER_PATH);
	uint8 *data = (uint8 *)rs1->Res_open(thbFile, thbFileHash, art2DCluster, art2DClusterHash);

	// First off, check the thumb surface is valid
	if (!to_surface_id)
		Fatal_error("LoadLogo() cannot read to a null surface");

	// Now lock the fucker
	uint8 *surface_address = surface_manager->Lock_surface(to_surface_id);
	uint32 pitch = surface_manager->Get_pitch(to_surface_id);

	// Now we need to read the 60 by 60 image data into the surface
	for (uint32 i = 0; i < 60; i++) {
		for (uint32 j = 0; j < 60; j++) {
			*surface_address++ = *data++;
			*surface_address++ = *data++;
			*surface_address++ = *data++;
			*surface_address++ = *data++;
		}

		surface_address += (pitch - 240);
	}

	// Release it now
	surface_manager->Unlock_surface(to_surface_id);
}

uint32 ExamineCharacter(char c) {
	if (c == TITLE_LINE_SYMBOL)
		return TITLE_LINE;

	if (c == CENTRE_LINE_SYMBOL)
		return CENTRED_LINE;

	if (c == IGNORE_LINE_SYMBOL)
		return IGNORE_LINE;

	if (c == OKAY_LINE_SYMBOL)
		return PC_LINE;

	if (c == 0)
		return EMPTY_LINE;

	// Must be a regular name line
	return PERSON_LINE;
}

uint32 GetFileSz(const char *path) {
	Common::File file;

	if (!file.open(path)) {
		return 0;
	}

	return (uint32)file.size();
}

Crediter::Crediter()
    : m_creditsFile(NULL), m_numberOfBytes(0), m_endOfCredits(0), m_currentHeight(0), m_cursor(0), m_scrollOffset(0), m_logoSurfaceID(0), m_logoDraw(0), m_logoAttached(0),
      m_movieSurfaceID(0), m_movieBackdrop(FALSE8), m_loopingMovie(FALSE8), m_frameStart(0), m_totalMovieFrames(0) {
	memset(m_theData, 0, MAX_BYTESIZE_OF_CREDITS_FILE);
	m_movieRect.left = m_movieRect.right = m_movieRect.bottom = m_movieRect.top = 0;
}

void Crediter::Initialise(const char *textFileName, const char *movieFileName, bool8 loopingMovie, bool8 attachLogo, int frameStart) {
	// Zero out our memory
	memset(m_theData, 0, MAX_BYTESIZE_OF_CREDITS_FILE);

	// Set this up for resman and open the file
	char globalClusterFile[MAXLEN_CLUSTER_URL];
	sprintf(globalClusterFile, GLOBAL_CLUSTER_PATH);
	uint32 globalClusterHash = NULL_HASH;
	uint32 buf_hash = NULL_HASH;

	m_loopingMovie = loopingMovie;
	m_frameStart = frameStart;

	uint8 *data;
	data = (uint8 *)rs1->Res_open(textFileName, buf_hash, globalClusterFile, globalClusterHash, 0, &m_numberOfBytes);

	// Check the size is ok
	if (m_numberOfBytes > MAX_BYTESIZE_OF_CREDITS_FILE)
		Fatal_error(pxVString("Credits file exceeds budget! (%d > %d)", m_numberOfBytes, MAX_BYTESIZE_OF_CREDITS_FILE));

	// Read the file into our private memory
	memcpy(m_theData, data, m_numberOfBytes);

	// Setup pointer to the file data
	m_creditsFile = (char *)m_theData;

	// Process the file first
	int i = 0;
	while (m_creditsFile[i]) {
		// New line encountered (NB: two bytes, carriage return and line feed)
		if (m_creditsFile[i] == 0x0d) {
			// Overwrite with terminators
			m_creditsFile[i] = 0;
			m_creditsFile[i + 1] = 0;
			i += 2;
		} else
			i++;
	}

	m_endOfCredits = -500;

	if (m_frameStart == 0)
		m_scrollOffset = 0;
	else
		m_scrollOffset = -1 * (SCREEN_DEPTH / 2) + 65;

	m_logoAttached = attachLogo;

	if (m_logoAttached) {
		// Load the bink logo
		m_logoSurfaceID = surface_manager->Create_new_surface("Bink logo", 60, 60, SYSTEM);
		LoadLogo(m_logoSurfaceID);
		m_logoDraw = -1;
	}

	// Now sort out the movie to play as the backdrop
	if (movieFileName == NULL) {
		m_movieBackdrop = FALSE8;
	} else {
		// Safety check
		if (g_personalSequenceManager->busy() == TRUE8)
			Fatal_error("Crediter() class: Can't use this sequence manager (g_personalSequenceManager) as it's busy");

		m_movieBackdrop = TRUE8;

		// Initialise background movie (looping)
		if (!g_personalSequenceManager->registerMovie(movieFileName, 0, m_loopingMovie)) {
			Fatal_error(pxVString("Couldn't register the movie: %s", movieFileName));
		}

		// Calculate movie blitting rectangle
		uint32 movieWidth = g_personalSequenceManager->getMovieWidth();
		uint32 movieHeight = g_personalSequenceManager->getMovieHeight();
		m_totalMovieFrames = g_personalSequenceManager->getMovieFrames();

		if (m_frameStart >= m_totalMovieFrames)
			Fatal_error("Crediter() class: Can't start scrolling text at frame %d when movie only has %d frames", m_frameStart, m_totalMovieFrames);

		m_movieRect.left = 0;
		m_movieRect.top = 0;

		if (movieWidth != SCREEN_WIDTH) {
			m_movieRect.left = (SCREEN_WIDTH / 2) - (movieWidth / 2);
		}
		if (movieHeight != SCREEN_DEPTH) {
			m_movieRect.top = (SCREEN_DEPTH / 2) - (movieHeight / 2);
		}

		m_movieRect.right = m_movieRect.left + movieWidth;
		m_movieRect.bottom = m_movieRect.top + movieHeight;

		// Get a new surface for the movie
		m_movieSurfaceID = surface_manager->Create_new_surface("Crediter Movie", SCREEN_WIDTH, SCREEN_DEPTH, SYSTEM);
	}
}

int32 Crediter::DoScreen() {
	uint32 halfScreenW = SCREEN_WIDTH / 2;
	uint32 halfScreenH = SCREEN_DEPTH / 2;
	LRECT logo_rect;
	bool8 doText = FALSE8;
	bool8 onlastMovieFrame = FALSE8;

	// Are we done
	if (m_endOfCredits == 0 || Read_DI_keys(Common::KEYCODE_ESCAPE) || Read_Joystick_once(pause_button)) {
		if (m_logoAttached)
			surface_manager->Kill_surface(m_logoSurfaceID);
		if (m_movieBackdrop)
			surface_manager->Kill_surface(m_movieSurfaceID);
		g_personalSequenceManager->kill();
		return 0;
	}

	// Need to hold on last frame if not looping the movie and it ends
	if (m_loopingMovie == FALSE8 && m_totalMovieFrames == g_personalSequenceManager->getFrameNumber()) {
		onlastMovieFrame = TRUE8;
	}

	if (m_movieBackdrop == FALSE8) {
		// Clear screen
		surface_manager->Fill_surface(working_buffer_id, 0x001010);
	} else {
		// Control what frame we start to display the scrolling text
		if (m_frameStart <= g_personalSequenceManager->getFrameNumber())
			doText = TRUE8;

		// Draw a frame of the movie
		if (onlastMovieFrame == FALSE8)
			g_personalSequenceManager->drawFrame(m_movieSurfaceID);

		// Now blit the movie frame without transparency to the working buffer
		surface_manager->Blit_surface_to_surface(m_movieSurfaceID, working_buffer_id, &m_movieRect, &m_movieRect, 0);
	}

	// Only draw the scrolling text after a certain frame number
	if (doText == FALSE8)
		return 1;

	// Reset
	m_currentHeight = halfScreenH;
	m_cursor = 0;

	// Scroll up please
	m_currentHeight -= m_scrollOffset;

linesDone:

	// Do that funky chicken
	if (m_currentHeight < 0) {
		uint32 r = ExamineCharacter(m_creditsFile[m_cursor]);

		if (r == TITLE_LINE) {
			// Need to skip an extra line if we find a title character
			m_cursor += strlen(&m_creditsFile[m_cursor]) + 2;

			// Check next line as well
			uint32 r2 = ExamineCharacter(m_creditsFile[m_cursor]);
			// Special case if next line is to be ignored
			if (r2 == IGNORE_LINE)
				goto linesDone;
		} else if (r == IGNORE_LINE) {
			// Don't go to new line if we're to ignore this line
			m_cursor += strlen(&m_creditsFile[m_cursor]) + 2;
			goto linesDone;
		} else if (r == PC_LINE) {
			m_cursor++;
			goto linesDone;
		}

		m_cursor += strlen(&m_creditsFile[m_cursor]) + 2;
		m_currentHeight += CREDIT_LINE_SPACING;

		goto linesDone;
	}

	// Now lock the fucker
	uint8 *ad = surface_manager->Lock_surface(working_buffer_id);
	uint32 pitch = surface_manager->Get_pitch(working_buffer_id);

	// Draw the credit lines all nicely formatted
	for (uint32 count = 0; TRUE8; count++) {
		// Is this the end of the file
		if (m_cursor >= (uint32)m_numberOfBytes) {
			if (m_logoAttached) {
				// Trigger the bink logo drawing
				if (m_logoDraw == -1)
					m_logoDraw = 470;
			}

			m_endOfCredits++;
			break;
		}

		// Now look at a line
		uint32 ret = ExamineCharacter(m_creditsFile[m_cursor]);

		// An empty line
		if (ret == EMPTY_LINE) {
			// Skip the terminator
			m_cursor += 2;
			// Increment line height
			m_currentHeight += CREDIT_LINE_SPACING;
			// Dont draw this line
		} else if (ret == IGNORE_LINE) {
			// Skip the special character
			m_cursor++;
			// Skip this line
			m_cursor += strlen(&m_creditsFile[m_cursor]) + 2;
			continue;
		} else if (ret == CENTRED_LINE) {
			// Skip the special character
			m_cursor++;
			// Draw line centred
			g_theOptionsManager->DisplayText(ad, pitch, &m_creditsFile[m_cursor], 0, m_currentHeight, NORMALFONT, TRUE8);
			// Move pointer to next line
			m_cursor += strlen(&m_creditsFile[m_cursor]) + 2;
			// Increment line height
			m_currentHeight += CREDIT_LINE_SPACING;
		} else if (ret == TITLE_LINE) {
			// Skip the special character
			m_cursor++;
			// Draw line on the left
			g_theOptionsManager->DisplayText(ad, pitch, &m_creditsFile[m_cursor],
			                                 halfScreenW - g_theOptionsManager->CalculateStringWidth(&m_creditsFile[m_cursor]) - 10, m_currentHeight, PALEFONT, FALSE8);
			// Move pointer to next line
			m_cursor += strlen(&m_creditsFile[m_cursor]) + 2;

			// Don't increment height for title lines unless it's a double title line
			if (ExamineCharacter(m_creditsFile[m_cursor]) == TITLE_LINE)
				m_currentHeight += CREDIT_LINE_SPACING;
		} else if (ret == PERSON_LINE) {
			// Draw line on the right
			g_theOptionsManager->DisplayText(ad, pitch, &m_creditsFile[m_cursor], halfScreenW + 10, m_currentHeight, NORMALFONT, FALSE8);
			// Move pointer to next line
			m_cursor += strlen(&m_creditsFile[m_cursor]) + 2;
			// Increment line height
			m_currentHeight += CREDIT_LINE_SPACING;
		} else {
			// Line must be a forced pc line so take move over this character and do it again
			m_cursor++;
			continue;
		}

		// This is what will usually halt this loop (ie running out of space on the screen)
		if (m_currentHeight > SCREEN_DEPTH - CREDIT_LINE_SPACING)
			break;
	}

	surface_manager->Unlock_surface(working_buffer_id);

	// Scrolling speed
	if (m_endOfCredits < 0) {
		// Increment scrolling offset
		m_scrollOffset++;

		if (m_logoDraw != -1)
			m_logoDraw--;
	}

	// The bink logo at the end of the file
	if (m_logoDraw != -1 && m_logoAttached == TRUE8) {
		logo_rect.top = m_logoDraw;

		int remainder = SCREEN_DEPTH - m_logoDraw;

		if (remainder < 60) {
			logo_rect.bottom = SCREEN_DEPTH - 1;
		} else {
			logo_rect.bottom = logo_rect.top + 60;
		}

		logo_rect.left = halfScreenW - 30;
		logo_rect.right = logo_rect.left + 60;

		surface_manager->Blit_surface_to_surface(m_logoSurfaceID, working_buffer_id, NULL, &logo_rect, 0);
	}

	// Draw border rects
	Fill_rect(0, 0, SCREEN_WIDTH, 67, 0);
	Fill_rect(0, 413, SCREEN_WIDTH, SCREEN_DEPTH, 0);

	return 1;
}

} // End of namespace ICB
