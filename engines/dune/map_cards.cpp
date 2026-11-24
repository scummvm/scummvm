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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dune/dune.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/system.h"
#include "graphics/cursorman.h"

namespace Dune {

// DNVGA 2572 normally samples retrace status. Its three-tick timer is a
// fallback. Use the cadence observed in the recorded VGA map transitions.
static constexpr uint32 kMapOutlineFrameMillis = 7;

DuneEngine::PanelMenuAction::PanelMenuAction(DuneEngine &engine) : _engine(engine) {
	_engine._panelMenuDispatchDepth++;
}

DuneEngine::PanelMenuAction::~PanelMenuAction() {
	// A person with no applicable line still gets the newly prepared menu.
	// Most dialogue flushes when its voiced portrait is composed; this is the
	// input handler's fallback, after nested choices and early returns unwind.
	if (--_engine._panelMenuDispatchDepth == 0 && !_engine._panelMenuOldPage.empty()) {
		_engine.redraw();
		_engine.finishPanelMenuTransition();
	}
}

/** D316: explicit menu-stack callers arm the roll, not ordinary repaints. */
void DuneEngine::armPanelMenuTransition() {
	// C950/CA01's retained film stream suppresses arming. Native flight and
	// partial-title readers retain that same logical ownership without DOS
	// file handles; do not infer an open stream merely from being on the map.
	if (!READ_LE_UINT16(_state.begin() + 0x35a6) && _flightMovie < 0 &&
			_approachMovie < 0 && _openMovie == 0xffff)
		_state[0xdce6] |= 1;
	if ((int8)_state[0xdce6] > 0 && _panelMenuOldPage.empty()) {
		_panelMenuOldPage.resize(kScreenWidth * kScreenHeight);
		memcpy(_panelMenuOldPage.begin(), _framebuffer1, _panelMenuOldPage.size());
	}
}

/** D280: side-box shutters around seventeen rolling command-list frames. */
void DuneEngine::finishPanelMenuTransition() {
	if (_panelMenuOldPage.empty())
		return;
	if ((int8)_state[0xdce6] <= 0) {
		_panelMenuOldPage.clear();
		return;
	}
	_state[0xdce6] = 0;
	Common::Array<byte> old;
	old.swap(_panelMenuOldPage); // A nested repaint must not restart this effect.
	Common::Array<byte> target(kScreenWidth * kScreenHeight);
	memcpy(target.begin(), _framebuffer1, target.size());
	const bool cursorShown = CursorMan.showMouse(false);
	const bool previousTransition = _noticeTransitionActive;
	_noticeTransitionActive = true;
	const bool fast = ConfMan.getBool("dune_nodelay");
	uint32 deadline = _system->getMillis();
	uint32 elapsed = 0;
	auto present = [&]() {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	};
	auto wait = [&](uint32 ms, bool speech) {
		elapsed += ms;
		deadline += ms;
		do {
			// D280 polls only A7C2; E387's shutters poll no callbacks.
			// The music driver still fades on its independent timer, but
			// ACE6 cannot load a pending song during this nested effect.
			updateMusicFade();
			// D280 polls A7C2, not D9D2: the voice's mouths continue, but
			// neither the troop actors nor the selection ring advances here.
			Common::Rect mouthWindow;
			if (speech && pollMapSpeechMouth()) {
				const int top = (_troopContactTop ? _troopContactTop : 5) + 4;
				mouthWindow = Common::Rect(10, top, 69, top + 59);
			} else if (speech && pollLocalSpeechMouth()) {
				mouthWindow = _localSpeechWindow;
			}
			if (!mouthWindow.isEmpty()) {
				for (int y = mouthWindow.top; y < mouthWindow.bottom; y++)
					memcpy(target.begin() + y * kScreenWidth + mouthWindow.left,
							_framebuffer1 + y * kScreenWidth + mouthWindow.left, mouthWindow.width());
				present(); // C4F0 updates only the face, retaining this roll frame.
			}
			const uint32 now = _system->getMillis();
			if (fast || now >= deadline || shouldQuit())
				break;
			_system->delayMillis(MIN<uint32>(5, deadline - now));
		} while (true);
	};
	for (int row = 159; row < 200; row++)
		memcpy(_framebuffer1 + row * 320 + 92, old.begin() + row * 320 + 92, 136);
	uint16 left = _onGlobe || (_onWorld && !_inCabin && _panel[1].sprite == 0)
		? 6 : _panel[1].sprite;
	uint16 right = _panel[2].sprite;
	auto shutters = [&](uint phase) {
		if (left % 3 == phase)
			return;
		const int delta = left % 3 < phase ? 1 : -1;
		for (uint pass = 0; pass < 2; pass++) {
			left += delta; right += delta;
			WRITE_LE_UINT16(_state.begin() + 0x1afe, left);
			WRITE_LE_UINT16(_state.begin() + 0x1b0c, right);
			openSpriteSheet("ICONES.HSQ", false);
			drawSprite(left, _panel[1].x, _panel[1].y);
			drawSprite(right, _panel[2].x, _panel[2].y);
			if (!left)
				drawTimeOfDay(); // 1A34 draws only while its left-box sprite is zero.
			present();
			if (!pass)
				wait(50, false); // E387's shutter wait does not poll speech.
		}
	};
	shutters(2);
	for (uint phase = 0; phase < 17 && !shouldQuit(); phase++) {
		Graphics::rollPanelMenuFrame(old.begin(), target.begin(), _framebuffer1, phase);
		present();
		wait(30, true); // D280 polls A7C2 until six ticks elapse, without input.
	}
	shutters(0);
	memcpy(_framebuffer1, target.begin(), target.size());
	present();
	if (fast)
		advanceClockElapsed(elapsed, false, false);
	_noticeTransitionActive = previousTransition;
	CursorMan.showMouse(cursorShown);
}

/** 79EE/7B58, DNVGA effects 2/4: travelling boxes and growing corners. */
void DuneEngine::animateTroopWindow(bool closing) {
	const bool cursorShown = CursorMan.showMouse(false);
	const bool fast = ConfMan.getBool("dune_nodelay");
	const bool previousTransition = _noticeTransitionActive;
	_noticeTransitionActive = true;
	uint32 deadline = _system->getMillis();
	uint frames = 0;
	for (uint phase = 0; phase < 32 && !shouldQuit(); phase++) {
		Graphics::xorTroopWindowFrame(_framebuffer1, _troopWindowAnchor, _troopWindowBounds, phase, closing);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		updateMusicFade(); // The driver wait leaves D9D2 and input alone.
		deadline += kMapOutlineFrameMillis;
		const uint32 now = _system->getMillis();
		if (!fast && now < deadline)
			_system->delayMillis(deadline - now);
		frames++;
	}
	if (fast)
		advanceClockElapsed(frames * kMapOutlineFrameMillis, false, false);
	_noticeTransitionActive = previousTransition;
	CursorMan.showMouse(cursorShown);
}

/** 5F25/7B2B/C0E8, DNVGA effects 6/8: transient XOR window outlines. */
void DuneEngine::animateMapWindow(const Common::Point &anchor, const Common::Rect &bounds, bool closing) {
	if (!closing) {
		WRITE_LE_UINT16(_state.begin() + 0xd816, anchor.x - 10);
		WRITE_LE_UINT16(_state.begin() + 0xd818, anchor.y - 10);
		_state[0x46d8] = 0;
		_mapWindowAnchor = anchor;
		_mapWindowBounds = bounds;
		_mapWindowGeometryValid = true;
	}
	animateMapWindowEffect(anchor, bounds, closing);
}

/** The shared driver effect, without changing a compact card's ownership. */
void DuneEngine::animateMapWindowEffect(const Common::Point &anchor, const Common::Rect &bounds, bool closing) {
	const bool cursorShown = CursorMan.showMouse(false);
	const bool fast = ConfMan.getBool("dune_nodelay");
	const bool previousTransition = _noticeTransitionActive;
	_noticeTransitionActive = true;
	uint32 deadline = _system->getMillis();
	uint frames = 0;
	for (uint phase = 1; phase <= 15 && !shouldQuit(); phase++) {
		Graphics::xorMapWindowFrame(_framebuffer1, anchor, bounds, phase, closing);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		updateMusicFade(); // The driver wait leaves D9D2 and input alone.
		deadline += kMapOutlineFrameMillis;
		const uint32 now = _system->getMillis();
		if (!fast && now < deadline)
			_system->delayMillis(deadline - now);
		Graphics::xorMapWindowFrame(_framebuffer1, anchor, bounds, phase, closing);
		frames++;
	}
	if (fast)
		advanceClockElapsed(frames * kMapOutlineFrameMillis, false, false);
	_noticeTransitionActive = previousTransition;
	CursorMan.showMouse(cursorShown);
}

/** 80DF: paint the short instruction before building and opening MAP2. */
void DuneEngine::openTroopSurvey() {
	_state[0x46eb] &= ~0x40;
	// 80DF replaces the paragraph over the retained chief/map, then presents
	// it before 541F draws the inset. Neither step calls 5D82 or 6715.
	_graphics.setFrameBuffer(_framebuffer1);
	drawTroopStatus(_orderingTroop);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
	Common::Array<byte> instructionPage(kScreenWidth * kScreenHeight);
	memcpy(instructionPage.begin(), _framebuffer1, instructionPage.size());
	WRITE_LE_UINT16(_state.begin() + 0x4720, 0x18f3); // 8129's portrait source.
	_state[0x4722] = 0;
	updateSurveyTerritory();
	// 541F copies the large-map centre before B714 clamps latitude.
	_surveyLongitude = _worldLongitude;
	_surveyLatitude = clampSurveyLatitude(_worldLatitude, surveyMapRect().height());
	resetSurveyHover();
	_state[0x46eb] = 0xc0;
	drawSpiceDensityCard(); // 541F paints MAP2, retaining the surrounding map.
	showSurveyOpeningTransition(instructionPage.begin());
}

/** 80DF/553E: expand MAP2 from the retained chief portrait, then copy it. */
void DuneEngine::showSurveyOpeningTransition(const byte *oldPage) {
	Common::Array<byte> target(kScreenWidth * kScreenHeight);
	memcpy(target.begin(), _framebuffer1, target.size());
	// 553E consumes the source descriptor before the effect. The driver
	// adds eight to its left/top; the compact-card helper's actor convention
	// subtracts two, so normalize this rectangle origin by adding ten.
	const Common::Point anchor(READ_LE_UINT16(_state.begin() + 0x18f3) + 10,
			READ_LE_UINT16(_state.begin() + 0x18f5) + 10);
	WRITE_LE_UINT16(_state.begin() + 0x4720, 0);
	memcpy(_framebuffer1, oldPage, target.size());
	animateMapWindowEffect(anchor, surveyFrameRect(), false);
	// 555E/C4F0 installs the completed inset only after all fifteen outlines
	// have been erased from the old instruction page. MAP2 has no closing
	// outline and must not replace the compact card's cached driver geometry.
	memcpy(_framebuffer1, target.begin(), target.size());
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** 5CCA/79DE/5F79: user dismissal, unlike 7B36's immediate teardown. */
void DuneEngine::dismissMapInformation() {
	// DNVGA retains these in CS, independent of both the moving actor and
	// D816 scratch (which e.g. a later Harkonnen hourly raid overwrites).
	const Common::Point anchor = _mapWindowAnchor;
	Common::Rect bounds = _mapWindowBounds;
	if (!_mapWindowGeometryValid) {
		// Loaded cards retain DS's frame, but not the driver's XOR geometry.
		if (mapTroopCard() >= 0)
			bounds = mapTroopCardBounds();
		else if (_pressedPlace >= 0) {
			const byte *box = _state.begin() + 0x1668;
			bounds = Common::Rect(READ_LE_INT16(box), READ_LE_INT16(box + 2),
				READ_LE_INT16(box + 4), READ_LE_INT16(box + 6));
		}
	}
	const bool animate = _mapWindowGeometryValid && !_state[0x46d8];
	const bool placeMenu = _menuKind == kMenuPlace;
	const bool placeCard = _pressedPlace >= 0;
	if (placeMenu)
		armPanelMenuTransition(); // D2E2 arms before invoking 5F91's close callback.
	closeMapTroopCard();
	closeMapPlaceCard();
	_mapWindowGeometryValid = false;
	if (placeCard)
		_speech.clear();
	restoreMapInformationBackground(bounds, true); // 5F9F presents the restored C412 pixels before the outline.
	if (animate)
		animateMapWindow(anchor, bounds, true);
	if (placeMenu) {
		// D2FD restores the parent only after 5F91 has removed the window.
		// The returning list rolls over the clean map, not over the old card.
		showMapMenu(false);
		drawPanel();
		drawMenu();
		finishPanelMenuTransition();
	}
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** 600E/5EE4: expand a place's window only on opening, not every repaint. */
void DuneEngine::openMapPlaceCard(uint place) {
	if (place >= _map.places().size())
		return;
	const Common::Point anchor = placeOnScreen(_map.places()[place]);
	const Common::Rect bounds = mapPlaceCardBounds(place);
	const byte *box = _state.begin() + 0x1668;
	const Common::Rect previous = _pressedPlace >= 0 ?
			Common::Rect(READ_LE_INT16(box), READ_LE_INT16(box + 2),
					READ_LE_INT16(box + 4), READ_LE_INT16(box + 6)) : Common::Rect();
	closeMapPlaceCard();
	// 600E paints over the retained map. Its caller has released older
	// windows; direct replacements also restore their registered rectangle.
	restoreMapInformationBackground(previous, true);
	_pressedPlace = place;
	if (_map.places()[place].isKnown() && Common::Rect(4, 4, 316, 148).contains(anchor)) {
		prepareMapPlaceCard(place, bounds); // 5F5F precedes the opening effect.
		animateMapWindow(anchor, bounds);
	}
	drawMapTooltip(place);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** 5F12/5F25: register the frame before 600E retains its place afterward. */
void DuneEngine::prepareMapPlaceCard(uint place, const Common::Rect &bounds) {
	byte *box = _state.begin() + 0x1668;
	WRITE_LE_UINT16(box, bounds.left); WRITE_LE_UINT16(box + 2, bounds.top);
	WRITE_LE_UINT16(box + 4, bounds.right); WRITE_LE_UINT16(box + 6, bounds.bottom);
	_state[0x46f7] = mapPlaceCardKind(place) + 1;
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0x1668);
}

/** 5F79/5F91: release an active place window, leaving unrelated idle slots. */
void DuneEngine::closeMapPlaceCard() {
	if (_pressedPlace < 0 && !READ_LE_UINT16(_state.begin() + 0x46f8))
		return;
	_pressedPlace = -1;
	_state[0x46f7] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x46f8, 0);
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0);
	_mapWindowGeometryValid = false;
}

int DuneEngine::mapTroopCard() const {
	if (_state.size() <= 0x46fb)
		return -1;
	const uint record = READ_LE_UINT16(_state.begin() + 0x46fa);
	return record >= 0x8aa && (record - 0x8aa) % 27 == 0 &&
		(record - 0x8aa) / 27 < _map.troops().size() ? (record - 0x8aa) / 27 : -1;
}

Common::Rect DuneEngine::mapTroopCardBounds() const {
	const byte *box = _state.begin() + 0x18df;
	return Common::Rect(READ_LE_INT16(box), READ_LE_INT16(box + 2),
		READ_LE_INT16(box + 4), READ_LE_INT16(box + 6));
}

bool DuneEngine::mapInformationContains(const Common::Point &position) const {
	Common::Rect box;
	if (_menuKind == kMenuMap && mapTroopCard() >= 0)
		box = mapTroopCardBounds();
	else if ((_menuKind == kMenuMap || _menuKind == kMenuPlace) && _pressedPlace >= 0) {
		// 5C76/D6FE tests the registered, visible 1668 frame. Work may
		// change the next card's height before 1C18 repaints this one.
		const byte *frame = _state.begin() + 0x1668;
		box = Common::Rect(READ_LE_INT16(frame), READ_LE_INT16(frame + 2),
			READ_LE_INT16(frame + 4), READ_LE_INT16(frame + 6));
	} else
		return false;
	// D6FE compares unsigned coordinates. A card whose left edge wraps
	// past zero stays on screen, but its visible portion passes input through.
	const uint16 x = position.x, y = position.y;
	return x > uint16(box.left) && x < uint16(box.right) &&
			y > uint16(box.top) && y < uint16(box.bottom);
}

void DuneEngine::closeMapTroopCard() {
	if (mapTroopCard() >= 0)
		_mapWindowGeometryValid = false;
	if (_state.size() > 0x46fb) {
		// 79DE does not clear another window when no compact report is held.
		if (READ_LE_UINT16(_state.begin() + 0x46fa))
			WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0);
		WRITE_LE_UINT16(_state.begin() + 0x46fa, 0);
	}
	_mapTroopCardPixels.clear();
}

