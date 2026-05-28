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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/util.h"
#include "audio/decoders/apc.h"
#include "audio/mixer.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"

#include "cryomni3d/atlantis/engine.h"

namespace CryOmni3D {
namespace Atlantis {

// Vertical span allocated to a FONTMAX text item.  FONTMAX glyphs render
// with their bottom near anchorY; we treat the hit-box as [anchorY-32, anchorY+4].
static const int kFontMaxHitTop    = 32;
static const int kFontMaxHitBottom = 4;

// Space advance from exe width table at 0x4965eb[35] = 22.
static const int kFontMaxSpaceAdvance = 22;
// FONTMIN is the smaller font; its space advance scales down proportionally.
static const int kFontMinSpaceAdvance = 18;

int CryOmni3DEngine_Atlantis::fontMaxCharAdvance(unsigned char c) const {
	if (c == ' ')
		return kFontMaxSpaceAdvance;
	int idx = (int)c - 0x20;
	if (idx < 0 || idx >= (int)_fontMaxSprites.size())
		return 0;
	return _fontMaxSprites[idx].w;
}

int CryOmni3DEngine_Atlantis::fontMaxStringWidth(const Common::String &text) const {
	int w = 0;
	for (uint i = 0; i < text.size(); ++i)
		w += fontMaxCharAdvance((unsigned char)text[i]);
	return w;
}

// Glyph-sprite text blitter shared by the FONTMAX and FONTMIN font families.
// `layoutGlyphs` drives the pen advance so a hover swap to a differently-
// metricised glyph set never reflows the line; `drawGlyphs` supplies the
// pixels, each anchored by its own xoff/yoff.  anchorY is the text baseline.
void CryOmni3DEngine_Atlantis::drawSprFontText(Graphics::ManagedSurface &dst,
        const Common::String &text, int anchorX, int anchorY,
        const Common::Array<SubjSprite> &drawGlyphs,
        const Common::Array<SubjSprite> &layoutGlyphs, int spaceAdvance) const {
	int cx = anchorX;
	for (uint i = 0; i < text.size(); ++i) {
		unsigned char c = (unsigned char)text[i];
		if (c == ' ') {
			cx += spaceAdvance;
			continue;
		}
		int idx = (int)c - 0x20;
		if (idx < 0 || idx >= (int)layoutGlyphs.size())
			continue;
		const SubjSprite &lay = layoutGlyphs[idx];   // advance + anchor
		if (idx < (int)drawGlyphs.size() && !drawGlyphs[idx].pixels.empty()) {
			const SubjSprite &spr = drawGlyphs[idx]; // pixels
			// Anchor each glyph by its own offsets so a hover-font glyph whose
			// metrics differ still lands where that font intends.
			int dx = cx - spr.xoff;
			int dy = anchorY - spr.yoff;
			for (int sy = 0; sy < (int)spr.h; ++sy) {
				int ty = dy + sy;
				if (ty < 0 || ty >= dst.h)
					continue;
				for (int sx = 0; sx < (int)spr.w; ++sx) {
					uint sidx = sy * spr.w + sx;
					uint16 pix = spr.pixels[sidx];
					if (pix == kSprTransp)
						continue;
					int tx = dx + sx;
					if (tx < 0 || tx >= dst.w)
						continue;
					uint16 *dp = (uint16 *)dst.getBasePtr(tx, ty);
					if (spr.blend[sidx] != kSprNoBlend)
						pix = blendSprPixel565(pix, *dp, spr.blend[sidx]);
					*dp = pix;
				}
			}
		}
		cx += lay.w;
	}
}

// FONTMAX.SPR menu font.  `hover` swaps in the FONTMAX2.SPR glyphs for colour;
// layout always uses FONTMAX so a hovered entry never shifts.
void CryOmni3DEngine_Atlantis::drawFontMaxText(Graphics::ManagedSurface &dst,
        const Common::String &text, int anchorX, int anchorY, bool hover) const {
	drawSprFontText(dst, text, anchorX, anchorY,
	    (hover && !_fontMaxHoverSprites.empty()) ? _fontMaxHoverSprites
	                                            : _fontMaxSprites,
	    _fontMaxSprites, kFontMaxSpaceAdvance);
}

// FONTMIN.SPR — the smaller, red-tinted font for player names; FONTMIN2.SPR is
// its hover variant.  Layout always uses FONTMIN so hovering only recolours.
void CryOmni3DEngine_Atlantis::drawFontMinText(Graphics::ManagedSurface &dst,
        const Common::String &text, int anchorX, int anchorY, bool hover) const {
	drawSprFontText(dst, text, anchorX, anchorY,
	    (hover && !_fontMin2Sprites.empty()) ? _fontMin2Sprites : _fontMinSprites,
	    _fontMinSprites, kFontMinSpaceAdvance);
}

// ---------------------------------------------------------------------------
// CREDBLAN.SPR / CREDBLEU.SPR — credits roll glyph renderer.  Char advance
// widths come from the atlantis.exe table at VA 0x004966fc (loaded by
// loadAtlantisExeTables() into _creditCharWidths).  Glyph drawing reuses
// the same SubjSprite atlas the existing menu code uses, so per-glyph
// xoff/yoff hotspots are honoured automatically.
// ---------------------------------------------------------------------------

int CryOmni3DEngine_Atlantis::creditCharAdvance(unsigned char c) const {
	return (int)_creditCharWidths[c];
}

int CryOmni3DEngine_Atlantis::creditStringWidth(const Common::String &text) const {
	int w = 0;
	for (uint i = 0; i < text.size(); i++)
		w += creditCharAdvance((unsigned char)text[i]);
	return w;
}

void CryOmni3DEngine_Atlantis::drawCreditText(Graphics::ManagedSurface &dst,
        const Common::String &text, int anchorX, int anchorY, bool blue) const {
	const Common::Array<SubjSprite> &glyphs = blue ? _creditBleuSprites
	                                                : _creditBlanSprites;
	if (glyphs.empty())
		return;
	int cx = anchorX;
	for (uint i = 0; i < text.size(); i++) {
		unsigned char c = (unsigned char)text[i];
		int adv = creditCharAdvance(c);
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
			cx += adv;
			continue;
		}
		// CREDBLAN.SPR / CREDBLEU.SPR contain 136 glyphs covering chars
		// 0x21..0xa8 — there is NO sprite for the space character; the
		// engine renders space by advance-only.  Cross-checking each
		// stored sprite.w against the char-indexed width table at
		// atlantis.exe 0x4966fc confirms the mapping:
		//   sprite_index = char - 0x21      (NOT char - 0x20)
		// e.g. exe['A']=9 ↔ sprite[32].w=9, exe['I']=4 ↔ sprite[40].w=4.
		// FONTMAX.SPR is different — it has 137 slots and DOES include
		// a placeholder for space at slot 0, so its renderer uses
		// `char - 0x20`.  Keep the two paths separate.
		int idx = (int)c - 0x21;
		if (idx < 0 || idx >= (int)glyphs.size()) {
			cx += adv;
			continue;
		}
		const SubjSprite &spr = glyphs[idx];
		if (spr.pixels.empty()) {
			cx += adv;
			continue;
		}
		int dx = cx - spr.xoff;
		int dy = anchorY - spr.yoff;
		for (int sy = 0; sy < (int)spr.h; ++sy) {
			int ty = dy + sy;
			if (ty < 0 || ty >= dst.h)
				continue;
			for (int sx = 0; sx < (int)spr.w; ++sx) {
				uint sidx = sy * spr.w + sx;
				uint16 pix = spr.pixels[sidx];
				if (pix == kSprTransp)
					continue;
				int tx = dx + sx;
				if (tx < 0 || tx >= dst.w)
					continue;
				uint16 *dp = (uint16 *)dst.getBasePtr(tx, ty);
				if (spr.blend[sidx] != kSprNoBlend)
					pix = blendSprPixel565(pix, *dp, spr.blend[sidx]);
				*dp = pix;
			}
		}
		cx += adv;
	}
}

// ---------------------------------------------------------------------------
// SPRLIST\*MENU.TXT — menu layout parsing, rendering, and event loops.
// All coordinates come from the original game data; the only constants used
// here are the FONTMAX hit-box vertical span (above) and the SAVE_GAME /
// LOAD_GAME / CONTINUE / QUIT pseudo-actions used to thread the click back
// to the engine's existing menu-return state machine.
// ---------------------------------------------------------------------------

bool CryOmni3DEngine_Atlantis::loadMenuLayout(const char *baseName,
        MenuLayout &out) const {
	Common::ScopedPtr<Common::SeekableReadStream> s(
	    openBigFileStream(kFileTypeSprite, baseName));
	if (!s) {
		warning("loadMenuLayout: cannot open SPRLIST\\%s", baseName);
		return false;
	}
	return out.loadFromStream(*s);
}

void CryOmni3DEngine_Atlantis::renderMenuLayout(Graphics::ManagedSurface &dst,
        const MenuLayout &layout, int hoveredTextIdx,
        Common::Array<Common::Rect> &hitRectsOut) const {
	hitRectsOut.clear();
	hitRectsOut.resize(layout.items.size());

	// Sprites first — text labels render on top so they stay legible above
	// the decorative arrows / ornaments.
	for (uint i = 0; i < layout.items.size(); i++) {
		const MenuItem &it = layout.items[i];
		if (it.kind != MenuItem::kSprite)
			continue;
		blitMenuSprite(dst, (uint)it.sprIdx, it.hx, it.hy);
	}

	// Text labels and toggle values.  Hovered entries are drawn with the
	// FONTMAX2.SPR glyph set (bright-yellow) — that is how the original
	// engine produces the hover tint.  A kToggle item renders its currently
	// selected option string at the same anchor as a plain text label.
	for (uint i = 0; i < layout.items.size(); i++) {
		const MenuItem &it = layout.items[i];
		Common::String label;
		if (it.kind == MenuItem::kText) {
			label = it.text;
		} else if (it.kind == MenuItem::kToggle && !it.options.empty()) {
			int sel = (it.selected >= 0 && it.selected < (int)it.options.size())
			          ? it.selected : 0;
			label = it.options[sel];
		} else {
			continue;
		}

		int w = fontMaxStringWidth(label);
		int x = it.centerX ? (640 - w) / 2 : it.anchorX;
		bool hover = ((int)i == hoveredTextIdx);
		drawFontMaxText(dst, label, x, it.anchorY, hover);

		hitRectsOut[i] = Common::Rect(x, it.anchorY - kFontMaxHitTop,
		                              x + w, it.anchorY + kFontMaxHitBottom);
	}
}

// Map a mouse position to the index of the hovered text/toggle item (or -1).
static int hitTestMenu(const MenuLayout &layout,
                       const Common::Array<Common::Rect> &rects,
                       const Common::Point &mouse) {
	for (uint i = 0; i < layout.items.size(); i++) {
		MenuItem::Kind k = layout.items[i].kind;
		if (k != MenuItem::kText && k != MenuItem::kToggle)
			continue;
		if (rects[i].contains(mouse))
			return (int)i;
	}
	return -1;
}

// Composite the in-game menu over the current panorama snapshot.
// All UI coordinates come from SPRLIST\MAINMENU.TXT; we synthesize three
// extra entries (CONTINUE / SAVE GAME / NEW GAME) that the original menu
// expressed via separate screens.  Each synthetic entry uses the same
// FONTMAX rendering and a layout-derived y position.
uint CryOmni3DEngine_Atlantis::displayInGameMenu() {
	// Snapshot the panorama (or a black frame if none is loaded yet).
	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface bg;
	bg.create(640, 480, fmt);
	if (_warpLoaded) {
		const Graphics::Surface *pano = _omni3dMan.getSurface();
		if (pano)
			bg.blitFrom(*pano);
	} else {
		bg.fillRect(Common::Rect(0, 0, 640, 480),
		            fmt.RGBToColor(0, 0, 0));
	}

	// Parse the original menu layout.  MAINMENU.TXT carries three entries
	// authored by Cryo (LOAD GAME / OPTIONS MENU / QUIT GAME); we follow it
	// verbatim — no synthetic insertions.  Escape dismisses the menu (the
	// original behaviour) so a CONTINUE entry is intentionally absent.
	MenuLayout layout;
	loadMenuLayout("MAINMENU.TXT", layout);

	// Classify each label by its ordinal position, never by the label text.
	// MAINMENU.TXT is authored as exactly three buttons in a fixed order —
	// LOAD GAME, OPTIONS MENU, QUIT GAME — and a localized disc only swaps
	// the strings, not the order.  Matching on English substrings ("LOAD",
	// "QUIT") would break every non-English release, so the action for each
	// button is taken from its position in the file instead.  OPTIONS MENU
	// drives the OPTMENU sub-menu.
	enum Action {
		kNone     = 0,
		kContinue = 1,
		kLoad     = 3,
		kQuit     = 5,
		kOptions  = 6,
	};
	static const int kMainMenuActions[] = { kLoad, kOptions, kQuit };
	Common::Array<int> actions;
	actions.resize(layout.items.size());
	uint textOrdinal = 0;
	for (uint i = 0; i < layout.items.size(); i++) {
		if (layout.items[i].kind != MenuItem::kText) {
			actions[i] = kNone;
			continue;
		}
		actions[i] = (textOrdinal < ARRAYSIZE(kMainMenuActions))
		             ? kMainMenuActions[textOrdinal] : kNone;
		textOrdinal++;
	}

	Graphics::ManagedSurface surf;
	surf.create(640, 480, fmt);

	Common::Array<Common::Rect> hitRects;
	int hovered = -1;

	clearKeys();
	showMouse(true);
	setArrowCursor();

	// Initial render so hitRects is populated before any pollEvents/hover query.
	surf.blitFrom(bg);
	renderMenuLayout(surf, layout, -1, hitRects);
	g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);
	g_system->updateScreen();

