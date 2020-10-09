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

#ifndef ICB_REMORA_H_INCLUDED
#define ICB_REMORA_H_INCLUDED

// Include headers needed by this class.
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_bitmap.h"
#include "engines/icb/common/px_mapfile.h"
#include "engines/icb/p4.h"
#include "engines/icb/debug.h"
#include "engines/icb/player.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/remora_sprite.h"
#include "engines/icb/icon_list_manager.h"

namespace ICB {

// This definition works because the PC and PSX have their own definitions of SCREEN_WIDTH and SCREEN_DEPTH.
#define REMORA_SCREEN_WIDTH SCREEN_WIDTH
#define REMORA_SCREEN_HEIGHT SCREEN_DEPTH

// The maximum number of megas the remora will do brightness scaling for the radar sweep.
// Note, it needs to be multiple of 4 to get DWORD alignment
#define REMORA_MAX_NUMBER_OBJECTS ((MAX_session_objects + 3) & ~3)

#define REMORA_LOG "remora_log.txt"

// This is what the Remora MUST be called in the session.
#define REMORA_NAME "remora"

// Resource names.
#define REMORA_BITMAP_REMORA "remora" // This is the casing for the Remora, with a hole where the screen is.

// PC's blips have all been renamed.  The PSX might continue using just this one thoug, 'cos it
// can do its own colouring.
#if defined(_PSX)
#define REMORA_BITMAP_SCREEN "screen" // This is the screen that fills the hole that swallowed the spider ...
#define REMORA_FLIC_BLIP "blip"
#endif

// This is used as a constant to indicate the Remora is not active.
#define REMORA_NOT_ACTIVE (-1)

// This is the default activation mode.
#define REMORA_DEFAULT_MODE MOTION_SCAN

// This defines a buffer size for unpacking and formatting all that goes on the screen.
#define REMORA_TEXT_BUFFER_WIDTH 61 // CHECK PSX PADDING BEFORE CHANGING THIS!
#define REMORA_TEXT_BUFFER_ROWS MAX_LINES

// These constants are used by the motion scanner.
#define REMORA_SCAN_PAN_STEP (FLOAT_ONE / 200)
#define REMORA_SCAN_START_ZOOM 200
#define REMORA_SCAN_ZOOM_HARD_UPPER 512
#define REMORA_SCAN_ZOOM_HARD_LOWER 100
#define REMORA_SCAN_ZOOM_STEP 7

// This defines the maximum menu depth in the menu tree modes.
#define REMORA_MENU_DEPTH 5

// This is the name pattern for the variables that control the Remora's menus.
#define REMORA_MENU_LEVEL_NAME "menu_level_*"

// These are text attributes for the text displayed on the Remora's screen.
#define REMORA_TEXT_FLASH ((uint8)0x80)
#define REMORA_TEXT_PICTURE ((uint8)0x40)
#define REMORA_TEXT_HEADING ((uint8)0x01)
#define REMORA_TEXT_WARNING ((uint8)0x02)
#define REMORA_TEXT_OPTION ((uint8)0x04)
#define REMORA_TEXT_PARAGRAPH ((uint8)0x08)

// This is the first line that is drawn (skipping the heading, which doesn't scroll).
#define REMORA_FIRST_SCROLLING_LINE 1

// This controls how int32 the screen flash effect lasts.
#define REMORA_FLASH_EFFECT_LENGTH 6

// This controls the rate at which flashing text flashes.
#define REMORA_TEXT_FLASH_RATE 8

// And this controls how int32 it flashes for.
#define REMORA_TEXT_FLASH_DURATION 4

// These are for the one-off interface in Mission 08.
#define REMORA_M08_NUM_LOCKS 12
#define REMORA_M08_ZOOM_X 120
#define REMORA_M08_ZOOM_Z 100
#define REMORA_M08_X_ORIGIN (-800)
#define REMORA_M08_Z_ORIGIN 2680

// We can probably dispense with these and just draw one blip in different colours.
#define REMORA_FLIC_REMORA_BLIP "blip_green"
#define REMORA_FLIC_ALIVE_ROBOT_BLIP "blip_orange"
#define REMORA_FLIC_DEAD_ROBOT_BLIP "blip_dark_orange"
#define REMORA_FLIC_ALIVE_HUMAN_BLIP "blip_red"
#define REMORA_FLIC_DEAD_HUMAN_BLIP "blip_dark_red"
#define REMORA_FLIC_RECHARGE_BLIP "blip_dark_yellow"

// This is the maximum floor ranges that can be set in any one session (probably only one will ever be set).
#define REMORA_MAX_FLOOR_RANGES 3

// This is the maximum levels that can be included in scanning.
#define REMORA_MAX_INCLUDED_SLICES 4

// Maximum knowledge level in the Remora maps.
#define REMORA_KNOWLEDGE_MAX_LEVEL 3

// This is the maximum length of an email ID string.
#define REMORA_MAXLEN_EMAIL_ID_STRING 11

#define REMORA_SCROLL_CYCLES 4

// Holds information about a single line on the Remora's display.
struct _remora_line {
	union _pos {
		_pin_position s_ePinPosition;
		uint32 s_nXOffset;

	} s_uPos;

