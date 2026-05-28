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

#ifndef CRYOMNI3D_ATLANTIS_ENGINE_H
#define CRYOMNI3D_ATLANTIS_ENGINE_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/stream.h"

#include "cryomni3d/cryomni3d.h"
#include "cryomni3d/omni3d.h"

#include "graphics/surface.h"

#include "cryomni3d/atlantis/bigfile.h"
#include "cryomni3d/atlantis/con_script.h"
#include "cryomni3d/atlantis/dialogs_manager.h"
#include "cryomni3d/atlantis/menu_layout.h"
#include "cryomni3d/atlantis/sprite_blend.h"
#include "cryomni3d/atlantis/toolbar.h"
#include "cryomni3d/atlantis/wam_parser.h"

namespace CryOmni3D {
struct FixedImageConfiguration;
class ZonFixedImage;
}

namespace CryOmni3D {
namespace Atlantis {

enum AbortCommand {
	kAbortNoAbort   = 0,
	kAbortQuit      = 1,
	kAbortLoadGame  = 2,
	kAbortNewGame   = 3,
	kAbortNextChapter = 5,
	kAbortFinished  = 6,
	kAbortGameOver  = 7
};

// File type enum mapping Atlantis's BigFile directory layout.
// All files are read from the BigFile archive; these constants describe
// which subdirectory prefix to prepend.
enum FileType {
	kFileTypeCyclo,        // CYCLO\        — panoramic HNM videos
	kFileTypeTransition,   // UBB_VUE\      — place-to-place transition HNM videos
	kFileTypeDialog,       // DIALOG\       — dialog UBB audio + font + init files
	kFileTypeSync,         // SYC\          — lip-sync timing data
	kFileTypeFont,         // FONTS\        (or game root) — font files
	kFileTypeSprite,       // SPRLIST\      — sprite list text files
	kFileTypeWAM,          // WAM\          — navigation map files
	kFileTypeObject,       // OBJETS\       — object images
	kFileTypeMenu,         // MENU\         — menu/UI images
	kFileTypeScript,       // SCENAR\       — chapter scenario scripts (.CON files)
	kFileTypeSound,        // WAV\          — APC audio files (music, ambient, dialog)
	kFileTypeSpriteCyclo,  // SPRITE\CYCLO\ — pre-rendered NPC/prop sprites (SPW files)
	kFileTypeSpriteUbb,    // SPRITE\UBB\   — per-transition NPC animations (SPF files)
	kFileTypeSprite2D,     // SPRITE\2D\    — 2D subject/cursor icon sprites (SPR files)
	kFileTypeImages,       // IMAGES\       — full-screen TGA background images
	kFileTypePuzzles       // PUZZLES\      — per-minigame sprites + TGAs (eclipse, chess, …)
};

struct PlaceState {
	typedef void (CryOmni3DEngine_Atlantis::*InitFunc)();
	typedef bool (CryOmni3DEngine_Atlantis::*FilterEventFunc)(uint *event);

	PlaceState() : initPlace(nullptr), filterEvent(nullptr), state(0) {}
	PlaceState(InitFunc init, FilterEventFunc filter) :
		initPlace(init), filterEvent(filter), state(0) {}

	InitFunc initPlace;
	FilterEventFunc filterEvent;
	uint state;
};

struct LevelInitialState {
	uint placeId;
	float alpha;
	float beta;
};

// One entry from the WAM's SPF transition-sprite list (e.g. SPFATL1.TXT).
// Format per line: `fromPlace toPlace persoId spfFile`.  During a transition
// from fromPlace to toPlace the engine composites the matching SPF's frames
// onto the HNM transition video so NPCs remain visible as the camera moves.
// SPF on-disk format is identical to SPW (same header, frame table, RLE
// pixel stream), just authored against the 640×480 video frame instead of
// the 2048-wide cyclo panorama.
struct SpriteTransEntry {
	uint16 fromPlace;
	uint16 toPlace;
	int    persoId;
	Common::String spfFile;
};

// One entry from the WAM's SPW sprite list (e.g. SPWATL1.TXT).
// NPC entries have type==0 and persoId>0; animated props (torches) have type!=0.
struct SpritePlaceEntry {
	uint16 placeId;
	int type;     // 0 = NPC, non-zero = animated prop (torch, etc.)
	int frames;   // animation frame count (0 for static NPC)
	int persoId;  // character ID (0 for props)
	float angle;  // camera look angle for NPC orientation
	Common::String spwFile;
	// True when this entry marks the NPC's "home" place — the only place
	// where the player can interact (talk) with them.  In the SPW list this
	// is encoded as a leading minus sign on the angle field (e.g. `-00`
	// vs. plain `00`); from every other listed place the NPC sprite is
	// visible at a distance but not clickable.
	bool interactive;
	// Initial "active" state encoded by trailing '-' on the placeId token.
	// "180" (no dash) -> startActive=true ; "180-" (dash) -> startActive=false.
	// startActive entries (torches, ambient props) render on place load.
	// Dashed entries (NPCs, alternate wspranim poses) require an explicit
	// /set(NewWsprAnim=N) — 1-based index into the place's sprite array —
	// or a /set(ShowPerso=N) to flip `active` true before they render.
	// Mirrors atlantis.exe sprite-record field +0x04 (rendered iff non-zero).
	bool startActive;
	// Runtime active flag.  Reset to startActive on place load; flipped by
	// ShowPerso/HidePerso/NewWsprAnim/StopWsprAnim.
	mutable bool active;
	// Current animation frame for this sprite.  Type-2 wspranim sprites use
	// their own per-sprite frame counter; the script's WSPRFRAME variable
	// reflects the active sprite's frame each tick.
	mutable int frame;
};

class CryOmni3DEngine_Atlantis : public CryOmni3DEngine {
	friend class Atlantis_DialogsManager;
	friend class Atlantis_Toolbar;

protected:
	Common::Error run() override;

public:
	CryOmni3DEngine_Atlantis(OSystem *syst, const CryOmni3DGameDescription *gamedesc);
	~CryOmni3DEngine_Atlantis() override;

