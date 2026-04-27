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
 * Based on the original engine code by EA Kids / Storm Software (1994).
 *
 */

#ifndef EEM_EEM_H
#define EEM_EEM_H

#include "common/array.h"
#include "common/platform.h"
#include "common/random.h"
#include "common/scummsys.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "eem/animation.h"
#include "eem/font.h"
#include "eem/mystery.h"
#include "eem/resource.h"

namespace EEM {

/**
 * Screen IDs used by the original ScreenDriver dispatch table at 1a35:0e5e.
 * The table holds 14 (id, handler) entries; the loop iterates until it finds
 * a matching id and calls its handler. ID 0xFFFF is the exit sentinel.
 */
enum ScreenId {
	kScreenInvalid       = 0xFFFF,
	kScreenChoosePartner = 0x09,  ///< _DoChoosePartner @ 1a35:0756 (boy/girl picker)
	kScreenTitle         = 0x0B   ///< _ShowTitlePage @ 1a35:06b7
};

class EEMEngine : public Engine {
public:
	EEMEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~EEMEngine() override;

	Common::Error run() override;

	const char *getGameId() const;
	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) override;

	const ADGameDescription *_gameDescription;

	DBDArchive &getPics()    { return _picsArchive; }
	DBDArchive &getAni()     { return _aniArchive; }
	DBDArchive &getSites()   { return _sitesArchive; }
	DBDArchive &getBalloons(){ return _balloonArchive; }
	DBDArchive &getButtons() { return _buttonArchive; }
	Mystery    &getMystery() { return _mystery; }
	const EEMFont &getFont() const { return _font; }
	uint8       getPartnerIndex() const { return _partner; }

	/// Display one ClueBlock. @p clueBlock points at the u16 frame count
	/// followed by 62-byte ClueEntries. Mirrors _DisplayClue @ 2404:05e6.
	void displayClue(const byte *clueBlock);

	/// Apply a single ClueEntry's side effects — notebook adds, gallery
	/// updates, site flags. Called both by `displayClue` after a normal
	/// click-through and when the player ESC-skips a multi-entry clue.
	void applyClueSideEffects(const byte *entry);

	/// Show clue/notebook screen. Mirrors `_DrawNotes` @ 161e:01d0.
	void doNotebook();

	/// Show suspect gallery. Mirrors `_DrawGallery` @ 158f:0046.
	void doGallery();

	/// Show big map; click chooses next site. Mirrors `_DoBigMap` @ 20fe:09e7.
	void doBigMap();

	/// Run the accuse flow (pick suspect, evaluate chains, show ending).
	/// Mirrors `_DoAccuseGallery` @ 1df2:0a31 + `_DisplayEnding` @ 1df2:0548.
	void doAccuse();

	/// Show a host hint from `KDTextIndex`. Mirrors `_KDHelp` @ 1560:010a +
	/// `_DisplayHint` @ 1560:0009. Cycles between the two hint slots that
	/// the original engine tracks via `_SawHelpHint`.
	void doHelp();

	/// Display the interface-help picture sequence. Mirrors `_InterfaceHelp`
	/// @ 1560:0205 — walks `HelpData @ 29be:00c8`, blits each pic fullscreen,
	/// and waits for click / key (ESC ends the cycle).
	void doInterfaceHelp(uint num = 0);

	/// First-char-dispatch balloon picker. Mirrors `_GetKDTextBalloon @
	/// 1df2:0105`. For digit first chars (0..9) returns balloon from the
	/// table at `29be:1064`; for any other char returns the constant
	/// `*(u16*)29be:1068 = 0x17`. The original branch is on Borland's
	/// ctype-bit-1 (= digit) at `29be:2be1 + char`.
	uint16 getKDTextBalloon(byte firstChar) const;

	/// Substitute the 0x80..0x89 control bytes the engine uses inside
	/// `TextBlock` strings. Mirrors `_ParseString @ 1b66:07c3`; jump
	/// table at 1b66:0cbe. Used by every clue / hint / balloon caller.
	Common::String parseString(const Common::String &raw,
							   const Common::String &playerName,
							   uint partner) const;

	/// Play the partner's one-shot reaction animation slot @num. Mirrors
	/// `_DoKDAnim @ 168d:028a` + `_PlayAnimation @ 172b:1f46`. The
	/// per-partner (animId, x, y) come from `_WaitAnims[1+num] @ 29be:0228`,
	/// and the per-frame timing follows the sequence script that the
	/// original would index at `_AnimationSequences[seqnum=animId]`. Used
	/// by `displayClue` when a ClueEntry's KD-anim field (+0x3a) is set —
	/// e.g. Jenny's "take a picture" gesture when the player searches an
	/// NPC. Blocks until the script's first 0x80 marker.
	void playKdAnim(uint16 num);

	/// Provide a "clean" 320x200 backdrop for the next `playKdAnim` (and
	/// any future blocking partner-anim playback) to use as the
	/// background-erase source. Without this, the camera animation would
	/// composite on top of the static partner sprite from the screen and
	/// the previous resting frame would bleed through transparent pixels.
	/// `SiteScreen` calls this with its `_bgSnapshot` (site BG + static
	/// drops, no NPCs / partner) before invoking `displayClue` from a
	/// hotspot click. Pass `nullptr` to clear.
	void setPartnerEraseBg(const Graphics::ManagedSurface *bg);

	/// Look up balloon-text-inset metadata. Mirrors the 52-entry table at
	/// `29be:0875`, indexed by `(bubNum & 0x7F)`. 10 bytes per entry; only
	/// the first 3 fields (x inset, y inset, text width) are used for
	/// rendering. Returns false if `bubNum` is outside the table.
	bool getBalloonInsets(uint16 bubNum, uint16 &xInset, uint16 &yInset,
						  uint16 &textW) const;