	uint8 s_nAttribute;

	union _xy {
		uint8 s_nIndent;
		uint8 s_nPictureRow;

	} s_uXY;

	char s_pcText[REMORA_TEXT_BUFFER_WIDTH];
};

// Holds a floor range.  All floors within this range are displayed together in the scan modes.
struct _slice_range {
	uint8 s_nLower;
	uint8 s_nUpper;
	uint8 s_nPad1;
	uint8 s_nPad2;
};

// Class which provides all the Remora functions for the game.
class _remora {
public:
	// Definitions visible outside the class.
	enum RemoraMode { UNUSED_MAIN_MENU = 0, MOTION_SCAN, INFRA_RED_LINK, DATABASE, COMMUNICATIONS, MAP, M08_LOCK_CONTROL, NO_OVERRIDE = 999 };

	enum MenuVariableAccessMode { GET = 0, SET };

	enum ScreenSymbol {
		DO_NOT_DISPLAY = 0,
		SS_REMORA = 1,
		ALIVE_ROBOT = 2,
		DEAD_ROBOT = 3,
		ALIVE_HUMAN = 4,
		DEAD_HUMAN = 5,
		RECHARGE_ARMED = 6,
		RECHARGE_UNARMED = 7,
		DOOR_OPEN = 8,
		DOOR_CLOSED = 9
	};

	enum TextScroll { SCROLL_NONE = 0, SCROLL_UP, SCROLL_DOWN };

	// Default constructor and destructor.
	_remora();
	inline ~_remora();

	// Call this before using the Remora.
	void InitialiseRemora();

	// Call this to activate the Remora in a given mode.
	void ActivateRemora(RemoraMode eMode);

	// These are used to set a specific activation mode from script.
	void SetModeOverride(RemoraMode eMode) { m_eModeOverride = eMode; }
	void SetDefaultOrOverrideMode();

	// And call this to deactivate it.
	void DeactivateRemora(bool8 bForceShutdown);

	// This is the main Remora call, which should get made every logic cycle (though it won't always do anything).
	void CycleRemoraLogic(const _input &sKeyboardState);

	// This draws the Remora in its current state.
	void DrawRemora();

	// This finds text from a reference in the Remora's compiled text files.
	const char *LocateTextFromReference(uint32 nHashRef);

	// These functions save and restore necessary parts of the Remora.
	void Save(Common::WriteStream *stream) const;
	void Restore(Common::SeekableReadStream *stream);

	// This allows the script variables to be set from the engine that control the Remora's menus.
	void AccessMenuLevelVariables(int32 *pnParams, MenuVariableAccessMode eRetrieve);

	// These draw the text used in the Remora.  These are now shared between the PC and PSX.
	void DrawScreenText();
	void DrawVoiceOverText() const;