	bool hasFeature(EngineFeature f) const override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::String getSaveStateName(int slot) const override;
	// Atlantis autosaves on chapter transitions (see autosave()), never on a
	// timer — disable ScummVM's periodic autosave so it cannot write a slot.
	int getAutosaveSlot() const override { return -1; }

	// Whether dialog text should be drawn on screen.  Reads ScummVM's
	// "subtitles" config key, which is wired to the standard ScummVM GUI
	// toggle and to the F-key shortcut.  Engine advertises
	// kSupportsSubtitleOptions via the cryomni3d base class so the option
	// is exposed in the launcher and in-game GMM.
	bool showSubtitles() const;

	// Build a path for a named file of the given type (for files outside the BigFile).
	Common::Path getFilePath(FileType fileType, const Common::String &baseName) const;

	// Open a file from the BigFile archive and return a new stream (caller owns it).
	// Returns nullptr if not found.
	Common::SeekableReadStream *openBigFileStream(FileType fileType,
	                                               const Common::String &baseName) const;

	void setupPalette(const byte *colors, uint start, uint num) override;
	void makeTranslucent(Graphics::Surface &dst, const Graphics::Surface &src) const override;

	bool displayToolbar(const Graphics::Surface *original) override { return _toolbar.displayToolbar(original); }
	bool hasPlaceDocumentation() override { return false; }
	bool displayPlaceDocumentation() override { return false; }
	uint displayOptions() override;
	bool shouldAbort() override;
	void drawInventoryIcon(Graphics::ManagedSurface &dst, const Object *obj,
	                       const Common::Rect &r) override;

	// Draw the "object just received" icon centred horizontally near the
	// bottom of `dst` while a give is pending (_dialogGivenObjSprite >= 0).
	// Mirrors the original's DAT_0049619c per-dialog-frame draw.
	void drawGivenObject(Graphics::Surface &dst) const;

private:
	void setupFonts();
	void setupObjects();
	void loadStaticData();

	// Wire up place-specific initPlace/filterEvent callbacks for a chapter.
	void initChapterPlaceStates(int chapter);

	void syncSoundSettings() override;
	void calculateTransparentMapping();

	// Load the panoramic HNM for a place and set it as the Omni3D source.
	// Returns true if successful.
	bool loadCycloHNM(const Common::String &name);

	void changeChapter(int chapter);
	void initNewChapter(int chapter);
	void setupChapterWAM(int chapter);
	void setupWAMByName(const Common::String &wamName);
	void initPlacesStates();

	void gameStep();
	void doPlaceChange();
	void executeTransition(uint nextPlaceId);

	int handleWarp();

	void drawMenuTitle(Graphics::ManagedSurface *surface, byte color);
	uint displayFilePicker(const Graphics::Surface *bgFrame, bool saveMode,
	                       Common::String &saveName);

	bool loadGame(uint saveNum);
	void saveGame(uint saveNum, const Common::String &saveName);

	// Parse one of the original SPRLIST\*MENU.TXT files (relative basename
	// only, e.g. "MAINMENU.TXT") into the supplied MenuLayout.  Returns
	// false only on missing file / unreadable stream — an empty layout is
	// reported as success.
	bool loadMenuLayout(const char *baseName, MenuLayout &out) const;

	// Render every item of `layout` onto `dst` and fill `hitRectsOut` with
	// the bounding box of each text item (in the same order as
	// `layout.items`).  Sprite items get a Rect() (empty).
	void renderMenuLayout(Graphics::ManagedSurface &dst, const MenuLayout &layout,
	                      int hoveredTextIdx,
	                      Common::Array<Common::Rect> &hitRectsOut) const;

	// In-game (Escape) menu.  Returns:
	//   0  — continue (resume warp)
	//   28 — load game (caller must consume _loadedSave)
	//   27 — new game
	//   40 — quit
	uint displayInGameMenu();

	// Quit confirmation dialog.  Composites SPRLIST\SUREMENU.TXT (the
	// "ARE YOU SURE ?" prompt with its OK / Cancel buttons and ornament
	// sprites) over `bg`.  All text and positions come from that file, so
	// localized discs work unchanged.  Returns true when the player
	// confirms, false on cancel or Escape.
	bool confirmQuit(const Graphics::ManagedSurface &bg);

	// Options menu (SPRLIST\OPTMENU.TXT): the SUBTITLES and OMNI 3D toggles
	// plus a SOUND OPTIONS sub-menu button.  Modal; returns when OK is
	// clicked or Escape is pressed.
	void displayOptionsMenu(const Graphics::ManagedSurface &bg);

	// Sound-options sub-menu (SPRLIST\SONMENU.TXT): four volume sliders.
	// OK commits the volumes to the ScummVM config, Cancel discards them.
	void displaySoundMenu(const Graphics::ManagedSurface &bg);

	// Play WAV\<apcName> once with a stereo balance (-127 = full left ...
	// +127 = full right) — the sound menu's Left/Right speaker test.
	void playPannedSound(const char *apcName, int8 balance);

