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

#ifndef ICB_OPTIONS_MANAGER_PC_H_INCLUDED
#define ICB_OPTIONS_MANAGER_PC_H_INCLUDED

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/p4.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/sound/fx_manager.h"
#include "engines/icb/sound/speech_manager.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/sound.h"
#include "engines/icb/direct_input.h"

namespace ICB {

class MovieManager;
extern MovieManager *g_personalSequenceManager;

#define MAX_BYTESIZE_OF_CREDITS_FILE (8 * 1024)

class Crediter {
private:
	uint8 m_theData[MAX_BYTESIZE_OF_CREDITS_FILE];
	char *m_creditsFile;
	int32 m_numberOfBytes;

	int32 m_endOfCredits;

	int32 m_currentHeight;
	uint32 m_cursor;
	int32 m_scrollOffset;

	uint32 m_logoSurfaceID;
	int32 m_logoDraw;
	bool8 m_logoAttached;

	uint32 m_movieSurfaceID;
	LRECT m_movieRect;
	bool8 m_movieBackdrop;

	bool8 m_loopingMovie;
	int m_frameStart;
	int m_totalMovieFrames;

public:
	Crediter();
	void Initialise(const char *textFileName, const char *movieFileName, bool8 loopingMovie, bool8 attachLogo, int frameStart);
	int32 DoScreen();
};

// Global options controller
class OptionsManager;
extern OptionsManager *g_theOptionsManager;

// Type to describe every menu in the entire game
enum _MENU_ {
	INGAME_TOP,
	INGAME_OPTIONS,
	INGAME_AUDIO,
	INGAME_VIDEO,
	INGAME_SAVE,
	INGAME_SAVECONFIRM,
	INGAME_LOAD,
	INGAME_CONTROLS,
	INGAME_QUIT,

	MAIN_TOP,
	MAIN_LOAD,
	MAIN_OPTIONS,
	MAIN_AUDIO,
	MAIN_VIDEO,
	MAIN_CONTROLS,
	MAIN_EXTRAS,
	MAIN_MOVIES,
	MAIN_PLAYSELECT,
	MAIN_PROFILES,
	MAIN_QUIT,

	MAIN_A_PROFILE,

