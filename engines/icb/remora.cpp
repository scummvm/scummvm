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

#include "engines/icb/remora.h"
#include "engines/icb/line_of_sight.h"
#include "engines/icb/sound.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/custom_logics.h"
#include "engines/icb/mission.h"

#include "common/stream.h"

namespace ICB {

_remora::_remora() {
	m_eGameState = INACTIVE;
	m_eCurrentMode = MOTION_SCAN;
	m_eLastMode = MOTION_SCAN;
	m_bModeChanged = FALSE8;
	m_eModeOverride = NO_OVERRIDE;
	m_nNextAvailableRow = 0;
	m_nFirstLineToDraw = REMORA_FIRST_SCROLLING_LINE;
	m_bScrollingRequired = FALSE8;
	m_bTextPictureLoaded = FALSE8;
	m_bScrolling = FALSE8;
	m_nStartYPixelOffset = 0;
	m_nCharacterHeight = 0;
	m_bFlashingTextVisible = FALSE8;
	m_nFlashCounter = 0;
	m_bMainHeadingSet = FALSE8;
	m_nCurrentPalette = 0;
	m_pDisplayBuffer = NULL;

	// Set initial zoom range and an initial zoom.
	m_nMinZoom = REMORA_SCAN_ZOOM_HARD_LOWER;
	m_nMaxZoom = REMORA_SCAN_ZOOM_HARD_UPPER;
	m_nCurrentZoom = REMORA_SCAN_START_ZOOM;

	// Name of the cluster which holds remora graphics.
	strcpy(m_pcRemoraCluster, REMORA_CLUSTER_PATH);
	m_nScanPan = 0;

	// Make the hash name of the remora graphics cluster.
	m_nRemoraClusterHash = EngineHashString(m_pcRemoraCluster);

	// No progress bar.
	m_nProgressBarValue = -1;
}

void _remora::InitialiseRemora() {
	// Initialise the mode control variables.
	m_eGameState = INACTIVE;
	m_eCurrentMode = MOTION_SCAN;
	m_eLastMode = MOTION_SCAN;
	m_eModeOverride = NO_OVERRIDE;
	m_bModeChanged = FALSE8;

	// Clear any mega speech (there shouldn't be any though).
	m_pcSpeechText = NULL;
	m_nSpeechTimer = 0;

	// Initially, the zoom is set at 1X and cannot be moved.  This might well change.
	m_nMinZoom = REMORA_SCAN_ZOOM_HARD_LOWER;
	m_nMaxZoom = REMORA_SCAN_ZOOM_HARD_UPPER;
	m_nCurrentZoom = REMORA_SCAN_START_ZOOM;

	// Make sure there is no outstanding email.
	memset(m_pcEmailID, 0, (REMORA_MAXLEN_EMAIL_ID_STRING + 1) * sizeof(char));

	// Text system initialisation.
	m_nNextAvailableRow = 0;
	m_nFirstLineToDraw = REMORA_FIRST_SCROLLING_LINE;
	m_bScrollingRequired = FALSE8;
	m_bTextPictureLoaded = FALSE8;
	m_nScreenFlashCount = 0;
	m_bScrolling = FALSE8;
	m_nStartYPixelOffset = 0;
	m_nCharacterHeight = 0;
	m_bFlashingTextVisible = FALSE8;
	m_nFlashCounter = 0;
	m_bMainHeadingSet = FALSE8;

	// Reset any floor ranges that may have been set.
	m_nNumFloorRangesSet = 0;
	m_nNumCurrentFloorRanges = 0;

	// No progress bar.
	m_nProgressBarValue = -1;

	// Now do any platform-specific initialisation.
	DoPlatformSpecificInitialisation();
}

void _remora::DeactivateRemora(bool8 bForceShutdown) {
	// In order to stop Remora scripts dropping out during pauses and speech, we enforce
	// the rule here that the Remora will not quit unless it is at a point where it is
	// waiting for user-input - i.e. the icon menu is active.
	if (bForceShutdown || g_oIconMenu->IsActive()) {
		// Play the Remora-deactivate sound.
		RegisterSoundSpecial(deactivateRemoraSfx, deactivateRemoraDesc, 127, 0);

		m_eGameState = SWITCHING_OFF;
		ReleaseTextFormattingMemory();
	}
}

void _remora::SetMode(RemoraMode eMode) {
	// Ignore the function call if the Remora is not active.
	if (m_eGameState == INACTIVE)
		return;

	// If we are setting a new mode, we need to flag the fact to make sure the current bit gets
	// closed down properly.
	if (m_eCurrentMode != eMode)
		m_bModeChanged = TRUE8;

	// Clear text in case script writers are jumping into a non-text mode like the scan.
	ClearAllText();

	// And set the new mode.
	m_eLastMode = m_eCurrentMode;
	m_eCurrentMode = eMode;
}

void _remora::SetDefaultOrOverrideMode() {
	// Check if the override is set.
	if (m_eModeOverride != NO_OVERRIDE) {
		// There is an override set, so use it.
		SetMode(m_eModeOverride);
		m_eModeOverride = NO_OVERRIDE;
	} else {
		// No, there isn't one set, so just use the default.
		SetMode(REMORA_DEFAULT_MODE);
	}
}

void _remora::DisplayCharacterSpeech(uint32 nHash) {
	// No character speech is displayed unless sub_titles are turned on
	if (px.on_screen_text == TRUE8) {
		const char *pcText;

		// Find the text in the resources.
		pcText = g_oRemora->LocateTextFromReference(nHash);

		// Check we have a valid pointer.
		if (!pcText)
			Fatal_error("_remora::DisplayCharacterSpeech() failed to find text for hash %x", nHash);

		// Text must begin with an asterisk, which we don't display.
		if (pcText[0] != TS_SPOKEN_LINE)
			Fatal_error("Text [%s] not marked for actors in _remora::DisplayCharacterSpeech()", pcText);

		// Put the text in the buffer.
		m_pcSpeechText = &pcText[1];
	} else {
		m_pcSpeechText = NULL;
	}

	// Initialise the counter for how int32 it will be displayed.
	m_nSpeechTimer = SayLineOfSpeech(nHash);
}

void _remora::SetCurrentZoom(uint32 nZoom) {
	// The zoom gets clipped at the current limits.
	if (nZoom > m_nMaxZoom)
		m_nCurrentZoom = m_nMaxZoom;
	else if (nZoom < m_nMinZoom)
		m_nCurrentZoom = m_nMinZoom;
	else
		m_nCurrentZoom = nZoom;
}

bool8 _remora::IsThisEmailWaiting(const char *pcEmailID) const {
	if ((pcEmailID == NULL) || (strlen(m_pcEmailID) == 0))
		return (FALSE8);

	if (strcmp(pcEmailID, m_pcEmailID))
		return (FALSE8);
	else
		return (TRUE8);
}

bool8 _remora::EMPEffect() {
	if (m_nScreenFlashCount == 0) {
		m_nScreenFlashCount = REMORA_FLASH_EFFECT_LENGTH;
		return (TRUE8);
	} else {
		--m_nScreenFlashCount;

		if (m_nScreenFlashCount == 0)
			return (FALSE8);
		else
			return (TRUE8);
	}
}

void _remora::CycleRemoraLogic(const _input &sKeyboardState) {
	_logic *pPlayerObject;
	uint32 nRemoraID;

	// Count down the speech-display counter.
	m_nSpeechTimer = (m_nSpeechTimer > 0) ? m_nSpeechTimer - 1 : 0;

	// Work the counter that controls the flashing of email waiting symbols.
	++m_nFlashCounter;

	if (m_nFlashCounter == REMORA_TEXT_FLASH_RATE) {
		// Reset the counter and toggle the visibility state of the text.
		m_nFlashCounter = 0;
		m_bFlashingTextVisible = (bool8)!m_bFlashingTextVisible;
	}

	// If text is scrolling, process the scroll.
	if (m_bScrolling) {
		int scrollAmount = (256 * m_nCharacterHeight) / REMORA_SCROLL_CYCLES;

		// See which way we're scrolling.
		if (m_eTextScroll == SCROLL_UP) {
			m_nStartYPixelOffset -= scrollAmount;

			if (m_nStartYPixelOffset <= -256 * m_nCharacterHeight) {
				m_nStartYPixelOffset += 256 * m_nCharacterHeight;
				m_bScrolling = FALSE8;
				m_eTextScroll = SCROLL_NONE;
				++m_nFirstLineToDraw;
			}
		} else {
			m_nStartYPixelOffset += scrollAmount;

			if (m_nStartYPixelOffset >= 256 * m_nCharacterHeight) {
				m_nStartYPixelOffset -= 256 * m_nCharacterHeight;
				m_bScrolling = FALSE8;
				m_eTextScroll = SCROLL_NONE;
				--m_nFirstLineToDraw;
			}
		}
	}

	// This function works a bit like a Windows message handler in that is has to be reentrant.  That's why it has
	// initialisation, normal operation, and close-down sections.
	switch (m_eGameState) {
	case SWITCHING_ON:
		// This creates sprites etc., ready to be blitted to the working buffer.
		SetUpRemora();

		// This initialises the Remora's menu control variables.
		InitialiseMenuControlVariables();

		// Now run any initialisation code for each of the modes.
		switch (m_eCurrentMode) {
		case MOTION_SCAN:
			SetUpWideScan();
			break;

		case INFRA_RED_LINK:
			break;

		case DATABASE:
			break;

		case COMMUNICATIONS:
			break;

		case M08_LOCK_CONTROL:
			SetUpM08LockControl();
			break;

		default:
			// This should never happen.
			Fatal_error("Invalid Remora mode %d while initialising", m_eCurrentMode);

		} // end switch

		// Remora is now active.
		m_eGameState = ACTIVE;

		break;

	case ACTIVE:
		// If there has been a mode change, we need to close down the old mode and start up the new one.
		if (m_bModeChanged) {
			// We're handling the mode change, so clear the trigger nFlag.
			m_bModeChanged = FALSE8;

			// Do the close down for the old mode.
			switch (m_eLastMode) {
			case MOTION_SCAN:
				break;

			case INFRA_RED_LINK:
				break;

			case DATABASE:
				break;

			case COMMUNICATIONS:
				break;

			case MAP:
				break;

			case M08_LOCK_CONTROL:
				break;

			default:
				// This should never happen.
				Fatal_error("Invalid Remora mode %d switching Remora modes", m_eCurrentMode);
			}

			// Do the set-up for the new mode.
			switch (m_eCurrentMode) {
			case MOTION_SCAN:
				SetUpWideScan();
				break;

			case INFRA_RED_LINK:
				break;

			case DATABASE:
				break;

			case COMMUNICATIONS:
				break;

			case M08_LOCK_CONTROL:
				SetUpM08LockControl();
				break;

			default:
				// This should never happen.
				Fatal_error("Invalid Remora mode %d switching to new Remora mode.", m_eCurrentMode);
			}
		} else {
			// There was no mode change, so just run any logic associated with the current mode.
			switch (m_eCurrentMode) {
			case MOTION_SCAN:

				// Update player position (x and z only).
				pPlayerObject = MS->logic_structs[MS->player.Fetch_player_id()];
				m_nPlayerX = (int32)pPlayerObject->mega->actor_xyz.x;
				m_nPlayerZ = (int32)pPlayerObject->mega->actor_xyz.z;

				// Check if the local pan has been updated, which means the Remora is being 'swung'
				// from side to side.
				if (sKeyboardState.IsButtonSet(__SIDESTEP)) {
					if (sKeyboardState.turn == __LEFT)
						m_fPlayerPan = remainder(m_fPlayerPan - REMORA_SCAN_PAN_STEP, FULL_TURN, HALF_TURN);
					else if (sKeyboardState.turn == __RIGHT)
						m_fPlayerPan = remainder(m_fPlayerPan + REMORA_SCAN_PAN_STEP, FULL_TURN, HALF_TURN);
				} else {
					ProcessUpDownZoomKeys(sKeyboardState);
				}

				// Check for the palette-change key.
				if (sKeyboardState.IsButtonSet(__ATTACK))
					m_nCurrentPalette = (uint8)(((int32)m_nCurrentPalette + 1) % REMORA_NUM_COLOUR_SCHEMES);

				// Update the beam position.
				m_nScanPan = (m_nScanPan + REMORA_SCAN_SPEED) % 360;

				break;

			case INFRA_RED_LINK:
				ProcessUpDownTextKeys(sKeyboardState);
				break;

			case DATABASE:
				ProcessUpDownTextKeys(sKeyboardState);
				break;

			case COMMUNICATIONS:
				break;

			case M08_LOCK_CONTROL:
				break;

			default:
				// This should never happen.
				Fatal_error("Invalid Remora mode %d while Remora active", m_eCurrentMode);

			} // end switch

		} // end if

		// Fire button quits the speech timer.
		if (sKeyboardState.IsButtonSet(__ATTACK))
			m_nSpeechTimer = 0;

		break;

	case SWITCHING_OFF:
		// Remora is closing down.  First close down according to which mode it is in.
		switch (m_eCurrentMode) {
		case MOTION_SCAN:
			break;

		case INFRA_RED_LINK:
			break;

		case DATABASE:
			break;

		case COMMUNICATIONS:
			break;

		case MAP:
			break;

		case M08_LOCK_CONTROL:
			break;

		default:
			// This should never happen.
			Fatal_error("Invalid Remora mode %d switching Remora off", m_eCurrentMode);

		} // end switch

		// Close down the icon menu.
		g_oIconMenu->CloseDownIconMenu();

		// Now close down the base graphic.
		CloseDownRemora();

		// Remora is now inactive.
		m_eGameState = INACTIVE;

		nRemoraID = MS->objects->Fetch_item_number_by_name(REMORA_NAME);

		if (nRemoraID == PX_LINKED_DATA_FILE_ERROR)
			Fatal_error("No logic object for Remora in _remora::CycleRemoraLogic()");

		g_oEventManager->PostNamedEventToObject(EVENT_LOGIC_RERUN, nRemoraID, nRemoraID);

		break;

	default:
		// This should never happen.
		Fatal_error("Invalid Remora state");
	}
}

void _remora::SetText(const char *pcText, uint8 nAttribute, uint8 nIndent, _pin_position ePosition) {
	uint32 i;
	_TSparams *psTextParameters;
	const char *pcParsePos;
	uint32 nNumRemoraFormattedLines;
	uint32 nLineLength;
	bool8 bMainHeading;
	uint32 nEffectiveWidth;

	// Ignore the function call if the Remora is not active.
	if (m_eGameState == INACTIVE)
		return;

	// If this is a blank line, deal with it now.
	if (nAttribute == 0) {
		// Check we haven't run out of lines.
		if (m_nNextAvailableRow == REMORA_TEXT_BUFFER_ROWS)
			Fatal_error("Run out of adding blank line in Remora text - limit %d", REMORA_TEXT_BUFFER_ROWS);

		// Insert single blank line.
		m_pDisplayBuffer[m_nNextAvailableRow++].s_nAttribute = 0;

		// Nothing more to do with blank lines.
		return;
	}

	// We have sub-headings and main headings, which share a text colour (and as a bad left-over from the old code,
	// also share a text attribute value).  Main headings are centred though, so we can easily differentiate them.
	if (((nAttribute & REMORA_TEXT_HEADING) != 0) && (ePosition == PIN_AT_CENTRE))
		bMainHeading = TRUE8;
	else
		bMainHeading = FALSE8;

	// Work out the effective print width accounting for tabs.The X-coordinate we pin the text to depends on whether or not the text is centred or not.
	if (ePosition == PIN_AT_CENTRE) {
		nEffectiveWidth = REMORA_DISPLAY_WIDTH;
	} else {
		// Left-justification must also handle tabs.
		nEffectiveWidth = REMORA_DISPLAY_WIDTH - nIndent * (REMORA_TEXT_TAB_ONE);

		// Check we haven't tabbed down to a too-small column of text.
		if (nEffectiveWidth < 50)
			Fatal_error("Too many tabs in [%s] in _remora::SetText()", pcText);
	}

	// First we need to ask the text sprite drawer what the text will format like, so we can work out if it will fit.
	MS->Format_remora_text(pcText, REMORA_ROW_SPACING, REMORA_CHARACTER_SPACING, nEffectiveWidth);
	nNumRemoraFormattedLines = MS->text_bloc->GetLineInfo()->noOfLines;

	// The Format_remora_text() function sets up the parameter block for the text sprite.  Since this block
	// has to be set up to get the height of a character and since a heading has to be set before anything
	// else, this is a convenient place for the Remora to initialise its character-height variable.
	if (m_nCharacterHeight == 0) {
		psTextParameters = MS->text_bloc->GetParams();
		m_nCharacterHeight = (uint8)MS->text_bloc->CharHeight(psTextParameters->fontResource, psTextParameters->fontResource_hash);

		// To have ceiling based rounding
		m_nPictureHeightCorrection = (u_char)(m_nCharacterHeight - 1);
	}

	// Check if we are setting the main heading for the screen.
	if (bMainHeading) {
		// This is a heading, so we are starting the display afresh.
		m_nNextAvailableRow = 0;

		// First scrolling line to draw is one past the heading.
		m_nFirstLineToDraw = REMORA_FIRST_SCROLLING_LINE;

		// We now have a heading.
		m_bMainHeadingSet = TRUE8;
	} else {
		// Must have set a heading for the screen before any paragraph text can be set.
		if (m_nNextAvailableRow == 0)
			Fatal_error("You cannot set text on the Remora screen until a heading has been set for the screen.");
	}

	// Copy the text into the buffer.
	pcParsePos = pcText;

	for (i = 0; i < nNumRemoraFormattedLines; ++i) {
		// Get length of line.
		nLineLength = MS->text_bloc->GetLineInfo()->line[i].length;

		// Copy one line.
		strncpy(m_pDisplayBuffer[m_nNextAvailableRow].s_pcText, const_cast<char *>(pcParsePos), nLineLength);

		// Make sure we haven't lost the terminator (but I'm sure he'll be back, if we have).
		m_pDisplayBuffer[m_nNextAvailableRow].s_pcText[nLineLength] = '\0';

		// Fill in the attributes for the line.
		m_pDisplayBuffer[m_nNextAvailableRow].s_uPos.s_ePinPosition = ePosition;
		m_pDisplayBuffer[m_nNextAvailableRow].s_nAttribute = nAttribute;
		m_pDisplayBuffer[m_nNextAvailableRow].s_uXY.s_nIndent = nIndent;

		// Move the line count on 1.
		++m_nNextAvailableRow;

		// And move the parse pointer past the line we have just dealt with.
		pcParsePos += nLineLength;

		// And skip any spaces.
		while (*pcParsePos == ' ')
			++pcParsePos;
	}

	// Check if the scroll controls need activating.
	if (m_nNextAvailableRow > REMORA_DISPLAYED_TEXT_ROWS)
		m_bScrollingRequired = TRUE8;
}

void _remora::SetupPicture(uint32 nXPixelOffset, const char *pcPictureName) {
	uint32 i;
	uint32 nPictureHeight, nPictureWidth;
	uint32 nNumPictureRows;

	const char *pcFullPictureNameAndPath;

	// Ignore the function call if the Remora is not active.
	if (m_eGameState == INACTIVE)
		return;

	// Check there is not a picture loaded already.
	if (m_bTextPictureLoaded)
		return;

	// Flag the fact that there is now a picture loaded.
	m_bTextPictureLoaded = TRUE8;

	// Must have set a heading for the screen before anything else can be set.
	if (m_nNextAvailableRow == 0)
		Fatal_error("You cannot put a picture in the Remora until a heading has been set for the screen.");

	// Load the picture.
	pcFullPictureNameAndPath = MakeRemoraGraphicsPath(pcPictureName);
	m_oTextPicture.InitialiseFromBitmapName(pcFullPictureNameAndPath, m_pcRemoraCluster, m_nRemoraClusterHash);

	// Get the size of the image.
	nPictureHeight = m_oTextPicture.GetHeight();
	nPictureWidth = m_oTextPicture.GetWidth();

	// Check it isn't too big.
	if ((nPictureWidth > REMORA_MAX_PICTURE_WIDTH) || (nPictureHeight > REMORA_MAX_PICTURE_HEIGHT))
		Fatal_error("Picture [%s] is %d wide X %d high (maximum is %d X %d)", pcPictureName, nPictureWidth, nPictureHeight, REMORA_MAX_PICTURE_WIDTH,
		            REMORA_MAX_PICTURE_HEIGHT);

	// Need to know how many rows it occupies, so we can leave a gap in the text.
	nNumPictureRows = (nPictureHeight + m_nPictureHeightCorrection) / m_nCharacterHeight;

	// Check it fits in the virtual screen buffer.
	if (m_nNextAvailableRow + nNumPictureRows >= REMORA_TEXT_BUFFER_ROWS)
		Fatal_error("Picture [%s] at row %d will go over the end of the Remora's buffer", pcPictureName, m_nNextAvailableRow);

	for (i = 0; i < nNumPictureRows; ++i) {
		// Fill in the attributes for the line.
		m_pDisplayBuffer[m_nNextAvailableRow].s_uPos.s_nXOffset = nXPixelOffset;
		m_pDisplayBuffer[m_nNextAvailableRow].s_nAttribute = REMORA_TEXT_PICTURE;

		// This is a count maintained in the rows occupied by the picture of how far into the
		// picture we currently are.  Makes working out a Y-draw-height easy later.
		m_pDisplayBuffer[m_nNextAvailableRow].s_uXY.s_nPictureRow = (uint8)i;

		// Move the line count on 1.
		++m_nNextAvailableRow;
	}

	// Check if the scroll controls need activating.
	if (m_nNextAvailableRow > REMORA_DISPLAYED_TEXT_ROWS)
		m_bScrollingRequired = TRUE8;
}

void _remora::AddFloorRange(uint32 nLower, uint32 nUpper) {
	_linked_data_file *pSlices;

	// Check that top value is within the available slices (bottom one must be because it is unsigned).  First,
	// get the pointer to the slices (this will already have been loaded by the line-of-sight engine).
	pSlices = g_oLineOfSight->GetSlicesPointer();

	if (nUpper >= pSlices->Fetch_number_of_items())
		nUpper = pSlices->Fetch_number_of_items() - 1;

	// Upper must be greater than lower, or it isn't a range.
	if (nUpper <= nLower)
		return;

	// And there is a fixed maximum range that can be accommodated.
	if (((nUpper - nLower) + 1) > REMORA_MAX_INCLUDED_SLICES)
		Fatal_error("Range (%d-%d) greater than maximum %d in _remora::AddFloorRange()", nLower, nUpper, REMORA_MAX_INCLUDED_SLICES);

	// Okay, it's a valid range now, so let's add it.
	m_pFloorRanges[m_nNumFloorRangesSet].s_nLower = (uint8)nLower;
	m_pFloorRanges[m_nNumFloorRangesSet].s_nUpper = (uint8)nUpper;

	// Update count of how many we have.
	++m_nNumFloorRangesSet;
}

void _remora::Save(Common::WriteStream *stream) const {
	// Save any outstanding email.  Just write the whole string since it is small and there's only one.
	stream->write(m_pcEmailID, sizeof(char) * (REMORA_MAXLEN_EMAIL_ID_STRING + 1)); // TODO: Refactor to string.
}

void _remora::Restore(Common::SeekableReadStream *stream) {
	// Save any outstanding email.  Just write the whole string since it is small and there's only one.
	if (stream->read(m_pcEmailID, sizeof(char) * (REMORA_MAXLEN_EMAIL_ID_STRING + 1)) != sizeof(char) * (REMORA_MAXLEN_EMAIL_ID_STRING + 1))
		Fatal_error("Error restoring email ID string in _remora::Restore()");

	// Inform the icon menu whether or not there is an email waiting now we have restored.
	if (strlen(m_pcEmailID) > 0)
		g_oIconMenu->SetEmailArrived();
	else
		g_oIconMenu->ClearEmailArrived();
}

void _remora::ProcessUpDownTextKeys(const _input &sKeyboardState) {
	// If the text does not extend over the edge of the screen then there is nothing to do.
	if (!m_bScrollingRequired)
		return;

	// Also do nothing if text is already in process of scrolling.
	if (m_bScrolling)
		return;

	// Check for up and down keys.
	if (sKeyboardState.momentum == __STILL) {
		return;
	} else if (sKeyboardState.momentum == __BACKWARD_1) {
		// Moving down through the text, so text will scroll upwards on the screen.  Check that
		// we can still scroll in this direction.
		if ((m_nFirstLineToDraw + REMORA_DISPLAYED_TEXT_ROWS) < m_nNextAvailableRow) {
			m_nStartYPixelOffset = 0;
			m_eTextScroll = SCROLL_UP;
			m_bScrolling = TRUE8;
		} else {
			m_eTextScroll = SCROLL_NONE;
			m_bScrolling = FALSE8;
		}
	} else {
		// Moving up through the text, so text will scroll down on the screen.  Check that we
		// can still scroll in this direction.
		if (m_nFirstLineToDraw > REMORA_FIRST_SCROLLING_LINE) {
			m_nStartYPixelOffset = 0;
			m_eTextScroll = SCROLL_DOWN;
			m_bScrolling = TRUE8;
		} else {
			m_eTextScroll = SCROLL_NONE;
			m_bScrolling = FALSE8;
		}
	}
}

void _remora::ProcessUpDownZoomKeys(const _input &sKeyboardState) {
	int32 nNewZoom;

	// Check for up and down keys.
	if (sKeyboardState.momentum == __STILL) {
		return;
	} else if (sKeyboardState.momentum == __BACKWARD_1) {
		// Zooming out.
		nNewZoom = m_nCurrentZoom - REMORA_SCAN_ZOOM_STEP;
	} else {
		// Zooming in.
		nNewZoom = m_nCurrentZoom + REMORA_SCAN_ZOOM_STEP;
	}

	// Apply the new zoom value (this function keeps it range-capped to the current limits).
	SetCurrentZoom(nNewZoom);
}

const char *_remora::LocateTextFromReference(uint32 nHashRef) {
	const char *pcTextLine;

	// Look for the reference.
	pcTextLine = (const char *)MS->text->Try_fetch_item_by_hash(nHashRef);

	// If we found it, return it.
	if (pcTextLine)
		return (pcTextLine);

	// Look in the global text file.
	pcTextLine = (const char *)global_text->Try_fetch_item_by_hash(nHashRef);

	// Return the pointer regardless.
	return (pcTextLine);
}

void _remora::DrawScreenText() {
	int32 nStartXPixel, nStartYPixel;
	int32 nVirtualRow, nScreenRow;
	uint8 nRed, nGreen, nBlue;
	uint32 nEffectiveWidth, nTabWidth;
	_rs_params sParams;
	uint32 nBaseY;
	int32 nDisplayedTextRows;

	// We always start drawing at row 0 on the screen.
	nScreenRow = 0;

	// The place we start taking stuff to draw depends on how far down the virtual buffer we are.  If
	// we are scrolling down then we start drawing one line early to make it look smooth.
	switch (m_eTextScroll) {
	case SCROLL_DOWN:
		nVirtualRow = m_nFirstLineToDraw - 1;
		nBaseY = REMORA_TEXT_TOP_MARGIN - m_nCharacterHeight;
		nDisplayedTextRows = REMORA_DISPLAYED_TEXT_ROWS + 2;
		break;

	case SCROLL_UP:
		nVirtualRow = m_nFirstLineToDraw;
		nBaseY = REMORA_TEXT_TOP_MARGIN;
		nDisplayedTextRows = REMORA_DISPLAYED_TEXT_ROWS + 2;
		break;

	default:
		nVirtualRow = m_nFirstLineToDraw;
		nBaseY = REMORA_TEXT_TOP_MARGIN;
		nDisplayedTextRows = REMORA_DISPLAYED_TEXT_ROWS;
		break;
	}

	// Loop for each line in view.
	while ((nScreenRow < nDisplayedTextRows) && (nVirtualRow < m_nNextAvailableRow)) {
		// Work out how far down the screen to start drawing this line
		nStartYPixel = nBaseY + (nScreenRow * m_nCharacterHeight);
		nStartYPixel += (m_nStartYPixelOffset / 256); // scaled down

		// Check to see if it is a picture or text.
		if (m_pDisplayBuffer[nVirtualRow].s_nAttribute & REMORA_TEXT_PICTURE) {
			// We have a picture to display.  If the picture goes off the top of the screen then we
			// need to move the top edge of the picture to make it look like we are displaying the
			// picture from part-way down.
			nStartYPixel -= (m_nCharacterHeight * m_pDisplayBuffer[nVirtualRow].s_uXY.s_nPictureRow);

			// Draw it.
			sParams.bCentre = FALSE8;
			sParams.bUpdate = FALSE8;
			sParams.nW = 0;
			sParams.nH = 0;
			sParams.bAllFrames = TRUE8;
			nStartXPixel = REMORA_TEXT_LEFT_MARGIN + m_pDisplayBuffer[nVirtualRow].s_uPos.s_nXOffset;

			m_oTextPicture.DrawXYSprite(nStartXPixel + REMORA_SCREEN_ORIGIN_X, nStartYPixel + REMORA_SCREEN_ORIGIN_Y, &sParams);

			// Picture gets drawn in one go, so we can simply skip forward past the whole thing now.
			while (m_pDisplayBuffer[nVirtualRow].s_nAttribute & REMORA_TEXT_PICTURE) {
				++nVirtualRow;
				++nScreenRow;
			}
		} else {
			// Displaying a line of text, not a picture.  Check if it's just a blank line.
			if (m_pDisplayBuffer[nVirtualRow].s_nAttribute == 0) {
				++nVirtualRow;
				++nScreenRow;
				continue;
			}

			// Set the colour to draw this text in.
			ColourToRGB(m_pDisplayBuffer[nVirtualRow].s_nAttribute, nRed, nGreen, nBlue);
			SetTextColour(nRed, nGreen, nBlue);

			// The X-coordinate we pin the text to depends on whether or not the text is centred or not.
			if (m_pDisplayBuffer[nVirtualRow].s_uPos.s_ePinPosition == PIN_AT_CENTRE) {
				nStartXPixel = REMORA_TEXT_CENTRE;
				nEffectiveWidth = REMORA_DISPLAY_WIDTH;
			} else {
				// Left-justification must also handle tabs.
				nTabWidth = m_pDisplayBuffer[nVirtualRow].s_uXY.s_nIndent * REMORA_TEXT_TAB_ONE;
				nStartXPixel = REMORA_TEXT_LEFT_MARGIN + nTabWidth;
				nEffectiveWidth = REMORA_DISPLAY_WIDTH - nTabWidth;
			}

			// Draw the line of text.
			MS->Create_remora_text(nStartXPixel, nStartYPixel, m_pDisplayBuffer[nVirtualRow].s_pcText, 0, m_pDisplayBuffer[nVirtualRow].s_uPos.s_ePinPosition,
			                       REMORA_ROW_SPACING, REMORA_CHARACTER_SPACING, nEffectiveWidth);
			MS->Render_speech(MS->text_bloc);
			MS->Kill_remora_text();

			// Moved on on the screen and in the virtual buffer.
			++nVirtualRow;
			++nScreenRow;
		}
	}
}

void _remora::DrawEmailWaiting() {
	uint32 nHashRef;
	const char *pcEmailWaitingText;

	// Check if there is a message waiting and the flash counter is currently on.
	if ((strlen(m_pcEmailID) > 0) && m_bFlashingTextVisible) {
		// Find the text message to display.
		nHashRef = HashString("email_waiting");
		pcEmailWaitingText = LocateTextFromReference(nHashRef);

		MS->Create_remora_text(REMORA_EMAIL_WAITING_X, REMORA_EMAIL_WAITING_Y, pcEmailWaitingText, 0, PIN_AT_TOP_LEFT, 0, 0, REMORA_DISPLAY_WIDTH);
		MS->Render_speech(MS->text_bloc);
		MS->Kill_remora_text();
	}
}

void _remora::ClearAllText() {
	// Ignore the function call if the Remora is not active.
	if (m_eGameState == INACTIVE)
		return;

	// Reset the text control variables.
	m_nNextAvailableRow = 0;
	m_nFirstLineToDraw = 0;

	m_bScrollingRequired = FALSE8;
	m_bScrolling = FALSE8; // these three need reseting so when we go to a new page we don't try scrolling off screen
	m_nStartYPixelOffset = 0;
	m_eTextScroll = SCROLL_NONE;

	m_bTextPictureLoaded = FALSE8;
	m_bMainHeadingSet = FALSE8;

	// These control the spoken text display.
	m_pcSpeechText = NULL;
	m_nSpeechTimer = 0;
}

void _remora::DrawVoiceOverText() const {
	// Here we draw the text that characters may say inside the Remora.
	if (px.on_screen_text && (m_nSpeechTimer > 0) && (strlen(m_pcSpeechText) > 0)) {
		// Yes we need to display speech text.
		MS->Create_remora_text(REMORA_SPEECH_X_POSITION, REMORA_SPEECH_Y_POSITION, m_pcSpeechText, 0, PIN_AT_CENTRE, 0, 0, REMORA_DISPLAY_WIDTH);

		MS->Render_speech(MS->text_bloc);
		MS->Kill_remora_text();
	}
}

void _remora::SetCommonActivateInfo(RemoraMode eMode) {
	uint32 i, j;
	_logic *pPlayerObject;
	_linked_data_file *pSlices;
	_barrier_slice *pSlice;
	int32 nSlice;
	uint32 nNumSlices;
	bool8 bInFloorRange;

	// Set the mode flags.
	m_eGameState = SWITCHING_ON;
	m_eCurrentMode = eMode;
	m_bModeChanged = FALSE8;

	// Cancel the flash counter if it was left running last time.
	m_nScreenFlashCount = 0;

	// Get position of player.  The scanner modes need this information.
	pPlayerObject = MS->logic_structs[MS->player.Fetch_player_id()];
	m_nPlayerY = (int32)pPlayerObject->mega->actor_xyz.y;
	m_nPlayerX = (int32)pPlayerObject->mega->actor_xyz.x;
	m_nPlayerZ = (int32)pPlayerObject->mega->actor_xyz.z;

	// If we are in the special one-off M08 interface or the map mode, the pan is fixed, otherwise we
	// get it from the player.
	if ((m_eCurrentMode == M08_LOCK_CONTROL) || (m_eCurrentMode == MAP))
		m_fPlayerPan = REAL_ZERO;
	else
		m_fPlayerPan = pPlayerObject->pan;

	// Just knowing the y height of the player is now not enough for the scan modes because they may need to
	// include floors and objects from different heights.  Here we work out the slices and range of heights
	// that need including in scan displays.
	pSlices = g_oLineOfSight->GetSlicesPointer();

	// Find out which slice we're in.
	nNumSlices = pSlices->Fetch_number_of_items();
	nSlice = 0;

	for (i = 0; i < nNumSlices; ++i) {
		// Get the slice.
		pSlice = (_barrier_slice *)pSlices->Fetch_item_by_number(i);

		// See if the player's feet are in this slice.
		if ((m_nPlayerY >= pSlice->bottom) && (m_nPlayerY < pSlice->top))
			nSlice = i;
	}

	// Right, we know which slice the player is in.  Check to see if this is in one of the ranges.
	bInFloorRange = FALSE8;
	m_nNumCurrentFloorRanges = 0;
	i = 0;
	while (!bInFloorRange && (i < m_nNumFloorRangesSet)) {
		if ((nSlice >= m_pFloorRanges[i].s_nLower) && (nSlice <= m_pFloorRanges[i].s_nUpper)) {
			// Yes, the slice is contained in a range, so when we do a scan, we need to include everything
			// in the range.
			bInFloorRange = TRUE8;

			for (j = m_pFloorRanges[i].s_nLower; j <= m_pFloorRanges[i].s_nUpper; ++j) {
				m_pSlices[m_nNumCurrentFloorRanges] = (_barrier_slice *)pSlices->Fetch_item_by_number(j);
				m_pnSlices[m_nNumCurrentFloorRanges] = j;
				++m_nNumCurrentFloorRanges;
			}
		}

		++i;
	}

	// If we didn't set a floor range then we must set a single floor slice.
	if (!bInFloorRange) {
		// Only one slice required to be displayed.
		m_pSlices[0] = (_barrier_slice *)pSlices->Fetch_item_by_number(nSlice);
		m_pnSlices[0] = nSlice;
		m_nNumCurrentFloorRanges = 1;
	}

	// Set an absolute floor and ceiling.
	m_nIncludedFloor = (int32)m_pSlices[0]->bottom;
	m_nIncludedCeiling = (int32)m_pSlices[m_nNumCurrentFloorRanges - 1]->top;

	// Clear the text buffer.
	ClearAllText();
}

void _remora::AccessMenuLevelVariables(int32 *pnParams, MenuVariableAccessMode eRetrieve) {
	uint32 i, j;
	c_game_object *pGameObject;
	char pcVarName[] = REMORA_MENU_LEVEL_NAME;
	uint32 nDigitPos;

	// Get the Remora's game object.
	pGameObject = (c_game_object *)MS->objects->Fetch_item_by_name(REMORA_NAME);

	// Get the position where we need to add the digit to the menu variable name.
	nDigitPos = strlen(pcVarName) - 1;

	// System currently supports 5 menu levels.  We must set these variables.
	for (i = 0; i < REMORA_MENU_DEPTH; ++i) {
		// Make the name of the variable.
		pcVarName[nDigitPos] = (char)(i + '1');

		// Find the variable in the Remora's game object.
		j = 0;
		while ((j < pGameObject->GetNoLvars()) && strcmp(pcVarName, pGameObject->GetScriptVariableName(j)))
			++j;

		// If we ran out of variables, this is an error because we haven't found the one we're looking for.
		if (j == pGameObject->GetNoLvars())
			Fatal_error("Failed to find menu variable %s in _remora::AccessMenuLevelVariables()", pcVarName);

		// Found it, so get or set it.
		if (eRetrieve == GET)
			pnParams[i] = pGameObject->GetIntegerVariable(j);
		else
			pGameObject->SetIntegerVariable(j, pnParams[i]);
	}
}

_remora::ScreenSymbol _remora::GetSymbolToDrawObject(_logic *pObject, uint32 nID) const {
	__object_type eObjectType;
	c_game_object *pGameObject;
	uint32 nScriptVar, nVarVal;

	// If it's player, always return same symbol.
	if (nID == MS->player.Fetch_player_id())
		return (SS_REMORA);

	// Get the type field stored in the object.
	eObjectType = pObject->object_type;

	// Decide what colour it should be.
	switch (eObjectType) {
	case (__BUTTON_OPERATED_DOOR):
		// In the case of doors, we must work out whether or not they are closed.
		if (pObject->list[BOD_STATE_INDEX])
			return (DOOR_CLOSED);
		else
			return (DOOR_OPEN);
		break;

	case (__ORGANIC_MEGA):
		// Need to find out if the human is alive or dead.
		pGameObject = (c_game_object *)MS->objects->Fetch_item_by_number(nID);
		nScriptVar = pGameObject->GetVariable("state");
		nVarVal = pGameObject->GetIntegerVariable(nScriptVar);
		if (nVarVal == 1)
			return (DEAD_HUMAN);
		else
			return (ALIVE_HUMAN);

		break;

	case (__NON_ORGANIC_MEGA):
		// Need to find out if the robot is alive or dead.
		pGameObject = (c_game_object *)MS->objects->Fetch_item_by_number(nID);
		nScriptVar = pGameObject->GetVariable("state");
		nVarVal = pGameObject->GetIntegerVariable(nScriptVar);
		if (nVarVal == 1)
			return (DEAD_ROBOT);
		else
			return (ALIVE_ROBOT);

		break;

	case (__REMORA_CARRIER):
		// This is an object carrying a Remora, but only the player gets a special symbol now.
		pGameObject = (c_game_object *)MS->objects->Fetch_item_by_number(nID);
		nScriptVar = pGameObject->GetVariable("state");
		nVarVal = pGameObject->GetIntegerVariable(nScriptVar);
		if (nVarVal == 1)
			return (DEAD_HUMAN);
		else
			return (ALIVE_HUMAN);
		break;

	case (__RECHARGE_POINT):
		pGameObject = (c_game_object *)MS->objects->Fetch_item_by_number(nID);
		nScriptVar = pGameObject->GetVariable("set_mine");
		nVarVal = pGameObject->GetIntegerVariable(nScriptVar);
		if (nVarVal == 1)
			return (RECHARGE_ARMED);
		else
			return (RECHARGE_UNARMED);
		break;

	case (__AUTO_DOOR):
		// In the case of doors, we must work out whether or not they are closed.
		if (pObject->list[CAD_STATE_INDEX])
			return (DOOR_CLOSED);
		else
			return (DOOR_OPEN);
		break;

	default:
		// If the object has no type set then don't display it.
		return (DO_NOT_DISPLAY);
	}
}

void _remora::BuildM08DoorList() {
	m_pnDoorIDs[0] = MS->Fetch_named_objects_id("door_corridor1_to_interorgation");
	m_pnDoorIDs[1] = MS->Fetch_named_objects_id("cell_door");
	m_pnDoorIDs[2] = MS->Fetch_named_objects_id("door_corridor1_to_liftstart");
	m_pnDoorIDs[3] = MS->Fetch_named_objects_id("door_corridor1_to_security");
	m_pnDoorIDs[4] = MS->Fetch_named_objects_id("door_cell_to_lukyan");
	m_pnDoorIDs[5] = MS->Fetch_named_objects_id("door_security_to_lukyan");
	m_pnDoorIDs[6] = MS->Fetch_named_objects_id("door_corridor2_to_security");
	m_pnDoorIDs[7] = MS->Fetch_named_objects_id("door_security_to_exitlift");
	m_pnDoorIDs[8] = MS->Fetch_named_objects_id("door_security_to_doorcontrol");
	m_pnDoorIDs[9] = MS->Fetch_named_objects_id("door_corridor2_to_doorcontrol");
	m_pnDoorIDs[10] = MS->Fetch_named_objects_id("door_exitlift_to_mainlift_sec");
	m_pnDoorIDs[11] = MS->Fetch_named_objects_id("door_robot");
}

} // End of namespace ICB