	// True when the player selected OMNI 3D "MODE 1" — that mode inverts
	// the vertical (up/down) panorama look control in handleWarp().
	bool omni3dInvertY() const;

	// --- Player profiles (original-game player management) ----------------
	// Up to kMaxPlayers named players, each owning the absolute save-slot
	// block [id*kPlayerSlotStride, id*kPlayerSlotStride + kPlayerSlotStride).
	// The id is the fixed array index, so a player's saves stay associated
	// when other players are created or deleted.  Within a block the local
	// slot index IS the chapter number: a player keeps one named save per
	// episode (see autosave()), so chapter N's save lives at local slot N.
	static const uint kMaxPlayers       = 5;
	static const uint kPlayerSlotStride = 100;
	static const uint kPlayerNameLen    = 16;

	Common::String _players[kMaxPlayers];  // empty string = unused slot
	int            _currentPlayer;
	// Per-player resume pointer: the chapter the player is currently in, so a
	// resume reloads that episode's save instead of guessing.  0 = no save.
	// Persisted in atlantis-players.dat (format version 2).
	int            _playerChapter[kMaxPlayers];

	void loadPlayers();
	void savePlayers();
	void deletePlayerSaves(uint player);
	// Select / create / delete screen.  Returns the chosen player id, or -1.
	int  displayPlayerScreen();

	// Native save-slot picker laid out from SPRLIST\SELEMENU.TXT.  Lists
	// every kept per-episode save of the current player by its EPI.TXT
	// checkpoint name.  Returns the absolute slot number (1-based), or 0 on
	// cancel.
	uint displaySavePicker(bool saveMode, Common::String &outSaveName);

	// --- Per-episode saves (original-game behaviour) -----------------------
	// Atlantis has no manual save: the game keeps one named save per episode
	// and rewrites it whenever that episode is (re-)entered.  A CON
	// `chapter=N` transition autosaves into the new chapter's slot, named
	// from SPRLIST\EPI.TXT (mirrors atlantis.exe FUN_00427634 -> FUN_0042d6b8).
	// Absolute (0-based) save slot a chapter occupies in a player's block.
	int  episodeSaveSlot(uint player, uint chapter) const {
		return (int)(player * kPlayerSlotStride + chapter);
	}
	// True if an episode save file exists at the given absolute slot.
	bool saveSlotExists(int absSlot) const;
	// True if `player` has any episode save to resume.
	bool playerHasSave(uint player) const;
	// The save `player` should resume into — their tracked current chapter's
	// slot.  Returns the absolute (1-based) save number, or 0 when none.
	uint playerResumeSave(uint player) const;
	void autosave();
	// Set when a CON `chapter=N` command advances the chapter; consumed by
	// doPlaceChange() so the autosave is written as the new chapter's first
	// place is entered (mirrors atlantis.exe FUN_00427634 -> FUN_0042d6b8).
	bool _autosavePending;

	// Checkpoint name for `chapter` — EPI.TXT line (chapter - 1).
	Common::String episodeName(uint chapter) const;
	// Episode checkpoint names parsed once from SPRLIST\EPI.TXT at startup.
	// Index i is the name shown for a save made on entering chapter i + 1.
	void loadEpisodeNames();
	Common::Array<Common::String> _episodeNames;

	// Play the transition HNM video.  When fromPlace and toPlace are both
	// supplied (non-uint(-1)), the renderer composites SPF transition
	// sprites onto each video frame so NPCs remain visible as the camera
	// moves between the two places.
	void playTransitionVideo(const Common::String &videoName,
	                         uint fromPlace = uint(-1),
	                         uint toPlace   = uint(-1));

	// QUIT GAME outro: the original game's end-card slideshow.  Shows
	// IMAGES\END.TGA followed by IMAGES\CREDIT01..10.TGA with
	// WAV\04GENERI.APC playing throughout.  Skippable on any key or
	// mouse click.  Invoked from displayInGameMenu()'s kQuit branch
	// before returning the kAbortQuit code to handleWarp().
	void playQuitOutro();

	// gameover=N (CON command) — the game-over screen (GereGameOver,
	// atlantis.exe FUN_00426590).  Shows the full-screen image
	// IMAGES\GOVER<NN>.TGA, plays the narrator's commentary over it
	// (WAV\ATA<section4d><track>.APC, the first [Narratrice] line that
	// follows the /set(gameover=N) command in the section) with its
	// subtitle, waits for the voice to finish (skippable; ~5 s cap when no
	// audio), then returns.  showGameOver() itself only presents the screen;
	// playConSection() then reloads the chapter checkpoint (a game-over is
	// gated on a persistent CON variable, so the only escape is a reload).
	//   sectionId/narratorTrack — together name the ATA APC file.
	//   narratorText            — the narrator subtitle (empty = none).
	void showGameOver(int sectionId, int n, const Common::String &narratorText,
	                  char narratorTrack);

	// Parse the WAM's SPW sprite list (e.g. SPWATL1.TXT) into _spritePlaceList.
	void loadSpritePlaceList();
	// Parse the WAM's SPF transition-sprite list (e.g. SPFATL1.TXT) into
	// _spriteTransList.  Run once per chapter alongside loadSpritePlaceList.
	void loadSpfList();
	// Composite NPC/prop sprites for the current place onto _warpSurface.
	// Also populates _npcPlaceBounds for hit testing.
	void compositeNPCSprites();
	// Rebuild only the NPC click-target list (_npcPlaceBounds) for the
	// current place, honouring _hiddenPersos.  Safe to call mid-place after
	// a showperso/hideperso change — unlike compositeNPCSprites() it leaves
	// prop-/wspr-animation state untouched.
	void rebuildNpcPlaceBounds();