	// This draws a flashing message if there is an email waiting.
	void DrawEmailWaiting();

	// This toggles the PC's formatting of text between normal mode and PSX emulation.
	void ToggleTextFormattingPlatform() { m_bFormatForPSX = (m_bFormatForPSX == FALSE8) ? TRUE8 : FALSE8; }
	bool8 FormatForPSX() const { return (m_bFormatForPSX); }
	uint32 GetRemoraSurfaceId() { return (m_nRemoraSurfaceID); }

	// This is public so prim route builder can access it.
	bool8 CohenSutherland(DXrect oRect, int32 &nX1, int32 &nY1, int32 &nX2, int32 &nY2, bool8 bnClip) const;

	// These functions have script counterparts.
	bool8 IsActive() const { return ((m_eGameState == INACTIVE) ? FALSE8 : TRUE8); }
	RemoraMode GetMode() const { return m_eCurrentMode; }
	void SetMode(RemoraMode eMode);
	void SetText(const char *pcText, uint8 nAttribute, uint8 nIndent, _pin_position ePosition);
	void DrawHeadingText();
	void SetupPicture(uint32 nPixelOffset, const char *pcPictureName);
	void ClearAllText();
	void DisplayCharacterSpeech(uint32 nHash);
	inline void SetMaximumZoom(uint32 nZoom);
	inline void SetMinimumZoom(uint32 nZoom);
	void SetCurrentZoom(uint32 nZoom);
	void NewEmail(const char *pcEmailID);
	bool8 IsEmailWaiting() const { return ((strlen(m_pcEmailID) > 0) ? TRUE8 : FALSE8); }
	bool8 IsThisEmailWaiting(const char *pcEmailID) const;
	inline void MarkEmailRead();
	void AddFloorRange(uint32 nLower, uint32 nUpper);
	inline void ResetFloorRanges();
	bool8 EMPEffect();

	// One of the fn_routines() needs access to the speech timer so it knows when to allow script to run on.
	uint32 GetSpeechTimer() const { return (m_nSpeechTimer); }

	int32 GetProgressBarValue() { return m_nProgressBarValue; }
	void SetProgressBarValue(int32 v) { m_nProgressBarValue = v; }

	int32 GetProgressBarTotal() { return m_nProgressBarTotal; }
	void SetProgressBarTotal(int32 t) { m_nProgressBarTotal = t; }
	void DrawProgressBar();

private:
	// Private definitions used internally by this class.
	enum RemoraGameState { INACTIVE = 0, SWITCHING_ON, ACTIVE, SWITCHING_OFF };

	enum Outcode { OUTCODE_LEFT = 0x00000001, OUTCODE_RIGHT = 0x00000002, OUTCODE_BOTTOM = 0x00000004, OUTCODE_TOP = 0x00000008 };

	RemoraGameState m_eGameState; // Current state of Remora in the game (not its internal mode).
	RemoraMode m_eCurrentMode;    // Current mode Remora is in.
	RemoraMode m_eLastMode;       // Last mode the Remora was in.
	RemoraMode m_eModeOverride;   // Used to set a mode from script.

	// General graphics.
	DXrect m_sScreenRectangle;         // The drawable area of the Remora's screen.
	DXrect m_sBackgroundRectangle;     // Actually this is the size of the whole screen.
	DXrect m_sCasingSourceRectangle;   // The rectangle to blit the casing from.
	DXrect m_sCasingTargetRectangle;   // The rectangle to blit the casing to.
	DXrect m_sMoreUpSourceRectangle;   // Source rectangle for the more-up arrow.
	DXrect m_sMoreUpTargetRectangle;   // Target rectangle for the more-up arrow.
	DXrect m_sMoreDownSourceRectangle; // Source rectangle for the more-down arrow.
	DXrect m_sMoreDownTargetRectangle; // Target rectangle for the more-down arrow.

