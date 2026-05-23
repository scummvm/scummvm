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
#include "engines/nancy/enginedata.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;

namespace UI {

// Nancy 10+ cell phone popup, driven by the UICL chunk.
// TODO: email, search, help, browser modes; in-call menu; redial.
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

	// Swaps the welcome graphic for the No Signal / No Access / Old Email
	// Only labels and blocks outgoing calls. TODO: hook to scene flag.
	void setNoSignal(bool noSignal);

private:
	enum ScreenState : int {
		kWelcome          = 0,
		kDialing          = 1,
		kPlaceCall        = 2,
		kWaitOutgoingRing = 3,
		kLookupContact    = 4,
		kWaitPickup       = 5,
		kConnected        = 6,
		kInvalidNumber    = 7,
		kWaitInvalid      = 8,
		kDirectory        = 9
	};

	void drawChrome();
	void drawScreenContent();
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
	void drawWelcomeScreen();
	void drawBackLabel();

	void resetDialPad();
	void enterScreenState(ScreenState newState);
	void appendDigit(byte slotIndex);
	bool playSoundIfPresent(const Common::Path &soundName);
	bool callSoundIsStillPlaying() const;
	void triggerContactCallSceneChange(uint contactIndex);
	int findContactByDialBuffer() const;

	uint maxDirectoryRows() const;
	uint directoryRowAt(const Common::Point &chunkMouse) const;
	Common::Rect directoryRowRect(uint visibleIndex) const;
	void startCallToContact(uint contactIndex);
	// Visible (deduplicated) row -> raw contact index, or -1.
	int contactIndexForVisibleRow(uint visibleRow) const;
	uint deduplicatedContactCount() const;
	// True when the contact's visibility flag is currently unlocked.
	bool isContactVisible(const UICL::Contact &c) const;
	// Popup-local rect of the Back hotspot in directory mode.
	Common::Rect backLabelHitRect() const;
	// Move the directory selection by delta, scrolling as needed.
	void moveDirectorySelection(int delta);

	Common::Point mouseToChunkCoords(const Common::Point &mouse) const;

	const UICL *_uiclData;

	// Chrome (header.imageName) and sprite atlas (overlayImageName).
	Graphics::ManagedSurface _overlayImage;
	Graphics::ManagedSurface _spritesImage;

	bool _closeButtonHovered = false;

	ScreenState _screenState = kWelcome;

	// Dialed digits as '0'..'9' chars; convert with `c - '0'` to get
	// the slot index that matches a contact's dial prefix.
	Common::String _dialedNumber;

	SoundDescription _callSound;

	// Resolved during kLookupContact, valid through kWaitPickup; -1 = miss.
	int _resolvedContact = -1;

	int _hoveredSlot = -1;

	// First visible deduplicated contact, and the active row within the page.
	uint _directoryScroll = 0;
	uint _directorySelection = 0;

	bool _noSignal = false;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_CELLPHONEPOPUP_H