	// Load and parse the chapter CON script (CHAPI<N>.CON) into _conScript.
	// Processes /INIT commands into _sujEnabled.
	void loadConScript(int chapter);
	// True for the 7 VARIAS.CON variables (indices 0-6) that persist across a
	// chapter change and are written to the save; all others are transient.
	static bool isPersistentGameVar(const Common::String &key);

	// Process all INIT-section /set(...) commands (enableSuj, hidePerso, etc.).
	void execConInitCommands();

	// Execute a single /set(...) command string (content inside the parentheses).
	void execConSetCommand(const Common::String &cmd);

	// Fade the screen to black over a few frames (the /set(fadeout) command).
	void fadeScreenToBlack();

	// Launch the minigame indexed by puzzleId (the /set(StartPuzzle=N)
	// command).  On exit writes back _gameVars["finpuzzle"] — 255 on
	// success, other codes per-puzzle (e.g. CHAPI018's chess uses 1/2
	// for distinct loss-line variants).  Phase-1 stub: every call
	// immediately resolves to FinPuzzle=255 so script chains continue;
	// per-puzzle UIs land in follow-up commits.
	void playPuzzle(int puzzleId);

	// NPC interaction: present enabled dialog subjects for persoId, play chosen sections.
	void interactNPC(int persoId);

	// Show the subject-selection menu for the given options (one ConSection* per choice).
	// Returns the chosen index, or -1 to exit conversation.
	int showSubjectMenu(const Common::Array<ConSection *> &options);

	// Play all dialog lines of a section (audio + subtitles), then execute its /set commands.
	void playConSection(ConSection &sec);

	// Play one APC audio track for a section, displaying the given subtitle text.
	// If videoName is non-empty, that HNM from DIALOG\ is looped as background video.
	//
	// `hasAngle`/`angleX`/`angleY` come from the CON dialog directive's optional
	// numeric tokens (`[Speaker, X, Y, camN]` — see ConLine).  When supplied,
	// the renderer rotates the panorama view to (X, Y) in panorama-pixel space
	// before starting the line, so the cyclo background under the talking head
	// faces the authored direction.  Conversion to radians uses the live
	// panorama dimensions (`_warpSurface.w/h`); no hardcoded scale.
	//
	// `camNum` (2..5) selects the per-cam zoom level — the renderer narrows
	// the panorama FOV using the table extracted from atlantis.exe at VA
	// 0x00496080.  This is what visibly changes the view between dialog
	// subjects in the original game.  The caller (playConSection) picks the
	// cam via pickDialCam() so callers should always pass a valid 2..5.
	// `spfBaseName` (basename only, no extension) selects the per-cam SPF overlay
	// composited frame-by-frame onto the UBB talking head — INITDIAL columns 10..13
	// per persoId, resolved by playConSection() from `_dialInitData[persoId].camSPF`.
	// Empty string disables the overlay (matches the original "none" sentinel).
	// `animMBase` (basename only) selects the per-character mouth vertex animation
	// at DIALOG\PERSO\<base>.3DA (INITDIAL column 5).  When loaded, the parsed F3DC
	// frame vertices are projected and splatted into the UBB's chroma-key mouth
	// hole each tick as a debug point cloud — placeholder for the future scene-
	// graph triangle rasteriser.  Empty disables the cloud.
	// `s3dBase` (basename only) selects the per-character scene-graph mesh at
	// DIALOG\PERSO\<base>.S3D (INITDIAL column 6).  Currently only the header is
	// parsed for verification; triangle extraction is the next implementation pass.
	void playSingleConLine(int sectionId, char track, const Common::String &text,
	                       const Common::String &videoName = "",
	                       bool hasAngle = false, int angleX = 0, int angleY = 0,
	                       int camNum = 2,
	                       const Common::String &spfBaseName = "",
	                       const Common::String &animMBase = "",
	                       const Common::String &s3dBase = "",
	                       const Common::String &anim3DCBase = "",
	                       bool interactiveDialog = true);

	// Returns true if subject sujId is in the enabled list for persoId.
	bool isSujEnabled(int persoId, int sujId) const;

	// Return (creating if absent) the enabled-subjects list for persoId.
	Common::Array<int> &sujsForPerso(int persoId);

	// Evaluate all conditions of a /sel section.
	// departZone: WAM zone the player is trying to leave via (0 for arrival checks).
	// currentVue: the place ID currently active.
	bool evalSelSection(const ConSection &sec, int departZone, int currentVue) const;

	// Run /sel sections whose departvue condition matches departZone.
	// Sets _nextPlaceId = uint(-1) if abortdepart=1 fires; may redirect via vue=N.
	void runDepartureSelSections(int departZone);

	// Run /con* sections whose cliczone condition matches zoneId (item-use on a zone).
	// Temporarily sets _gameVars["cliczone"] so evalSelSection can check it.
	void runZoneConSections(int zoneId);

	// Run /sel sections whose arriveevue condition matches (ArriveeVue>0)&(vue=arrivedVue).
	// Marks each fired section as played to prevent re-triggering.
	void runArrivalSelSections(int arrivedVue, int fromVue);