	GAME_OVER,
	DEAD_LOAD,
	DEAD_QUIT
};

#define NUMBER_OF_IN_GAME_TOP_CHOICES 5

enum IN_GAME_TOP_CHOICES { CONTINUE = 0, SAVE_GAME = 1, LOAD_GAME = 2, OPTIONS = 3, QUIT = 4 };

#define NUMBER_OPTION_CHOICES 4

enum OPTION_CHOICES {
	VIDEO_SETTINGS = 0,
	AUDIO_SETTINGS = 1,
	CONTROLS = 2,
	BACK = 3
};

#define NUMBER_OF_AUDIO_CHOICES 4

enum AUDIO_CHOICES {
	MUSIC_VOLUME = 0,
	SPEECH_VOLUME = 1,
	SFX_VOLUME = 2,
	DO_ONE = 3
};

#define NUMBER_OF_VIDEO_CHOICES 5

enum VIDEO_CHOICES {
	SUBTITLES = 0,
	SEMITRANS = 1,
	SHADOWS = 2,
	FRAMELIMITER = 3,
	LEAVE = 4
};

#define NUMBER_OF_CONTROL_CHOICES 11

enum CONTROL_CHOICES {
	DEVICE = 0,
	METHOD = 1,
	UP_CROUCH = 2,
	DOWN_INTERACT = 3,
	LEFT_ARM = 4,
	RIGHT_ATTACK = 5,
	RUN_INVENTORY = 6,
	SIDESTEP_REMORA = 7,
	PAUSE = 8,
	DEFAULTS = 9,
	DONE = 10
};

#define TOTAL_NUMBER_OF_GAME_SLOTS 100
#define NUMBER_OF_VISIBLE_GAME_SLOTS 4
#define NUMBER_OF_GAMESLOT_CHOICES 5

enum GAMESLOT_CHOICES {
	SLOT1 = 0,
	SLOT2 = 1,
	SLOT3 = 2,
	SLOT4 = 3,
	RETURN = 4
};

#define NUMBER_OF_SAVECONFIRM_CHOICES 2

enum SAVECONFIRM_CHOICES { YEY = 0, NAY = 1 };

#define NUMBER_OF_QUIT_CHOICES 2

enum QUIT_CHOICES { YES = 0, NO = 1 };

#define NUMBER_OF_MAIN_TOP_CHOICES 5

enum MAIN_TOP_CHOICES {
	_NEWGAME = 0,
	_LOAD_GAME = 1,
	_OPTIONS = 2,
	_EXTRAS = 3,
	_EXIT_GAME = 4
};
#define NUMBER_OF_EXTRA_CHOICES 6

enum M_EXTRA_CHOICES {
	MOVIES = 0,
	SLIDESHOW = 1,
	PLAYSELECT = 2,
	PROFILES = 3,
	CREDITS = 4,
	ALLDONE = 5
};
//-----------------------------------------------------------------------
#define NUMBER_OF_PLAYSELECT_CHOICES 10

enum M_PLAYSELECT_CHOICES {
	M01 = 0,
	M02 = 1,
	M03 = 2,
	M04 = 3,
	M05 = 4,
	M07 = 5,
	M08 = 6,
	M09 = 7,
	M10 = 8,
	CANCEL = 9
};
//-----------------------------------------------------------------------
#define TOTAL_NUMBER_OF_MOVIES 47
#define M_NUMBER_OF_VISIBLE_MOVIE_SLOTS 12
#define M_NUMBER_OF_MOVIE_CHOICES 13

enum M_MOVIE_CHOICES {
	MOVIE01 = 0,
	MOVIE02 = 1,
	MOVIE03 = 2,
	MOVIE04 = 3,
	MOVIE05 = 4,
	MOVIE06 = 5,
	MOVIE07 = 6,
	MOVIE08 = 7,
	MOVIE09 = 8,
	MOVIE10 = 9,
	MOVIE11 = 10,
	MOVIE12 = 11,
	NOTHANKS = 12, // Excluded from name display if greater than this
	PAGELEFT = 13,
	PAGERIGHT = 14
};
#define M_NUMBER_OF_PROFILE_CHOICES 11

enum M_PROFILES_CHOICES {
	CORD = 0,
	CHI = 1,
	GREGOR = 2,
	NAGAROV = 3,
	LUKYAN = 4,
	KEIFFER = 5,
	TOLSTOV = 6,
	ALEXANDRA = 7,
	OLIAKOV = 8,
	SPECTRE = 9,
	RET = 10
};
#define NUMBER_OF_GAMEOVER_CHOICES 3

enum GAMEOVER_CHOICES { RESTORE = 0, RESTART = 1, FUCKTHAT = 2 };

#define MAX_LABEL_LENGTH 24

typedef struct {
	char label[MAX_LABEL_LENGTH];
	uint32 secondsPlayed;

} _SLOT;

typedef struct {
	char filename[MAX_LABEL_LENGTH];
	bool8 visible;

} _MOVIESLOT;

// These initialised from a saved game and switched when movies are played
extern _MOVIESLOT g_movieLibrary[TOTAL_NUMBER_OF_MOVIES];

// Anti-aliased support provided for these coloured fonts (DisplayText)
#define NORMALFONT 0x0000
#define SELECTEDFONT 0x0001
#define PALEFONT 0x0002

class OptionsManager {
	friend class Crediter;

private:
	bool8 m_inGame;        // In-game or title screen flag
	bool8 m_useDirtyRects; // For increased speed over a static background
	bool8 m_gameover;      // Game Over flag
	bool8 m_haveControl;   // OptionsManager active flag
	bool8 m_thatsEnoughTa; // Quit flag

	int32 m_autoAnimating; // Animating flags
	int32 m_autoAnimating2;
	LRECT m_optionsBox;       // Top level screen bounding box
	int32 m_over_n_Frames;    // Animating variable
	int32 m_grower;           // Animating variable
	uint32 m_lipLength;       // Length of top-level box lips
	LRECT m_box;              // Animating variable
	LRECT m_targetBox;        // Child screen title box
	int32 m_interFrames;      // Animating variable
	int32 m_widthIncrements;  // Animating variable
	int32 m_bottomIncrements; // Animating variable
	int32 m_topIncrements;    // Animating variable
	bool8 m_warpDirection;    // Direction of animation

	char m_fontName[ENGINE_STRING_LEN]; // Bitmap font handling
	_pxBitmap *m_font_file;
	uint32 *m_fontPalette;
	uint32 m_fontHeight;
	_pxPCSprite *m_currentSprite;
	_rgb m_selectedShade;
	_linked_data_file *m_global_text;

	_rgb m_drawColour; // Current primitive drawing colour

	_MENU_ m_activeMenu; // Active menu identifier

