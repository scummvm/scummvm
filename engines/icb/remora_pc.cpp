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

#include "engines/icb/p4.h"
#include "engines/icb/remora.h"
#include "engines/icb/surface_manager.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/sound.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/res_man.h"
#include "engines/icb/floors.h"
#include "engines/icb/mission.h"
#include "engines/icb/icb.h"

#include "common/util.h"

namespace ICB {

// This array defines alternate colour schemes for the Remora.
enum ColourIndex {
	CI_HEADING = 0,       // Colour of heading text.
	CI_WARNING,           // Colour of warning text.
	CI_OPTION,            // Colour of option text.
	CI_PARAGRAPH,         // Colour of paragraph text.
	CI_BACKGROUND,        // Colour of the backdrop to the scanner and behind the text.
	CI_GRID,              // Colour of grid
	CI_CLIP,              // Colour of the clip area above and below the text.
	CI_FLOORS,            // Colour of floor rectangles in the scanner.
	CI_BARRIERS,          // Colour of barriers in the scanner.
	CI_BEAM_LEADING,      // Colour of the leading edge of the beam in the scanner.
	CI_BEAM_TRAILING,     // Colour of the trailing edge of the beam in the scanner.
	CI_ROBOT,             // Colour of alive robots in the scanner.
	CI_DEAD_ROBOT,        // Colour of dead robots in the scanner.
	CI_HUMAN,             // Colour of alive humans in the scanner.
	CI_DEAD_HUMAN,        // Colour of dead humans in the scanner.
	CI_RECHARGE,          // Colour of the recharge symbols in the scanner.
	CI_RECHARGE_ARMED,    // Colour of the armed recharge symbols in the scanner.
	CI_PLAYER,            // Colour of the player symbol in the scanner.
	CI_DOOR_OPEN,         // Colour of an open door.
	CI_DOOR_CLOSED,       // Colour of a closed door.
	CI_PROGRESS_HEAD,     // Colour of progress bar's leading edge.
	CI_PROGRESS_TAIL,     // Colour of progress bar's trailing edge.
	CI_M08_LOCKED_DOOR,   // Colour of the armed recharge symbols in the scanner.
	CI_M08_UNLOCKED_DOOR, // Colour of the armed recharge symbols in the scanner.
	CI_M08_BARRIERS       // Colour of barriers in M08 lock control interface.
};                            // Allows the cardinality of the enum to be got.

// This macro shortens the syntax for accessing the paletts.
#define REMPAL(c, v) (pnRemoraColour[m_nCurrentPalette][c][v])

const uint8 pnRemoraColour[REMORA_NUM_COLOUR_SCHEMES][REMORA_COLOURS_PER_SCHEME][3] = {
    // Pallete 0.
    {{0, 180, 241},   // CI_HEADING
     {255, 255, 255}, // CI_WARNING
     {0, 221, 160},   // CI_OPTION
     {0, 140, 200},   // CI_PARAGRAPH
     {0, 3, 35},      // CI_BACKGROUND
     {30, 60, 120},   // CI_GRID
     {0, 3, 35},      // CI_CLIP
     {2, 55, 128},    // CI_FLOORS
     {50, 80, 255},   // CI_BARRIERS
     {0, 20, 180},    // CI_BEAM_LEADING
     {0, 0, 0},       // CI_BEAM_TRAILING
     {225, 34, 5},    // CI_ROBOT
     {160, 10, 0},    // CI_DEAD_ROBOT
     {2, 24, 200},    // CI_HUMAN
     {1, 10, 140},    // CI_DEAD_HUMAN
     {38, 255, 0},    // CI_RECHARGE
     {255, 20, 20},   // CI_RECHARGE_ARMED
     {255, 255, 255}, // CI_PLAYER
     {38, 255, 0},    // CI_DOOR_OPEN
     {255, 125, 0},   // CI_DOOR_CLOSED
     {255, 255, 255}, // CI_PROGRESS_HEAD
     {15, 50, 200},   // CI_PROGRESS_TAIL
     {255, 30, 30},   // CI_M08_LOCKED_DOOR
     {7, 141, 23},    // CI_M08_UNLOCKED_DOOR
     {0, 90, 220}},   // CI_M08_BARRIERS

    // Pallete 1.
    {{0, 241, 180},   // CI_HEADING
     {255, 255, 255}, // CI_WARNING
     {0, 160, 221},   // CI_OPTION
     {0, 200, 140},   // CI_PARAGRAPH
     {0, 35, 10},     // CI_BACKGROUND
     {0, 220, 80},    // CI_GRID
     {0, 35, 10},     // CI_CLIP
     {0, 75, 15},     // CI_FLOORS
     {0, 220, 80},    // CI_BARRIERS
     {40, 255, 100},  // CI_BEAM_LEADING
     {0, 20, 0},      // CI_BEAM_TRAILING
     {255, 34, 5},    // CI_ROBOT
     {170, 10, 0},    // CI_DEAD_ROBOT
     {5, 225, 34},    // CI_HUMAN
     {5, 160, 10},    // CI_DEAD_HUMAN
     {238, 156, 0},   // CI_RECHARGE
     {255, 20, 20},   // CI_RECHARGE_ARMED
     {255, 255, 255}, // CI_PLAYER
     {238, 156, 0},   // CI_DOOR_OPEN
     {238, 4, 0},     // CI_DOOR_CLOSED
     {255, 255, 255}, // CI_PROGRESS_HEAD
     {5, 200, 50},    // CI_PROGRESS_TAIL
     {255, 30, 30},   // CI_M08_LOCKED_DOOR
     {7, 235, 23},    // CI_M08_UNLOCKED_DOOR
     {0, 230, 90}},   // CI_M08_BARRIERS

    // Pallete 2.
    {{247, 236, 23},  // CI_HEADING
     {252, 21, 10},   // CI_WARNING
     {235, 165, 33},  // CI_OPTION
     {233, 238, 30},  // CI_PARAGRAPH
     {85, 45, 0},     // CI_BACKGROUND
     {216, 151, 1},   // CI_GRID
     {85, 45, 0},     // CI_CLIP
     {133, 84, 1},    // CI_FLOORS
     {216, 151, 1},   // CI_BARRIERS
     {224, 200, 123}, // CI_BEAM_LEADING
     {35, 20, 0},     // CI_BEAM_TRAILING
     {250, 140, 15},  // CI_ROBOT
     {176, 94, 2},    // CI_DEAD_ROBOT
     {24, 175, 3},    // CI_HUMAN
     {15, 104, 2},    // CI_DEAD_HUMAN
     {247, 236, 23},  // CI_RECHARGE
     {255, 20, 20},   // CI_RECHARGE_ARMED
     {255, 255, 255}, // CI_PLAYER
     {197, 47, 20},   // CI_DOOR_OPEN
     {235, 47, 20},   // CI_DOOR_CLOSED
     {255, 255, 255}, // CI_PROGRESS_HEAD
     {216, 137, 8},   // CI_PROGRESS_TAIL
     {255, 30, 30},   // CI_M08_LOCKED_DOOR
     {247, 236, 23},  // CI_M08_UNLOCKED_DOOR
     {247, 236, 23}}  // CI_M08_BARRIERS
};

// struct Point2DColoured
// This is used in the Gouraud drawing routines.
typedef struct {
	int32 x, y;
	int32 u, v;
	uint8 r, g, b;

} Point2DColoured;

// struct Span
// This is used in the Gouraud drawing routines.
typedef struct {
	int32 x0, x1;
	uint8 r0, g0, b0;
	uint8 r1, g1, b1;

} Span;
Span remora_spans[REMORA_MAX_YS];

void DrawGouraudQuad(uint32 nX0, uint32 nY0, uint32 nX1, uint32 nY1, uint32 nX2, uint32 nY2, uint32 nX3, uint32 nY3, uint8 nRed0, uint8 nGreen0, uint8 nBlue0, uint8 nRed1,
                     uint8 nGreen1, uint8 nBlue1, uint8 nRed2, uint8 nGreen2, uint8 nBlue2, uint8 nRed3, uint8 nGreen3, uint8 nBlue3, int32 nOpacity);

void DrawGouraudTriangle(uint32 x0, uint32 y0, uint32 x1, uint32 y1, uint32 x2, uint32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
                         int32 nOpacity);

// These values are used for mapping game-world points to the Remora's screen.
float fRotateCos, fRotateSin;     // Used to store cosine and sine for rotating, so we don't have to keep working it out.
float fXDrawScale, fZDrawScale;   // Scaling factors for the display.
float fXDrawOrigin, fZDrawOrigin; // The origin for displaying points (in the game world).

inline void InitialiseGameToRemora(float fPan, uint32 nCurrentXScale, uint32 nCurrentZScale, int32 nCurrentXOrigin, int32 nCurrentZOrigin) {
	// These terms are needed to take account of the player's direction of looking.
	fRotateCos = (float)cos(fPan * TWO_PI + M_PI);
	fRotateSin = (float)sin(fPan * TWO_PI + M_PI);

	// The scale is a number between 1 and 256 (probably clamped to a smaller range).  The formulat for
	// scaling this to the Remora's screen is P = (W * (scale/512) ) / 4, where P is a point on the Remora
	// screen and W is a point in the game world.
	fXDrawScale = (float)nCurrentXScale / 2048.0f;
	fZDrawScale = (float)nCurrentZScale / 2048.0f;
	fXDrawOrigin = (float)nCurrentXOrigin;
	fZDrawOrigin = (float)nCurrentZOrigin;
}

inline void GameToRemora(float &fX, float &fZ) {
	float fTempX, fTempZ;

	// First calculate the point relative to the origin for the display.
	fTempX = fX - fXDrawOrigin;
	fTempZ = fZ - fZDrawOrigin;

	// Now rotate to take account of the viewing direction.
	fX = fTempX * fRotateCos + fTempZ * -fRotateSin;
	fZ = fTempX * fRotateSin + fTempZ * fRotateCos;

	// Now scale the point to fit on the screen.
	fX *= fXDrawScale;
	fZ *= fZDrawScale;

	// Now work out the position of the point relative to the centre of the drawn screen.
	fX += REMORA_SCREEN_CENTRE_X;
	fZ += REMORA_SCREEN_CENTRE_Z;
}

void _remora::ActivateRemora(RemoraMode eMode) {
	if (g_theMusicManager)
		g_theMusicManager->StopMusic();

	// Play the Remora-activate sound.
	RegisterSoundSpecial(activateRemoraSfx, activateRemoraDesc, 127, 0);

	// Make a rectangle for the drawable screen area (used to come from a sprite).
	m_sScreenRectangle = MakeRECTFromSpriteSizes(REMORA_DRAWABLE_SCREEN_LEFT, REMORA_DRAWABLE_SCREEN_TOP, REMORA_DRAWABLE_SCREEN_WIDTH, REMORA_DRAWABLE_SCREEN_HEIGHT);

	// Call the common PC/PSX set-up code.  (Note that this code relies on m_sScreenRectangle being set.)
	SetCommonActivateInfo(eMode);

	// Grab the memory for text formatting.
	GrabTextFormattingMemory();

	// Only the PC has this pulse counter.
	m_nPulseHighlight = 0;

	// Here we set some text formatting parameters depending on whether the PC is simulating PSX
	// text formatting.
	if (m_bFormatForPSX) {
		m_nDisplayedTextRows = REMORA_PSXFORMAT_DISPLAYED_ROWS;
		m_nCharacterSpacing = REMORA_PSXFORMAT_CHAR_SPACING;
		m_nRowSpacing = REMORA_PSXFORMAT_ROW_SPACING;
	} else {
		m_nDisplayedTextRows = REMORA_PC_DISPLAYED_TEXT_ROWS;
		m_nCharacterSpacing = 0;
		m_nRowSpacing = 0;
	}

	Zdebug("Activated Remora (mode = %d)", eMode);
}

void _remora::DoPlatformSpecificInitialisation() {

	uint32 nFileNameHash, nClusterHash;
	char pcFileName[ENGINE_STRING_LEN];

	// Load the Map Data.  When clustered the session files have the base stripped.
	strcpy(pcFileName, REMORA_MAP_FILENAME);

	nClusterHash = MS->Fetch_session_cluster_hash();
	nFileNameHash = NULL_HASH;

	// Allocate a text buffer if one hasn't been allocated already.
	if (!m_pDisplayBuffer)
		m_pDisplayBuffer = new _remora_line[REMORA_TEXT_BUFFER_ROWS];
}

void _remora::GrabTextFormattingMemory() {
	// Allocate a text buffer if one hasn't been allocated already.
	if (!m_pDisplayBuffer)
		m_pDisplayBuffer = new _remora_line[REMORA_TEXT_BUFFER_ROWS];
}

void _remora::ReleaseTextFormattingMemory() {
	if (m_pDisplayBuffer) {
		delete[] m_pDisplayBuffer;
		m_pDisplayBuffer = NULL;
	}
}

void _remora::SetUpRemora() {
	Zdebug("Entered _remora::SetUpRemora()");

	PXTRY
	// Create a surface for the remora to draw into
	m_nRemoraSurfaceID = surface_manager->Create_new_surface("RemoraBB", SCREEN_WIDTH, SCREEN_DEPTH, SYSTEM);

	// Can make the background size now.
	m_sBackgroundRectangle = MakeRECTFromSpriteSizes(0, 0, REMORA_SCREEN_WIDTH, REMORA_SCREEN_HEIGHT);

	// Create a surface for the whole screen and clear it.
	uint32 dwFillPixel = GetPen(m_nCurrentPalette, CI_BACKGROUND);
	surface_manager->Blit_fillfx(m_nRemoraSurfaceID, &m_sBackgroundRectangle, dwFillPixel);

	// Prepare the Remora casing.
	SetUpSurfaceForBitmap(REMORA_BITMAP_REMORA, m_sCasingSourceRectangle, m_sCasingTargetRectangle, m_nCasingSurfaceID);

	// Draw the the casing.
	surface_manager->Blit_surface_to_surface(m_nCasingSurfaceID, m_nRemoraSurfaceID, &m_sCasingSourceRectangle, &m_sCasingTargetRectangle);

	// Set up the sprites that flash when text goes off the top or botttom of the screen.
	SetUpSurfaceForBitmap(REMORA_BITMAP_MORE_UP, m_sMoreUpSourceRectangle, m_sMoreUpTargetRectangle, m_nMoreUpSurfaceID);
	SetUpSurfaceForBitmap(REMORA_BITMAP_MORE_DOWN, m_sMoreDownSourceRectangle, m_sMoreDownTargetRectangle, m_nMoreDownSurfaceID);

	PXCATCH

	Tdebug(EXCEPTION_LOG, "Exception in _remora::SetUpRemora()");
	Fatal_error("Exception in _remora::SetUpRemora()");

	PXENDCATCH

	Zdebug("Leaving _remora::SetUpRemora()...");
}

void _remora::CloseDownRemora() {
	Zdebug("Closing down Remora ...");

	// Dump the surfaces I've created.
	Zdebug("Killing casing surface - ID=%d", m_nCasingSurfaceID);
	surface_manager->Kill_surface(m_nCasingSurfaceID);

	Zdebug("Killing casing surface - ID=%d", m_nMoreUpSurfaceID);
	surface_manager->Kill_surface(m_nMoreUpSurfaceID);

	Zdebug("Killing casing surface - ID=%d", m_nMoreDownSurfaceID);
	surface_manager->Kill_surface(m_nMoreDownSurfaceID);

	Zdebug("Killing Remora Back Buffer surface - ID=%d", m_nRemoraSurfaceID);
	surface_manager->Kill_surface(m_nRemoraSurfaceID);

	Zdebug("Remora closed down");
}

void _remora::DrawRemora() {
	LRECT sBlankingRect;
	RemoraMode eModeToDraw;
	uint8 nRed, nGreen, nBlue;

	Zdebug("_remora::DrawRemora()");

	PXTRY
	uint32 oldTextSurface = MS->text_bloc->GetSurface();
	MS->text_bloc->SetSurface(m_nRemoraSurfaceID);

	// Start by wiping the drawing area.
	sBlankingRect.left = REMORA_SCREEN_ORIGIN_X;
	sBlankingRect.right = REMORA_SCREEN_ORIGIN_X + REMORA_SCREEN_WIDTH;
	sBlankingRect.top = REMORA_SCREEN_ORIGIN_Y;
	sBlankingRect.bottom = REMORA_SCREEN_ORIGIN_Y + REMORA_SCREEN_HEIGHT;

	uint32 dwFillPixel = GetPen(m_nCurrentPalette, CI_BACKGROUND);

	surface_manager->Blit_fillfx(m_nRemoraSurfaceID, &sBlankingRect, dwFillPixel);

	// If the mode changed flag is set, it means a new mode has been set but the logic has
	// not yet responded to it.  This means the old graphics won't have been closed down and
	// the new ones won't have been initialised.  In this case, we just continue to draw the
	// old mode until the logic catches up.
	if (m_bModeChanged) {
		eModeToDraw = m_eLastMode;
		m_nPulseHighlight = 0;
	} else {
		eModeToDraw = m_eCurrentMode;
	}

	// See what mode we currently have to draw.
	switch (eModeToDraw) {
	case MOTION_SCAN:
		DrawGrid();
		DrawWideScan();
		DrawHeadingText();
		DrawHeaderAndFooterLines();
		DrawPulse();
		SetTextColour(voice_over_red, voice_over_green, voice_over_blue);
		DrawVoiceOverText();
		DrawEmailWaiting();
		break;

	case INFRA_RED_LINK:
		DrawGrid();
		DrawScreenText();
		ClipTopAndBottom();
		DrawHeadingText();
		DrawMoreUpDownArrows();
		DrawHeaderAndFooterLines();
		DrawIRLinkPulse();
		DrawVoiceOverText();
		DrawEmailWaiting();
		DrawProgressBar();
		break;

	case DATABASE:
		DrawGrid();
		DrawScreenText();
		ClipTopAndBottom();
		DrawHeadingText();
		DrawMoreUpDownArrows();
		DrawHeaderAndFooterLines();
		DrawPulse();
		DrawVoiceOverText();
		DrawEmailWaiting();
		DrawProgressBar();
		break;

	case COMMUNICATIONS:
		DrawGrid();
		DrawScreenText();
		ClipTopAndBottom();
		DrawHeadingText();
		DrawMoreUpDownArrows();
		DrawHeaderAndFooterLines();
		DrawIRLinkPulse();
		DrawVoiceOverText();
		DrawEmailWaiting();
		DrawProgressBar();
		break;

	case M08_LOCK_CONTROL:
		DrawGrid();
		DrawM08LockControl();
		DrawHeadingText();
		DrawPulse();
		DrawHeaderAndFooterLines();
		DrawVoiceOverText();
		DrawEmailWaiting();
		break;

	default:
		// This should never happen.
		Fatal_error("Invalid Remora mode %d in _remora::DrawRemora()", eModeToDraw);

	} // end switch

	// Put the shadow on the edge of the screen.
	nRed = pnRemoraColour[m_nCurrentPalette][CI_BACKGROUND][CI_RED];
	nGreen = pnRemoraColour[m_nCurrentPalette][CI_BACKGROUND][CI_GREEN];
	nBlue = pnRemoraColour[m_nCurrentPalette][CI_BACKGROUND][CI_BLUE];

	DrawGouraudQuad(75, 60, 110, 60, 75, REMORA_SCREEN_CENTRE_Z, 95, REMORA_SCREEN_CENTRE_Z, 0, 0, 0, 0, 0, 0, 0, 0, 0, nRed, nGreen, nBlue, 115);

	DrawGouraudQuad(75, REMORA_SCREEN_CENTRE_Z + 1, 95, REMORA_SCREEN_CENTRE_Z + 1, 75, REMORA_SCREEN_HEIGHT - 70, 110, REMORA_SCREEN_HEIGHT - 70, 0, 0, 0, nRed, nGreen, nBlue,
	                0, 0, 0, nRed, nGreen, nBlue, 115);

	DrawGouraudQuad(110, 60, REMORA_SCREEN_WIDTH - 60, 60, 107, 75, REMORA_SCREEN_WIDTH - 60, 75, 0, 0, 0, 0, 0, 0, nRed, nGreen, nBlue, nRed, nGreen, nBlue, 115);

	// Here we draw the case on again, in case anything has gone over the screen edge.
	surface_manager->Blit_surface_to_surface(m_nCasingSurfaceID, m_nRemoraSurfaceID, &m_sCasingSourceRectangle, &m_sCasingTargetRectangle, DDBLT_KEYSRC);

	// Ok, now blit the remora back buffer across.
	surface_manager->Blit_surface_to_surface(m_nRemoraSurfaceID, working_buffer_id, &full_rect, &full_rect, 0);

	Zdebug("Leaving _remora::DrawRemora()");

	MS->text_bloc->SetSurface(oldTextSurface);

	PXCATCH

	Fatal_error("Exception in _remora::DrawRemora()");

	PXENDCATCH
}

void _remora::SetUpWideScan() {}

void _remora::DrawWideScan() {
	_rgb oLineColour;

	// Set a line colour.
	oLineColour.red = REMPAL(CI_BARRIERS, CI_RED);
	oLineColour.green = REMPAL(CI_BARRIERS, CI_GREEN);
	oLineColour.blue = REMPAL(CI_BARRIERS, CI_BLUE);

	// Set up the function to map game points to the Remora's screen.
	InitialiseGameToRemora(m_fPlayerPan, m_nCurrentZoom, m_nCurrentZoom, m_nPlayerX, m_nPlayerZ);

	// Draw the non-animating barriers.
	DrawStaticBarriers(oLineColour);

	// Set a line colour.
	oLineColour.red = REMPAL(CI_DOOR_OPEN, CI_RED);
	oLineColour.green = REMPAL(CI_DOOR_OPEN, CI_GREEN);
	oLineColour.blue = REMPAL(CI_DOOR_OPEN, CI_BLUE);

	// Draw the animating barriers.  At some point, we are going to have to provide some visual
	// indication of whether or not they are open or closed.
	DrawAnimatingBarriers(oLineColour);

	// Draw the floor rectangles.
	DrawFloorRectangles();

	// Draw other object symbols.
	DrawObjects();

	// Draw on the scanning beam and add interference.
	DrawScanBeam();

	// Put the crosshairs on.
	DrawCrosshairs();

	// Add the flash effect if it is active.
	if (m_nScreenFlashCount > 0)
		DrawEMPEffect();
}

void _remora::DrawFloorRectangles() const {
	uint32 i;
	uint32 nNumFloors;
	_floor *pFloor;
	_rect sRect;
	int32 nX0, nZ0, nX1, nZ1, nX2, nZ2, nX3, nZ3;
	float fX0, fZ0, fX1, fZ1, fX2, fZ2, fX3, fZ3;
	uint8 nRed, nGreen, nBlue;

	PXTRY

	// Get the colour to draw them.
	nRed = pnRemoraColour[m_nCurrentPalette][CI_FLOORS][CI_RED];
	nGreen = pnRemoraColour[m_nCurrentPalette][CI_FLOORS][CI_GREEN];
	nBlue = pnRemoraColour[m_nCurrentPalette][CI_FLOORS][CI_BLUE];

	// Draw floor rectangles.
	nNumFloors = MS->floor_def->Fetch_number_of_floors();

	for (i = 0; i < nNumFloors; ++i) {
		// Get the floor.
		pFloor = MS->floor_def->Fetch_floor_number(i);

		// See if it is in the height range we are drawing.
		if (((PXfloat)pFloor->base_height >= m_nIncludedFloor) && ((PXfloat)pFloor->base_height <= m_nIncludedCeiling)) {
			// Get the actual rectangle.
			sRect = pFloor->rect;

			// Map the points onto the Remora's screen.
			fX0 = (float)sRect.x1;
			fZ0 = (float)sRect.z1;

			fX1 = (float)sRect.x2;
			fZ1 = (float)sRect.z1;

			fX2 = (float)sRect.x1;
			fZ2 = (float)sRect.z2;

			fX3 = (float)sRect.x2;
			fZ3 = (float)sRect.z2;

			GameToRemora(fX0, fZ0);
			GameToRemora(fX1, fZ1);
			GameToRemora(fX2, fZ2);
			GameToRemora(fX3, fZ3);

			nX0 = (int32)fX0;
			nZ0 = (int32)fZ0;

			nX1 = (int32)fX1;
			nZ1 = (int32)fZ1;

			nX2 = (int32)fX2;
			nZ2 = (int32)fZ2;

			nX3 = (int32)fX3;
			nZ3 = (int32)fZ3;

			DrawGouraudQuad(nX0, nZ0, nX1, nZ1, nX2, nZ2, nX3, nZ3, nRed, nGreen, nBlue, nRed, nGreen, nBlue, nRed, nGreen, nBlue, nRed, nGreen, nBlue,
			                REMORA_BRIGHT_MIN);
		}
	}

	PXCATCH

	Fatal_error("Exception in _remora::DrawFloorRectangles()");

	PXENDCATCH
}

void _remora::DrawStaticBarriers(_rgb oLineColour) const {
	uint32 i, j, k;
	float fX1, fZ1, fX2, fZ2;
	int32 nX1, nZ1, nX2, nZ2;
	_barrier_cube *pBarrierCube;
	_barrier_slice *pSlice;
	uint32 *pBarrierArray;
	uint32 nBarrierCubeOffset;
	uint32 nBarrierIndex;
	_route_barrier *pBarrier;

	PXTRY

	// Loop for all the slices we have to draw (mostly will be only one).
	for (i = 0; i < m_nNumCurrentFloorRanges; ++i) {
		// Get the pointer to the slice.
		pSlice = m_pSlices[i];

		// Loop for all the cubes on the slice we are in.
		for (j = 0; j < pSlice->num_cubes; ++j) {
			// Get to the barriers for this cube.
			nBarrierCubeOffset = pSlice->offset_cubes[j];
			pBarrierCube = (_barrier_cube *)((unsigned char *)pSlice + nBarrierCubeOffset);
			pBarrierArray = (uint32 *)((unsigned char *)pSlice + pBarrierCube->barriers);

			// Draw the barriers for this cube.
			for (k = 0; k < (uint32)pBarrierCube->num_barriers; ++k) {
				// Work out the barrier index.
				nBarrierIndex = pBarrierArray[k];

				// Get the actual barrier.
				pBarrier = MS->session_barriers->Fetch_barrier(nBarrierIndex);

				// Set up the vector, the player relative bit has been done in the translation_matrix setting up
				fX1 = pBarrier->x1();
				fZ1 = pBarrier->z1();
				fX2 = pBarrier->x2();
				fZ2 = pBarrier->z2();

				GameToRemora(fX1, fZ1);
				GameToRemora(fX2, fZ2);

				nX1 = (int32)fX1;
				nZ1 = (int32)fZ1;
				nX2 = (int32)fX2;
				nZ2 = (int32)fZ2;

				if (CohenSutherland(m_sScreenRectangle, nX1, nZ1, nX2, nZ2, TRUE8))
					RemoraLineDraw(nX1, nZ1, nX2, nZ2, oLineColour, oLineColour, REMORA_LINE_FUZZ);
			}
		}
	}

	PXCATCH

	Fatal_error("Exception in _remora::DrawStaticBarriers()");

	PXENDCATCH
}

void _remora::DrawAnimatingBarriers(_rgb oLineColour) const {
	uint32 i, j, k, m;
	float fX1, fZ1, fX2, fZ2;
	int32 nX1, nZ1, nX2, nZ2;
	uint32 nBarrierIndex;
	_route_barrier *pBarrier;
	uint32 nPropID;
	uint32 nPropState;
	uint32 nBarriersPerState;
	uint16 *pnBarriers;
	uint32 nSliceIndex;

	PXTRY

	// Loop for each slice we have to draw.
	for (i = 0; i < m_nNumCurrentFloorRanges; ++i) {
		// Get the slice index.
		nSliceIndex = m_pnSlices[i];

		// Loop for all the parent boxes on the slice we are in.
		for (j = 0; j < MAX_parents_per_anim_slice; ++j) {
			// Check if the parent has any animating barriers.
			if (MS->session_barriers->anim_slices[nSliceIndex].anim_parents[j]) {
				// Yes it does.
				for (k = 0; k < MS->session_barriers->anim_slices[nSliceIndex].anim_parents[j]->num_props; ++k) {
					// Get the prop's ID.
					nPropID = MS->session_barriers->anim_slices[nSliceIndex].anim_parents[j]->prop_number[k];

					// Get its state.
					nPropState = MS->prop_state_table[nPropID];

					// Get number of barriers per state.
					nBarriersPerState = MS->session_barriers->anim_prop_info[nPropID].barriers_per_state;

					// Get index into barrier list to first barrier for this state.
					nBarrierIndex = nPropState * nBarriersPerState;

					// Position a pointer at the start of the barriers for the prop in this state.
					pnBarriers = (uint16 *)(&MS->session_barriers->anim_prop_info[nPropID].barrier_list[0]);
					pnBarriers += nBarrierIndex;

					for (m = 0; m < nBarriersPerState; m++) {
						// Get the barrier.
						pBarrier = MS->session_barriers->Fetch_barrier(*(pnBarriers++));

						// Set up the vector, the player relative bit has been done in the translation_matrix setting up
						fX1 = pBarrier->x1();
						fZ1 = pBarrier->z1();
						fX2 = pBarrier->x2();
						fZ2 = pBarrier->z2();

						GameToRemora(fX1, fZ1);
						GameToRemora(fX2, fZ2);

						nX1 = (int32)fX1;
						nZ1 = (int32)fZ1;
						nX2 = (int32)fX2;
						nZ2 = (int32)fZ2;

						if (CohenSutherland(m_sScreenRectangle, nX1, nZ1, nX2, nZ2, TRUE8))
							RemoraLineDraw(nX1, nZ1, nX2, nZ2, oLineColour, oLineColour, REMORA_LINE_FUZZ);
					}
				}
			}
		}
	}

	PXCATCH

	Fatal_error("Exception in _remora::DrawAnimatingBarriers()");

	PXENDCATCH
}

void _remora::DrawObjects() {
	uint32 i;
	float fX, fZ;
	int32 nX, nY, nZ;
	_logic *pGameObject;
	ScreenSymbol eSymbol;
	static bool8 bArmedRechargeFlash = FALSE8;
	char pcSymbolString[2] = {'&', '\0'};

	PXTRY

	// This is the loop that draws the blips for megas.
	for (i = 0; i < MS->total_objects; ++i) {
		// Ignore held objects.
		if (MS->logic_structs[i]->ob_status != OB_STATUS_HELD) {
			// Get the game object.
			pGameObject = MS->logic_structs[i];

			// Get the x,y,z of the object.
			if (pGameObject->image_type == VOXEL) {
				fX = pGameObject->mega->actor_xyz.x;
				fZ = pGameObject->mega->actor_xyz.z;
				nY = (int32)pGameObject->mega->actor_xyz.y;
			} else {
				fX = pGameObject->prop_xyz.x;
				fZ = pGameObject->prop_xyz.z;
				nY = (int32)pGameObject->prop_xyz.y;
			}

			// Ignore objects which are not in the height range we are drawing.
			if ((nY >= m_nIncludedFloor) && (nY <= m_nIncludedCeiling)) {
				// Map the point to the Remora's screen.
				GameToRemora(fX, fZ);

				// We only need integers now.
				nX = (int32)fX;
				nZ = (int32)fZ;

				// Here we get an ID back of which symbol to draw for the object
				eSymbol = GetSymbolToDrawObject(pGameObject, i);

				// Make a string to display.
				switch (eSymbol) {
				case SS_REMORA:
					DrawTriangleSymbol(nX, nZ, CI_PLAYER, REMORA_SYMBOL_SIZE_BIG);
					break;

				case ALIVE_ROBOT:
					DrawSquareSymbol(nX, nZ, CI_ROBOT, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case DEAD_ROBOT:
					DrawSquareSymbol(nX, nZ, CI_DEAD_ROBOT, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case ALIVE_HUMAN:
					DrawOctagonSymbol(nX, nZ, CI_HUMAN, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case DEAD_HUMAN:
					DrawOctagonSymbol(nX, nZ, CI_DEAD_HUMAN, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case RECHARGE_UNARMED:
					DrawSquareSymbol(nX, nZ, CI_RECHARGE, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case RECHARGE_ARMED:
					if (bArmedRechargeFlash) {
						DrawSquareSymbol(nX, nZ, CI_RECHARGE, REMORA_SYMBOL_SIZE_NORMAL);
						bArmedRechargeFlash = FALSE8;
					} else {
						DrawSquareSymbol(nX, nZ, CI_RECHARGE_ARMED, REMORA_SYMBOL_SIZE_NORMAL);
						bArmedRechargeFlash = TRUE8;
					}
					break;

				case DOOR_OPEN:
					pcSymbolString[0] = 'O';
					SetTextColour(REMPAL(CI_DOOR_OPEN, CI_RED), REMPAL(CI_DOOR_OPEN, CI_GREEN), REMPAL(CI_DOOR_OPEN, CI_BLUE));

					MS->Create_remora_text(nX, nZ, pcSymbolString, 0, PIN_AT_CENTRE, 0, 0, REMORA_DISPLAY_WIDTH);
					MS->Render_speech(MS->text_bloc);
					MS->Kill_remora_text();
					break;

				case DOOR_CLOSED:
					pcSymbolString[0] = 'X';
					SetTextColour(REMPAL(CI_DOOR_CLOSED, CI_RED), REMPAL(CI_DOOR_CLOSED, CI_GREEN), REMPAL(CI_DOOR_CLOSED, CI_BLUE));
					MS->Create_remora_text(nX, nZ, pcSymbolString, 0, PIN_AT_CENTRE, 0, 0, REMORA_DISPLAY_WIDTH);
					MS->Render_speech(MS->text_bloc);
					MS->Kill_remora_text();
					break;
				default:
					break;
				}
			} // end if
		} // end if
	} // end for

	PXCATCH

	Fatal_error("Exception in _remora::DrawObjects()");

	PXENDCATCH
}

void _remora::DrawScanBeam() const {
	int32 nLeadingX, nLeadingY;
	int32 nTrailingX, nTrailingY;
	int32 nOrigin2X, nOrigin2Y;
	int32 nLeadingAngle;
	uint8 nLeadingR, nLeadingG, nLeadingB;
	uint8 nTrailingR, nTrailingG, nTrailingB;

	// Work out the angular position of the endpoint of the leading edge.
	nLeadingAngle = (m_nScanPan + REMORA_SCAN_WIDTH) % 360;

	// Now work out the actual drawing coordinates for the beam ends.
	nLeadingX = (int32)(cos(((float)nLeadingAngle / 360.0f) * TWO_PI) * REMORA_SCAN_LENGTH);
	nLeadingY = (int32)(sin(((float)nLeadingAngle / 360.0f) * TWO_PI) * REMORA_SCAN_LENGTH);

	nLeadingX += REMORA_SCREEN_CENTRE_X;
	nLeadingY += REMORA_SCREEN_CENTRE_Z;

	nTrailingX = (int32)(cos(((float)m_nScanPan / 360.0f) * TWO_PI) * REMORA_SCAN_LENGTH);
	nTrailingY = (int32)(sin(((float)m_nScanPan / 360.0f) * TWO_PI) * REMORA_SCAN_LENGTH);

	nTrailingX += REMORA_SCREEN_CENTRE_X;
	nTrailingY += REMORA_SCREEN_CENTRE_Z;

	nOrigin2X = (int32)(cos(((float)nLeadingAngle / 360.0f) * TWO_PI) * 3);
	nOrigin2Y = (int32)(sin(((float)nLeadingAngle / 360.0f) * TWO_PI) * 3);

	nOrigin2X += REMORA_SCREEN_CENTRE_X;
	nOrigin2Y += REMORA_SCREEN_CENTRE_Z;

	// Get the RGB values for the beam.
	nLeadingR = REMPAL(CI_BEAM_LEADING, CI_RED);
	nLeadingG = REMPAL(CI_BEAM_LEADING, CI_GREEN);
	nLeadingB = REMPAL(CI_BEAM_LEADING, CI_BLUE);

	nTrailingR = REMPAL(CI_BEAM_TRAILING, CI_RED);
	nTrailingG = REMPAL(CI_BEAM_TRAILING, CI_GREEN);
	nTrailingB = REMPAL(CI_BEAM_TRAILING, CI_BLUE);

	// Draw the beam.
	DrawGouraudQuad(REMORA_SCREEN_CENTRE_X, REMORA_SCREEN_CENTRE_Z, nLeadingX, nLeadingY, nOrigin2X, nOrigin2Y, nTrailingX, nTrailingY, nLeadingR, nLeadingG, nLeadingB,
	                nLeadingR, nLeadingG, nLeadingB, nTrailingR, nTrailingG, nTrailingB, nTrailingR, nTrailingG, nTrailingB, -255);
}

void _remora::AddInterference() const {
	uint32 i;
	int32 nStartX, nStartY, nLength;

	// Loop for how many interference lines we are going to draw.
	for (i = 0; i < 50; ++i) {
		// Pick a random row.
		nStartY = g_icb->getRandomSource()->getRandomNumber(REMORA_SCREEN_HEIGHT - 1);

		// Pick a random startpoint in the row.
		nStartX = g_icb->getRandomSource()->getRandomNumber(REMORA_SCREEN_WIDTH - 1);

		// Pick a random length of interference.
		nLength = g_icb->getRandomSource()->getRandomNumber(40 - 1);

		// Draw a line that int32.
		DrawGouraudTriangle(nStartX, nStartY, nStartX, nStartY, nStartX + nLength, nStartY, 30, 255, 30, 30, 255, 30, 0, 5, 0, REMORA_BRIGHT_MIN);
	}
}

void _remora::DrawCrosshairs() const {
	int32 i;
	_rgb sLineColour;
	int32 nStart, nEnd;

	// Set a colour for the crosshair.
	sLineColour.red = REMPAL(CI_PLAYER, CI_RED);
	sLineColour.green = REMPAL(CI_PLAYER, CI_GREEN);
	sLineColour.blue = REMPAL(CI_PLAYER, CI_BLUE);

	// Draw the vertical line.
	RemoraLineDraw(REMORA_SCREEN_CENTRE_X, 0, REMORA_SCREEN_CENTRE_X, REMORA_SCREEN_HEIGHT, sLineColour, sLineColour);

	// Draw the horizontal line.
	RemoraLineDraw(0, REMORA_SCREEN_CENTRE_Z - 4, REMORA_SCREEN_WIDTH, REMORA_SCREEN_CENTRE_Z - 4, sLineColour, sLineColour);

	// Draw vertical increments.
	nStart = (REMORA_SCREEN_CENTRE_Z - (5 * REMORA_GRID_SIZE)) - 4;
	nEnd = (REMORA_SCREEN_CENTRE_Z + (4 * REMORA_GRID_SIZE)) - 4;
	i = nStart;
	while (i <= nEnd) {
		RemoraLineDraw(REMORA_SCREEN_CENTRE_X, i, REMORA_SCREEN_CENTRE_X + 4, i, sLineColour, sLineColour);
		i += REMORA_GRID_SIZE;
	}

	// Draw horizontal increments.
	nStart = (REMORA_SCREEN_CENTRE_Z - (6 * REMORA_GRID_SIZE)) - 1;
	nEnd = (REMORA_SCREEN_CENTRE_Z + (8 * REMORA_GRID_SIZE)) - 1;
	i = nStart;
	while (i <= nEnd) {
		RemoraLineDraw(i, REMORA_SCREEN_CENTRE_Z - 4, i, REMORA_SCREEN_CENTRE_Z, sLineColour, sLineColour);
		i += REMORA_GRID_SIZE;
	}
}

void _remora::DrawEMPEffect() {
	uint32 i;

	// For now, just add loads more interference.
	for (i = 0; i < 100; ++i)
		AddInterference();
}

void _remora::DrawHeadingText() {
	uint8 nRed, nGreen, nBlue;

	if (!m_bMainHeadingSet)
		return;

	ColourToRGB(m_pDisplayBuffer[0].s_nAttribute, nRed, nGreen, nBlue);
	SetTextColour(nRed, nGreen, nBlue);

	// Casing curves in where the heading is, so we have to move it over a bit.
	MS->Create_remora_text(REMORA_TEXT_LEFT_MARGIN + 5, REMORA_TEXT_TITLE_Y, m_pDisplayBuffer[0].s_pcText, 0, PIN_AT_CENTRE_OF_LEFT, REMORA_ROW_SPACING,
	                       REMORA_CHARACTER_SPACING, REMORA_DISPLAY_WIDTH);
	MS->Render_speech(MS->text_bloc);
	MS->Kill_remora_text();
}

void _remora::DrawProgressBar() {
	uint32 nX;
	_rgb sTailColour, sHeadColour;
	_rgb sBoxColour;

	// Check if bar is currently up.
	if (m_nProgressBarValue == -1)
		return;

	// Work out how much of the bar to draw.
	nX = (REMORA_PROGRESS_BAR_WIDTH * m_nProgressBarValue) / m_nProgressBarTotal;

	// Set a colour for the trailing edge.
	sTailColour.red = REMPAL(CI_PROGRESS_TAIL, CI_RED);
	sTailColour.green = REMPAL(CI_PROGRESS_TAIL, CI_GREEN);
	sTailColour.blue = REMPAL(CI_PROGRESS_TAIL, CI_BLUE);

	// Work out a colour for the leading edge.
	sHeadColour.red = (uint8)(((REMPAL(CI_PROGRESS_HEAD, CI_RED) - sTailColour.red) * m_nProgressBarValue) / m_nProgressBarTotal);
	sHeadColour.green = (uint8)(((REMPAL(CI_PROGRESS_HEAD, CI_GREEN) - sTailColour.green) * m_nProgressBarValue) / m_nProgressBarTotal);
	sHeadColour.blue = (uint8)(((REMPAL(CI_PROGRESS_HEAD, CI_BLUE) - sTailColour.blue) * m_nProgressBarValue) / m_nProgressBarTotal);

	sHeadColour.red = (uint8)(sHeadColour.red + sTailColour.red);
	sHeadColour.green = (uint8)(sHeadColour.green + sTailColour.green);
	sHeadColour.blue = (uint8)(sHeadColour.blue + sTailColour.blue);

	// Draw it.
	DrawGouraudQuad(REMORA_PROGRESS_BAR_X, REMORA_PROGRESS_BAR_Y, REMORA_PROGRESS_BAR_X + nX, REMORA_PROGRESS_BAR_Y, REMORA_PROGRESS_BAR_X,
	                REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT, REMORA_PROGRESS_BAR_X + nX, REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT, sTailColour.red,
	                sTailColour.green, sTailColour.blue, sHeadColour.red, sHeadColour.green, sHeadColour.blue, sTailColour.red, sTailColour.green, sTailColour.blue,
	                sHeadColour.red, sHeadColour.green, sHeadColour.blue, REMORA_BRIGHT_FULL);

	// Draw the box around it.
	sBoxColour.red = REMPAL(CI_HEADING, CI_RED);
	sBoxColour.green = REMPAL(CI_HEADING, CI_GREEN);
	sBoxColour.blue = REMPAL(CI_HEADING, CI_BLUE);

	RemoraLineDraw(REMORA_PROGRESS_BAR_X - 6, REMORA_PROGRESS_BAR_Y - 6, REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 6, REMORA_PROGRESS_BAR_Y - 6, sBoxColour,
	               sBoxColour);
	RemoraLineDraw(REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 6, REMORA_PROGRESS_BAR_Y - 6, REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 6,
	               REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 6, sBoxColour, sBoxColour);
	RemoraLineDraw(REMORA_PROGRESS_BAR_X - 6, REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 6, REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 6,
	               REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 6, sBoxColour, sBoxColour);
	RemoraLineDraw(REMORA_PROGRESS_BAR_X - 6, REMORA_PROGRESS_BAR_Y - 6, REMORA_PROGRESS_BAR_X - 6, REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 6, sBoxColour,
	               sBoxColour);

	RemoraLineDraw(REMORA_PROGRESS_BAR_X - 1, REMORA_PROGRESS_BAR_Y - 1, REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 1, REMORA_PROGRESS_BAR_Y - 1, sBoxColour,
	               sBoxColour);
	RemoraLineDraw(REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 1, REMORA_PROGRESS_BAR_Y - 1, REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 1,
	               REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 1, sBoxColour, sBoxColour);
	RemoraLineDraw(REMORA_PROGRESS_BAR_X - 1, REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 1, REMORA_PROGRESS_BAR_X + REMORA_PROGRESS_BAR_WIDTH + 1,
	               REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 1, sBoxColour, sBoxColour);
	RemoraLineDraw(REMORA_PROGRESS_BAR_X - 1, REMORA_PROGRESS_BAR_Y - 1, REMORA_PROGRESS_BAR_X - 1, REMORA_PROGRESS_BAR_Y + REMORA_PROGRESS_BAR_HEIGHT + 1, sBoxColour,
	               sBoxColour);
}

void _remora::DrawHeaderAndFooterLines() {
	_rgb sLineColour;

	// Set a line colour.
	sLineColour.red = REMPAL(CI_HEADING, CI_RED);
	sLineColour.green = REMPAL(CI_HEADING, CI_GREEN);
	sLineColour.blue = REMPAL(CI_HEADING, CI_BLUE);

	// Draw the header lines.
	RemoraLineDraw(0, REMORA_TEXT_CLIP_TOP - 5, 402, REMORA_TEXT_CLIP_TOP - 5, sLineColour, sLineColour);
	RemoraLineDraw(402, REMORA_TEXT_CLIP_TOP - 5, 452, REMORA_TEXT_CLIP_TOP - 35, sLineColour, sLineColour);
	RemoraLineDraw(452, REMORA_TEXT_CLIP_TOP - 35, 630, REMORA_TEXT_CLIP_TOP - 35, sLineColour, sLineColour);

	RemoraLineDraw(0, REMORA_TEXT_CLIP_TOP - 6, 402, REMORA_TEXT_CLIP_TOP - 6, sLineColour, sLineColour);
	RemoraLineDraw(402, REMORA_TEXT_CLIP_TOP - 6, 452, REMORA_TEXT_CLIP_TOP - 36, sLineColour, sLineColour);
	RemoraLineDraw(452, REMORA_TEXT_CLIP_TOP - 36, 630, REMORA_TEXT_CLIP_TOP - 36, sLineColour, sLineColour);

	RemoraLineDraw(401, REMORA_TEXT_CLIP_TOP - 6, 451, REMORA_TEXT_CLIP_TOP - 36, sLineColour, sLineColour);
	RemoraLineDraw(403, REMORA_TEXT_CLIP_TOP - 6, 451, REMORA_TEXT_CLIP_TOP - 36, sLineColour, sLineColour);
	RemoraLineDraw(401, REMORA_TEXT_CLIP_TOP - 6, 450, REMORA_TEXT_CLIP_TOP - 36, sLineColour, sLineColour);
	RemoraLineDraw(452, REMORA_TEXT_CLIP_TOP - 35, 401, REMORA_TEXT_CLIP_TOP - 6, sLineColour, sLineColour);
}

void _remora::ClipTopAndBottom() {
	uint32 i;
	LRECT sBlankingRect;
	_rgb oLineColour;

	// Left and right coordinates are same for both rectangles we have to draw.
	sBlankingRect.left = REMORA_SCREEN_ORIGIN_X;
	sBlankingRect.right = REMORA_SCREEN_ORIGIN_X + REMORA_SCREEN_WIDTH;

	// Set colour to be same as Remora background colour.
	uint32 dwFillPixel = GetPen(m_nCurrentPalette, CI_CLIP);

	// Setup the rectangle to clip the top.
	sBlankingRect.top = 0;
	sBlankingRect.bottom = REMORA_TEXT_CLIP_TOP;

	// Draw the rectangle.
	surface_manager->Blit_fillfx(m_nRemoraSurfaceID, &sBlankingRect, dwFillPixel);

	// Setup the rectangle to clip the bottom.
	sBlankingRect.top = REMORA_TEXT_CLIP_BOTTOM;
	sBlankingRect.bottom = REMORA_SCREEN_HEIGHT;

	// Draw the rectangle.
	surface_manager->Blit_fillfx(m_nRemoraSurfaceID, &sBlankingRect, dwFillPixel);

	// Set a line colour to redraw part of background grid.
	oLineColour.red = REMPAL(CI_GRID, CI_RED);
	oLineColour.green = REMPAL(CI_GRID, CI_GREEN);
	oLineColour.blue = REMPAL(CI_GRID, CI_BLUE);

	// Draw horizontal lines at top of screen.
	for (i = REMORA_GRID_Y; i < REMORA_TEXT_CLIP_TOP; i += REMORA_GRID_SIZE)
		RemoraLineDraw(0, i, REMORA_SCREEN_WIDTH, i, oLineColour, oLineColour);

	// Draw horizontal lines at bottom of screen.
	for (i = REMORA_TEXT_CLIP_BOTTOM - 1; i < REMORA_SCREEN_HEIGHT; i += REMORA_GRID_SIZE)
		RemoraLineDraw(0, i, REMORA_SCREEN_WIDTH, i, oLineColour, oLineColour);

	// Draw vertical lines at top.
	for (i = REMORA_GRID_X; i < REMORA_SCREEN_WIDTH; i += REMORA_GRID_SIZE)
		RemoraLineDraw(i, 0, i, REMORA_TEXT_CLIP_TOP, oLineColour, oLineColour);

	// Draw vertical lines at bottom.
	for (i = REMORA_GRID_X; i < REMORA_SCREEN_WIDTH; i += REMORA_GRID_SIZE)
		RemoraLineDraw(i, REMORA_TEXT_CLIP_BOTTOM, i, REMORA_SCREEN_HEIGHT, oLineColour, oLineColour);
}

void _remora::DrawGrid() {
	uint32 i;
	_rgb oLineColour;

	// Set a line colour.
	oLineColour.red = REMPAL(CI_GRID, CI_RED);
	oLineColour.green = REMPAL(CI_GRID, CI_GREEN);
	oLineColour.blue = REMPAL(CI_GRID, CI_BLUE);

	// Draw horizontal lines.
	for (i = REMORA_GRID_Y; i < REMORA_SCREEN_HEIGHT; i += REMORA_GRID_SIZE)
		RemoraLineDraw(0, i, REMORA_SCREEN_WIDTH, i, oLineColour, oLineColour);

	// Draw vertical lines.
	for (i = REMORA_GRID_X; i < REMORA_SCREEN_WIDTH; i += REMORA_GRID_SIZE)
		RemoraLineDraw(i, 0, i, REMORA_SCREEN_HEIGHT, oLineColour, oLineColour);
}

void _remora::DrawPulse() {
	uint32 i, j;
	int32 nHighlightIndex;
	int32 pnWholePulse[REMORA_PULSE_POINTS * REMORA_MAX_HEALTH][2];
	float pfHighlightValues[REMORA_PULSE_POINTS * REMORA_MAX_HEALTH];
	int32 nBaseX, nBaseY;
	int32 nX1, nY1, nX2, nY2;
	_rgb oLineColour, oDrawColour;
	uint32 nWholePulseIndex;
	int32 nXCoord;
	int32 nWhiteValue;
	float fHighlightStep, fCurrentHighlight;
	c_game_object *pPlayer;
	uint32 nHits, nHealth;

	// Calculate a base drawing point for the whole thing.
	nBaseX = REMORA_PULSE_X;
	nBaseY = REMORA_PULSE_Y;

	// Work out player's health.
	pPlayer = (c_game_object *)MS->objects->Fetch_item_by_name("player");
	nHits = pPlayer->GetIntegerVariable(pPlayer->GetVariable("hits"));

	// This counts from 10 down to zero (check player's script for this figure if it changes).
	if (nHits > 6)
		nHealth = 3;
	else if (nHits > 3)
		nHealth = 2;
	else
		nHealth = 1;

	// Build an array of points for drawing the pulse, depending on how many
	// pulse symbols are required for indicating the player's health.
	nWholePulseIndex = 0;

	for (i = 0; i < nHealth; ++i) {
		for (j = 0; j < REMORA_PULSE_POINTS; ++j) {
			// Copy the x,y point and add the offset to the x for which pulse we're drawing.
			nXCoord = REMORA_PULSE_X + (i * REMORA_PULSE_WIDTH) + pnPulsePoints[j][0];
			pnWholePulse[nWholePulseIndex][1] = REMORA_PULSE_Y + pnPulsePoints[j][1];

			// Apply an offset to the x to centre it in the space available.
			nXCoord += ((REMORA_MAX_HEALTH - nHealth) * REMORA_PULSE_WIDTH / 2);
			pnWholePulse[nWholePulseIndex][0] = nXCoord;

			++nWholePulseIndex;
		}
	}

	// Work out the starting highlight colour.
	nHighlightIndex = m_nPulseHighlight;
	fCurrentHighlight = 0.7f;
	fHighlightStep = fCurrentHighlight / (float)nWholePulseIndex;

	// Go through and work out the real highlight value.
	for (i = 0; i < nWholePulseIndex; ++i) {
		// Assign a highlight.
		pfHighlightValues[nHighlightIndex] = fCurrentHighlight;

		// Work out next one.
		fCurrentHighlight -= fHighlightStep;

		// Move on to next segment of line.
		nHighlightIndex = (nHighlightIndex == -1) ? nWholePulseIndex - 1 : nHighlightIndex - 1;
	}

	// Unhighlighted colour of line is same as floors.
	oLineColour.red = REMPAL(CI_BACKGROUND, CI_RED);
	oLineColour.green = REMPAL(CI_BACKGROUND, CI_GREEN);
	oLineColour.blue = REMPAL(CI_BACKGROUND, CI_BLUE);

	// Draw it.
	for (i = 0; i < nWholePulseIndex - 2; ++i) {
		// Get ends of line to draw.
		nX1 = pnWholePulse[i][0];
		nY1 = pnWholePulse[i][1];
		nX2 = pnWholePulse[i + 1][0];
		nY2 = pnWholePulse[i + 1][1];

		// Draw a bigger blob right at the tip of the highlight.
		if (i == m_nPulseHighlight) {
			oDrawColour.red = 255;
			oDrawColour.green = 255;
			oDrawColour.blue = 255;

			RemoraLineDraw(nX1, nY1, nX2, nY2, oDrawColour, oDrawColour, 5);
		} else {
			// Get amount of white to add to this line segment.
			nWhiteValue = (int32)(255.0f * pfHighlightValues[i]);

			// Work out a colour for this line segment by adding in white to create highlight
			// where needed, capping at pure white (0xff).
			oDrawColour.red = ((nWhiteValue + oLineColour.red) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.red);
			oDrawColour.green = ((nWhiteValue + oLineColour.green) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.green);
			oDrawColour.blue = ((nWhiteValue + oLineColour.blue) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.blue);

			RemoraLineDraw(nX1, nY1, nX2, nY2, oDrawColour, oDrawColour);
		}
	}

	// Move the highlight.
	m_nPulseHighlight = (m_nPulseHighlight + 1) % nWholePulseIndex;
}

void _remora::DrawIRLinkPulse() {
	uint32 i;
	float fCurrentHighlight, fHighlightStep;
	float pfForwardHighlight[REMORA_IR_LINK_POINTS];
	float pfReverseHighlight[REMORA_IR_LINK_POINTS];
	int32 nForwardHighlight, nReverseHighlight, nSaveReverseHighlight;
	_rgb oLineColour, oDrawColour;
	int32 nWhiteValue;
	int32 nBaseX, nBaseY;
	int32 nX1, nY1, nX2, nY2;

	// Work out a base point for drawing the data lines.
	nBaseX = REMORA_IRPULSE_X + 20;
	nBaseY = REMORA_IRPULSE_Y + 3;

	// Unhighlighted colour of line is same as floors.
	oLineColour.red = REMPAL(CI_BACKGROUND, CI_RED);
	oLineColour.green = REMPAL(CI_BACKGROUND, CI_GREEN);
	oLineColour.blue = REMPAL(CI_BACKGROUND, CI_BLUE);

	// Initialise.
	nForwardHighlight = m_nPulseHighlight;
	nReverseHighlight = (REMORA_IR_LINK_POINTS - m_nPulseHighlight) - 1;
	nSaveReverseHighlight = nReverseHighlight;
	fCurrentHighlight = 0.7f;
	fHighlightStep = fCurrentHighlight / (float)REMORA_IR_LINK_POINTS;

	// Loop to fill in highlight values for both lines.
	for (i = 0; i < REMORA_IR_LINK_POINTS; ++i) {
		pfForwardHighlight[nForwardHighlight] = fCurrentHighlight;
		pfReverseHighlight[nReverseHighlight] = fCurrentHighlight;

		fCurrentHighlight -= fHighlightStep;

		nForwardHighlight = (nForwardHighlight > 0) ? nForwardHighlight - 1 : REMORA_IR_LINK_POINTS - 1;
		nReverseHighlight = (nReverseHighlight + 1) % REMORA_IR_LINK_POINTS;
	}

	// Draw the lines.
	for (i = 0; i < REMORA_IR_LINK_POINTS - 1; ++i) {
		// Draw forward line.
		nX1 = pnIRLinkPoints[i][0] + nBaseX;
		nY1 = pnIRLinkPoints[i][1] + nBaseY;
		nX2 = pnIRLinkPoints[i + 1][0] + nBaseX;
		nY2 = pnIRLinkPoints[i + 1][1] + nBaseY;

		if (i == m_nPulseHighlight) {
			oDrawColour.red = REMPAL(CI_WARNING, CI_RED);
			oDrawColour.green = REMPAL(CI_WARNING, CI_GREEN);
			oDrawColour.blue = REMPAL(CI_WARNING, CI_BLUE);

			RemoraLineDraw(nX1, nY1, nX2, nY2, oDrawColour, oDrawColour, 5);
		} else {
			// Get amount of white to add to this line segment.
			nWhiteValue = (int32)(255.0f * pfForwardHighlight[i]);

			// Work out a colour for this line segment by adding in white to create highlight
			// where needed, capping at pure white (0xff).
			oDrawColour.red = ((nWhiteValue + oLineColour.red) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.red);
			oDrawColour.green = ((nWhiteValue + oLineColour.green) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.green);
			oDrawColour.blue = ((nWhiteValue + oLineColour.blue) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.blue);

			RemoraLineDraw(nX1, nY1, nX2, nY2, oDrawColour, oDrawColour);
		}

		// Draw reverse line.
		nX1 = pnIRLinkPointsR[i][0] + nBaseX;
		nY1 = pnIRLinkPointsR[i][1] + nBaseY;
		nX2 = pnIRLinkPointsR[i + 1][0] + nBaseX;
		nY2 = pnIRLinkPointsR[i + 1][1] + nBaseY;

		if (i == (uint32)nSaveReverseHighlight) {
			oDrawColour.red = REMPAL(CI_WARNING, CI_RED);
			oDrawColour.green = REMPAL(CI_WARNING, CI_GREEN);
			oDrawColour.blue = REMPAL(CI_WARNING, CI_BLUE);

			RemoraLineDraw(nX1, nY1, nX2, nY2, oDrawColour, oDrawColour, 5);
		} else {
			// Get amount of white to add to this line segment.
			nWhiteValue = (int32)(255.0f * pfReverseHighlight[i]);

			// Work out a colour for this line segment by adding in white to create highlight
			// where needed, capping at pure white (0xff).
			oDrawColour.red = ((nWhiteValue + oLineColour.red) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.red);
			oDrawColour.green = ((nWhiteValue + oLineColour.green) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.green);
			oDrawColour.blue = ((nWhiteValue + oLineColour.blue) > 255) ? (uint8)255 : (uint8)(nWhiteValue + oLineColour.blue);

			RemoraLineDraw(nX1, nY1, nX2, nY2, oDrawColour, oDrawColour);
		}
	}

	// Move the highlight.
	m_nPulseHighlight = (m_nPulseHighlight + 1) % REMORA_IR_LINK_POINTS;

	// Set drawing colour for symbols.
	oDrawColour.red = REMPAL(CI_WARNING, CI_RED);
	oDrawColour.green = REMPAL(CI_WARNING, CI_GREEN);
	oDrawColour.blue = REMPAL(CI_WARNING, CI_BLUE);

	// Base point for drawing the Remora symbol.
	nBaseX = REMORA_IRPULSE_X;
	nBaseY = REMORA_IRPULSE_Y;

	RemoraLineDraw(nBaseX, nBaseY, nBaseX + 20, nBaseY, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX + 20, nBaseY, nBaseX + 20, nBaseY + 15, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX, nBaseY + 15, nBaseX + 20, nBaseY + 15, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX, nBaseY + 15, nBaseX, nBaseY, oDrawColour, oDrawColour);

	RemoraLineDraw(nBaseX + 5, nBaseY, nBaseX + 5, nBaseY - 2, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX + 5, nBaseY - 2, nBaseX + 15, nBaseY - 2, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX + 15, nBaseY - 2, nBaseX + 15, nBaseY, oDrawColour, oDrawColour);

	RemoraLineDraw(nBaseX + 5, nBaseY + 15, nBaseX + 5, nBaseY + 17, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX + 5, nBaseY + 17, nBaseX + 15, nBaseY + 17, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX + 15, nBaseY + 17, nBaseX + 15, nBaseY + 15, oDrawColour, oDrawColour);

	// Base point for drawing the Remora symbol.
	nBaseX = REMORA_IRPULSE_X + 56;
	nBaseY = REMORA_IRPULSE_Y + 11;

	RemoraLineDraw(nBaseX, nBaseY, nBaseX + 40, nBaseY, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX + 40, nBaseY, nBaseX + 40, nBaseY + 15, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX, nBaseY + 15, nBaseX + 40, nBaseY + 15, oDrawColour, oDrawColour);
	RemoraLineDraw(nBaseX, nBaseY + 15, nBaseX, nBaseY, oDrawColour, oDrawColour);

	// Draw the data arrows.
	if ((m_nPulseHighlight == 8) || (m_nPulseHighlight == 9)) {
		RemoraLineDraw(nBaseX + 4, nBaseY + 4, nBaseX + 8, nBaseY + 8, oDrawColour, oDrawColour);
		RemoraLineDraw(nBaseX + 4, nBaseY + 12, nBaseX + 8, nBaseY + 8, oDrawColour, oDrawColour);
	} else if ((m_nPulseHighlight == 2) || (m_nPulseHighlight == 3)) {
		RemoraLineDraw(nBaseX + 4, nBaseY + 8, nBaseX + 9, nBaseY + 4, oDrawColour, oDrawColour);
		RemoraLineDraw(nBaseX + 4, nBaseY + 8, nBaseX + 9, nBaseY + 12, oDrawColour, oDrawColour);
	}
}

void _remora::DrawMoreUpDownArrows() {
	// Check the flash timer before drawing anything.
	if (!m_bFlashingTextVisible)
		return;

	// Check if top arrow is required.
	if (m_nFirstLineToDraw > REMORA_FIRST_SCROLLING_LINE) {
		// Yes, there is some text off the top of the screen.
		surface_manager->Blit_surface_to_surface(m_nMoreUpSurfaceID, m_nRemoraSurfaceID, &m_sMoreUpSourceRectangle, &m_sMoreUpTargetRectangle, DDBLT_KEYSRC);
	}

	// Check if the bottom arrow is required.
	if (m_nNextAvailableRow > (m_nFirstLineToDraw + REMORA_DISPLAYED_TEXT_ROWS)) {
		// Yes, there is some text off the bottom of the screen.
		surface_manager->Blit_surface_to_surface(m_nMoreDownSurfaceID, m_nRemoraSurfaceID, &m_sMoreDownSourceRectangle, &m_sMoreDownTargetRectangle, DDBLT_KEYSRC);
	}
}

void _remora::ColourToRGB(uint8 nAttributes, uint8 &nRed, uint8 &nGreen, uint8 &nBlue) const {
	uint32 nColourAttribute;

	// The colour is stored in the lower 4 bits.
	nColourAttribute = (uint32)(nAttributes & 0x0f);

	// Need to get the bit position and map to an integer.
	switch (nColourAttribute) {
	case 0x01:
		nColourAttribute = 0;
		break;
	case 0x02:
		nColourAttribute = 1;
		break;
	case 0x04:
		nColourAttribute = 2;
		break;
	case 0x08:
		nColourAttribute = 3;
		break;
	}

	// Set the RGB for the colour.
	nRed = pnRemoraColour[m_nCurrentPalette][nColourAttribute][CI_RED];
	nGreen = pnRemoraColour[m_nCurrentPalette][nColourAttribute][CI_GREEN];
	nBlue = pnRemoraColour[m_nCurrentPalette][nColourAttribute][CI_BLUE];
}

void _remora::SetUpM08LockControl() {
	uint32 oBitmapNameHash;

	// We need the size of the scan screen for clipping the barriers.
	oBitmapNameHash = NULL_HASH;

	// Here we build a list of the door names and what object ID they are.
	BuildM08DoorList();
}

void _remora::DrawM08LockControl() {
	_rgb oLineColour;

	// Set up the function to map game points to the Remora's screen.  Doesn't actually need calling each
	// cycle, but it keeps it consistent with the motion scan and it won't hurt.
	InitialiseGameToRemora(0.0f, REMORA_M08_ZOOM_X, REMORA_M08_ZOOM_Z, REMORA_M08_X_ORIGIN, REMORA_M08_Z_ORIGIN);

	// Set a line colour.
	oLineColour.red = pnRemoraColour[m_nCurrentPalette][CI_M08_BARRIERS][CI_RED];
	oLineColour.green = pnRemoraColour[m_nCurrentPalette][CI_M08_BARRIERS][CI_GREEN];
	oLineColour.blue = pnRemoraColour[m_nCurrentPalette][CI_M08_BARRIERS][CI_BLUE];

	// Draw the non-animating barriers.
	DrawStaticBarriers(oLineColour);

	// Draw the animating barriers.  At some point, we are going to have to provide some visual
	// indication of whether or not they are open or closed.
	DrawAnimatingBarriers(oLineColour);

	// Draw the floor rectangles.
	DrawFloorRectangles();

	// Draw other object symbols.
	DrawM08LockControlObjects();

	// Draw the door-lock symbols.
	DrawM08DoorLocks();
}

void _remora::DrawM08DoorLocks() {
	uint32 i;
	int32 nX, nZ;
	float fX, fZ;
	_logic *pGameObject;
	uint32 nDoorID;
	char pcDigitString[16];
	_rs_params sSpriteParams;

	PXTRY

	// All sprites are going to be drawn the same, so we can fill
	// in the parameter block here.
	sSpriteParams.bCentre = TRUE8;
	sSpriteParams.bUpdate = FALSE8;

	// Loop for each door lock.
	for (i = 0; i < REMORA_M08_NUM_LOCKS; ++i) {
		// Get the ID of this door.
		nDoorID = m_pnDoorIDs[i];

		// Get the game object.
		pGameObject = MS->logic_structs[nDoorID];

		// Ignore held objects.
		if ((pGameObject->ob_status != OB_STATUS_HELD) && pGameObject->prop_coords_set) {
			// Get the object's x, z.
			fX = pGameObject->prop_xyz.x;
			fZ = pGameObject->prop_xyz.z;

			// Map the point to the Remora's screen.
			GameToRemora(fX, fZ);

			// We only need integers now.
			nX = (int32)fX;
			nZ = (int32)fZ;

			// Check the point is on the screen.  Allow a 20-pixel border.
			if ((nX > 20) && (nX < REMORA_SCREEN_WIDTH - 20) && (nZ > 20) && (nZ < REMORA_SCREEN_HEIGHT - 20)) {
				// Draw either a red or green symbol depending on the door's state.
				if (MS->Fetch_object_integer_variable(pGameObject->GetName(), "locked_in_place") == 1) {
					SetTextColour(REMPAL(CI_M08_LOCKED_DOOR, CI_RED), REMPAL(CI_M08_LOCKED_DOOR, CI_GREEN), REMPAL(CI_M08_LOCKED_DOOR, CI_BLUE));
				} else {
					SetTextColour(REMPAL(CI_M08_UNLOCKED_DOOR, CI_RED), REMPAL(CI_M08_UNLOCKED_DOOR, CI_GREEN), REMPAL(CI_M08_UNLOCKED_DOOR, CI_BLUE));
				}

				// Write a number on them so player knows which icon controls which lock.
				snprintf(pcDigitString, 16, "%d", i + 1);

				MS->Create_remora_text(nX, nZ - 7, pcDigitString, 0, PIN_AT_CENTRE, 0, 0, REMORA_DISPLAY_WIDTH);

				MS->Render_speech(MS->text_bloc);
				MS->Kill_remora_text();
			}
		}
	}

	PXCATCH

	Fatal_error("Exception in _remora::DrawM08DoorLocks()");

	PXENDCATCH
}

void _remora::DrawM08LockControlObjects() {
	uint32 i;
	float fX, fZ;
	int32 nX, nY, nZ;
	_logic *pGameObject;
	ScreenSymbol eSymbol;
	static bool8 bArmedRechargeFlash;

	PXTRY

	// This is the loop that draws the blips for megas.
	for (i = 0; i < MS->total_objects; ++i) {
		// Ignore held objects.
		if (MS->logic_structs[i]->ob_status != OB_STATUS_HELD) {
			// Get the game object.
			pGameObject = MS->logic_structs[i];

			// Get the x,y,z of the object.
			if (pGameObject->image_type == VOXEL) {
				fX = pGameObject->mega->actor_xyz.x;
				fZ = pGameObject->mega->actor_xyz.z;
				nY = (int32)pGameObject->mega->actor_xyz.y;
			} else {
				fX = pGameObject->prop_xyz.x;
				fZ = pGameObject->prop_xyz.z;
				nY = (int32)pGameObject->prop_xyz.y;
			}

			// Ignore objects which are not on our height.
			if (m_nPlayerY == nY) {
				// Map the point to the Remora's screen.
				GameToRemora(fX, fZ);

				// We only need integers now.
				nX = (int32)fX;
				nZ = (int32)fZ;

				// Here we get an ID back of which symbol to draw for the object
				eSymbol = GetSymbolToDrawObject(pGameObject, i);

				switch (eSymbol) {
				case SS_REMORA:
					// Fudge drawing player symbol at correct angle 'cos it's fixed.
					DrawGouraudTriangle(nX + (REMORA_SYMBOL_SIZE_NORMAL >> 1), nZ - REMORA_SYMBOL_SIZE_NORMAL, nX + (REMORA_SYMBOL_SIZE_NORMAL << 1),
					                    nZ + REMORA_SYMBOL_SIZE_NORMAL, nX - (REMORA_SYMBOL_SIZE_NORMAL >> 1), nZ + REMORA_SYMBOL_SIZE_NORMAL,
					                    REMPAL(CI_PLAYER, CI_RED), REMPAL(CI_PLAYER, CI_GREEN), REMPAL(CI_PLAYER, CI_BLUE), REMPAL(CI_PLAYER, CI_RED),
					                    REMPAL(CI_PLAYER, CI_GREEN), REMPAL(CI_PLAYER, CI_BLUE), REMPAL(CI_PLAYER, CI_RED), REMPAL(CI_PLAYER, CI_GREEN),
					                    REMPAL(CI_PLAYER, CI_BLUE), 255);

					break;

				case ALIVE_ROBOT:
					DrawSquareSymbol(nX, nZ, CI_ROBOT, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case DEAD_ROBOT:
					DrawSquareSymbol(nX, nZ, CI_DEAD_ROBOT, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case ALIVE_HUMAN:
					DrawOctagonSymbol(nX, nZ, CI_HUMAN, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case DEAD_HUMAN:
					DrawOctagonSymbol(nX, nZ, CI_DEAD_HUMAN, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case RECHARGE_UNARMED:
					DrawSquareSymbol(nX, nZ, CI_RECHARGE, REMORA_SYMBOL_SIZE_NORMAL);
					break;

				case RECHARGE_ARMED:
					if (bArmedRechargeFlash) {
						DrawSquareSymbol(nX, nZ, CI_RECHARGE, REMORA_SYMBOL_SIZE_NORMAL);
						bArmedRechargeFlash = FALSE8;
					} else {
						DrawSquareSymbol(nX, nZ, CI_RECHARGE_ARMED, REMORA_SYMBOL_SIZE_NORMAL);
						bArmedRechargeFlash = TRUE8;
					}
					break;
				default:
					break;
				} // end switch
			} // end if
		} // end if
	} // end for

	PXCATCH

	Fatal_error("Exception in _remora::DrawM08LockControlObjects()");

	PXENDCATCH
}

void _remora::RemoraLineDraw(int32 nX1, int32 nZ1, int32 nX2, int32 nZ2, _rgb sColour, _rgb, uint32 nHalfThickness) const {
	_rgb col = sColour;

	BlendedLine(nX1, nZ1, nX2, nZ2, col, m_nRemoraSurfaceID);

	for (uint32 i = 1; i < nHalfThickness; i++) {
		col.blue >>= 1;
		col.green >>= 1;
		col.red >>= 1;
		col.alpha >>= 1;
		BlendedLine(nX1 - i, nZ1 - i, nX2 - i, nZ2 - i, col, m_nRemoraSurfaceID);
		BlendedLine(nX1 + i, nZ1 + i, nX2 + i, nZ2 + i, col, m_nRemoraSurfaceID);
	}
}

void _remora::SetUpSurfaceForBitmap(const char *pcBitmapName, DXrect &sSourceRect, DXrect &sTargetRect, uint32 &nSurfaceID) {
	_pxBitmap *pBitmap;
	_pxSprite *pSprite;
	uint32 nFullBitmapNameHash;
	uint8 *pSurfaceBitmap;
	uint32 nPitch;
	const char *pcFullBitmapName;

	// Now do the same with the screen inlay.
	nFullBitmapNameHash = NULL_HASH;
	pcFullBitmapName = MakeRemoraGraphicsPath(pcBitmapName);
	pBitmap = (_pxBitmap *)rs_remora->Res_open(pcFullBitmapName, nFullBitmapNameHash, m_pcRemoraCluster, m_nRemoraClusterHash);

	if (pBitmap->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", pcFullBitmapName, PC_BITMAP_SCHEMA, pBitmap->schema);

	pSprite = pBitmap->Fetch_item_by_number(0);

	// Prepare the source/target rectangles for blitting later.
	sSourceRect = MakeRECTFromSpriteSizes(0, 0, pSprite->width, pSprite->height);
	sTargetRect = MakeRECTFromSpriteSizes(pSprite->x, pSprite->y, pSprite->width, pSprite->height);

	nSurfaceID = surface_manager->Create_new_surface(pcBitmapName, pSprite->width, pSprite->height, SYSTEM);
	surface_manager->Set_transparent_colour_key(nSurfaceID, g_oIconMenu->GetTransparencyKey());

	pSurfaceBitmap = surface_manager->Lock_surface(nSurfaceID);
	nPitch = surface_manager->Get_pitch(nSurfaceID);
	SpriteXYFrameDraw(pSurfaceBitmap, nPitch, pSprite->width, pSprite->height, pBitmap, 0, 0, 0, FALSE8, NULL, 255);
	surface_manager->Unlock_surface(nSurfaceID);
}

inline void _remora::DrawSquareSymbol(int32 nX, int32 nY, uint32 nPal, uint32 nSize) {
	int32 nX1, nX2, nY1, nY2;

	// Work out the coordinates for it
	nX1 = nX - nSize;
	nX2 = nX + nSize;
	nY1 = nY - nSize;
	nY2 = nY + nSize;

	// Check the symbol is on the screen.
	if (nX1 < REMORA_SYMBOL_BORDER)
		return;

	if (nX2 > (REMORA_SCREEN_WIDTH - REMORA_SYMBOL_BORDER))
		return;

	if (nY1 < REMORA_SYMBOL_BORDER)
		return;

	if (nY2 > (REMORA_SCREEN_HEIGHT - REMORA_SYMBOL_BORDER))
		return;

	// Okay, safe to draw.
	DrawGouraudQuad(nX1, nY1, nX2, nY1, nX1, nY2, nX2, nY2, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                REMPAL(nPal, CI_BLUE), 255);
}

inline void _remora::DrawTriangleSymbol(int32 nX, int32 nY, uint32 nPal, uint32 nSize) {
	int32 nX1, nX2, nY1, nY2, nX3, nY3;

	// Work out the coordinates for it.
	nX1 = nX;
	nY1 = nY - (nSize << 1);
	nX2 = nX - nSize;
	nY2 = nY + nSize;
	nX3 = nX + nSize;
	nY3 = nY + nSize;

	// Check the symbol is on the screen.
	if (nX1 < REMORA_SYMBOL_BORDER)
		return;

	if (nX2 > (REMORA_SCREEN_WIDTH - REMORA_SYMBOL_BORDER))
		return;

	if (nY1 < REMORA_SYMBOL_BORDER)
		return;

	if (nY2 > (REMORA_SCREEN_HEIGHT - REMORA_SYMBOL_BORDER))
		return;

	// Okay, safe to draw.
	DrawGouraudTriangle(nX1, nY1, nX2, nY2, nX3, nY3, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);
}

inline void _remora::DrawOctagonSymbol(int32 nX, int32 nY, uint32 nPal, uint32 nSize) {
	int32 nX1, nX2, nX3, nX4;
	int32 nY1, nY2, nY3, nY4;

	// Work out the coordinates for it.
	nX1 = nX - nSize;
	nX2 = nX - ((nSize >> 1));
	nX3 = nX + ((nSize >> 1));
	nX4 = nX + nSize;

	nY1 = nY - nSize;
	nY2 = nY - ((nSize >> 1));
	nY3 = nY + ((nSize >> 1));
	nY4 = nY + nSize;

	// Check the symbol is on the screen.
	if (nX1 < REMORA_SYMBOL_BORDER)
		return;

	if (nX4 > (REMORA_SCREEN_WIDTH - REMORA_SYMBOL_BORDER))
		return;

	if (nY1 < REMORA_SYMBOL_BORDER)
		return;

	if (nY4 > (REMORA_SCREEN_HEIGHT - REMORA_SYMBOL_BORDER))
		return;

	// Okay, safe to draw.
	DrawGouraudTriangle(nX1, nY2, nX2, nY1, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);

	DrawGouraudTriangle(nX2, nY1, nX3, nY1, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);

	DrawGouraudTriangle(nX3, nY1, nX4, nY2, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);

	DrawGouraudTriangle(nX4, nY2, nX4, nY3, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);

	DrawGouraudTriangle(nX4, nY3, nX3, nY4, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);

	DrawGouraudTriangle(nX3, nY4, nX2, nY4, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);

	DrawGouraudTriangle(nX2, nY4, nX1, nY3, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);

	DrawGouraudTriangle(nX1, nY3, nX1, nY2, nX, nY, REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN),
	                    REMPAL(nPal, CI_BLUE), REMPAL(nPal, CI_RED), REMPAL(nPal, CI_GREEN), REMPAL(nPal, CI_BLUE), 255);
}

void DrawGouraudQuad(uint32 nX0, uint32 nY0, uint32 nX1, uint32 nY1, uint32 nX2, uint32 nY2, uint32 nX3, uint32 nY3, uint8 nRed0, uint8 nGreen0, uint8 nBlue0, uint8 nRed1,
                     uint8 nGreen1, uint8 nBlue1, uint8 nRed2, uint8 nGreen2, uint8 nBlue2, uint8 nRed3, uint8 nGreen3, uint8 nBlue3, int32 nOpacity) {
	DrawGouraudTriangle(nX0, nY0, nX1, nY1, nX2, nY2, nRed0, nGreen0, nBlue0, nRed1, nGreen1, nBlue1, nRed2, nGreen2, nBlue2, nOpacity);

	DrawGouraudTriangle(nX1, nY1, nX2, nY2, nX3, nY3, nRed1, nGreen1, nBlue1, nRed2, nGreen2, nBlue2, nRed3, nGreen3, nBlue3, nOpacity);
}

void DrawGouraudTriangle(uint32 x0, uint32 y0, uint32 x1, uint32 y1, uint32 x2, uint32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
                         int32 nOpacity) {
	// Range-check the opacity.
	if (nOpacity == 0)
		return;

	int32 div;

	int32 minY;
	int32 maxY;

	Point2DColoured ps[3];
	Point2DColoured *p0;
	Point2DColoured *p1;

	int32 i;

	int32 dx, dy;
	int32 x, y;
	int32 ystep;
	int32 xstep;

	int32 rstep, gstep, bstep, r, g, b;

	int32 count;
	int32 swap;

	uint8 *pSurfaceBitmap; // Pointer to the surface we are going to draw on.
	uint32 nPitch;         // Pitch of the surface.

	ps[0].x = x0;
	ps[0].y = y0;
	ps[0].r = r0;
	ps[0].g = g0;
	ps[0].b = b0;
	ps[1].x = x1;
	ps[1].y = y1;
	ps[1].r = r1;
	ps[1].g = g1;
	ps[1].b = b1;
	ps[2].x = x2;
	ps[2].y = y2;
	ps[2].r = r2;
	ps[2].g = g2;
	ps[2].b = b2;

	// Rendering colour array
	uint8 newCol[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	// GET MIN AND MAX
	minY = ps[0].y;
	maxY = ps[0].y;
	for (i = 1; i < 3; i++) {
		if ((ps[i].y) < minY)
			minY = ps[i].y;
		if ((ps[i].y) > maxY)
			maxY = ps[i].y;
	}

	// clamp min y and max y
	if (minY < 0)
		minY = 0;
	if (maxY >= REMORA_SCREEN_HEIGHT)
		maxY = REMORA_SCREEN_HEIGHT - 1;

	// RESET SPANS
	for (i = minY; i <= maxY; i++) {
		remora_spans[i].x0 = -99999;
		remora_spans[i].x1 = -99999;
	}

	// scan line
	for (i = 0; i < 3; i++) {
		p0 = ps + i;

		// work out next point along
		if (i == 2)
			p1 = ps;
		else
			p1 = ps + i + 1;

		if (p0->y == p1->y) {
			xstep = 0;
			ystep = 0;
			rstep = 0;
			gstep = 0;
			bstep = 0;
		} else {
			dx = (p1->x) - (p0->x);

			if (p1->y > p0->y) {
				dy = (p1->y) - (p0->y);
				ystep = 1;
			} else {
				dy = (p0->y) - (p1->y);
				ystep = -1;
			}

			div = (0xffff) / dy;

			xstep = (div * dx) >> 8;

			rstep = (div * (p1->r - p0->r)) >> 8;
			gstep = (div * (p1->g - p0->g)) >> 8;
			bstep = (div * (p1->b - p0->b)) >> 8;
		}

		x = p0->x << 8;
		y = p0->y;

		r = p0->r << 8;
		g = p0->g << 8;
		b = p0->b << 8;

		do {
			if (r < 0)
				r = 0;
			if (g < 0)
				g = 0;
			if (b < 0)
				b = 0;
			if ((y > 0) && (y < REMORA_SCREEN_HEIGHT)) {
				int32 sx = x >> 8;
				if (remora_spans[y].x0 == -99999) {
					remora_spans[y].x0 = sx;
					remora_spans[y].r0 = (uint8)(r >> 8);
					remora_spans[y].g0 = (uint8)(g >> 8);
					remora_spans[y].b0 = (uint8)(b >> 8);
				} else if (sx > remora_spans[y].x0) {
					remora_spans[y].x1 = sx;
					remora_spans[y].r1 = (uint8)(r >> 8);
					remora_spans[y].g1 = (uint8)(g >> 8);
					remora_spans[y].b1 = (uint8)(b >> 8);
				} else { // must swap
					swap = remora_spans[y].x0;
					remora_spans[y].x0 = sx;
					remora_spans[y].x1 = swap;

					swap = remora_spans[y].r0;
					remora_spans[y].r0 = (uint8)(r >> 8);
					remora_spans[y].r1 = (uint8)swap;

					swap = remora_spans[y].g0;
					remora_spans[y].g0 = (uint8)(g >> 8);
					remora_spans[y].g1 = (uint8)swap;

					swap = remora_spans[y].b0;
					remora_spans[y].b0 = (uint8)(b >> 8);
					remora_spans[y].b1 = (uint8)swap;
				}
			}
			x += xstep;
			y += ystep;

			r += rstep;
			g += gstep;
			b += bstep;
		} while (y != p1->y);
	}

	// draw remora_spans

	// Get the surface to draw on.
	pSurfaceBitmap = surface_manager->Lock_surface(g_oRemora->GetRemoraSurfaceId());

	// Get its pitch.
	nPitch = surface_manager->Get_pitch(g_oRemora->GetRemoraSurfaceId()) >> 2;

	// Check to see if pixel-merging or the special brightening mode are required.  This check is done
	// here to avoid repeating it in the inner loop.
	if (nOpacity < 0) {
		nOpacity = -nOpacity;

		// The new pixel is used only to brighten what is already in the surface.
		for (y = minY; y <= maxY; ++y) {
			count = remora_spans[y].x1 - remora_spans[y].x0;
			if (count > 0) {
				div = 0xffff / count;

				rstep = (div * (remora_spans[y].r1 - remora_spans[y].r0)) >> 8;
				gstep = (div * (remora_spans[y].g1 - remora_spans[y].g0)) >> 8;
				bstep = (div * (remora_spans[y].b1 - remora_spans[y].b0)) >> 8;

				int32 ar = remora_spans[y].r0 << 8;
				int32 ag = remora_spans[y].g0 << 8;
				int32 ab = remora_spans[y].b0 << 8;

				x = remora_spans[y].x0;

				while (x < 0) {
					ar += rstep;
					ag += gstep;
					ab += bstep;
					++x;
				}

				int xLim = remora_spans[y].x1;
				if (xLim > REMORA_SCREEN_WIDTH)
					xLim = REMORA_SCREEN_WIDTH;

				uint32 *left = (uint32 *)(pSurfaceBitmap) + (nPitch * y) + x;
				while (x < xLim) {
					if (ar < 0)
						ar = 0;
					if (ag < 0)
						ag = 0;
					if (ab < 0)
						ab = 0;
					newCol[0] = (uint8)(ab >> 8);
					newCol[1] = (uint8)(ag >> 8);
					newCol[2] = (uint8)(ar >> 8);

#if 1
					// 32-bit BGR pixel
					uint8 *pixel = (uint8 *)&left;
					uint8 *add = (uint8 *)&newCol;
					// Add from RGB components
					for (int p = 0; p < 3; p++) {
						pixel[p] = MIN(255, pixel[p] + add[p]);
					}
#else
					__asm {
						lea  edi, newCol    ; // Get the adress of the colour table
						mov  esi, left      ; // Load the address of the pixel pointer
						movq mm0, [edi]     ; // Load the colour
						pxor mm4, mm4       ; // Clear mm4 for packing/unpacking
						movd mm5, [esi]     ; // Get a pixel
						paddusb mm5, mm0    ; // Add the colour
						movd  [esi], mm5    ; // Store the new colour
						emms
					}
#endif

					++left;
					ar += rstep;
					ag += gstep;
					ab += bstep;
					++x;
				}
			}
		}
	} else if (nOpacity == 255) {
		// New pixel fully overwrites the existing pixel.
		for (y = minY; y <= maxY; y++) {
			count = remora_spans[y].x1 - remora_spans[y].x0;
			if (count > 0) {
				div = (0xffff) / count;

				rstep = (div * (remora_spans[y].r1 - remora_spans[y].r0)) >> 8;
				gstep = (div * (remora_spans[y].g1 - remora_spans[y].g0)) >> 8;
				bstep = (div * (remora_spans[y].b1 - remora_spans[y].b0)) >> 8;

				r = remora_spans[y].r0 << 8;
				g = remora_spans[y].g0 << 8;
				b = remora_spans[y].b0 << 8;

				x = remora_spans[y].x0;

				if (x < 0) {
					r -= rstep * x;
					g -= gstep * x;
					b -= bstep * x;
					count += x;
					x = 0;
				}

				if (remora_spans[y].x1 >= REMORA_SCREEN_WIDTH) {
					count -= (remora_spans[y].x1 - REMORA_SCREEN_WIDTH);
				}

				uint32 *left = (uint32 *)(pSurfaceBitmap) + (nPitch * y) + x;
				do {
					*left = ((b >> 8) & 0xff) + (g & 0xff00) + ((r << 8) & 0xff0000);

					++left;

					r += rstep;
					g += gstep;
					b += bstep;
					++x;

					count--;
				} while (count > 0);
			}
		}
	} else {
		// New pixel RGB needs merging into the pixel already in the surface.
		for (y = minY; y <= maxY; y++) {
			// do this after working out go and tex bits
			count = remora_spans[y].x1 - remora_spans[y].x0;
			if ((count > 0) && (remora_spans[y].x1 > 0)) {
				x = remora_spans[y].x0;

				if (x < 0)
					x = 0;

				int xLim = remora_spans[y].x1;
				if (xLim > REMORA_SCREEN_WIDTH)
					xLim = REMORA_SCREEN_WIDTH;

				xLim -= x;

				if (xLim < 1)
					continue;

				uint32 *left = (uint32 *)(pSurfaceBitmap) + (nPitch * y) + x;

				// colours
				newCol[0] = (uint8)remora_spans[y].b0;
				newCol[2] = (uint8)remora_spans[y].g0;
				newCol[4] = (uint8)remora_spans[y].r0;

#if 1
				// 32-bit BGR pixel
				uint8 *pixel = (uint8 *)&left;
				uint8 *add = (uint8 *)&newCol;
				// Add from RGB components
				for (int p = 0; p < 3; p++) {
					pixel[p] = (pixel[p] + add[p]) >> 1;
				}
#else
				_asm {
					lea  edi, newCol; // Get the adress of the colour table
					mov  ecx, xLim  ; // Load the counter
					movq mm0, [edi] ; // Load the colour
					mov  esi, left  ; // Load the address of the pixel pointer
					pxor mm4, mm4   ; // Clear mm4 for packing/unpacking
					sub  esi, 4     ; // compensate for index

					remora_blend_loop:
					movd      mm5, [esi+ecx*4]; // Get a pixel
					punpcklbw mm5, mm4  ; // Unpack the pixel into words
					paddusw   mm5, mm0  ; // Add the colour
					psrlw     mm5, 1    ; // Divide by 2
					packuswb  mm5, mm4  ; // Repack into bytes
					movd  [esi+ecx*4], mm5  ; // Store the new colour
					dec ecx             ; // decrement the counter
					jne remora_blend_loop   ; // On to the next
					emms
				}
#endif
			}
		}
	}

	// Release the surface now we have done drawing.
	surface_manager->Unlock_surface(g_oRemora->GetRemoraSurfaceId());
}

bool8 _remora::CohenSutherland(DXrect oRect, int32 &nZ1, int32 &nY1, int32 &nX2, int32 &nY2, bool8 bClip) const {
	uint32 nOutcode1, nOutcode2;
	uint32 nOutcodeOutside;

	int32 nNewX = nZ1;
	int32 nNewY = nY1;

	// Compute outcode for line's endpoints.
	nOutcode1 = ComputeOutcode(oRect, nZ1, nY1);
	nOutcode2 = ComputeOutcode(oRect, nX2, nY2);

	// Main loop.  In the full algorithm, this runs until clipping is complete, but since this function has to return
	// whether or not the line intersects the rectangle, we might be able to quit early.
	while (TRUE8) {
		// If both outcodes are zero then the line lies wholly within the rectangle.
		if ((nOutcode1 | nOutcode2) == 0x00000000)
			return (TRUE8);

		// If logical AND of codes is not zero then line cannot intersect the rectangle.
		if ((nOutcode1 & nOutcode2) != 0x00000000)
			return (FALSE8);

		// Right the easy tests have failed to give the answer.  Now we pick the endpoint outside the clip rectangle
		// and calculate the line's intersection with the infinitely-extended sides of the clip rectangle.
		nOutcodeOutside = (nOutcode1 != 0x00000000) ? nOutcode1 : nOutcode2;

		if (TestOutcode(nOutcodeOutside, OUTCODE_TOP)) {
			// In here, I calculate the intersection.  If the function is not required to do clipping, I might be able to
			// quit early if the intersection point with the INFINITE rectangle-side passes through the ACTUAL rectangle-side.
			nNewX = nZ1 + (int32)((float)(nX2 - nZ1) * (float)(oRect.top - nY1) / (float)(nY2 - nY1));
			if (!bClip) {
				if ((nNewX >= oRect.left) && (nNewX <= oRect.right))
					return (TRUE8);
			}
			nNewY = oRect.top;
		} else if (TestOutcode(nOutcodeOutside, OUTCODE_BOTTOM)) {
			// See notes on previous if().
			nNewX = nZ1 + (int32)((float)(nX2 - nZ1) * (float)(oRect.bottom - nY1) / (float)(nY2 - nY1));
			if (!bClip) {
				if ((nNewX >= oRect.left) && (nNewX <= oRect.right))
					return (TRUE8);
			}
			nNewY = oRect.bottom;
		} else if (TestOutcode(nOutcodeOutside, OUTCODE_RIGHT)) {
			// See notes on first if().
			nNewY = nY1 + (int32)((float)(nY2 - nY1) * (float)(oRect.right - nZ1) / (float)(nX2 - nZ1));
			if (!bClip) {
				if ((nNewY <= oRect.bottom) && (nNewY >= oRect.top))
					return (TRUE8);
			}
			nNewX = oRect.right;
		} else if (TestOutcode(nOutcodeOutside, OUTCODE_LEFT)) {
			// See notes on first if().
			nNewY = nY1 + (int32)((float)(nY2 - nY1) * (float)(oRect.left - nZ1) / (float)(nX2 - nZ1));
			if (!bClip) {
				if ((nNewY <= oRect.bottom) && (nNewY >= oRect.top))
					return (TRUE8);
			}
			nNewX = oRect.left;
		}

		// Now move the outside endpoint to the intersection point we've just calculated and iterate again.
		if (nOutcodeOutside == nOutcode1) {
			nZ1 = nNewX;
			nY1 = nNewY;
			nOutcode1 = ComputeOutcode(oRect, nZ1, nY1);
		} else {
			nX2 = nNewX;
			nY2 = nNewY;
			nOutcode2 = ComputeOutcode(oRect, nX2, nY2);
		}
	} // end while
}

uint32 _remora::ComputeOutcode(DXrect oRect, int32 nX, int32 nY) const {
	uint32 nRetVal = 0x00000000;

	if (nX < oRect.left)
		nRetVal = OUTCODE_LEFT;
	else if (nX > oRect.right)
		nRetVal = OUTCODE_RIGHT;

	if (nY < oRect.top)
		nRetVal = nRetVal | OUTCODE_TOP;
	else if (nY > oRect.bottom)
		nRetVal = nRetVal | OUTCODE_BOTTOM;

	return (nRetVal);
}

} // End of namespace ICB