	// Generic /con scan -- mirror of atlantis.exe FUN_00425cf4.  The original
	// engine runs this on every left-click whose dispatched action falls in
	// the object id range [256, 512): it iterates every /Con or /con section
	// (skipping /Suj-prefixed topic gates) and fires the FIRST whose
	// conditions evaluate true via the generic eval path -- exactly the way
	// /sel sections are dispatched on arrival/departure, just keyed off
	// general game state instead of (arriveevue, departvue).
	//
	// Used for "use held object in the world at a specific state" scripts
	// that have no ClicPerso / ClicZone gate.  Canonical case: CHAPI013
	// section 178 + the three /014 fallbacks ("throw the flower pot on the
	// priest aide") -- the player holds object 261, aims the camera within a
	// narrow (AngleWarpX, AngleWarpY) window, and clicks while wsprframe is
	// in the right pose window.  Returns true if a section fired so the
	// caller can consume the click (no NPC dialog / pickup / zone navigation
	// after a hit).
	bool runGenericConSections();
	// Debug: log the full story state (place, CON chapter, music track and
	// every game variable) so a progression problem can be diagnosed from
	// the log against the CON section conditions.
	void debugDumpStoryState(const char *ctx);

	// Run all /tim<timerNum> sections whose conditions currently match.
	// Timer sections are not marked played — they re-evaluate on every tick.
	void runTimerSections(int timerNum);

	// Hit-test panoramic point against cached NPC bounding boxes.
	// If a match is found, sets outPersoId and returns true.
	bool npcPanoHitTest(const Common::Point &pan, int &outPersoId) const;

	// Place-specific filterEvent callbacks (logic.cpp).
	// Each returns true if the event should proceed, false to block it.
	// *event may be modified to redirect the action.
	// (currently none — guard logic at place 17 is CON-script driven)

	// Play a CON-script dialog: display subtitle text while playing
	// sequential APC tracks (ATA<dialogId4>A.APC, B.APC, ...).
	// dialogId is the CON section number, which doubles as the 4-digit audio ID.
	// If bgVideoName is non-empty, that HNM is looped as background video during playback.
	void playConDialog(int dialogId, const Common::String &text,
	                   const Common::String &bgVideoName = "");

	// Music is fully CON-driven: startmusik selects a looping track, stopmusik
	// clears it.  musicUpdate() keeps the mixer in sync with that selection
	// every frame, including across the mute state.
	void musicUpdate();
	void musicStop();
	// Select and (re)play a numbered music track (1-based, matching the
	// startmusik CON command IDs).  The track loops until changed.
	void musicPlayTrack(int trackId);
	// Stop mixer playback and free the decoded buffer without clearing the
	// selected track — used for the mute state and before a track switch.
	void musicHaltPlayback();

	static const LevelInitialState kChapterInitialStates[];

	Atlantis_Toolbar _toolbar;
	Atlantis_DialogsManager _dialogsMan;

	BigFileArchive _bigFile;
	AtlantisWAMParser _wam;

	Omni3DManager _omni3dMan;
	ZonFixedImage *_fixedImage;

	byte *_mainPalette;
	byte *_transparentPaletteMap;
	uint _transparentSrcStart;
	uint _transparentSrcStop;
	uint _transparentDstStart;
	uint _transparentDstStop;
	uint _transparentNewStart;
	uint _transparentNewStop;

	bool _isPlaying;
	AbortCommand _abortCommand;
	uint _loadedSave;

	uint _currentChapter;
	uint _currentPlaceId;
	uint _nextPlaceId;
	const AtlantisPlace *_currentPlace;

	// Decoded panorama for the current place (owned by this object).
	// _cycloSurface holds the clean cyclo pixels; _warpSurface is the composited display copy.
	Graphics::Surface _cycloSurface;
	Graphics::Surface _warpSurface;
	bool _warpLoaded;

	// Animated prop sprite state for the current place (torches etc, type=1).
	int   _propAnimFrames;  // total frame count (0 = no animated prop at current place)
	uint  _propAnimFrame;   // current animation frame index
	uint32 _propAnimNextMs; // g_system->getMillis() threshold for next frame advance
	static const uint32 kPropAnimIntervalMs = 80;  // ~12.5 fps

	// Active wspranim sprite index for the CURRENT place.  /set(NewWsprAnim=N)
	// sets this to N-1 (1-based index in the place's sprite array) and
	// activates that sprite; /set(StopWsprAnim=N) clears the active sprite.
	// Driven from handleWarp's main loop, NOT from time1/time2 — wsprframe
	// is an animation-frame counter, not a one-per-second tick.  Each
	// frame advance re-runs the /Tim2 scan with the new wsprframe in
	// scope, mirroring atlantis.exe FUN_00425304's timer-2 gate hijack
	// (line 20086 in the decompile) that lets `/Tim2(WSprFrame=N)`
	// conditions match against the live sprite frame.
	int    _wsprAnimActiveIdx;     // place-local sprite index; -1 = none active
	uint32 _wsprAnimNextMs;        // next allowed frame-advance timestamp
	// Per-frame interval.  Exe uses a 13-ms threshold (~75 fps wspranim
	// gated by the render loop's actual rate); 30 ms gives ~33 fps
	// wspranim, so CHAPI012's 33-frame guard walk finishes in ~1 s —
	// matching how the original played on period hardware where the
	// main loop itself, not the threshold, was the cadence bottleneck.
	static const uint32 kWsprAnimIntervalMs = 30;