	uint32 m_nCasingSurfaceID;   // Surface that holds the casing.
	uint32 m_nMoreUpSurfaceID;   // Surface for the more-up arrow.
	uint32 m_nMoreDownSurfaceID; // Surface for the more-down arrow.
	uint32 m_nRemoraSurfaceID;   // Surface to draw everything into before blitting it to the working buffer
	char m_pcRemoraCluster[MAXLEN_CLUSTER_URL];
	bool8 m_bFormatForPSX; // Simulate PSX text formatting when true.
	uint32 m_nDisplayedTextRows;
	uint32 m_nCharacterSpacing;
	uint32 m_nRowSpacing;

#if defined(_PSX)
	DXrect m_sCasingTargetRectangle; // The rectangle to blit the casing to.
	char *m_pcRemoraCluster;
#endif // #if defined(_PSX)

	// Data for the text display.
	_remora_line *m_pDisplayBuffer; // Holds info about text and pictures to display.
	TextScroll m_eTextScroll;       // Text scroll direction.

	// These text buffers are for when characters speak within the Remora.
	const char *m_pcSpeechText; // Buffer to hold character-spoken text.
	uint32 m_nSpeechTimer;      // Used to count how int32 speech has been displayed.

	// These data items are for the scanning modes.
	_remora_sprite m_oTextPicture; // Used for drawing a picture in the text.

#if defined(_PSX)
	_remora_sprite m_oBlipSprite;       // Holds the sprite for drawing the blips on the motion tracker.
	_remora_sprite m_oPlayerBlipSprite; // Holds the sprite fot the player's blip.
	_remora_sprite m_oCasing;           // REMORA casing sprite
	int counter;
#else
	uint32 m_nScanPan;        // Current position of scan beam.
	uint32 m_nPulseHighlight; // This keeps track of the highlight in the pulse.
#endif

	PXfloat m_fPlayerPan;                                  // Local player pan so we can rotate the Remora scanner.
	int32 m_nPlayerX, m_nPlayerY, m_nPlayerZ;              // Position of player (may be 'fixed' for some scan modes).
	uint32 m_nCurrentZoom;                                 // 128 = scale of 1.0, 1 = x(1/128), 512 = scale of x4
	uint32 m_nMinZoom;                                     // Minimum zoom level allowed for the Remora.
	uint32 m_nMaxZoom;                                     // Maximum zoom level allowed for the Remora.
	_barrier_slice *m_pSlices[REMORA_MAX_INCLUDED_SLICES]; // Slices of barriers for player's current height.
	uint32 m_pnSlices[REMORA_MAX_INCLUDED_SLICES];         // Index of slice player is on.
	int32 m_nIncludedCeiling;                              // Top coordinate of things to include in scan modes.
	int32 m_nIncludedFloor;                                // Bottom coordinate of things to include in scan modes.
	DXrect m_nPlayerBlipTargetRectangle;                   // Rectangle area to draw the player blip.
	_slice_range m_pFloorRanges[REMORA_MAX_FLOOR_RANGES];  // Holds floor ranges for displaying multiple floor levels.

	int32 m_nProgressBarValue; // 0 means not displayed
	int32 m_nProgressBarTotal; // total of latest progress bar

	int32 m_nStartYPixelOffset; // This is the scroll offset (to smooth scrolling).

	// These are some extra definitions for the M08 lock-control interface.
	uint32 m_pnDoorIDs[REMORA_M08_NUM_LOCKS]; // IDs of the doors in M08.

	// These data items are for the Remora's email system.
	char m_pcEmailID[REMORA_MAXLEN_EMAIL_ID_STRING + 1]; // Have to store actual string for savegames.

	// Counter to remember when objects's are in the sweep beam
	// so bright when in beam, after that the counter goes down
	// so they fade away
	uint8 m_pObjectScale[REMORA_MAX_NUMBER_OBJECTS];

	// Clustering.
	uint32 m_nRemoraClusterHash;

	// These are here at the end to keep the data alligned.
	bool8 m_bModeChanged;       // Set to true when there is a mode change.
	uint8 m_nNextAvailableRow;  // Last line text has been written to.
	uint8 m_nFirstLineToDraw;   // Display from this point (to do scrolling).
	bool8 m_bScrollingRequired; // Set to true when the scroll controls are needed.

