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

#include "common/algorithm.h"
#include "common/system.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/misc/hypertext.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/taskbar.h"
#include "engines/nancy/ui/viewport.h"

#include "graphics/surface.h"

#include "engines/nancy/ui/cellphonepopup.h"

namespace Nancy {
namespace UI {

// Nancy 13 LCD tiles in the UI_Cell_Xtra atlas. The original composes the LCD
// background into a pre-rendered per-state surface; these are the fixed source
// rects for the plain keyboard background, the camera framing box, and the five
// message screens. The grid is anchored to the two rects stored in the chunk
// (the "[<?>]" help and "[<->]" framing tiles): 171x164 tiles, 173px column
// pitch, 166px row pitch.
static constexpr Common::Rect kN13PlainBg(Common::Point(174, 187), 345 - 174, 351 - 187);
static constexpr Common::Rect kN13MsgWelcome(Common::Point(1, 353), 172 - 1, 517 - 353);
static constexpr Common::Rect kN13MsgPictureSent(Common::Point(174, 353), 345 - 174, 517 - 353);
static constexpr Common::Rect kN13MsgNoPictures(Common::Point(347, 353), 518 - 347, 517 - 353);
static constexpr Common::Rect kN13MsgCameraFull(Common::Point(1, 519), 172 - 1, 683 - 519);
static constexpr Common::Rect kN13MsgDeleteConfirm(Common::Point(174, 519), 345 - 174, 683 - 519);
static constexpr Common::Rect kN13MsgPictureDeleted(Common::Point(347, 519), 518 - 347, 683 - 519);
// The two-phones "connecting" LCD graphic, drawn over the plain background while
// a call is being placed. (This is the rect the chunk stores as the idle sprite
// source; the "Welcome / River Heights" tile is kN13MsgWelcome.)
static constexpr Common::Rect kN13CallGraphic(Common::Point(347, 187), 518 - 347, 351 - 187);

// Nancy 13 online sub-button roles (index = Ghidra widget id − 0x10). Nancy
// 10-12 use a different 10-button layout, so these apply only when the game is
// Nancy 13 or newer. Widget 0x10 (the Back button) is the extra one Nancy 13
// prepended to the array.
enum {
	kN13SubBack     = 0,   // small-LCD Back (directory / menu / picture view)
	kN13SubDirUp    = 1,
	kN13SubDirDown  = 2,
	kN13SubViewPics = 3,   // Menu "View Pictures" option (widget 0x13 -> state 0xb)
	kN13SubEmail    = 4,   // Menu "E-mail / Messaging" option (widget 0x14)
	kN13SubBrowser  = 5,   // Menu "Internet Browser" option (widget 0x15, removed)
	kN13SubListUp   = 6,   // zoomed list / picture-view paging
	kN13SubListDown = 7,
	kN13SubBackFull = 8    // Back at the bottom of a zoomed (full-screen) list
};

// Nancy 10-12 online sub-button roles (the 10-button layout, before Nancy 13
// prepended a dedicated Back button and reshuffled the rest).
enum {
	kSubBack      = 0,   // small-LCD Back (directory / online hub)
	kSubDirUp     = 1,   // directory / help scroll up
	kSubDirDown   = 2,   // directory / help scroll down
	kSubEmail     = 3,   // hub "E-mail / Messaging" option
	kSubWeb       = 4,   // hub "Internet Browser" option
	kSubListUp    = 5,   // zoomed list scroll up
	kSubListDown  = 6,   // zoomed list scroll down
	kSubEmailBack = 7,   // email list / zoomed content Back (to the hub)
	kSubSearch    = 8,   // browser-page SEARCH button
	kSubWebHome   = 9    // web search-list HOME (to the browser homepage)
};

// Contacts are shown alphabetically by name (case-insensitive), matching
// the original's CCellPhoneSortContacts.
static bool contactNameLess(const UICL::Contact &a, const UICL::Contact &b) {
	return a.name.compareToIgnoreCase(b.name) < 0;
}

// Renders the engine's hypertext markup (colour / formatting tags) into a
// scratch surface, which the content view then blits into the LCD. Thin
// wrapper that exposes HypertextParser's protected rendering entry points.
class CellPhoneHypertext : public Misc::HypertextParser {
public:
	// Expose the inherited per-page image hooks so the popup can register
	// the UIBW image table before render() is called.
	using Misc::HypertextParser::setImageName;
	using Misc::HypertextParser::addImage;

	void render(uint width, uint height, uint32 transColor,
				const Common::String &text, uint fontID) {
		initSurfaces(width, height, g_nancy->_graphics->getInputPixelFormat(),
						transColor, transColor);
		_fullSurface.setTransparentColor(transColor);
		addTextLine(text);

		const Font *font = g_nancy->_graphics->getFont(fontID);
		const TBOX *tbox = GetEngineData(TBOX);
		Common::Rect textBounds(0, 0, (int16)width, (int16)height);
		const uint d = font ? (font->getFontHeight() + 1) / 2 + 1 : 0;
		textBounds.left += d;
		textBounds.top += d + 1;
		const int leftOffset = tbox ? (int)tbox->leftOffset - textBounds.left : 0;
		drawAllText(textBounds, (uint)MAX(0, leftOffset), fontID, fontID);
	}

	const Graphics::ManagedSurface &surface() const { return _fullSurface; }
	uint16 textHeight() const { return _drawnTextHeight; }
	const Common::Array<Common::Rect> &hotspots() const { return _hotspots; }
};

// Chunk destRects are raw screen coords; subtract _screenPosition.topLeft
// to get popup-local. srcRects are atlas-image coords for _spritesImage
// and pass through unchanged.

CellPhonePopup::CellPhonePopup() :
		RenderObject(12),
		_uiclData(nullptr) {}

void CellPhonePopup::init() {
	_uiclData = GetEngineData(UICL);
	assert(_uiclData);

	// Chrome image; the sprite atlas is loaded separately below.
	g_nancy->_resource->loadImage(_uiclData->header.imageName, _overlayImage);

	if (!_uiclData->overlayImageName.empty()) {
		g_nancy->_resource->loadImage(_uiclData->overlayImageName, _spritesImage);
	} else if (_uiclData->header.secondaryButtonEnabled &&
				!_uiclData->header.secondaryButton.primaryImageName.empty()) {
		g_nancy->_resource->loadImage(_uiclData->header.secondaryButton.primaryImageName,
										_spritesImage);
	}

	Common::Rect popupRect = _uiclData->header.normalDestRect;
	if (_uiclData->header.overlayInGameFrame) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			popupRect.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	moveTo(popupRect);

	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getScreenPixelFormat());

