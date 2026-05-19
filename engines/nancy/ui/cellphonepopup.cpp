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

#include "engines/nancy/cursor.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/cellphonepopup.h"

namespace Nancy {
namespace UI {

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
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getInputPixelFormat());

	_screenState = kWelcome;
	_dialedNumber.clear();
	_resolvedContact = -1;
	_directoryScroll = 0;

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
	if (_noSignal == noSignal) {
		return;
	}
	_noSignal = noSignal;
	if (_isVisible) {
		drawScreenContent();
	}
}

void CellPhonePopup::open() {
	if (_isVisible) {
		return;
	}

	_screenState = kWelcome;
	_dialedNumber.clear();
	_resolvedContact = -1;
	_directoryScroll = 0;
	_closeButtonHovered = false;

	drawChrome();
	drawScreenContent();
	setVisible(true);

	if (!_uiclData->header.sounds[0].name.empty()) {
		g_nancy->_sound->loadSound(_uiclData->header.sounds[0]);
		g_nancy->_sound->playSound(_uiclData->header.sounds[0]);
	}
}

void CellPhonePopup::close() {
	if (!_isVisible) {
		return;
	}

	if (!_callSound.name.empty()) {
		g_nancy->_sound->stopSound(_callSound);
	}

	setVisible(false);

	if (!_uiclData->header.sounds[1].name.empty()) {
		g_nancy->_sound->loadSound(_uiclData->header.sounds[1]);
		g_nancy->_sound->playSound(_uiclData->header.sounds[1]);
	}
}

