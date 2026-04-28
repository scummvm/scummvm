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

/**
 * Screen IDs used by the original `_ScreenDriver` dispatch table at
 * 1a35:0e5e (and the fallback at 1a35:0e54). 14 entries total: each
 * one is a screen ID + a near function pointer at offset +0x1c. The
 * driver does `JMP word ptr CS:[BX + 0x1c]`, so handlers tail-call —
 * each one runs the screen and updates `_NextScreen` for the next
 * trip through the dispatcher.
 *
 * IDs and their handlers (verified from disassembly at 1a35:0dec..0e4f):
 *
 *   0  INIT_CLUES  → `_PreLoad` + `_DoInitClues`, sets _NextScreen=1
 *   1  MAP         → `_DoMapScreen` @ 20fe:120b (sets _NextScreen
 *                    inside; 3 = a site was clicked, 6 = setup, etc.)
 *   2  MAP         → same handler as 1 (alternate entry, used when
 *                    `_LastScreen == 2` to swap the briefcase anim)
 *   3  SITE        → `_DoSiteLoop` @ 168d:03f4
 *   4  NOTEBOOK    → `_DoNotebook` @ 161e:0500
 *   5  GALLERY     → `_DoGallery` @ 158f:065b
 *   6  SETUP       → `_DoSetup` @ 1f78:044e
 *   7  ACCUSE      → `_DoAccuse` @ 1df2:0bdd (win → 12, lose → last)
 *   8  PROFILE     → `screen8_handler` @ 1c33:1012; tail sets =9
 *   9  PARTNER     → `_DoChoosePartner` @ 1a35:0756; sets =0xc inside
 *   10 (0xa) CHOOSE_MYSTERY → `_DoChooseMystery` + `_CaseSelection`;
 *                    starts with _NextScreen=0 so a successful pick
 *                    falls through to INIT_CLUES.
 *   11 (0xb) TITLE  → set _NextScreen=8 then dispatch (TITLE.ANM is
 *                    actually shown earlier by `_DoOpeningAnims`, this
 *                    handler is the post-intro "fall into PROFILE"
 *                    redirect)
 *   12 (0xc) ACTION → `_ActionScreen` @ 1c33:195b — post-mystery menu
 *                    ("Solve a Mystery", scrapbook, more mysteries,
 *                    setup). Action 1 sets =10 (CHOOSE_MYSTERY).
 *   0xFFFF SENTINEL → exit loop
 *
 * Screen-driver state writes verified via xrefs to `_NextScreen @
 * 2d5d:3f26`: `_DisplayCorrect` writes 0xc (winner returns to ACTION),
 * `_DisplayAlibi` writes `_LastScreen` (loser snaps back), and
 * `_DoSiteLoop` writes 1/3/4 plus 0xffff on ESC.
 */
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

	// Disable ScummVM's periodic autosave. The original engine's
	// `screen8_handler @ 1c33:1012` builds the profile picker by
	// walking every `*.PLR` file in the save dir, and we mirror that
	// via `listProfiles() → MetaEngine::listSaves`. Letting the
	// framework write a slot-0 autosave creates a phantom profile
	// that shows up on the picker as a real save. Returning -1 tells
	// the framework to skip autosave entirely.
	int getAutosaveSlot() const override { return -1; }

	// ScummVM extended-save hooks. The base `Engine::saveGameState` /
	// `loadGameState` write/read the framework header (description,
	// thumbnail, playtime, version) around our body via these
	// streams. We keep all per-profile state in the body, with a
	// single `Common::Serializer` version so future field additions
	// stay backward-compatible.
	Common::Error saveGameStream(Common::WriteStream *stream,
								  bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	// Per-profile save helpers. The original `_PlayerRecord` lives at
	// `2d5d:3f6a` (159 bytes) and is written by `_SavePlayerRecord @
	// 1c33:034f` to `C:\EEMCDSAV\<name>.PLR`. The DOS launcher screen
	// `screen8_handler @ 1c33:1012` walks `*.PLR`, lets the player
	// pick a profile, and calls `_LoadPlayerRecord`. We mirror the
	// pattern by mapping each ScummVM save slot to one profile (slot
	// description = player name) — same approach Wetlands uses.

	/// Mirrors `_SavePlayerRecord @ 1c33:034f`. Saves into the slot
	/// whose description matches @p name, or the lowest unused slot
	/// if no match. Returns the kNoError on success.
	Common::Error saveProfile(const Common::String &name);

	/// Mirrors `_LoadPlayerRecord @ 1c33:03a6`. Returns false if no
	/// slot has @p name as its description.
	bool loadProfile(const Common::String &name);

	/// Mirrors the `_findfirst("*.PLR")` walk inside
	/// `screen8_handler`. Sorted by slot.
	SaveStateList listProfiles() const;

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

	/// Show the accuse-notes screen (PIC 0x1A7, the red "accuse-mode"
	/// BG with selectable clue list + "N clues" remaining counter).
	/// Mirrors the outer loop of `_DoAccuse @ 1df2:0bdd`. Returns
	/// true if the player committed (selected the chain-required
	/// number of clues and clicked SOLVE), false if they exited via
	/// ESC / back. Called from `doAccuse` before the evidence gate.
	bool doAccuseNotes();

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

	/// Re-render helpers used by the corresponding `doX()` modal screens.
	/// Each replaces what would otherwise be a `[&]()` capture-everything
	/// lambda inside the `doX()` body; called from the screen's redraw
	/// triggers (input changes, frame ticks). The state these need is
	/// passed via reference parameters or read off engine members.
	void drawNotebookFrame(int &page);
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

	/// Stop every active audio channel — voice, sound spool, and
	/// MIDI. Mirrors the `_CleanMysterySounds @ 202f:05a5` +
	/// `_StopMIDI @ 20a2:0512` pair the original triggers when the
	/// player aborts the opening-anim chain or dismisses the title
	/// (`_DoOpeningAnims @ 2520:082a` writes `_LoopMIDI = 0;
	/// _StopMIDI();` after the title-input loop, and
	/// `_CleanMysterySounds` is called twice — once after the loop
	/// and once before TITLE.ANM). Called from every ESC handler in
	/// the intro / title chain so the theme music + voice spool
	/// don't bleed past the abort.
	void interruptAudio();

	// Screen handlers — port targets in screens/ later.
	void showEAKidsLogo();
	void showHighScoreLogo();

	/// Profile selector — mirrors `screen8_handler @ 1c33:1012`.
	/// Walks `listProfiles()`, draws the list of existing profile
	/// names plus a "[New Player]" entry, and either calls
	/// `loadProfile(name)` on a click or falls through to
	/// `doNewPlayer()` if the user picks "New". When no profiles
	/// exist, behaves identically to `doNewPlayer()` (the original
	/// also bypasses the picker when `local_20 == 0` — see
	/// 1c33:1170: `if (saves == 0) _NewPlayer();`).
	void doProfilePicker();
	void doNewPlayer();          ///< Mirrors `_NewPlayer` @ 1c33:0dda
	void doChoosePartner();

	/// Display the per-mystery ending pages from `E<num>.BIN`.
	/// Mirrors `_DisplayEnding @ 1df2:0548` + `_DisplayEndingPage
	/// @ 1df2:044c`. The file format is a 2-byte page count followed
	/// by N pages, each `{ u16 picNum, u16 x1, u16 y1, u16 x2, u16 y2,
	/// char text[] (null-terminated, ParseString placeholders) }`.
	/// Blocks until the player clicks past the last page or hits ESC.
	/// `_ShowOneScrap @ 1f78:0773` is just `_DisplayEnding(num, 1)`,
	/// so this same call covers the post-mystery scrapbook view from
	/// the action menu.
	void doShowEnding(uint num);

	/// Walk every solved mystery in tier @p stage (1=Junior, 2=Senior,
	/// 3=Master) and display each one's ending pages in sequence.
	/// Mirrors `_ShowScrapbook(stage, 0) @ 1f78:0642`: the original
	/// computes the mystery range from `(stage - 1) * 0x18 + 1` to
	/// `(stage - 1) * 0x18 + 0x18` and skips entries whose
	/// `_3f9b[i] == 0` (unsolved) so the scrapbook only contains
	/// completed cases. Used by both the setup-screen ScrapBook
	/// buttons and the action-menu "See ScrapBook 1/2/3" entries.
	void doShowScrapbook(uint stage);

	void doCaseSelection();
	void doSiteLoop();

	/// Setup / preferences screen. Mirrors `_DoSetup @ 1f78:044e` —
	/// per-profile preferences (voice on/off via `DAT_2d5d_3f97`,
	/// partner pick via SwapColors on Kid1/Kid2 rects). Reachable
	/// from BigMap's setup button (sets `_NextScreen = 6` per
	/// `_DoBigMap @ 20fe:0c33`). Returns to whatever
	/// `_lastScreen` was — typically MAP.
	void doSetup();

	/// Render the case briefing background + game/book decorations and
	/// display the briefing ClueBlock. Mirrors `_DoInitClues` @ 1a35:0411
	/// minus the live ANI sequence playback.
	void doInitClues();

public:
	/// Mirrors `_StartTravelMusic @ 20a2:0595`. Picks `MUS%05d.XMI`
	/// based on `_mystery._siteNumber % 5` and starts it (looping). The
	/// site loop calls this each time `enter(siteNum)` runs so the
	/// music changes as the player travels between sites.
	void startTravelMusic();

	/// Stop any currently playing MIDI track. Mirrors `_StopMIDI @
	/// 20a2:0512` — used by `_DoSiteLoop @ 168d:06c0` after the
	/// one-shot travel track plays out and by `_DisplayCorrect /
	/// _DisplayAlibi` between MIDI cues.
	void stopMusic();

	/// Forwarded from `Engine::syncSoundSettings`. Re-pulls the user's
	/// `music_volume` slider into the MIDI player's `_masterVolume`,
	/// otherwise the AdLib output stays at whatever the slider was at
	/// the moment `_music` was constructed (and the live launcher
	/// changes to the volume slider have no effect).
	void syncSoundSettings() override;
private:

	Common::String _playerName;  ///< Substituted into 0x80 placeholders

	/// Per-mystery solved state. 0 = unsolved, 1 = solved, 2 = solved
	/// on first try. Mirrors `_PlayerRecord.SolvedMysteries[55]` in the
	/// original `_DisplayCorrect` flow.
	uint8 _mysteriesSolved[55] = {};

	/// Current chain/tier the player is at — mirrors `DAT_2d5d_3f99`
	/// (`_PlayerRecord +0x2f`):
	///   1 = Junior detective  (mysteries  1 .. 24, "A chain")
	///   2 = Senior detective  (mysteries 25 .. 48, "B chain")
	///   3 = Master detective  (mysteries 49 .. 54, "C chain")
	/// Initialized to 1 in `_NewPlayer @ 1c33:0fa3` and bumped by
	/// `_DisplayCorrect @ 1df2:0853` once every mystery in the current
	/// tier is solved (range checks at 1df2:080d / 0824 / 0837). The
	/// value also gates `_CaseSelection`'s book label and selection
	/// list (1c33:0a87 onwards).
	uint8 _chainStage = 1;

	/// Voice / digital-audio enable flag. Mirrors `DAT_2d5d_3f97`
	/// (`_PlayerRecord +0x2d`). Set to 1 by `_NewPlayer @ 1c33:0fa3`,
	/// toggled by the SoundOn / SoundOff hot-rects in `_DoSetup @
	/// 1f78:044e` (verified at `_SetupSettings` 1f78:0076 reading
	/// the same byte to colour the on/off labels). Gates every
	/// `_PlayVoice` and `_SpoolSound` call site (clue voices,
	/// partner speech, intro VO etc. — see `_DoChoosePartner`,
	/// `_DisplayClue`, `_SayKDDigital` xrefs).
	bool _voiceOn = true;

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

	/// XMIDI music player. Mirrors the original `MIDI.C` family
	/// (`_MIDIPlayFile`, `_MIDIPlay`, `_StopMIDI`, `_StartTravelMusic`
	/// at 20a2:00e2-05c9). Constructed lazily during `run()` once the
	/// MIDI driver / timer system is up.
	MusicPlayer *_music = nullptr;

	/// Digitised audio (voice + SFX). Mirrors `SOUND.C` / `SPOOLSND.C`
	/// — VOC playback (`_PlayVoice @ 1ff1:023e`) and the per-mystery
	/// SDB spool (`_SpoolSound @ 202f:068d` / `_InitMysterySounds @
	/// 202f:05cb`). Constructed alongside `_music` in `run()`.
public:
	AudioPlayer *_audio = nullptr;

	/// Public setter for `_nextScreen` so site loop / inline screens
	/// can drive the screen-driver state machine without making the
	/// member itself public. Mirrors the original's direct write to
	/// `_NextScreen @ 2d5d:3f26` from anywhere in the engine.
	void setNextScreen(ScreenId s) { _nextScreen = s; }
};

} // End of namespace EEM

#endif