	uint retval = kContinue;
	bool done = false;

	while (!done && !shouldAbort()) {
		pollEvents();

		// Escape dismisses the menu.
		Common::KeyState k = getNextKey();
		if (k.keycode == Common::KEYCODE_ESCAPE) {
			retval = kContinue;
			break;
		}

		Common::Point mouse = getMousePos();
		int newHover = hitTestMenu(layout, hitRects, mouse);
		if (newHover != hovered) {
			hovered = newHover;
			surf.blitFrom(bg);
			renderMenuLayout(surf, layout, hovered, hitRects);
			g_system->copyRectToScreen(surf.getPixels(), surf.pitch,
			                           0, 0, 640, 480);
		}

		if (getCurrentMouseButton() == 1 && hovered >= 0) {
			int act = actions[hovered];
			waitMouseRelease();
			if (act == kNone)
				continue;
			if (act == kOptions) {
				displayOptionsMenu(bg);
				// Repaint the in-game menu after the sub-menu closes.
				hovered = -1;
				surf.blitFrom(bg);
				renderMenuLayout(surf, layout, -1, hitRects);
				g_system->copyRectToScreen(surf.getPixels(), surf.pitch,
				                           0, 0, 640, 480);
				g_system->updateScreen();
				continue;
			}
			if (act == kQuit && !confirmQuit(bg)) {
				// Player backed out of the "ARE YOU SURE ?" prompt —
				// repaint the in-game menu and stay in the loop.
				hovered = -1;
				surf.blitFrom(bg);
				renderMenuLayout(surf, layout, -1, hitRects);
				g_system->copyRectToScreen(surf.getPixels(), surf.pitch,
				                           0, 0, 640, 480);
				g_system->updateScreen();
				continue;
			}
			retval = (uint)act;
			done = true;
			break;
		}

		musicUpdate();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	clearKeys();

	switch (retval) {
	case kContinue:
		return 0;
	case kLoad: {
		// LOAD GAME: the per-episode save picker — every kept checkpoint of
		// the current player, listed by its EPI.TXT name.  Picking the
		// current chapter's entry restarts that episode (its save was made
		// on chapter entry); picking an earlier one rewinds the story.
		Common::String dummy;
		uint slot = displaySavePicker(false, dummy);
		if (slot > 0) {
			_loadedSave = slot;
			return 28;
		}
		return 0;
	}
	case kQuit:
		// Run the credits slideshow (END.TGA + CREDIT01..10.TGA + 04GENERI.APC)
		// before signalling kAbortQuit.  Only the in-game menu's explicit
		// QUIT GAME button triggers this — other quit paths (Esc dismiss,
		// window close, GMM Quit) bypass the outro by design.
		playQuitOutro();
		return 40;
	default:
		return 0;
	}
}

// Checkpoint name for `chapter`, served from the SPRLIST\EPI.TXT table parsed
// at startup by loadEpisodeNames(): line (chapter - 1) is the name shown for a
// save made on entering that chapter.  Chapter 1 (new game) and unused chapter
// numbers map to an empty entry — they have no checkpoint.
Common::String CryOmni3DEngine_Atlantis::episodeName(uint chapter) const {
	if (chapter < 1 || chapter > _episodeNames.size())
		return Common::String();
	return _episodeNames[chapter - 1];
}

// Quit confirmation — composited from SPRLIST\SUREMENU.TXT.  That file is
// authored as a fixed structure: text item 0 is the prompt ("ARE YOU SURE ?"),
// item 1 the confirm button, item 2 the cancel button, plus two decorative
// /spr ornaments.  A localized disc only swaps the strings, never the order,
// so the buttons are identified by ordinal position rather than label text.
// The prompt line is drawn but stays inert (not a click target).
bool CryOmni3DEngine_Atlantis::confirmQuit(const Graphics::ManagedSurface &bg) {
	MenuLayout layout;
	if (!loadMenuLayout("SUREMENU.TXT", layout))
		return true;  // no confirmation data on this disc — proceed to quit

	int confirmIdx = -1, cancelIdx = -1;
	uint textOrdinal = 0;
	for (uint i = 0; i < layout.items.size(); i++) {
		if (layout.items[i].kind != MenuItem::kText)
			continue;
		if (textOrdinal == 1)
			confirmIdx = (int)i;
		else if (textOrdinal == 2)
			cancelIdx = (int)i;
		textOrdinal++;
	}

	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface surf;
	surf.create(640, 480, fmt);

	Common::Array<Common::Rect> hitRects;
	int hovered = -1;

	clearKeys();
	showMouse(true);
	setArrowCursor();

	surf.blitFrom(bg);
	renderMenuLayout(surf, layout, -1, hitRects);
	g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);
	g_system->updateScreen();