/** DS:1A9E/D8B1/D911: 5CE4 is right-click, not the idle/hover callback. */
void DuneEngine::rightClickMap(const Common::Point &position) {
	if (!_onWorld || _onGlobe || _onMixer || _inCabin ||
			_menuKind != kMenuMap || _pressedPlace >= 0)
		return;
	dismissMapTitle(); // 5BEB restores the legend before 6946 tests actor bounds.
	const int current = mapTroopCard();
	// D8B1/D911 bypasses the viewport arrows on a right-click. 6946 tests
	// the registered actor rectangle even where it extends past the viewport.
	Common::Rect actor;
	const int troop = troopAt(position, &actor);
	if ((current >= 0 && mapInformationContains(position)) || (current >= 0 && troop == current)) {
		dismissMapInformation();
		return;
	}
	if (troop < 0)
		return;
	// 7B36 tears down the previous report before 78BC checks the new troop's range.
	_state[0x46d8] = 1;
	_state[0xdce6] = 0x80;
	const Common::Rect previousBounds = mapTroopCardBounds();
	closeMapTroopCard();
	if (current >= 0)
		restoreMapInformationBackground(previousBounds, true);
	_state[0xdce6] = 0;
	_state[0x46f4] = 0;
	const uint record = 0x8aa + troop * 27;
	// 78BC uses the troop's CURRENT coordinates and strict distance < reach,
	// unlike 872C's gate for a full conversation. It never sets the hold id.
	if (distanceFromPaul(READ_LE_UINT16(_state.begin() + record + 6),
			(int16)READ_LE_UINT16(_state.begin() + record + 8)) <
			READ_LE_UINT16(_state.begin() + 0x1176)) {
		int left = actor.left + 15;
		if (left >= 210)
			left -= 130;
		// 5F25 clears BH before calculating both the rectangle and the
		// animation origin. An actor clipped at the north edge wraps in Y.
		const Common::Point anchor(actor.left, (byte)actor.top);
		const int top = CLIP(anchor.y - 50, 4, 48);
		byte *box = _state.begin() + 0x18df;
		WRITE_LE_UINT16(box, left); WRITE_LE_UINT16(box + 2, top);
		WRITE_LE_UINT16(box + 4, left + 106); WRITE_LE_UINT16(box + 6, top + 100);
		// 78BC opens on the retained display; only dismissed windows were
		// restored above. A fresh map would expose unrequested terrain changes.
		WRITE_LE_UINT16(_state.begin() + 0x46fa, record); // 78D0 before 5F25.
		WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0x18df);
		animateMapWindow(anchor, mapTroopCardBounds());
		_state[0x1955] = _state[record];
		refreshMapTroopCard(true);
		drawMapTroopCard();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/** 78E9/1C18 refresh the report on opening and at the end of an hour. */
void DuneEngine::refreshMapTroopCard(bool updateContext) {
	const int troop = mapTroopCard();
	if (troop < 0)
		return;
	const Common::Rect box = mapTroopCardBounds();
	if (box.width() != 106 || box.height() != 100 || box.right <= 0 ||
			box.left >= kScreenWidth || box.top < 4 || box.bottom > 148) {
		closeMapTroopCard();
		return;
	}
	// A loaded save already contains the original query's side effects.
	// Reconstruct its transient image without applying query side effects again.
	Common::Array<byte> saved;
	if (!updateContext)
		saved = _state;
	prepareTroopContactContext(troop);
	WRITE_LE_UINT16(_state.begin() + 0x11f3, 36 + _state[0x2f]);
	// The driver's fill skips a rectangle starting at a negative X.
	// Seed the scratch page with the displayed map for that sparse paint.
	if (box.left < 0)
		memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
	SWAP(_framebuffer1, _framebuffer2);
	_graphics.setFrameBuffer(_framebuffer1);
	renderMapTroopCard();
	_mapTroopCardPixels.resize(106 * 100);
	const int left = MAX<int>(box.left, 0), right = MIN<int>(box.right, kScreenWidth);
	for (int y = 0; y < 100; y++)
		memcpy(_mapTroopCardPixels.begin() + y * 106 + left - box.left,
			_framebuffer1 + (box.top + y) * kScreenWidth + left, right - left);
	SWAP(_framebuffer1, _framebuffer2);
	_graphics.setFrameBuffer(_framebuffer1);
	if (box.left < -4) {
		// Text and equipment left of X=0 land in the preceding scanline.
		for (int y = box.top; y < box.bottom; y++) {
			const int start = y * kScreenWidth + box.left;
			memcpy(_framebuffer1 + start, _framebuffer2 + start, -box.left);
		}
	}
	if (!updateContext)
		memcpy(_state.begin(), saved.begin(), _state.size());
}

/** 5F91/79DE: C6AD restores only the dismissed information rectangle. */
void DuneEngine::restoreMapInformationBackground(Common::Rect bounds, bool present) {
	bounds.clip(Common::Rect(4, 4, 316, 148));
	if (bounds.isEmpty())
		return;
	Common::Array<byte> visible(kScreenWidth * kScreenHeight);
	memcpy(visible.begin(), _framebuffer1, visible.size());
	const bool previousRedraw = _mapAnimationRedraw;
	const bool previousRepaint = _mapActorNeedsRepaint;
	_mapAnimationRedraw = true;
	repaintMapActors(false, &bounds);
	_mapAnimationRedraw = previousRedraw;
	_mapActorNeedsRepaint = previousRepaint;
	for (int y = bounds.top; y < bounds.bottom; y++)
		memcpy(visible.begin() + y * kScreenWidth + bounds.left,
				_framebuffer1 + y * kScreenWidth + bounds.left, bounds.width());
	memcpy(_framebuffer1, visible.begin(), visible.size());
	if (present) {
		// 1C18's 79DE/5F91 copies this released rectangle through C6AD
		// before any closing/opening outline or replacement card paint.
		_system->copyRectToScreen(_framebuffer1 + bounds.top * kScreenWidth + bounds.left,
				kScreenWidth, bounds.left, bounds.top, bounds.width(), bounds.height());
		_system->updateScreen();
	}
}

/** 1C18: refresh the active map/results view after completed work. */
void DuneEngine::refreshMapInformation() {
	// Native globe ownership represents 1C39's signed FB view gate. Its
	// closed doors correspond to BDBB's DD0F zero gate, not a new opening.
	if (_onGlobe && !_state[0x46eb]) {
		refreshResultsGauges();
		return;
	}
	if (!_onWorld || _onGlobe || _onMixer || _inCabin || !(_state[0x46eb] & 0x80))
		return;
	Common::Rect repaint(4, 4, 316, 148);
	bool refreshed = false;
	if (_menuKind == kMenuMap && mapTroopCard() >= 0) {
		bool registered = false;
		for (uint i = 0; i < _mapUnitOrder.size(); i++)
			registered |= _mapUnitOrder[i] == (uint)mapTroopCard();
		if (registered) {
			refreshMapTroopCard(true);
			drawMapTroopCard();
			if (mapTroopCard() >= 0) {
				// 78E9 paints directly into the visible page. Its card can
				// reach the west border or wrap text around to the east.
				const Common::Rect box = mapTroopCardBounds();
				repaint.left = CLIP<int>(box.left, 0, repaint.left);
				repaint.right = box.left < -4 ? kScreenWidth : CLIP<int>(box.right, repaint.right, kScreenWidth);
			}
		} else {
			// 78E9/6917 goes straight to 79DE, without a 31F6 query.
			const Common::Rect bounds = mapTroopCardBounds();
			const Common::Point anchor = _mapWindowAnchor;
			const bool animate = _mapWindowGeometryValid && !_state[0x46d8];
			closeMapTroopCard();
			restoreMapInformationBackground(bounds, true);
			if (animate)
				animateMapWindow(anchor, bounds, true);
		}
		refreshed = true;
	}
	if ((_menuKind == kMenuMap || _menuKind == kMenuPlace) && _pressedPlace >= 0) {
		const int place = _pressedPlace;
		const Common::Rect bounds = mapPlaceCardBounds(place);
		if (_mapWindowGeometryValid && _mapWindowBounds.height() != bounds.height()) {
			// 5EE4/5F91 fast-closes the old window before opening the new
			// kind. Restore its exposed terrain/actors, without registering
			// a new pool, advancing frames or repainting other foregrounds.
			closeMapPlaceCard();
			restoreMapInformationBackground(_mapWindowBounds, true);
			_pressedPlace = place;
			_mapWindowGeometryValid = true; // Retain the old height for 5EE4.
		}
		drawMapTooltip(place);
		refreshed = true;
	}
	if (refreshed) {
		_system->copyRectToScreen(_framebuffer1 + repaint.top * kScreenWidth + repaint.left,
				kScreenWidth, repaint.left, repaint.top, repaint.width(), repaint.height());
		_system->updateScreen();
	}
}

void DuneEngine::drawMapTroopCard() {
	const int troop = mapTroopCard();
	if (_menuKind != kMenuMap || troop < 0)
		return;
	bool registered = false;
	for (uint i = 0; i < _mapUnitOrder.size(); i++)
		registered |= _mapUnitOrder[i] == (uint)troop;
	if (!registered) {
		closeMapTroopCard(); // 6917/78EE: the actor has left this map.
		return;
	}
	if (_mapTroopCardPixels.empty())
		refreshMapTroopCard(false); // Loaded saves already include query side effects.
	if (_mapTroopCardPixels.empty())
		return;
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0x18df); // Rebuild old held-save registration.
	const Common::Rect box = mapTroopCardBounds();
	const int left = MAX<int>(box.left, 0), right = MIN<int>(box.right, kScreenWidth);
	for (int y = 0; y < 100; y++)
		memcpy(_framebuffer1 + (box.top + y) * kScreenWidth + left,
			_mapTroopCardPixels.begin() + y * 106 + left - box.left, right - left);
}