	_SLOT *m_slots[TOTAL_NUMBER_OF_GAME_SLOTS]; // Save slot pointers
	uint32 m_slotOffset;                        // Offset to current slot page
	uint32 m_movieOffset;                       // Offset to current movie page

	// Which option is selected for the main choices
	MAIN_TOP_CHOICES m_M_TOP_selected;
	M_EXTRA_CHOICES m_M_EXTRA_selected;
	M_MOVIE_CHOICES m_M_MOVIE_selected;
	M_PLAYSELECT_CHOICES m_M_PLAYSELECT_selected;
	M_PROFILES_CHOICES m_M_PROFILES_selected;

	// Which option is selected for the in-game choices
	IN_GAME_TOP_CHOICES m_IG_TOP_selected;

	// Generic chosers used in both option screens
	OPTION_CHOICES m_OPTION_selected;
	VIDEO_CHOICES m_VIDEO_selected;
	AUDIO_CHOICES m_AUDIO_selected;
	CONTROL_CHOICES m_CONTROL_selected;
	GAMESLOT_CHOICES m_GAMESLOT_selected;
	SAVECONFIRM_CHOICES m_SAVECONFIRM_selected;
	QUIT_CHOICES m_QUIT_selected;
	GAMEOVER_CHOICES m_GAMEOVER_selected;

	// Extra temporary direct draw surfaces to work with
	uint32 m_myScreenSurfaceID;
	uint32 m_mySlotSurface1ID;
	uint32 m_thumbSurfaceIDs[8];
	uint32 m_grayThumbSurfaceIDs[8];
	uint32 m_movieSurfaceIDs[24];
	uint32 m_grayMovieSurfaceIDs[24];
	uint32 m_profileSurface;

	LRECT m_fullscreen; // Useful rectangle
	LRECT m_movieRect;  // Paging rectangle
	uint32 m_colourKey; // Blitting colour key

	bool8 m_moveLimiter;        // Vertical input flag
	bool8 m_alterLimiter;       // Horizontal input flag
	bool8 m_choiceLimiter;      // Select input flag
	bool8 m_controlPage1;       // Control screen page indicator
	uint32 m_controlAnimCursor; // Animation sequence position

	int m_move_sfx_channel;   // Audio channel for move sfx
	int m_choose_sfx_channel; // Audio channel for select sfx

	bool8 m_canSave; // Is save disabled in game

	// Edit label buffer position
	uint32 m_cursorPos;
	char m_editBuffer[MAX_LABEL_LENGTH];
	char m_defaultSlotName[MAX_LABEL_LENGTH];
	bool8 m_editing;
	bool8 m_defaultWiper;
	int m_emptySlotFlag;

	uint32 m_timePlayed;

	// For animating and handling slot pages
	bool8 m_paging;              // Are we currently paging (animating)
	bool8 m_pageleft;            // Left or right paging
	LRECT m_slotBoundingRect;    // Box containing the 10 visible slots
	int32 m_slotsFuckOffBy;      // Animation step incrementer
	LRECT m_pageOn_from;         // Source blit rect for slots entering the screen
	LRECT m_pageOn_dest;         // Target blit rect for slots entering the screen
	LRECT m_pageOff_from;        // Source blit rect for slots leaving the screen
	LRECT m_pageOff_dest;        // Target blit rect for slots leaving the screen
	bool8 m_letJoystickQuitEdit; // Flag to restrain joystick presses

	bool8 m_awaitingKeyPress; // For control assignment
	bool8 m_configLimiter;
	int m_assignFlash;

	bool8 m_creditControl; // Credits displaying flag
	Crediter m_crediter;   // Crediter class instance

	bool8 m_slideshowActive; // Slideshow active flag
	bool8 m_slideLimiter;    // Slideshow variables
	int32 m_currentSlide;
	int32 m_slideWadger;
	uint32 m_slideFillColour;

	uint32 m_margin; // Profile screen variables
	LRECT m_profileRect;
	int32 m_profileScrollingOffset;
	int32 m_profileScrollingLine;
	bool8 m_lastLineDisplayed;
	int32 m_profileScrolling;

public:
	OptionsManager();  // Constructor
	~OptionsManager(); // Destructor

	// Initialise menu functions
	void StartInGameOptions();
	void StartMainOptions();
	void StartGameOverOptions();

	// Cycle
	void CycleLogic();

	// Force screen refresh
	void ForceInGameScreenRefresh();

	// Enquire as to present status
	bool8 HasControl() { return m_haveControl; }

