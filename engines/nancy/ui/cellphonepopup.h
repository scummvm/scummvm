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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_UI_CELLPHONEPOPUP_H
#define NANCY_UI_CELLPHONEPOPUP_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/renderobject.h"
#include "engines/nancy/sound.h"

namespace Nancy {

struct NancyInput;
struct UICL;

namespace UI {

// Nancy 10+ cell phone popup driven by the UICL chunk.
//
// Implemented modes: welcome screen, call-placement (dial → ring →
// lookup → pickup → connect → invalid), directory list, no-signal
// overlay.
//
// TODO: email, contact search, help, and browser modes are not
// implemented; web/email/redial sub-buttons and the in-call menu
// are unhooked.
class CellPhonePopup : public RenderObject {
public:
	CellPhonePopup();
	~CellPhonePopup() override = default;

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	void open();
	void close();
	void toggle() { if (_isVisible) close(); else open(); }

	// Replaces the welcome graphic with the No Signal / No Access /
	// Old Email Only labels and disables outgoing calls.
	// TODO: hook this up to the scene event flag that drives it in
	// the original engine.
	void setNoSignal(bool noSignal);

private:
	enum ScreenState : int {
		kWelcome          = 0,  // idle / dial-pad accepting input
		kDialing          = 1,  // digit accumulating
		kPlaceCall        = 2,  // start outgoing-ring sound
		kWaitOutgoingRing = 3,
		kLookupContact    = 4,  // match dial buffer, start pickup
		kWaitPickup       = 5,
		kConnected        = 6,  // trigger contact scene change
		kInvalidNumber    = 7,  // start invalid-number sound
		kWaitInvalid      = 8,
		kDirectory        = 9   // contact list, scrollable
	};

	// Drawing helpers
	void drawChrome();              // popup overlay + persistent labels + buttons
	void drawScreenContent();       // LCD area: dispatches on _screenState
	void drawStatusIcons();
	void drawWebDirLabels();
	void drawDialLabel();
	void drawTypeMessage();
	void drawConnectedLabel();
	void drawConnectingSprite();
	void drawDialedNumber();
	void drawCallButtonState(uint state);
	void drawCloseButton(uint state);
	void drawStatusLabels();
	void drawDirectoryList();
	void drawDirHeading();
	void drawDirectoryArrows();

	// State machine helpers
	void resetDialPad();
	void enterScreenState(ScreenState newState);
	void appendDigit(byte slotIndex);
	bool playSoundIfPresent(const Common::Path &soundName);
	bool callSoundIsStillPlaying() const;
	void triggerContactCallSceneChange(uint contactIndex);
	int findContactByDialBuffer() const;

	// Directory helpers
	uint maxDirectoryRows() const;
	uint directoryRowAt(const Common::Point &chunkMouse) const;
	Common::Rect directoryRowRect(uint visibleIndex) const;
	void startCallToContact(uint contactIndex);
	// Visible (deduplicated) row -> raw contact index, or -1.
	int contactIndexForVisibleRow(uint visibleRow) const;
	// Total deduplicated contacts in the directory list.
	uint deduplicatedContactCount() const;

	// Mouse hit-test helpers
	Common::Point mouseToChunkCoords(const Common::Point &mouse) const;

	const UICL *_uiclData;

	// Chrome image (header.imageName).
	Graphics::ManagedSurface _overlayImage;
	// Sprite atlas (overlayImageName). Source for every non-chrome
	// blit: status icons, labels, headings, call/close buttons.
	Graphics::ManagedSurface _spritesImage;

	bool _closeButtonHovered = false;

	ScreenState _screenState = kWelcome;

	// '0'..'9' chars of the digits the player has dialed. Convert with
	// `c - '0'` to recover the slot index that the contact prefix stores.
	Common::String _dialedNumber;

	// Active dial-tone / ring / pickup / invalid-number cue. The
	// original engine reuses one channel for all of them.
	SoundDescription _callSound;

	// Contact index resolved during kLookupContact, valid through
	// kWaitPickup; -1 means the number didn't match.
	int _resolvedContact = -1;

	// Dial-pad slot under the cursor, or -1.
	int _hoveredSlot = -1;

	// Index of the first deduplicated contact rendered in directory mode.
	// Updated by the up/down arrow sub-buttons.
	uint _directoryScroll = 0;

	// When true, the LCD shows the status labels and outgoing calls
	// are blocked. See setNoSignal().
	bool _noSignal = false;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_CELLPHONEPOPUP_H