/** 78FC..79DB paints a prepared context, without conversation or voice. */
void DuneEngine::renderMapTroopCard() {
	const int troop = mapTroopCard();
	if (troop < 0)
		return;
	const Common::Rect box = mapTroopCardBounds();
	const byte border = _state[0x18e7];
	const byte fill = _state[0x18e8];
	const byte textBackground = box.left < 0 ? fill : 0;
	if (box.left < 0) {
		// C551 still draws the clipped outline after the failed fill.
		_graphics.fillRect(0, box.top, box.right, box.top + 1, border);
		_graphics.fillRect(0, box.bottom - 1, box.right, box.bottom, border);
		_graphics.fillRect(box.right - 1, box.top + 1, box.right, box.bottom - 1, border);
	} else {
		_graphics.fillRect(box.left, box.top, box.right, box.bottom, border);
		_graphics.fillRect(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1, fill);
	}
	const uint record = 0x8aa + 27 * troop;
	if (box.left >= -4) {
		renderMapTroopCardContents(box, record, textBackground);
		return;
	}

	// Draw content in a shifted rectangle so the clipped blitters can
	// reproduce the driver's linear-address writes across the left edge.
	Common::Array<byte> shifted(kScreenWidth * kScreenHeight);
	byte *const page = _framebuffer1;
	for (int y = box.top; y < box.bottom; y++)
		memcpy(shifted.begin() + y * kScreenWidth, page + y * kScreenWidth + box.left, box.width());
	_framebuffer1 = shifted.begin();
	_graphics.setFrameBuffer(_framebuffer1);
	renderMapTroopCardContents(Common::Rect(0, box.top, box.width(), box.bottom), record, textBackground);
	_framebuffer1 = page;
	_graphics.setFrameBuffer(page);
	for (int y = box.top; y < box.bottom; y++)
		memcpy(page + y * kScreenWidth + box.left, shifted.begin() + y * kScreenWidth, box.width());
	for (uint item = 0; item < 7; item++) {
		byte *column = _state.begin() + 0x4c60 + 4 * item;
		const uint16 end = READ_LE_UINT16(column + 2);
		if (end) {
			WRITE_LE_UINT16(column, READ_LE_UINT16(column) + box.left);
			WRITE_LE_UINT16(column + 2, end + box.left);
		}
	}
}