	while (!shouldAbort()) {
		pollEvents();

		if (getNextKey().keycode == Common::KEYCODE_ESCAPE)
			return false;

		// Only the confirm / cancel buttons react; the prompt is inert.
		int hit = hitTestMenu(layout, hitRects, getMousePos());
		if (hit != confirmIdx && hit != cancelIdx)
			hit = -1;
		if (hit != hovered) {
			hovered = hit;
			surf.blitFrom(bg);
			renderMenuLayout(surf, layout, hovered, hitRects);
			g_system->copyRectToScreen(surf.getPixels(), surf.pitch,
			                           0, 0, 640, 480);
		}

		if (getCurrentMouseButton() == 1 && hovered >= 0) {
			waitMouseRelease();
			return hovered == confirmIdx;
		}

		musicUpdate();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	return false;
}

// Config key for the OMNI 3D control mode.  MODE 2 (false) is the default
// look behaviour; MODE 1 (true) inverts the vertical look axis.
static const char *const kOmni3dInvertKey = "atlantis_omni3d_invert";

// Config key for the SONMENU "Volume" master slider — read here and applied
// on top of the per-channel volumes by syncSoundSettings() (engine.cpp).
static const char *const kMasterVolKey = "atlantis_master_volume";

bool CryOmni3DEngine_Atlantis::omni3dInvertY() const {
	return ConfMan.hasKey(kOmni3dInvertKey) && ConfMan.getBool(kOmni3dInvertKey);
}

// Options menu — SPRLIST\OPTMENU.TXT.  Fixed structure on every localized
// disc, classified by ordinal position:
//   kText   ordinal 0 = "OPTIONS MENU" title  (inert)
//           ordinal 1 = SOUND OPTIONS button  (opens SONMENU)
//           ordinal 2 = "SUBTITLES:" label    (inert)
//           ordinal 3 = "OMNI 3D:"  label     (inert)
//           ordinal 4 = OK button             (close)
//   kToggle ordinal 0 = subtitles  ON / OFF
//           ordinal 1 = omni 3d    MODE 2 / MODE 1
void CryOmni3DEngine_Atlantis::displayOptionsMenu(const Graphics::ManagedSurface &bg) {
	MenuLayout layout;
	if (!loadMenuLayout("OPTMENU.TXT", layout))
		return;

	int soundIdx = -1, okIdx = -1, subIdx = -1, omniIdx = -1;
	{
		int textOrd = 0, toggleOrd = 0;
		for (uint i = 0; i < layout.items.size(); i++) {
			if (layout.items[i].kind == MenuItem::kText) {
				if (textOrd == 1)      soundIdx = (int)i;
				else if (textOrd == 4) okIdx    = (int)i;
				textOrd++;
			} else if (layout.items[i].kind == MenuItem::kToggle) {
				if (toggleOrd == 0)      subIdx  = (int)i;
				else if (toggleOrd == 1) omniIdx = (int)i;
				toggleOrd++;
			}
		}
	}

	// Seed the toggles from the live settings.  Both are authored with the
	// "default" label first: SUBTITLES "ON&&OFF", OMNI 3D "MODE 2&&MODE 1".
	if (subIdx >= 0)
		layout.items[subIdx].selected = showSubtitles() ? 0 : 1;
	if (omniIdx >= 0)
		layout.items[omniIdx].selected = omni3dInvertY() ? 1 : 0;

	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface surf;
	surf.create(640, 480, fmt);

	Common::Array<Common::Rect> hitRects;
	int hovered = -1;
	bool done = false;

	clearKeys();
	showMouse(true);
	setArrowCursor();

	surf.blitFrom(bg);
	renderMenuLayout(surf, layout, -1, hitRects);
	g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);
	g_system->updateScreen();

