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
	// Only labels and blocks outgoing calls.
	void setNoSignal(bool noSignal);

	// Swaps the battery sprite for the low/dead variant.
	void setBatteryLow(bool low);

	// Insert or replace a contact (matched by its 11-byte dial pattern).
	// Used by AR 130 to add/modify entries at runtime.
	void upsertContact(const UICL::Contact &c);

	// Append an entry to the search/email results list (mode 0) or the
	// web bookmarks list (mode 1). Driven by AR 131 (AddSearchLink).
	void addSearchLink(int16 mode, const Common::String &key,
						const Common::String &value, int16 extra,
						int16 flag, int16 eventFlag);

	// Phone-call return scene. Set before jumping into a conversation scene
	// so AR 128 (CellPhonePopCellSceneFromStack) can return there without
	// trampling the global push slot used by closeups/inventory views.
	void setReturnScene(const SceneChangeDescription &scene);
	bool consumeReturnScene(SceneChangeDescription &out);

	// Start an incoming-call sequence: opens the popup, stores the
	// destination scene, and joins the kPlaceCall state chain so it
	// rings, picks up, shows the connecting sprite, and changeScenes
	// into `scene` (AR 128 returns via the setReturnScene slot).
	void startIncomingCall(const SceneChangeDescription &scene);

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
		kDirectory        = 9,
		kOnlineHub        = 10,  // Online heading + Email / Web sub-buttons
		kWebList          = 11,  // web search-results list (AR-131 mode 1)
		kEmailList        = 12,  // email message list (AR-131 mode 0)
		kContentView      = 13   // full-text view of a single email / page
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
	void drawHelpButton(uint state);
	void drawCloseButton(bool hovered);
	void drawStatusLabels();
	void drawDirectoryList();
	void drawDirectoryArrows();
	void drawWelcomeScreen();
	// Blit a sub-button's idle sprite at its chunk dest (used for the visible
	// Back buttons: subButtons[0] on the help / directory / online screens,
	// subButtons[7] in the zoomed email / browser content view).
	void drawBackButton(uint subButtonIndex);
	// Blit the lit key sprite of the currently held dial-pad slot over its
	// dest rect, so keypad keys visually depress while pressed.
	void drawPressedDialKey();

	// Generic list renderer used by web / email modes.
	void drawLinkList();
	// Blit a heading sprite (e.g. emailHeading) at its chunk dest. The
	// heading sits in the title-bar strip above the LCD content.
	void drawHeading(const UICL::SrcDestRectPair &heading);
	// Render the opened entry's body text in the LCD area, word-wrapped.
	void drawContentView();
	// Enter the content view for a list entry whose AUTOTEXT key is `key`.
	void openContentView(const Common::String &key, const UICL::SrcDestRectPair &heading);
	// Web button: open the first url entry as the browser home page (page 0).
	void openBrowserHome();

	// Up/down scroll buttons differ by mode: subButtons[1]/[2] for the
	// directory's narrower list area, [5]/[6] for the taller search /
	// email / browser-content LCD area.
	const UICL::ThreeRectWidget &scrollUpButton() const;
	const UICL::ThreeRectWidget &scrollDownButton() const;

	// True when the current screen uses the zoomed-in (no-keypad)
	// chrome variant: search list, email list, and content view.
	bool isZoomedChromeState() const {
		return _screenState == kWebList ||
				_screenState == kEmailList ||
				_screenState == kContentView;
	}

	// The help "?" page reuses the content-view state, but unlike browser /
	// email articles the original renders it in the regular (keypad-visible)
	// chrome and the small LCD, not the zoomed full-screen variant.
	bool isHelpContentView() const {
		return _screenState == kContentView && _uiclData &&
				_contentHeading == &_uiclData->helpHeading;
	}

	// True for screens that hide the status icons and "?" button so the
	// top bar shows only the section heading and the up arrow.
	bool isSubScreenState() const {
		return _screenState == kDirectory ||
				_screenState == kOnlineHub ||
				isZoomedChromeState();
	}

	void resetDialPad();
	void enterScreenState(ScreenState newState);
	void appendDigit(byte slotIndex);
	// Play a dial-pad key's DTMF tone. The name is a raw sound filename, so it
	// is played through the phone's call-sound channel rather than the common
	// (boot-registered) sound table.
	void playDialPadSound(const Common::String &name);
	// Play a popup button's click sound (the close X), like the inventory
	// popup. Falls back to the shared button-click slot in the popup header.
	void playButtonClickSound(const UIButtonRecord &button);
	bool playSoundIfPresent(const Common::Path &soundName);
	bool callSoundIsStillPlaying() const;
	void triggerContactCallSceneChange(uint contactIndex);
	int findContactByDialBuffer() const;

	uint maxDirectoryRows() const;
	uint directoryRowAt(const Common::Point &chunkMouse) const;
	Common::Rect directoryRowRect(uint visibleIndex) const;
	// Row pitch and first-row Y (screen coords) from the layout data.
	int rowPitch() const;
	int rowTopScreen() const;
	bool isLinkListMode() const { return _screenState == kWebList || _screenState == kEmailList; }
	bool isOnlineMode() const { return _screenState == kOnlineHub || isLinkListMode(); }

	// Section headings live in the phone's title-bar strip above the LCD,
	// so they don't consume a list row. Kept as a hook in case a game
	// needs an in-LCD title row.
	uint listTitleRows() const { return 0; }

	// Layout for the two clickable labels on the Online hub.
	Common::Rect hubEmailRect() const;
	Common::Rect hubWebRect() const;
	void startCallToContact(uint contactIndex);
	// Visible (deduplicated) row -> raw contact index, or -1.
	int contactIndexForVisibleRow(uint visibleRow) const;
	uint deduplicatedContactCount() const;
	// Entry count for whichever list the popup is currently showing.
	uint currentListEntryCount() const;
	// Absolute indices into the current list's backing array that pass
	// the active filter. Email applies the "Old Email Only" filter
	// (no-signal -> read messages only); web shows everything.
	Common::Array<uint> listVisibleIndices() const;
	// True when the contact's visibility flag is currently unlocked.
	bool isContactVisible(const UICL::Contact &c) const;
	// Popup-local rect of the Back hotspot in directory mode.
	Common::Rect backLabelHitRect() const;
	// Popup-local rect of a visible Back sub-button (subButtons[index]).
	Common::Rect backButtonHitRect(uint subButtonIndex) const;
	// Move the directory selection by delta, scrolling as needed.
	void moveDirectorySelection(int delta);

	Common::Point mouseToChunkCoords(const Common::Point &mouse) const;

	const UICL *_uiclData;

	// Runtime contact list, seeded from _uiclData->contacts and then
	// mutable (AR 130 inserts/replaces entries).
	Common::Array<UICL::Contact> _contacts;

	// Chrome (header.imageName) and sprite atlas (overlayImageName).
	Graphics::ManagedSurface _overlayImage;
	Graphics::ManagedSurface _spritesImage;

	bool _closeButtonHovered = false;
	bool _scrollUpHovered = false;
	bool _scrollDownHovered = false;

	ScreenState _screenState = kWelcome;

	// Dialed digits as '0'..'9' chars; convert with `c - '0'` to get
	// the slot index that matches a contact's dial prefix.
	Common::String _dialedNumber;

	SoundDescription _callSound;

	// Resolved during kLookupContact, valid through kWaitPickup; -1 = miss.
	int _resolvedContact = -1;

	int _hoveredSlot = -1;

	// Dial-pad slot currently held down (shows the lit / depressed key), or -1.
	int _pressedSlot = -1;

	// A call queued by auto-dial / Talk, waiting for the key's DTMF tone to
	// finish before entering kPlaceCall (see updateGraphics).
	bool _autoDialPending = false;

	// First visible deduplicated contact, and the active row within the page.
	uint _directoryScroll = 0;
	uint _directorySelection = 0;

	// Content-view (single email / page) state.
	ScreenState _contentReturnState = kOnlineHub;
	const UICL::SrcDestRectPair *_contentHeading = nullptr;
	Common::String _contentKey;
	uint _contentScroll = 0;

	// In-page hyperlinks: rects (popup-local, recomputed every draw) and
	// the target CVTX key parsed from each <H>...<L> region of the body.
	Common::Array<Common::Rect> _contentHotspots;
	Common::Array<Common::String> _contentHotspotTargets;

	bool _noSignal = false;
	bool _batteryLow = false;

	// Incoming-call destination (set by startIncomingCall, consumed by the
	// kConnected handler once the player has answered).
	SceneChangeDescription _pendingCallScene;
	bool _hasPendingCallScene = false;

	SceneChangeDescription _returnScene;
	bool _hasReturnScene = false;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_CELLPHONEPOPUP_H