	// Deferred-refresh flags for CON commands that change the panorama sprite
	// layer while it is not on screen (e.g. mid-dialog).  handleWarp()
	// consumes them on its next frame so the change shows without a place
	// change.  _spriteLayerDirty triggers a recomposite + redraw (set by
	// showperso/hideperso and newwspranim/stopwspranim); _npcBoundsDirty
	// additionally rebuilds the NPC click-target list (set only by
	// showperso/hideperso, the only commands that alter it).
	bool _spriteLayerDirty;
	bool _npcBoundsDirty;

	// Re-composite all sprites for the current place onto _warpSurface from _cycloSurface.
	// Called both at place entry and during animation ticks.
	void recompositeSpriteLayer();

	Common::Array<uint> _gameVariables;
	Common::Array<PlaceState> _placeStates;
	// Per-place visit counter (NbVisite), indexed by place id; survives
	// chapter changes.  /set(disable=N) bumps entry N to suppress place N's
	// first-visit /sel sections.  Mirrors atlantis.exe's 0x6cc6cc array.
	Common::Array<byte> _placeVisits;

	// Named game variables: set by /set(key=val) and evaluated in /sel conditions.
	Common::HashMap<Common::String, int> _gameVars;

	// Per-character dialog resource table parsed from DIALOG\INITDIAL.TXT.
	// camUBB[0..3] = UBB video basenames for camera angles 2..5 ("none" or empty = unavailable).
	// camSPF[0..3] = optional per-cam decorative SPF overlay basenames composited each frame
	//               onto the talking-head UBB, sourced from INITDIAL columns 10..13.
	//               These are the Sprite-Per-Frame files the original engine pulls from
	//               SPRITE\UBB\<base>.SPF (FUN_0042f9e8 in atlantis.exe.c:26543-26615
	//               opens them via FUN_00434814 and composites them inside the UBB
	//               playback loop on the same frame counter as the host video).
	//               "none" or missing in the source row → empty here; skip compositing.
	// animMBase  = INITDIAL column 5 (e.g. "heroan1m" for Seth) — basename of the
	//               per-character mouth vertex-animation file at DIALOG\PERSO\<base>.3DA.
	//               This is what fills the mouth-shaped chroma-key hole in the UBB; loaded
	//               by F3dcMouthAnim in playSingleConLine.  Empty when the parser was unable
	//               to find a per-character mouth.
	// s3dBase    = INITDIAL column 6 (e.g. "hero" for Seth, "garde" for the Gardes) —
	//               basename of the per-character mesh topology at DIALOG\PERSO\<base>.S3D.
	//               This is the scene-graph file that holds the triangle indices into the
	//               .3DA vertex pool; loaded by F3dcMesh in playSingleConLine.  Currently
	//               only the header is parsed — full scene-graph walking is TBD.
	struct DialInitEntry {
		Common::String camUBB[4];
		Common::String camSPF[4];
		Common::String animMBase;
		Common::String s3dBase;
		Common::String anim3DCBase;
	};
	void loadDialInitData();
	Common::HashMap<int, DialInitEntry> _dialInitData;

	// Random camera selection per dialog line.  Mirrors atlantis.exe.c
	// FUN_0042844c lines 22460-22470: when a CON directive has no explicit
	// `camN`, the original engine picks a random cam in 2..5, rejecting the
	// previous line's choice (so consecutive lines never use the same shot)
	// and any cam whose UBB resource is missing for the speaker.
	Common::RandomSource _dialRng;
	int _lastDialCam;  // 0 = no previous line in this dialog session
	int pickDialCam(int persoId, int explicitCam);

	// Index into _objSprites (== _objects) of an object an NPC just gave Seth
	// during the current dialog, or -1 when none.  Set by /set(inventN=1) and
	// drawn centre-bottom on every dialog frame (the original's DAT_0049619c),
	// cleared when the dialog ends (interactNPC return / place change).
	int _dialogGivenObjSprite;

	// RNG for CON `random` conditions.  Each evalSelSection call queries this
	// once and stores the byte (0..255) as the LHS of any `random` comparison
	// — letting CON authors gate dialog branches probabilistically (e.g.
	// `/con(...)&(random<128) /suj5`).  Mutable so the const-marked
	// evalSelSection can advance the stream.
	mutable Common::RandomSource _conRng;

	// Panorama alpha captured at the start of an NPC interaction (the
	// direction the player was facing when they clicked the NPC).  This is
	// the baseline for the default per-line angle: NPC lines preserve it,
	// hero lines flip 180° from it.  Without anchoring on the click angle,
	// the previous line's reverse-shot would leak into the next NPC line.
	// Set true only between interactNPC entry and exit.
	double _dialBaseAlpha;
	bool   _dialBaseAlphaValid;

	// Resolve a dialog line's speaker name to a perso ID.  Transcribes the
	// speaker-name → persoId strncmp chain from atlantis.exe (VA 0x428f58):
	// a fixed table keyed by a case-insensitive prefix of the speaker name.
	// The player character ("Seth") always resolves to _heroPersoId.
	int resolveSpeakerPersoId(const Common::String &speaker) const;
	int _heroPersoId;  // persoId of the player character (detected from INITDIAL "hero" entry)

	Common::String _currentWAMName;
	Common::Array<SpritePlaceEntry> _spritePlaceList;

	// Parsed SPF transition-sprite list (e.g. SPFATL1.TXT).
	Common::Array<SpriteTransEntry> _spriteTransList;

	// Decode a single frame of an SPF transition sprite stream and composite
	// it onto `dst`.  The SPF format is identical to SPW (same header / RLE
	// pixel stream), so the same flat-buffer decoder is reused — but here
	// the surface width matches the transition video (640) rather than the
	// 2048-wide cyclo panorama.  Implemented in sprite_renderer.cpp.
	void compositeSpfFrame(Common::SeekableReadStream *spf, uint frameIdx,
	                       Graphics::Surface &dst);