	while (!done && !shouldAbort()) {
		pollEvents();

		if (getNextKey().keycode == Common::KEYCODE_ESCAPE)
			break;

		Common::Point mouse = getMousePos();
		int newHover = hitTestMenu(layout, hitRects, mouse);
		// Only the two buttons and the two toggles react to the mouse.
		if (newHover != soundIdx && newHover != okIdx &&
		    newHover != subIdx && newHover != omniIdx)
			newHover = -1;
		if (newHover != hovered) {
			hovered = newHover;
			surf.blitFrom(bg);
			renderMenuLayout(surf, layout, hovered, hitRects);
			g_system->copyRectToScreen(surf.getPixels(), surf.pitch,
			                           0, 0, 640, 480);
		}

		if (getCurrentMouseButton() == 1 && hovered >= 0) {
			waitMouseRelease();
			if (hovered == okIdx) {
				done = true;
			} else if (hovered == soundIdx) {
				displaySoundMenu(bg);
			} else {
				// Cycle the clicked toggle and persist the new value.
				MenuItem &tog = layout.items[hovered];
				if (!tog.options.empty())
					tog.selected = (tog.selected + 1) % (int)tog.options.size();
				if (hovered == subIdx)
					ConfMan.setBool("subtitles", tog.selected == 0);
				else if (hovered == omniIdx)
					ConfMan.setBool(kOmni3dInvertKey, tog.selected == 1);
			}
			// Repaint after any action (sub-menu, toggle flip).
			hovered = -1;
			surf.blitFrom(bg);
			renderMenuLayout(surf, layout, -1, hitRects);
			g_system->copyRectToScreen(surf.getPixels(), surf.pitch,
			                           0, 0, 640, 480);
		}

		musicUpdate();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	ConfMan.flushToDisk();
	clearKeys();
}

// Play WAV\<apcName> once as an effect with the given stereo balance — the
// SONMENU Left/Right speaker test.  Reuses the newsound SFX handle.
void CryOmni3DEngine_Atlantis::playPannedSound(const char *apcName, int8 balance) {
	Common::SeekableReadStream *apcFile = openBigFileStream(kFileTypeSound, apcName);
	if (!apcFile)
		return;
	Audio::PacketizedAudioStream *apc = Audio::makeAPCStream(*apcFile);
	if (apc) {
		int64 remaining = apcFile->size() - apcFile->pos();
		if (remaining > 0) {
			byte *buf = new byte[remaining];
			apcFile->read(buf, (uint32)remaining);
			apc->queuePacket(new Common::MemoryReadStream(buf, remaining,
			                                              DisposeAfterUse::YES));
		}
		apc->finish();
		if (_mixer->isSoundHandleActive(_newSoundHandle))
			_mixer->stopHandle(_newSoundHandle);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_newSoundHandle, apc,
		                   -1, Audio::Mixer::kMaxChannelVolume, balance);
	}
	delete apcFile;
}