	/// "Are you sure?" yes/no dialog. Mirrors `_AreYouSure` @ 1a35:0a5c.
	/// Returns true if the user picked YES.
	bool areYouSure();

private:
	/**
	 * Central dispatch loop matching the original _ScreenDriver @ 1a35:0dc1.
	 * Each iteration calls the screen handler that matches _nextScreen.
	 * Handlers update _lastScreen / _nextScreen and return; the loop exits
	 * when _nextScreen == kScreenInvalid.
	 */
	void screenDriver();

	/**
	 * Open the five .DBD/.DBX archive pairs the way _InitGraphicsSystem
	 * @ 172b:0145 does at boot.
	 */
	bool openArchives();

	/** Slurp SITEPALS into @c _sitePals. Mirrors _ReadPalettes @ 172b:0d89. */
	bool loadSitePalettes();

	/**
	 * Upload palette index @p num (one of 40 stored in SITEPALS) to the
	 * screen, with the VGA-DAC 6-bit-to-8-bit shift. Mirrors _GetPalette
	 * @ 172b:0e80 followed by _setmany @ 1000:0930.
	 */
	void setSitePalette(uint num);

	/**
	 * Upload a 6-bit VGA palette read from the head of an .ANM file (the
	 * first 0x300 bytes per Load_Sequence @ 2503:0006). Used until the
	 * full title-page animation chain is wired in.
	 */
	bool setAnmPalette(const Common::Path &anmPath);

public:
	/// Public so SiteScreen can switch palettes per site.
	void setSitePaletteForSite(uint siteNum) { setSitePalette(siteNum + 1); }
private:

	/** Blit @p pic to @p x, @p y on screen. */
	void blitAt(const Picture &pic, int x, int y);

	/** Hold the current frame for up to @p maxMs or until the user inputs. */
public:
	void waitForInput(uint32 maxMs);
private:

	/**
	 * Play a difference-encoded animation file (.ANM / .A) on the full
	 * 320x200 screen. Mirrors the data flow of `OpenDifferenceAnimation`
	 * @ 2520:0337 → `Load_Sequence` + `Play_Sequence`. Audio cues are
	 * skipped for now. The default frame delay is 120 ms to match the
	 * original FRAME_RATE = 0x78 used by `_DoOpeningAnims`.
	 *
	 * If @p holdLastFrame is true the call blocks on the final frame
	 * until the user clicks or hits a key — used for the title screen.
	 */
	void playAnm(const Common::Path &path, uint frameDelayMs = 120,
				 bool holdLastFrame = false);

	// Screen handlers — port targets in screens/ later.
	void showEAKidsLogo();
	void showHighScoreLogo();
	void doNewPlayer();          ///< Mirrors `_NewPlayer` @ 1c33:0dda
	void doChoosePartner();
	void doCaseSelection();
	void doSiteLoop();

	/// Render the case briefing background + game/book decorations and
	/// display the briefing ClueBlock. Mirrors `_DoInitClues` @ 1a35:0411
	/// minus the live ANI sequence playback.
	void doInitClues();

	Common::String _playerName;  ///< Substituted into 0x80 placeholders

	/// Per-mystery solved state. 0 = unsolved, 1 = solved, 2 = solved
	/// on first try. Mirrors `_PlayerRecord.SolvedMysteries[55]` in the
	/// original `_DisplayCorrect` flow.
	uint8 _mysteriesSolved[55] = {};

	Common::RandomSource _rng;

	DBDArchive _picsArchive;     ///< PICS.DBD/.DBX (sprites, buttons, frame backgrounds)
	DBDArchive _aniArchive;      ///< ANI.DBD/.DBX (multi-frame character animations)
	DBDArchive _sitesArchive;    ///< SITES.DBD/.DBX (one full-screen scene per site)
	DBDArchive _balloonArchive;  ///< BALLOON.DBD/.DBX (speech-balloon sprites)
	DBDArchive _buttonArchive;   ///< BUTTON.DBD/.DBX (per-site labeled map buttons; `_GetButton`)
	Mystery    _mystery;         ///< Currently-loaded case file (M<n>.BIN)
	EEMFont    _font;            ///< FONT.FNT - main 8 px font

	Common::Array<byte> _sitePals; ///< 40 x 768 bytes of 6-bit VGA palettes

	uint16 _lastScreen;  ///< Mirrors _LastScreen @ 2d5d:3f24
	uint16 _nextScreen;  ///< Mirrors _NextScreen @ 2d5d:3f26
	uint8  _partner;     ///< Mirrors _Partner: 0 = boy (Jake), 1 = girl (Jenny)

	/// Set when ESC is pressed during an intro animation or logo. Tells
	/// the opening-anim loop in run() to skip the rest of the chain
	/// instead of asking the user to click through every screen.
	bool _skipIntro = false;

	/// Per-slot rectangles + clue IDs from the most recent notebook
	/// render, populated by the `draw` lambda inside `doNotebook` and
	/// consumed by the click handler. The original walks the notes
	/// inline; we cache the layout to keep click hit-testing simple.
	Common::Array<Common::Rect> _notebookSlotRects;
	Common::Array<uint>         _notebookSlotClues;

	/// Optional clean BG (no partner / NPC sprites) used by `playKdAnim`
	/// to erase the partner's resting frame between camera-anim cells.
	/// Empty when no caller has provided one (PDA / case briefing /
	/// accuse contexts use their own composed backdrops). See
	/// `setPartnerEraseBg`.
	Graphics::ManagedSurface _partnerEraseBg;
};

} // End of namespace EEM

#endif