void DuneEngine::renderMapTroopCardContents(const Common::Rect &box, uint record, byte textBackground) {
	_font.setFace(Font::kFaceAlternate);
	auto text = [this, record, textBackground](uint word, int x, int y, byte ink) {
		const Common::String line = fillInLine(command(word - 1), record);
		const int left = x;
		for (uint i = 0; i < line.size(); i++) {
			if (line[i] == '\r') {
				x = left;
				y += 7;
			} else
				x = _font.drawChar(_framebuffer1, x, y, line[i], ink, textBackground);
		}
	};
	const byte status = _state[0x30], task = _state[0x2f];
	text(status & 0x40 ? 59 : 58, box.left + 12, box.top + 4, 0x9a);
	const uint place = READ_LE_UINT16(_state.begin() + 0x2c);
	if (place >= 0x100 && (place - 0x100) / 28 < _map.places().size())
		_font.drawString(_framebuffer1, box.left + 4, box.top + 13,
			nameOfPlace(_map.places()[(place - 0x100) / 28]), 0x96, textBackground);
	int y = box.top + 23;
	if (status & 0x20) {
		text(status == 0x22 ? 66 : 65, box.left + 4, y, 0x9a);
		y += 17;
	} else {
		text(60, box.left + 4, y, 0x9a);
		y += 15;
		if (status != 2) {
			text(READ_LE_UINT16(_state.begin() + 0x11f7 + 2 * (task >> 2)), box.left + 4, y, 0x9a);
			y += 10;
			if (!(status & 0x40)) {
				const uint flags = READ_LE_UINT16(_state.begin() + 0x32);
				const uint secondary = READ_LE_UINT16(_state.begin() + 0x34);
				const uint word = flags & 0x200 ? 63 : !(flags & 0x100) || (secondary & 0x30) ? 64
					: status == 0 ? 61 : task == 1 ? 67 : status == 6 ? 62 : 0;
				if (word) {
					text(word, box.left + 4, y, 0x9a);
					y += 17;
				}
			}
		}
	}
	y += 4;
	text(110, box.left + 4, y, 0x96);
	for (uint item = 0; item < 7; item++)
		_state[0x4705 + item] = (_state[record + 25] & (0x80 >> item)) ? 1 : 0;
	drawEquipmentRow(_state.begin() + 0x4705, box.left + 4, y + 8, box.bottom,
		0x96, 0x4c60, Font::kFaceAlternate, textBackground); // 7E3D fills the shared equipment column table.
}

} // namespace Dune
