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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/mystery.h"
#include "eem/site.h"

namespace EEM {

void SiteScreen::enter(uint siteNum) {
	if (!_mystery || !_mystery->isLoaded()) {
		warning("SiteScreen::enter: no mystery loaded");
		return;
	}
	if (siteNum >= _mystery->numSites()) {
		warning("SiteScreen::enter: site %u out of range (max %u)",
				siteNum, _mystery->numSites());
		return;
	}

	_mystery->_siteNumber = siteNum;
	if (siteNum < Mystery::kVisitedSiteCap)
		_mystery->_visitedSite[siteNum] = 1;
	debugC(1, kDebugSite, "Entering site %u (%u hotspots)",
		   siteNum, _mystery->hotspotCount(siteNum));

	// Palette: original `_BuildBackground` calls `GetPalette(sitenum + 1)`
	// where sitenum is the global SITES.DBD index (= the per-mystery
	// `sitepic` field), not the per-mystery site index.
	const byte *sd = _mystery->siteData(siteNum);
	const uint16 sitepic = sd ? READ_LE_UINT16(sd) : 0;
	_vm->setSitePaletteForSite(sitepic);

	renderBackground(siteNum);
	renderHotspots(siteNum);
	g_system->updateScreen();
}

void SiteScreen::run() {
	if (!_mystery || !_mystery->isLoaded())
		return;

	uint cur = 0;
	enter(cur);

	while (!_vm->shouldQuit()) {
		Common::Event event;
		bool exitRequested = false;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				return;

			case Common::EVENT_LBUTTONDOWN: {
				const int idx = hotspotAtPoint(cur, event.mouse.x, event.mouse.y);
				if (idx >= 0) {
					onHotspotClicked(cur, (uint)idx);
					// Restore the site BG after the clue overlay.
					enter(cur);
				}
				break;
			}

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					if (_vm->areYouSure())
						return;
					enter(cur);
					break;
				case Common::KEYCODE_m:
					_vm->doBigMap();
					// Either way the map covered the site — re-render.
					if (_mystery->_siteNumber < _mystery->numSites())
						cur = _mystery->_siteNumber;
					enter(cur);
					break;
				case Common::KEYCODE_n:
					_vm->doNotebook();
					enter(cur);
					break;
				case Common::KEYCODE_g:
					_vm->doGallery();
					enter(cur);
					break;
				case Common::KEYCODE_a:
					_vm->doAccuse();
					exitRequested = true;
					break;
				case Common::KEYCODE_h:
					_vm->doHelp();
					enter(cur);
					break;
				case Common::KEYCODE_v:
					_showHotspots = !_showHotspots;
					enter(cur);
					break;
				case Common::KEYCODE_r:
					// Restart the mystery from scratch (mirrors `_ReloadMystery`).
					if (_mystery->load(_mystery->number())) {
						cur = 0;
						enter(cur);
					}
					break;
				case Common::KEYCODE_QUESTION:
				case Common::KEYCODE_F1: {
					if (_vm->getFont().isLoaded()) {
						Graphics::ManagedSurface help(320, 200,
							Graphics::PixelFormat::createFormatCLUT8());
						help.clear();
						const EEMFont &fnt = _vm->getFont();
						int y = 8;
						const char *lines[] = {
							"EAGLE EYE MYSTERIES — keys",
							"",
							"  click   search a hotspot",
							"  V       toggle hotspot outlines",
							"  M       map (travel between sites)",
							"  N       notebook (mark evidence with 1..9)",
							"  G       gallery (suspect portraits)",
							"  H       hint from the case host",
							"  A       accuse a suspect",
							"  R       restart current mystery",
							"  Tab     next site (cycle)",
							"  F5      save / load (ScummVM dialog)",
							"  ? / F1  this help",
							"  Esc     quit (with confirm)",
							"",
							"Notebook: select evidence with 1..9.",
							"Selected-points > 99 wins the case."
						};
						for (uint i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
							fnt.drawString(&help, 8, y, lines[i], 0xF);
							y += fnt.height() + 1;
						}
						g_system->copyRectToScreen(help.getPixels(),
							help.pitch, 0, 0, 320, 200);
						g_system->updateScreen();
						_vm->waitForInput(60000);
						enter(cur);
					}
					break;
				}
				case Common::KEYCODE_TAB:
					_mystery->_lastSite = cur;
					cur = (cur + 1) % _mystery->numSites();
					enter(cur);
					break;
				default:
					break;
				}
				break;

			default:
				break;
			}
		}
		if (exitRequested)
			return;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void SiteScreen::renderBackground(uint siteNum) {
	// Mirrors `_BuildBackground` @ 172b:13e2 (simplified):
	//   1. Load PIC 0x3d (the site frame) from PICS.DBD.
	//   2. Load entry @p siteNum from SITES.DBD (the site scene).
	//   3. Composite scene into the frame at the position carried in the
	//      SiteData fields.
	//   4. Set palette to (siteNum + 1) — per-site palettes start at 1.
	// We render frame + scene at (0,0); the original positions the scene
	// at (x,y) read from the SiteData but we don't have the offsets fully
	// decoded yet so a top-left placement will do.

	// Frame.
	Picture frame;
	if (_vm->getPics().getPicture(0x3d, frame)) {
		g_system->copyRectToScreen(frame.surface.getPixels(),
								   frame.surface.pitch,
								   0, 0, frame.surface.w, frame.surface.h);
	}

	// Scene from SITES.DBD: indexed by `sitepic` from SiteData (global
	// SITES.DBD entry, NOT the per-mystery site index). Falls back to
	// `_GetPicture(sitepic)` if SITES is unavailable.
	const byte *site = _mystery->siteData(siteNum);
	const uint16 sitepic = site ? READ_LE_UINT16(site) : 0;
	Picture scene;
	bool haveScene = false;
	bool fromPics = false;
	if (sitepic > 0 && _vm->getSites().size() > sitepic - 1)
		haveScene = _vm->getSites().loadEntry(sitepic - 1, scene);
	if (!haveScene && sitepic > 0) {
		haveScene = _vm->getPics().getPicture(sitepic, scene);
		fromPics = haveScene;
	}
	if (haveScene) {
		const int w = MIN<int>(scene.surface.w, 320);
		const int h = MIN<int>(scene.surface.h, 200);
		// Full-screen pictures (sitepic fallback) go at (0, 0); smaller
		// SITES.DBD scenes are centred horizontally with the top below
		// the HUD bar so progress info stays visible.
		const int x = fromPics ? 0 : (320 - w) / 2;
		const int y = fromPics ? 0 : (h < 180 ? 4 : 0);
		g_system->copyRectToScreen(scene.surface.getPixels(),
								   scene.surface.pitch, x, y, w, h);
	}
}