	bool8 m_bMainHeadingSet;        // Set to true when a screen heading has been set.
	uint8 m_nScreenFlashCount;      // Counter to work a screen flash effect.
	uint8 m_nNumFloorRangesSet;     // Number of floor ranges currently set.
	uint8 m_nNumCurrentFloorRanges; // Number floor ranges currently included in scan.

	bool8 m_bScrolling;               // Set to true while text is scrolling.
	uint8 m_nCurrentPalette;          // Pointer to the current palette.
	uint8 m_nCharacterHeight;         // Used to estimate picture heights in rows of text.
	uint8 m_nPictureHeightCorrection; // Correction to stop picture sizes being rounded down.

	uint8 m_nFlashCounter;        // Counter for flashing text on and off.
	bool8 m_bFlashingTextVisible; // Flag that goes with previous count.
	uint8 m_bTextPictureLoaded;   // Gets set to true when there is a picture in the text.

	// These are used to explicitally declare the padding in this structure to make the alignment correct.
	// Use these variables when you add more data to the structure, so that the aligment stays correct.
	uint8 m_nPad1;

	// Here I block the use of the default '='.
	_remora(const _remora &) { ; }
	void operator=(const _remora &) { ; }

	// Private functions used only by this class.
	void DoPlatformSpecificInitialisation();
	void SetCommonActivateInfo(RemoraMode eMode);
	void SetUpRemora();
	void CloseDownRemora();

	void RemoraLineDraw(int32 nX1, int32 nZ1, int32 nX2, int32 nZ2, _rgb sColour0, _rgb sColour1, uint32 nHalfThickness = 0) const;

	void InitialiseMenuControlVariables();

	void DrawGrid();

	inline const char *MakeRemoraGraphicsPath(const char *pcBitmapName) const;
	void SetUpSurfaceForBitmap(const char *pcBitmapName, DXrect &sSourceRect, DXrect &sTargetRect, uint32 &nSurfaceID);
	void DrawHeaderAndFooterLines();
	void ClipTopAndBottom();
	uint32 ComputeOutcode(DXrect oRect, int32 nX, int32 nY) const;
	inline bool8 TestOutcode(uint32 nOutcode, Outcode enCodeToTest) const;
#if defined(_PSX)
	void DrawWideScanMask(const int xc, const int yc, const int x1, const int y1, const int x2, const int y2);
	void DrawBlip(int32 x, int32 z, _rgb sColour, uint32 size, uint32 pulse) const;
	void InitRotMatrix(const int32 angle, const int zoom, const int x, const int z, int *dx, int *dz);
#endif

	// These functions are used by the text system.
	uint32 GetColour(uint8 nAttribute) const { return ((uint32)(nAttribute & (uint8)0x0f)); }
	bool8 GetFlashing(uint8 nAttribute) const { return ((bool8)((nAttribute & REMORA_TEXT_FLASH) >> 7)); }
	void ProcessUpDownTextKeys(const _input &sKeyboardState);
	void DrawMoreUpDownArrows();
	void ColourToRGB(uint8 nAttributes, uint8 &nRed, uint8 &nGreen, uint8 &nBlue) const;
	void GrabTextFormattingMemory();
	void ReleaseTextFormattingMemory();

	// These are for the scan mode.
	void SetUpWideScan();
	void DrawWideScan();
	ScreenSymbol GetSymbolToDrawObject(_logic *pObject, uint32 nID) const;
	void DrawEMPEffect();
	void ProcessUpDownZoomKeys(const _input &sKeyboardState);

	void DrawStaticBarriers(_rgb oLineColour) const;
	void DrawAnimatingBarriers(_rgb oLineColour) const;
	void DrawM08DoorLocks();
	void DrawM08LockControlObjects();
	void DrawFloorRectangles() const;

