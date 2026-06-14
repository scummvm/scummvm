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
 *
 */

#ifndef EEM_EEM_H
#define EEM_EEM_H

#include "common/array.h"
#include "common/keyboard.h"
#include "common/language.h"
#include "common/platform.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "common/serializer.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "engines/savestate.h"

#include "eem/animation.h"
#include "eem/font.h"
#include "eem/mystery.h"
#include "eem/resource.h"

namespace EEM {

class AudioPlayer;
class MusicPlayer;

/// VGA palette size in bytes (256 colours × RGB). Defined in eem.cpp.
const uint kPalSize = 768;

/// Palette fade helpers (defined in eem.cpp): ramp the VGA palette to / from
/// black over 16 steps, matching `_FadeToBlack` / `_OpenFadeIn`.
void fadeCurrentPaletteToBlack(uint delayMs = 8);
void fadePaletteFromBlack(const byte *target, uint delayMs = 8);

/// _ScreenDriver dispatch table @ 1a35:0e5e (fallback @ 1a35:0e54).
/// 14 entries total: each is a screen ID + near fn ptr at +0x1c; driver
/// tail-calls via `JMP word ptr CS:[BX + 0x1c]`. Handler bodies update
/// _NextScreen before returning. Handlers @ 1a35:0dec..0e4f:
///
///   0  INIT_CLUES → `_PreLoad` + `_DoInitClues`, writes _NextScreen=1
///   1  MAP        → `_DoMapScreen @ 20fe:120b` (writes 3=site clicked,
///                   6=setup, 0xffff=quit)
///   2  MAP_ALT    → same handler as 1 (used when `_LastScreen == 2` to
///                   swap the briefcase anim)
///   3  SITE       → `_DoSiteLoop @ 168d:03f4`
///   4  NOTEBOOK   → `_DoNotebook @ 161e:0500`
///   5  GALLERY    → `_DoGallery @ 158f:065b`
///   6  SETUP      → `_DoSetup @ 1f78:044e`
///   7  ACCUSE     → `_DoAccuse @ 1df2:0bdd` (win → 0xc, lose → _LastScreen)
///   8  PROFILE    → `screen8_handler @ 1c33:1012`; tail sets =9
///   9  PARTNER    → `_DoChoosePartner @ 1a35:0756`; sets =0xc inside
///   0xa CHOOSE_MYSTERY → `_DoChooseMystery` + `_CaseSelection`; presets
///                   _NextScreen=0 so a successful pick falls through to
///                   INIT_CLUES
///   0xb TITLE     → floppy `_DoTitle_Floppy` plays TITLE.ANM, writes =8.
///                   CD shows TITLE.ANM in `_DoOpeningAnims`, so this
///                   handler is usually unused there.
///   0xc ACTION    → `_ActionScreen @ 1c33:195b` — Choose A Mystery /
///                   Practice / See ScrapBook 1..3. Action 1 sets =0xa.
///   0xFFFF SENTINEL → exit loop
///
/// State writes (via xrefs to `_NextScreen @ 2d5d:3f26`): `_DisplayCorrect`
/// writes 0xc, `_DisplayAlibi` writes `_LastScreen`, `_DoSiteLoop` writes
/// 1/3/4 plus 0xffff on ESC.
enum ScreenId {
	kScreenInvalid        = 0xFFFF,
	kScreenInitClues      = 0x00,
	kScreenMap            = 0x01,
	kScreenMapAlt         = 0x02,
	kScreenSite           = 0x03,
	kScreenNotebook       = 0x04,
	kScreenGallery        = 0x05,
	kScreenSetup          = 0x06,
	kScreenAccuse         = 0x07,
	kScreenProfile        = 0x08,
	kScreenChoosePartner  = 0x09,
	kScreenChooseMystery  = 0x0A,
	kScreenTitle          = 0x0B,
	kScreenAction         = 0x0C
};

/// Distribution variant from `ADGameDescription::extra` (set by
/// `gameDescriptions[]` in `detection.cpp`). Gates filename selection
/// (TRAVEL-N.XMI vs MUS%05u.XMI, FANFARE2.XMI vs MUS00005.XMI,
/// PHONESL.VOC vs PHONE.VOC), opening-anim flow (MOVIE.ANM vs
/// ANIM01..20.A), and per-variant SFX (DING.VOC / NEWSCAN.VOC ship only
/// with floppy).
enum Variant {
	kVariantCD       = 0,
	kVariantFloppy   = 1,
	kVariantLondonCD = 2, ///< Eagle Eye Mysteries in London (EEM2CD.EXE).
};

/// `_Partner @ 29be:7918`. Selected at the partner-pick screen
/// (_DoChoosePartner @ 1a35:0756) and persisted in the player profile.
enum Partner {
	kPartnerJake  = 0,
	kPartnerJenny = 1,
};

/// VGA mode 13h dimensions (initGraphics(320, 200) in `EEMEngine::run`).
constexpr int kScreenWidth  = 320;
constexpr int kScreenHeight = 200;

/// Shared PDA-frame navigation rects (PIC 0x3f) — reachable from Site,
/// Notebook, Gallery, Accuse, and MoreInfo. Original `_NoteButtons` table
/// @ 29be:0147 + the site-screen hit tests in `_DoSiteLoop @ 168d:03f4`.
constexpr Common::Rect kPdaSiteRect             (Common::Point(35, 111), 21, 25);
constexpr Common::Rect kPdaPartnerFootMapRect   (Common::Point( 7, 177), 50, 23);
constexpr Common::Rect kPdaPartnerHeadHintRect  (Common::Point( 5,  80), 39, 30);

class EEMEngine : public Engine {
public:
	EEMEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~EEMEngine() override;