	// Public access routines
	void DoCredits();
	void InitialiseScrollingText(const char *textFileName, const char *movieFileName, int frameStart);
	void DoScrollingText();
	void DisplayText(uint8 *ad, uint32 pitch, const char *str, int32 x, int32 y, uint32 col, bool8 centredHorizontally, bool8 boxed = FALSE8);
	const char *GetTextFromReference(uint32 hashRef);
	uint32 CalculateStringWidth(const char *str);
	void UnlockMovies();

private:
	// Logic
	void CycleInGameOptionsLogic();
	void CycleMainOptionsLogic();
	void CycleGameOverLogic();
	void InitialiseInGameOptions();
	void MoveSelected(bool8 _down_);
	void AlterSelected(bool8 _right_);
	void DoChoice();
	void OnEscapeKey();

	// Input
	void PollInput();

	// Graphics handling
	void MakeAllSurfii();
	void KillAllSurfii();

	void SetDrawColour(uint32 def);
	void DrawRectangle(bool8 selected, uint32 x, uint32 y, uint32 width, uint32 height, uint8 *surface_address, uint32 pitch);
	void DarkenScreen();
	void BloodScreen();
	void SetDesiredOptionsBoxParameters(uint32 width, uint32 height, uint32 nFrames);
	void SetTargetBox(uint32 x1, uint32 x2, uint32 y1, uint32 y2, uint32 nFrames);
	bool8 AnimateThoseBrackets(bool8 forwards);
	bool8 AnimateBracketsToBox(bool8 forwards, uint32 surface_id = working_buffer_id);
	void AnimateSlotsPaging();
	void GrabThumbnailImage();
	void LoadVisibleThumbnails();
	void LoadPagingThumbnails(uint32 slotOffset);
	void MakeGrayScaleThumbnail(uint32 src_surface_id, uint32 dst_surface_id, uint32 w = 64, uint32 h = 48);
	void LoadVisibleMovieShots();
	void LoadPagingMovieShots(uint32 slotOffset);
	void DrawWidescreenBorders();
	void DrawPageIndicator(uint32 x, uint32 y, bool8 up, bool8 selected, uint8 *ad, uint32 pitch);
	void FadeStrip(uint32 x, uint32 y, uint32 w, bool8 up, uint8 *ad, uint32 pitch);

	// Screen drawing functions
	void DrawMainOptionsScreen(uint32 surface_id);
	void DrawInGameOptionsScreen();
	void DrawGameOverScreen();
	void DrawGameSlots(uint32 slotOffset, uint32 surface_id = working_buffer_id);
	void DrawMovieSlots(uint32 offset, uint32 surface_id = working_buffer_id);
	void DrawGameOptions();
	void DrawAudioSettings();
	void DrawVideoSettings();
	void DrawControllerConfiguration();
	void DrawControls(uint32 surface_id = working_buffer_id);
	void DrawMainLoadScreen(uint32 surface_id = working_buffer_id);
	void DrawMovieScreen(uint32 surface_id = working_buffer_id);
	void DrawQuitGameConfirmScreen(uint32 surface_id = working_buffer_id);
	void DrawExtrasScreen(uint32 surface_id = working_buffer_id);
	void DrawPlaySelectScreen(uint32 surface_id = working_buffer_id);
	void DrawSlideShow();
	void DrawProfileSelectScreen(uint32 surface_id = working_buffer_id);
	void DrawProfileScreen(uint32 surface_id = working_buffer_id);
	void DrawOverwriteSaveConfirmScreen(uint32 surface_id = working_buffer_id);

	// Screen initialising functions
	void InitialiseControlsScreen();
	void InitialiseSlideShow();
	void InitialiseAProfile();

	// Slot management
	void InitialiseSlots();
	void DestroySlots();

	// Assign control functions
	void EditSlotLabel();
	bool8 VerifyLabel();
	void GetKeyAssignment();

	// Font handling
	void LoadBitmapFont();
	void LoadGlobalTextFile();
	bool8 SetCharacterSprite(char c);
	void RenderCharacter(uint8 *ad, uint32 pitch, int32 nX, int32 nY, uint32 col);
	void GetCentredRectFotText(const char *str, LRECT &r, int32 height);

	// Option screen sound functions
	void InitialiseSounds();
	void PlayMoveFX();
	void PlayChosenFX();

	// For the movie library
	void PlayMovie(uint32 id);

	// Title screen movie backdrop
	void LoadTitleScreenMovie();
	void UnloadTitleScreenMovie();
};

} // End of namespace ICB

#endif