// Sound-options sub-menu — SPRLIST\SONMENU.TXT.  Four horizontal volume
// sliders (Volume / Music / Effects / Voices), each built from a track
// sprite plus a knob sprite whose x is derived from the volume.  "Volume"
// is a master applied by syncSoundSettings().  The Left / Right labels are
// a speaker test that plays WAV\VAGUE_10.APC panned hard to one side.  OK
// commits the volumes to the ScummVM config; Cancel or Escape discards.
void CryOmni3DEngine_Atlantis::displaySoundMenu(const Graphics::ManagedSurface &bg) {
	MenuLayout layout;
	if (!loadMenuLayout("SONMENU.TXT", layout))
		return;

	// SONMENU.TXT lists four track /spr items then four knob /spr items.
	// Tracks give the slider extent (hx + w); knobs give the sprite index
	// and the authored knob y for each row.  Only the knob x is computed,
	// from the live volume.
	int trackItem[4];
	int knobItem[4];
	int nTrack = 0, nKnob = 0;
	for (uint i = 0; i < layout.items.size(); i++) {
		if (layout.items[i].kind != MenuItem::kSprite)
			continue;
		if (nTrack < 4)
			trackItem[nTrack++] = (int)i;
		else if (nKnob < 4)
			knobItem[nKnob++] = (int)i;
	}
	if (nTrack < 4 || nKnob < 4)
		return;  // not the expected slider layout
	const int knobSpr = layout.items[knobItem[0]].sprIdx;
	// The knob travels by its own left edge, so its run stops one knob-width
	// short of the track end — otherwise it overshoots past the bar.
	const int knobW = ((uint)knobSpr < _menuSprites.size())
	                  ? (int)_menuSprites[knobSpr].w : 0;

	// OK / Cancel are text ordinals 7 and 8 (after the title and the six
	// Volume/Music/Effects/Voices/Left/Right labels).
	int leftIdx = -1, rightIdx = -1, okIdx = -1, cancelIdx = -1;
	{
		int textOrd = 0;
		for (uint i = 0; i < layout.items.size(); i++) {
			if (layout.items[i].kind != MenuItem::kText)
				continue;
			if (textOrd == 5)      leftIdx   = (int)i;
			else if (textOrd == 6) rightIdx  = (int)i;
			else if (textOrd == 7) okIdx     = (int)i;
			else if (textOrd == 8) cancelIdx = (int)i;
			textOrd++;
		}
	}

	// Four independent volumes (0..kMaxMixerVolume).  Slider 0 ("Volume") is
	// the master applied by syncSoundSettings(); 1..3 are the ScummVM
	// per-channel volumes.  Each slider moves on its own.
	const int kMaxVol = Audio::Mixer::kMaxMixerVolume;
	int vol[4];
	vol[0] = ConfMan.hasKey(kMasterVolKey) ? ConfMan.getInt(kMasterVolKey)
	                                        : kMaxVol;
	vol[1] = ConfMan.getInt("music_volume");
	vol[2] = ConfMan.getInt("sfx_volume");
	vol[3] = ConfMan.getInt("speech_volume");

	// Fixed hit-boxes for the clickable labels (their text never changes).
	Common::Rect leftRect, rightRect, okRect, cancelRect;
	for (uint i = 0; i < layout.items.size(); i++) {
		const MenuItem &it = layout.items[i];
		if (it.kind != MenuItem::kText)
			continue;
		int w = fontMaxStringWidth(it.text);
		int x = it.centerX ? (640 - w) / 2 : it.anchorX;
		Common::Rect r(x, it.anchorY - kFontMaxHitTop,
		               x + w, it.anchorY + kFontMaxHitBottom);
		if ((int)i == leftIdx)   leftRect   = r;
		if ((int)i == rightIdx)  rightRect  = r;
		if ((int)i == okIdx)     okRect     = r;
		if ((int)i == cancelIdx) cancelRect = r;
	}

	// Static background: panorama + track sprites + any non-knob ornament.
	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface base;
	base.create(640, 480, fmt);
	base.blitFrom(bg);
	for (uint i = 0; i < layout.items.size(); i++) {
		const MenuItem &it = layout.items[i];
		if (it.kind == MenuItem::kSprite && it.sprIdx != knobSpr)
			blitMenuSprite(base, (uint)it.sprIdx, it.hx, it.hy);
	}

	Graphics::ManagedSurface surf;
	surf.create(640, 480, fmt);

	int  activeSlider = -1;
	bool wasDown = false;
	bool done = false;
	bool commit = false;

	clearKeys();
	showMouse(true);
	setArrowCursor();

	while (!done && !shouldAbort()) {
		pollEvents();

		if (getNextKey().keycode == Common::KEYCODE_ESCAPE)
			break;

		Common::Point mouse = getMousePos();
		bool down = (getCurrentMouseButton() == 1);
		bool pressEdge = down && !wasDown;
		wasDown = down;

		if (pressEdge) {
			activeSlider = -1;
			for (int t = 0; t < 4; t++) {
				const MenuItem &tr = layout.items[trackItem[t]];
				Common::Rect r(tr.hx, tr.hy, tr.hx + tr.w, tr.hy + tr.h);
				if (r.contains(mouse)) {
					activeSlider = t;
					break;
				}
			}
			if (activeSlider < 0) {
				if (okRect.contains(mouse)) {
					commit = true;
					done = true;
				} else if (cancelRect.contains(mouse)) {
					done = true;
				} else if (leftRect.contains(mouse)) {
					playPannedSound("VAGUE_10.APC", -127);
				} else if (rightRect.contains(mouse)) {
					playPannedSound("VAGUE_10.APC", 127);
				}
			}
		}

		if (down && activeSlider >= 0) {
			const MenuItem &tr = layout.items[trackItem[activeSlider]];
			int span = MAX(1, tr.w - knobW);
			// The knob is placed by its left edge; bias by half its width
			// so it stays centred on the cursor while dragging.
			int v = ((mouse.x - tr.hx - knobW / 2) * kMaxVol) / span;
			vol[activeSlider] = CLIP(v, 0, kMaxVol);
		}
		if (!down)
			activeSlider = -1;

		// Render: static background + knobs + text labels.
		surf.blitFrom(base);
		for (int t = 0; t < 4; t++) {
			const MenuItem &tr = layout.items[trackItem[t]];
			int span = MAX(1, tr.w - knobW);
			int knobX = tr.hx + (vol[t] * span) / kMaxVol;
			int knobY = layout.items[knobItem[t]].hy;
			blitMenuSprite(surf, (uint)knobSpr, knobX, knobY);
		}
		for (uint i = 0; i < layout.items.size(); i++) {
			const MenuItem &it = layout.items[i];
			if (it.kind != MenuItem::kText)
				continue;
			int w = fontMaxStringWidth(it.text);
			int x = it.centerX ? (640 - w) / 2 : it.anchorX;
			bool hover = ((int)i == okIdx     && okRect.contains(mouse)) ||
			             ((int)i == cancelIdx && cancelRect.contains(mouse)) ||
			             ((int)i == leftIdx   && leftRect.contains(mouse)) ||
			             ((int)i == rightIdx  && rightRect.contains(mouse));
			drawFontMaxText(surf, it.text, x, it.anchorY, hover);
		}
		g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);

		musicUpdate();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	waitMouseRelease();
	clearKeys();

	if (commit) {
		ConfMan.setInt(kMasterVolKey,   vol[0]);
		ConfMan.setInt("music_volume",  vol[1]);
		ConfMan.setInt("sfx_volume",    vol[2]);
		ConfMan.setInt("speech_volume", vol[3]);
		ConfMan.flushToDisk();
		syncSoundSettings();
	}
}

// Per-character advance widths for the player-screen name fields, taken from
// atlantis.exe's table at VA 0x496697 (indexed by glyph = char - 0x20).  These
// are wider than the FONTMIN glyph bitmaps — they include inter-letter spacing.
static const uint8 kPlayerCharWidths[137] = {
	15, 8,13, 6,13,20,17, 6,13,20,11, 6, 4,20,15,12,17, 8,16,16,
	14,13,15,13,16,15, 7, 6, 6, 6, 6,20, 6,17,19,18,19,16,16,19,
	18,12,12,17,16,24,20,23,17,22,17,15,19,20,17,23,22,18,17, 7,
	16, 6,10, 6, 7,16,16,15,15,16,12,16,17, 8, 9,16, 7,19,16,17,
	16,15,13,12,10,17,15,19,17,15,14,16, 6,17,11,16,17,17,15,16,
	15,16,16,16,15,15,15,11,10,10,18,18,16,17,17,17,17,15,23,19,
	15,19, 0,21,17,16,10,17,17,10,20,14,14,13, 3, 5, 8
};