	// Per-NPC subject enable state (keyed by persoId).
	struct SujState {
		int persoId;
		Common::Array<int> sujs;
	};
	Common::Array<SujState> _sujEnabled;

	// Panoramic bounding boxes for NPCs in the current place (for mouse hit testing).
	struct NPCBound {
		int persoId;
		Common::Rect panoBounds;  // in panoramic coords (Y=0 at top, exclusive right/bottom)
	};
	Common::Array<NPCBound> _npcPlaceBounds;

	// Set of persoIds hidden by /set(hideperso=N) or /set(disableperso=N).
	// Cleared on chapter change; showperso=N removes from the set.
	Common::Array<int> _hiddenPersos;

	// Set of persoIds currently following the hero (companion NPCs).
	// Added via /set(addguest=N), removed via /set(subguest=N).  At
	// compositeNPCSprites time every guest's sprite at the current place
	// (if one exists) is force-shown -- this overrides _hiddenPersos for
	// that perso while they're a guest, so a script that hid an NPC at
	// chapter init can still let them appear later as a follower without
	// an explicit /set(showperso=N).  Persists across place changes
	// within a chapter; cleared on chapter switch like _hiddenPersos.
	Common::Array<int> _guests;

	// Parsed CON script for the current chapter.
	ConScript _conScript;

	uint _currentCONChapter;  // which CHAPI00N.CON is currently loaded (0 = none)

	// Wall-clock at which the active CHAPI*.CON was loaded.  Used to compute
	// the `chaptertime` CON condition (seconds elapsed since chapter switch).
	// Reset by loadConScript on every chapter advance; read live by
	// evalSelSection so dialog gates like CHAPI030's
	// `/Con(...)&(ChapterTime>120)` fire when the time actually elapses.
	uint32 _chapterStartMs;

	// Per-timer tick timestamps: _timerTickNextMs[i] is the g_system->getMillis() threshold
	// for the next tick of timer i+1.  Timer sections increment timeN each tick when
	// testTimeN=1 is set.  Currently supports timers 1 and 2 (indices 0 and 1).
	uint32 _timerTickNextMs[2];
	static const uint32 kTimerTickIntervalMs = 1000;  // 1 tick per second

	// zoom=1 (CON) — close-up FOV ramp.  _zoomLastMs is the getMillis() of the
	// previous ramp frame (0 = not ramping); see the gameStep warp loop.
	uint32 _zoomLastMs;

	const char *_musicCurrentFile;
	Audio::SoundHandle _musicHandle;
	byte *_musicRawData;
	uint32 _musicRawSize;
	int _musicTrackId;   // selected startmusik track id (0 = stopped / none)

	// Ambient SFX playback driven by CON `/set(newsound=N)`.  The id-to-APC
	// mapping lives in atlantis.exe at VA 0x004961e0: a pointer table that
	// starts with 17 WAM names then continues with APC filenames.  newsound
	// IDs are 0-based into the APC portion, so id N maps directly to
	// _newSoundApcNames[N] (== file-table index N+17).  Verified empirically:
	// CON `newsound=14` at CHAPI001/zone 168 plays Toctoc.apc, which is the
	// 15th APC entry in the table (array index 14).  Loaded once at engine
	// start by loadAtlantisExeTables().
	Common::Array<Common::String> _newSoundApcNames;
	Audio::SoundHandle _newSoundHandle;

	// World-object-id table extracted from atlantis.exe at VA 0x004967b0:
	// a zero-terminated int32 array (~39 entries) where index i is the sprite
	// slot in SPRITE\2D\OBJETS1.SPR and the value is the world object id that
	// uses that sprite as its inventory icon.  setupObjects() builds _objects
	// from this so drawInventoryIcon() (which indexes _objSprites by
	// (obj - &_objects.front())) lands on the matching icon by construction.
	Common::Array<uint> _inventoryObjectIds;

	void loadAtlantisExeTables();

	static const FixedImageConfiguration kFixedImageConfiguration;

	// Subject icon sprites decoded from SPRITE\2D\SUJETS.SPR.
	struct SubjSprite {
		uint16 w, h;
		int16  xoff, yoff;
		Common::Array<uint16> pixels;  // RGB565; kSprTransp marks transparent pixels
		Common::Array<uint8>  blend;   // per-pixel blend factor; kSprNoBlend = opaque
	};
	static const uint16 kSprTransp = 0x0001;  // sentinel: not a real game color

	// Decode one sprite's RLE rows (atlantis.exe FUN_0041047f) into spr.pixels
	// and spr.blend.  spr.w/h must already be set; the stream must be positioned
	// at the per-row offset table.  Shared by every SPR loader.
	void decodeSprite(Common::SeekableReadStream &stream, SubjSprite &spr,
	                  const uint16 *colorTable);

	void loadSubjectSprites();
	void drawSubjectSprite(Graphics::Surface &dst, uint sprIdx, int cx, int cy) const;

	Common::Array<SubjSprite> _subjSprites;

	// Inventory item icon sprites decoded from SPRITE\2D\OBJETS1.SPR.
	// Indexed by position in _objects (sprite[i] is the icon for _objects[i]).
	void loadObjectSprites();
	Common::Array<SubjSprite> _objSprites;