	// Persistent state lives in CellPhoneData (saved across the game).
	// First-time init seeds the runtime contact list from the chunk;
	// subsequent inits (e.g. after a load) restore the saved state.
	CellPhoneData *cellData = (CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (cellData) {
		if (!cellData->seeded) {
			cellData->contacts = _uiclData->contacts;
			cellData->seeded = true;

			// The UICL chunk can ship one initial email and one initial
			// web-search entry, populated at new-game start (an empty key
			// means none). addSearchLink appends into cellData's lists.
			if (!_uiclData->initialEmail.key.empty()) {
				addSearchLink(0, _uiclData->initialEmail);
			}
			if (!_uiclData->initialSearch.key.empty()) {
				addSearchLink(1, _uiclData->initialSearch);
			}
		}
		_contacts = cellData->contacts;
		_noSignal = cellData->noSignal;
		_batteryLow = cellData->batteryLow;
	} else {
		_contacts = _uiclData->contacts;
	}

	// Keep the list sorted, including contacts restored from older saves that
	// predate the alphabetical ordering.
	Common::sort(_contacts.begin(), _contacts.end(), contactNameLess);

	_screenState = kWelcome;
	_dialedNumber.clear();
	_resolvedContact = -1;
	_directoryScroll = 0;
	_directorySelection = 0;

	drawChrome();
	drawScreenContent();

	setTransparent(false);
	setVisible(false);

	RenderObject::init();
}

void CellPhonePopup::registerGraphics() {
	RenderObject::registerGraphics();
}

void CellPhonePopup::setNoSignal(bool noSignal) {
	_noSignal = noSignal;
	CellPhoneData *cellData = (CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (cellData) {
		cellData->noSignal = noSignal;
	}
	if (_isVisible) {
		drawScreenContent();
	}
}

void CellPhonePopup::setBatteryLow(bool low) {
	_batteryLow = low;
	CellPhoneData *cellData = (CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (cellData) {
		cellData->batteryLow = low;
	}
	if (_isVisible) {
		drawScreenContent();
	}
}

void CellPhonePopup::addSearchLink(int16 mode, const SearchLink &link) {
	CellPhoneData *cellData = (CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (!cellData) {
		return;
	}

	// Original (AddSearchLink @ 004dac11) branches on `mode == 0` (email)
	// vs anything else (search) — not specifically mode == 1.
	const bool isSearch = (mode != 0);
	Common::Array<SearchLink> &list =
		isSearch ? cellData->searchLinks : cellData->emailMessages;

	// Skip duplicates (matched by key) so re-running the scene doesn't
	// pile up the same entries.
	for (uint i = 0; i < list.size(); ++i) {
		if (list[i].key.equalsIgnoreCase(link.key)) {
			return;
		}
	}

	list.push_back(link);

	if (_isVisible &&
			((isSearch && _screenState == kWebList) ||
			 (!isSearch && _screenState == kEmailList))) {
		drawScreenContent();
	}
}

void CellPhonePopup::upsertContact(const UICL::Contact &c) {
	// Match against the 11-byte dial pattern (prefix[2..12]). If an entry
	// already carries that pattern, overwrite it; otherwise append.
	bool replaced = false;
	for (uint i = 0; i < _contacts.size(); ++i) {
		if (memcmp(_contacts[i].unknownPrefix + 2,
					c.unknownPrefix + 2, 11) == 0) {
			_contacts[i] = c;
			replaced = true;
			break;
		}
	}
	if (!replaced) {
		_contacts.push_back(c);
	}

	Common::sort(_contacts.begin(), _contacts.end(), contactNameLess);

	CellPhoneData *cellData = (CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (cellData) {
		cellData->contacts = _contacts;
	}

	if (_isVisible && _screenState == kDirectory) {
		drawScreenContent();
	}
}

void CellPhonePopup::open() {
	if (_isVisible) {
		return;
	}

	// Re-pull persistent state in case a save was loaded after init().
	CellPhoneData *cellData = (CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (cellData && cellData->seeded) {
		_contacts = cellData->contacts;
		_noSignal = cellData->noSignal;
		_batteryLow = cellData->batteryLow;
		Common::sort(_contacts.begin(), _contacts.end(), contactNameLess);
	}

	_screenState = kWelcome;
	_dialedNumber.clear();
	_resolvedContact = -1;
	_directoryScroll = 0;
	_directorySelection = 0;
	_closeButtonHovered = false;
	_scrollUpHovered = false;
	_scrollDownHovered = false;
	_helpButtonHovered = false;
	_backButtonHovered = false;
	_autoDialPending = false;
	_pressedSlot = -1;

	drawChrome();
	drawScreenContent();
	setVisible(true);

	g_nancy->_cursor->warpCursor(Common::Point(_screenPosition.left + _screenPosition.width() / 2,
												_screenPosition.top + _screenPosition.height() / 2));

	// Badge sub-categories clear when their list is viewed (enterScreenState),
	// not on open.

	if (!_uiclData->header.sounds[0].name.empty()) {
		g_nancy->_sound->loadSound(_uiclData->header.sounds[0]);
		g_nancy->_sound->playSound(_uiclData->header.sounds[0]);
	}
}

void CellPhonePopup::startIncomingCall(const SceneChangeDescription &scene) {
	// open() resets state, so save the pending scene afterwards. Joining
	// kPlaceCall hands off to the existing ring / pickup / connect chain;
	// kLookupContact skips the contact lookup when _hasPendingCallScene
	// is set, and kConnected uses the stored scene for changeScene.
	if (!_isVisible) {
		open();
	}
	_pendingCallScene = scene;
	_hasPendingCallScene = true;
	_resolvedContact = -1;
	resetDialPad();
	enterScreenState(kPlaceCall);
}

void CellPhonePopup::close() {
	if (!_isVisible)
		return;

	if (!_callSound.name.empty()) {
		g_nancy->_sound->stopSound(_callSound);
	}

	// Closing the phone while ringing declines the call.
	_hasPendingCallScene = false;
	_autoDialPending = false;
	_pressedSlot = -1;

	setVisible(false);
}

void CellPhonePopup::endCall() {
	if (_callWasIncoming) {
		// Incoming call: take the phone down.
		_callWasIncoming = false;
		close();
		return;
	}

	// Player-placed call: return to the welcome screen and stay open.
	_callWasIncoming = false;
	if (!_isVisible) {
		return;
	}
	_screenState = kWelcome;
	_dialedNumber.clear();
	_resolvedContact = -1;
	_autoDialPending = false;
	_pressedSlot = -1;
	drawScreenContent();
}

void CellPhonePopup::updateGraphics() {
	if (!_isVisible) {
		return;
	}

	// Finish the email "opening" flash: once the brief delay elapses, open the
	// message body (the closed→open envelope flash showed on the list in the
	// meantime).
	if (_openingEmailRow != -1 && g_system->getMillis() >= _openingEmailTime) {
		const Common::String key = _openingEmailKey;
		_openingEmailRow = -1;
		_openingEmailKey.clear();
		openContentView(key, _uiclData->emailHeading);
		return;
	}

	// A queued auto-dial / Talk waits for the key's DTMF tone to finish so the
	// last digit stays audible before the outgoing-ring sound takes over the
	// shared call-sound channel.
	if (_autoDialPending) {
		if (!callSoundIsStillPlaying()) {
			_autoDialPending = false;
			enterScreenState(kPlaceCall);
		}
		return;
	}

	// Per-frame state advancement. Only the call-progress states below drive
	// themselves (via ring/pickup sounds and timers); the interactive screens
	// (welcome, directory, online hub, lists, content, camera, ...) stay put
	// until the user acts, so they do nothing here.
	switch (_screenState) {
	case kWelcome:
	case kDialing:
	case kDirectory:
	case kOnlineHub:
	case kWebList:
	case kEmailList:
	case kContentView:
	case kCamera:
	case kPictureView:
	case kDeleteConfirm:
	case kMessageScreen:
		break;

	case kPlaceCall:
		// Incoming calls skip the dial-out ring entirely (it's a manual-dial
		// cue); they go straight to the pickup/connect step.
		if (!_hasPendingCallScene && playSoundIfPresent(_uiclData->outgoingRingSound)) {
			enterScreenState(kWaitOutgoingRing);
		} else {
			enterScreenState(kLookupContact);
		}
		break;

	case kWaitOutgoingRing:
		if (!callSoundIsStillPlaying()) {
			g_nancy->_sound->stopSound(_callSound);
			enterScreenState(kLookupContact);
		}
		break;

	case kLookupContact: {
		// Incoming calls already know the destination scene, so the
		// contact lookup is skipped. Directory-mode outgoing calls
		// pre-resolve the contact, leaving only dial-buffer lookup.
		if (!_hasPendingCallScene) {
			if (_resolvedContact == -1) {
				_resolvedContact = findContactByDialBuffer();
			}
			if (_resolvedContact == -1) {
				enterScreenState(kInvalidNumber);
				break;
			}
		}
		if (playSoundIfPresent(_uiclData->pickupSound)) {
			enterScreenState(kWaitPickup);
		} else {
			enterScreenState(kConnected);
		}
		break;
	}

	case kWaitPickup:
		if (!callSoundIsStillPlaying()) {
			g_nancy->_sound->stopSound(_callSound);
			enterScreenState(kConnected);
		}
		break;

	case kConnected:
		// Trigger the scene change once, then sit in kConnected so the
		// connecting sprite stays on screen for the duration of the
		// conversation. AR 128 (endCall) takes the phone down afterwards.
		// Incoming calls carry their destination in _pendingCallScene;
		// outgoing calls resolve it from the active contact. The origin is
		// remembered so AR 128 can close the phone after an incoming call but
		// leave it open (welcome screen) after a player-placed one.
		if (_hasPendingCallScene) {
			SceneChangeDescription scene = _pendingCallScene;
			_hasPendingCallScene = false;
			_callWasIncoming = true;
			setReturnScene(NancySceneState.getSceneInfo());
			NancySceneState.changeScene(scene);
			resetDialPad();
		} else if (_resolvedContact >= 0 &&
				_resolvedContact < (int)_contacts.size()) {
			_callWasIncoming = false;
			triggerContactCallSceneChange((uint)_resolvedContact);
			_resolvedContact = -1;
			resetDialPad();
		}
		break;

	case kInvalidNumber:
		if (playSoundIfPresent(_uiclData->invalidNumberSound)) {
			enterScreenState(kWaitInvalid);
		} else {
			resetDialPad();
			enterScreenState(kWelcome);
		}
		break;

	case kWaitInvalid:
		if (!callSoundIsStillPlaying()) {
			g_nancy->_sound->stopSound(_callSound);
			resetDialPad();
			enterScreenState(kWelcome);
		}
		break;
	}
}

// --------------------------------------------------------------------
// Drawing
// --------------------------------------------------------------------

void CellPhonePopup::drawChrome() {
	// The chrome image holds two layouts side-by-side: the normal
	// phone-with-keypad and a zoomed-in "full screen" variant with the
	// keypad hidden. fullEmptyScreenSrc (chunk+0x10b5) points at the
	// latter; the original swaps to it for browser/list/email-content
	// modes so the LCD can extend down into the keypad area.
	const Common::Rect &chromeSrc =
		isZoomedChromeState() && !isHelpContentView() && !_uiclData->fullEmptyScreenSrc.isEmpty()
			? _uiclData->fullEmptyScreenSrc
			: _uiclData->header.normalSrcRect;
	_drawSurface.blitFrom(_overlayImage, chromeSrc, Common::Point(0, 0));
	drawCloseButton(_closeButtonHovered);
	// The help "?" button lives on the dialer face only. The original hides
	// it once a call is being placed (the connecting / "We're sorry" screens)
	// and on every sub-screen that shows its own heading.
	if (_screenState == kWelcome || _screenState == kDialing) {
		drawHelpButton(_helpButtonHovered ? 1 : 0);
	}
	_needsRedraw = true;
}

void CellPhonePopup::drawScreenContent() {
	if (_inCameraFraming) {
		// Camera framing: the popup covers the viewport and shows only the
		// movable framing box; the scene shows through the transparent surface.
		drawCameraFraming();
		return;
	}

	drawChrome();

	// Nancy 13 fills the LCD with a background graphic on every keyboard
	// screen (earlier games left it blank). The expanded/zoomed screens keep
	// their white background, so only draw it on the non-zoomed states.
	const bool n13Keyboard = g_nancy->getGameType() >= kGameTypeNancy13 && !isZoomedChromeState();
	if (n13Keyboard) {
		drawLcdTile(kN13PlainBg);
		// While a call is being placed, the LCD shows the two-phones "connecting"
		// picture over the plain background. (The idle Welcome picture is drawn in
		// the kWelcome case below.)
		switch (_screenState) {
		case kPlaceCall:
		case kWaitOutgoingRing:
		case kLookupContact:
		case kWaitPickup:
		case kConnected:
		case kInvalidNumber:
		case kWaitInvalid:
			drawLcdTile(kN13CallGraphic);
			break;
		default:
			break;
		}
	}

	// Signal + battery indicators show on the welcome screen; the connected
	// in-call screen keeps the battery but not the signal. Every other state
	// (dialing, ringing, lists, browser, ...) hides both.
	if (_screenState == kWelcome) {
		drawStatusIcons(true);
	} else if (_screenState == kConnected) {
		drawStatusIcons(false);
	}

	switch (_screenState) {
	case kWelcome:
		drawWebDirLabels();
		if (n13Keyboard) {
			// Nancy 13's top row is Cam / Menu / Dir; drawWebDirLabels() paints
			// Menu + Dir, so add the Cam label. The "Welcome / River Heights
			// Wireless" picture sits over the plain keyboard background.
			drawRibbonLabel(_uiclData->dialLabel);
			if (!_noSignal) {
				drawLcdTile(kN13MsgWelcome);
			}
		}
		if (_noSignal) {
			drawStatusLabels();
		} else if (!n13Keyboard) {
			// Earlier games draw the welcome graphic here; Nancy 13 already
			// painted the keyboard background above.
			drawWelcomeScreen();
		}
		break;

	case kDialing:
	case kPlaceCall:
	case kWaitOutgoingRing:
	case kLookupContact:
		// Web / Dir labels show only on the welcome screen, not while dialing.
		if (!_dialedNumber.empty()) {
			// User is manually dialing — show the dial header,
			// "please dial a number" hint, the typed digits and
			// the Talk highlight.
			drawDialLabel();
			drawTypeMessage();
			drawDialedNumber();
			drawHeading(_uiclData->dialHilite);
		} else {
			// Call placed from the directory / incoming call —
			// no digits to display, just the connecting animation.
			drawConnectingSprite();
		}
		// Back button on the connecting strip cancels the ringing call.
		if (isCallBackButtonActive()) {
			drawBackButton(kSubBack);
		}
		break;

	case kWaitPickup:
		// Still ringing — only the connecting sprite, plus the Back button.
		drawConnectingSprite();
		if (isCallBackButtonActive()) {
			drawBackButton(kSubBack);
		}
		break;

	case kConnected:
		// In-call screen: keeps the Web / Dir labels (like the welcome screen).
		drawWebDirLabels();
		drawConnectedLabel();
		drawConnectingSprite();
		break;

	case kInvalidNumber:
	case kWaitInvalid:
		drawConnectingSprite();
		break;

	case kDirectory:
		drawHeading(_uiclData->dirHeading);
		drawDirectoryList();
		drawDirectoryArrows();
		drawHeading(_uiclData->dialHilite);
		drawBackButton(kSubBack);
		if (n13Keyboard) {
			// Relabel the first bottom button (Cam): "Send" while choosing a photo
			// recipient, "Dial" while browsing contacts to place a call.
			drawRibbonLabelAt(_sendingPicture ? _uiclData->sendLabel.srcRect
											  : _uiclData->dialingLabel.srcRect,
							  _uiclData->dialLabel.destRect);
		}
		break;

	case kOnlineHub: {
		drawHeading(_uiclData->onlineHeading);
		drawBackButton(kSubBack);
		// The two LCD option buttons highlight (pressed sprite) on hover. Nancy
		// 13's Menu is View Pictures (subButtons[3]) + E-mail (subButtons[4]);
		// the removed web browser was subButtons[5].
		if (g_nancy->getGameType() >= kGameTypeNancy13) {
			drawHubButton(kN13SubEmail);
			drawHubButton(kN13SubViewPics);
		} else {
			drawHubButton(kSubEmail);
			drawHubButton(kSubWeb);
		}
		break;
	}

	case kWebList:
		// Web search-results list (AR-131 mode 1). Bottom button is HOME
		// (subButtons[9]) → back to the browser homepage.
		drawHeading(_uiclData->searchHeading);
		drawLinkList();
		drawDirectoryArrows();
		drawBackButton(kSubWebHome);
		break;

	case kEmailList:
		drawHeading(_uiclData->emailHeading);
		drawLinkList();
		drawDirectoryArrows();
		// Zoomed-list Back: subButtons[7] before Nancy 13, [8] in Nancy 13.
		drawBackButton(g_nancy->getGameType() >= kGameTypeNancy13 ? kN13SubBackFull : kSubEmailBack);
		break;

	case kContentView:
		// Articles show the BROWSER heading; the main browser page shows the
		// interactive SEARCH button (drawn below) instead.
		if (_contentHeading &&
			(_contentHeading != &_uiclData->browserHeading || isBrowserArticle())) {
			drawHeading(*_contentHeading);
		}
		drawContentView();
		drawDirectoryArrows();
		drawBackButton(contentViewBottomButton());
		if (_contentHeading == &_uiclData->browserHeading && !isBrowserArticle()) {
			drawHubButton(kSubSearch);
		}
		break;

	case kCamera:
		// The framing overlay is drawn by the _inCameraFraming short-circuit at
		// the top of this function.
		break;

	case kDeleteConfirm:
		// The "DELETE? YES OR NO" prompt tile plus the Yes / No ribbon labels.
		drawLcdTile(kN13MsgDeleteConfirm);
		drawRibbonLabel(_uiclData->yesLabel);
		drawRibbonLabel(_uiclData->noLabel);
		break;

	case kMessageScreen:
		// A transient message tile (Picture Sent / Deleted / Camera Full).
		if (_messageTileSrc) {
			drawLcdTile(*_messageTileSrc);
		}
		drawBackButton(kSubBack);
		break;

	case kPictureView: {
		// Nancy 13 "view pictures": the captured photo (or the "no pictures"
		// background) fills the LCD, with the Cam / Del / Send ribbon labels and
		// a Back button.
		drawPictureView();
		drawRibbonLabel(_uiclData->dialLabel);	// CAM (retake)
		drawRibbonLabel(_uiclData->delLabel);
		drawRibbonLabel(_uiclData->sendLabel);
		drawBackButton(kSubBack);
		// Paging arrows appear only when there is more than one photo to leaf
		// through (they page _pictureIndex; see handleInput).
		const CellPhonePictureData *pd = pictureData();
		if (pd && pd->pictures.size() > 1) {
			drawScrollArrow(scrollUpButton(), _scrollUpHovered);
			drawScrollArrow(scrollDownButton(), _scrollDownHovered);
		}
		break;
	}
	}

	// Keypad depress feedback sits on top of everything else.
	drawPressedDialKey();

	_needsRedraw = true;
}

void CellPhonePopup::drawStatusIcons(bool includeSignal) {
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);

	// Signal indicator uses the alt source when no-signal is set. It is hidden
	// during a call (the connected screen keeps only the battery).
	const Common::Rect &signalSrc = _noSignal && !_uiclData->signalSpriteSrcAlt.isEmpty()
		? _uiclData->signalSpriteSrcAlt
		: _uiclData->signalSpriteSrc;
	if (includeSignal && !signalSrc.isEmpty() && !_uiclData->signalSpriteDest.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, signalSrc,
								Common::Point(_uiclData->signalSpriteDest.left - chunkOrigin.x,
												_uiclData->signalSpriteDest.top - chunkOrigin.y));
	}

	const Common::Rect &batterySrc = _batteryLow && !_uiclData->batterySpriteSrcAlt.isEmpty()
		? _uiclData->batterySpriteSrcAlt
		: _uiclData->batterySpriteSrc;
	if (!batterySrc.isEmpty() && !_uiclData->batterySpriteDest.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, batterySrc,
								Common::Point(_uiclData->batterySpriteDest.left - chunkOrigin.x,
												_uiclData->batterySpriteDest.top - chunkOrigin.y));
	}
}

void CellPhonePopup::drawWebDirLabels() {
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);

	if (!_uiclData->webLabel.srcRect.isEmpty() && !_uiclData->webLabel.destRect.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, _uiclData->webLabel.srcRect,
								Common::Point(_uiclData->webLabel.destRect.left - chunkOrigin.x,
												_uiclData->webLabel.destRect.top - chunkOrigin.y));
	}

	if (!_uiclData->dirLabel.srcRect.isEmpty() && !_uiclData->dirLabel.destRect.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, _uiclData->dirLabel.srcRect,
								Common::Point(_uiclData->dirLabel.destRect.left - chunkOrigin.x,
												_uiclData->dirLabel.destRect.top - chunkOrigin.y));
	}
}

void CellPhonePopup::drawDialLabel() {
	drawRibbonLabel(_uiclData->dialLabel);
}

void CellPhonePopup::drawRibbonLabel(const UICL::SrcDestRectPair &label) {
	if (label.srcRect.isEmpty() || label.destRect.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, label.srcRect,
							Common::Point(label.destRect.left - chunkOrigin.x,
											label.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawRibbonLabelAt(const Common::Rect &src, const Common::Rect &dest) {
	// Draw one ribbon-label sprite at another label's slot — used to relabel the
	// first bottom button (Cam) as Dial in the directory and Send in the
	// picture-recipient chooser.
	if (src.isEmpty() || dest.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(dest.left - chunkOrigin.x,
											dest.top - chunkOrigin.y));
}

void CellPhonePopup::drawTypeMessage() {
	const UICL::SrcDestRectPair &tm = _uiclData->typeMessage;
	if (tm.srcRect.isEmpty() || tm.destRect.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, tm.srcRect,
							Common::Point(tm.destRect.left - chunkOrigin.x,
											tm.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawConnectedLabel() {
	const UICL::SrcDestRectPair &cl = _uiclData->connectedLabel;
	if (cl.srcRect.isEmpty() || cl.destRect.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, cl.srcRect,
							Common::Point(cl.destRect.left - chunkOrigin.x,
											cl.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawConnectingSprite() {
	// Invalid-number states swap in the alternate connecting sprite
	// (the "try again" / red-light variant).
	const bool useAlt = (_screenState == kInvalidNumber || _screenState == kWaitInvalid) &&
						!_uiclData->connectingSpriteSrcAlt.isEmpty();
	const Common::Rect &src = useAlt
		? _uiclData->connectingSpriteSrcAlt
		: _uiclData->connectingSpriteSrc;
	if (src.isEmpty() || _uiclData->connectingSpriteDest.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(_uiclData->connectingSpriteDest.left - chunkOrigin.x,
											_uiclData->connectingSpriteDest.top - chunkOrigin.y));
}

void CellPhonePopup::drawDialedNumber() {
	if (_dialedNumber.empty()) {
		return;
	}

	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId1);
	if (!font) {
		return;
	}

	const int x = _uiclData->statusTextX - _screenPosition.left;
	const int y = _uiclData->statusTextY - _screenPosition.top;

	font->drawString(&_drawSurface, _dialedNumber, x, y,
						_screenPosition.width() - x, 0);
}

void CellPhonePopup::drawHelpButton(uint state) {
	const UICL::ThreeRectWidget &hb = _uiclData->helpButton;
	if (hb.destRect.isEmpty()) {
		return;
	}

	const Common::Rect &src = (state == 1 && !hb.srcRectPressed.isEmpty())
								? hb.srcRectPressed
								: hb.srcRectIdle;
	if (src.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(hb.destRect.left - chunkOrigin.x,
											hb.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawCloseButton(bool hovered) {
	const UIButtonRecord &btn = _uiclData->header.secondaryButton;
	if (!_uiclData->header.secondaryButtonEnabled || btn.destRect.isEmpty()) {
		return;
	}

	Common::Rect src = btn.sourceRects[hovered ? kUIButtonHover : kUIButtonIdle];
	if (src.isEmpty()) {
		src = btn.sourceRects[0];
	}
	if (src.isEmpty()) {
		return;
	}

	Common::Rect dstRect = btn.destRect;
	if (btn.destUsesGameFrameOffset) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			dstRect.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	const Common::Point dst(dstRect.left - _screenPosition.left,
							dstRect.top - _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src, dst);
}

void CellPhonePopup::drawStatusLabels() {
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId1);
	if (!font) {
		return;
	}

	const int x = _uiclData->statusTextX - _screenPosition.left;
	const int yBase = _uiclData->statusTextY - _screenPosition.top;
	const int kLineYOffsets[UICL::kNumStatusLabels] = { -10, 20, 50 };

	for (uint i = 0; i < UICL::kNumStatusLabels; ++i) {
		const Common::String &label = _uiclData->statusLabels[i];
		if (label.empty()) {
			continue;
		}
		const int y = yBase + kLineYOffsets[i];
		font->drawString(&_drawSurface, label, x, y,
							_screenPosition.width() - x, 0);
	}
}

void CellPhonePopup::drawHeading(const UICL::SrcDestRectPair &heading) {
	if (heading.srcRect.isEmpty() || heading.destRect.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, heading.srcRect,
							Common::Point(heading.destRect.left - chunkOrigin.x,
											heading.destRect.top - chunkOrigin.y));
}

Common::Array<uint> CellPhonePopup::listVisibleIndices() const {
	Common::Array<uint> out;
	const CellPhoneData *cellData = (const CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (!cellData) {
		return out;
	}

	if (_screenState == kWebList) {
		for (uint i = 0; i < cellData->searchLinks.size(); ++i) {
			out.push_back(i);
		}
	} else if (_screenState == kEmailList) {
		// "Old Email Only" (no-signal) hides messages not yet read.
		for (uint i = 0; i < cellData->emailMessages.size(); ++i) {
			if (!_noSignal || cellData->emailMessages[i].read) {
				out.push_back(i);
			}
		}
	}
	return out;
}

void CellPhonePopup::drawLinkList() {
	const CellPhoneData *cellData = (const CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
	if (!cellData) {
		return;
	}
	const Common::Array<SearchLink> &list =
		_screenState == kWebList ? cellData->searchLinks : cellData->emailMessages;
	const Common::Array<uint> visible = listVisibleIndices();
	if (visible.empty()) {
		return;
	}

	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return;
	}

	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");

	const uint titleRows = listTitleRows();
	const uint totalRows = maxDirectoryRows();
	const uint maxEntries = totalRows > titleRows ? totalRows - titleRows : 0;
	for (uint visibleRow = 0;
			visibleRow < maxEntries && _directoryScroll + visibleRow < visible.size();
			++visibleRow) {
		const uint absolute = visible[_directoryScroll + visibleRow];
		const Common::Rect rowRect = directoryRowRect(titleRows + visibleRow);

		// Every inbox row shows the closed-envelope icon; the opened envelope is
		// only flashed on the row being opened (see _openingEmailRow), not used
		// as a persistent read/selection indicator.
		int textX = rowRect.left;
		if (_screenState == kEmailList) {
			const bool opening = ((int)(_directoryScroll + visibleRow) == _openingEmailRow) &&
									!_uiclData->emailIconSelected.isEmpty();
			const Common::Rect &icon = opening
				? _uiclData->emailIconSelected
				: _uiclData->emailIconUnread;
			if (!icon.isEmpty()) {
				const int iconX = MAX(0, rowRect.left - icon.width() - 2);
				_drawSurface.blitFrom(_spritesImage, icon,
										Common::Point(iconX, rowRect.top));
				textX = MAX(textX, iconX + icon.width() + 2);
			}
		}

		Common::String lookupKey = list[absolute].key;
		Common::String rowText = autotext->texts.getValOrDefault(lookupKey, "");

		// Single-line draw — drop every <n> markup so they don't render as
		// literal "<n>" glyphs and crowd the row.
		while (rowText.contains("<n>")) {
			rowText.erase(rowText.find("<n>"), 3);
		}

		// Original anchors row text on the row's bottom (baseline-up
		// rendering); mirror that so the glyphs sit at the bottom of
		// the row instead of glued to the top.
		const int textY = MAX<int16>(rowRect.top,
								rowRect.bottom - font->getFontHeight());
		font->drawString(&_drawSurface, rowText,
							textX, textY,
							rowRect.right - textX, 0);
	}
}

void CellPhonePopup::openContentView(const Common::String &key, const UICL::SrcDestRectPair &heading) {
	_contentReturnState = _screenState;
	_contentHeading = &heading;
	_contentKey = key;
	_contentKey.toUppercase();
	_contentScroll = 0;
	enterScreenState(kContentView);
}

void CellPhonePopup::openBrowserHome() {
	// The web browser was removed in Nancy 13 (its atlas CUR_URLimages_OVL no
	// longer ships); the Menu's second option there is "view pictures", wired
	// separately. Never open the (missing) browser for Nancy 13.
	if (g_nancy->getGameType() >= kGameTypeNancy13) {
		return;
	}

	// The Web button opens the browser home page (UIBW page 0 — the "River
	// Heights Wireless" homepage). Its in-page hyperlinks then navigate to
	// further pages / the search list.
	const UIBW *browserData = GetEngineData(UIBW);
	if (browserData && !browserData->pages.empty()) {
		// Remember the main-page key so isBrowserArticle() can tell it apart.
		_browserHomeKey = browserData->pages[0].imageName.toString();
		_browserHomeKey.toUppercase();
		openContentView(browserData->pages[0].imageName.toString(), _uiclData->browserHeading);
		// The homepage's Back button always returns to the main phone (welcome)
		// screen, regardless of whether the browser was opened from the online
		// hub or reached via the search list's HOME button. openContentView
		// otherwise records whichever screen we came from, which could send Back
		// to the search list.
		_contentReturnState = kWelcome;
	} else {
		enterScreenState(kWebList);
	}
}

void CellPhonePopup::renderContentPage(int surfaceWidth) {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	const Common::String renderText = autotext->texts.getValOrDefault(_contentKey, "");

	// Find this page in the UIBW chunk (browser pages only); its hotspot
	// records are the per-page image table the article references.
	const UIBW *browserData = nullptr;
	int pageIdx = -1;
	if (_contentHeading == &_uiclData->browserHeading) {
		browserData = GetEngineData(UIBW);
		if (browserData) {
			for (uint p = 0; p < browserData->pages.size(); ++p) {
				Common::String pageKey = browserData->pages[p].imageName.toString();
				pageKey.toUppercase();
				if (pageKey == _contentKey) {
					pageIdx = (int)p;
					break;
				}
			}
		}
	}

	// Parse the <H>...<L> regions out of the body — each becomes a clickable
	// in-page hyperlink; the text between the markers is the target CVTX key.
	_contentCacheTargets.clear();
	{
		uint32 cursor = 0;
		while (cursor < renderText.size()) {
			const uint32 hStart = renderText.find("<H>", cursor);
			if (hStart >= renderText.size()) {
				break;
			}
			const uint32 linkTextStart = hStart + 3;
			const uint32 lStart = renderText.find("<L>", linkTextStart);
			if (lStart >= renderText.size()) {
				break;
			}
			Common::String linkText = renderText.substr(linkTextStart, lStart - linkTextStart);
			linkText.toUppercase();
			_contentCacheTargets.push_back(linkText);
			cursor = lStart + 3;
		}
	}

	CellPhoneHypertext ht;
	if (pageIdx >= 0 && !browserData->pages[pageIdx].hotspots.empty()) {
		// UIBW hotspots are misnamed — they're per-page image records
		// (id = line in the rendered text, rect = source in the atlas).
		ht.setImageName(browserData->imageName);
		for (uint i = 0; i < browserData->pages[pageIdx].hotspots.size(); ++i) {
			const UIBW::Hotspot &h = browserData->pages[pageIdx].hotspots[i];
			ht.addImage(h.id, h.rect);
		}
	}
	const uint32 trans = g_nancy->_graphics->getTransColor();

	// Render into a tall scratch surface, then cache only the used height
	static const uint16 kMaxContentHeight = 6000;
	ht.render(surfaceWidth, kMaxContentHeight, trans, renderText, _uiclData->fontId2);

	const uint16 textHeight = MIN<uint16>(MAX<uint16>(ht.textHeight(), 1), kMaxContentHeight);
	_contentCacheSurface.create(surfaceWidth, textHeight, ht.surface().format);
	_contentCacheSurface.setTransparentColor(trans);
	_contentCacheSurface.clear(trans);
	_contentCacheSurface.blitFrom(ht.surface(), Common::Rect(0, 0, (int16)surfaceWidth, (int16)textHeight),
									Common::Point(0, 0));
	_contentCacheTextHeight = textHeight;
	_contentCacheHotspots = ht.hotspots();
}

uint CellPhonePopup::contentScrollStep() const {
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return 12;
	}

	// Original: one click scrolls ~1/10th of the article (capped near a full
	// page), plus 1.25 line heights.
	const Common::Rect &ws =
		(isHelpContentView() || _uiclData->emailListContainer.isEmpty())
			? _uiclData->welcomeScreen.destRect
			: _uiclData->emailListContainer;
	const int viewH = MAX(0, ws.height() - 2);
	int page = MIN((int)_contentCacheTextHeight / 10, MAX(0, viewH - 30));
	return (font->getFontHeight() * 5) / 4 + page;
}

void CellPhonePopup::drawContentView() {
	if (_contentKey.empty()) {
		return;
	}

	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return;
	}

	// Browser / email articles run under the zoomed-in chrome (drawChrome
	// blits fullEmptyScreenSrc), so the keypad is no longer visible underneath
	// and we render into the larger LCD area that emailListContainer defines.
	// The help page keeps the regular chrome, so it renders into the small LCD.
	const Common::Rect &ws =
		(isHelpContentView() || _uiclData->emailListContainer.isEmpty())
			? _uiclData->welcomeScreen.destRect
			: _uiclData->emailListContainer;
	const int lcdLeft = ws.left - _screenPosition.left;
	const int lcdTop  = ws.top  - _screenPosition.top;
	const int lcdW    = ws.width();
	const int lcdH    = ws.height();
	// The heading (help / email / browser) sits in the title-bar strip above the
	// LCD, so the body text starts flush with the LCD top — a small inset only.
	const int textTop = 2;
	const int viewH   = MAX(0, lcdH - textTop);

	// (Re)render the page only when its key changes; scrolling and hover
	// redraws reuse the cached surface (just re-blit a different window).
	if (_contentKey != _contentCacheKey) {
		renderContentPage(lcdW);
		_contentCacheKey = _contentKey;
	}
	_contentHotspotTargets = _contentCacheTargets;

	// Clamp scroll (a pixel offset) to the rendered text height.
	const int maxScrollPx = MAX(0, (int)_contentCacheTextHeight - viewH);
	if ((int)_contentScroll > maxScrollPx) {
		_contentScroll = maxScrollPx;
	}

	const int srcTop = (int)_contentScroll;
	Common::Rect srcRect(0, srcTop, lcdW, srcTop + viewH);
	srcRect.clip(Common::Rect(_contentCacheSurface.w, _contentCacheSurface.h));
	if (srcRect.isEmpty()) {
		_contentHotspots.clear();
		return;
	}

	_drawSurface.blitFrom(_contentCacheSurface, srcRect,
							Common::Point(lcdLeft, lcdTop + textTop));

	// Translate the cached hotspots (surface coords) into popup-local coords
	// for the current scroll. Drop any that aren't fully visible inside the
	// LCD window so we don't fire on partially-clipped links.
	_contentHotspots.clear();
	const uint linkCount = MIN(_contentCacheHotspots.size(), _contentHotspotTargets.size());
	for (uint i = 0; i < linkCount; ++i) {
		Common::Rect r = _contentCacheHotspots[i];
		r.translate(lcdLeft, lcdTop + textTop - srcTop);
		const Common::Rect lcdClip(lcdLeft, lcdTop + textTop,
									lcdLeft + lcdW, lcdTop + textTop + viewH);
		Common::Rect clipped = r.findIntersectingRect(lcdClip);
		if (!clipped.isEmpty()) {
			_contentHotspots.push_back(clipped);
		} else {
			_contentHotspots.push_back(Common::Rect());
		}
	}
	// Resize so indices align even if some links were clipped to empty.
	if (_contentHotspotTargets.size() > linkCount) {
		_contentHotspotTargets.resize(linkCount);
	}
}

void CellPhonePopup::drawDirectoryList() {
	// Contacts have one record per dial-pattern variant; collapse by name.
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return;
	}

	const uint maxRows = maxDirectoryRows();
	Common::Array<Common::String> seenNames;
	uint visibleRow = 0;
	uint visited = 0;

	for (uint contactIdx = 0;
			contactIdx < _contacts.size() && visibleRow < maxRows;
			++contactIdx) {
		const UICL::Contact &c = _contacts[contactIdx];
		if (c.name.empty() || !isContactVisible(c)) {
			continue;
		}

		bool duplicate = false;
		for (uint s = 0; s < seenNames.size(); ++s) {
			if (seenNames[s].equalsIgnoreCase(c.name)) {
				duplicate = true;
				break;
			}
		}
		if (duplicate) {
			continue;
		}
		seenNames.push_back(c.name);

		if (visited < _directoryScroll) {
			++visited;
			continue;
		}
		++visited;

		const Common::Rect rowRect = directoryRowRect(visibleRow);
		const int textY = MAX<int16>(rowRect.top,
								rowRect.bottom - font->getFontHeight());
		font->drawString(&_drawSurface, c.name,
							rowRect.left, textY,
							rowRect.width(), 0);
		++visibleRow;
	}
}

void CellPhonePopup::drawWelcomeScreen() {
	const UICL::SrcDestRectPair &ws = _uiclData->welcomeScreen;
	if (ws.srcRect.isEmpty() || ws.destRect.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, ws.srcRect,
							Common::Point(ws.destRect.left - chunkOrigin.x,
											ws.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawLcdTile(const Common::Rect &src) {
	const Common::Rect &lcd = _uiclData->welcomeScreen.destRect;
	if (src.isEmpty() || lcd.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(lcd.left - chunkOrigin.x,
											lcd.top - chunkOrigin.y));
}

void CellPhonePopup::showMessageScreen(const Common::Rect &tileSrc, ScreenState returnState) {
	_messageTileSrc = &tileSrc;
	_messageReturnState = returnState;
	enterScreenState(kMessageScreen);
}

CellPhonePictureData *CellPhonePopup::pictureData() const {
	// Lazily created (Nancy 13 only) — persists in the 'CPIC' save chunk.
	return (CellPhonePictureData *)NancySceneState.getPuzzleData(CellPhonePictureData::getTag());
}

void CellPhonePopup::drawPictureView() {
	CellPhonePictureData *pd = pictureData();
	if (!pd || pd->pictures.empty()) {
		drawLcdTile(kN13MsgNoPictures);
		return;
	}

	_pictureIndex = CLIP<int>(_pictureIndex, 0, (int)pd->pictures.size() - 1);
	const CapturedPicture &pic = pd->pictures[_pictureIndex];
	if (pic.width == 0 || pic.height == 0 ||
			pic.pixels.size() < (uint)pic.width * (uint)pic.height * 4) {
		return;
	}

	const Common::Rect &lcd = _uiclData->welcomeScreen.destRect;
	if (lcd.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);

	// Wrap the stored BGRA32 pixels and scale them into the LCD area.
	Graphics::Surface src;
	src.init(pic.width, pic.height, pic.width * 4,
				const_cast<byte *>(pic.pixels.data()),
				g_nancy->_graphics->getScreenPixelFormat());
	Graphics::Surface *scaled = src.scale((int16)lcd.width(), (int16)lcd.height(), false);
	if (scaled) {
		_drawSurface.blitFrom(*scaled,
				Common::Point(lcd.left - chunkOrigin.x, lcd.top - chunkOrigin.y));
		scaled->free();
		delete scaled;
	}
}

void CellPhonePopup::captureViewport(const Common::Rect &screenRegion) {
	CellPhonePictureData *pd = pictureData();
	if (!pd) {
		return;
	}

	const Viewport &viewport = NancySceneState.getViewport();
	const Graphics::ManagedSurface &vp = viewport.getBackground();
	if (vp.w == 0 || vp.h == 0) {
		return;
	}

	// Translate the framed screen region into the (scrolled) viewport background.
	const Common::Rect vpScreen = viewport.getScreenPosition();
	const int scrollY = (int)viewport.getCurVerticalScroll();
	Common::Rect grab;
	if (screenRegion.isEmpty()) {
		grab = Common::Rect(0, scrollY, vpScreen.width(), scrollY + vpScreen.height());
	} else {
		grab = screenRegion;
		grab.translate(-vpScreen.left, -vpScreen.top + scrollY);
	}
	grab.clip(Common::Rect(vp.w, vp.h));
	if (grab.isEmpty()) {
		return;
	}

	// Copy the sub-area, converted to BGRA32 so it can be re-displayed and saved.
	Graphics::Surface sub = vp.rawSurface().getSubArea(grab);
	Graphics::Surface *conv = sub.convertTo(g_nancy->_graphics->getScreenPixelFormat());
	if (!conv) {
		return;
	}

	CapturedPicture pic;
	pic.width = (uint16)conv->w;
	pic.height = (uint16)conv->h;
	pic.pixels.resize((uint)pic.width * (uint)pic.height * 4);
	for (int y = 0; y < conv->h; ++y) {
		memcpy(pic.pixels.data() + (uint)y * (uint)pic.width * 4,
				conv->getBasePtr(0, y), (uint)pic.width * 4);
	}
	conv->free();
	delete conv;

	pd->pictures.push_back(pic);
	_pictureIndex = (int)pd->pictures.size() - 1;
}

Common::Rect CellPhonePopup::framingScreenRect() const {
	const Common::Rect vp = NancySceneState.getViewport().getScreenPosition();
	const int w = MIN<int>(kFramingWidth, vp.width());
	const int h = MIN<int>(kFramingHeight, vp.height());
	const int cx = CLIP<int>(_framingMouse.x, vp.left + w / 2, vp.right - w / 2);
	const int cy = CLIP<int>(_framingMouse.y, vp.top + h / 2, vp.bottom - h / 2);
	return Common::Rect(cx - w / 2, cy - h / 2, cx - w / 2 + w, cy - h / 2 + h);
}

void CellPhonePopup::enterCameraFraming() {
	if (_inCameraFraming) {
		return;
	}
	_inCameraFraming = true;
	_savedPhoneRect = _screenPosition;

	// Grow the popup to cover the viewport so the framing box can be drawn
	// anywhere over the live scene.
	const Common::Rect vp = NancySceneState.getViewport().getScreenPosition();
	moveTo(vp);
	_drawSurface.create(vp.width(), vp.height(), g_nancy->_graphics->getScreenPixelFormat());
	setTransparent(true);
	_framingMouse = Common::Point(vp.left + vp.width() / 2, vp.top + vp.height() / 2);
}

void CellPhonePopup::exitCameraFraming() {
	if (!_inCameraFraming) {
		return;
	}
	_inCameraFraming = false;
	moveTo(_savedPhoneRect);
	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getScreenPixelFormat());
	setTransparent(false);
}

void CellPhonePopup::drawCameraFraming() {
	const uint32 trans = g_nancy->_graphics->getTransColor();
	_drawSurface.clear(trans);

	Common::Rect box = framingScreenRect();
	box.translate(-_screenPosition.left, -_screenPosition.top);

	// TODO: The original swaps the mouse cursor for a framing-rectangle sprite
	// loaded from the game resources (via the scene-prep capture path, not the
	// UICL chunk); locate that cursor and blit it here instead of the drawn
	// outline. The 220x176 framing size is likewise a placeholder to confirm.
	const uint32 col = _drawSurface.format.RGBToColor(255, 255, 255);
	_drawSurface.frameRect(box, col);
	if (box.width() > 4 && box.height() > 4) {
		box.grow(-1);
		_drawSurface.frameRect(box, col);
	}
	_needsRedraw = true;
}

void CellPhonePopup::drawBackButton(uint subButtonIndex) {
	// kSubBack is the Back button in the lower ribbon (help / sub-screens);
	// kSubEmailBack is the Back button at the bottom of the zoomed content view.
	const UICL::ThreeRectWidget &back = _uiclData->subButtons[subButtonIndex];
	if (back.destRect.isEmpty()) {
		return;
	}

	// Highlight (pressed sprite, green arrow) when the cursor is over this
	// button and it's the one currently accepting input.
	const bool hovered = _backButtonHovered && (int)subButtonIndex == currentBackButtonIndex();
	const Common::Rect &src = (hovered && !back.srcRectPressed.isEmpty())
								? back.srcRectPressed
								: back.srcRectIdle;
	if (src.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(back.destRect.left - chunkOrigin.x,
											back.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawHubButton(uint subButtonIndex) {
	const UICL::ThreeRectWidget &btn = _uiclData->subButtons[subButtonIndex];
	if (btn.destRect.isEmpty()) {
		return;
	}
	const bool hovered = (_hoveredHubButton == (int)subButtonIndex);
	const Common::Rect &src = (hovered && !btn.srcRectPressed.isEmpty())
								? btn.srcRectPressed
								: btn.srcRectIdle;
	if (src.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(btn.destRect.left - chunkOrigin.x,
											btn.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawPressedDialKey() {
	if (_pressedSlot < 0 || _pressedSlot >= (int)UICL::kNumDialPadSlots) {
		return;
	}
	// A dial-pad slot's single srcRect is the lit / depressed key sprite; the
	// idle keypad is baked into the chrome image. Blit it over the key's dest
	// rect so the key visibly depresses while held.
	const UICL::DialPadSlot &slot = _uiclData->dialPadSlots[_pressedSlot];
	if (slot.srcRect.isEmpty() || slot.destRect.isEmpty()) {
		return;
	}
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, slot.srcRect,
							Common::Point(slot.destRect.left - chunkOrigin.x,
											slot.destRect.top - chunkOrigin.y));
}

Common::Rect CellPhonePopup::backButtonHitRect(uint subButtonIndex) const {
	// Popup-local hit rect for a Back sub-button.
	Common::Rect r = _uiclData->subButtons[subButtonIndex].destRect;
	if (r.isEmpty()) {
		return r;
	}
	r.translate(-_screenPosition.left, -_screenPosition.top);
	return r;
}

int CellPhonePopup::currentBackButtonIndex() const {
	// Mirrors the drawBackButton() calls in drawScreenContent: which
	// subButtons slot holds the visible Back / HOME button per screen.
	if (isCallBackButtonActive()) {
		return 0;
	}
	switch (_screenState) {
	case kDirectory:
	case kOnlineHub:
	case kPictureView:
	case kMessageScreen:
		return kSubBack;
	case kWebList:
		return kSubWebHome;
	case kEmailList:
		return kSubEmailBack;
	case kContentView:
		return (int)contentViewBottomButton();
	default:
		return -1;
	}
}

uint CellPhonePopup::contentViewBottomButton() const {
	// Help Back (0); browser-article HOME (9); main page / email article Back (7).
	if (isHelpContentView()) {
		return kSubBack;
	}
	return isBrowserArticle() ? kSubWebHome : kSubEmailBack;
}

bool CellPhonePopup::isBrowserArticle() const {
	// A browser content view other than the main page opened by openBrowserHome().
	if (_contentHeading != &_uiclData->browserHeading) {
		return false;
	}
	return !_browserHomeKey.empty() && !_contentKey.equalsIgnoreCase(_browserHomeKey);
}

const UICL::ThreeRectWidget &CellPhonePopup::scrollUpButton() const {
	// Directory and help scroll with the small-LCD arrow pair (subButtons[1]/[2]);
	// the zoomed email / browser lists use a different pair. Nancy 13 keeps the
	// directory on [1]/[2] and uses the list arrows [6]/[7] elsewhere; earlier
	// games use [5]/[6] for the zoomed lists.
	const bool smallLcd = _screenState == kDirectory || isHelpContentView();
	if (g_nancy->getGameType() >= kGameTypeNancy13) {
		return smallLcd ? _uiclData->subButtons[kN13SubDirUp]
						: _uiclData->subButtons[kN13SubListUp];
	}
	return smallLcd ? _uiclData->subButtons[kSubDirUp] : _uiclData->subButtons[kSubListUp];
}

const UICL::ThreeRectWidget &CellPhonePopup::scrollDownButton() const {
	const bool smallLcd = _screenState == kDirectory || isHelpContentView();
	if (g_nancy->getGameType() >= kGameTypeNancy13) {
		return smallLcd ? _uiclData->subButtons[kN13SubDirDown]
						: _uiclData->subButtons[kN13SubListDown];
	}
	return smallLcd ? _uiclData->subButtons[kSubDirDown] : _uiclData->subButtons[kSubListDown];
}

void CellPhonePopup::drawScrollArrow(const UICL::ThreeRectWidget &arrow, bool hovered) {
	// Scroll/paging arrows are not in the chrome image; blit on every redraw.
	// The pressed (lit) sprite is used while the cursor is over the arrow.
	if (arrow.destRect.isEmpty()) {
		return;
	}
	const Common::Rect &src = (hovered && !arrow.srcRectPressed.isEmpty())
		? arrow.srcRectPressed
		: arrow.srcRectIdle;
	if (src.isEmpty()) {
		return;
	}
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(arrow.destRect.left - _screenPosition.left,
											arrow.destRect.top - _screenPosition.top));
}

void CellPhonePopup::drawDirectoryArrows() {
	drawScrollArrow(scrollUpButton(), _scrollUpHovered);
	drawScrollArrow(scrollDownButton(), _scrollDownHovered);

	// Selection indicator (dirArrowSrc sprite) — only the contacts directory
	// shows it. The search-topic and email lists are plain lists in the
	// original (no per-row selection arrow).
	if (_screenState != kDirectory) {
		return;
	}
	const Common::Rect &arrowSrc = _uiclData->dirArrowSrc;
	const Common::Rect &cursor = _uiclData->dirCursorSrc;
	if (arrowSrc.isEmpty() || cursor.isEmpty()) {
		return;
	}
	const uint titleRows = listTitleRows();
	const uint selRow = titleRows + _directorySelection;
	if (selRow >= maxDirectoryRows()) {
		return;
	}
	const int arrowX = cursor.left - _screenPosition.left;
	const int arrowY = cursor.top - _screenPosition.top + (int)selRow * rowPitch();
	_drawSurface.blitFrom(_spritesImage, arrowSrc,
							Common::Point(arrowX, arrowY));
}

// --------------------------------------------------------------------
// State-machine helpers
// --------------------------------------------------------------------

void CellPhonePopup::resetDialPad() {
	_dialedNumber.clear();
}

void CellPhonePopup::enterScreenState(ScreenState newState) {
	// Viewing a list clears its badge: directory = sub 0, email = sub 1, web = sub 2.
	if (UI::Taskbar *taskbar = NancySceneState.getTaskbar()) {
		switch (newState) {
		case kDirectory:
			taskbar->clearNotification(kTaskButtonCellphone, 0);
			break;
		case kEmailList:
			taskbar->clearNotification(kTaskButtonCellphone, 1);
			break;
		case kWebList:
			taskbar->clearNotification(kTaskButtonCellphone, 2);
			break;
		default:
			break;
		}
	}

	// Always redraw, so successive digit entries refresh the readout.
	_screenState = newState;
	_hoveredHubButton = -1;
	_helpButtonHovered = false;
	_backButtonHovered = false;
	if (newState != kContentView) {
		// Cancel a pending email-open flash unless we're completing it.
		_openingEmailRow = -1;
	}
	drawScreenContent();
}

void CellPhonePopup::cancelCall() {
	if (!_callSound.name.empty()) {
		g_nancy->_sound->stopSound(_callSound);
	}
	_autoDialPending = false;
	_resolvedContact = -1;
	_pressedSlot = -1;
	resetDialPad();
	enterScreenState(kWelcome);
}

void CellPhonePopup::appendDigit(byte slotIndex) {
	if (_dialedNumber.size() >= 11) {
		return;
	}
	_dialedNumber += (char)('0' + slotIndex);
	enterScreenState(kDialing);

	// Auto-dial without a Talk press only once the full 11-digit number has
	// been entered. The call is queued rather than placed immediately so
	// updateGraphics can wait for the last key's DTMF tone to finish (which
	// shares the call-sound channel with the outgoing ring).
	if (_noSignal) {
		return;
	}
	if (_dialedNumber.size() >= 11) {
		_autoDialPending = true;
	}
}

void CellPhonePopup::playDialPadSound(const Common::String &name) {
	if (name.empty() || name.equalsIgnoreCase("NO SOUND")) {
		return;
	}
	// Dial-pad tones are raw sound filenames, so play them through the phone's
	// call-sound channel (a single, non-looping cue) instead of the common
	// sound table, which only holds boot-registered sounds.
	SoundDescription sound = _uiclData->callSoundTemplate;
	sound.name = name;
	sound.numLoops = 1;
	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
	// Track the tone on the call-sound channel so a queued auto-dial / Talk can
	// wait for it to finish before ringing (see updateGraphics).
	_callSound = sound;
}

void CellPhonePopup::playButtonClickSound(const UIButtonRecord &button) {
	SoundDescription sound = button.clickSound;
	if (sound.name.empty() || sound.name.equalsIgnoreCase("NO SOUND"))
		return;

	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
}

bool CellPhonePopup::playSoundIfPresent(const Common::Path &soundName) {
	if (soundName.empty()) {
		return false;
	}
	Common::String nameStr = soundName.toString();
	if (nameStr.equalsIgnoreCase("NO SOUND")) {
		return false;
	}

	// Play through the chunk's call-sound template: channel / volume /
	// loops come from it, only the filename varies per cue.
	_callSound = _uiclData->callSoundTemplate;
	_callSound.name = nameStr;

	g_nancy->_sound->loadSound(_callSound);
	g_nancy->_sound->playSound(_callSound);
	return true;
}

bool CellPhonePopup::callSoundIsStillPlaying() const {
	if (_callSound.name.empty()) {
		return false;
	}
	return g_nancy->_sound->isSoundPlaying(_callSound);
}

int CellPhonePopup::findContactByDialBuffer() const {
	if (_dialedNumber.empty()) {
		return -1;
	}

	// Dial pattern lives in prefix[2..], terminated by '\n'.
	const uint dialLen = _dialedNumber.size();
	const uint kDialOffset = 2;
	for (uint i = 0; i < _contacts.size(); ++i) {
		const UICL::Contact &c = _contacts[i];
		if (!isContactVisible(c)) {
			continue;
		}
		bool match = true;
		for (uint b = 0; b < dialLen; ++b) {
			const byte slotIdx = (byte)(_dialedNumber[b] - '0');
			if (kDialOffset + b >= sizeof(c.unknownPrefix) ||
					slotIdx != c.unknownPrefix[kDialOffset + b]) {
				match = false;
				break;
			}
		}
		if (match && kDialOffset + dialLen < sizeof(c.unknownPrefix) &&
				c.unknownPrefix[kDialOffset + dialLen] == '\n') {
			return (int)i;
		}
	}
	return -1;
}

void CellPhonePopup::triggerContactCallSceneChange(uint contactIndex) {
	if (contactIndex >= _contacts.size()) {
		return;
	}

	const UICL::Contact &c = _contacts[contactIndex];

	const uint16 sceneID = (uint16)c.unknownSuffix[0] | ((uint16)c.unknownSuffix[1] << 8);
	if (sceneID == kNoScene) {
		return;
	}
	const uint16 frameID = (uint16)c.unknownSuffix[2] | ((uint16)c.unknownSuffix[3] << 8);
	const int16 eventFlagLabel = (int16)((uint16)c.unknownSuffix[4] |
											((uint16)c.unknownSuffix[5] << 8));
	const byte eventFlagValue = c.unknownSuffix[6];

	SceneChangeDescription scene;
	scene.sceneID = sceneID;
	scene.frameID = frameID;
	scene.verticalOffset = 0;
	// The destination scene's sound carries the conversation audio.
	scene.continueSceneSound = kLoadSceneSound;

	if (eventFlagLabel != -1) {
		NancySceneState.setEventFlag(eventFlagLabel,
										eventFlagValue ? g_nancy->_true : g_nancy->_false);
	}

	// Save the pre-call scene on the popup so AR 128 can return there
	// without clobbering the global push slot (used by closeups, etc).
	setReturnScene(NancySceneState.getSceneInfo());

	NancySceneState.changeScene(scene);

	// Phone stays on screen through the conversation; AR 128 closes it.
}

void CellPhonePopup::setReturnScene(const SceneChangeDescription &scene) {
	_returnScene = scene;
	_hasReturnScene = true;
}

bool CellPhonePopup::consumeReturnScene(SceneChangeDescription &out) {
	if (!_hasReturnScene)
		return false;
	out = _returnScene;
	_hasReturnScene = false;
	return true;
}

// --------------------------------------------------------------------
// Directory helpers
// --------------------------------------------------------------------

int CellPhonePopup::rowPitch() const {
	// The email and search lists render in the tall zoomed LCD with generous,
	// evenly-spaced rows (sized by the envelope icon); the contacts directory
	// uses the compact arrow-cursor pitch.
	if (isLinkListMode() && !_uiclData->emailIconUnread.isEmpty()) {
		return _uiclData->emailIconUnread.height() + 1;
	}
	const Common::Rect &cursor = _uiclData->dirCursorSrc;
	if (!cursor.isEmpty()) {
		return cursor.height() + 8;
	}
	return MAX(_uiclData->dirArrowSrc.height() + 4, 14);
}

int CellPhonePopup::rowTopScreen() const {
	// The email / search lists anchor on the zoomed-chrome list container;
	// the directory stacks under the arrow-cursor row.
	if (isLinkListMode() && !_uiclData->emailListContainer.isEmpty()) {
		return _uiclData->emailListContainer.top;
	}
	const Common::Rect &cursor = _uiclData->dirCursorSrc;
	if (!cursor.isEmpty()) {
		return cursor.top - 5;
	}
	return _uiclData->welcomeScreen.destRect.top + 22;
}

uint CellPhonePopup::maxDirectoryRows() const {
	const int pitch = rowPitch();
	if (pitch <= 0) {
		return 0;
	}
	const int yLimit = (isLinkListMode() && !_uiclData->emailListContainer.isEmpty())
		? _uiclData->emailListContainer.bottom
		: _uiclData->welcomeScreen.destRect.bottom;
	int y = rowTopScreen();
	uint count = 0;
	while (y + pitch < yLimit) {
		++count;
		y += pitch;
	}
	return count;
}

Common::Rect CellPhonePopup::directoryRowRect(uint visibleIndex) const {
	const Common::Rect &cursor = _uiclData->dirCursorSrc;
	const Common::Rect &ws = _uiclData->welcomeScreen.destRect;
	const int pitch = rowPitch();

	// The web / email lists render under the zoomed (keypad-hidden) chrome,
	// where the LCD extends into the wider emailListContainer. Use that as
	// the right bound so long entries aren't clipped to the narrow
	// keypad-mode screen; the directory list keeps the small LCD.
	const Common::Rect &lcd =
		(isZoomedChromeState() && !_uiclData->emailListContainer.isEmpty())
			? _uiclData->emailListContainer
			: ws;

	// Row text spans from just right of the arrow cursor to a margin
	// inside the LCD's right edge.
	int xLeftScreen, xRightScreen;
	if (_screenState == kWebList) {
		// Search list: a plain left-aligned list — no arrow/icon column.
		xLeftScreen  = lcd.left + 8;
		xRightScreen = lcd.right - 8;
	} else if (!cursor.isEmpty()) {
		xLeftScreen  = cursor.right + 5;
		xRightScreen = lcd.right - 30;
	} else {
		const Common::Rect &arrow = _uiclData->dirArrowSrc;
		xLeftScreen  = ws.left + arrow.width() + 4;
		xRightScreen = lcd.right - 2;
	}

	const int yTopScreen = rowTopScreen() + (int)visibleIndex * pitch;

	const int x = xLeftScreen - _screenPosition.left;
	const int y = yTopScreen - _screenPosition.top;
	const int right = xRightScreen - _screenPosition.left;
	return Common::Rect(x, y, MAX(right, x), y + pitch);
}

bool CellPhonePopup::isContactVisible(const UICL::Contact &c) const {
	const uint16 flag = (uint16)c.unknownPrefix[0] | ((uint16)c.unknownPrefix[1] << 8);
	if (flag == 10) {
		return true;
	}
	if (flag == 11) {
		return false;
	}
	return NancySceneState.getEventFlag((int16)flag, g_nancy->_true);
}

Common::Rect CellPhonePopup::hubEmailRect() const {
	// The Email option button: subButtons[3] before Nancy 13, [4] in Nancy 13.
	const uint slot = g_nancy->getGameType() >= kGameTypeNancy13 ? kN13SubEmail : kSubEmail;
	Common::Rect r = _uiclData->subButtons[slot].destRect;
	r.translate(-_screenPosition.left, -_screenPosition.top);
	return r;
}

Common::Rect CellPhonePopup::hubWebRect() const {
	// The lower LCD option button: Web (subButtons[4]) before Nancy 13, View
	// Pictures (subButtons[3]) in Nancy 13.
	const uint slot = g_nancy->getGameType() >= kGameTypeNancy13 ? kN13SubViewPics : kSubWeb;
	Common::Rect r = _uiclData->subButtons[slot].destRect;
	r.translate(-_screenPosition.left, -_screenPosition.top);
	return r;
}

Common::Rect CellPhonePopup::backLabelHitRect() const {
	// Back overlays the Web/Dir label area. Returns popup-local coordinates.
	Common::Rect hit;
	const Common::Rect &web = _uiclData->webLabel.destRect;
	const Common::Rect &dir = _uiclData->dirLabel.destRect;
	if (!web.isEmpty()) {
		hit = web;
	}
	if (!dir.isEmpty()) {
		if (hit.isEmpty()) {
			hit = dir;
		} else {
			hit.extend(dir);
		}
	}
	if (hit.isEmpty()) {
		return hit;
	}
	hit.translate(-_screenPosition.left, -_screenPosition.top);
	return hit;
}

int CellPhonePopup::contactIndexForVisibleRow(uint visibleRow) const {
	Common::Array<Common::String> seenNames;
	uint visited = 0;
	uint visibleSoFar = 0;
	for (uint i = 0; i < _contacts.size(); ++i) {
		const UICL::Contact &c = _contacts[i];
		if (c.name.empty() || !isContactVisible(c)) {
			continue;
		}
		bool duplicate = false;
		for (uint s = 0; s < seenNames.size(); ++s) {
			if (seenNames[s].equalsIgnoreCase(c.name)) {
				duplicate = true;
				break;
			}
		}
		if (duplicate) {
			continue;
		}
		seenNames.push_back(c.name);
		if (visited < _directoryScroll) {
			++visited;
			continue;
		}
		if (visibleSoFar == visibleRow) {
			return (int)i;
		}
		++visibleSoFar;
		++visited;
	}
	return -1;
}

uint CellPhonePopup::currentListEntryCount() const {
	switch (_screenState) {
	case kDirectory:
		return deduplicatedContactCount();
	case kWebList:
	case kEmailList:
		return listVisibleIndices().size();
	default:
		return 0;
	}
}

void CellPhonePopup::moveDirectorySelection(int delta) {
	if (delta == 0) {
		return;
	}

	const uint total = currentListEntryCount();
	const uint totalRows = maxDirectoryRows();
	const uint titleRows = listTitleRows();
	const uint pageRows = totalRows > titleRows ? totalRows - titleRows : 0;
	if (total == 0 || pageRows == 0) {
		return;
	}

	uint absolute = _directoryScroll + _directorySelection;

	if (delta < 0) {
		const uint dec = (uint)(-delta);
		absolute = dec >= absolute ? 0 : absolute - dec;
	} else {
		absolute += (uint)delta;
		if (absolute >= total) {
			absolute = total - 1;
		}
	}

	if (absolute < _directoryScroll) {
		_directoryScroll = absolute;
		_directorySelection = 0;
	} else if (absolute >= _directoryScroll + pageRows) {
		_directorySelection = pageRows - 1;
		_directoryScroll = absolute - _directorySelection;
	} else {
		_directorySelection = absolute - _directoryScroll;
	}

	drawScreenContent();
}

uint CellPhonePopup::directoryRowAt(const Common::Point &chunkMouse) const {
	const Common::Point popupMouse(chunkMouse.x - _screenPosition.left,
									chunkMouse.y - _screenPosition.top);
	const uint maxRows = maxDirectoryRows();
	for (uint i = 0; i < maxRows; ++i) {
		if (directoryRowRect(i).contains(popupMouse)) {
			return i;
		}
	}
	return (uint)-1;
}

void CellPhonePopup::startCallToContact(uint contactIndex) {
	if (contactIndex >= _contacts.size()) {
		return;
	}
	const UICL::Contact &c = _contacts[contactIndex];

	// Rebuild _dialedNumber so the call flow's lookup matches.
	_dialedNumber.clear();
	for (uint b = 2; b < sizeof(c.unknownPrefix); ++b) {
		const byte v = c.unknownPrefix[b];
		if (v == '\n') {
			break;
		}
		if (v > 9) {
			_dialedNumber.clear();
			break;
		}
		_dialedNumber += (char)('0' + v);
	}
	if (_dialedNumber.empty()) {
		return;
	}
	enterScreenState(kPlaceCall);
}

uint CellPhonePopup::deduplicatedContactCount() const {
	Common::Array<Common::String> seen;
	for (uint i = 0; i < _contacts.size(); ++i) {
		const UICL::Contact &c = _contacts[i];
		if (c.name.empty() || !isContactVisible(c)) {
			continue;
		}
		bool dup = false;
		for (uint s = 0; s < seen.size(); ++s) {
			if (seen[s].equalsIgnoreCase(c.name)) {
				dup = true;
				break;
			}
		}
		if (!dup) {
			seen.push_back(c.name);
		}
	}
	return seen.size();
}

// --------------------------------------------------------------------
// Input
// --------------------------------------------------------------------

Common::Point CellPhonePopup::mouseToChunkCoords(const Common::Point &mouse) const {
	return mouse;
}

void CellPhonePopup::handleInput(NancyInput &input) {
	if (!_isVisible) {
		return;
	}

	// Nancy 13 camera framing: the movable box tracks the mouse; a left click
	// takes the shot, a right click cancels back to the welcome screen.
	if (_inCameraFraming) {
		if (input.mousePos != _framingMouse) {
			_framingMouse = input.mousePos;
			drawScreenContent();
		}
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			const Common::Rect shot = framingScreenRect();
			exitCameraFraming();
			CellPhonePictureData *pd = pictureData();
			if (pd && pd->pictures.size() >= kMaxPictures) {
				// The camera roll is full — the original refuses the shot and
				// asks the player to delete one or more pictures.
				showMessageScreen(kN13MsgCameraFull, kWelcome);
			} else {
				captureViewport(shot);
				enterScreenState(kPictureView);
			}
		} else if (input.input & NancyInput::kRightMouseButtonUp) {
			exitCameraFraming();
			enterScreenState(kWelcome);
		}
		input.eatMouseInput();
		return;
	}

	// Mid-call states accept only the close X.
	const bool transientCallState =
		_screenState == kPlaceCall || _screenState == kWaitOutgoingRing ||
		_screenState == kLookupContact || _screenState == kWaitPickup ||
		_screenState == kConnected || _screenState == kInvalidNumber ||
		_screenState == kWaitInvalid;

	// Close (X) wins on overlap.
	if (_uiclData->header.secondaryButtonEnabled) {
		const UIButtonRecord &closeBtn = _uiclData->header.secondaryButton;
		Common::Rect closeScreen = closeBtn.destRect;
		if (closeBtn.destUsesGameFrameOffset) {
			const VIEW *view = GetEngineData(VIEW);
			if (view) {
				closeScreen.translate(view->screenPosition.left, view->screenPosition.top);
			}
		}
		const bool overClose = closeScreen.contains(input.mousePos);
		if (overClose != _closeButtonHovered) {
			_closeButtonHovered = overClose;
			drawCloseButton(overClose);
			_needsRedraw = true;
		}
		if (overClose) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.eatMouseInput();
				// close() stops the call-sound channel, which the X's click
				// sound may share; close first so the click sound isn't cut off
				// once a call / dial / web tone has occupied that channel.
				close();
				playButtonClickSound(closeBtn);
				return;
			}
		}
	}

	if (transientCallState) {
		// While ringing, only the Back button is live (cancels the call).
		if (isCallBackButtonActive()) {
			const Common::Rect backHit = backButtonHitRect(kSubBack);
			const Common::Point popupMouse(input.mousePos.x - _screenPosition.left,
											input.mousePos.y - _screenPosition.top);
			const bool overBack = !backHit.isEmpty() && backHit.contains(popupMouse);
			if (overBack != _backButtonHovered) {
				_backButtonHovered = overBack;
				drawScreenContent();
			}
			if (overBack) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					input.eatMouseInput();
					cancelCall();
					return;
				}
			}
		}
		// Block the viewport from seeing the cursor (edge-pan, etc.).
		input.eatMouseInput();
		return;
	}

	const Common::Point chunkMouse = mouseToChunkCoords(input.mousePos);

	// Light the up/down arrows on hover in any state that uses them (directory,
	// link lists, the content view — help included, which scrolls via the
	// small-LCD arrow pair — and the Nancy 13 picture-view paging arrows).
	const bool arrowsActive = _screenState == kDirectory || isLinkListMode() ||
								_screenState == kContentView ||
								_screenState == kPictureView;
	const bool overUp = arrowsActive &&
			scrollUpButton().destRect.contains(chunkMouse);
	const bool overDown = arrowsActive && !overUp &&
			scrollDownButton().destRect.contains(chunkMouse);
	if (overUp != _scrollUpHovered || overDown != _scrollDownHovered) {
		_scrollUpHovered = overUp;
		_scrollDownHovered = overDown;
		drawScreenContent();
	}

	// Green-arrow highlight for the captioned "> HELP" and "< BACK" / HOME
	// buttons: swap to the pressed sprite while the cursor is over them.
	const bool helpVisible = (_screenState == kWelcome || _screenState == kDialing);
	const bool overHelp = helpVisible &&
			!_uiclData->helpButton.destRect.isEmpty() &&
			_uiclData->helpButton.destRect.contains(chunkMouse);
	const int backIndex = currentBackButtonIndex();
	const bool overBack = backIndex >= 0 &&
			!_uiclData->subButtons[backIndex].destRect.isEmpty() &&
			_uiclData->subButtons[backIndex].destRect.contains(chunkMouse);
	if (overHelp != _helpButtonHovered || overBack != _backButtonHovered) {
		_helpButtonHovered = overHelp;
		_backButtonHovered = overBack;
		drawScreenContent();
	}

	// The keypad is only on screen in the non-zoomed chrome (welcome / dialing /
	// directory / help), so its slots are only interactive there. The zoomed
	// web / email / browser views hide the keypad but keep the slots' dest rects
	// covering that now-blank area, so without this guard hovering or clicking
	// there would light the underlying key sprites and even switch to dialing.
	const bool keypadVisible = !isZoomedChromeState() || isHelpContentView();

	// Depress the dial-pad key under the cursor while the mouse button is held;
	// clear it on release (this runs before the click handlers so the depressed
	// sprite isn't left behind once the key's action redraws the screen).
	// Skip when the cursor is over a scroll arrow: the arrows overlap the dial
	// pad geometrically (e.g. the down arrow sits over the "#" key), so pressing
	// one would otherwise light the underlying key sprite.
	int newPressed = -1;
	if (keypadVisible && !overUp && !overDown &&
			(input.input & (NancyInput::kLeftMouseButtonDown | NancyInput::kLeftMouseButtonHeld)) &&
			!(input.input & NancyInput::kLeftMouseButtonUp)) {
		for (uint i = 0; i < UICL::kNumDialPadSlots; ++i) {
			if (_uiclData->dialPadSlots[i].destRect.contains(chunkMouse)) {
				newPressed = (int)i;
				break;
			}
		}
	}
	if (newPressed != _pressedSlot) {
		_pressedSlot = newPressed;
		drawScreenContent();
	}

	// Help "?" button: opens the help page in the content view. Hidden
	// (and unclickable) on sub-screens that already show their own heading.
	if (!isSubScreenState() &&
			!_uiclData->helpButton.destRect.isEmpty() && !_uiclData->helpTextKey.empty() &&
			!(_screenState == kContentView && _contentKey == _uiclData->helpTextKey) &&
			_uiclData->helpButton.destRect.contains(chunkMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			// Nancy 13 stores two help keys: the phone-help text lives under
			// helpTextKey2 ("PHNUSE") in AUTOTEXT, not helpTextKey ("NTR025"), so
			// use whichever actually resolves.
			Common::String helpKey = _uiclData->helpTextKey;
			if (g_nancy->getGameType() >= kGameTypeNancy13 && !_uiclData->helpTextKey2.empty()) {
				const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
				const Common::String help2 = _uiclData->helpTextKey2.toString();
				if (autotext && !autotext->texts.contains(helpKey) &&
						autotext->texts.contains(help2)) {
					helpKey = help2;
				}
			}
			openContentView(helpKey, _uiclData->helpHeading);
			input.eatMouseInput();
			return;
		}
	}

	if (_screenState == kDirectory) {
		const Common::Rect &upDst = scrollUpButton().destRect;
		const Common::Rect &downDst = scrollDownButton().destRect;

		// Up/down move the selection; scrolling kicks in at page edges.
		if (upDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				moveDirectorySelection(-1);
				input.eatMouseInput();
				return;
			}
		} else if (downDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				moveDirectorySelection(+1);
				input.eatMouseInput();
				return;
			}
		}

		// Visible Back button at the bottom of the display. Gated so it can't
		// intercept up/down clicks.
		const Common::Rect backHit = backButtonHitRect(kSubBack);
		const Common::Point popupMouse(chunkMouse.x - _screenPosition.left,
										chunkMouse.y - _screenPosition.top);
		const bool overUpDown =
			upDst.contains(chunkMouse) || downDst.contains(chunkMouse);
		if (!overUpDown && !backHit.isEmpty() && backHit.contains(popupMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_directoryScroll = 0;
				_directorySelection = 0;
				_dialedNumber.clear();
				enterScreenState(kWelcome);
				input.eatMouseInput();
				return;
			}
		}

		// Row click only selects; the first bottom button places the call (or,
		// while choosing a photo recipient, sends the picture).
		const uint row = directoryRowAt(chunkMouse);
		if (row != (uint)-1) {
			const int contactIdx = contactIndexForVisibleRow(row);
			if (contactIdx >= 0) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_directorySelection = row;
					drawScreenContent();
					input.eatMouseInput();
					return;
				}
			}
		}
		// Fall through so slot 14 can toggle the mode off and slots 0..9
		// can override the directory by starting a fresh dial.
	}

	// Online hub: two labels — Email and Web — plus the Back hotspot.
	if (_screenState == kOnlineHub) {
		const Common::Point popupMouse(chunkMouse.x - _screenPosition.left,
										chunkMouse.y - _screenPosition.top);
		const Common::Rect emailR = hubEmailRect();
		const Common::Rect webR   = hubWebRect();
		const Common::Rect backHit = backButtonHitRect(kSubBack);

		// Highlight whichever option button the cursor is over.
		const bool n13Hub = g_nancy->getGameType() >= kGameTypeNancy13;
		const int emailSlot = n13Hub ? kN13SubEmail : kSubEmail;
		const int webSlot = n13Hub ? kN13SubViewPics : kSubWeb;
		const int newHubHover = emailR.contains(popupMouse) ? emailSlot
								: webR.contains(popupMouse) ? webSlot : -1;
		if (newHubHover != _hoveredHubButton) {
			_hoveredHubButton = newHubHover;
			drawScreenContent();
		}

		if (emailR.contains(popupMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_directoryScroll = 0;
				_directorySelection = 0;
				enterScreenState(kEmailList);
				input.eatMouseInput();
				return;
			}
		} else if (webR.contains(popupMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_directoryScroll = 0;
				_directorySelection = 0;
				if (g_nancy->getGameType() >= kGameTypeNancy13) {
					// Nancy 13's Menu second option is "view pictures" (the web
					// browser was removed).
					_pictureIndex = 0;
					enterScreenState(kPictureView);
				} else {
					openBrowserHome();
				}
				input.eatMouseInput();
				return;
			}
		} else if (!backHit.isEmpty() && backHit.contains(popupMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				enterScreenState(kWelcome);
				input.eatMouseInput();
				return;
			}
		}
	}

	// Nancy 13 "view pictures" review screen. The top-row buttons are relabelled
	// Cam (retake) / Del / Send; the directory scroll arrows page between photos.
	if (_screenState == kPictureView) {
		const Common::Point popupMouse(chunkMouse.x - _screenPosition.left,
										chunkMouse.y - _screenPosition.top);
		CellPhonePictureData *pd = pictureData();
		const int numPics = pd ? (int)pd->pictures.size() : 0;

		Common::Rect camR = _uiclData->dialPadSlots[12].destRect;
		Common::Rect delR = _uiclData->dialPadSlots[13].destRect;
		Common::Rect sendR = _uiclData->dialPadSlots[14].destRect;
		Common::Rect upR = _uiclData->subButtons[kN13SubListUp].destRect;
		Common::Rect downR = _uiclData->subButtons[kN13SubListDown].destRect;
		const Common::Point origin(_screenPosition.left, _screenPosition.top);
		camR.translate(-origin.x, -origin.y);
		delR.translate(-origin.x, -origin.y);
		sendR.translate(-origin.x, -origin.y);
		upR.translate(-origin.x, -origin.y);
		downR.translate(-origin.x, -origin.y);
		const Common::Rect backHit = backButtonHitRect(kSubBack);

		const bool overButton = camR.contains(popupMouse) ||
			(numPics > 0 && (delR.contains(popupMouse) || sendR.contains(popupMouse))) ||
			(numPics > 1 && (upR.contains(popupMouse) || downR.contains(popupMouse))) ||
			(!backHit.isEmpty() && backHit.contains(popupMouse));
		if (overButton) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
		}
		if (overButton != _backButtonHovered) {
			_backButtonHovered = overButton;
			drawScreenContent();
		}

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (camR.contains(popupMouse)) {
				// Retake — back into the framing overlay.
				_screenState = kCamera;
				enterCameraFraming();
				drawScreenContent();
			} else if (numPics > 0 && delR.contains(popupMouse)) {
				enterScreenState(kDeleteConfirm);
			} else if (numPics > 0 && sendR.contains(popupMouse)) {
				// Pick a recipient in the directory, then confirm "sent".
				_sendingPicture = true;
				_directoryScroll = 0;
				_directorySelection = 0;
				enterScreenState(kDirectory);
			} else if (numPics > 1 && upR.contains(popupMouse)) {
				_pictureIndex = (_pictureIndex + numPics - 1) % numPics;
				drawScreenContent();
			} else if (numPics > 1 && downR.contains(popupMouse)) {
				_pictureIndex = (_pictureIndex + 1) % numPics;
				drawScreenContent();
			} else if (!backHit.isEmpty() && backHit.contains(popupMouse)) {
				enterScreenState(kOnlineHub);
			}
			input.eatMouseInput();
			return;
		}
		input.eatMouseInput();
		return;
	}

	// Nancy 13 delete-confirm ("DELETE? YES OR NO"): Yes and No are the Menu
	// (slot 13) and Dir (slot 14) dial-pad keys relabelled (like slot 12's
	// Cam/Dial/Send), so those slots are the hitboxes. Yes deletes the current
	// photo; No returns to the picture view.
	if (_screenState == kDeleteConfirm) {
		const Common::Point popupMouse(chunkMouse.x - _screenPosition.left,
										chunkMouse.y - _screenPosition.top);
		Common::Rect yesR = _uiclData->dialPadSlots[13].destRect;
		Common::Rect noR = _uiclData->dialPadSlots[14].destRect;
		yesR.translate(-_screenPosition.left, -_screenPosition.top);
		noR.translate(-_screenPosition.left, -_screenPosition.top);
		if (yesR.contains(popupMouse) || noR.contains(popupMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
		}
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			CellPhonePictureData *pd = pictureData();
			if (yesR.contains(popupMouse) && pd && _pictureIndex >= 0 &&
					_pictureIndex < (int)pd->pictures.size()) {
				pd->pictures.remove_at(_pictureIndex);
				if (_pictureIndex >= (int)pd->pictures.size()) {
					_pictureIndex = MAX(0, (int)pd->pictures.size() - 1);
				}
				showMessageScreen(kN13MsgPictureDeleted, kPictureView);
				input.eatMouseInput();
				return;
			}
			if (noR.contains(popupMouse)) {
				enterScreenState(kPictureView);
				input.eatMouseInput();
				return;
			}
		}
		input.eatMouseInput();
		return;
	}

	// Nancy 13 transient message tile (Sent / Deleted / Camera Full): any click
	// (or the Back button) dismisses it back to the state it was raised from.
	if (_screenState == kMessageScreen) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			ScreenState ret = _messageReturnState;
			// If the roll is now empty (last photo deleted), fall back to Menu.
			CellPhonePictureData *pd = pictureData();
			if (ret == kPictureView && (!pd || pd->pictures.empty())) {
				ret = kOnlineHub;
			}
			enterScreenState(ret);
		}
		input.eatMouseInput();
		return;
	}

	// Link-list modes (web search results / email messages). Up/down +
	// Back behave like in directory mode; row clicks navigate to the
	// link's scene and set its event flag.
	if (isLinkListMode()) {
		const Common::Rect &upDst = scrollUpButton().destRect;
		const Common::Rect &downDst = scrollDownButton().destRect;

		if (upDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				moveDirectorySelection(-1);
				input.eatMouseInput();
				return;
			}
		} else if (downDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				moveDirectorySelection(+1);
				input.eatMouseInput();
				return;
			}
		}

		// The list views show a bottom button at the same spot: the email list
		// a BACK (subButtons[7]) to the hub, the search list a HOME
		// (subButtons[9]) to the browser homepage. Nancy 13 moved the zoomed-list
		// Back to subButtons[8] (and dropped the web list). Same dest rect either.
		const uint listBackIdx = g_nancy->getGameType() >= kGameTypeNancy13
			? (uint)kN13SubBackFull
			: (uint)(_screenState == kWebList ? kSubWebHome : kSubEmailBack);
		const Common::Rect backHit = backButtonHitRect(listBackIdx);
		const Common::Point popupMouse(chunkMouse.x - _screenPosition.left,
										chunkMouse.y - _screenPosition.top);
		const bool overUpDown =
			upDst.contains(chunkMouse) || downDst.contains(chunkMouse);
		if (!overUpDown && !backHit.isEmpty() && backHit.contains(popupMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_directoryScroll = 0;
				_directorySelection = 0;
				// Search list returns HOME (the browser homepage); the email
				// list returns to the online hub it was opened from.
				if (_screenState == kWebList) {
					openBrowserHome();
				} else {
					enterScreenState(kOnlineHub);
				}
				input.eatMouseInput();
				return;
			}
		}

		const uint row = directoryRowAt(chunkMouse);
		const uint titleRows = listTitleRows();
		if (row != (uint)-1 && row >= titleRows) {
			const uint entryRow = row - titleRows;
			CellPhoneData *cellData = (CellPhoneData *)NancySceneState.getPuzzleData(CellPhoneData::getTag());
			Common::Array<SearchLink> *list = nullptr;
			if (cellData) {
				list = (_screenState == kWebList) ? &cellData->searchLinks
												  : &cellData->emailMessages;
			}
			// Map the visible row through the active filter to a real index.
			const Common::Array<uint> visible = listVisibleIndices();
			const uint visIdx = _directoryScroll + entryRow;
			if (list && visIdx < visible.size()) {
				const uint absolute = visible[visIdx];
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					// Move the selection indicator to the clicked row, then
					// act on the entry.
					_directorySelection = entryRow;
					SearchLink &e = (*list)[absolute];
					// Original sets the event flag when an entry is opened;
					// it does not scene-change from a list click.
					if (e.eventFlag != -1) {
						NancySceneState.setEventFlag(e.eventFlag, g_nancy->_true);
					}
					if (_screenState == kEmailList && !e.value.empty()) {
						// Flash the opened-envelope icon on this row, then open
						// the message body a beat later (see updateGraphics).
						e.read = true;
						_openingEmailRow = (int)visIdx;
						_openingEmailKey = e.value;
						_openingEmailTime = g_system->getMillis() + 200;
						drawScreenContent();
					} else if (_screenState == kWebList) {
						// AR-131 mode-1 stores a browser-page INDEX in
						// `extra`; the page body lives in the UIBW chunk
						// (UrlPage.imageName is actually the body CVTX
						// key, despite the field name). Fall back to
						// `value`/`key` if the index is out of range.
						Common::String articleKey;
						const UIBW *browserData = GetEngineData(UIBW);
						if (browserData && e.extra >= 0 &&
								(uint)e.extra < browserData->pages.size()) {
							articleKey = browserData->pages[e.extra].imageName.toString();
						}
						if (articleKey.empty()) {
							articleKey = e.value.empty() ? e.key : e.value;
						}
						openContentView(articleKey, _uiclData->browserHeading);
					} else {
						drawScreenContent();
					}
					input.eatMouseInput();
					return;
				}
			}
		}
	}

	// Content view (single email / page text). Up/down scroll the text;
	// Back returns to the list it was opened from.
	if (_screenState == kContentView) {
		// In-page hyperlinks first so they take priority over any
		// overlapping fallthrough hit (e.g. the back hotspot).
		const Common::Point popupMouseLink(chunkMouse.x - _screenPosition.left,
											chunkMouse.y - _screenPosition.top);

		// The main browser page carries the top-row SEARCH button (subButtons[8])
		// which opens the search list; it highlights green while hovered.
		if (_contentHeading == &_uiclData->browserHeading && !isBrowserArticle() &&
				!_uiclData->subButtons[kSubSearch].destRect.isEmpty()) {
			Common::Rect searchBtn = _uiclData->subButtons[kSubSearch].destRect;
			searchBtn.translate(-_screenPosition.left, -_screenPosition.top);
			const int newHover = searchBtn.contains(popupMouseLink) ? kSubSearch : -1;
			if (newHover != _hoveredHubButton) {
				_hoveredHubButton = newHover;
				drawScreenContent();
			}
			if (searchBtn.contains(popupMouseLink)) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_directoryScroll = 0;
					_directorySelection = 0;
					enterScreenState(kWebList);
					input.eatMouseInput();
					return;
				}
			}
		}

		for (uint i = 0; i < _contentHotspots.size(); ++i) {
			if (_contentHotspots[i].isEmpty()) {
				continue;
			}
			if (_contentHotspots[i].contains(popupMouseLink)) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					if (i < _contentHotspotTargets.size() &&
							!_contentHotspotTargets[i].empty()) {
						openContentView(_contentHotspotTargets[i],
										_uiclData->browserHeading);
					}
					input.eatMouseInput();
					return;
				}
				break;
			}
		}

		// scrollUpButton()/scrollDownButton() return the right pair for help
		// (subButtons[1]/[2]) or the zoomed articles ([5]/[6]).
		const Common::Rect &upDst = scrollUpButton().destRect;
		const Common::Rect &downDst = scrollDownButton().destRect;

		const uint step = contentScrollStep();
		if (upDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if (_contentScroll > 0) {
					_contentScroll = _contentScroll > step ? _contentScroll - step : 0;
					drawScreenContent();
				}
				input.eatMouseInput();
				return;
			}
		} else if (downDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_contentScroll += step;
				drawScreenContent();
				input.eatMouseInput();
				return;
			}
		}
		const bool overUpDown =
			upDst.contains(chunkMouse) || downDst.contains(chunkMouse);

		// Hit-test the bottom button that this view actually draws.
		const bool onBrowserArticle = isBrowserArticle();
		const Common::Rect backHit = backButtonHitRect(contentViewBottomButton());
		const Common::Point popupMouse(chunkMouse.x - _screenPosition.left,
										chunkMouse.y - _screenPosition.top);
		if (!overUpDown && !backHit.isEmpty() && backHit.contains(popupMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_contentKey.clear();
				_contentHeading = nullptr;
				if (onBrowserArticle) {
					// HOME → the browser homepage (River Heights Wireless).
					openBrowserHome();
				} else {
					enterScreenState(_contentReturnState);
				}
				input.eatMouseInput();
				return;
			}
		}
	}

	// Call/talk button. Checked before the dial-pad loop so an overlapping
	// slot can't eat it. The Talk key is dial-pad slot 12. Only live while the
	// keypad is on screen (skipped in the zoomed web / email / browser views).
	if (keypadVisible && _uiclData->dialPadSlots[12].destRect.contains(chunkMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			playDialPadSound(_uiclData->dialPadSlots[12].soundName);
			if (g_nancy->getGameType() >= kGameTypeNancy13 &&
					(_screenState == kWelcome || _screenState == kDialing)) {
				// Nancy 13's slot 12 is dual-purpose (Ghidra widget 0xc): on the
				// welcome / dialing screen it is the camera button — enter the
				// framing overlay (the phone yields to the viewport and a movable
				// box marks the shot). In the directory it dials instead, so fall
				// through to the call logic below.
				_screenState = kCamera;
				enterCameraFraming();
				drawScreenContent();
				input.eatMouseInput();
				return;
			}
			if (_sendingPicture && _screenState == kDirectory) {
				// Choosing a photo recipient: the first button is "Send" — mark
				// the picture sent and show the confirmation.
				_sendingPicture = false;
				CellPhonePictureData *pd = pictureData();
				if (pd && _pictureIndex >= 0 && _pictureIndex < (int)pd->pictures.size()) {
					pd->pictures[_pictureIndex].sent = true;
				}
				showMessageScreen(kN13MsgPictureSent, kPictureView);
				input.eatMouseInput();
				return;
			}
			if (!_noSignal) {
				if (_screenState == kDirectory) {
					const int contactIdx =
						contactIndexForVisibleRow(_directorySelection);
					if (contactIdx >= 0) {
						// Pre-resolve so kLookupContact skips the dial-buffer
						// match and the ring/pickup animation still plays.
						_resolvedContact = contactIdx;
						// Defer until the Talk key's tone finishes (see
						// updateGraphics).
						_autoDialPending = true;
					}
				} else if (!_dialedNumber.empty()) {
					_autoDialPending = true;
				}
			}
			input.eatMouseInput();
			return;
		}
	}

	// Dial-pad slot behaviour:
	//   0..9   - digit input
	//   10, 11 - *, # (no-op)
	//   12     - call/talk key, or Nancy 13 camera/dial/send (handled above)
	//   13     - Menu: opens the online hub (e-mail + web / view pictures)
	//   14     - directory toggle
	int newHovered = -1;
	if (keypadVisible) {
		for (uint i = 0; i < UICL::kNumDialPadSlots; ++i) {
			const UICL::DialPadSlot &slot = _uiclData->dialPadSlots[i];
			if (slot.destRect.contains(chunkMouse)) {
				newHovered = (int)i;
				break;
			}
		}
	}
	_hoveredSlot = newHovered;

	if (newHovered != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			const UICL::DialPadSlot &slot = _uiclData->dialPadSlots[newHovered];

			playDialPadSound(slot.soundName);

			if (newHovered < 10) {
				if (_screenState == kDirectory || isLinkListMode()) {
					_dialedNumber.clear();
				}
				appendDigit((byte)newHovered);
			} else if (newHovered == 13) {
				// Opens the Email/Web hub. Re-pressing does not toggle back to
				// the welcome screen — the on-screen Back button does that.
				_dialedNumber.clear();
				_directoryScroll = 0;
				_directorySelection = 0;
				enterScreenState(kOnlineHub);
			} else if (newHovered == 14) {
				_dialedNumber.clear();
				_directoryScroll = 0;
				_directorySelection = 0;
				enterScreenState(kDirectory);
			}
			input.eatMouseInput();
			return;
		}
	}

	// Nothing else claimed the click: block the viewport from acting on the
	// cursor while the phone is up.
	input.eatMouseInput();
}

} // End of namespace UI
} // End of namespace Nancy