void CellPhonePopup::updateGraphics() {
	if (!_isVisible) {
		return;
	}

	// TODO: Process states 0xa..0x10 (web/email/search/help/browser)
	// are not implemented — the corresponding screen states are
	// missing from the enum too.
	switch (_screenState) {
	case kWelcome:
	case kDialing:
	case kDirectory:
		break;

	case kPlaceCall:
		if (playSoundIfPresent(_uiclData->outgoingRingSound)) {
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
		_resolvedContact = findContactByDialBuffer();
		if (_resolvedContact == -1) {
			enterScreenState(kInvalidNumber);
		} else if (playSoundIfPresent(_uiclData->pickupSound)) {
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
		if (_resolvedContact >= 0 &&
				_resolvedContact < (int)_uiclData->contacts.size()) {
			triggerContactCallSceneChange((uint)_resolvedContact);
		}
		_resolvedContact = -1;
		resetDialPad();
		enterScreenState(kWelcome);
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
	// Chrome reads from the primary overlay; all other blits use _spritesImage.
	_drawSurface.blitFrom(_overlayImage, _uiclData->header.normalSrcRect,
							Common::Point(0, 0));
	drawCloseButton(_closeButtonHovered ? 1 : 0);
	drawWebDirLabels();
	drawCallButtonState(0);
	_needsRedraw = true;
}

void CellPhonePopup::drawScreenContent() {
	// Repaint chrome (clears the LCD strip), then layer state-specific
	// content on top.
	drawChrome();

	if (_screenState != kConnected) {
		drawStatusIcons();
	}

	switch (_screenState) {
	case kWelcome:
		// Welcome graphic is baked into the chrome; only the status-label
		// overlay is layered on top when the carrier is unreachable.
		if (_noSignal) {
			drawStatusLabels();
		}
		break;

	case kDialing:
	case kPlaceCall:
	case kWaitOutgoingRing:
	case kLookupContact:
		drawDialLabel();
		drawTypeMessage();
		drawDialedNumber();
		break;

	case kWaitPickup:
	case kConnected:
		drawConnectedLabel();
		drawConnectingSprite();
		break;

	case kInvalidNumber:
	case kWaitInvalid:
		drawConnectingSprite();
		break;

	case kDirectory:
		drawDirHeading();
		drawDirectoryList();
		drawDirectoryArrows();
		break;
	}

	_needsRedraw = true;
}

void CellPhonePopup::drawStatusIcons() {
	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);

	// Signal indicator uses the alt source when no-signal is set.
	const Common::Rect &signalSrc = _noSignal && !_uiclData->signalSpriteSrcAlt.isEmpty()
		? _uiclData->signalSpriteSrcAlt
		: _uiclData->signalSpriteSrc;
	if (!signalSrc.isEmpty() && !_uiclData->signalSpriteDest.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, signalSrc,
								Common::Point(_uiclData->signalSpriteDest.left - chunkOrigin.x,
												_uiclData->signalSpriteDest.top - chunkOrigin.y));
	}

	if (!_uiclData->batterySpriteSrc.isEmpty() && !_uiclData->batterySpriteDest.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, _uiclData->batterySpriteSrc,
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
	const UICL::SrcDestRectPair &dl = _uiclData->dialLabel;
	if (dl.srcRect.isEmpty() || dl.destRect.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, dl.srcRect,
							Common::Point(dl.destRect.left - chunkOrigin.x,
											dl.destRect.top - chunkOrigin.y));
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
	if (_uiclData->connectingSpriteSrc.isEmpty() || _uiclData->connectingSpriteDest.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, _uiclData->connectingSpriteSrc,
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

void CellPhonePopup::drawCallButtonState(uint state) {
	const UICL::ThreeRectWidget &cb = _uiclData->callButton;
	if (cb.destRect.isEmpty()) {
		return;
	}

	const Common::Rect &src = (state == 1 && !cb.srcRectPressed.isEmpty())
								? cb.srcRectPressed
								: cb.srcRectIdle;
	if (src.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, src,
							Common::Point(cb.destRect.left - chunkOrigin.x,
											cb.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawCloseButton(uint state) {
	const UIButtonRecord &btn = _uiclData->header.secondaryButton;
	if (!_uiclData->header.secondaryButtonEnabled || btn.destRect.isEmpty()) {
		return;
	}

	const uint stateIdx = MIN<uint>(state, 3);
	Common::Rect src = btn.sourceRects[stateIdx];
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
	// Three stacked lines (No Signal / No Access / Old Email Only)
	// at statusTextX with Y offsets -10 / +20 / +50.
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

void CellPhonePopup::drawDirHeading() {
	const UICL::SrcDestRectPair &dh = _uiclData->dirHeading;
	if (dh.srcRect.isEmpty() || dh.destRect.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, dh.srcRect,
							Common::Point(dh.destRect.left - chunkOrigin.x,
											dh.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawDirectoryList() {
	// The chunk stores one entry per dial-pattern variant, so several
	// entries can share a name. Collapse by name to avoid showing the
	// same contact multiple times.
	// TODO: also sort by name and respect per-entry visibility flags,
	// the way the original engine's contact-filter pass does.
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return;
	}

	const uint maxRows = maxDirectoryRows();
	Common::Array<Common::String> seenNames;
	uint visibleRow = 0;
	uint visited = 0;

	for (uint contactIdx = 0;
			contactIdx < _uiclData->contacts.size() && visibleRow < maxRows;
			++contactIdx) {
		const UICL::Contact &c = _uiclData->contacts[contactIdx];
		if (c.name.empty()) {
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

		// _directoryScroll counts deduplicated entries.
		if (visited < _directoryScroll) {
			++visited;
			continue;
		}
		++visited;

		const Common::Rect rowRect = directoryRowRect(visibleRow);
		font->drawString(&_drawSurface, c.name,
							rowRect.left, rowRect.top,
							rowRect.width(), 0);
		++visibleRow;
	}
}

void CellPhonePopup::drawDirectoryArrows() {
	// Sub-buttons 1 and 2 carry the up/down scroll arrows for list modes.
	const UICL::ThreeRectWidget &up = _uiclData->subButtons[1];
	const UICL::ThreeRectWidget &down = _uiclData->subButtons[2];

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);

	if (!up.srcRectIdle.isEmpty() && !up.destRect.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, up.srcRectIdle,
								Common::Point(up.destRect.left - chunkOrigin.x,
												up.destRect.top - chunkOrigin.y));
	}
	if (!down.srcRectIdle.isEmpty() && !down.destRect.isEmpty()) {
		_drawSurface.blitFrom(_spritesImage, down.srcRectIdle,
								Common::Point(down.destRect.left - chunkOrigin.x,
												down.destRect.top - chunkOrigin.y));
	}
}

// --------------------------------------------------------------------
// State-machine helpers
// --------------------------------------------------------------------

void CellPhonePopup::resetDialPad() {
	_dialedNumber.clear();
}

void CellPhonePopup::enterScreenState(ScreenState newState) {
	// Always redraw, even when newState == _screenState, so successive
	// digit entries in kDialing each refresh the readout.
	_screenState = newState;
	drawScreenContent();
}

void CellPhonePopup::appendDigit(byte slotIndex) {
	// 11-char cap matches the LCD readout buffer width.
	if (_dialedNumber.size() >= 11) {
		return;
	}
	_dialedNumber += (char)('0' + slotIndex);
	enterScreenState(kDialing);
}

bool CellPhonePopup::playSoundIfPresent(const Common::Path &soundName) {
	if (soundName.empty()) {
		return false;
	}
	Common::String nameStr = soundName.toString();
	if (nameStr.equalsIgnoreCase("NO SOUND")) {
		return false;
	}

	_callSound.name = nameStr;
	if (_callSound.channelID == 0) {
		// TODO: read the channel from the chunk's per-call channel slot
		// instead of hardcoding it.
		_callSound.channelID = 28;
	}
	_callSound.volume = 100;
	_callSound.numLoops = 1;
	_callSound.playCommands = 1;

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

	// Contact prefix bytes are slot indices (0..9); _dialedNumber holds
	// '0'..'9' chars, so convert each char back to its slot index.
	const uint dialLen = _dialedNumber.size();
	for (uint i = 0; i < _uiclData->contacts.size(); ++i) {
		const UICL::Contact &c = _uiclData->contacts[i];
		bool match = true;
		for (uint b = 0; b < dialLen; ++b) {
			const byte slotIdx = (byte)(_dialedNumber[b] - '0');
			if (slotIdx != c.unknownPrefix[b]) {
				match = false;
				break;
			}
		}
		// Require the prefix to terminate at the dialed length so a
		// partial dial doesn't match a longer number.
		if (match && dialLen < sizeof(c.unknownPrefix) &&
				c.unknownPrefix[dialLen] == '\n') {
			return (int)i;
		}
	}
	return -1;
}

void CellPhonePopup::triggerContactCallSceneChange(uint contactIndex) {
	if (contactIndex >= _uiclData->contacts.size()) {
		return;
	}

	// Suffix layout: [0..1]=sceneID, [4..5]=eventFlag label, [6]=eventFlag value.
	const UICL::Contact &c = _uiclData->contacts[contactIndex];

	const uint16 sceneID = (uint16)c.unknownSuffix[0] | ((uint16)c.unknownSuffix[1] << 8);
	if (sceneID == 9999) {
		return; // "no scene" sentinel
	}
	const int16 eventFlagLabel = (int16)((uint16)c.unknownSuffix[4] |
											((uint16)c.unknownSuffix[5] << 8));
	const byte eventFlagValue = c.unknownSuffix[6];

	SceneChangeDescription scene;
	scene.sceneID = sceneID;
	scene.frameID = 0;
	scene.verticalOffset = 0;
	scene.continueSceneSound = kContinueSceneSound;

	if (eventFlagLabel != -1) {
		NancySceneState.setEventFlag(eventFlagLabel,
										eventFlagValue ? g_nancy->_true : g_nancy->_false);
	}

	NancySceneState.changeScene(scene);

	setVisible(false);
}

// --------------------------------------------------------------------
// Directory helpers
// --------------------------------------------------------------------

uint CellPhonePopup::maxDirectoryRows() const {
	// fontId2 is the list font; fontId1 is the (larger) dial readout.
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return 0;
	}

	const Common::Rect &arrow = _uiclData->dirArrowSrc;
	if (arrow.isEmpty()) {
		return 0;
	}

	// TODO: derive the real visible-row count from the layout
	// FUN_004d8476 sets up at directory entry (case 0xe). For now,
	// cap at the LCD height divided by row pitch.
	const int rowH = arrow.height() + 8;
	if (rowH <= 0) {
		return 0;
	}
	// Approximate LCD strip height as welcomeScreen height.
	const int lcdH = _uiclData->welcomeScreen.destRect.height();
	return MAX<int>(1, lcdH / rowH);
}

Common::Rect CellPhonePopup::directoryRowRect(uint visibleIndex) const {
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return Common::Rect();
	}

	// Row layout: X start = dirArrowSrc.right + 5,
	//             Y start = dirArrowSrc.top - 5,
	//             pitch  = dirArrowSrc.height() + 8.
	const Common::Rect &arrow = _uiclData->dirArrowSrc;
	const int rowH = arrow.height() + 8;
	const int xScreen = arrow.right + 5;
	const int yScreen = arrow.top - 5 + (int)visibleIndex * rowH;
	const int x = xScreen - _screenPosition.left;
	const int y = yScreen - _screenPosition.top;
	const int width = _screenPosition.width() - x - 4;
	return Common::Rect(x, y, x + width, y + rowH);
}

int CellPhonePopup::contactIndexForVisibleRow(uint visibleRow) const {
	// Mirrors the dedup walk in drawDirectoryList.
	Common::Array<Common::String> seenNames;
	uint visited = 0;
	uint visibleSoFar = 0;
	for (uint i = 0; i < _uiclData->contacts.size(); ++i) {
		const UICL::Contact &c = _uiclData->contacts[i];
		if (c.name.empty()) {
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

uint CellPhonePopup::directoryRowAt(const Common::Point &chunkMouse) const {
	// directoryRowRect returns popup-local rects; convert the mouse too.
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
	if (contactIndex >= _uiclData->contacts.size()) {
		return;
	}
	const UICL::Contact &c = _uiclData->contacts[contactIndex];

	// Rebuild _dialedNumber from the contact prefix (slot indices,
	// terminated by '\n') so the call flow's lookup matches.
	_dialedNumber.clear();
	for (uint b = 0; b < sizeof(c.unknownPrefix); ++b) {
		const byte v = c.unknownPrefix[b];
		if (v == '\n') {
			break;
		}
		if (v > 9) {
			// Non-digit slot index: abort so the call hits kInvalidNumber.
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
	for (uint i = 0; i < _uiclData->contacts.size(); ++i) {
		const UICL::Contact &c = _uiclData->contacts[i];
		if (c.name.empty()) {
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
	// Chunk destRects are screen coords, so chunk-mouse == screen-mouse.
	return mouse;
}

void CellPhonePopup::handleInput(NancyInput &input) {
	if (!_isVisible) {
		return;
	}

	// Transient call-flow states ignore everything except the close X.
	const bool transientCallState =
		_screenState == kPlaceCall || _screenState == kWaitOutgoingRing ||
		_screenState == kLookupContact || _screenState == kWaitPickup ||
		_screenState == kConnected || _screenState == kInvalidNumber ||
		_screenState == kWaitInvalid;

	// Close (X) — checked first so it wins overlap.
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
			drawCloseButton(overClose ? 1 : 0);
			_needsRedraw = true;
		}
		if (overClose) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.eatMouseInput();
				close();
				return;
			}
		}
	}

	if (transientCallState) {
		if (_screenPosition.contains(input.mousePos)) {
			input.eatMouseInput();
		}
		return;
	}

	const Common::Point chunkMouse = mouseToChunkCoords(input.mousePos);

	// Directory mode: scroll arrows + click-to-call on a contact row.
	if (_screenState == kDirectory) {
		const Common::Rect &upDst = _uiclData->subButtons[1].destRect;
		const Common::Rect &downDst = _uiclData->subButtons[2].destRect;

		if (upDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if (_directoryScroll > 0) {
					--_directoryScroll;
					drawScreenContent();
				}
				input.eatMouseInput();
				return;
			}
		} else if (downDst.contains(chunkMouse)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				const uint total = deduplicatedContactCount();
				const uint rows = maxDirectoryRows();
				const uint maxScroll = total > rows ? total - rows : 0;
				if (_directoryScroll < maxScroll) {
					++_directoryScroll;
					drawScreenContent();
				}
				input.eatMouseInput();
				return;
			}
		}

		const uint row = directoryRowAt(chunkMouse);
		if (row != (uint)-1) {
			const int contactIdx = contactIndexForVisibleRow(row);
			if (contactIdx >= 0) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					startCallToContact((uint)contactIdx);
					input.eatMouseInput();
					return;
				}
			}
		}
		// Fall through so slot 14 can toggle the mode off and slots 0..9
		// can override the directory by starting a fresh dial.
	}

	// Dial-pad slot behaviour:
	//   0..9   - digit input
	//   10, 11 - *, # (no-op for now)
	//   12     - welcome / dial toggle (TODO: hook up)
	//   13     - web mode (TODO: not implemented)
	//   14     - directory toggle
	int newHovered = -1;
	for (uint i = 0; i < UICL::kNumDialPadSlots; ++i) {
		const UICL::DialPadSlot &slot = _uiclData->dialPadSlots[i];
		if (slot.destRect.contains(chunkMouse)) {
			newHovered = (int)i;
			break;
		}
	}
	_hoveredSlot = newHovered;

	if (newHovered != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			const UICL::DialPadSlot &slot = _uiclData->dialPadSlots[newHovered];

			if (!slot.soundName.empty()) {
				g_nancy->_sound->playSound(slot.soundName);
			}

			if (newHovered < 10) {
				if (_screenState == kDirectory) {
					_dialedNumber.clear();
				}
				appendDigit((byte)newHovered);
			} else if (newHovered == 14) {
				if (_screenState == kDirectory) {
					_directoryScroll = 0;
					enterScreenState(kWelcome);
				} else {
					_dialedNumber.clear();
					_directoryScroll = 0;
					enterScreenState(kDirectory);
				}
			}
			// TODO: slots 10/11 (*, #), slot 12 (mode toggle), slot 13
			// (web mode) are unimplemented; they fall through as no-ops.

			input.eatMouseInput();
			return;
		}
	}

	// Call button — disabled while in no-signal mode.
	if (_uiclData->callButton.destRect.contains(chunkMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (!_dialedNumber.empty() && !_noSignal) {
				enterScreenState(kPlaceCall);
			}
			input.eatMouseInput();
			return;
		}
	}

	// TODO: sub-buttons (10 entries in _uiclData->subButtons) are not
	// hooked up — they drive email/search/help/browser modes and the
	// in-call menu in the original.

	// Swallow any remaining click so it doesn't fall through to the scene.
	if (_screenPosition.contains(input.mousePos)) {
		input.eatMouseInput();
	}
}

} // End of namespace UI
} // End of namespace Nancy