// Player-name renderer for the player-management screen.  Glyphs come from
// FONTMIN.SPR (FONTMIN2.SPR when `hover`); each character is advanced by the
// original's 0x496697 width table, so the layout is identical whichever glyph
// set is used — a hover swap only recolours, it never reflows.
void CryOmni3DEngine_Atlantis::drawPlayerText(Graphics::ManagedSurface &dst,
        const Common::String &text, int anchorX, int anchorY, bool hover) const {
	const Common::Array<SubjSprite> &glyphs =
	    (hover && !_fontMin2Sprites.empty()) ? _fontMin2Sprites : _fontMinSprites;
	int cx = anchorX;
	for (uint i = 0; i < text.size(); ++i) {
		int gi = (int)(unsigned char)text[i] - 0x20;
		if (gi < 0 || gi >= (int)glyphs.size())
			continue;
		const SubjSprite &spr = glyphs[gi];
		if (!spr.pixels.empty()) {
			int dx = cx - spr.xoff;
			int dy = anchorY - spr.yoff;
			for (int sy = 0; sy < (int)spr.h; ++sy) {
				int ty = dy + sy;
				if (ty < 0 || ty >= dst.h)
					continue;
				for (int sx = 0; sx < (int)spr.w; ++sx) {
					uint sidx = sy * spr.w + sx;
					uint16 pix = spr.pixels[sidx];
					if (pix == kSprTransp)
						continue;
					int tx = dx + sx;
					if (tx < 0 || tx >= dst.w)
						continue;
					uint16 *dp = (uint16 *)dst.getBasePtr(tx, ty);
					if (spr.blend[sidx] != kSprNoBlend)
						pix = blendSprPixel565(pix, *dp, spr.blend[sidx]);
					*dp = pix;
				}
			}
		}
		if (gi < (int)ARRAYSIZE(kPlayerCharWidths))
			cx += kPlayerCharWidths[gi];
	}
}

// Player-management screen — the original game's startup select / create /
// delete-player flow.  The PREINTRO.TGA artwork and the SELEMENU.TXT frame
// ("SELECT PLAYER NAME" / "DELETE PLAYER NAME" / "OK") form the backdrop.
// Existing players and a "New Player" entry are listed in FONTMIN; creating a
// player shows a dotted name field above the list.  Returns the chosen player
// id, or -1 if quit.
int CryOmni3DEngine_Atlantis::displayPlayerScreen() {
	Graphics::PixelFormat fmt = g_system->getScreenFormat();

	// Backdrop: the PREINTRO.TGA menu artwork (the "Atlantis" title scene),
	// with the SELEMENU.TXT ornaments and labels composited on top — the same
	// background the main menu uses.
	Graphics::ManagedSurface bg;
	{
		Graphics::Surface *raw = loadTGA(kFileTypeImages, "PREINTRO.TGA");
		if (raw) {
			bg.copyFrom(*raw);
			raw->free();
			delete raw;
		} else {
			bg.create(640, 480, fmt);
			bg.fillRect(Common::Rect(0, 0, 640, 480), fmt.RGBToColor(0, 0, 0));
		}
	}
	MenuLayout decor;
	loadMenuLayout("SELEMENU.TXT", decor);
	Common::Array<Common::Rect> decorRects;
	// Locate the interactive "OK" and "DELETE PLAYER NAME" labels so they can
	// be hover-lit and hit-tested each frame — "OK" validates a name being
	// typed, "DELETE PLAYER NAME" toggles delete mode.
	int okIdx = -1, delIdx = -1;
	for (uint i = 0; i < decor.items.size(); i++) {
		if (decor.items[i].kind != MenuItem::kText)
			continue;
		if (decor.items[i].text == "OK")
			okIdx = (int)i;
		else if (decor.items[i].text == "DELETE PLAYER NAME")
			delIdx = (int)i;
	}
	// Player-row layout — the exact grid the original hit-tests with
	// (atlantis.exe FUN_004211f0, captured live): a row's clickable cell is
	// [kGridY0 + j*kGridStep, kGridY0 + (j+1)*kGridStep) x [kGridX0, +kGridW).
	// The FONTMAX baseline sits on the cell bottom.
	static const int kGridX0   = 175;
	static const int kGridW    = 320;
	static const int kGridY0   = 248;
	static const int kGridStep = 22;
	// The dotted name-entry field's fixed anchor, above the list
	// (atlantis.exe FUN_00420f60 caller at 0x41e3df: EAX=0xA0, EDX=0xBE).
	static const int kEditFieldX = 160;
	static const int kEditFieldY = 190;

	Graphics::ManagedSurface surf;
	surf.create(640, 480, fmt);
	// Render the SELEMENU frame once so decorRects is populated before the
	// first hit-test; it is re-rendered with hover state inside the loop.
	renderMenuLayout(surf, decor, -1, decorRects);

	int  editing    = -1;          // storage slot being typed into, or -1
	int  selected   = -1;          // existing player picked, shown above list
	Common::String editBuf;
	int  result     = -2;          // -2 running, -1 quit, >=0 chosen player
	uint32 lastClickMs = 0;        // press time of the last player-row click
	const uint32 kDoubleClickMs = 500;

	clearKeys();
	showMouse(true);
	setArrowCursor();

	while (result == -2 && !shouldAbort()) {
		pollEvents();

		// Build the visible row list: existing players, then a "New Player"
		// entry while there is still room.  The dotted name-entry field is
		// drawn separately, above the list (see the redraw below).
		const int kRowNew = -2;
		int  rowSlot[kMaxPlayers + 1];
		int  numRows = 0;
		uint numPlayers = 0;
		for (uint i = 0; i < kMaxPlayers; i++) {
			if (!_players[i].empty()) {
				rowSlot[numRows++] = (int)i;
				numPlayers++;
			}
		}
		if ((editing >= 0 ? numPlayers + 1 : numPlayers) < kMaxPlayers)
			rowSlot[numRows++] = kRowNew;

		// Keyboard: while creating a player, capture the typed name.
		Common::KeyState k = getNextKey();
		if (k.keycode != Common::KEYCODE_INVALID) {
			if (editing >= 0) {
				if (k.keycode == Common::KEYCODE_ESCAPE) {
					editing = -1;
					editBuf.clear();
				} else if (k.keycode == Common::KEYCODE_RETURN
				        || k.keycode == Common::KEYCODE_KP_ENTER) {
					if (!editBuf.empty()) {
						_players[editing] = editBuf;
						savePlayers();
						result = editing;   // the new player becomes current
					}
				} else if (k.keycode == Common::KEYCODE_BACKSPACE) {
					if (!editBuf.empty())
						editBuf.deleteLastChar();
				} else if (k.ascii >= 0x20 && k.ascii < 0x7f
				        && editBuf.size() < kPlayerNameLen) {
					// The original rejects a leading space or period.
					if (!(editBuf.empty() && (k.ascii == ' ' || k.ascii == '.')))
						editBuf += (char)k.ascii;
				}
			} else if (k.keycode == Common::KEYCODE_ESCAPE) {
				result = -1;
				break;
			}
		}

		// Mouse hover / click.
		Common::Point mouse = getMousePos();
		int hovered = -1;
		for (int j = 0; j < numRows; j++) {
			Common::Rect r(kGridX0, kGridY0 + j * kGridStep,
			               kGridX0 + kGridW, kGridY0 + (j + 1) * kGridStep);
			if (r.contains(mouse)) {
				hovered = j;
				break;
			}
		}
		// Hover state for the interactive SELEMENU labels.
		bool okHover  = (okIdx  >= 0) && decorRects[okIdx].contains(mouse);
		bool delHover = (delIdx >= 0) && decorRects[delIdx].contains(mouse);

		if (getCurrentMouseButton() == 1) {
			uint32 now = g_system->getMillis();
			waitMouseRelease();
			if (editing >= 0) {
				// While creating a player, clicking OK validates the typed
				// name — the same as pressing Return.
				if (okHover && !editBuf.empty()) {
					_players[editing] = editBuf;
					savePlayers();
					result = editing;   // the new player becomes current
				}
			} else if (delHover) {
				// Delete the selected player after a confirmation dialog.
				// confirmQuit() shows the SUREMENU prompt over the bare title
				// backdrop (bg) rather than the live player screen, so the
				// list, "SELECT PLAYER NAME" / "DELETE PLAYER NAME" / "OK"
				// labels all vanish behind "ARE YOU SURE ?" — matching the
				// original.  SUREMENU.TXT carries its own ornament sprites.
				if (selected >= 0 && confirmQuit(bg)) {
					_players[selected].clear();
					deletePlayerSaves((uint)selected);
					savePlayers();
					selected = -1;
				}
			} else if (okHover) {
				// OK starts the game for the selected existing player.
				if (selected >= 0)
					result = selected;
			} else if (hovered >= 0) {
				int slot = rowSlot[hovered];
				if (slot >= 0) {                  // an existing player
					if (selected == slot
					        && now - lastClickMs < kDoubleClickMs) {
						result = slot;            // double-click → play
					} else {
						selected = slot;          // single click → show name
						lastClickMs = now;
					}
				} else if (slot == kRowNew) {
					// Create: type into the first free storage slot.
					for (uint i = 0; i < kMaxPlayers; i++) {
						if (_players[i].empty()) {
							editing = (int)i;
							editBuf.clear();
							selected = -1;
							break;
						}
					}
				}
			}
		}

		// Redraw.  The SELEMENU frame is re-rendered each frame so the "OK"
		// and "DELETE PLAYER NAME" labels can light up on hover.
		surf.blitFrom(bg);
		renderMenuLayout(surf, decor, okHover ? okIdx
		                              : (delHover ? delIdx : -1), decorRects);
		// Player rows use FONTMIN (FONTMIN2 for the hovered row), left-aligned
		// at the grid's X origin.
		for (int j = 0; j < numRows; j++) {
			int slot = rowSlot[j];
			Common::String label = (slot == kRowNew)
			    ? Common::String("New Player") : _players[slot];
			bool hot = (j == hovered) || (slot >= 0 && slot == selected);
			drawPlayerText(surf, label, kGridX0,
			               kGridY0 + (j + 1) * kGridStep, hot);
		}
		// The name area above the list shows either the new name being typed
		// (with the dotted placeholder), or — once an existing player has been
		// picked — that player's name, plain and not editable.
		if (editing >= 0) {
			Common::String field = editBuf;
			while (field.size() < kPlayerNameLen)
				field += '.';
			drawPlayerText(surf, field, kEditFieldX, kEditFieldY, true);
		} else if (selected >= 0) {
			drawPlayerText(surf, _players[selected], kEditFieldX, kEditFieldY, true);
		}

		g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);
		musicUpdate();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	clearKeys();
	return result == -2 ? -1 : result;
}