	void DrawObjects(void);
	void AddInterference() const;
	void DrawScanBeam() const;
	void DrawPulse();
	void DrawIRLinkPulse();
	void DrawSquareSymbol(int32 nX, int32 nY, uint32 nPal, uint32 nSize);
	void DrawTriangleSymbol(int32 nX, int32 nY, uint32 nPal, uint32 nSize);
	void DrawOctagonSymbol(int32 nX, int32 nY, uint32 nPal, uint32 nSize);
	void DrawCrosshairs() const;
#if defined(_PSX)
	void DrawObjects(const int32 x1, const int32 y1, const int32 x2, const int32 y2);
#endif

	// These are for the one-off M08 lock control interface.
	void SetUpM08LockControl();
	void DrawM08LockControl();
	void BuildM08DoorList();
};

} // End of namespace ICB (to avoid nesting)

// Here we include platform-specific definitions.  The only reason we can get away with doing it here is because
// none of the platform-specific definitions happen to have been used in the above class definition.
#if defined(_PSX)

#include "remora_psx.h"

#define REMORA_DISPLAYED_TEXT_ROWS REMORA_PSX_DISPLAYED_TEXT_ROWS
#define REMORA_CHARACTER_SPACING REMORA_PSX_CHARACTER_SPACING
#define REMORA_ROW_SPACING REMORA_PSX_ROW_SPACING

#else

#include "remora_pc.h"

#define REMORA_DISPLAYED_TEXT_ROWS m_nDisplayedTextRows
#define REMORA_CHARACTER_SPACING m_nCharacterSpacing
#define REMORA_ROW_SPACING m_nRowSpacing

#endif

namespace ICB {

inline void _remora::InitialiseMenuControlVariables() {
	int32 pnVariables[5] = {0, 0, 0, 0, 0};

	AccessMenuLevelVariables((int32 *)pnVariables, SET);
}

inline void _remora::SetMaximumZoom(uint32 nZoom) {
	// First check against the 'hard' limit.
	if ((nZoom < REMORA_SCAN_ZOOM_HARD_LOWER) || (nZoom > REMORA_SCAN_ZOOM_HARD_UPPER))
		Fatal_error("Zoom %d out of range %d-%d in _remora::SetMaximumZoom()", nZoom, REMORA_SCAN_ZOOM_HARD_LOWER, REMORA_SCAN_ZOOM_HARD_UPPER);

	// Maximum can't be less than minimum.  If it is, we move the minimum rather than generate an error
	// otherwise the ordering of the calls to set min and max in script will be significant.
	m_nMinZoom = (nZoom < m_nMinZoom) ? nZoom : m_nMinZoom;

	// Now we can set the new value.
	m_nMaxZoom = nZoom;
}

inline void _remora::SetMinimumZoom(uint32 nZoom) {
	// First check against the 'hard' limit.
	if ((nZoom < REMORA_SCAN_ZOOM_HARD_LOWER) || (nZoom > REMORA_SCAN_ZOOM_HARD_UPPER))
		Fatal_error("Zoom %d out of range %d-%d in _remora::SetMinimumZoom()", nZoom, REMORA_SCAN_ZOOM_HARD_LOWER, REMORA_SCAN_ZOOM_HARD_UPPER);

	// Minimum can't be greater than maximum.  If it is, we move the maximum rather than generate an error
	// otherwise the ordering of the calls to set min and max in script will be significant.
	m_nMaxZoom = (nZoom > m_nMaxZoom) ? nZoom : m_nMaxZoom;

	// Now we can set the new value.
	m_nMinZoom = nZoom;
}

inline void _remora::NewEmail(const char *pcEmailID) { strcpy(m_pcEmailID, pcEmailID); }

inline void _remora::ResetFloorRanges() {
	m_nNumFloorRangesSet = 0;
	m_nNumCurrentFloorRanges = 0;
}

inline void _remora::MarkEmailRead() {
	m_pcEmailID[0] = '\0';
	g_oIconMenu->ClearEmailArrived();
}

extern _remora *g_oRemora;

} // End of namespace ICB

#endif // #if !defined( REMORA_H_INCLUDED )
