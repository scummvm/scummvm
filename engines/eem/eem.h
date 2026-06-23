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

/// VGA palette size in bytes (256 colours × RGB)
const uint kPalSize = 768;

void fadeCurrentPaletteToBlack(uint delayMs = 8);
void fadePaletteFromBlack(const byte *target, uint delayMs = 8);

/// _ScreenDriver dispatch table @ 1a35:0e5e (fallback @ 1a35:0e54).
/// 14 entries total: 
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
/// `gameDescriptions[]` in `detection.cpp`).
enum Variant {
	kVariantCD       = 0,
	kVariantFloppy   = 1,
	kVariantLondonCD = 2,
	kVariantMac      = 3,
};

/// `_Partner @ 29be:7918`. Selected at the partner-pick screen
/// (_DoChoosePartner @ 1a35:0756) and persisted in the player profile.
enum Partner {
	kPartnerJake  = 0,
	kPartnerJenny = 1,
};

constexpr int kScreenWidth  = 320;
constexpr int kScreenHeight = 200;
constexpr int kMacScreenWidth  = 512;
constexpr int kMacScreenHeight = 384;

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
	bool isFloppy() const { return _variant == kVariantFloppy || isDemo(); }
	bool isLondon() const { return _variant == kVariantLondonCD; }
	bool isMacintosh() const { return _variant == kVariantMac; }
	bool isDemo() const {
		return _gameDescription && (_gameDescription->flags & ADGF_DEMO);
	}
	int screenWidth() const { return isMacintosh() ? kMacScreenWidth : kScreenWidth; }
	int screenHeight() const { return isMacintosh() ? kMacScreenHeight : kScreenHeight; }
	int scaleX(int x) const {
		return isMacintosh() ? scaleCoord(x, kMacScreenWidth, kScreenWidth) : x;
	}
	int scaleY(int y) const {
		return isMacintosh() ? scaleCoord(y, kMacScreenHeight, kScreenHeight) : y;
	}
	int unscaleX(int x) const {
		return isMacintosh() ? scaleCoord(x, kScreenWidth, kMacScreenWidth) : x;
	}
	int unscaleY(int y) const {
		return isMacintosh() ? scaleCoord(y, kScreenHeight, kMacScreenHeight) : y;
	}
	Common::Point scalePoint(int x, int y) const {
		return Common::Point(scaleX(x), scaleY(y));
	}
	Common::Rect scaleRect(const Common::Rect &r) const {
		return Common::Rect(scaleX(r.left), scaleY(r.top),
							scaleX(r.right), scaleY(r.bottom));
	}

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	int getAutosaveSlot() const override { return -1; }

	Common::Error saveGameStream(Common::WriteStream *stream,
								  bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	Common::Error saveProfile(const Common::String &name);

	bool loadProfile(const Common::String &name);

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

	/// `_DisplayClue @ 2404:05e6`. 
	void displayClue(const byte *clueBlock);

	/// EEM2/London `_DoPuzzle @ 2542:1482`. A clue entry can gate the rest of
	/// itself behind a "check the manual / a real map" puzzle
	bool doPuzzle(uint puzzleId);

	void displayFloppyHotspotDialog(uint siteNum, uint hotIdx);

	bool floppyHotspotSearched(uint siteNum, uint hotspotIdx) const;

	/// Active player name (= profile-save description).
	const Common::String &playerName() const { return _playerName; }

	/// Apply a ClueEntry's side effects (notebook, gallery, site flags).
	void applyClueSideEffects(const byte *entry);

	void doNotebook();
	void doGallery();
	bool moreInfo(const byte *gd, uint suspectIdx,
				   const Picture &galBg, bool haveBg);
	void doBigMap();

	/// Accuse flow. `_DoAccuseGallery @ 1df2:0a31` + `_DisplayEnding @ 1df2:0548`.
	void doAccuse();
	void doAccuseFloppy();

	/// Accuse-notes screen (PIC 0x1A7). Outer loop of `_DoAccuse @ 1df2:0bdd`.
	/// Returns true if the player committed (SOLVE clicked), false on ESC.
	bool doAccuseNotes();

	void drawKDBalloonOverCurrentScreen(Common::String text);

	/// `_KDHelp @ 1560:010a` + `_DisplayHint @ 1560:0009`. 
	void doHelp();

	/// `_InterfaceHelp @ 1560:0205`. Walks `HelpData @ 29be:00c8`.
	void doInterfaceHelp(uint num = 0);

	/// `_GetKDTextBalloon @ 1df2:0105`.
	uint16 getKDTextBalloon(byte firstChar) const;

	/// Pick a shorter balloon sibling when wrapped text leaves empty lines.
	uint16 fitBalloonToText(uint16 bubNum, const Common::String &text);

	/// `_ParseString @ 1b66:07c3`. 
	Common::String parseString(const Common::String &raw,
							   const Common::String &playerName,
							   uint partner) const;

	/// Load the partner gesture (animId + anchor) for concurrent playback with
	/// the clue balloon/voice. false if out of range.
	bool loadKdAnim(uint16 num, Animation &anim, int &px, int &py,
					uint16 &animId);

	void setPartnerEraseBg(const Graphics::ManagedSurface *bg);

	/// Partner idle wait-anim that `displayClue` loops once a clue gesture's
	/// one-shot ends. Set around `displayClue` by the site loop.
	void setPartnerIdleAnim(bool has, uint16 animId, int x, int y) {
		_hasPartnerIdle = has;
		_partnerIdleAnimId = animId;
		_partnerIdleX = x;
		_partnerIdleY = y;
	}

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

	void drawFloppyBubbleIndicator(Graphics::ManagedSurface &dst,
								   uint16 bubNum, int ballX, int ballY,
								   bool endIndicator);

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
	bool mysteryTierRange(uint stage, uint &lo, uint &hi) const;

	void advanceChainStageAfterSolve(uint mysteryNum);
	void applySkipRepeatedCasesOption();

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
	/// 320x200 screen. 
	/// If @p holdLastFrame is true the call blocks on the final frame
	/// until the user clicks or hits a key — used for the title screen.
	/// If @p fadeIn is true the first decoded frame is copied while the
	/// palette is black, then ramped in like `_OpenFadeIn`.
	void playAnm(const Common::Path &path, uint frameDelayMs = 120,
				 bool holdLastFrame = false, bool fadeIn = false,
				 bool setSkipIntroOnEsc = true);

private:
	void interruptAudio(bool stopMusicToo = true);

	void showEAKidsLogo();
	void showHighScoreLogo();
	void showFloppyStormLogo();

	bool waitIntroDelay(uint32 maxMs);
	void runMacStartup();
	void showMacEAKidsLogo();
	void showMacStillLogo(uint picId, uint palId, uint holdMs,
						  bool playThunder);
	void showMacTitleIntro();

	void runLondonStartup();
	/// Start London mystery 0 after a freshly-created detective chooses a partner.
	bool startLondonTrainingMystery();
	void showLondonLogo(uint picId, uint palId, uint holdMs);
	void showLondonCharSelect();
	void playLondonInitCluesAnim(uint16 caseType, const Picture &bg,
								 bool haveBriefingBg);
	void playCdFloppyInitCluesAnim(uint16 caseType, bool floppy,
								   const Picture &bg, bool haveBriefingBg);

	void doProfilePicker();
	void doNewPlayer();
	void doChoosePartner();

	/// Display the per-mystery ending pages from `E<num>.BIN`. Mirrors
	/// `_DisplayEnding @ 1df2:0548` + `_DisplayEndingPage @ 1df2:044c`.
	/// File format: u16 page count, then N pages of
	/// `{ u16 picNum, u16 x1, u16 y1, u16 x2, u16 y2,
	///    char text[] (null-terminated, ParseString placeholders) }`.
	/// Returns the caller's nav direction (per `[BP-0x18]` @ 1df2:0723):
	///   -1 → previous mystery (LEFT on first page or click in PrevPageRect),
	///    0 → exit scrapbook (ESC / quit),
	///   +1 → next mystery (RIGHT/SPACE/Enter/click on last page).
	int doShowEnding(uint num, bool firstPage = true);

	/// EEM1 `_ShowScrapbook(stage, 0) @ 1f78:0642`; EEM2/London scrapbook
	/// `2046:09dd`. Walks the `mysteryTierRange(stage)` cases, skipping
	/// unsolved entries in the current chain stage. 
	void doShowScrapbook(uint stage);

	void doActionScreen();
	void doCaseSelection();
	void doSiteLoop();

	void doSetup();

	void setupDrawScreen();
	Common::KeyCode setupShowFullscreenPic(uint16 picId, bool transparent);
	void setupLeave();

	void doSetupLondon();
	void setupDrawScreenLondon();
	/// London "profile saved" confirmation: PIC 0x203 centred over the setup
	/// screen, dismissed by any click/key (caller redraws the setup screen).
	void setupShowSavedConfirm();

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
	static int scaleCoord(int value, int target, int source) {
		const bool negative = value < 0;
		const int magnitude = negative ? -value : value;
		const int scaled = (magnitude * target + source / 2) / source;
		return negative ? -scaled : scaled;
	}

	Common::String _playerName;  ///< Substituted into 0x80 placeholders.

	/// London passport gender (EEM2 `_NewPlayer @ 1cd3:0f27` gender pick,
	/// `DAT_3036_4c4c`: left/0 = male, right/1 = female). Drives the player
	/// pronoun opcodes 0x86/0x87/0x88 in `parseString` (he·him·his / she·her·
	/// her). EEM1 has no gender selection, so it stays false (male)
	bool _playerFemale = false;

	/// `_PlayerRecord.SolvedMysteries[55]`. 0=unsolved, 1=solved, 2=first-try.
	uint8 _mysteriesSolved[55] = {};
	uint8 _chainStage = 1;

	bool _voiceOn = true;
	bool _musicOn = true;

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
	EEMFont    _dialogFont;      ///< Mac small FONT used inside speech balloons.

	Common::Array<byte> _sitePals; ///< 40 × 768 bytes, 6-bit VGA.

	uint16 _lastScreen;  ///< `_LastScreen @ 2d5d:3f24`.
	uint16 _nextScreen;  ///< `_NextScreen @ 2d5d:3f26`.
	uint8  _partner;     ///< `_Partner`: 0=Jake, 1=Jenny.

	/// ESC during intro: skip remaining opening-anim chain.
	bool _skipIntro = false;

	/// Partner-less scene that `displayClue` composites the gesture/idle over.
	Graphics::ManagedSurface _partnerEraseBg;

	/// Idle wait-anim to resume after a clue gesture (see setPartnerIdleAnim).
	bool   _hasPartnerIdle = false;
	uint16 _partnerIdleAnimId = 0;
	int    _partnerIdleX = 0;
	int    _partnerIdleY = 0;

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