// In-game LOAD GAME screen — SPRLIST\LOADMENU.TXT composited over the live
// panorama.  The current player's per-episode saves are listed in the red
// FONTMIN font beneath the "LOAD GAME" title (oldest first, up to 11 visible
// and scrollable), and RESTART EPISODE / OK / Cancel are the FONTMAX buttons
// from the layout, classified by ordinal so localized discs (which only swap
// the strings) work unchanged.  Mirrors atlantis.exe FUN_0041cf78: clicking a
// name selects it, OK loads the selection, RESTART EPISODE reloads the current
// chapter, Cancel/Escape backs out.  Returns the absolute slot (1-based) to
// load, or 0 on cancel.
uint CryOmni3DEngine_Atlantis::displaySavePicker(bool saveMode,
        Common::String &outSaveName) {
	(void)saveMode;       // Atlantis has no manual save: this is load-only.
	(void)outSaveName;

	// SPRLIST\LOADMENU.TXT authored geometry: the episode list sits between
	// the title (y=50) and RESTART EPISODE (y=390); atlantis.exe shows 11 rows.
	const int kListX    = 50;
	const int kListY0   = 133;   // FONTMIN baseline of the first row
	const int kRowStep  = 20;
	const int kVisible  = 11;

	Graphics::PixelFormat fmt = g_system->getScreenFormat();

	// Backdrop: the live panorama (the LOAD screen composites over the scene),
	// or black when the engine is not yet in a place.
	Graphics::ManagedSurface bg;
	bg.create(640, 480, fmt);
	const Graphics::Surface *pano = _warpLoaded ? _omni3dMan.getSurface() : nullptr;
	if (pano)
		bg.blitFrom(*pano);
	else
		bg.fillRect(Common::Rect(0, 0, 640, 480), fmt.RGBToColor(0, 0, 0));

	// Frame: "LOAD GAME" title (text ordinal 0, inert) + RESTART EPISODE / OK
	// / Cancel (ordinals 1/2/3) + the ornament sprites.
	MenuLayout layout;
	if (!loadMenuLayout("LOADMENU.TXT", layout))
		return 0;
	int restartIdx = -1, okIdx = -1, cancelIdx = -1;
	{
		uint textOrd = 0;
		for (uint i = 0; i < layout.items.size(); i++) {
			if (layout.items[i].kind != MenuItem::kText)
				continue;
			if (textOrd == 1)      restartIdx = (int)i;
			else if (textOrd == 2) okIdx      = (int)i;
			else if (textOrd == 3) cancelIdx  = (int)i;
			textOrd++;
		}
	}

	// Collect the player's per-episode saves.  A save's local slot index is
	// its chapter number, so the checkpoint name comes straight from EPI.TXT
	// (episodeName) — the full text, not the 20-char description truncated
	// into the save file.  Older/unnamed saves fall back to that description.
	struct Entry {
		int            localSlot;   // = chapter number
		Common::String name;
	};
	Common::Array<Entry> saves;
	int selected = -1;              // index into saves, or -1
	for (uint ch = 0; ch < kPlayerSlotStride; ch++) {
		int abs = episodeSaveSlot((uint)_currentPlayer, ch);
		Common::InSaveFile *in = _saveFileMan->openForLoading(getSaveStateName(abs));
		if (!in)
			continue;
		char desc[kSaveDescriptionLen + 1];
		memset(desc, 0, sizeof(desc));
		bool ok = (in->read(desc, kSaveDescriptionLen) == kSaveDescriptionLen);
		desc[kSaveDescriptionLen] = '\0';
		delete in;
		if (!ok)
			continue;
		Entry e;
		e.localSlot = (int)ch;
		e.name = episodeName(ch);
		if (e.name.empty())
			e.name = desc[0] ? Common::String(desc)
			                 : Common::String::format("Chapter %u", ch);
		if (ch == _currentCONChapter)
			selected = (int)saves.size();   // pre-select the current episode
		saves.push_back(e);
	}

	// FONTMIN advance width of a name, for a snug per-row hit-box.
	auto playerTextWidth = [&](const Common::String &s) {
		int w = 0;
		for (uint i = 0; i < s.size(); i++) {
			int gi = (int)(unsigned char)s[i] - 0x20;
			if (gi >= 0 && gi < (int)ARRAYSIZE(kPlayerCharWidths))
				w += kPlayerCharWidths[gi];
		}
		return w;
	};

	int scroll = 0;                 // index of the first visible row
	if (selected >= 0)
		scroll = CLIP(selected - kVisible / 2, 0,
		              MAX(0, (int)saves.size() - kVisible));

	Graphics::ManagedSurface surf;
	surf.create(640, 480, fmt);
	Common::Array<Common::Rect> hitRects;

	// Scrollbar geometry (LOADMENU.TXT: /spr=4 track at x=601, /spr=13 thumb).
	// atlantis.exe FUN_0041cf78 makes the list scrollable only once it overflows
	// the 11 visible rows; the thumb is dragged within the track's interactive
	// Y span [0x7b, 0x152] = [123, 338] and maps linearly onto the scroll
	// offset.  Below that count the thumb is parked off-screen (exe sets it to
	// 0xffce).  Find the thumb sprite so we can position/hide it per frame.
	const int kBarTop   = 123;   // exe 0x7b
	const int kBarBot   = 338;   // exe 0x152
	const int kBarH     = kBarBot - kBarTop;   // 215 (0xd7)
	const int kBarX0    = 595;   // right-edge scrollbar column (track ~601..638)
	const int kBarX1    = 640;
	const int maxScroll = MAX(0, (int)saves.size() - kVisible);
	const bool scrollable = maxScroll > 0;
	int thumbItem = -1;
	for (uint i = 0; i < layout.items.size(); i++)
		if (layout.items[i].kind == MenuItem::kSprite
		        && layout.items[i].sprIdx == 13) {
			thumbItem = (int)i;
			break;
		}

	clearKeys();
	showMouse(true);
	setArrowCursor();

	uint result = 0;
	bool done = false;
	while (!done && !shouldAbort()) {
		pollEvents();

		// Keyboard: Escape / Enter act on the selection; the arrows scroll it.
		Common::KeyState k = getNextKey();
		if (k.keycode == Common::KEYCODE_ESCAPE) {
			result = 0;
			break;
		} else if ((k.keycode == Common::KEYCODE_RETURN
		         || k.keycode == Common::KEYCODE_KP_ENTER) && selected >= 0) {
			result = (uint)episodeSaveSlot((uint)_currentPlayer,
			             (uint)saves[selected].localSlot) + 1;
			done = true;
			break;
		} else if (k.keycode == Common::KEYCODE_UP && selected > 0) {
			selected--;
		} else if (k.keycode == Common::KEYCODE_DOWN
		        && selected + 1 < (int)saves.size()) {
			selected++;
		} else if (k.keycode == Common::KEYCODE_PAGEUP) {
			selected = MAX(0, selected - kVisible);
		} else if (k.keycode == Common::KEYCODE_PAGEDOWN) {
			selected = MIN((int)saves.size() - 1, selected + kVisible);
		}
		// Keep the selected row inside the visible window.
		if (selected >= 0) {
			if (selected < scroll)
				scroll = selected;
			else if (selected >= scroll + kVisible)
				scroll = selected - kVisible + 1;
		}

		// Per-row hit-boxes for the visible window.
		Common::Point mouse = getMousePos();
		int rowHover = -1;
		for (int i = 0; i + scroll < (int)saves.size() && i < kVisible; i++) {
			int entry = scroll + i;
			int ay = kListY0 + i * kRowStep;
			int w  = MAX(playerTextWidth(saves[entry].name), 60);
			Common::Rect r(kListX, ay - 17, kListX + w, ay + 4);
			if (r.contains(mouse)) {
				rowHover = entry;
				break;
			}
		}

		// Button hover (only RESTART / OK / Cancel react; the title is inert).
		int btnHover = hitRects.empty() ? -1
		             : hitTestMenu(layout, hitRects, mouse);
		if (btnHover != restartIdx && btnHover != okIdx && btnHover != cancelIdx)
			btnHover = -1;

		// Scrollbar drag (atlantis.exe FUN_0041cf78): holding the button in the
		// track column scrolls the list continuously as the cursor moves.  A
		// single click in the track jumps the thumb there.  Handled before the
		// click logic and without waitMouseRelease so the drag stays live.
		bool draggingBar = false;
		if (scrollable && getCurrentMouseButton() == 1
		        && mouse.x >= kBarX0 && mouse.x <= kBarX1
		        && mouse.y >= kBarTop && mouse.y <= kBarBot) {
			scroll = CLIP((mouse.y - kBarTop) * maxScroll / kBarH, 0, maxScroll);
			draggingBar = true;
		}

		if (!draggingBar && getCurrentMouseButton() == 1) {
			waitMouseRelease();
			if (rowHover >= 0) {
				selected = rowHover;
			} else if (btnHover == okIdx) {
				if (selected >= 0) {
					result = (uint)episodeSaveSlot((uint)_currentPlayer,
					             (uint)saves[selected].localSlot) + 1;
					done = true;
					break;
				}
			} else if (btnHover == restartIdx) {
				// RESTART EPISODE — reload the current chapter's checkpoint.
				int slot = episodeSaveSlot((uint)_currentPlayer, _currentCONChapter);
				if (saveSlotExists(slot)) {
					result = (uint)slot + 1;
					done = true;
					break;
				}
			} else if (btnHover == cancelIdx) {
				result = 0;
				break;
			}
		}

		// Position the scrollbar thumb to reflect the scroll offset, or park it
		// off-screen when the list fits (mirrors the exe's 0xffce hide).
		if (thumbItem >= 0)
			layout.items[thumbItem].hy = scrollable
			    ? kBarTop + (maxScroll > 0 ? scroll * kBarH / maxScroll : 0)
			    : -100;

		// Redraw: backdrop, LOADMENU frame, then the episode list on top.
		surf.blitFrom(bg);
		renderMenuLayout(surf, layout, btnHover, hitRects);
		for (int i = 0; i + scroll < (int)saves.size() && i < kVisible; i++) {
			int entry = scroll + i;
			drawPlayerText(surf, saves[entry].name, kListX,
			               kListY0 + i * kRowStep, entry == selected);
		}

		g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);

		musicUpdate();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	clearKeys();
	return result;
}

} // namespace Atlantis
} // namespace CryOmni3D