	Common::Error run() override;

	const char *getGameId() const;
	Common::Platform getPlatform() const;
	Variant getVariant() const { return _variant; }
	bool isFloppy() const { return _variant == kVariantFloppy; }
	/// EEM2 ("Eagle Eye Mysteries in London") — reimplementation in progress.
	/// Shares the EEM1 DBD/palette/font formats, but ships a 63-entry
	/// "SITEPALS." file and uses different picture/palette IDs per screen.
	bool isLondon() const { return _variant == kVariantLondonCD; }

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	// Autosave disabled: profile picker (screen8_handler @ 1c33:1012)
	// lists `*.PLR` and a slot-0 autosave would appear as a profile.
	int getAutosaveSlot() const override { return -1; }

	Common::Error saveGameStream(Common::WriteStream *stream,
								  bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	// Per-profile saves: `_PlayerRecord` @ 2d5d:3f6a (159 bytes), written
	// by `_SavePlayerRecord @ 1c33:034f` to `C:\EEMCDSAV\<name>.PLR`.
	// Each save slot maps to one profile (slot description = the
	// player name) — same approach Wetlands uses. `screen8_handler @
	// 1c33:1012` walks `*.PLR`, lets the player pick a profile, and calls
	// `_LoadPlayerRecord`.

	/// `_SavePlayerRecord @ 1c33:034f`.
	Common::Error saveProfile(const Common::String &name);

	/// `_LoadPlayerRecord @ 1c33:03a6`.
	bool loadProfile(const Common::String &name);

	/// `_findfirst("*.PLR")` walk inside `screen8_handler`.
	SaveStateList listProfiles() const;

	const ADGameDescription *_gameDescription;
	Variant _variant = kVariantCD;
	Common::Language _language = Common::EN_ANY;

	/// Spanish floppy release.
	bool isSpanish() const { return _language == Common::ES_ESP; }

	DBDArchive &getPics()    { return _picsArchive; }
	DBDArchive &getAni()     { return _aniArchive; }
	DBDArchive &getSites()   { return _sitesArchive; }
	DBDArchive &getBalloons() { return _balloonArchive; }
	DBDArchive &getButtons() { return _buttonArchive; }
	Mystery    &getMystery() { return _mystery; }
	const EEMFont &getFont() const { return _font; }
	uint8       getPartnerIndex() const { return _partner; }

	/// Red-outline cursor for interactive regions without original highlight art.
	void setInteractiveMouseCursor(bool active);

	/// Interactive cursor over searchable hotspots.
	void setHotspotMouseCursor(bool active);

	/// EEM2 `_SwitchMouse @ 17ee:2c83`: swap the cursor SHAPE to one of the
	/// seven loaded cursors by ID (0 arrow, 1 wait, 2/3 examine, 4/5 partner
	/// hand, 6 approach). London site hotspots carry a cursor ID at row +0xc;
	/// EEM1 cursors are all 0 so this is a no-op there.
	void setSiteHotspotCursorId(int cursorId);

	/// `_DisplayClue @ 2404:05e6`. @p clueBlock points at the u16 frame
	/// count followed by 62-byte ClueEntries.
	void displayClue(const byte *clueBlock);

	/// Floppy hotspot click. `FUN_22dc_0b80 + FUN_1652_00e6 + FUN_1652_006c`.
	/// Locates dialog records in site_data[+6] and dispatches them.
	void displayFloppyHotspotDialog(uint siteNum, uint hotIdx);

	/// `_HotspotSearched_Floppy @ 22dc:096c`. Walks the per-hotspot dialog
	/// records and returns whether the hotspot's searched text index was seen.
	bool floppyHotspotSearched(uint siteNum, uint hotspotIdx) const;

	/// Active player name (= profile-save description).
	const Common::String &playerName() const { return _playerName; }

	/// Apply a ClueEntry's side effects (notebook, gallery, site flags).
	void applyClueSideEffects(const byte *entry);

	/// `_DrawNotes @ 161e:01d0`.
	void doNotebook();

	/// `_DrawGallery @ 158f:0046`.
	void doGallery();

	/// `MoreInfo @ 158f:0419`. Suspect-detail view inside the gallery; button
	/// dispatch via `_HandleMoreButton @ 158f:027d`. Returns true if the
	/// caller should exit `doGallery` (NOTEBOOK / ACCUSE / MAP).
	bool moreInfo(const byte *gd, uint suspectIdx,
				   const Picture &galBg, bool haveBg);

	/// `_DoBigMap @ 20fe:09e7`.
	void doBigMap();

	/// Accuse flow. `_DoAccuseGallery @ 1df2:0a31` + `_DisplayEnding @ 1df2:0548`.
	void doAccuse();
	void doAccuseFloppy();

	/// Accuse-notes screen (PIC 0x1A7). Outer loop of `_DoAccuse @ 1df2:0bdd`.
	/// Returns true if the player committed (SOLVE clicked), false on ESC.
	bool doAccuseNotes();

	/// `_KDHelp @ 1560:010a` + `_DisplayHint @ 1560:0009`. Cycles two
	/// hint slots tracked via `_SawHelpHint`.
	void doHelp();

	/// `_InterfaceHelp @ 1560:0205`. Walks `HelpData @ 29be:00c8`.
	void doInterfaceHelp(uint num = 0);

	/// `_GetKDTextBalloon @ 1df2:0105`. Digits (0..9) → table @ 29be:1064;
	/// otherwise `*(u16*)29be:1068 = 0x17`.
	uint16 getKDTextBalloon(byte firstChar) const;

	/// Pick a shorter balloon sibling when wrapped text leaves empty lines.
	uint16 fitBalloonToText(uint16 bubNum, const Common::String &text);

	/// `_ParseString @ 1b66:07c3`. Substitutes 0x80..0x89 control bytes;
	/// jump table @ 1b66:0cbe.
	Common::String parseString(const Common::String &raw,
							   const Common::String &playerName,
							   uint partner) const;

	/// `_DoKDAnim @ 168d:028a` + `_PlayAnimation @ 172b:1f46`. Per-partner
	/// (animId, x, y) from `_WaitAnims[1+num] @ 29be:0228`. Blocks until
	/// the script's first 0x80 marker.
	void playKdAnim(uint16 num);

	/// Provide a "clean" 320x200 backdrop (site BG + static drops, no
	/// NPCs / partner) for the next `playKdAnim` to use as the
	/// background-erase source. Without this, the camera animation would
	/// composite on top of the static partner sprite and the previous
	/// resting frame would bleed through transparent pixels. `SiteScreen`
	/// passes its `_bgSnapshot` before `displayClue` from a hotspot click.
	/// Pass `nullptr` to clear.
	void setPartnerEraseBg(const Graphics::ManagedSurface *bg);

	/// Balloon-text-inset metadata. 52-entry table @ 29be:0875 (CD) /
	/// 2608:05f9 (floppy), indexed by `(bubNum & 0x7F)`. 10 bytes per
	/// entry: the first 3 fields (x inset, y inset, text width) are used
	/// for text wrap; the last 2 (indicator dX/dY) by
	/// `drawFloppyBubbleIndicator`. Returns false if `bubNum` is outside
	/// the table.
	bool getBalloonInsets(uint16 bubNum, uint16 &xInset, uint16 &yInset,
						  uint16 &textW) const;
	bool getBalloonIndicatorPos(uint16 bubNum, uint16 &dx,
								 uint16 &dy) const;

	/// `FUN_22dc_05c8 @ 22dc:08aa` (mid-page) / `@ 22dc:08c0` (end).
	/// PIC 0xa0 if !endIndicator else PIC 0xa1.
	void drawFloppyBubbleIndicator(Graphics::ManagedSurface &dst,
								   uint16 bubNum, int ballX, int ballY,
								   bool endIndicator);

	/// `_AreYouSure @ 1a35:0a5c`. Returns true on YES.
	bool areYouSure();

private:
	void applyStartupTestOverrides();
	bool areMysteriesSolved(uint lo, uint hi) const;

	/// True if *any* mystery in the inclusive 1-based range [lo, hi] is
	/// marked solved.
	bool anyMysterySolved(uint lo, uint hi) const;

	/// Number of case "books"/tiers in this variant. EEM1 ships three
	/// (BOOK1..3.NME — 24/24/6 cases); EEM2/London ships two (BOOK1..2.NME —
	/// 25 cases each, no BOOK3.NME). See `mysteryTierRange`.
	uint mysteryTierCount() const { return isLondon() ? 2 : 3; }

	/// Inclusive 1-based mystery range [lo, hi] for chain `stage` (1-based).
	/// Returns false when `stage` is not a real tier in this variant.
	/// EEM1 `_DisplayCorrect @ 1df2:073c` (1..24 / 25..48 / 49..54);
	/// EEM2/London `_DisplayCorrect @ 1ea1:0619` (1..25 / 26..50).
	bool mysteryTierRange(uint stage, uint &lo, uint &hi) const;

	void advanceChainStageAfterSolve(uint mysteryNum);
	void applySkipRepeatedCasesOption();

	/// Central dispatch loop matching `_ScreenDriver @ 1a35:0dc1`. Each
	/// iteration calls the handler that matches `_nextScreen`; handlers
	/// update `_lastScreen` / `_nextScreen` and return. Loop exits when
	/// `_nextScreen == kScreenInvalid`.
	void screenDriver();

	/// Re-render helpers for the corresponding `doX()` modal screens.
	void drawNotebookFrame(int &page);

	/// Resolve a single NoteIndex entry to displayable notebook text.
	/// Handles the CD (4-byte) vs floppy (7-byte) entry strides.
	Common::String notebookNoteText(uint clueId, const byte *ni,
									uint16 niCount, bool floppyNb,
									const byte *bufBase,
									uint32 mysSz) const;

	/// Shared state for `doAccuseNotes` helpers (text lookup, pagination,
	/// render). Pointer fields refer to locals owned by `doAccuseNotes`.
	struct AccuseNotesCtx {
		const byte *ni;
		uint16 niCount;
		bool floppyNote;
		const byte *bufBaseNotes;
		const Common::Array<uint> *found;
		int rectX, rectY, rectW, rectH;
		uint expected;
		bool haveBg;
		const Picture *accuseBg;
		Common::Array<Common::Rect> *slotRects;
		Common::Array<uint> *slotClues;
		int *pageBreaks;
		int pageBreaksCap;
		int *numPages;
		int *page;
	};

	/// One NoteIndex entry as displayable accuse-screen text.
	Common::String accuseNoteText(uint clueId, const AccuseNotesCtx &ctx) const;

	/// Recompute `pageBreaks`/`numPages` after `_noteSelected` or font changes.
	void accuseRebuildPagination(const AccuseNotesCtx &ctx);

	/// Render one frame of the accuse-notes screen.
	void accuseDrawScreen(const AccuseNotesCtx &ctx);

	/// Floppy accuse helpers: KD-balloon hint (`_DisplayHint_Floppy @
	/// 1503:00ca`) and the suspect portrait grid
	/// (`_DrawGallery_Floppy @ 154e:0050`).
	void floppyKDHint(uint kdSlot, const byte *kdIdx,
					  const byte *bufBase, uint32 mysSize);
	void displayScrapbookExtra(uint mysteryNum);
	void accuseDrawGallery(int highlighted,
						   Common::Array<Common::Rect> &rects,
						   Common::Array<int> &suspects, uint8 num,
						   bool haveAccuseBg, const Picture &accuseBg);
	void drawGalleryFrame(const byte *gd, uint8 numSuspects,
						  Common::Array<Common::Rect> &slotRects,
						  Common::Array<int> &slotSuspect);
	void drawBigMapOverview(uint32 elapsedMs);
	void drawBigMapDetail(int scrollX, int scrollY,
						  const Common::Array<byte> &mapPixels,
						  uint16 mapW, uint16 mapH,
						  uint32 elapsedMs);
	void drawAccuseGallery(uint8 numSuspects, const byte *gd,
						   int highlighted,
						   Common::Array<Common::Rect> &slotRects,
						   Common::Array<int> &slotSuspect);

	/// Open the five .DBD/.DBX archive pairs. `_InitGraphicsSystem @ 172b:0145`.
	bool openArchives();

	/// `_ReadPalettes @ 172b:0d89`. Slurps SITEPALS into `_sitePals`.
	bool loadSitePalettes();

	/// `_GetPalette @ 172b:0e80` + `_setmany @ 1000:0930`. Uploads one of
	/// 40 SITEPALS palettes with the VGA-DAC 6→8 bit shift.
	void setSitePalette(uint num);

	/// Fill @p out (256 × 3 bytes) with SITEPALS palette @p num, expanded
	/// from 6-bit DAC to 8-bit. False if out of range.
	bool getSitePalette(uint num, byte *out) const;

	/// Upload the 6-bit VGA palette from the head of an .ANM file
	/// (first 0x300 bytes per `Load_Sequence @ 2503:0006`).
	bool setAnmPalette(const Common::Path &anmPath);

public:
	/// Public so SiteScreen can switch palettes per site.
	void setSitePaletteForSite(uint siteNum) { setSitePalette(siteNum + 1); }
private:

	void blitAt(const Picture &pic, int x, int y);

public:
	void waitForInput(uint32 maxMs);

	/// Play a difference-encoded animation file (.ANM / .A) on the full
	/// 320x200 screen. Mirrors the data flow of `OpenDifferenceAnimation
	/// @ 2520:0337` → `Load_Sequence` + `Play_Sequence`. Audio cues are
	/// skipped for now. The default 120 ms frame delay matches the
	/// original `FRAME_RATE = 0x78` used by `_DoOpeningAnims`.
	/// If @p holdLastFrame is true the call blocks on the final frame
	/// until the user clicks or hits a key — used for the title screen.
	/// If @p fadeIn is true the first decoded frame is copied while the
	/// palette is black, then ramped in like `_OpenFadeIn`.
	void playAnm(const Common::Path &path, uint frameDelayMs = 120,
				 bool holdLastFrame = false, bool fadeIn = false,
				 bool setSkipIntroOnEsc = true);

private:
	/// `_CleanMysterySounds @ 202f:05a5` + `_StopMIDI @ 20a2:0512`.
	/// `stopMusicToo=false` keeps MIDI playing across dialog skips.
	void interruptAudio(bool stopMusicToo = true);

	void showEAKidsLogo();
	void showHighScoreLogo();
	void showFloppyStormLogo();

	// --- EEM2 ("Eagle Eye Mysteries in London") — reimplementation in progress ---
	/// Opening sequence + character creation — EEM2's `_DoOpeningAnims`
	/// @ 2721:08e6 then screen 8 profile selection.
	void runLondonStartup();
	/// Start London mystery 0 after a freshly-created detective chooses a partner.
	bool startLondonTrainingMystery();
	/// Blit a full-screen still PIC and fade it in / hold / out using the
	/// given SITEPALS. palette index.
	void showLondonLogo(uint picId, uint palId, uint holdMs);
	/// EEM2 character creation (`_NewPlayer`: palette 0 + background PIC 0xc):
	/// first/last name entry + male/female player-gender selection.
	void showLondonCharSelect();
	/// EEM2 case-intro animation (`_DoInitClues` @ 1abf:03b3): single partner
	/// anim (Jake 0x18 / Jenny 0x71) faded in, then phone1.voc on caseType 1.
	void playLondonInitCluesAnim(uint16 caseType, const Picture &bg,
								 bool haveBriefingBg);
	/// EEM1 CD/floppy case-intro animation (`_DoInitClues` @ 1a35:0411):
	/// game/book/nancy cycle + `_PlayInSequence` partner entrance.
	void playCdFloppyInitCluesAnim(uint16 caseType, bool floppy,
								   const Picture &bg, bool haveBriefingBg);

	/// `screen8_handler @ 1c33:1012`. Profile selector — walks
	/// `listProfiles()`, falls through to `doNewPlayer()` if "New" or
	/// no profiles exist (1c33:1170: `if (saves == 0) _NewPlayer();`).
	void doProfilePicker();
	void doNewPlayer();          ///< `_NewPlayer @ 1c33:0dda`.
	void doChoosePartner();

	/// Display the per-mystery ending pages from `E<num>.BIN`. Mirrors
	/// `_DisplayEnding @ 1df2:0548` + `_DisplayEndingPage @ 1df2:044c`.
	/// File format: u16 page count, then N pages of
	/// `{ u16 picNum, u16 x1, u16 y1, u16 x2, u16 y2,
	///    char text[] (null-terminated, ParseString placeholders) }`.
	/// `_ShowOneScrap @ 1f78:0773` is just `_DisplayEnding(num, 1)`,
	/// so this call also covers the post-mystery scrapbook view.
	/// `firstPage=true` opens at page 0; `false` opens at the last page
	/// (back-nav from `doShowScrapbook`, mirrors the `local_8 = 0` write
	/// at `_ShowScrapbook @ 1f78:067e`).
	/// Returns the caller's nav direction (per `[BP-0x18]` @ 1df2:0723):
	///   -1 → previous mystery (LEFT on first page or click in PrevPageRect),
	///    0 → exit scrapbook (ESC / quit),
	///   +1 → next mystery (RIGHT/SPACE/Enter/click on last page).
	int doShowEnding(uint num, bool firstPage = true);

	/// EEM1 `_ShowScrapbook(stage, 0) @ 1f78:0642`; EEM2/London scrapbook
	/// `FUN_2046_09dd`. Walks the `mysteryTierRange(stage)` cases, skipping
	/// unsolved entries in the current chain stage. Tier sizes are
	/// variant-specific (EEM1 24/24/6; London 25/25, no stage 3).
	void doShowScrapbook(uint stage);

	void doActionScreen();
	void doCaseSelection();
	void doSiteLoop();

	/// `_DoSetup @ 1f78:044e`. Voice on/off (`DAT_2d5d_3f97`), partner
	/// pick via SwapColors on Kid1/Kid2 rects.
	void doSetup();

	/// `doSetup` helpers: redraw BG + label highlights, render a help/
	/// credits card with blocking input wait, and the shared exit fallback.
	void setupDrawScreen();
	Common::KeyCode setupShowFullscreenPic(uint16 picId, bool transparent);
	void setupLeave();

	/// `_DoInitClues @ 1a35:0411` (minus live ANI sequence playback).
	void doInitClues();

	/// Floppy briefing dialog renderer. Walks the dialog record list at
	/// the tail of the floppy InitBlock (`FUN_19bb_042f`, `FUN_22dc_05c8
	/// @ 22dc:05c8`). Record = 11 + textCount bytes:
	///   +0..1 picID  +2..3 picX  +4 picY
	///   +5 balloonId|(mirror<<7)  +6..7 balloonX  +8 balloonY
	///   +9 soundFlag|slot  +10 textCount  +11.. text indices
	void displayFloppyBriefing(const byte *initBlock);

	/// `FUN_22dc_05c8 @ 22dc:05c8`. Renders `count` floppy dialog records.
	/// `lastIndicator`: 0 = none, 1 = PIC 0xa0, 2 = PIC 0xa1 (records
	/// before the last always get PIC 0xa0).
	void displayFloppyDialogRecords(const byte *rec, uint count,
									 uint lastIndicator = 0);

	/// Wait for a click/keypress to advance a floppy dialog page.
	/// Returns true if the user requested to skip the rest (ESC/QUIT),
	/// false to advance one page.
	bool floppyDialogWaitForClick();

public:
	/// `_StartTravelMusic @ 20a2:0595`. Picks `MUS%05d.XMI` from
	/// `_mystery._siteNumber % 5`, one-shot.
	void startTravelMusic();

	/// `_IsMIDIPlaying` spin + `_StopMIDI` cleanup in `_DoSiteLoop`.
	void waitForMusicDone(uint32 maxMs = 60000);

	/// `_StopMIDI @ 20a2:0512`.
	void stopMusic();

	/// EEM2 `_DoApproach @ 1717:009b`: London-only place information
	/// screen with a short VIDEOnn.A clip and text pages from Ann.BIN.
	bool doLondonApproach(uint16 approachId);

	/// `Engine::syncSoundSettings` override. Re-pulls `music_volume`
	/// into the MIDI player's `_masterVolume`.
	void syncSoundSettings() override;

	/// EEM2 `_DoTravel @ 1717:0622` transition music. The matrix entry
	/// (1..3) chooses one of three short one-shot MUS tracks at random.
	void startLondonTravelMusic(uint8 travelKind);
private:

	Common::String _playerName;  ///< Substituted into 0x80 placeholders.

	/// London passport gender (EEM2 `_NewPlayer @ 1cd3:0f27` gender pick,
	/// `DAT_3036_4c4c`: left/0 = male, right/1 = female). Drives the player
	/// pronoun opcodes 0x86/0x87/0x88 in `parseString` (he·him·his / she·her·
	/// her). EEM1 has no passport, so it stays false (male) — matching that
	/// engine's never-written gender flag.
	bool _playerFemale = false;

	/// `_PlayerRecord.SolvedMysteries[55]`. 0=unsolved, 1=solved, 2=first-try.
	uint8 _mysteriesSolved[55] = {};

	/// Current chain/tier the player is at — `DAT_2d5d_3f99`
	/// (`_PlayerRecord +0x2f`):
	///   1 = Junior detective  (mysteries  1..24, "A chain")
	///   2 = Senior detective  (mysteries 25..48, "B chain")
	///   3 = Master detective  (mysteries 49..54, "C chain")
	/// Initialized to 1 in `_NewPlayer @ 1c33:0fa3`; bumped by
	/// `_DisplayCorrect @ 1df2:0853` once every mystery in the current
	/// tier is solved (range checks @ 1df2:080d / 0824 / 0837). Also
	/// gates `_CaseSelection`'s book label and selection list
	/// (1c33:0a87 onwards).
	uint8 _chainStage = 1;

	/// Voice / digital-audio enable flag. `DAT_2d5d_3f97` (`_PlayerRecord
	/// +0x2d`). Set to 1 by `_NewPlayer @ 1c33:0fa3`, toggled by the
	/// SoundOn / SoundOff hot-rects in `_DoSetup @ 1f78:044e`. Gates
	/// every `_PlayVoice` / `_SpoolSound` call site (clue voices,
	/// partner speech, intro VO).
	bool _voiceOn = true;

	/// Set by the profile/new-player screens. London uses it to decide whether
	/// to start the training mystery or resume the loaded profile's menu/state.
	bool _profileCreatedThisSession = false;

	Common::RandomSource _rng;

	DBDArchive _picsArchive;     ///< PICS.DBD/.DBX
	DBDArchive _aniArchive;      ///< ANI.DBD/.DBX
	DBDArchive _sitesArchive;    ///< SITES.DBD/.DBX
	DBDArchive _balloonArchive;  ///< BALLOON.DBD/.DBX
	DBDArchive _buttonArchive;   ///< BUTTON.DBD/.DBX (`_GetButton`)
	Mystery    _mystery;         ///< M<n>.BIN
	EEMFont    _font;            ///< FONT.FNT (8 px)

	Common::Array<byte> _sitePals; ///< 40 × 768 bytes, 6-bit VGA.

	uint16 _lastScreen;  ///< `_LastScreen @ 2d5d:3f24`.
	uint16 _nextScreen;  ///< `_NextScreen @ 2d5d:3f26`.
	uint8  _partner;     ///< `_Partner`: 0=Jake, 1=Jenny.

	/// ESC during intro: skip remaining opening-anim chain.
	bool _skipIntro = false;

	/// Clean BG (no partner/NPC) used by `playKdAnim` between camera-anim
	/// cells. See `setPartnerEraseBg`.
	Graphics::ManagedSurface _partnerEraseBg;

	bool _interactiveMouseCursor = false;
	/// Active EEM2 cursor shape (index into `kLondonCursorPics`). -1 forces a
	/// reload on the next `setSiteHotspotCursorId`.
	int _siteCursorId = 0;

	/// Site whose entrance animation has already played this mystery.
	/// Lives on the engine because PDA/gallery destroys+recreates SiteScreen.
	int _lastSiteArrivalAnim = -1;

	bool _restoredContentDataLoaded = false;

	/// `MIDI.C` family (`_MIDIPlayFile`/`_MIDIPlay`/`_StopMIDI`/
	/// `_StartTravelMusic` @ 20a2:00e2-05c9). Constructed lazily in `run()`.
	MusicPlayer *_music = nullptr;

	/// `SOUND.C` / `SPOOLSND.C`. `_PlayVoice @ 1ff1:023e`,
	/// `_SpoolSound @ 202f:068d`, `_InitMysterySounds @ 202f:05cb`.
public:
	AudioPlayer *_audio = nullptr;

	/// `_NextScreen @ 2d5d:3f26` writer for site loop / inline screens.
	void setNextScreen(ScreenId s) { _nextScreen = s; }
	bool shouldPlaySiteArrival(uint siteNum) const {
		return _lastSiteArrivalAnim != (int)siteNum;
	}
	void markSiteArrivalPlayed(uint siteNum) {
		_lastSiteArrivalAnim = (int)siteNum;
	}
	void resetSiteArrivalState() {
		_lastSiteArrivalAnim = -1;
	}
	void setSiteArrivalState(uint siteNum) {
		_lastSiteArrivalAnim = (int)siteNum;
	}
};

} // End of namespace EEM

#endif
