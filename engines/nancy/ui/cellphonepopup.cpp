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
	_directorySelection = 0;
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

	// TODO: web/email/search/help/browser modes not implemented.
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
		// Directory-mode calls pre-resolve the contact, so only fall back
		// to dial-buffer lookup when the contact isn't already known.
		if (_resolvedContact == -1) {
			_resolvedContact = findContactByDialBuffer();
		}
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
		// Trigger the scene change once, then sit in kConnected so the
		// connecting sprite stays on screen for the duration of the
		// conversation. AR 128 closes the popup when the call ends.
		if (_resolvedContact >= 0 &&
				_resolvedContact < (int)_uiclData->contacts.size()) {
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
	_drawSurface.blitFrom(_overlayImage, _uiclData->header.normalSrcRect,
							Common::Point(0, 0));
	drawCloseButton(_closeButtonHovered ? 1 : 0);
	drawCallButtonState(0);
	_needsRedraw = true;
}

void CellPhonePopup::drawScreenContent() {
	drawChrome();

	if (_screenState != kConnected) {
		drawStatusIcons();
	}

	switch (_screenState) {
	case kWelcome:
		drawWebDirLabels();
		if (_noSignal) {
			drawStatusLabels();
		} else {
			drawWelcomeScreen();
		}
		break;

	case kDialing:
	case kPlaceCall:
	case kWaitOutgoingRing:
	case kLookupContact:
		drawWebDirLabels();
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
			contactIdx < _uiclData->contacts.size() && visibleRow < maxRows;
			++contactIdx) {
		const UICL::Contact &c = _uiclData->contacts[contactIdx];
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
		font->drawString(&_drawSurface, c.name,
							rowRect.left, rowRect.top,
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

void CellPhonePopup::drawBackLabel() {
	const UICL::ThreeRectWidget &back = _uiclData->subButtons[2];
	if (back.srcRectIdle.isEmpty() || back.destRect.isEmpty()) {
		return;
	}

	const Common::Point chunkOrigin(_screenPosition.left, _screenPosition.top);
	_drawSurface.blitFrom(_spritesImage, back.srcRectIdle,
							Common::Point(back.destRect.left - chunkOrigin.x,
											back.destRect.top - chunkOrigin.y));
}

void CellPhonePopup::drawDirectoryArrows() {
	// Up/down scroll arrows are not in the chrome image; blit on every redraw.
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

	// Selection indicator next to the active row.
	const Common::Rect &arrowSrc = _uiclData->dirArrowSrc;
	if (arrowSrc.isEmpty()) {
		return;
	}
	const uint maxRows = maxDirectoryRows();
	if (_directorySelection >= maxRows) {
		return;
	}
	const Common::Rect rowRect = directoryRowRect(_directorySelection);
	const int arrowX = MAX(0, rowRect.left - arrowSrc.width() - 2);
	const int arrowY = rowRect.top;
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
	// Always redraw, so successive digit entries refresh the readout.
	_screenState = newState;
	drawScreenContent();
}

void CellPhonePopup::appendDigit(byte slotIndex) {
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
		// TODO: should come from the per-call channel slot in the chunk.
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

	// Dial pattern lives in prefix[2..], terminated by '\n'.
	const uint dialLen = _dialedNumber.size();
	const uint kDialOffset = 2;
	for (uint i = 0; i < _uiclData->contacts.size(); ++i) {
		const UICL::Contact &c = _uiclData->contacts[i];
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
	if (contactIndex >= _uiclData->contacts.size()) {
		return;
	}

	const UICL::Contact &c = _uiclData->contacts[contactIndex];

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

	// Pushed so AR 128 in the conversation scene can return the player here.
	NancySceneState.pushScene();

	NancySceneState.changeScene(scene);

	// Phone stays on screen through the conversation; AR 128 closes it.
}

// --------------------------------------------------------------------
// Directory helpers
// --------------------------------------------------------------------

uint CellPhonePopup::maxDirectoryRows() const {
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return 0;
	}

	const Common::Rect &arrow = _uiclData->dirArrowSrc;
	if (arrow.isEmpty()) {
		return 0;
	}

	const int rowH = MAX(arrow.height() + 4, 14);
	const Common::Rect &ws = _uiclData->welcomeScreen.destRect;
	const int firstRowOffset = 22;
	const int lcdH = MAX(0, ws.height() - firstRowOffset);
	return MAX<int>(1, lcdH / rowH);
}

Common::Rect CellPhonePopup::directoryRowRect(uint visibleIndex) const {
	const Font *font = g_nancy->_graphics->getFont(_uiclData->fontId2);
	if (!font) {
		return Common::Rect();
	}

	const Common::Rect &arrow = _uiclData->dirArrowSrc;
	const int rowH = MAX(arrow.height() + 4, 14);
	const Common::Rect &ws = _uiclData->welcomeScreen.destRect;
	const int lcdLeft = ws.left - _screenPosition.left;
	const int lcdTop  = ws.top  - _screenPosition.top;
	const int lcdWidth = ws.width();
	const int firstRowOffset = 22;

	const int x = lcdLeft + arrow.width() + 4;
	const int y = lcdTop + firstRowOffset + (int)visibleIndex * rowH;
	const int width = MAX(0, lcdWidth - (arrow.width() + 4) - 2);
	// Clamp to the LCD so a row rect can't leak onto the keypad below.
	const int lcdBottom = lcdTop + ws.height();
	const int rowBottom = MIN(y + rowH, lcdBottom);
	if (rowBottom <= y) {
		return Common::Rect();
	}
	return Common::Rect(x, y, x + width, rowBottom);
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
	for (uint i = 0; i < _uiclData->contacts.size(); ++i) {
		const UICL::Contact &c = _uiclData->contacts[i];
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

void CellPhonePopup::moveDirectorySelection(int delta) {
	if (_screenState != kDirectory || delta == 0) {
		return;
	}

	const uint total = deduplicatedContactCount();
	const uint maxRows = maxDirectoryRows();
	if (total == 0 || maxRows == 0) {
		return;
	}

	uint absolute = _directoryScroll + _directorySelection;

	if (delta < 0) {
		const uint dec = (uint)(-delta);
		if (dec >= absolute) {
			absolute = 0;
		} else {
			absolute -= dec;
		}
	} else {
		absolute += (uint)delta;
		if (absolute >= total) {
			absolute = total - 1;
		}
	}

	if (absolute < _directoryScroll) {
		_directoryScroll = absolute;
		_directorySelection = 0;
	} else if (absolute >= _directoryScroll + maxRows) {
		_directorySelection = maxRows - 1;
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
	if (contactIndex >= _uiclData->contacts.size()) {
		return;
	}
	const UICL::Contact &c = _uiclData->contacts[contactIndex];

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
	for (uint i = 0; i < _uiclData->contacts.size(); ++i) {
		const UICL::Contact &c = _uiclData->contacts[i];
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
		// Block the viewport from seeing the cursor (edge-pan, etc.).
		input.eatMouseInput();
		return;
	}

	const Common::Point chunkMouse = mouseToChunkCoords(input.mousePos);

	if (_screenState == kDirectory) {
		const Common::Rect &upDst = _uiclData->subButtons[1].destRect;
		const Common::Rect &downDst = _uiclData->subButtons[2].destRect;

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

		// Invisible Back hotspot. Gated so it can't intercept up/down clicks.
		const Common::Rect backHit = backLabelHitRect();
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

		// Row click selects (the call button does the actual call).
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

	// Call/talk button. Checked before the dial-pad loop so an overlapping
	// slot can't eat it. The keypad Talk key is slot 12; the callButton
	// widget covers a different region.
	if (_uiclData->callButton.destRect.contains(chunkMouse) ||
			_uiclData->dialPadSlots[12].destRect.contains(chunkMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (!_noSignal) {
				if (_screenState == kDirectory) {
					const int contactIdx =
						contactIndexForVisibleRow(_directorySelection);
					if (contactIdx >= 0) {
						// Pre-resolve so kLookupContact skips the dial-buffer
						// match and the ring/pickup animation still plays.
						_resolvedContact = contactIdx;
						enterScreenState(kPlaceCall);
					}
				} else if (!_dialedNumber.empty()) {
					enterScreenState(kPlaceCall);
				}
			}
			input.eatMouseInput();
			return;
		}
	}

	// Dial-pad slot behaviour:
	//   0..9   - digit input
	//   10, 11 - *, # (no-op)
	//   12     - call/talk key (handled above)
	//   13     - web mode (TODO)
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
					_directorySelection = 0;
					enterScreenState(kWelcome);
				} else {
					_dialedNumber.clear();
					_directoryScroll = 0;
					_directorySelection = 0;
					enterScreenState(kDirectory);
				}
			}
			input.eatMouseInput();
			return;
		}
	}

	// TODO: sub-buttons (email/search/help/browser/in-call menu) not hooked up.

	// Block the viewport from acting on the cursor while the phone is up.
	input.eatMouseInput();
}

} // End of namespace UI
} // End of namespace Nancy