void SiteScreen::renderHotspots(uint siteNum) {
	// HUD overlay: site number, found clues, selected points. Drawn at
	// the BOTTOM of the screen so the scene's top row stays visible —
	// 320x200 mode 13h has a small bottom strip that the original engine
	// uses for tool buttons; we repurpose it for the HUD.
	if (_vm->getFont().isLoaded()) {
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			uint cluesFound = 0;
			for (uint i = 0; i < Mystery::kCluesFoundCap; i++)
				if (_mystery->_cluesFound[i])
					cluesFound++;
			Common::String hud = Common::String::format(
				"Site %u  Clues %u  Pts %d   M N G H A R V Tab ?",
				siteNum, cluesFound, _mystery->selectedPoints());
			const int hudY = 192;
			screen->fillRect(Common::Rect(0, hudY, 320, 200), 0);
			Graphics::ManagedSurface mgr(320, 9,
				Graphics::PixelFormat::createFormatCLUT8());
			mgr.clear();
			_vm->getFont().drawString(&mgr, 4, 0, hud, 0x0F);
			for (int row = 0; row < 8; row++) {
				memcpy((byte *)screen->getBasePtr(0, hudY + row),
					   (const byte *)mgr.getBasePtr(0, row), 320);
			}
			g_system->unlockScreen();
		}
	}

	// Hotspot outlines (`_DrawSearchButtons`): toggle via V.
	if (!_showHotspots)
		return;

	const byte *spots = _mystery->hotspots(siteNum);
	const uint16 count = _mystery->hotspotCount(siteNum);
	if (!spots)
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;

	for (uint i = 0; i < count; i++) {
		const byte *r = spots + i * 14;
		const int16 x1 = (int16)READ_LE_UINT16(r + 0);
		const int16 y1 = (int16)READ_LE_UINT16(r + 2);
		const int16 x2 = (int16)READ_LE_UINT16(r + 4);
		const int16 y2 = (int16)READ_LE_UINT16(r + 6);
		const Common::Rect rect(MAX<int>(0, x1), MAX<int>(0, y1),
								MIN<int>(screen->w, x2),
								MIN<int>(screen->h, y2));
		// Hotspots flagged as "seen" get a different colour so the
		// player knows they've already searched them.
		const byte color =
			(i < Mystery::kHotSpotsCap && _mystery->_hotSpotsSeen[i]) ? 0x07 : 0x0F;
		screen->frameRect(rect, color);
	}

	g_system->unlockScreen();
}

int SiteScreen::hotspotAtPoint(uint siteNum, int x, int y) const {
	const byte *spots = _mystery->hotspots(siteNum);
	const uint16 count = _mystery->hotspotCount(siteNum);
	if (!spots)
		return -1;

	for (uint i = 0; i < count; i++) {
		const byte *r = spots + i * 14;
		const int16 x1 = (int16)READ_LE_UINT16(r + 0);
		const int16 y1 = (int16)READ_LE_UINT16(r + 2);
		const int16 x2 = (int16)READ_LE_UINT16(r + 4);
		const int16 y2 = (int16)READ_LE_UINT16(r + 6);
		if (x >= x1 && x < x2 && y >= y1 && y < y2)
			return (int)i;
	}
	return -1;
}

void SiteScreen::onHotspotClicked(uint siteNum, uint hotIdx) {
	debugC(1, kDebugSite, "Site %u: hotspot %u clicked", siteNum, hotIdx);

	// Mark the hotspot itself as seen.
	if (hotIdx < Mystery::kHotSpotsCap)
		_mystery->_hotSpotsSeen[hotIdx] = 1;
	_mystery->_searchLocationNumber = (uint16)hotIdx;

	// Bytes 8..9 of each 14-byte hotspot rect = byte offset within the
	// mystery blob pointing at a ClueBlock. Verified against M0.BIN:
	// site 0 hotspot 0 -> 0x0502 -> "Hi! I think somebody's playing a
	// trick on us...". `displayClue` runs the entry's side effects
	// (`_AddNotebook` for ClueEntry +0x30..+0x39, gallery +0x26..+0x2f,
	// onsite +0x1c..+0x25) so we don't need to touch `_cluesFound` here.
	const byte *spots = _mystery->hotspots(siteNum);
	if (spots) {
		const uint16 clueOff = READ_LE_UINT16(spots + hotIdx * 14 + 8);
		debugC(2, kDebugSite, "  hotspot %u -> clue offset 0x%04x",
			   hotIdx, clueOff);
		const byte *clueBlock = _mystery->blobAt(clueOff);
		if (clueBlock)
			_vm->displayClue(clueBlock);
	}
	// Caller (`SiteScreen::run`) re-renders the site after this returns.
}

} // End of namespace EEM