	// The single 599x82 inventory-bar background sprite from
	// SPRITE\2D\OBJETS0.SPR -- thin golden filigree line at top, then 8
	// star-of-David slot ornaments and decorative drops.  This IS the
	// inventory bar in the original game; SPRMENU.SPR is used only for the
	// menu-screen frames and dialog borders, not the toolbar.  Loaded by
	// loadInventoryBarSprite().
	void loadInventoryBarSprite();
	SubjSprite _inventoryBarSprite;

	// Toolbar/menu UI sprites decoded from SPRITE\2D\SPRMENU.SPR.
	// Sprite 0 is the full-width toolbar background bar.
	void loadMenuSprites();
	Common::Array<SubjSprite> _menuSprites;

	// Menu font sprites.  Two glyph sets are loaded from SPRITE\2D\:
	//   FONTMAX.SPR  → default menu label colour (gold).
	//   FONTMAX2.SPR → hover/highlight glyphs (brighter yellow).
	// Both share the same indexing convention (sprite_index = char_code - 0x20)
	// and the same space advance kFontMaxSpaceAdvance (22 px).  Selecting the
	// hover set is what produces the original game's "yellow tint" on the
	// currently-highlighted menu entry — there is no per-pixel modulation.
	void loadFontMaxSprites();
	void loadFontSpriteSet(const char *spr, Common::Array<SubjSprite> &out);
	Common::Array<SubjSprite> _fontMaxSprites;
	Common::Array<SubjSprite> _fontMaxHoverSprites;
	// FONTMIN.SPR / FONTMIN2.SPR — the smaller, red-tinted font the original
	// uses for player names on the player-management screen.
	Common::Array<SubjSprite> _fontMinSprites;
	Common::Array<SubjSprite> _fontMin2Sprites;

	int  fontMaxCharAdvance(unsigned char c) const;
	int  fontMaxStringWidth(const Common::String &text) const;
	void drawFontMaxText(Graphics::ManagedSurface &dst, const Common::String &text,
	                     int anchorX, int anchorY, bool hover = false) const;
	void drawFontMinText(Graphics::ManagedSurface &dst, const Common::String &text,
	                     int anchorX, int anchorY, bool hover = false) const;
	void drawPlayerText(Graphics::ManagedSurface &dst, const Common::String &text,
	                    int anchorX, int anchorY, bool hover = false) const;
	void drawSprFontText(Graphics::ManagedSurface &dst, const Common::String &text,
	                     int anchorX, int anchorY,
	                     const Common::Array<SubjSprite> &drawGlyphs,
	                     const Common::Array<SubjSprite> &layoutGlyphs,
	                     int spaceAdvance) const;

	// Credits font sprites — sprite atlases used by the original GereGameOver
	// path for the quit-game / completion credit roll.  Two banks:
	//   CREDBLAN.SPR → "blanc" (white) glyphs for regular credit lines.
	//   CREDBLEU.SPR → "bleu" (blue) glyphs used after `\` markers in
	//                   CREDITS.TXT (e.g. "\Directed by").
	// Both share the original engine's 256-byte per-char advance table
	// (atlantis.exe @ VA 0x004966fc), loaded into _creditCharWidths by
	// loadAtlantisExeTables() so the engine's existing per-glyph SPR data
	// drives the visuals while the table drives proportional spacing —
	// the same split FUN_0042073c uses for centering.
	void loadCreditFontSprites();
	Common::Array<SubjSprite> _creditBlanSprites;
	Common::Array<SubjSprite> _creditBleuSprites;
	byte _creditCharWidths[256];  // populated by loadAtlantisExeTables

	int  creditCharAdvance(unsigned char c) const;
	int  creditStringWidth(const Common::String &text) const;
	void drawCreditText(Graphics::ManagedSurface &dst, const Common::String &text,
	                    int anchorX, int anchorY, bool blue) const;

	Graphics::Surface *loadTGA(FileType ft, const char *name);

	// Blit a SPRMENU.SPR sprite onto dst with its hotspot at screen position (hx, hy).
	void blitMenuSprite(Graphics::ManagedSurface &dst, uint idx, int hx, int hy) const;

	// Animated warp-view cursors decoded from SPRITE\2D\CURSEURS.SPR.
	// Three groups map to WarpCursorState: default(0-8), navigate(9-17), talk(18-25).
	struct CursorSpr {
		uint16 w, h;
		int16  xoff, yoff;
		Common::Array<uint16> pixels;  // RGB565; kWarpCursorKey = transparent
		Common::Array<uint8>  blend;   // per-pixel blend factor; kSprNoBlend = opaque
	};

	void loadCursorSprites();
	// Install an alpha-blended hardware cursor from sprite pixels + blend factors.
	void setBlendedCursor(uint16 w, uint16 h, const uint16 *pixels,
	                      const uint8 *blend, int hotX, int hotY, uint16 transpKey);
	void setWarpCursor(int group);   // set cursor group and reset animation
	void tickWarpCursor();            // advance animation frame (call in game loop)
	// Set the carried-item cursor.  When overlayCursorFrame >= 0 that
	// _cursorSprites frame (an action cursor) is composited on top.
	void setItemCursor(const Object *obj, int overlayCursorFrame = -1);
public:
	void setArrowCursor();   // plain Atlantis arrow cursor (CURSEURS.SPR sprite 8)
private:

	Common::Array<CursorSpr> _cursorSprites;
	int    _warpCursorGroup;    // 0=default 1=navigate 2=talk
	int    _warpCursorFrame;    // current frame index within group
	uint32 _warpCursorNextMs;   // g_system->getMillis() threshold for next frame
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_ENGINE_H
