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

#include "audio/decoders/apc.h"
#include "audio/mixer.h"
#include "common/func.h"
#include "common/memstream.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "gui/saveload.h"
#include "common/archive.h"
#include "common/file.h"
#include "image/png.h"
#include <stdlib.h>

#include "cryomni3d/fixed_image.h"
#include "cryomni3d/omni3d.h"

#ifdef USE_HNM
#include "video/hnm_decoder.h"
#endif

#include "cryomni3d/atlantis/engine.h"
#include "cryomni3d/atlantis/f3dc_parser.h"
#include "cryomni3d/atlantis/f3dc_renderer.h"
#include "cryomni3d/atlantis/puzzle_eclipse.h"

namespace CryOmni3D {
namespace Atlantis {

const FixedImageConfiguration CryOmni3DEngine_Atlantis::kFixedImageConfiguration = {
	45, 223, 243, 238, 226, 198, 136, 145, 99, 113,
	470
};

// Chapter initial states: placeId, alpha (horizontal angle), beta (vertical angle).
// These will be updated with real values from game data analysis.
const LevelInitialState CryOmni3DEngine_Atlantis::kChapterInitialStates[] = {
	{  1,  0.f,  0.f },  // chapter 1 — ATLAN1.WAM, first place id=1
	{ 22,  0.f,  0.f },  // chapter 2 — ATLAN2.WAM, first place id=22
	{ 58,  0.f,  0.f },  // chapter 3 — ATLAN3.WAM, first place id=58
	{  1,  0.f,  0.f },  // chapter 4 — ATLAN4.WAM, first place id unknown
	{  1,  0.f,  0.f },  // chapter 5 — ATLAN5.WAM, first place id unknown
};

// Map FileType to the subdirectory prefix used within the BigFile archive.
// Keys are stored with backslash separators as found in BigFile directory entries.
static const char *const kBigFileSubdirs[] = {
	"CYCLO\\",         // kFileTypeCyclo
	"UBB_VUE\\",       // kFileTypeTransition
	"DIALOG\\",        // kFileTypeDialog
	"SYC\\",           // kFileTypeSync
	"FONTS\\",         // kFileTypeFont
	"SPRLIST\\",       // kFileTypeSprite
	"WAM\\",           // kFileTypeWAM
	"OBJETS\\",        // kFileTypeObject
	"MENU\\",          // kFileTypeMenu
	"SCENAR\\",        // kFileTypeScript
	"WAV\\",           // kFileTypeSound
	"SPRITE\\CYCLO\\", // kFileTypeSpriteCyclo
	"SPRITE\\UBB\\",   // kFileTypeSpriteUbb
	"SPRITE\\2D\\",    // kFileTypeSprite2D
	"IMAGES\\",        // kFileTypeImages
	"PUZZLES\\",       // kFileTypePuzzles
};

CryOmni3DEngine_Atlantis::CryOmni3DEngine_Atlantis(OSystem *syst,
        const CryOmni3DGameDescription *gamedesc) :
	CryOmni3DEngine(syst, gamedesc),
	_dialogsMan(this),
	_fixedImage(nullptr),
	_mainPalette(nullptr),
	_transparentPaletteMap(nullptr),
	_transparentSrcStart(uint(-1)), _transparentSrcStop(uint(-1)),
	_transparentDstStart(uint(-1)), _transparentDstStop(uint(-1)),
	_transparentNewStart(uint(-1)), _transparentNewStop(uint(-1)),
	_isPlaying(false),
	_abortCommand(kAbortQuit),
	_loadedSave(uint(-1)),
	_currentPlayer(0),
	_autosavePending(false),
	_currentChapter(uint(-1)),
	_currentPlaceId(uint(-1)),
	_nextPlaceId(uint(-1)),
	_currentPlace(nullptr),
	_warpLoaded(false),
	_propAnimFrames(0),
	_propAnimFrame(0),
	_propAnimNextMs(0),
	_wsprAnimActiveIdx(-1),
	_wsprAnimNextMs(0),
	_spriteLayerDirty(false),
	_npcBoundsDirty(false),
	_dialogGivenObjSprite(-1),
	_currentCONChapter(0),
	_chapterStartMs(0),
	_timerTickNextMs{0, 0},
	_zoomLastMs(0),
	_musicCurrentFile(nullptr),
	_musicRawData(nullptr),
	_musicRawSize(0),
	_musicTrackId(0),
	_warpCursorGroup(0),
	_warpCursorFrame(-1),
	_warpCursorNextMs(0),
	_heroPersoId(0),
	_dialRng("AtlantisDialCam"),
	_lastDialCam(0),
	_conRng("AtlantisConRandom"),
	_dialBaseAlpha(0.0),
	_dialBaseAlphaValid(false) {
	_placeVisits.resize(1500, 0);
}

CryOmni3DEngine_Atlantis::~CryOmni3DEngine_Atlantis() {
	_cycloSurface.free();
	_warpSurface.free();
	delete[] _mainPalette;
	delete[] _transparentPaletteMap;
	delete[] _musicRawData;
	delete _fixedImage;
}

bool CryOmni3DEngine_Atlantis::hasFeature(EngineFeature f) const {
	// No kSupportsSavingDuringRuntime: Atlantis autosaves on chapter
	// transitions and offers no manual save anywhere — not even via the
	// ScummVM global menu.  Loading is still permitted.
	return CryOmni3DEngine::hasFeature(f)
	       || (f == kSupportsLoadingDuringRuntime);
}

bool CryOmni3DEngine_Atlantis::showSubtitles() const {
	return ConfMan.getBool("subtitles");
}

bool CryOmni3DEngine_Atlantis::shouldAbort() {
	if (g_engine->shouldQuit()) {
		_abortCommand = kAbortQuit;
		return true;
	}
	return _isPlaying && _abortCommand != kAbortNoAbort;
}

Common::String CryOmni3DEngine_Atlantis::getSaveStateName(int slot) const {
	return Common::String::format("atlantis.%04d", slot + 1);
}

Common::Error CryOmni3DEngine_Atlantis::loadGameState(int slot) {
	// Loading a save (e.g. from the ScummVM launcher) selects the player that
	// owns the slot's block.
	_currentPlayer = slot / (int)kPlayerSlotStride;
	if (loadGame(slot + 1)) {
		_abortCommand = kAbortLoadGame;
		_loadedSave = slot + 1;
		return Common::kNoError;
	}
	return Common::kReadingFailed;
}

Common::Error CryOmni3DEngine_Atlantis::saveGameState(int slot, const Common::String &desc,
        bool isAutosave) {
	saveGame(slot + 1, desc);
	return Common::kNoError;
}

Common::Path CryOmni3DEngine_Atlantis::getFilePath(FileType fileType,
        const Common::String &baseName) const {
	// Files outside the BigFile (none currently); placeholder for future expansion.
	return Common::Path(baseName);
}

Common::SeekableReadStream *CryOmni3DEngine_Atlantis::openBigFileStream(FileType fileType,
        const Common::String &baseName) const {
	uint typeIdx = (uint)fileType;
	if (typeIdx >= ARRAYSIZE(kBigFileSubdirs))
		return nullptr;

	Common::String fullName = kBigFileSubdirs[typeIdx];
	fullName += baseName;
	// The BigFile stores names case-sensitively as they appear in the DOS archive.
	// Convert to uppercase to match archive entries.
	fullName.toUppercase();

	return _bigFile.createReadStreamForMember(fullName);
}

void CryOmni3DEngine_Atlantis::setupPalette(const byte *colors, uint start, uint num) {
	setPalette(colors, start, num);
}

void CryOmni3DEngine_Atlantis::makeTranslucent(Graphics::Surface &dst,
        const Graphics::Surface &src) const {
	// Placeholder: copy source to destination without translucency.
	// Real implementation requires transparent palette mapping.
	assert(dst.w == src.w && dst.h == src.h);
	const byte *srcPtr = (const byte *)src.getPixels();
	byte *dstPtr = (byte *)dst.getPixels();
	uint size = src.w * src.h;
	if (_transparentPaletteMap) {
		for (uint i = 0; i < size; i++) {
			if (srcPtr[i] >= _transparentSrcStart && srcPtr[i] <= _transparentSrcStop)
				dstPtr[i] = _transparentPaletteMap[srcPtr[i]];
			else
				dstPtr[i] = srcPtr[i];
		}
	} else {
		memcpy(dstPtr, srcPtr, size);
	}
}

void CryOmni3DEngine_Atlantis::calculateTransparentMapping() {
	if (!_transparentPaletteMap || !_mainPalette)
		return;

	for (uint i = _transparentSrcStart; i <= _transparentSrcStop; i++) {
		uint r = _mainPalette[3 * i + 0];
		uint g = _mainPalette[3 * i + 1];
		uint b = _mainPalette[3 * i + 2];
		// Find best match in destination range
		uint bestDist = uint(-1);
		byte bestIdx = (byte)_transparentNewStart;
		for (uint j = _transparentNewStart; j <= _transparentNewStop; j++) {
			uint dr = r > _mainPalette[3 * j] ? r - _mainPalette[3 * j] : _mainPalette[3 * j] - r;
			uint dg = g > _mainPalette[3 * j + 1] ? g - _mainPalette[3 * j + 1] : _mainPalette[3 * j + 1] - g;
			uint db = b > _mainPalette[3 * j + 2] ? b - _mainPalette[3 * j + 2] : _mainPalette[3 * j + 2] - b;
			uint dist = dr * dr + dg * dg + db * db;
			if (dist < bestDist) {
				bestDist = dist;
				bestIdx = (byte)j;
			}
		}
		_transparentPaletteMap[i] = bestIdx;
	}
}

void CryOmni3DEngine_Atlantis::syncSoundSettings() {
	CryOmni3DEngine::syncSoundSettings();

	// Scale every channel by the SONMENU "Volume" master slider, which the
	// base class knows nothing about.  Absent key → full volume (no-op).
	const int maxVol = Audio::Mixer::kMaxMixerVolume;
	int master = ConfMan.hasKey("atlantis_master_volume")
	             ? ConfMan.getInt("atlantis_master_volume") : maxVol;
	master = CLIP(master, 0, maxVol);
	static const Audio::Mixer::SoundType kTypes[] = {
		Audio::Mixer::kMusicSoundType,
		Audio::Mixer::kSFXSoundType,
		Audio::Mixer::kSpeechSoundType,
	};
	for (int i = 0; i < 3; i++)
		_mixer->setVolumeForSoundType(kTypes[i],
		    _mixer->getVolumeForSoundType(kTypes[i]) * master / maxVol);
}

void CryOmni3DEngine_Atlantis::loadStaticData() {
	// Static data (strings, sprite mappings etc.) will be loaded from cryomni3d.dat
	// once Atlantis entries are added to the dat file.
}

void CryOmni3DEngine_Atlantis::loadAtlantisExeTables() {
	// The original engine carries a pointer table at VA 0x004961e0 listing
	// every named resource it can play: 17 WAM filenames followed by ~137
	// APC filenames, terminated by a "FIN" sentinel.  The APC portion is
	// what CON `/set(newsound=N)` indexes into (1-based, so newsound=1 maps
	// to the first APC, "strange.apc").
	//
	// PE section parameters for atlantis.exe (imageBase 0x400000, DGROUP
	// section at vaddr 0x90000 / rawptr 0x71200) are well-known and stable
	// for the MD5-checked build we detect against.
	_newSoundApcNames.clear();
	Common::File exe;
	if (!exe.open("atlantis.exe")) {
		warning("loadAtlantisExeTables: cannot open atlantis.exe");
		return;
	}
	const uint32 kTableFileOff = 0x773e0;
	const uint32 kImageBase    = 0x400000;
	const uint32 kDgroupVaddr  = 0x90000;
	const uint32 kDgroupRawPtr = 0x71200;
	const int kSkipWams = 17;

	exe.seek(kTableFileOff + kSkipWams * 4);
	for (int i = 0; i < 200 && !exe.eos(); i++) {
		uint32 va = exe.readUint32LE();
		if (va < 0x4b0000 || va > 0x4d0000) break;  // off the end of the data
		uint32 fileOff = (va - kImageBase - kDgroupVaddr) + kDgroupRawPtr;
		int64 saved = exe.pos();
		exe.seek(fileOff);
		Common::String s;
		while (!exe.eos()) {
			byte b = exe.readByte();
			if (b == 0) break;
			s += (char)b;
		}
		if (s.equalsIgnoreCase("FIN")) break;
		_newSoundApcNames.push_back(s);
		exe.seek(saved);
	}
	debugC(1, kDebugScript, "loadAtlantisExeTables: %u newsound APC names from atlantis.exe",
	      _newSoundApcNames.size());

	// Inventory-icon table at VA 0x004967b0: zero-terminated int32 array
	// where index i is the sprite slot in SPRITE\2D\OBJETS1.SPR and the
	// value is the world object id that uses that sprite as its inventory
	// icon.  Empirically 39 entries followed by a 0 terminator for the
	// MD5-checked English build; the read loop stops at the first non-
	// positive value or after a generous upper bound so an unexpected
	// build can't run off the end of the data section.
	_inventoryObjectIds.clear();
	const uint32 kInvTableVA = 0x004967b0;
	const uint32 kInvTableFileOff =
	    (kInvTableVA - kImageBase - kDgroupVaddr) + kDgroupRawPtr;
	exe.seek(kInvTableFileOff);
	for (int i = 0; i < 64 && !exe.eos(); i++) {
		int32 id = exe.readSint32LE();
		if (id <= 0)
			break;
		_inventoryObjectIds.push_back((uint)id);
	}
	debugC(1, kDebugScript, "loadAtlantisExeTables: %u inventory object ids from atlantis.exe",
	      _inventoryObjectIds.size());

	// Credits font char-width table at VA 0x004966fc: 256 raw bytes indexed
	// by char code, used for proportional spacing of CREDBLAN.SPR /
	// CREDBLEU.SPR.  Cross-checked against each sprite's stored `w` field
	// (exe['A']=9 ↔ sprite[32].w=9, exe['B']=7 ↔ sprite[33].w=7, etc. —
	// see comment in drawCreditText), so we use the table verbatim with
	// no overrides; the table's space value (0x20 = 13) is authentic.
	memset(_creditCharWidths, 0, sizeof(_creditCharWidths));
	const uint32 kCredWidthsVA = 0x004966fc;
	const uint32 kCredWidthsFileOff =
	    (kCredWidthsVA - kImageBase - kDgroupVaddr) + kDgroupRawPtr;
	exe.seek(kCredWidthsFileOff);
	exe.read(_creditCharWidths, sizeof(_creditCharWidths));
	debugC(1, kDebugScript, "loadAtlantisExeTables: credit char-width table loaded");
}

void CryOmni3DEngine_Atlantis::setupFonts() {
	loadFontMaxSprites();
	loadCreditFontSprites();
}

void CryOmni3DEngine_Atlantis::setupObjects() {
	// Build the object catalogue from the runtime-extracted inventory table
	// (atlantis.exe @ VA 0x004967b0).  The array order is the canonical sprite
	// order in OBJETS1.SPR, so _objects[i] is the icon at sprite slot i — the
	// invariant drawInventoryIcon() depends on for its pointer-arithmetic
	// index lookup.
	_objects.clear();
	_objects.resize(_inventoryObjectIds.size());
	for (uint i = 0; i < _inventoryObjectIds.size(); i++)
		_objects[i].rename(_inventoryObjectIds[i]);
}

void CryOmni3DEngine_Atlantis::drawMenuTitle(Graphics::ManagedSurface *surface, byte color) {
	// Placeholder: no game-specific title font yet.
	(void)surface;
	(void)color;
}

uint CryOmni3DEngine_Atlantis::displayFilePicker(const Graphics::Surface *bgFrame, bool saveMode,
                                                  Common::String &saveName) {
	(void)bgFrame;
	// Delegate to the data-driven native picker (SPRLIST\SELEMENU.TXT layout).
	return displaySavePicker(saveMode, saveName);
}

uint CryOmni3DEngine_Atlantis::displayOptions() {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

	// Build menu entries depending on whether a game is in progress.
	struct Entry { const char *label; uint retval; };
	static const Entry kMenuStartup[] = {
		{ "New Game",  27 },
		{ "Load Game", 28 },
		{ "Quit",      40 },
	};
	// Atlantis has no manual save — the game autosaves on every chapter
	// transition (see autosave()), so the in-game menu offers no "Save Game".
	static const Entry kMenuInGame[] = {
		{ "Continue",  0  },
		{ "Load Game", 28 },
		{ "New Game",  27 },
		{ "Quit",      40 },
	};
	const Entry *entries = _isPlaying ? kMenuInGame : kMenuStartup;
	uint numEntries = _isPlaying ? ARRAYSIZE(kMenuInGame) : ARRAYSIZE(kMenuStartup);

	Graphics::PixelFormat fmt = g_system->getScreenFormat();

	// Load PREINTRO.TGA as menu background; fall back to black if unavailable.
	Graphics::ManagedSurface bgSurf;
	{
		Graphics::Surface *raw = loadTGA(kFileTypeImages, "PREINTRO.TGA");
		if (raw) {
			bgSurf.copyFrom(*raw);
			raw->free();
			delete raw;
		} else {
			bgSurf.create(640, 480, fmt);
			bgSurf.fillRect(Common::Rect(0, 0, 640, 480), bgSurf.format.RGBToColor(0, 0, 0));
		}
	}

	// Overlay the four decorative SPRMENU sprites onto the background.
	// Positions from SPRLIST\SELEMENU.TXT: /spr=idx,hx,hy,w,h
	blitMenuSprite(bgSurf,  8, 110, 182);  // left arrow
	blitMenuSprite(bgSurf,  7, 520, 184);  // right arrow
	blitMenuSprite(bgSurf,  9, 180, 270);  // top-left corner ornament
	blitMenuSprite(bgSurf, 10, 450, 340);  // bottom-right corner ornament

	// Render FONTMAX text labels (positions from SPRLIST\SELEMENU.TXT).
	// y_txt is the bottom of the text box; anchor_x centers the text on screen.
	// The -11 in FUN_0041fb6c is the hit-box left edge, not the render origin.
	if (!_fontMaxSprites.empty()) {
		auto drawCentred = [&](const Common::String &text, int y_txt) {
			int w  = fontMaxStringWidth(text);
			int ax = (640 - w) / 2;
			int ay = y_txt;
			drawFontMaxText(bgSurf, text, ax, ay);
		};
		drawCentred("SELECT PLAYER NAME", 140);
		drawCentred("DELETE PLAYER NAME", 420);
		drawCentred("OK",                 470);
	}

	Graphics::ManagedSurface surf;
	surf.create(640, 480, fmt);
	surf.blitFrom(bgSurf);

	const int lineH   = font ? (font->getFontHeight() + 4) : 20;
	const int blockH  = (int)numEntries * lineH;
	int y = (480 - blockH) / 2;

	uint32 white  = surf.format.RGBToColor(255, 255, 255);
	uint32 yellow = surf.format.RGBToColor(255, 255,   0);

	// Draw title text.
	if (font)
		font->drawString(&surf, "ATLANTIS: The Lost Tale", 0, y - lineH * 2, 640, white,
		                 Graphics::TextAlign::kTextAlignCenter);

	// Build hit-test rects.
	Common::Array<Common::Rect> rects;
	for (uint i = 0; i < numEntries; i++) {
		Common::Rect r(160, y + (int)i * lineH, 480, y + (int)(i + 1) * lineH);
		rects.push_back(r);
		if (font)
			font->drawString(&surf, entries[i].label, r.left, r.top, r.width(), white,
			                 Graphics::TextAlign::kTextAlignCenter);
	}

	g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);
	g_system->updateScreen();

	showMouse(true);
	setArrowCursor();

	int hovered = -1;
	while (!shouldAbort()) {
		pollEvents();
		Common::Point mouse = getMousePos();

		// Redraw with hover highlight.
		int newHovered = -1;
		for (int i = 0; i < (int)numEntries; i++)
			if (rects[i].contains(mouse)) { newHovered = i; break; }

		if (newHovered != hovered) {
			hovered = newHovered;
			surf.blitFrom(bgSurf);
			if (font)
				font->drawString(&surf, "ATLANTIS: The Lost Tale", 0, y - lineH * 2, 640,
				                 white, Graphics::TextAlign::kTextAlignCenter);
			for (uint i = 0; i < numEntries; i++) {
				uint32 col = ((int)i == hovered) ? yellow : white;
				if (font)
					font->drawString(&surf, entries[i].label, rects[i].left, rects[i].top,
					                 rects[i].width(), col,
					                 Graphics::TextAlign::kTextAlignCenter);
			}
			g_system->copyRectToScreen(surf.getPixels(), surf.pitch, 0, 0, 640, 480);
			g_system->updateScreen();
		}

		if (getCurrentMouseButton() == 1 && hovered >= 0) {
			uint retval = entries[hovered].retval;

			// Drain button release.
			while (getCurrentMouseButton() != 0 && !shouldAbort()) {
				pollEvents();
				g_system->delayMillis(10);
			}

			if (retval == 28) {
				// Load game: show slot picker; if cancelled, re-show menu.
				Common::String dummy;
				uint slot = displayFilePicker(nullptr, false, dummy);
				if (slot > 0) {
					_loadedSave = slot;
					return 28;
				}
				// Cancelled — redraw and loop.
				hovered = -1;
				continue;
			} else if (retval == 0) {
				// Continue — return without changing anything.
				return 0;
			} else {
				return retval;
			}
		}

		// Refresh every frame so the software cursor tracks the mouse — the
		// menu graphics above only redraw on a hover change.
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	// Aborted (window close).
	return 40;
}

Common::Error CryOmni3DEngine_Atlantis::run() {
	CryOmni3DEngine::run();

	// Open the game's data archives.  The full game spans four discs
	// (BIGCD1-4.BIG), all merged into one index.  Disc 1 is required; discs
	// 2-4 are mounted if present, so a disc-1-only install still launches
	// (with the later chapters' data unavailable).
	Common::Path bigFilePath("BIGCD1.BIG");
	if (!_bigFile.open(bigFilePath)) {
		bigFilePath = Common::Path("bigcd1.big");
		if (!_bigFile.open(bigFilePath)) {
			return Common::Error(Common::kPathDoesNotExist, "Cannot open BIGCD1.BIG");
		}
	}
	for (int disc = 2; disc <= 4; disc++) {
		if (!_bigFile.open(Common::Path(Common::String::format("BIGCD%d.BIG", disc)))
		        && !_bigFile.open(Common::Path(Common::String::format("bigcd%d.big", disc))))
			warning("Atlantis: BIGCD%d.BIG not found - its chapters will be unavailable", disc);
	}
	debugC(1, kDebugScript, "Atlantis: data archives mounted, %u files total", _bigFile.numFiles());

	// Register with SearchMan so Common::File::open() can find archive files.
	SearchMan.add("Atlantis_BigFile", &_bigFile, 0, false);

	// Pull data tables embedded in atlantis.exe (currently: the newsound APC
	// filename table at VA 0x004961e0).  Runs after the BigFile is mounted
	// so the engine can resolve atlantis.exe via SearchMan.
	loadAtlantisExeTables();

	// SPRLIST\EPI.TXT — the per-episode checkpoint names shown for each
	// per-chapter save (autosave() / displaySavePicker()).
	loadEpisodeNames();

	loadStaticData();
	setupObjects();
	loadSubjectSprites();
	loadCursorSprites();
	loadObjectSprites();
	loadInventoryBarSprite();
	loadMenuSprites();
	setupFonts();

	loadDialInitData();

	_omni3dMan.init(75. / 180. * M_PI);

	_mainPalette = new byte[3 * 256]();
	_transparentPaletteMap = new byte[256]();
	_transparentSrcStart = 0;
	_transparentSrcStop  = 240;
	_transparentDstStart = 0;
	_transparentDstStop  = 248;
	_transparentNewStart = 248;
	_transparentNewStop  = 254;

	_inventory.init(20, new Common::Functor1Mem<uint, void, Atlantis_Toolbar>(
	                        &_toolbar, &Atlantis_Toolbar::inventoryChanged));
	_gameVariables.resize(50, 0);

	_fixedImage = new ZonFixedImage(*this, _inventory, _sprites, &kFixedImageConfiguration);

	// Atlantis uses HNM6 16-bit true color panoramas.  initGraphics MUST be called
	// before toolbar.init so the toolbar surfaces are created in the correct pixel format.
	Graphics::PixelFormat screenFmt(2, 5, 6, 5, 0, 11, 5, 0, 0); // RGB565
	initGraphics(640, 480, &screenFmt);
	_omni3dMan.setPixelFormat(screenFmt);
	_toolbar.init(&_sprites, &_inventory, this);
	// The toolbar background is a single sprite: OBJETS0.SPR (599x82) with
	// the thin golden line, 9 star-of-David slot ornaments, and decorative
	// drops.  Centre the 599-wide sprite on the 640-wide toolbar by placing
	// its left edge at x=20: addMenuSprite computes draw_x = tx - xoff, and
	// the sprite carries xoff=-20, so tx=0 yields draw_x=20.  Inventory
	// item icons are composited on top by the slot rendering path.
	if (!_inventoryBarSprite.pixels.empty()) {
		const SubjSprite &s = _inventoryBarSprite;
		_toolbar.addMenuSprite(s.w, s.h, s.xoff, s.yoff, kSprTransp,
		                       0, s.yoff, s.pixels.data(), s.blend.data());
	}
	setMousePos(Common::Point(320, 240));

	syncSoundSettings();

	_isPlaying = false;
	_abortCommand = kAbortQuit;
	_musicCurrentFile = nullptr;

	int saveSlot = ConfMan.getInt("save_slot");

	// Play Cryo logo and pre-intro cinematics before the main menu.
	if (saveSlot < 0) {
		playTransitionVideo("LOGO15N.HNM");
		playTransitionVideo("PREINTRO.HNM");
	}

	loadPlayers();

	bool stopGame = false;
	while (!stopGame) {
		bool exitLoop = false;
		uint nextStep = 0;

		if (saveSlot > -1) {
			// Launched directly into a save from the ScummVM launcher: skip the
			// player screen and take the player from the save slot's block.
			_currentPlayer = saveSlot / (int)kPlayerSlotStride;
			nextStep = 28;
			_loadedSave = saveSlot + 1;
			saveSlot = -1;
		} else {
			// Original-style player select / create / delete screen.
			int chosen = displayPlayerScreen();
			if (chosen < 0) {
				stopGame = true;
				break;
			}
			_currentPlayer = chosen;
			// An existing player resumes their current episode (the latest
			// per-chapter save); a brand-new player starts a new game.  Either
			// way the New/Load/Quit startup menu (displayOptions) is bypassed.
			uint resume = playerResumeSave((uint)chosen);
			if (resume) {
				_loadedSave = resume;
				nextStep = 28;   // resume the player's current episode
			} else {
				nextStep = 27;   // new game
			}
		}

		if (_sprites.getSpritesCount() > 0)
			setCursor(0);
		while (!exitLoop) {
			_isPlaying = false;
			if (!nextStep) {
				nextStep = displayOptions();
			}
			if (nextStep == 40) {
				exitLoop = true;
				stopGame = true;
			} else if (nextStep == 27 || nextStep == 28 || nextStep == 65) {
				// Leaving the menu for the game: hide the menu cursor so the
				// intro / transition cinematics play with no cursor visible;
				// gameplay (handleWarp) shows its own cursor again.
				showMouse(false);
				if (nextStep == 27) {
					// New game: CON arrival section at the starting place (vue=450) drives
					// the intro cinematics and dialog — no hardcoded sequence here.
					_abortCommand = kAbortNoAbort;
					_isPlaying = true;
					changeChapter(1);
				} else if (nextStep == 28) {
					// Load game
					loadGame(_loadedSave);
				} else if (nextStep == 65) {
					// _currentChapter was set by executeTransition() when it detected
					// a chapter-WAM sub-WAM (e.g. atlan2.wam).
					changeChapter(_currentChapter);
				}

				_isPlaying = true;
				nextStep = 0;
				_abortCommand = kAbortNoAbort;

				gameStep();

				assert(_abortCommand != kAbortNoAbort);
				switch (_abortCommand) {
				case kAbortFinished:
				case kAbortGameOver:
					exitLoop = true;
					break;
				case kAbortQuit:
					exitLoop = true;
					stopGame = true;
					break;
				case kAbortNewGame:
					nextStep = 27;
					break;
				case kAbortLoadGame:
					nextStep = 28;
					break;
				case kAbortNextChapter:
					nextStep = 65;
					break;
				default:
					error("Invalid abortCommand: %d", _abortCommand);
				}
			}
		}
	}

	SearchMan.remove("Atlantis_BigFile");
	return Common::kNoError;
}

void CryOmni3DEngine_Atlantis::changeChapter(int chapter) {
	_currentChapter = chapter;

	_mixer->stopAll();
	musicStop();   // a new chapter's CON re-selects its music via startmusik

	if (chapter == 1) {
		for (uint &v : _gameVariables)
			v = 0;
		_inventory.clear();
		for (byte &b : _placeVisits)
			b = 0;
	}
	_sujEnabled.clear();
	_hiddenPersos.clear();
	_npcPlaceBounds.clear();
	_conScript.reset();
	_gameVars.clear();

	_nextPlaceId = uint(-1);
	initNewChapter(chapter);
}

void CryOmni3DEngine_Atlantis::initNewChapter(int chapter) {
	// setupChapterWAM calls loadConScript, which calls execConInitCommands.
	// The INIT block may set _nextPlaceId via vue=N — honour that value.
	setupChapterWAM(chapter);
	initPlacesStates();
	initChapterPlaceStates(chapter);

	if (chapter < 1 || chapter > (int)ARRAYSIZE(kChapterInitialStates))
		error("Invalid chapter %d", chapter);

	const LevelInitialState &init = kChapterInitialStates[chapter - 1];
	if (_nextPlaceId == uint(-1))
		_nextPlaceId = init.placeId;  // fall back to hardcoded start if CON gave none
	_omni3dMan.setAlpha(init.alpha);
	_omni3dMan.setBeta(init.beta);

	if (chapter == 1) {
		// Seth begins chapter 1 carrying his companion's pass (item 257).
		// No CON INIT command grants it; it's a starting item.
		for (Object &o : _objects) {
			if (o.valid() && o.idOBJ() == 257) {
				if (!_inventory.inInventoryByNameID(257))
					_inventory.add(&o);
				break;
			}
		}
	}
}

static const char *const kWAMNames[] = {
	nullptr,
	"ATLAN1.WAM",
	"ATLAN2.WAM",
	"ATLAN3.WAM",
	"ATLAN4.WAM",
	"ATLAN5.WAM",
};

void CryOmni3DEngine_Atlantis::setupChapterWAM(int chapter) {
	if (chapter < 1 || chapter >= (int)ARRAYSIZE(kWAMNames) || !kWAMNames[chapter])
		error("No WAM for chapter %d", chapter);

	Common::ScopedPtr<Common::SeekableReadStream> s(
	    openBigFileStream(kFileTypeWAM, kWAMNames[chapter]));
	if (!s)
		error("Cannot open WAM file for chapter %d (%s)", chapter, kWAMNames[chapter]);

	_wam.loadStream(*s);
	_currentWAMName = kWAMNames[chapter];
	debugC(1, kDebugScript, "Loaded WAM for chapter %d: %u places", chapter, _wam.getPlaces().size());
	loadSpritePlaceList();
	loadSpfList();
	if ((uint)chapter != _currentCONChapter) {
		_currentCONChapter = (uint)chapter;
		loadConScript(chapter);
	}
}

void CryOmni3DEngine_Atlantis::setupWAMByName(const Common::String &wamName) {
	Common::ScopedPtr<Common::SeekableReadStream> s(
	    openBigFileStream(kFileTypeWAM, wamName));
	if (!s) {
		warning("Cannot open WAM file '%s'", wamName.c_str());
		return;
	}
	_wam.loadStream(*s);
	_currentWAMName = wamName;
	debugC(1, kDebugScript, "Loaded WAM '%s': %u places", wamName.c_str(), _wam.getPlaces().size());
	initPlacesStates();
	// Each WAM references its own spwatlN.txt / spfatlN.txt — without
	// reloading these here, sub-WAM hot-swaps (e.g. atlan1 → atlan4) keep
	// the previous chapter's per-place sprite/transition data and NPCs in
	// the new area (place 92 perso 602, place 93, ...) never composite.
	loadSpritePlaceList();
	loadSpfList();
}

void CryOmni3DEngine_Atlantis::initPlacesStates() {
	_placeStates.resize(256);
	for (PlaceState &ps : _placeStates)
		ps = PlaceState();
}

void CryOmni3DEngine_Atlantis::initChapterPlaceStates(int chapter) {
	// Wire place-specific filterEvent callbacks (implemented in logic.cpp).
	// Guard logic at place 17 is handled by the CON script; no hardcoded filters needed.
	(void)chapter;
}

void CryOmni3DEngine_Atlantis::playConDialog(int dialogId,
                                              const Common::String &text,
                                              const Common::String &bgVideoName) {
	// Audio: WAV\ATA<dialogId4><track>.APC where dialogId is the CON section number.
	const int kBarHeight = 80;
	const int kScreenW   = 640;
	const int kScreenH   = 480;

	// Subtitles toggle: when the user has turned them off in the ScummVM
	// options, draw nothing into the subtitle bar -- the audio still plays.
	const Common::String &subText = showSubtitles() ? text : Common::String();

	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface sub;
	sub.create(kScreenW, kBarHeight, fmt);
	sub.fillRect(Common::Rect(kScreenW, kBarHeight), 0);

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (font && !subText.empty()) {
		uint32 white = fmt.RGBToColor(255, 255, 180);
		int lineH = font->getFontHeight() + 2;
		Common::Array<Common::String> lines;
		font->wordWrapText(subText, kScreenW - 16, lines);
		int y = (kBarHeight - (int)lines.size() * lineH) / 2;
		for (uint i = 0; i < lines.size(); i++, y += lineH)
			font->drawString(&sub, lines[i], 8, y, kScreenW - 16, white,
			                 Graphics::kTextAlignCenter);
	}

	if (_warpLoaded)
		_omni3dMan.updateCoords(0, 0, false);

#ifdef USE_HNM
	Video::HNMDecoder *bgDecoder = nullptr;
	const Graphics::Surface *bgFrame = nullptr;

	if (!bgVideoName.empty()) {
		Common::String hnmName = bgVideoName;
		const char *dot = strrchr(hnmName.c_str(), '.');
		if (dot && !scumm_stricmp(dot, ".UBB"))
			hnmName = Common::String(hnmName.c_str(), dot - hnmName.c_str()) + ".HNM";
		Common::SeekableReadStream *bgStream = openBigFileStream(kFileTypeDialog, hnmName);
		if (bgStream) {
			bgDecoder = new Video::HNMDecoder(fmt, true);  // loop=true: seeks back on EOF
			if (!bgDecoder->loadStream(bgStream)) {
				delete bgDecoder;
				bgDecoder = nullptr;
			} else {
				bgDecoder->start();
			}
		}
	}
#endif

	auto drawFrame = [&]() {
#ifdef USE_HNM
		if (bgDecoder) {
			if (bgDecoder->needsUpdate()) {
				const Graphics::Surface *f = bgDecoder->decodeNextFrame();
				if (f)
					bgFrame = f;
			}
			if (bgFrame)
				g_system->copyRectToScreen(bgFrame->getPixels(), bgFrame->pitch,
				                           0, 0, bgFrame->w, bgFrame->h);
		} else
#endif
		if (_warpLoaded) {
			const Graphics::Surface *bg = _omni3dMan.getSurface();
			if (bg)
				g_system->copyRectToScreen(bg->getPixels(), bg->pitch, 0, 0, bg->w, bg->h);
		}
		g_system->copyRectToScreen(sub.getPixels(), sub.pitch,
		                           0, kScreenH - kBarHeight, kScreenW, kBarHeight);
		g_system->updateScreen();
	};

	// Play sequential APC tracks A, B, C, ...
	for (char track = 'A'; track <= 'Z' && !shouldAbort(); track++) {
		// One click skips only one track: drain a still-held skip input
		// carried over from skipping the previous track.
		while (getCurrentMouseButton() != 0 && !shouldAbort()) {
			pollEvents();
			g_system->delayMillis(10);
		}
		clearKeys();
		Common::String apcName = Common::String::format("ATA%04d%c.APC", dialogId, track);
		Common::SeekableReadStream *apcFile = openBigFileStream(kFileTypeSound, apcName);
		if (!apcFile) {
			if (track == 'A')
				debugC(1, kDebugScript, "playConDialog: no audio for dialogId=%d", dialogId);
			break;
		}

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
			Audio::SoundHandle handle;
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, apc);
			debugC(1, kDebugScript, "playConDialog: playing %s", apcName.c_str());

			// Show panorama + subtitle while audio plays; allow click/key to skip.
			while (!shouldAbort() && _mixer->isSoundHandleActive(handle)) {
				drawFrame();
				pollEvents();
				if (checkKeysPressed(1, Common::KEYCODE_SPACE) || getCurrentMouseButton() != 0) {
					_mixer->stopHandle(handle);
					break;
				}
				g_system->delayMillis(10);
			}
			_mixer->stopHandle(handle);
		}
		delete apcFile;
	}

	// Leave subtitle visible for ~1s after all tracks (or if no audio).
	if (!shouldAbort()) {
		for (int ms = 0; ms < 1000 && !shouldAbort(); ms += 20) {
			drawFrame();
			pollEvents();
			if (checkKeysPressed(1, Common::KEYCODE_SPACE) || getCurrentMouseButton() != 0)
				break;
			g_system->delayMillis(20);
		}
	}
#ifdef USE_HNM
	delete bgDecoder;
#endif
	sub.free();
}

void CryOmni3DEngine_Atlantis::gameStep() {
	while (!_abortCommand) {
		if (_nextPlaceId != uint(-1)) {
			if (_nextPlaceId < _placeStates.size() && _placeStates[_nextPlaceId].initPlace)
				(this->*_placeStates[_nextPlaceId].initPlace)();
			// Music persists across places — only startmusik/stopmusik (run by
			// the new place's arrival CON sections, if any) change it.
			doPlaceChange();
			musicUpdate();
		}

		// An arrival /sel section (e.g. vue=1 at the end of the intro) may have set
		// _nextPlaceId during doPlaceChange().  Loop back to process the new destination
		// instead of showing the current (transitional) panorama to the player.
		if (_nextPlaceId != uint(-1))
			continue;

		uint actionId = handleWarp();
		musicUpdate();
		debugC(2, kDebugScript, "handleWarp returned %u", actionId);

		// handleWarp returns 66666 when it leaves its loop without a place to
		// navigate to.  If an abort is pending let run() process it; otherwise
		// it bailed for a transient reason — re-enter the warp.  Handled here,
		// before filterEvent(), so a place handler never sees the sentinel.
		if (actionId == 66666) {
			if (_abortCommand != kAbortNoAbort)
				break;
			debugC(1, kDebugScript, "gameStep: handleWarp bailed with no abort; resuming warp");
			continue;
		}

		// Script-initiated transition: handleWarp returned a destination that
		// was set by /set(Vue=N) inside an NPC dialog (it left _nextPlaceId
		// equal to the returned actionId).  Skip the user-click pipeline —
		// no zone /con scan, no departure /sel scan, no abortdepart gate —
		// just transition.  /set(Vue=N) in the exe schedules the new place
		// directly, independent of the lock-the-player /sel sections that
		// fire on player navigation clicks.
		if (actionId > 0 && _nextPlaceId == (uint)actionId) {
			uint dest = _nextPlaceId;
			_nextPlaceId = uint(-1);
			executeTransition(dest);
			continue;
		}

		_nextPlaceId = uint(-1);
		bool doEvent = true;
		if (_currentPlaceId < _placeStates.size() && _placeStates[_currentPlaceId].filterEvent)
			doEvent = (this->*_placeStates[_currentPlaceId].filterEvent)(&actionId);

		if (_abortCommand)
			break;

		if (actionId >= 1 && actionId < 10000) {
			if (doEvent) {
				// A zone action can be either a navigation destination (place ID)
				// or a zone-click handler key (matched against CON section
				// `clicZone` fields, e.g. `/con*(cliczone=499)` for an item-use
				// hotspot).  CON authors put click handlers in a separate
				// numeric range (typically 100+) from place IDs, but the data
				// itself is what tells us which is which: if any CON section's
				// clicZone matches this action, route the click through the
				// zone-click path and never invoke executeTransition (place
				// "499" doesn't exist anywhere).
				bool isClicZone = false;
				for (const ConSection &sec : _conScript.sections()) {
					if (sec.clicZone == (int)actionId) {
						isClicZone = true;
						break;
					}
				}

				if (isClicZone) {
					runZoneConSections((int)actionId);
					// The fired /con* section may have requested a place
					// change via /set(vue=N) -- CHAPI012 section 14
					// (`/con*(Cliczone=3260)&(ObjetEnMain=260)&(Flagtemp6=1)`
					// -> /set(Cinema=31) + /set(Vue=415)) is the canonical
					// case: clicking the lion-stone with object 260 plays
					// the door-opening cinematic and then must transition
					// to vue 415 (the cyclo showing the now-open door).
					// Previously this branch unconditionally cleared
					// _nextPlaceId, so the cinematic played but the cyclo
					// stayed on the closed-door view.
					if (!shouldAbort() && _nextPlaceId != uint(-1)) {
						uint dest = _nextPlaceId;
						_nextPlaceId = uint(-1);
						executeTransition(dest);
					}
				} else if (_inventory.selectedObject() &&
				           _inventory.selectedObject()->idOBJ() != uint(-1)) {
					// Object in hand: a plain navigation zone is inert.  The
					// original engine (atlantis.exe FUN_004212cc, the place-
					// change block at ~0x424070) only navigates when
					// objetenmain == 0 — it gates the whole move on
					// `*(int *)(... + 0x611210) == 0`.  Clicking a destination
					// while an item is selected does nothing; the player must
					// put the object away first.  Using an item *on* a hotspot
					// is a separate ClicZone /con path (handled above) and is
					// unaffected.
					_nextPlaceId = uint(-1);
				} else {
					// Place transition: first check zone-click /con* sections
					// (which may redirect via /set(vue=…) or abort by clearing
					// _nextPlaceId), then run departure /sel sections, then
					// execute the resulting transition.
					_nextPlaceId = actionId;       // tentative departure destination
					runZoneConSections((int)actionId);

					if (!shouldAbort() && _nextPlaceId == actionId)
						runDepartureSelSections(actionId);

					if (!shouldAbort() && _nextPlaceId != uint(-1)) {
						uint dest = _nextPlaceId;
						_nextPlaceId = uint(-1);    // executeTransition sets it at the end
						executeTransition(dest);
					} else {
						_nextPlaceId = uint(-1);    // aborted or shouldAbort
					}
				}
			}
		} else if (actionId >= 10000 && actionId < 20000) {
			// Speak/react action: player says something or an ambient reaction plays.
			// Not observed in Atlantis WAM data; no-op for now.
		} else if (actionId >= 40000 && actionId < 50000) {
			// Close-up examination: not yet implemented (Atlantis uses CON vue= redirects).
		}
		// actionId == 66666 is handled above, before filterEvent().
	}
}

// Dump the whole script-visible story state to the log.  The CON section
// conditions are already printed by loadConScript(), so a stuck point can be
// read off directly: compare a section's conditions against these values.
void CryOmni3DEngine_Atlantis::debugDumpStoryState(const char *ctx) {
	Common::String vars;
	for (Common::HashMap<Common::String, int>::const_iterator it = _gameVars.begin();
	     it != _gameVars.end(); ++it)
		vars += Common::String::format("%s=%d ", it->_key.c_str(), it->_value);
	debugC(1, kDebugScript, "STORY STATE [%s]: place=%u CONchapter=%u musicTrack=%d | %s",
	      ctx, _currentPlaceId, _currentCONChapter, _musicTrackId, vars.c_str());
}

void CryOmni3DEngine_Atlantis::doPlaceChange() {
	debugC(1, kDebugScript, "doPlaceChange: ENTER _currentPlaceId=%d _nextPlaceId=%d "
	         "_currentPlace=%s _autosavePending=%d",
	      (int)_currentPlaceId, (int)_nextPlaceId,
	      _currentPlace ? "set" : "null", (int)_autosavePending);
	// The place we are leaving — handed to runArrivalSelSections() as the
	// "arrived from" value so /sel (arriveevue=N)&(vue=M) conditions resolve.
	const uint fromPlaceId = _currentPlaceId;

	const AtlantisPlace *nextPlace = _wam.findPlaceById((uint16)_nextPlaceId);
	if (!nextPlace) {
		// Place not in current WAM — check sub-WAMs keyed by destination.
		for (const AtlantisSubWAM &sub : _wam.getSubWAMs()) {
			if (sub.dstPlaceId == _nextPlaceId) {
				// Copy wamName before setupWAMByName clears _subWAMs.
				Common::String wamName = sub.wamName;
				debugC(1, kDebugScript, "doPlaceChange: place %u not found, loading sub-WAM '%s'",
				      _nextPlaceId, wamName.c_str());
				setupWAMByName(wamName);
				nextPlace = _wam.findPlaceById((uint16)_nextPlaceId);
				break;
			}
		}
		if (!nextPlace) {
			warning("doPlaceChange: place %u not found in WAM or any sub-WAM", _nextPlaceId);
			_nextPlaceId = uint(-1);
			return;
		}
	}

	_currentPlace = nextPlace;
	_currentPlaceId = _nextPlaceId;
	_nextPlaceId = uint(-1);

	// A place change ends any dialog context: drop a pending "object received"
	// icon so it can't leak onto the next scene.
	_dialogGivenObjSprite = -1;

	// Original-game autosave: when a CON `chapter=N` command advanced the
	// chapter, persist the player's single save now that the new chapter's
	// first place is set (state — inventory, vars, place — is consistent
	// here, and the panorama has not been entered yet).
	if (_autosavePending) {
		debugC(1, kDebugScript, "doPlaceChange: running autosave for chapter %u", _currentCONChapter);
		_autosavePending = false;
		autosave();
		debugC(1, kDebugScript, "doPlaceChange: autosave done");
	}

	// Load the panoramic cyclo HNM, then composite NPC sprites on top.
	if (loadCycloHNM(nextPlace->cycloHnm)) {
		compositeNPCSprites();
		_omni3dMan.setSourceSurface(&_warpSurface);
		setupPalette(_mainPalette, 0, 256);
	}

	// Full 360° horizontal rotation — no alpha clamp, wrapping is handled
	// internally in updateCoords. Limit vertical tilt to ±45°.
	_omni3dMan.setBetaMinConstraint(-M_PI / 4.);
	_omni3dMan.setBetaMaxConstraint( M_PI / 4.);

	setMousePos(Common::Point(320, 240));

	debugC(1, kDebugScript, "doPlaceChange: at place %u (%s)", _currentPlaceId, nextPlace->cycloHnm.c_str());
	debugDumpStoryState("place change");
	for (const AtlantisZone &z : nextPlace->zones)
		debugC(2, kDebugScript, "  zone: action=%d x=%d y=%d w=%d h=%d",
		      z.action, z.x, z.y, z.w, z.h);

	// Each place starts at the 75 degrees navigation FOV.  Clear any zoom
	// left over from the place just left (atlantis.exe FUN_004212cc restores
	// the FOV and zeroes var[zoom] when its warp loop ends) so this place's
	// arrival /sel can request a fresh one.
	_omni3dMan.setHFov(75.0 / 180.0 * M_PI);
	_gameVars["zoom"] = 0;
	_zoomLastMs = 0;

	// Run /sel arrival sections: intro dialog, cinematics, story triggers.
	runArrivalSelSections((int)_currentPlaceId, (int)fromPlaceId);

	// Count this visit AFTER the arrival sections, so a place's first-visit
	// /sel sections (nbvisite=0) fire exactly once (NbVisite).
	if (_currentPlaceId < _placeVisits.size() && _placeVisits[_currentPlaceId] < 255)
		_placeVisits[_currentPlaceId]++;
}

void CryOmni3DEngine_Atlantis::executeTransition(uint nextPlaceId) {
	// _currentPlace can be null after /set(changewam=...) -- the script's
	// next /set(vue=N) is the destination, and there is no transition video
	// between the old and new WAMs anyway.  Skip the transition lookup and
	// the cross-WAM hand-off (the new WAM is already loaded) and just hand
	// the destination to gameStep's outer loop via _nextPlaceId so
	// doPlaceChange runs.  Previously we early-returned which left
	// _nextPlaceId at -1, so the destination silently dropped.
	if (!_currentPlace) {
		_nextPlaceId = nextPlaceId;
		return;
	}

	const AtlantisTransition *trans = _currentPlace->findTransition((uint16)nextPlaceId);
	if (trans) {
		playTransitionVideo(trans->videoName, _currentPlaceId, nextPlaceId);
		_omni3dMan.setAlpha(trans->dstAlpha);
		_omni3dMan.setBeta(trans->dstBeta);
	}

	// Cross-WAM transition: sub-WAMs hot-swap the navigation map when a
	// (src, dst) pair crosses into a different WAM.  Even chapter-named WAMs
	// (atlan2/3/4/5.wam) are hot-swapped here without resetting chapter
	// state — those WAMs serve as sub-areas of chapter 1.  Actual chapter
	// progression is driven by CON `/set(chapter=N)` once gameplay in the
	// new sub-area triggers it (e.g. arriving at vue=14 advances chapter 1
	// to chapter 2 via CHAPI001.CON section 10).
	for (const AtlantisSubWAM &sub : _wam.getSubWAMs()) {
		if (sub.srcPlaceId == _currentPlaceId && sub.dstPlaceId == nextPlaceId) {
			// Copy fields to locals BEFORE setupWAMByName: that call invokes
			// _wam.clear() which destroys _subWAMs, making sub a dangling ref.
			Common::String wamName = sub.wamName;
			uint16 dstPlaceId     = sub.dstPlaceId;

			debugC(1, kDebugScript, "Sub-WAM transition: place %u → WAM '%s' place %u",
			      nextPlaceId, wamName.c_str(), dstPlaceId);
			setupWAMByName(wamName);
			// _wam._places was cleared; _currentPlace is now a dangling pointer.
			_currentPlace   = nullptr;
			_currentPlaceId = uint(-1);
			_nextPlaceId    = dstPlaceId;
			return;
		}
	}

	_nextPlaceId = nextPlaceId;
}

bool CryOmni3DEngine_Atlantis::loadCycloHNM(const Common::String &name) {
#ifdef USE_HNM
	Common::SeekableReadStream *stream = openBigFileStream(kFileTypeCyclo, name);
	if (!stream && !name.hasSuffix(".hnm") && !name.hasSuffix(".HNM")) {
		// Some WAM entries use .tga extension but the archive only has .hnm
		Common::String hnmName = name;
		const char *dot = strrchr(hnmName.c_str(), '.');
		if (dot) {
			hnmName = Common::String(hnmName.c_str(), dot - hnmName.c_str()) + ".hnm";
			stream = openBigFileStream(kFileTypeCyclo, hnmName);
		}
	}
	if (!stream) {
		warning("Cannot open cyclo HNM: %s", name.c_str());
		return false;
	}

	// HNM6 panoramas are 16-bit true color; use the screen pixel format.
	// HNMDecoder takes ownership of the stream when loadStream() is called.
	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Video::HNMDecoder *decoder = new Video::HNMDecoder(fmt);

	// loadStream takes ownership of stream immediately (even on failure).
	if (!decoder->loadStream(stream)) {
		warning("Cannot load cyclo HNM: %s", name.c_str());
		delete decoder;
		return false;
	}

	decoder->start();

	const Graphics::Surface *frame = nullptr;
	while (!frame && !decoder->endOfVideo()) {
		if (decoder->needsUpdate())
			frame = decoder->decodeNextFrame();
		else
			decoder->decodeNextFrame();
	}

	if (!frame) {
		warning("No frames in cyclo HNM: %s", name.c_str());
		delete decoder;
		return false;
	}

	_cycloSurface.free();
	_cycloSurface.copyFrom(*frame);
	_warpSurface.free();
	_warpSurface.copyFrom(*frame);

	if (decoder->hasDirtyPalette())
		memcpy(_mainPalette, decoder->getPalette(), 3 * 256);

	delete decoder;
	_warpLoaded = true;
	_propAnimFrames = 0;
	_propAnimFrame  = 0;
	_propAnimNextMs = 0;
	debugC(1, kDebugScript, "loadCycloHNM: %s (%dx%d)", name.c_str(), _warpSurface.w, _warpSurface.h);
	return true;
#else
	warning("HNM support not compiled in; cannot load cyclo %s", name.c_str());
	return false;
#endif
}

void CryOmni3DEngine_Atlantis::playTransitionVideo(const Common::String &videoName,
                                                    uint fromPlace, uint toPlace) {
#ifdef USE_HNM
	// WAM records use .UBB extension but the BigFile stores them as .HNM.
	Common::String hnmName = videoName;
	{
		const char *dot = strrchr(hnmName.c_str(), '.');
		if (dot && !scumm_stricmp(dot, ".UBB"))
			hnmName = Common::String(hnmName.c_str(), dot - hnmName.c_str()) + ".HNM";
	}

	Common::SeekableReadStream *stream = openBigFileStream(kFileTypeTransition, hnmName);
	if (!stream) {
		debugC(1, kDebugScript, "playTransitionVideo: '%s' not found", hnmName.c_str());
		return;
	}

	// Open SPF transition-sprite streams that apply to this (from,to) pair.
	// Matches the original engine's FUN_0041b81c: scan the SPF table for
	// entries keyed by (current place, target place) and load each.  Streams
	// stay open for the duration of the transition so per-frame decode is
	// cheap; closed afterwards.  A no-op when either place ID is unknown
	// (e.g. cinematic videos played via the legacy single-arg overload).
	Common::Array<Common::SeekableReadStream *> spfStreams;
	if (fromPlace != uint(-1) && toPlace != uint(-1)) {
		for (const SpriteTransEntry &e : _spriteTransList) {
			if (e.fromPlace != fromPlace || e.toPlace != toPlace) continue;
			// Honour the hidden-perso list so /set(HidePerso=N) suppresses
			// transition sprites for that NPC just like static sprites.
			bool hidden = false;
			for (int h : _hiddenPersos)
				if (h == e.persoId) { hidden = true; break; }
			if (hidden) continue;
			Common::SeekableReadStream *spf =
			    openBigFileStream(kFileTypeSpriteUbb, e.spfFile);
			if (spf) {
				spfStreams.push_back(spf);
				debugC(2, kDebugScript, "playTransitionVideo: opened SPF '%s' (perso=%d)",
				      e.spfFile.c_str(), e.persoId);
			}
		}
	}

	Graphics::PixelFormat screenFmt = g_system->getScreenFormat();
	Graphics::PixelFormat fmt = (screenFmt.bytesPerPixel == 1)
	    ? Graphics::PixelFormat::createFormatCLUT8()
	    : screenFmt;
	byte *savedPalette = nullptr;
	if (screenFmt.bytesPerPixel == 1) {
		savedPalette = new byte[3 * 256];
		g_system->getPaletteManager()->grabPalette(savedPalette, 0, 256);
	}

	Video::HNMDecoder *decoder = new Video::HNMDecoder(fmt, false, savedPalette);
	decoder->setSoundType(Audio::Mixer::kSFXSoundType);
	// loadStream takes ownership immediately.
	if (!decoder->loadStream(stream)) {
		delete decoder;
		return;
	}

	decoder->start();

	// Play matching APC audio if present (e.g. WAV\CINEM020.APC for CINEM020.HNM).
	// AT*.HNM place-to-place transitions are intentionally silent.
	Audio::SoundHandle apcHandle;
	{
		const char *dot = strrchr(hnmName.c_str(), '.');
		Common::String apcName = dot
		    ? Common::String(hnmName.c_str(), dot - hnmName.c_str()) + ".APC"
		    : hnmName + ".APC";
		Common::SeekableReadStream *apcFile = openBigFileStream(kFileTypeSound, apcName);
		if (apcFile) {
			// makeAPCStream reads the file header; queue remaining bytes as one packet.
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
				_mixer->playStream(Audio::Mixer::kSFXSoundType, &apcHandle, apc);
			}
			delete apcFile;
		}
	}

	uint spfFrameIdx = 0;
	Graphics::Surface composite;  // per-frame scratch when sprites are active
	while (!shouldAbort() && !decoder->endOfVideo()) {
		if (decoder->needsUpdate()) {
			const Graphics::Surface *frame = decoder->decodeNextFrame();
			if (frame) {
				if (decoder->hasDirtyPalette())
					setPalette(decoder->getPalette(), 0, 256);

				const Graphics::Surface *toBlit = frame;
				if (!spfStreams.empty() && fmt.bytesPerPixel >= 2) {
					// Composite the SPF sprites for this frame into a
					// throwaway copy of the video frame.  The SPF pixel
					// stream is RGB565 (format=9), so we only do this
					// when the target surface is at least 16bpp — at
					// 8bpp the formats don't mix and we just show the
					// raw video.
					if (composite.w != frame->w || composite.h != frame->h
					 || composite.format != fmt) {
						composite.free();
						composite.create(frame->w, frame->h, fmt);
					}
					const uint rowBytes = (uint)frame->w * fmt.bytesPerPixel;
					for (int row = 0; row < frame->h; ++row)
						memcpy(composite.getBasePtr(0, row),
						       frame->getBasePtr(0, row), rowBytes);
					for (Common::SeekableReadStream *spf : spfStreams)
						compositeSpfFrame(spf, spfFrameIdx, composite);
					toBlit = &composite;
				}

				g_system->copyRectToScreen(toBlit->getPixels(), toBlit->pitch,
				                           0, 0, toBlit->w, toBlit->h);
				g_system->updateScreen();
				++spfFrameIdx;
			}
		}
		pollEvents();
		if (checkKeysPressed(1, Common::KEYCODE_SPACE))
			break;
		g_system->delayMillis(10);
	}
	_mixer->stopHandle(apcHandle);
	delete decoder;
	for (Common::SeekableReadStream *spf : spfStreams)
		delete spf;
	composite.free();
#else
	debugC(1, kDebugScript, "playTransitionVideo: %s (HNM not compiled)", videoName.c_str());
#endif
}

// ---------------------------------------------------------------------------
// QUIT GAME outro slideshow
// ---------------------------------------------------------------------------
//
// Triggered by the in-game (Escape) menu's QUIT GAME entry.  Mirrors the
// original game's credits sequence: CREDIT01..10.TGA in the left ~340 px
// of the screen (the right ~300 px is a black region authored as a text
// overlay area) and the FUN_00428... credits loop renders the matching
// $NN section of SPRLIST\CREDITS.TXT into that black area.  We reproduce
// both halves here.
//
// Asset notes (verified against BIGCD1.BIG):
//   * END.TGA and 04GENERI.APC are NOT on disc 1 — they ship on a later CD.
//     We skip END.TGA entirely and try 04GENERI.APC first (so multi-disc
//     installs still work), falling back to 33INTRO.APC (track 20 in
//     kMusicTrackNames; present on disc 1) for that "Cryo narration" feel.
//   * CREDIT01..10.TGA are 640×480; profiling the pixels confirms the right
//     half is uniformly black (intensity 0) starting around x=340.
//   * SPRLIST\CREDITS.TXT is segmented by `$NN` markers (10 sections);
//     within each section `<>TEXT` is a credit line and `<>\TEXT` is a
//     section label (rendered the same way here — the leading `\` is
//     stripped).  Section NN corresponds to CREDITNN.TGA.

namespace {

// One parsed line of credits text.
struct CreditsLine {
	Common::String text;
	bool label;     // true if the original line started with `\`
};

// Split CREDITS.TXT into 10 sections keyed by `$NN`.  Lines that are not
// `$NN` or `<>...` are ignored (the file contains blank padding lines).
struct CreditsScript {
	Common::Array<CreditsLine> sections[10];
};

static void parseCreditsScript(Common::ReadStream &stream, CreditsScript &out) {
	Common::Array<byte> buf;
	byte chunk[256];
	while (!stream.eos()) {
		uint32 got = stream.read(chunk, sizeof(chunk));
		if (!got) break;
		for (uint32 i = 0; i < got; i++)
			buf.push_back(chunk[i]);
	}
	buf.push_back(0);

	int section = -1;       // 0..9, set when we see `$NN`
	const char *cur = (const char *)buf.data();
	while (*cur) {
		Common::String line;
		while (*cur && *cur != '\n' && *cur != '\r') {
			line += *cur;
			++cur;
		}
		while (*cur == '\n' || *cur == '\r') ++cur;

		if (line.empty())
			continue;

		// Section marker: `$NN` (one or two digits).
		if (line[0] == '$' && line.size() >= 2) {
			int n = 0;
			for (uint i = 1; i < line.size(); i++) {
				char c = line[i];
				if (c < '0' || c > '9') break;
				n = n * 10 + (c - '0');
			}
			if (n >= 1 && n <= 10)
				section = n - 1;
			continue;
		}

		// Body line: `<>TEXT` or `<>\TEXT`.
		if (section >= 0 && line.size() >= 2 && line[0] == '<' && line[1] == '>') {
			CreditsLine cl;
			const char *p = line.c_str() + 2;
			if (*p == '\\') {
				cl.label = true;
				++p;
			} else {
				cl.label = false;
			}
			cl.text = Common::String(p);
			out.sections[section].push_back(cl);
		}
	}
}

} // anonymous

void CryOmni3DEngine_Atlantis::playQuitOutro() {
	static const char *const kSlides[10] = {
		"CREDIT01.TGA", "CREDIT02.TGA", "CREDIT03.TGA", "CREDIT04.TGA",
		"CREDIT05.TGA", "CREDIT06.TGA", "CREDIT07.TGA", "CREDIT08.TGA",
		"CREDIT09.TGA", "CREDIT10.TGA",
	};
	static const uint32 kSlideDurationMs = 6000;
	// 04GENERI.APC ("generique"=credits, music track 22 per the engine's
	// kMusicTrackNames table) is the authentic credits music but only
	// ships on a later CD — disc 1 just doesn't have it.  We try to load
	// it for completeness on multi-disc installs; on disc 1 the slideshow
	// runs in silence, which is exactly what the original game does there.
	// We DO NOT substitute a different track: the user reported that the
	// previous 33INTRO fallback fought with the still-running chapter
	// music and produced an obviously wrong soundscape.
	static const char *const kMusicApc = "04GENERI.APC";

	// Silence the chapter music first, regardless of whether the credits
	// APC loads — the original game's GereGameOver path stops sample
	// playback (FUN_0042b290 → _AIL_stop_sample_4) before drawing the
	// first credit slide.
	musicStop();

	// --- Music (credits track if present) ----------------------------
	Audio::SoundHandle musicHandle;
	bool musicPlaying = false;
	{
		Common::SeekableReadStream *apcFile =
		    openBigFileStream(kFileTypeSound, kMusicApc);
		if (apcFile) {
			Audio::PacketizedAudioStream *apc = Audio::makeAPCStream(*apcFile);
			if (apc) {
				int64 remaining = apcFile->size() - apcFile->pos();
				if (remaining > 0) {
					byte *buf = new byte[remaining];
					apcFile->read(buf, (uint32)remaining);
					apc->queuePacket(new Common::MemoryReadStream(
					    buf, remaining, DisposeAfterUse::YES));
				}
				apc->finish();
				_mixer->playStream(Audio::Mixer::kMusicSoundType,
				                   &musicHandle, apc);
				musicPlaying = true;
				debugC(1, kDebugScript, "playQuitOutro: %s playing", kMusicApc);
			}
			delete apcFile;
		} else {
			debugC(1, kDebugScript, "playQuitOutro: %s not on this disc (silent credits)",
			      kMusicApc);
		}
	}

	// --- Credits text -------------------------------------------------
	CreditsScript credits;
	{
		Common::ScopedPtr<Common::SeekableReadStream> txt(
		    openBigFileStream(kFileTypeSprite, "CREDITS.TXT"));
		if (txt)
			parseCreditsScript(*txt, credits);
	}

	// Centering matches FUN_0042073c (atlantis.exe @ 0x42073c):
	//   x = (0x3c0 - text_pixel_width) / 2
	// 0x3c0 = 960 is the original engine's text-area virtual width.  In
	// practice the CRED font lines are short enough that this places them
	// in the black band on the right of the slide art (artwork ends near
	// x≈340 per the per-column-brightness profile of CREDIT01.TGA).
	const int kCenterWidth = 0x3c0;

	// Vertical layout: stack lines with a step that fits the per-section
	// max line count (≈22 lines for the biggest sections) in 480 px, and
	// centre the block vertically.  Char height is ~18 in the original
	// SPR; 22 px step gives a comfortable readable gap.
	const int kLineStep = 22;

	showMouse(false);
	clearKeys();

	Graphics::PixelFormat fmt = g_system->getScreenFormat();

	bool skipped = false;
	for (uint i = 0; i < ARRAYSIZE(kSlides) && !skipped && !shouldAbort();
	     i++) {
		Graphics::Surface *slide = loadTGA(kFileTypeImages, kSlides[i]);
		if (!slide) {
			warning("playQuitOutro: missing slide '%s'", kSlides[i]);
			continue;
		}

		// Build the composite once: slide artwork + credit text overlay.
		Graphics::ManagedSurface frame;
		frame.create(slide->w, slide->h, fmt);
		frame.blitFrom(*slide);

		const Common::Array<CreditsLine> &lines = credits.sections[i];
		if (!lines.empty() && !_creditBlanSprites.empty()) {
			int totalH = (int)lines.size() * kLineStep;
			int y = (480 - totalH) / 2 + kLineStep;   // baseline of first line
			for (uint l = 0; l < lines.size(); l++) {
				const CreditsLine &cl = lines[l];
				int w = creditStringWidth(cl.text);
				int x = (kCenterWidth - w) / 2;        // authentic formula
				drawCreditText(frame, cl.text, x, y, cl.label);
				y += kLineStep;
				if (y > 478) break;
			}
		}

		g_system->copyRectToScreen(frame.getPixels(), frame.pitch,
		                            0, 0, frame.w, frame.h);
		g_system->updateScreen();

		slide->free();
		delete slide;

		// Per-slide wait, skippable.
		const uint32 deadline = g_system->getMillis() + kSlideDurationMs;
		while (g_system->getMillis() < deadline && !shouldAbort()) {
			pollEvents();   // pump engine-level queues

			// Direct event-manager poll for the skip signal.
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_KEYDOWN
				 || ev.type == Common::EVENT_LBUTTONDOWN
				 || ev.type == Common::EVENT_RBUTTONDOWN) {
					skipped = true;
					break;
				}
			}
			if (skipped)
				break;

			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}

	if (musicPlaying)
		_mixer->stopHandle(musicHandle);
	clearKeys();
}

// ---------------------------------------------------------------------------
// Warp-view cursor helpers
// ---------------------------------------------------------------------------

// Key color (transparent) for warp cursors: pure blue in RGB565.
// Not used in the warm Mediterranean/Atlantis palette.
static const uint16 kWarpCursorKey = 0x001Fu;

// Cursor group indices matching CURSEURS.SPR sprite groups.
enum WarpCursorState {
	kWarpCursorDefault  = 0,  // sprites 0–8   (20×20) — no interactive zone
	kWarpCursorNavigate = 1,  // sprites 9–17  (24×24) — place transition
	kWarpCursorObject   = 2,  // sprites 18–25 (32×32 crosshair) — object interaction
	kWarpCursorTalk     = 3   // sprites 26–45 (32×32 rotating)  — NPC interaction
};

// Fallback bitmask cursors used when CURSEURS.SPR is unavailable.
static void buildWarpCursorFallback(uint16 *out, const uint16 *rowMasks, uint16 color) {
	for (int r = 0; r < 16; r++) {
		uint16 mask = rowMasks[r];
		for (int c = 0; c < 16; c++)
			out[r * 16 + c] = (mask & (0x8000u >> c)) ? color : kWarpCursorKey;
	}
}
static const uint16 kCursorDefaultMask[16] = {
	0x8000, 0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00,
	0xE000, 0xC800, 0x8400, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000
};
static const uint16 kCursorNavigateMask[16] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0010, 0x0030, 0x0070, 0xFFF0,
	0xFFF8, 0xFFF0, 0x0070, 0x0030, 0x0010, 0x0000, 0x0000, 0x0000
};
static const uint16 kCursorTalkMask[16] = {
	0x0000, 0x1FF0, 0x2004, 0x2448, 0x2448, 0x2004, 0x2814, 0x27E4,
	0x2004, 0x1FF0, 0x0080, 0x0100, 0x0200, 0x0000, 0x0000, 0x0000
};

// Convert one RGB565 sprite pixel to straight-alpha ARGB8888 for a hardware
// cursor.  Opaque pixels become fully opaque; blend pixels carry a 5-bit
// factor and a premultiplied colour (atlantis.exe stores src*factor/32), so the
// colour is un-premultiplied and the factor becomes the alpha — ScummVM's
// cursor compositor uses straight alpha.
static uint32 spriteToArgb(uint16 p, uint8 factor, uint16 transpKey) {
	if (p == transpKey)
		return 0;  // fully transparent
	uint r = (p >> 11) & 0x1f;
	uint g = (p >>  5) & 0x3f;
	uint b =  p        & 0x1f;
	uint a = 0xff;
	if (factor != kSprNoBlend) {
		uint f = factor & 0x1f;
		if (f == 0)
			return 0;  // factor 0 == fully transparent edge pixel
		r = MIN<uint>(0x1f, r * 32 / f);
		g = MIN<uint>(0x3f, g * 32 / f);
		b = MIN<uint>(0x1f, b * 32 / f);
		a = f * 255 / 32;
	}
	uint r8 = (r << 3) | (r >> 2);
	uint g8 = (g << 2) | (g >> 4);
	uint b8 = (b << 3) | (b >> 2);
	return (a << 24) | (r8 << 16) | (g8 << 8) | b8;
}

void CryOmni3DEngine_Atlantis::setBlendedCursor(uint16 w, uint16 h,
        const uint16 *pixels, const uint8 *blend, int hotX, int hotY,
        uint16 transpKey) {
	const uint n = (uint)w * h;
	if (g_system->hasFeature(OSystem::kFeatureCursorAlpha)) {
		// ARGB8888 cursor: the backend alpha-composites it over the scene, so
		// anti-aliased edges blend the way the original game does.
		Graphics::PixelFormat fmt(4, 8, 8, 8, 8, 16, 8, 0, 24);
		Common::Array<uint32> buf(n);
		for (uint i = 0; i < n; i++)
			buf[i] = spriteToArgb(pixels[i],
			                      blend ? blend[i] : (uint8)kSprNoBlend, transpKey);
		CursorMan.replaceCursor(buf.data(), w, h, hotX, hotY, 0, false, &fmt);
	} else {
		// No alpha-cursor support: fall back to a key-colour RGB565 cursor.
		Graphics::PixelFormat fmt(2, 5, 6, 5, 0, 11, 5, 0, 0);
		Common::Array<uint16> buf;
		buf.assign(pixels, pixels + n);
		for (uint16 &p : buf)
			if (p == transpKey)
				p = kWarpCursorKey;
		CursorMan.replaceCursor(buf.data(), w, h, hotX, hotY, kWarpCursorKey,
		                        false, &fmt);
	}
}

void CryOmni3DEngine_Atlantis::setWarpCursor(int group) {
	_warpCursorGroup = group;
	_warpCursorFrame = -1;   // tickWarpCursor advances to 0 on first call
	_warpCursorNextMs = 0;   // trigger immediate update
	tickWarpCursor();
}

void CryOmni3DEngine_Atlantis::setItemCursor(const Object *obj, int overlayCursorFrame) {
	if (!obj || _objects.empty() || _objSprites.empty()) return;
	ptrdiff_t idx = obj - &_objects.front();
	if (idx < 0 || (uint)idx >= _objSprites.size()) return;
	const SubjSprite &spr = _objSprites[(uint)idx];
	if (spr.pixels.empty()) return;

	const bool overlay = overlayCursorFrame >= 0 &&
	    (uint)overlayCursorFrame < _cursorSprites.size() &&
	    !_cursorSprites[(uint)overlayCursorFrame].pixels.empty();

	if (!overlay) {
		// No action overlay: the object sprite alone is the cursor.
		setBlendedCursor(spr.w, spr.h, spr.pixels.data(), spr.blend.data(),
		                 spr.xoff, spr.yoff, kSprTransp);
		return;
	}

	// Carrying an object with an action available: composite the object
	// sprite with the action cursor drawn on top.  Both sub-sprites are
	// anchored by their own hotspot to a shared hotspot, so the result
	// behaves like the two cursors drawn at the same point.
	const CursorSpr &act = _cursorSprites[(uint)overlayCursorFrame];
	const int hx = MAX((int)spr.xoff, (int)act.xoff);
	const int hy = MAX((int)spr.yoff, (int)act.yoff);
	const int w  = hx + MAX((int)spr.w - spr.xoff, (int)act.w - act.xoff);
	const int h  = hy + MAX((int)spr.h - spr.yoff, (int)act.h - act.yoff);

	Common::Array<uint16> buf;
	Common::Array<uint8>  blendBuf;
	buf.resize((uint)(w * h), kSprTransp);
	blendBuf.resize((uint)(w * h), kSprNoBlend);

	const int ox = hx - spr.xoff, oy = hy - spr.yoff;   // object (base)
	for (int y = 0; y < (int)spr.h; y++)
		for (int x = 0; x < (int)spr.w; x++) {
			uint si = y * spr.w + x;
			if (spr.pixels[si] != kSprTransp) {
				uint di = (oy + y) * w + (ox + x);
				buf[di]      = spr.pixels[si];
				blendBuf[di] = spr.blend[si];
			}
		}
	const int ax = hx - act.xoff, ay = hy - act.yoff;   // action cursor (top)
	for (int y = 0; y < (int)act.h; y++)
		for (int x = 0; x < (int)act.w; x++) {
			uint si = y * act.w + x;
			if (act.pixels[si] != kWarpCursorKey) {
				uint di = (ay + y) * w + (ax + x);
				buf[di]      = act.pixels[si];
				blendBuf[di] = act.blend[si];
			}
		}
	setBlendedCursor((uint16)w, (uint16)h, buf.data(), blendBuf.data(),
	                 hx, hy, kSprTransp);
}

// The plain Atlantis pointer: CURSEURS.SPR sprite 8, the last (fully formed)
// frame of the default-cursor group.  This is the static arrow the original
// uses for the toolbar and the option menus — distinct from the animated
// navigation / interaction cursors.
void CryOmni3DEngine_Atlantis::setArrowCursor() {
	if (_cursorSprites.size() <= 8)
		return;
	const CursorSpr &spr = _cursorSprites[8];
	if (spr.pixels.empty())
		return;
	setBlendedCursor(spr.w, spr.h, spr.pixels.data(), spr.blend.data(),
	                 spr.xoff, spr.yoff, kWarpCursorKey);
}

void CryOmni3DEngine_Atlantis::tickWarpCursor() {
	static const int kGroupFirst[] = {  0,  9, 18, 26 };
	static const int kGroupCount[] = {  9,  9,  8, 20 };

	if (!_cursorSprites.empty()) {
		int grp = CLIP(_warpCursorGroup, 0, 3);

		// The default cursor — shown when the view is over plain scenery,
		// not a point of interest — is static in the original game; only the
		// navigate / object / talk cursors animate.  Cycling group 0 made it
		// visibly blink, so hold it on its first frame instead.
		if (grp == kWarpCursorDefault) {
			if (_warpCursorFrame == 0)
				return;  // already on the static frame
			_warpCursorFrame = 0;
		} else {
			uint32 now = g_system->getMillis();
			if (now < _warpCursorNextMs) return;
			_warpCursorNextMs = now + 80;  // ~12 fps
			_warpCursorFrame = (_warpCursorFrame + 1) % kGroupCount[grp];
		}

		const CursorSpr &spr = _cursorSprites[kGroupFirst[grp] + _warpCursorFrame];
		if (!spr.pixels.empty())
			setBlendedCursor(spr.w, spr.h, spr.pixels.data(), spr.blend.data(),
			                 spr.xoff, spr.yoff, kWarpCursorKey);
		return;
	}

	// Fallback: hardcoded bitmask cursors when CURSEURS.SPR is not loaded.
	if (_warpCursorFrame == 0) return;  // already set, avoid redundant update
	_warpCursorFrame = 0;
	// Stack-local scratch buffer (16×16 RGB565 = 512 bytes).  Was previously
	// declared `static` for no good reason — that made the engine non-
	// re-entrant per ScummVM's engine guidelines.
	uint16 buf[16 * 16];
	Graphics::PixelFormat fmt(2, 5, 6, 5, 0, 11, 5, 0, 0);
	switch (_warpCursorGroup) {
	case 1:
		buildWarpCursorFallback(buf, kCursorNavigateMask, 0xFFE0u);
		CursorMan.replaceCursor(buf, 16, 16, 0, 7, kWarpCursorKey, false, &fmt);
		break;
	case 2:
		// Object interaction — no dedicated bitmask; use white default arrow
		buildWarpCursorFallback(buf, kCursorDefaultMask, 0xFFFFu);
		CursorMan.replaceCursor(buf, 16, 16, 0, 0, kWarpCursorKey, false, &fmt);
		break;
	case 3:
		// NPC interaction — cyan face+bubble
		buildWarpCursorFallback(buf, kCursorTalkMask, 0x07FFu);
		CursorMan.replaceCursor(buf, 16, 16, 7, 4, kWarpCursorKey, false, &fmt);
		break;
	default:
		buildWarpCursorFallback(buf, kCursorDefaultMask, 0xFFFFu);
		CursorMan.replaceCursor(buf, 16, 16, 0, 0, kWarpCursorKey, false, &fmt);
		break;
	}
}

int CryOmni3DEngine_Atlantis::handleWarp() {
	bool leftButtonPressed = false;
	bool exit = false;
	bool firstDraw = true;
	bool moving = true;
	uint actionId = 0;

	showMouse(true);
	_canLoadSave = true;

	// FPS mode: center cursor so the first delta is zero.
	setMousePos(Common::Point(320, 240));

	WarpCursorState cursorState = kWarpCursorDefault;
	const Object *cursorHeldObj = nullptr;   // object whose cursor is shown
	int cursorOverlayFrame = 0;              // animated action-overlay frame
	uint32 cursorOverlayNextMs = 0;
	if (_inventory.selectedObject() == nullptr)
		setWarpCursor(cursorState);

	// Drain stale clicks from navigation or window-focus events.
	// Ignore input for ~100ms after entering a place (5 × 20ms) to consume focus clicks.
	for (int grace = 0; grace < 5 && !shouldAbort(); grace++) {
		pollEvents();
		g_system->delayMillis(20);
	}
	while (getCurrentMouseButton() != 0 && !shouldAbort()) {
		pollEvents();
		g_system->delayMillis(10);
	}
	setMousePos(Common::Point(320, 240));

	// Lock the cursor into SDL relative mode for the look.  warpMouse cannot
	// re-centre the pointer on Wayland (WSL), so reading an offset-from-centre
	// would make the camera scroll until the cursor crossed the centre.
	// Relative mode delivers pure per-frame motion (event.relMouse) instead.
	g_system->lockMouse(true);
	takeMouseRelative();   // discard motion accumulated before entry

	// FPS mouse-look sensitivity — atlantis.exe FUN_0041bfa0.  The exe runs a
	// momentum integrator   speed += delta / K;  angle += speed;  speed *= 0.8
	// with K = 3000 (yaw) and K = 4000 (pitch), in an uncapped render-bound
	// loop.  On any modern-speed machine the 0.8 decay collapses within a
	// frame, so there is no perceptible inertia — the net rotation of a
	// gesture is just its total delta / (K * (1 - 0.8)).  Reproduce that as a
	// direct mapping: 3000*0.2 = 600 px/rad yaw, 4000*0.2 = 800 px/rad pitch.
	static const double kLookYawDiv    = 600.0;
	static const double kLookPitchDiv  = 800.0;
	static const double kWarpBetaLimit = M_PI / 4.0;  // matches doPlaceChange

	while (!leftButtonPressed && !exit) {
		pollEvents();

		// FPS mouse-look: this frame's accumulated relative motion.
		//
		// SDL reports relative-mouse motion as right = positive on every
		// standard backend — verified on the native Windows build, where
		// relMouse.x came back positive for a rightward move.  WSLg's
		// Wayland relative-pointer is the lone exception: it inverts the X
		// axis (Y is consistent everywhere).  The Linux build runs solely
		// under WSLg, so flip X there; the Windows build takes it as-is.
		// xDelta must end up positive for a rightward look — the rotation
		// below subtracts it from alpha.
		Common::Point lookRel = takeMouseRelative();
#if defined(WIN32)
		int xDelta = lookRel.x;
#else
		int xDelta = -lookRel.x;   // compensate WSLg's inverted Wayland X
#endif
		int yDelta = -lookRel.y;

		// Pin the crosshair to the exact screen centre.  Every action check
		// (navigation zones, NPC dialog) is hit-tested at the centre, so the
		// cursor must stay there; discard the motion the re-centre produces.
		setMousePos(Common::Point(320, 240));
		takeMouseRelative();

		leftButtonPressed = (getCurrentMouseButton() == 1);

		// Hit-test panorama at the screen centre — that is always where the
		// crosshair/cursor sits in FPS mode.
		actionId = 0;
		Common::Point pan;
		bool havePan = false;
		if (_currentPlace && _warpLoaded) {
			pan = _omni3dMan.mapMouseCoords(Common::Point(320, 240));
			pan.y = 768 - pan.y; // flip Y to top-left origin
			havePan = true;
			int16 hit = _currentPlace->hitTest(pan.x, pan.y);
			if (hit > 0)
				actionId = (uint)hit;
		}

		// Update cursor shape based on what the centre is pointing at:
		// an NPC (talk), an object hotspot (converging-arrows), a
		// navigation zone (navigate), or nothing.
		//
		// A zone's action is an Object hotspot iff (a) some ConSection's
		// clicZone explicitly matches it (e.g. `/con*(cliczone=499)`) OR
		// (b) it isn't a real navigation destination at all -- neither a
		// place in the current WAM nor a sub-WAM destination.  Case (b)
		// catches puzzle-launcher sentinels like CHAPI012 vue 221's
		// action=1000, where the /sel that fires is a generic
		// `(vue=N)&(flagtemp6=0)` without a cliczone clause and the
		// action value is just a hotspot trigger, not a real place ID --
		// without case (b), the cursor would say "go there" over a
		// hotspot that does NOT navigate.
		{
			WarpCursorState action = kWarpCursorDefault;
			if (havePan) {
				int dummy = 0;
				if (npcPanoHitTest(pan, dummy)) {
					action = kWarpCursorTalk;
				} else if (actionId > 0) {
					bool isClicZone = false;
					for (const ConSection &sec : _conScript.sections()) {
						if (sec.clicZone == (int)actionId) {
							isClicZone = true;
							break;
						}
					}
					bool isNavTarget = (_wam.findPlaceById((uint16)actionId) != nullptr);
					if (!isNavTarget) {
						for (const AtlantisSubWAM &sub : _wam.getSubWAMs()) {
							if (sub.dstPlaceId == (uint16)actionId) {
								isNavTarget = true;
								break;
							}
						}
					}
					action = (isClicZone || !isNavTarget)
					         ? kWarpCursorObject : kWarpCursorNavigate;
				}
			}

			// CURSEURS.SPR action-cursor group for the carried-object overlay:
			// navigate is frames 9..17, object is 18..25, talk is 26..45.
			// ovFirst < 0 = none.
			int ovFirst = -1, ovCount = 0;
			if (action == kWarpCursorTalk)          { ovFirst = 26; ovCount = 20; }
			else if (action == kWarpCursorObject)   { ovFirst = 18; ovCount =  8; }
			else if (action == kWarpCursorNavigate) { ovFirst =  9; ovCount =  9; }

			const Object *heldObj = _inventory.selectedObject();
			// Navigation is impossible while an object is in hand (see the
			// gameStep nav gate), so don't overlay the navigate arrows on the
			// held-item cursor -- show the plain item icon instead.  The
			// talk/use overlays stay: using the item on an NPC or hotspot is
			// still valid.
			if (heldObj && action == kWarpCursorNavigate) {
				ovFirst = -1;
				ovCount = 0;
			}
			if (heldObj != cursorHeldObj || action != cursorState) {
				// Held object or action changed: rebuild and restart the
				// overlay animation.
				cursorHeldObj = heldObj;
				cursorState = action;
				cursorOverlayFrame = 0;
				cursorOverlayNextMs = g_system->getMillis() + 80;
				if (heldObj)
					setItemCursor(heldObj, ovFirst);
				else
					setWarpCursor(action);
			} else if (heldObj && ovFirst >= 0 &&
			           g_system->getMillis() >= cursorOverlayNextMs) {
				// Animate the overlaid action cursor so it looks like the
				// normal-navigation cursor, which tickWarpCursor() animates.
				cursorOverlayFrame = (cursorOverlayFrame + 1) % ovCount;
				cursorOverlayNextMs = g_system->getMillis() + 80;
				setItemCursor(heldObj, ovFirst + cursorOverlayFrame);
			}
		}

		// Log every left-click with the panorama hit-test result.
		if (leftButtonPressed)
			debugC(5, kDebugScript, "handleWarp: left-click pan=(%d,%d) havePan=%d hitActionId=%u",
			      pan.x, pan.y, (int)havePan, actionId);

		// Generic /con scan -- mirror of atlantis.exe FUN_00425cf4, which the
		// original engine runs before perso / zone dispatch whenever the click
		// action lands in the object id range.  Handles "use held object in
		// the world at a specific state" scripts that lack ClicPerso /
		// ClicZone gates (CHAPI013 sections 178 + /014 — "throw the pot on
		// the priest aide"): when the player holds object 261 and the camera
		// + animation frame land in the right window, a click anywhere fires
		// the section and consumes the click.  evalSelSection's conditions
		// are tight enough that this can't fire unintentionally.
		if (leftButtonPressed && havePan) {
			// Dump the live state the scan will see so a "throw missed"
			// failure can be reconciled against the section conditions.
			Object *heldObj = _inventory.selectedObject();
			debugC(1, kDebugScript, "leftclick: vue=%u anglewarpx=%d anglewarpy=%d "
			      "wsprframe=%d objetenmain=%d",
			      _currentPlaceId,
			      _gameVars["anglewarpx"],
			      _gameVars["anglewarpy"],
			      _gameVars["wsprframe"],
			      (heldObj && heldObj->idOBJ() != uint(-1))
			          ? (int)heldObj->idOBJ() : 0);
			// Gate the generic scan: atlantis.exe FUN_004278a0 only runs
			// FUN_00425cf4 when the click action lands in the object id
			// range [256, 512) OR with a control flag that empirically
			// tracks "an object is in hand".  Every generic /con section
			// in the data carries (ObjetEnMain=N) anyway, so condition on
			// "something is held" -- this prevents a stray click on the
			// panorama from triggering an unintended section, and also
			// keeps NPC clicks (action >= 512) going straight to
			// interactNPC without the scan stealing them.
			const bool scanEligible = (heldObj != nullptr);
			if (scanEligible && runGenericConSections()) {
				leftButtonPressed = false;
				actionId = 0;
				if (shouldAbort()) {
					actionId = 66666;
					exit = true;
				} else if (_nextPlaceId != uint(-1)) {
					// /set(Vue=N) fired inside the section -- hand the
					// destination to gameStep just like the NPC-dialog path.
					uint dest = _nextPlaceId;
					_canLoadSave = false;
					showMouse(false);
					return (int)dest;
				}
				// Drain the click then continue the loop; cursor will resync
				// on the next iteration via cursorHeldObj == nullptr below.
				cursorHeldObj = nullptr;
				while (getCurrentMouseButton() != 0 && !shouldAbort()) {
					pollEvents();
					g_system->delayMillis(10);
				}
				continue;
			}
		}

		// NPC sprite click takes priority over WAM zone navigation.
		// A guard or NPC standing in front of a zone must remain clickable.
		if (leftButtonPressed && havePan) {
			int clickedPerso = 0;
			if (npcPanoHitTest(pan, clickedPerso)) {
				leftButtonPressed = false;
				actionId = 0; // consume the action; suppress zone navigation below
				// Object pickup branch (atlantis.exe FUN_0042785c lines
				// 21720-21758): when the clicked sprite's "persoId" is
				// actually an OBJECT id (< 0x200 = 512; real NPC ids
				// start at 512), the engine adds the object to the
				// inventory list, hides the sprite from the cyclo, and
				// selects the object as held -- the canonical case is
				// CHAPI013 vue 126's flower pot (action=261, object id
				// 261, perso id 261 of the apotf126 sprite).
				if (clickedPerso > 0 && clickedPerso < 0x200) {
					Object *obj = nullptr;
					for (Object &o : _objects)
						if (o.valid() && o.idOBJ() == (uint)clickedPerso) {
							obj = &o; break;
						}
					if (obj) {
						// Replicate atlantis.exe FUN_0042785c (lines
						// 21717-21758): every world-pickup with action id
						// < 0x200 (= 512) is written to the inventory slot
						// array at &DAT_006ce138 *and* selected as held.
						// One unified path -- no per-object "transient"
						// special-casing.  Items that are throw-and-forget
						// (CHAPI013's pot 261) leave the inventory via the
						// throw script's /set(invent261=0); items the
						// player keeps (CHAPI014's knife 262) stay until
						// used elsewhere.  Empirically pot 261 and knife
						// 262 are both entries in the exe's inventory
						// table at 0x004967b0, so neither is special at
						// the engine level.
						if (!_inventory.inInventoryByNameID((uint)clickedPerso))
							_inventory.add(obj);
						_inventory.setSelectedObject(obj);
						// Deactivate every sprite entry for this perso AND
						// add it to _hiddenPersos -- mirror the ShowPerso /
						// HidePerso wiring exactly.  Without the entry.active
						// flip, recompositeSpriteLayer keeps drawing the
						// picked-up sprite until a place change or
						// compositeNPCSprites runs again.
						bool found = false;
						for (int h : _hiddenPersos)
							if (h == clickedPerso) { found = true; break; }
						if (!found)
							_hiddenPersos.push_back(clickedPerso);
						for (SpritePlaceEntry &e : _spritePlaceList)
							if (e.persoId == clickedPerso) e.active = false;
						_spriteLayerDirty = true;
						_npcBoundsDirty   = true;
						cursorHeldObj = nullptr;   // force cursor re-sync
						debugC(1, kDebugScript, "object pickup: picked up id=%d "
						         "(added to inventory + selected)",
						         clickedPerso);
						// Drain the click then continue; no NPC dialog.
						while (getCurrentMouseButton() != 0 && !shouldAbort()) {
							pollEvents();
							g_system->delayMillis(10);
						}
						continue;
					}
					// Perso id < 512 but NOT a pickable inventory object --
					// it's a "use on this thing" target (rope at vue 158
					// perso 277, palace traps at 194/215, etc.).  Atlantis.exe
					// FUN_004278a0 still dispatches the action through the
					// generic /con scan path so /con(ClicPerso=N) sections
					// like CHAPI015 `/203 /con*(ClicPerso=277)&(ObjetEnMain=
					// 262)` (cut the rope with the knife) can fire.  Route
					// through interactNPC -- it sets clicperso and walks the
					// /con section list with the same condition gate the
					// NPC-dialog path uses; sections without /suj fire
					// directly with no menu, exactly the exe count==0 branch.
					// Fall through to the standard interactNPC handler below.
				}
				// Dialog (and its subject menu) needs a normal free cursor.
				g_system->lockMouse(false);
				// Was an object being used on this NPC?  Capture before the
				// /con section plays.  Only real NPCs (id >= 0x200) go through
				// the exe's NPC handler; the perso<0x200 "use on a world thing"
				// path (rope/traps) is handled by a different exe branch that
				// does NOT auto-clear -- those sections manage the item.
				const bool usedObjectOnNpc =
				    clickedPerso >= 0x200 && _inventory.selectedObject() &&
				    _inventory.selectedObject()->idOBJ() != uint(-1);
				interactNPC(clickedPerso);
				// atlantis.exe FUN_00425828: when an NPC is clicked with an
				// object in hand, the handler runs the /con scan (use object on
				// NPC) and then UNCONDITIONALLY clears the held slot
				// (*(int *)(... + 0x611210) = 0), so the item returns to the
				// inventory.  Mirror that.
				if (usedObjectOnNpc) {
					_inventory.setSelectedObject(nullptr);
					_gameVars["objetenmain"] = 0;
				}
				// The dialog is over: stop showing any "object received" icon
				// (the original clears DAT_0049619c when the dialog restores the
				// navigation FOV).
				_dialogGivenObjSprite = -1;
				if (shouldAbort()) {
					actionId = 66666;
					exit = true;
				} else if (_nextPlaceId != uint(-1)) {
					// Dialog triggered a place transition (/set(Vue=N) inside
					// playConSection).  Hand it off to gameStep, BUT leave
					// _nextPlaceId set: gameStep distinguishes a script-set
					// destination from a user click by `_nextPlaceId == actionId`
					// and bypasses the /con and /sel departure scans in that
					// case — those are gates on player-driven navigation, not on
					// scripted scene changes.  Without this, CHAPI011 section 128's
					// /set(Vue=50) (Creon dismissing Seth back to vue 50) is
					// caught by section 10's throne-room lock /sel(departvue>0)&
					// (vue>274)&(vue<300) and the abortdepart cancels the move.
					uint dest = _nextPlaceId;
					_canLoadSave = false;
					showMouse(false);
					return (int)dest;
				} else {
					// Dialog playback hides the cursor — restore it so the
					// warp cursor is visible again over points of interest.
					showMouse(true);
					// Item may have been used/deselected; ensure cursor is updated.
					if (_inventory.selectedObject() == nullptr) {
						cursorState = kWarpCursorDefault;
						setWarpCursor(cursorState);
					}
					cursorHeldObj = nullptr;   // force the cursor block to re-sync
					// Drain button release, then re-arm relative look mode.
					while (getCurrentMouseButton() != 0 && !shouldAbort()) {
						pollEvents();
						g_system->delayMillis(10);
					}
					setMousePos(Common::Point(320, 240));
					g_system->lockMouse(true);
					takeMouseRelative();
					firstDraw = true;
				}
			}
		}

		if (shouldAbort()) {
			actionId = 66666;
			exit = true;
		}

		// Drain one keypress per frame; route Space → toolbar, Escape →
		// in-game menu.  All other keys are discarded by design (the
		// underlying queue continues to fill from pollEvents()).
		Common::KeyCode kc = getNextKey().keycode;
		bool toolbarTrigger = (getCurrentMouseButton() == 2 ||
		                       kc == Common::KEYCODE_SPACE);
		bool menuTrigger = (kc == Common::KEYCODE_ESCAPE);

		if (!exit && toolbarTrigger) {
			if (_warpLoaded) {
				const Graphics::Surface *bg = _omni3dMan.getSurface();
				g_system->copyRectToScreen(bg->getPixels(), bg->pitch, 0, 0, bg->w, bg->h);
			}
			g_system->lockMouse(false);   // toolbar needs a normal cursor
			// The toolbar uses the plain Atlantis arrow cursor — unless an
			// object is being carried, in which case the cursor stays in
			// "object" mode so you can see what you hold while the bar is
			// open.  (The toolbar's own setCursor() calls are inert: Atlantis
			// never populates the base _sprites collection.)
			if (_inventory.selectedObject() != nullptr)
				setItemCursor(_inventory.selectedObject());
			else
				setArrowCursor();
			displayToolbar(_warpLoaded ? _omni3dMan.getSurface() : nullptr);
			if (shouldAbort()) {
				actionId = 66666;
				exit = true;
			}
			// If item was deselected in toolbar, restore warp cursor.
			if (_inventory.selectedObject() == nullptr) {
				cursorState = kWarpCursorDefault;
				setWarpCursor(cursorState);
			}
			cursorHeldObj = nullptr;   // force the cursor block to re-sync
			// Re-arm relative look mode after the toolbar.
			setMousePos(Common::Point(320, 240));
			g_system->lockMouse(true);
			takeMouseRelative();
			firstDraw = true;
		}

		// Escape: open the in-game menu.  displayInGameMenu() runs its own
		// non-blocking sub-loop (pollEvents + updateScreen + delayMillis(10)
		// per iteration, shouldAbort tested every tick) so the host stays
		// responsive.  Its return value uses the same codes as displayOptions
		// so the engine's existing abort state machine routes the action.
		if (!exit && menuTrigger) {
			showMouse(true);
			g_system->lockMouse(false);   // in-game menu needs a normal cursor
			uint act = displayInGameMenu();
			switch (act) {
			case 28:  // Load game
				actionId = 66666;
				_abortCommand = kAbortLoadGame;
				exit = true;
				break;
			case 27:  // New game
				actionId = 66666;
				_abortCommand = kAbortNewGame;
				exit = true;
				break;
			case 40:  // Quit — clean GMM-friendly path.
				actionId = 66666;
				_abortCommand = kAbortQuit;
				exit = true;
				break;
			default:  // Continue / save / cancelled — resume warp.
				break;
			}
			// Re-arm relative look mode and force redraw so the menu
			// snapshot is not blitted as the panorama next frame.
			setMousePos(Common::Point(320, 240));
			g_system->lockMouse(true);
			takeMouseRelative();
			firstDraw = true;
		}

		// zoom=1 (CON) — narrow the panorama FOV for a close-up.  The original
		// (atlantis.exe FUN_004212cc) ramps the warp FOV by -3 degrees/frame
		// via FUN_0045179a while var[zoom] is set; here it is time-based
		// (~30 deg/s) and clamped, holding once the close-up FOV is reached.
		// doPlaceChange() restores the 75 degrees FOV and clears the flag.
		{
			Common::HashMap<Common::String, int>::iterator zit = _gameVars.find("zoom");
			if (_warpLoaded && zit != _gameVars.end() && zit->_value != 0) {
				const double kZoomTargetFov = 40.0 / 180.0 * M_PI;
				double hf = _omni3dMan.getHFov();
				uint32 now = g_system->getMillis();
				if (hf > kZoomTargetFov && _zoomLastMs != 0) {
					double step = (30.0 / 180.0 * M_PI) * (now - _zoomLastMs) / 1000.0;
					hf = (hf - step < kZoomTargetFov) ? kZoomTargetFov : hf - step;
					_omni3dMan.setHFov(hf);
					firstDraw = true;
				}
				_zoomLastMs = now;
			} else {
				_zoomLastMs = 0;
			}
		}

		if (_warpLoaded && (firstDraw || xDelta || yDelta)) {
			// Direct mouse-look (atlantis.exe FUN_0041bfa0).  The exe's
			// delta is prevCentre - cursor = -xDelta; yaw uses 600 px/rad,
			// pitch 800 px/rad (see note above the loop).
			double alpha = _omni3dMan.getAlpha() - xDelta / kLookYawDiv;
			// OMNI 3D "MODE 1" (options menu) inverts the vertical axis.
			int pitch = omni3dInvertY() ? -yDelta : yDelta;
			double beta = _omni3dMan.getBeta() + pitch / kLookPitchDiv;

			while (alpha >= 2.0 * M_PI) alpha -= 2.0 * M_PI;
			while (alpha < 0.0)         alpha += 2.0 * M_PI;
			if (beta < -kWarpBetaLimit) beta = -kWarpBetaLimit;
			if (beta >  kWarpBetaLimit) beta =  kWarpBetaLimit;

			_omni3dMan.setAlpha(alpha);
			_omni3dMan.setBeta(beta);
			// Keep angle vars in sync for CON conditions (AngleWarpX<N).
			_gameVars["anglewarpx"] = (int)(alpha * 180.0 / M_PI);
			_gameVars["anglewarpy"] = (int)(beta  * 180.0 / M_PI);
			const Graphics::Surface *result = _omni3dMan.getSurface();
			if (result)
				g_system->copyRectToScreen(result->getPixels(), result->pitch, 0, 0, result->w, result->h);
			if (!exit)
				g_system->updateScreen();
			moving = true;
			firstDraw = false;
		} else if (moving) {
			if (_warpLoaded) {
				const Graphics::Surface *result = _omni3dMan.getSurface();
				if (result)
					g_system->copyRectToScreen(result->getPixels(), result->pitch, 0, 0, result->w, result->h);
			}
			if (!exit)
				g_system->updateScreen();
			moving = false;
		} else {
			if (!exit)
				g_system->updateScreen();
		}

		if (_inventory.selectedObject() == nullptr)
			tickWarpCursor();

		// Advance animated prop sprite if the interval has elapsed.
		// Mirror the new frame into _gameVars["wsprframe"] so script
		// conditions like `/con(...)&(Wsprframe>7)&(Wsprframe<34)`
		// (CHAPI013 line 92, the priest-aide bottle-use action at
		// vue 126) see the live animation frame -- the script only
		// fires the action when the perso's idle cycle is in the
		// right pose window.  Mirrors the wspranim path's wsprframe
		// hijack via atlantis.exe FUN_00425304 (the propAnim and
		// wspranim share the same gate slot at runtime).
		if (_propAnimFrames > 1 && g_system->getMillis() >= _propAnimNextMs) {
			_propAnimFrame = (_propAnimFrame + 1) % (uint)_propAnimFrames;
			_propAnimNextMs = g_system->getMillis() + kPropAnimIntervalMs;
			_gameVars["wsprframe"] = (int)_propAnimFrame;
			recompositeSpriteLayer();
			_omni3dMan.setSourceSurface(&_warpSurface);
			firstDraw = true;  // force Omni3D redraw with updated source
		}

		// Advance the active wspranim sprite, mirror its frame counter
		// into the script's WSPRFRAME variable, and then re-evaluate /Tim2
		// sections immediately — this is what makes `WSprFrame=31` (an
		// exact equality on one frame out of 33) actually fire in CHAPI012.
		//
		// atlantis.exe FUN_00425304 (the per-frame sprite renderer) does
		// two things on each render iteration: (1) hijacks the timer-2
		// gate at 0x611210[1] by overwriting it with the active sprite's
		// current frame counter (line 20086 in the decompile), and (2)
		// advances the frame counter if more than 0xd ms (13 ms) have
		// passed since the last advance.  The next iteration's main-loop
		// call to FUN_0042be3c then scans every /Tim2 section with that
		// hijacked value in scope — so conditions like `(WSprFrame=31)`
		// resolve against the *current* sprite frame, not against the
		// 1-Hz time2 counter.  Without re-running the /Tim2 scan on each
		// wspranim frame change, the equality is essentially never caught:
		// frame 31 only exists for ~30 ms, and a 1-Hz time2 tick is
		// unlikely to land inside that window — the dialog only fires
		// later when a place change runs the fallback
		// /sel(departvue>0)&(Flagtemp3=1) on line 29 of CHAPI012.
		//
		// We use 30 ms here (~33 fps wspranim → ~1 s 33-frame walk) rather
		// than the exe's 13 ms threshold: ScummVM's main loop iterates
		// roughly every 15 ms, so a 13 ms threshold would advance nearly
		// every iteration and the walk would finish in ~500 ms, faster
		// than the original game appeared on period hardware.
		if (_wsprAnimActiveIdx >= 0 && g_system->getMillis() >= _wsprAnimNextMs) {
			_wsprAnimNextMs = g_system->getMillis() + kWsprAnimIntervalMs;
			int seen = 0;
			for (SpritePlaceEntry &e : _spritePlaceList) {
				if (e.placeId != (uint16)_currentPlaceId) continue;
				if (seen != _wsprAnimActiveIdx) { seen++; continue; }
				e.frame++;
				_gameVars["wsprframe"] = e.frame;
				_spriteLayerDirty = true;
				// Mirror FUN_0042be3c's per-frame /Tim2 scan with the
				// just-advanced wsprframe value in scope.  Gated on
				// testtime2 the same way the regular 1-Hz scan is (a
				// section can opt out of polling by setting testtime2=0).
				Common::HashMap<Common::String, int>::const_iterator t2 =
				    _gameVars.find("testtime2");
				if (t2 != _gameVars.end() && t2->_value != 0)
					runTimerSections(2);
				// Re-find the entry: runTimerSections may have changed
				// chapter (e.g. /set(chapter=N) from a /go169 trigger),
				// which calls loadConScript and rebuilds _spritePlaceList.
				if (shouldAbort() || _nextPlaceId != uint(-1))
					break;
				// Wrap check happens AFTER the scan so the terminal frame
				// value is observable to /Tim2 conditions one tick before
				// the counter resets (matching the exe's ordering, where
				// the gate is set from the pre-advance frame).
				Common::SeekableReadStream *spw =
				    openBigFileStream(kFileTypeSpriteCyclo, e.spwFile);
				if (spw) {
					spw->seek(0x04);
					uint32 nFrames = spw->readUint32LE();
					delete spw;
					if (nFrames > 0 && (uint)e.frame >= nFrames) {
						e.frame = 0;
						// type=2 "play once then disable" — exe sets
						// sprite.field+0x04 = 0 at the wrap.
						if (e.type == 2) {
							e.active = false;
							_wsprAnimActiveIdx = -1;
						}
						_gameVars["wsprframe"] = 0;
					}
				}
				break;
			}
		}

		// A CON command changed the panorama sprite layer while it was off
		// screen (e.g. mid-dialog).  Rebuild the NPC click-target list if a
		// showperso/hideperso changed visibility, then recomposite and force
		// a redraw — so the change shows without waiting for a place change.
		if (_npcBoundsDirty) {
			_npcBoundsDirty = false;
			rebuildNpcPlaceBounds();
		}
		if (_spriteLayerDirty) {
			_spriteLayerDirty = false;
			recompositeSpriteLayer();
			_omni3dMan.setSourceSurface(&_warpSurface);
			firstDraw = true;
		}

		// Tick auto-incrementing timers and poll /tim* sections.
		//
		// time1 and time2 are free-running counters that increment on EVERY
		// tick interval, regardless of testtime1/testtime2.  The script
		// resets them with /set(timeN=0) before any /sel or /tim section
		// that reads them, and conditions like `/Tim1(time1>10000)` or
		// `/Sel(...&time2>25)` only make sense if the counter accumulates
		// continuously (CHAPI011 section 158 sets time2=0 alone — without
		// any testtime2=1 — yet section 159's /sel(...&time2<5) waiting
		// lock is meant to lift after ~5 seconds, which only works if
		// time2 ticks anyway).
		//
		// testtimeN gates ONLY /tim* section polling — i.e. whether the
		// engine bothers to scan /timN sections each tick.  The /tim
		// sections themselves carry their own (Vue=...), (FlagX=...)
		// guards, so when the script switches testtimeN on, the relevant
		// /tim section starts firing on every tick whose conditions hold.
		uint32 nowMs = g_system->getMillis();
		for (int t = 0; t < 2 && !shouldAbort() && _nextPlaceId == uint(-1) && !exit; t++) {
			int timerNum = t + 1;
			const char *testKey = (timerNum == 1) ? "testtime1" : "testtime2";
			const char *timeKey = (timerNum == 1) ? "time1"     : "time2";
			if (nowMs < _timerTickNextMs[t]) continue;
			_timerTickNextMs[t] = nowMs + kTimerTickIntervalMs;
			_gameVars[timeKey]++;
			debugC(2, kDebugScript, "timer%d tick: %s=%d wsprframe=%d", timerNum, timeKey,
			      _gameVars[timeKey], _gameVars.contains("wsprframe") ? _gameVars["wsprframe"] : 0);
			Common::HashMap<Common::String, int>::const_iterator ti = _gameVars.find(testKey);
			if (ti != _gameVars.end() && ti->_value != 0)
				runTimerSections(timerNum);
		}

		g_system->delayMillis(10);
	}

	_canLoadSave = false;
	g_system->lockMouse(false);
	showMouse(false);
	return (int)actionId;
}

// ---------------------------------------------------------------------------
// CON script loading and subject state
// ---------------------------------------------------------------------------

// CON variables live in one global array (VARIAS.CON line order).  Only
// indices 0-6 are persistent — Chapter, GardeCel, NewSound, FlagChp8,
// BoucleCreon, CodeTroneVu, NbStatues; they are written to the save and
// survive a chapter change.  Indices 7+ are transient: atlantis.exe's
// FUN_00427634 zeroes them on every `chapter=` (for i=7; i<0x80; var[i]=0).
bool CryOmni3DEngine_Atlantis::isPersistentGameVar(const Common::String &key) {
	static const char *const kPersistent[] = {
		"chapter", "gardecel", "newsound", "flagchp8",
		"bouclecreon", "codetronevu", "nbstatues"
	};
	for (const char *p : kPersistent)
		if (key.equalsIgnoreCase(p))
			return true;
	return false;
}

void CryOmni3DEngine_Atlantis::loadConScript(int chapter) {
	// Stamp the wall-clock so the `chaptertime` condition (seconds since the
	// active CHAPI*.CON loaded) can be evaluated dynamically.  CHAPI030
	// section 65 (`/Con(ClicPerso=551)&(Flagtemp5=0)&(ChapterTime>120)`)
	// and section 69 (`/sel(...)&(ChapterTime>20)`) read this -- before, both
	// always failed because the var stayed at 0.  Reset on every chapter
	// switch so the clock matches what the original engine sees.
	_chapterStartMs = g_system->getMillis();
	_gameVars["chaptertime"] = 0;

	_conScript.reset();
	_sujEnabled.clear();
	// A chapter change keeps only the 7 persistent CON variables; the
	// transient ones (broche30, parletemp*, flagtemp*, vue, ...) reset to 0.
	{
		Common::HashMap<Common::String, int> persist;
		for (const auto &kv : _gameVars)
			if (isPersistentGameVar(kv._key))
				persist[kv._key] = kv._value;
		_gameVars = persist;
	}

	// The hidden-perso list PERSISTS across chapter switches -- it is the
	// exe's persistent list at 0x611010 (see execConSetCommand), distinct
	// from the per-chapter CON variable array that FUN_00427634 zeroes.  A
	// chapter's /INIT block carries only the DELTAS (the persos it newly
	// shows/hides), not a complete re-statement: CHAPI001 hides ~42 persos,
	// CHAPI002 re-lists only ~18 of them.  Clearing here made the ~24 left
	// out of CHAPI002's INIT (279/280/281/524/531-538/565-582/587/591/595)
	// pop back into view every time the script reloaded.  The concrete bug
	// was perso 587 (Anaan): hidden by CHAPI001, never re-shown until
	// CHAPI056, but visible+clickable at vue 110 in chapters 17-24 (its SPW
	// entry has startActive=true), producing "interactNPC: no options found".
	// Meljanz (617) -- the case the old clear was meant to fix -- is shown in
	// CHAPI012 via /set(NewWsprAnim=2), which activates the sprite by index
	// (bypassing _hiddenPersos), so persisting the hide does not hide him.
	// A genuine new game still starts clean: changeChapter() clears the list
	// before the first loadConScript, and loadGame restores the saved list.
	// Guests are chapter-scoped: a follower from chapter N doesn't leak
	// into chapter N+1 -- the new chapter's INIT block re-establishes
	// who's around.  Clear before the script loads so /set(addguest=N)
	// in the new /INIT lands in a clean list.
	_guests.clear();
	_spriteLayerDirty = true;
	_npcBoundsDirty   = true;

	Common::String name = Common::String::format("CHAPI%03d.CON", chapter);
	Common::SeekableReadStream *s = openBigFileStream(kFileTypeScript, name);
	if (!s) {
		debugC(1, kDebugScript, "loadConScript: no CON file '%s'", name.c_str());
		return;
	}

	uint32 sz = (uint32)s->size();
	byte *buf = new byte[sz + 1];
	s->read(buf, sz);
	buf[sz] = '\0';
	delete s;

	_conScript.parse((const char *)buf, sz);
	delete[] buf;

	execConInitCommands();
	debugC(1, kDebugScript, "loadConScript: '%s' → %u sections, %u suj-states",
	      name.c_str(), _conScript.sections().size(), _sujEnabled.size());
	for (const ConSection &sec : _conScript.sections()) {
		debugC(2, kDebugScript, "  section %d: isCon=%d needsItem=%d clicPerso=%d clicZone=%d sujId=%d gotoId=%d items=%u",
		      sec.id, sec.isCon, sec.needsItem, sec.clicPerso, sec.clicZone, sec.sujId, sec.gotoId, sec.items.size());
		for (const ConCondition &cc : sec.conditions) {
			const char sep = cc.nextSep ? cc.nextSep : ' ';
			debugC(2, kDebugScript, "    cond: %s %c %d   [next=%c]", cc.key.c_str(), cc.op, cc.value, sep);
		}
	}
}

// Resolve a dialog line's speaker name to a perso ID.
//
// The original engine resolves speakers with a fixed table, not a heuristic:
// the strncmp chain in atlantis.exe at VA 0x428f58 (which drives the
// CLICPERSO game variable) tests the speaker name against a sequence of
// fixed prefixes, each paired with a persoId.  The compare (sub_44a83f) is a
// plain case-sensitive strncmp of the first N characters; the exe stores
// both a capitalised and a lowercase spelling of every name to absorb case.
// We transcribe the same table — in the same order, with the same per-entry
// compare length — but match case-insensitively against the capitalised
// spelling, which covers both exe variants in a single row.
//
// Order is significant: it is the exe's test order, so a shorter prefix that
// is also a prefix of a later name takes precedence (e.g. "Rein" before
// "Reinem").  The persoIds cross-check against INITDIAL.TXT (520 = agat1,
// 521 = lasc, 522 = meljan1a, ...).
//
// The player character "Seth" is handled before the table: the exe maps
// "seth" to disguise-specific hero meshes (513 herpret, 514 hegr), but for
// dialog playback we always want the plain hero persoId that
// loadDialInitData() detected from INITDIAL's s3d="hero" row.
int CryOmni3DEngine_Atlantis::resolveSpeakerPersoId(const Common::String &speaker) const {
	if (speaker.empty())
		return 0;
	if (_heroPersoId > 0 && scumm_strnicmp(speaker.c_str(), "seth", 4) == 0)
		return _heroPersoId;

	struct SpeakerEntry { const char *prefix; uint len; int persoId; };
	static const SpeakerEntry kSpeakerTable[] = {
		{ "Agat",      4, 520 }, { "Lasc",      4, 521 }, { "Melj",      4, 522 },
		{ "Sorc",      4, 567 }, { "Gard",      4, 604 }, { "Sold",      4, 604 },
		{ "Pech",      4, 524 }, { "Acty",      4, 524 }, { "Creo",      4, 525 },
		{ "Chas",      4, 540 }, { "Passant9",  8, 602 }, { "Carv",      4, 602 },
		{ "Passant6",  8, 601 }, { "Passante7", 9, 606 }, { "Hect",      4, 543 },
		{ "Cuis",      4, 551 }, { "Rein",      4, 560 }, { "Reinem",    6, 561 },
		{ "Reinep",    6, 562 }, { "Garc",      4, 526 }, { "Servan",    6, 609 },
		{ "Servag",    6, 582 }, { "Chie",      4, 650 }, { "Anna",      4, 663 },
		{ "Escl",      4, 597 }, { "Enne",      4, 667 }, { "Hona",      4, 682 },
		{ "Sama",      4, 683 }, { "Naka",      4, 684 }, { "Lona",      4, 576 },
		{ "JPil",      4, 571 }, { "Gimb",      4, 563 },
	};
	for (const SpeakerEntry &e : kSpeakerTable) {
		if (scumm_strnicmp(speaker.c_str(), e.prefix, e.len) == 0)
			return e.persoId;
	}
	return 0;
}

void CryOmni3DEngine_Atlantis::execConInitCommands() {
	for (const Common::String &cmd : _conScript.initCmds())
		execConSetCommand(cmd);
}

Common::Array<int> &CryOmni3DEngine_Atlantis::sujsForPerso(int persoId) {
	for (SujState &ss : _sujEnabled)
		if (ss.persoId == persoId) return ss.sujs;
	SujState ns;
	ns.persoId = persoId;
	_sujEnabled.push_back(ns);
	return _sujEnabled.back().sujs;
}

// True when subject `sujId` is in perso `persoId`'s enabled-subject list.
// Mirrors the per-perso subject table atlantis.exe builds at 0x602f10 (225
// bytes/perso: a count byte + up to 16 entries): enableSuj (FUN_00425f6c)
// appends, disableSuj (FUN_0042606c) removes.  A perso with no record — or
// one whose every subject was disabled — has no enabled subject, exactly
// like a zero count byte.  interactNPC() only consults this for a perso
// that *has* a non-empty record; a perso with none is not menu-driven.
bool CryOmni3DEngine_Atlantis::isSujEnabled(int persoId, int sujId) const {
	for (const SujState &ss : _sujEnabled) {
		if (ss.persoId != persoId) continue;
		for (int s : ss.sujs)
			if (s == sujId) return true;
		return false;
	}
	return false;
}

// gameover=N (CON) — the game-over screen, GereGameOver (atlantis.exe
// FUN_00426590).  Displays the full-screen image IMAGES\GOVER<NN>.TGA (NN =
// the GameOver variable, zero-padded to two digits — GOVER01..23 in the
// BigFile), then plays the narrator's verdict over it: the first [Narratrice]
// line that follows the /set(gameover=N) command, voiced by
// WAV\ATA<section4d><track>.APC with its subtitle drawn on the image.  It
// waits for the voice to finish — skippable on a click/key — or, when no APC
// is found, for the original's ~5 s timeout (its 4999 ms cap).  Then it
// returns to the caller, which reloads the chapter checkpoint (see
// playConSection): a game-over is gated on a persistent CON variable, so
// returning in place would only loop straight back into the same death.
//
// The narrator commentary build mirrors the original exactly: it always picks
// the *first* [Narratrice] line of the run (GereGameOver's per-section repeat
// counter at 0x6cccac is cleared on entry, so its variant-B branch is dead
// code) and derives the APC track letter from that line's index in the
// section (the '[' lines GereGameOver counts from the section header).
void CryOmni3DEngine_Atlantis::showGameOver(int sectionId, int n,
		const Common::String &narratorText, char narratorTrack) {
	Common::String name = Common::String::format("GOVER%02d.TGA", n);
	Graphics::Surface *raw = loadTGA(kFileTypeImages, name.c_str());
	if (!raw) {
		warning("showGameOver: cannot load IMAGES\\%s", name.c_str());
		return;
	}

	const Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface img(640, 480, fmt);
	img.fillRect(Common::Rect(640, 480), 0);
	img.blitFrom(*raw);
	raw->free();
	delete raw;

	// Narrator subtitle, bottom-anchored over the image — same metrics and
	// drop-shadow scheme as playSingleConLine().  Honour the ScummVM subtitle
	// toggle; the voice still plays when it is off.
	if (showSubtitles() && !narratorText.empty()) {
		const int leftMargin = 5;
		int widthBudget = img.w - leftMargin - 5;
		if (widthBudget < 32)
			widthBudget = 32;

		// Subtitles render in a proportional sans-serif font, matching the
		// original engine which draws all dialog/narration text in Arial via
		// GDI (atlantis.exe FUN_0041aee4: CreateFontA "Arial" + TextOutA, white
		// over a black shadow, transparent background) -- NOT the gold FONTMAX
		// sprite font, which the original uses only for menus.  kBigGUIFont is
		// ScummVM's built-in proportional Helvetica (Arial-like), compiled in so
		// it also works on the no-FreeType/no-theme Windows build.
		const bool useFontMax = false;
		const Graphics::Font *guiFont =
		    FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		if (!guiFont)
			guiFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		Common::Array<Common::String> subLines;
		int subLineH = 0;

		if (useFontMax) {
			int maxGlyphH = 0;
			for (uint si = 0; si < _fontMaxSprites.size(); ++si)
				if ((int)_fontMaxSprites[si].h > maxGlyphH)
					maxGlyphH = _fontMaxSprites[si].h;
			subLineH = (maxGlyphH > 0) ? maxGlyphH + 2 : 24;

			// Greedy whitespace word-wrap using FONTMAX advance widths.
			Common::String line, word;
			for (uint i = 0; i <= narratorText.size(); ++i) {
				char c = (i < narratorText.size()) ? narratorText[i] : ' ';
				if (c == ' ' || c == '\r' || c == '\n') {
					if (!word.empty()) {
						Common::String cand =
						    line.empty() ? word : line + " " + word;
						if (fontMaxStringWidth(cand) > widthBudget &&
						    !line.empty()) {
							subLines.push_back(line);
							line = word;
						} else {
							line = cand;
						}
						word.clear();
					}
				} else {
					word += c;
				}
			}
			if (!line.empty())
				subLines.push_back(line);
		} else if (guiFont) {
			subLineH = guiFont->getFontHeight() + 2;
			guiFont->wordWrapText(narratorText, widthBudget, subLines);
		}

		const uint32 subColor  = fmt.RGBToColor(255, 255, 255);
		const uint32 subShadow = fmt.RGBToColor(0, 0, 0);
		int subBaseY = img.h - (int)subLines.size() * subLineH - 6;
		if (subBaseY < 0)
			subBaseY = 0;
		for (uint i = 0; i < subLines.size(); ++i) {
			if (useFontMax) {
				int by = subBaseY + (int)i * subLineH + subLineH - 4;
				drawFontMaxText(img, subLines[i], leftMargin + 1, by + 1);
				drawFontMaxText(img, subLines[i], leftMargin,     by);
			} else if (guiFont) {
				int by = subBaseY + (int)i * subLineH;
				guiFont->drawString(&img, subLines[i], leftMargin + 1, by + 1,
				                    widthBudget, subShadow,
				                    Graphics::kTextAlignLeft);
				guiFont->drawString(&img, subLines[i], leftMargin, by,
				                    widthBudget, subColor,
				                    Graphics::kTextAlignLeft);
			}
		}
	}

	g_system->copyRectToScreen(img.getPixels(), img.pitch, 0, 0, 640, 480);
	g_system->updateScreen();

	// The screen owns the player's attention now — keep the cursor hidden and
	// drain the click/key that ended the triggering dialog so it is not read
	// as an immediate skip.
	showMouse(false);
	while (getCurrentMouseButton() != 0 && !shouldAbort()) {
		pollEvents();
		g_system->delayMillis(10);
	}
	clearKeys();

	// Narrator voice: WAV\ATA<section4d><track>.APC.  Wait for it to finish;
	// a click or key skips it (atlantis.exe GereGameOver's AIL_sample_status
	// loop, broken by its skip flag).
	Common::String apcName =
	    Common::String::format("ATA%04d%c.APC", sectionId, narratorTrack);
	Common::SeekableReadStream *apcFile =
	    narratorText.empty() ? nullptr
	                         : openBigFileStream(kFileTypeSound, apcName);
	Audio::PacketizedAudioStream *apc =
	    apcFile ? Audio::makeAPCStream(*apcFile) : nullptr;

	if (apc) {
		int64 remaining = apcFile->size() - apcFile->pos();
		if (remaining > 0) {
			byte *buf = new byte[remaining];
			apcFile->read(buf, (uint32)remaining);
			apc->queuePacket(new Common::MemoryReadStream(buf, remaining,
			                                              DisposeAfterUse::YES));
		}
		apc->finish();
		Audio::SoundHandle handle;
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, apc);
		debugC(1, kDebugScript, "showGameOver: GOVER%02d.TGA + %s", n, apcName.c_str());

		while (!shouldAbort() && _mixer->isSoundHandleActive(handle)) {
			pollEvents();
			if (checkKeysPressed(1, Common::KEYCODE_SPACE) || getCurrentMouseButton() != 0)
				break;
			g_system->delayMillis(10);
		}
		_mixer->stopHandle(handle);
	} else {
		// No narrator APC — hold the image for the original's ~5 s cap.
		debugC(1, kDebugScript, "showGameOver: GOVER%02d.TGA (no narrator audio)", n);
		uint32 deadline = g_system->getMillis() + 5000;
		while (!shouldAbort() && g_system->getMillis() < deadline) {
			pollEvents();
			if (checkKeysPressed(1, Common::KEYCODE_SPACE) || getCurrentMouseButton() != 0)
				break;
			g_system->delayMillis(10);
		}
	}
	delete apcFile;

	// Drain the skip click so it does not leak into the resumed game.
	while (getCurrentMouseButton() != 0 && !shouldAbort()) {
		pollEvents();
		g_system->delayMillis(10);
	}
	clearKeys();
}

// Fade the on-screen image to black over ~0.3 s.  Used by /set(fadeout) just
// before a gameover or a scene change; a plain multiplicative ramp of the
// framebuffer, then a solid-black fill so the next screen draws over black.
void CryOmni3DEngine_Atlantis::fadeScreenToBlack() {
	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	for (int step = 0; step < 16 && !shouldAbort(); step++) {
		Graphics::Surface *s = g_system->lockScreen();
		if (s && fmt.bytesPerPixel == 2) {
			for (int y = 0; y < s->h; y++) {
				uint16 *row = (uint16 *)s->getBasePtr(0, y);
				for (int x = 0; x < s->w; x++) {
					uint8 r, g, b;
					fmt.colorToRGB(row[x], r, g, b);
					row[x] = (uint16)fmt.RGBToColor(r * 13 / 16, g * 13 / 16, b * 13 / 16);
				}
			}
		}
		g_system->unlockScreen();
		g_system->updateScreen();
		g_system->delayMillis(20);
	}
	Graphics::Surface *s = g_system->lockScreen();
	if (s)
		s->fillRect(Common::Rect(0, 0, s->w, s->h), 0);
	g_system->unlockScreen();
	g_system->updateScreen();
}

void CryOmni3DEngine_Atlantis::playPuzzle(int puzzleId) {
	// Dispatch to the per-puzzle handler.  Puzzles not yet implemented
	// fall back to the auto-win stub so their CHAPI script chains keep
	// progressing (most of them gate further story content behind
	// FinPuzzle=255, e.g. CHAPI012's /sel(FinPuzzle=255)&(Vue=N)&...).
	int finPuzzle = 255;
	switch (puzzleId) {
	case 0: {
		// Celestial alignment ("eclipse on Atlantis") at CHAPI012 vue 221.
		EclipsePuzzle puz(this);
		finPuzzle = puz.run();
		break;
	}
	default:
		debugC(1, kDebugScript, "playPuzzle: no handler for puzzle %d, auto-win", puzzleId);
		break;
	}
	_gameVars["finpuzzle"] = finPuzzle;
	// Puzzle launches are often gated behind a click on a sentinel zone
	// (CHAPI012 vue 221 zone.action=1000 — "1000" is not a real place,
	// just a hotspot trigger for /sel(vue=221)&(flagtemp6=0)).  The /sel
	// itself doesn't /set(abortdepart=1), so the pending departure to
	// place 1000 would still fire after the puzzle returns — emitting a
	// "place 1000 not found" warning and stranding the player.  Cancel
	// any pending departure here so the puzzle's caller stays put.
	_nextPlaceId = uint(-1);

	// Re-run the arrival /sel scan so /sel sections that gate on
	// `FinPuzzle = 255` fire immediately on win.  CHAPI012 line 129
	// (`/sel(FinPuzzle=255)&(Vue=221)&(Flagtemp6=0)` -> /set(cinema=30)
	// + hide/show NPCs + Flagtemp6=1) is the canonical case: without
	// this re-scan the cinematic only triggers on the next natural
	// place transition.  The puzzle-launch /sel
	// (`/sel(vue=221)&(flagtemp6=0)` -> /set(Startpuzzle=0)) sits
	// in the same scan but the cinema gate fires first (earlier in
	// the CON file), sets Flagtemp6=1, and the puzzle-launch /sel's
	// `flagtemp6=0` cond then fails -- no recursive puzzle relaunch.
	if (finPuzzle == 255 && _currentPlaceId != uint(-1))
		runArrivalSelSections((int)_currentPlaceId, (int)_currentPlaceId);
}

void CryOmni3DEngine_Atlantis::execConSetCommand(const Common::String &cmd) {
	const char *p = cmd.c_str();

	// enableSuj,persoId=suj1,suj2,...
	if (scumm_strnicmp(p, "enablesuj,", 10) == 0) {
		p += 10;
		int perso = atoi(p);
		while (*p && *p != '=') p++;
		if (*p == '=') {
			p++;
			Common::Array<int> &sujs = sujsForPerso(perso);
			while (*p) {
				int suj = atoi(p);
				if (suj > 0) {
					bool found = false;
					for (int s : sujs) { if (s == suj) { found = true; break; } }
					if (!found) sujs.push_back(suj);
				}
				while (*p && *p != ',' && *p != ')') p++;
				if (*p == ',') p++; else break;
			}
		}
		return;
	}

	// disablesuj,persoId=sujId[,sujId...]
	// Supports a comma-separated list (e.g. `/Set(DisableSuj,560=1,5)` in
	// CHAPI*.CON disables BOTH subjects 1 and 5 for perso 560).  The previous
	// single-value parse silently dropped every id past the first, leaving
	// stale subjects in `_sujEnabled` that re-appeared in the topic menu.
	if (scumm_strnicmp(p, "disablesuj,", 11) == 0) {
		p += 11;
		int perso = atoi(p);
		while (*p && *p != '=') p++;
		if (*p == '=') {
			p++;
			Common::Array<int> &sujs = sujsForPerso(perso);
			while (*p) {
				int suj = atoi(p);
				if (suj > 0) {
					for (int i = (int)sujs.size() - 1; i >= 0; i--)
						if (sujs[i] == suj) { sujs.remove_at(i); break; }
				}
				while (*p && *p != ',' && *p != ')') p++;
				if (*p == ',') p++; else break;
			}
		}
		return;
	}

	// hideperso=N / disableperso=N — deactivate every sprite entry for
	// perso N across every place (atlantis.exe FUN_00427bd0: walks the
	// 512-place sprite array, sets field+0x04=0 on records whose persoId
	// matches, and adds N to the persistent hidden-perso list at +0x611010).
	if (scumm_strnicmp(p, "hideperso=", 10) == 0 ||
	        scumm_strnicmp(p, "disableperso=", 13) == 0) {
		const char *eq = strchr(p, '=');
		int perso = eq ? atoi(eq + 1) : 0;
		if (perso > 0) {
			bool found = false;
			for (int h : _hiddenPersos) if (h == perso) { found = true; break; }
			if (!found)
				_hiddenPersos.push_back(perso);
			for (SpritePlaceEntry &e : _spritePlaceList)
				if (e.persoId == perso) e.active = false;
			_spriteLayerDirty = true;
			_npcBoundsDirty   = true;
			debugC(1, kDebugScript, "execConSetCommand: hide perso %d", perso);
		}
		return;
	}

	// showperso=N — activate every sprite entry for perso N (exe's
	// FUN_00427eb0: walks all places, sets field+0x04=1 on matching
	// records, also restores the perso from the hidden list).
	if (scumm_strnicmp(p, "showperso=", 10) == 0) {
		int perso = atoi(p + 10);
		for (int i = (int)_hiddenPersos.size() - 1; i >= 0; i--)
			if (_hiddenPersos[i] == perso) _hiddenPersos.remove_at(i);
		for (SpritePlaceEntry &e : _spritePlaceList)
			if (e.persoId == perso) e.active = true;
		_spriteLayerDirty = true;
		_npcBoundsDirty   = true;
		debugC(1, kDebugScript, "execConSetCommand: show perso %d", perso);
		return;
	}

	// addguest=N -- NPC N starts following the hero.  Whatever the exe's
	// internal record layout (it shares the 9-byte inventory-slot array at
	// 0x6ce138 according to FUN_0042785c's pickup branch), the visible
	// effect is: from now until /set(subguest=N), the guest is force-shown
	// at every place they have a sprite, overriding any prior HidePerso.
	// CHAPI032/038/056-58/64/66-68/78-79 use this for Servage (523), the
	// queen's lady (530), Lascoyt (587/592), and friends.  Without it, the
	// companion NPCs that should accompany Seth are simply absent from
	// every place after the script switches them on.
	if (scumm_strnicmp(p, "addguest=", 9) == 0) {
		int perso = atoi(p + 9);
		if (perso > 0) {
			bool found = false;
			for (int g : _guests) if (g == perso) { found = true; break; }
			if (!found) _guests.push_back(perso);
			// A guest is by definition shown -- pull them out of the
			// hidden list and activate their sprites at every place.
			for (int i = (int)_hiddenPersos.size() - 1; i >= 0; i--)
				if (_hiddenPersos[i] == perso) _hiddenPersos.remove_at(i);
			for (SpritePlaceEntry &e : _spritePlaceList)
				if (e.persoId == perso) e.active = true;
			_spriteLayerDirty = true;
			_npcBoundsDirty   = true;
			debugC(1, kDebugScript, "execConSetCommand: addguest %d", perso);
		}
		return;
	}

	// subguest=N -- NPC N stops following the hero.  Mirror of addguest:
	// remove from the guest list AND deactivate their sprites so the
	// companion vanishes from the current and future places.
	if (scumm_strnicmp(p, "subguest=", 9) == 0) {
		int perso = atoi(p + 9);
		if (perso > 0) {
			for (int i = (int)_guests.size() - 1; i >= 0; i--)
				if (_guests[i] == perso) _guests.remove_at(i);
			for (SpritePlaceEntry &e : _spritePlaceList)
				if (e.persoId == perso) e.active = false;
			_spriteLayerDirty = true;
			_npcBoundsDirty   = true;
			debugC(1, kDebugScript, "execConSetCommand: subguest %d", perso);
		}
		return;
	}

	// loadmusik=N,M,...  — preload one or more music tracks (ignore for now)
	// loadmusik / delmusik — in the original these copy APC tracks between the
	// CD and a hard-disk cache.  This port streams every track straight from
	// the BigFile, so there is nothing to pre-load or evict; accept and ignore
	// both.  startmusik / stopmusik do the actual playback control.
	if (scumm_strnicmp(p, "loadmusik=", 10) == 0 ||
	        scumm_strnicmp(p, "delmusik=", 9) == 0) {
		return;
	}

	// vue=N — transition to panoramic place N
	if (scumm_strnicmp(p, "vue=", 4) == 0) {
		_nextPlaceId = (uint)atoi(p + 4);
		return;
	}

	// startmusik=N
	if (scumm_strnicmp(p, "startmusik=", 11) == 0) {
		musicPlayTrack(atoi(p + 11));
		return;
	}

	// stopmusik=N
	if (scumm_strnicmp(p, "stopmusik=", 10) == 0) {
		musicStop();
		return;
	}

	// exit=N -- end-of-game trigger.  Only one use in the data
	// (CHAPI085 line 42 after the final narrator line + cinema 210), so the
	// concrete behaviour is "the game has reached the credits, stop the
	// regular game loop".  Set the gameVar so any /sel(exit=N) condition can
	// pick it up, then route through the existing main-menu abort path
	// (kAbortNewGame is the closest match: brings the player back to the
	// title menu rather than auto-quitting).
	if (scumm_strnicmp(p, "exit=", 5) == 0) {
		int n = atoi(p + 5);
		_gameVars["exit"] = n;
		debugC(1, kDebugScript, "execConSetCommand: exit=%d -> returning to main menu", n);
		_abortCommand = kAbortNewGame;
		return;
	}

	// cinema=N — play transition video CINEM<N>.HNM
	if (scumm_strnicmp(p, "cinema=", 7) == 0) {
		int cinId = atoi(p + 7);
		Common::String hnm = Common::String::format("CINEM%03d.HNM", cinId);
		debugC(1, kDebugScript, "execConSetCommand: cinema=%d -> %s", cinId, hnm.c_str());
		playTransitionVideo(hnm);
		return;
	}

	// abortdepart=1 — cancel pending place departure
	if (scumm_strnicmp(p, "abortdepart=", 12) == 0) {
		_nextPlaceId = uint(-1);
		return;
	}

	// changewam=name.wam
	// setupWAMByName invokes _wam.clear() which destroys every AtlantisPlace
	// the old WAM owned -- including the one _currentPlace points at.  Null
	// the pointer here (matching the sub-WAM hand-off at line 1728) so the
	// next executeTransition / handleWarp doesn't dereference freed memory.
	// CHAPI015 section 203 reproduces the crash: the Meljanz escape runs
	// /set(Changewam=atlan4.wam) followed by /set(vue=81), and the
	// subsequent place-change in gameStep crashed on _currentPlace inside
	// executeTransition.
	if (scumm_strnicmp(p, "changewam=", 10) == 0) {
		setupWAMByName(p + 10);
		_currentPlace   = nullptr;
		_currentPlaceId = uint(-1);
		return;
	}

	// inventN=1 — add object N to inventory; inventN=0 — remove it.
	if (scumm_strnicmp(p, "invent", 6) == 0 && Common::isDigit(p[6])) {
		const char *eqPos = strchr(p + 6, '=');
		if (eqPos) {
			uint objId = (uint)atoi(p + 6);
			int val = atoi(eqPos + 1);
			if (val) {
				// findObjectByNameID calls error() on failure; check first.
				Object *obj = nullptr;
				for (Object &o : _objects)
					if (o.valid() && o.idOBJ() == objId) { obj = &o; break; }
				if (obj) {
					if (!_inventory.inInventoryByNameID(objId)) {
						_inventory.add(obj);
						debugC(1, kDebugScript, "execConSetCommand: added object %u to inventory", objId);
					}
					// Remember the given object so it shows centre-bottom for
					// the rest of the dialog (atlantis.exe DAT_0049619c, set by
					// the INVENT add in FUN_0042b328 and drawn each dialog frame
					// until the dialog ends).  The sprite index is the object's
					// position in _objects (parallel to _objSprites).
					if (!_objects.empty())
						_dialogGivenObjSprite = (int)(obj - &_objects.front());
				} else {
					debugC(1, kDebugScript, "execConSetCommand: invent%u=1 — object not defined", objId);
				}
			} else {
				_inventory.removeByNameID(objId);
				// removeByNameID only deselects if the object is in the
				// persistent inventory list.  World pickups (CHAPI013
				// vue 126's flower pot) bypass the list and live only
				// as selectedObject(), so the deselect would miss them
				// -- match by id and clear the selection explicitly.
				Object *sel = _inventory.selectedObject();
				if (sel && sel->idOBJ() == objId)
					_inventory.deselectObject();
				debugC(1, kDebugScript, "execConSetCommand: removed object %u from inventory", objId);
			}
			return;
		}
	}

	// chapter=N — switch active CON script to chapter N (independent of WAM chapter).
	if (scumm_strnicmp(p, "chapter=", 8) == 0) {
		int newChapter = atoi(p + 8);
		_gameVars["chapter"] = newChapter;
		if ((uint)newChapter != _currentCONChapter) {
			debugC(1, kDebugScript, "execConSetCommand: switching CON script to chapter %d", newChapter);
			_currentCONChapter = (uint)newChapter;
			_sujEnabled.clear();
			loadConScript(newChapter);
			execConInitCommands();
			// Original-game autosave point: a real chapter advance rewrites
			// the player's single save.  Defer the write to doPlaceChange()
			// so it captures the new chapter's first place (atlantis.exe
			// FUN_00427634 saves as it enters that place).
			_autosavePending = true;
			// Fire the new chapter's intro /sel sections -- canonical example
			// is CHAPI014.CON section 10 (`/sel(initflag=0) /set(Cinema=36)`),
			// the throw-success cinematic that plays right after CHAPI013's
			// /014 fires /set(chapter=14).  Without this scan the chapter
			// switches silently and the player just lands at the same vue with
			// no cinematic.  Same path arrival uses _currentPlaceId on both
			// sides so (arriveevue>0) still holds.
			runArrivalSelSections((int)_currentPlaceId, (int)_currentPlaceId);
			debugC(1, kDebugScript, "execConSetCommand: chapter=%d switch DONE (back from arrival /sel)",
			      newChapter);
		}
		return;
	}

	// AngleWarpX=N — snap camera horizontal angle to N degrees.
	if (scumm_strnicmp(p, "anglewarpx=", 11) == 0) {
		int deg = atoi(p + 11);
		_omni3dMan.setAlpha(deg * M_PI / 180.0);
		_gameVars["anglewarpx"] = deg;
		return;
	}

	// AngleWarpY=N — snap camera vertical angle to N degrees.
	if (scumm_strnicmp(p, "anglewarpy=", 11) == 0) {
		int deg = atoi(p + 11);
		_omni3dMan.setBeta(deg * M_PI / 180.0);
		_gameVars["anglewarpy"] = deg;
		return;
	}

	// gameover=N — store the variable only.  The game-over screen itself is
	// driven by playConSection(): the original engine's section player
	// (atlantis.exe FUN_0042844c) checks the GameOver variable after every
	// command and, when it is non-zero, breaks out of the section and calls
	// GereGameOver with the lines that follow.  Showing it from here would
	// run it *before* those narrator lines instead of feeding them to it.
	if (scumm_strnicmp(p, "gameover=", 9) == 0) {
		int n = atoi(p + 9);
		_gameVars["gameover"] = n;
		return;
	}

	// objetenmain=N — force object N into the player's hand (N=0 clears selection).
	if (scumm_strnicmp(p, "objetenmain=", 12) == 0) {
		int objId = atoi(p + 12);
		if (objId == 0) {
			_inventory.setSelectedObject(nullptr);
		} else {
			for (Object &o : _objects) {
				if (o.valid() && (int)o.idOBJ() == objId) {
					_inventory.setSelectedObject(&o);
					break;
				}
			}
		}
		_gameVars["objetenmain"] = objId;
		return;
	}

	// newsound=N — play the ambient/SFX track identified by APC table index N
	// (table extracted from atlantis.exe by loadAtlantisExeTables).  Always
	// stops any active newsound before starting a new one, so blocked-zone
	// feedback (e.g. CHAPI001 `/set(newsound=14)` at zones 150/168) doesn't
	// pile up.  Played as one-shot SFX on the SFX channel — long enough that
	// the player hears the cue; looping would need an APC-aware loop adapter.
	if (scumm_strnicmp(p, "newsound=", 9) == 0) {
		int soundId = atoi(p + 9);
		_gameVars["newsound"] = soundId;

		if (_mixer->isSoundHandleActive(_newSoundHandle))
			_mixer->stopHandle(_newSoundHandle);

		// soundId is a 0-based index into the APC table extracted from
		// atlantis.exe: empirically, CON `newsound=14` plays the 15th file in
		// our enumeration (Toctoc.apc), so do NOT subtract 1 here.
		if (soundId < 0 || (uint)soundId >= _newSoundApcNames.size()) {
			if (!_newSoundApcNames.empty())
				debugC(1, kDebugScript, "execConSetCommand: newsound=%d out of table range (0..%u)",
				      soundId, _newSoundApcNames.size() - 1);
			return;
		}

		const Common::String &apcName = _newSoundApcNames[soundId];
		Common::SeekableReadStream *apcFile =
		    openBigFileStream(kFileTypeSound, apcName);
		if (!apcFile) {
			debugC(1, kDebugScript, "execConSetCommand: newsound=%d '%s' not in BigFile",
			      soundId, apcName.c_str());
			return;
		}

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
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_newSoundHandle, apc);
			debugC(1, kDebugScript, "execConSetCommand: newsound=%d playing '%s'",
			      soundId, apcName.c_str());
		}
		delete apcFile;
		return;
	}

	// stopsound=N — stop the currently playing newsound.  N is ignored: the
	// original engine only tracks one ambient channel and any stopsound
	// silences it.
	if (scumm_strnicmp(p, "stopsound=", 10) == 0) {
		_gameVars["newsound"] = 0;
		if (_mixer->isSoundHandleActive(_newSoundHandle))
			_mixer->stopHandle(_newSoundHandle);
		return;
	}

	// newwspranim=N — activate sprite[N-1] at the current place.
	//
	// N is a 1-based index into the place's sprite array (in spwpal1.txt
	// order), NOT a sprite "type".  Per atlantis.exe disassembly at
	// 0x428376:
	//   sprite[N-1].field+0x04 = 1   ; mark active
	//   place_base[+0x04] = N-1      ; remember "current wspranim sprite"
	//   sprite[N-1].field+0x26 = 0   ; reset frame counter
	//   vars[NEWWSPRANIM]    = 0     ; consume the command
	// Example (CHAPI012 vue 180): NewWsprAnim=2 activates the 2nd sprite
	// at vue 180 (a183h180.spw, static guard); the alarm fires
	// NewWsprAnim=3 which swaps in the 3rd (a183g180.spw, 33-frame guard
	// walking animation).  The frame counter on the active sprite is what
	// the script reads as WSPRFRAME.
	if (scumm_strnicmp(p, "newwspranim=", 12) == 0) {
		int n = atoi(p + 12);
		if (n >= 1) {
			int seen = 0;
			for (SpritePlaceEntry &e : _spritePlaceList) {
				if (e.placeId != (uint16)_currentPlaceId) continue;
				if (seen == n - 1) {
					e.active = true;
					e.frame  = 0;
					_wsprAnimActiveIdx = n - 1;
					_gameVars["wsprframe"] = 0;
					_spriteLayerDirty = true;
					break;
				}
				seen++;
			}
		}
		return;
	}

	// stopwspranim=N — deactivate sprite[N-1] at the current place; reset
	// its frame counter and the script's WSPRFRAME variable.  (Same VA
	// 0x428450 disassembly path as NewWsprAnim above.)
	if (scumm_strnicmp(p, "stopwspranim=", 13) == 0) {
		int n = atoi(p + 13);
		if (n >= 1) {
			int seen = 0;
			for (SpritePlaceEntry &e : _spritePlaceList) {
				if (e.placeId != (uint16)_currentPlaceId) continue;
				if (seen == n - 1) {
					e.active = false;
					e.frame  = 0;
					if (_wsprAnimActiveIdx == n - 1)
						_wsprAnimActiveIdx = -1;
					_gameVars["wsprframe"] = 0;
					_spriteLayerDirty = true;
					break;
				}
				seen++;
			}
		}
		return;
	}

	// disable=N — bump place N's visit count so its first-visit /sel sections
	// (nbvisite=0) stop firing (atlantis.exe /set DISABLE branch).
	if (scumm_strnicmp(p, "disable=", 8) == 0) {
		uint placeId = (uint)atoi(p + 8);
		if (placeId < _placeVisits.size() && _placeVisits[placeId] < 255)
			_placeVisits[placeId]++;
		debugC(1, kDebugScript, "execConSetCommand: disable place %u", placeId);
		return;
	}

	// fadeout=N — fade the screen to black before a dramatic transition
	// (a gameover or a scene change).  fadein is unused by any chapter.
	if (scumm_strnicmp(p, "fadeout=", 8) == 0) {
		fadeScreenToBlack();
		return;
	}

	// startpuzzle=N — launch minigame N.  N is an enum of ~15 distinct
	// puzzles (eclipse, sliding tile, hexa, serpent, crab, lion, throne,
	// meta, ...) referenced across chapters 12-79.  On completion the
	// puzzle writes back FinPuzzle (255 = win; smaller values denote
	// loss patterns, e.g. CHAPI018's chess puzzle uses 1/2 for "poor
	// queen" / "poor hero" voice-line variants).
	//
	// Framework only: actual puzzle UIs are not yet implemented.  Each
	// /set(StartPuzzle=N) immediately resolves to FinPuzzle=255 so the
	// CHAPI012 (and other) script chains continue past the puzzle gate
	// instead of stalling.  Subsequent commits will replace the auto-
	// win in playPuzzle() with the per-puzzle UIs.
	if (scumm_strnicmp(p, "startpuzzle=", 12) == 0) {
		int puzzleId = atoi(p + 12);
		playPuzzle(puzzleId);
		return;
	}

	// Generic assignment: varname=N — store in _gameVars for condition evaluation.
	const char *eqPos = strchr(p, '=');
	if (eqPos && eqPos > p) {
		Common::String key(p, (uint32)(eqPos - p));
		key.toLowercase();
		int val = atoi(eqPos + 1);
		_gameVars[key] = val;
		// Several VARIAS.CON names are commands, not plain variables.  The
		// ones execConSetCommand does not act on still get stored here (so
		// flag conditions keep working), but their *action* is lost — make
		// that visible as a stub line rather than a silent gamevar write.
		static const char *const kStubCommands[] = {
			"startpuzzle", "finpuzzle", "ouvrirporte", "tirfleche",
			"addguest", "subguest", "hideguest", "showguest", "incperso",
			"fadein", "waitclic"
		};
		bool isStub = false;
		for (const char *s : kStubCommands)
			if (key == s) { isStub = true; break; }
		if (isStub)
			debugC(1, kDebugScript, "execConSetCommand: '%s=%d' — command not implemented (stub)",
			      key.c_str(), val);
		else
			debugC(2, kDebugScript, "execConSetCommand: gamevar %s=%d", key.c_str(), val);
		return;
	}
	debugC(2, kDebugScript, "execConSetCommand: unhandled '%s'", cmd.c_str());
}

// ---------------------------------------------------------------------------
// NPC hit testing
// ---------------------------------------------------------------------------

bool CryOmni3DEngine_Atlantis::npcPanoHitTest(const Common::Point &pan, int &outPersoId) const {
	for (const NPCBound &nb : _npcPlaceBounds) {
		debugC(5, kDebugScript, "npcPanoHitTest: pan=(%d,%d) vs perso=%d bounds [%d,%d]-[%d,%d]",
		      pan.x, pan.y, nb.persoId,
		      nb.panoBounds.left, nb.panoBounds.top,
		      nb.panoBounds.right, nb.panoBounds.bottom);
		if (nb.panoBounds.contains(pan)) {
			outPersoId = nb.persoId;
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// /sel condition evaluation and automatic section runners
// ---------------------------------------------------------------------------

bool CryOmni3DEngine_Atlantis::evalSelSection(const ConSection &sec,
                                               int departZone, int currentVue) const {
	// Roll a fresh random byte once per section evaluation so multiple
	// `random` clauses inside the same conjunction (e.g. `(random>64)&
	// (random<128)`) see a consistent value, while *different* sections
	// see independent rolls.  Matches the original engine's per-section
	// probabilistic gating used for ambient dialog variety.
	const int rolledRandom = (int)_conRng.getRandomNumber(255);

	// Left-to-right short-circuit eval, mirroring atlantis.exe FUN_004298ac:
	//   nextSep '&' + fail -> EXIT FAIL ; pass -> continue
	//   nextSep '|' + pass -> EXIT SUCCESS ; fail -> continue
	//   end of list: last cond's result IS the overall result.
	// So `A & B | C & D` reads as `A AND (B OR (C AND D))` — neither standard
	// nor OR-tighter precedence; just a left-to-right chain.  This is the
	// only reading that makes every multi-clause /sel in the game data
	// behave as authored (e.g. /sel(parletemp3=3)&(departvue=38)|(56)|(22)
	// gates the OR-list on the leading parletemp3=3 guard the way the exe
	// evaluates it — without it, every navigation to 38/56/22 would silently
	// abort regardless of parletemp3).
	if (sec.conditions.empty()) return true;
	for (const ConCondition &cc : sec.conditions) {
		int lhs;
		if (cc.key == "departvue") {
			lhs = departZone;
		} else if (cc.key == "vue") {
			lhs = currentVue;
		} else if (cc.key == "arriveevue") {
			// When evalSelSection is called from runArrivalSelSections, the caller
			// sets _gameVars["arriveevue"] to the arrived place ID.  Fall through to
			// generic path so the condition is evaluated against that stored value.
			Common::HashMap<Common::String, int>::const_iterator it = _gameVars.find(cc.key);
			lhs = (it != _gameVars.end()) ? it->_value : 0;
		} else if (cc.key == "objetenmain") {
			Object *sel = _inventory.selectedObject();
			lhs = (sel && sel->idOBJ() != uint(-1)) ? (int)sel->idOBJ() : 0;
		} else if (cc.key.hasPrefix("invent") && cc.key.size() > 6
		           && Common::isDigit(cc.key[6])) {
			// invent<N> — 1 when object N is in the inventory, else 0.  The
			// original (atlantis.exe FUN_00429e54) tests the live inventory
			// here; it is not a stored game variable.
			uint objId = (uint)atoi(cc.key.c_str() + 6);
			lhs = _inventory.inInventoryByNameID(objId) ? 1 : 0;
		} else if (cc.key == "chaptertime") {
			// Live wall-clock seconds since the active CHAPI*.CON loaded.
			// CHAPI030 uses (ChapterTime>120) and (ChapterTime>20) to gate
			// dialog on real time elapsed -- the storage path through
			// _gameVars would never tick this up, so it's computed here
			// from the loadConScript stamp.
			lhs = _chapterStartMs
			      ? (int)((g_system->getMillis() - _chapterStartMs) / 1000)
			      : 0;
		} else if (cc.key == "nbvisite") {
			// Per-place visit count of the place being evaluated.
			lhs = (currentVue >= 0 && (uint)currentVue < _placeVisits.size())
			      ? (int)_placeVisits[currentVue] : 0;
		} else if (cc.key == "random") {
			lhs = rolledRandom;
		} else {
			Common::HashMap<Common::String, int>::const_iterator it = _gameVars.find(cc.key);
			lhs = (it != _gameVars.end()) ? it->_value : 0;
		}

		bool ok;
		switch (cc.op) {
		case '=': ok = (lhs == cc.value); break;
		case '<': ok = (lhs <  cc.value); break;
		case '>': ok = (lhs >  cc.value); break;
		case '!': ok = (lhs != cc.value); break;
		default:  ok = false;             break;
		}

		if (cc.nextSep == '&') {
			if (!ok) return false;
		} else if (cc.nextSep == '|') {
			if (ok) return true;
		} else {
			// End of condition list — this cond's pass/fail is the answer.
			return ok;
		}
	}
	// Trailing separator with no closing condition (malformed input) — treat
	// as the last group's success carrying through.
	return true;
}

void CryOmni3DEngine_Atlantis::runZoneConSections(int zoneId) {
	// Temporarily expose the clicked zone as a game variable so evalSelSection
	// can evaluate (cliczone=N) conditions through the generic path.
	_gameVars["cliczone"] = zoneId;

	// A fired section may /set(chapter=N), reparsing _conScript and
	// invalidating this range-for's iterators; stop if the chapter changes.
	const uint entryChapter = _currentCONChapter;

	for (ConSection &sec : _conScript.sections()) {
		if (!sec.isCon || sec.clicZone != zoneId) continue;
		// Don't gate on the /con* asterisk for zone clicks: sections that
		// actually require an item-in-hand spell it out as an
		// (ObjetEnMain=N) condition (handled by evalSelSection).  The
		// asterisk on a cliczone section just marks it as a zone-click
		// handler — empirically, CHAPI001.CON section 14 uses
		// `/con*(cliczone=499)` with no ObjetEnMain and is expected to
		// fire on a plain click (plays Toctoc.apc for the locked door).
		if (!evalSelSection(sec, 0, (int)_currentPlaceId)) continue;

		debugC(1, kDebugScript, "runZoneConSections: zone=%d firing section %d", zoneId, sec.id);
		playConSection(sec);

		if (shouldAbort() || _nextPlaceId == uint(-1) ||
		    _currentCONChapter != entryChapter)
			break;
	}

	_gameVars["cliczone"] = 0;
}

void CryOmni3DEngine_Atlantis::runDepartureSelSections(int departZone) {
	// Faithful port of the exe's single /sel scanner (FUN_004280f8): walk
	// every non-/con section in file order and fire the FIRST whose condition
	// evaluates true.  Arrival vs departure are not categorised in the
	// original — arriveevue stays 0 during a departure scan, so arrival-only
	// sections (those gated on (arriveevue>0) / (initflag=0)) self-exclude;
	// departure-only sections gate on (departvue>0) or (departvue=N).  Only
	// the *operator* on departvue varies — the catch-all place-lock pattern
	// is /sel(departvue>0)&(vue>A)&(vue<B) (e.g. CHAPI011 section 10 that
	// locks the player into the throne-room cluster while Creon's cinema
	// flow is active), so no pre-filter by operator/value; evalSelSection
	// decides.  Single-fire also sidesteps dangling iterators if
	// playConSection ends up doing /set(chapter=N).
	for (ConSection &sec : _conScript.sections()) {
		if (sec.isCon) continue;
		// /tim* sections fire only on timer ticks — exclude them from the
		// /sel scan, same reasoning as runArrivalSelSections.
		if (sec.isTimer) continue;

		if (!evalSelSection(sec, departZone, (int)_currentPlaceId)) continue;

		debugC(1, kDebugScript, "runDepartureSelSections: zone=%d firing section %d", departZone, sec.id);
		playConSection(sec);
		break;
	}
}

void CryOmni3DEngine_Atlantis::runArrivalSelSections(int arrivedVue, int fromVue) {
	// arriveevue = the place we arrived FROM, so /sel conditions resolve both
	// the plain marker form (arriveevue>0)&(vue=N) and the specific form
	// (arriveevue=FROM)&(vue=TO).  On the very first transition there is no
	// prior place; fall back to the arrived place so (arriveevue>0) still holds.
	_gameVars["arriveevue"] = (fromVue > 0) ? fromVue : arrivedVue;

	// Faithful port of the original /sel scanner — atlantis.exe FUN_004280f8,
	// reached on every place arrival via FUN_00423ad8 -> FUN_00427814 ->
	// FUN_004277c8.  The exe keeps a SINGLE /sel pool with no arrival/
	// departure/init categorisation: it walks the CON script top to bottom and
	// fires the FIRST /sel section whose condition expression evaluates true,
	// then returns immediately (the `goto` out of FUN_004280f8 — exactly one
	// section per scan).  arriveevue, departvue, vue, initflag, ... are just
	// entries in the variable array at 0x611210, all compared generically by
	// FUN_004298ac; there is no key filter.
	//
	// So every /sel (isCon==0) section is eligible, evaluated in file order.
	// Two consequences fall out for free, both matching the exe:
	//   - The chapter intro fires: every CHAPI*.CON opens (right after
	//     /FININIT) with /sel(initflag=0), so it is the first section the scan
	//     reaches, and initflag is 0 right after loadConScript clears the
	//     transient variables.  Its first body item /set(initflag=1) then makes
	//     the condition false (CHAPI011 section 125 — the throne-room cinema).
	//   - Pure-conditional /sel sections (e.g. CHAPI011 167/168/162) become
	//     reachable instead of dead.
	// /con (clic-perso / clic-zone) sections are a separate scanner in the exe
	// (FUN_004281f8); they are skipped here via sec.isCon.
	for (ConSection &sec : _conScript.sections()) {
		// /con sections belong to the separate FUN_004281f8 scanner.
		if (sec.isCon) continue;
		// /tim* sections fire only on timer ticks (runTimerSections), not on
		// the arrival /sel scan.  Without this exclusion an arrival at vue 182
		// in palais2 (CHAPI012 line 76-79 — a /tim2 with cond Vue=182 &
		// flagtemp3=0) would run on entry and set flagtemp3=1 immediately,
		// stranding the player on the first departure: line 29's death gate
		// /sel(departvue>0)&(Flagtemp3=1) then fires on the next move.
		if (sec.isTimer) continue;

		// One-shot guard.  The exe has no per-/sel played flag — re-fire is
		// prevented purely by the section mutating its own gate variable (e.g.
		// /set(initflag=1)).  sec.played is kept as a safety net for sections
		// that do not self-guard; it only ever suppresses a *repeat* firing,
		// never one the exe's first-match scan would have produced.
		if (sec.played) continue;

		// departZone=0: on an arrival scan departvue resolves to 0, so any
		// (departvue=N) clause fails on its own — departure /sel sections
		// self-exclude with no key filter, just as in the exe (where the
		// departvue variable simply holds 0 at this point).  currentVue=
		// arrivedVue so (vue=N) checks the place just entered.
		if (!evalSelSection(sec, 0, arrivedVue)) continue;

		debugC(1, kDebugScript, "runArrivalSelSections: vue=%d firing section %d", arrivedVue, sec.id);
		sec.played = true;
		playConSection(sec);

		// Single-fire: FUN_004280f8 returns the instant a section matches, so
		// exactly one /sel runs per scan.  Breaking now also sidesteps the
		// range-for iterator — playConSection() may /set(chapter=N) and reparse
		// _conScript, which would leave `sec` and the iterator dangling.
		break;
	}

	_gameVars["arriveevue"] = 0;
}

bool CryOmni3DEngine_Atlantis::runGenericConSections() {
	// Generic /con scan -- mirror of atlantis.exe FUN_00425cf4.  The original
	// engine triggers this whenever the click action lands in the object id
	// range [256, 512): it walks every /Con or /con section (skipping /Suj-
	// prefixed topic gates) and fires the FIRST whose conditions evaluate
	// true.  Used for "use held object in the world at a specific state"
	// scripts that don't gate on ClicPerso/ClicZone -- canonical case is
	// CHAPI013 section 178 + /014 fallbacks ("throw the pot on the priest").
	//
	// We skip sections with a clicperso / cliczone clause (those belong to
	// the NPC-dialog and zone-click paths respectively) and sections with a
	// /suj subject gate (NPC topic menu entries).  Sections marked played
	// are also skipped to match the one-shot semantics enforced by
	// interactNPC().  A fired section may /set(chapter=N) which reparses
	// _conScript, so break the moment that happens.
	const uint entryChapter = _currentCONChapter;

	for (ConSection &sec : _conScript.sections()) {
		if (!sec.isCon) continue;
		if (sec.played) continue;
		if (sec.sujId > 0) continue;
		bool gated = false;
		for (const ConCondition &cc : sec.conditions) {
			if (cc.key == "clicperso" || cc.key == "cliczone") {
				gated = true;
				break;
			}
		}
		if (gated) continue;

		// /con* (needsItem) auto sections require an object in hand --
		// match the same guard interactNPC uses for sujId==0 sections.
		if (sec.needsItem && !_inventory.selectedObject()) continue;

		if (!evalSelSection(sec, 0, (int)_currentPlaceId)) continue;

		// Several /con sections often share an id (CHAPI013 has four /014 success
		// sections + one /014 miss catch-all, all id 14).  Log the section's
		// gotoId and first /set command so success vs catch-all is unambiguous.
		Common::String firstSet;
		for (const ConSectionItem &it : sec.items) {
			if (it.isSet) { firstSet = it.setCmd; break; }
		}
		debugC(1, kDebugScript, "runGenericConSections: firing section %d gotoId=%d firstSet=%s",
		      sec.id, sec.gotoId, firstSet.c_str());
		sec.played = true;
		playConSection(sec);

		// Whether or not the chapter changed, the section was fired and the
		// click is consumed.  Returning true tells handleWarp to skip the
		// pickup / NPC-dialog / zone-navigation fall-through.
		(void)entryChapter;
		return true;
	}
	return false;
}

void CryOmni3DEngine_Atlantis::runTimerSections(int timerNum) {
	// Single-fire per scan, matching atlantis.exe FUN_0042bef4 (/tim1 scanner)
	// and FUN_0042bfa4 (/tim2 scanner).  Both walk the script in file order
	// for the first "/tim1" / "/tim2" directive whose condition evaluates true
	// via FUN_004298ac, then return the section offset to the caller -- the
	// caller fires exactly one section per tick.  Previously this loop would
	// run every matching /tim section in turn, which could stack cinematics
	// and /set commands on the same tick (e.g. two queued GameOvers from
	// overlapping safety nets).  The first-match break also sidesteps
	// iterator invalidation if playConSection /set(chapter=N) reparses
	// _conScript.
	for (ConSection &sec : _conScript.sections()) {
		if (!sec.isTimer || sec.timerNum != timerNum) continue;
		if (!evalSelSection(sec, 0, (int)_currentPlaceId)) continue;

		debugC(1, kDebugScript, "runTimerSections: timer=%d firing section %d", timerNum, sec.id);
		playConSection(sec);
		break;
	}
}

// ---------------------------------------------------------------------------
// NPC interaction
// ---------------------------------------------------------------------------

void CryOmni3DEngine_Atlantis::interactNPC(int persoId) {
	debugC(1, kDebugScript, "interactNPC: persoId=%d", persoId);
	debugDumpStoryState("interactNPC");
	// Expose clicperso as a game variable so evalSelSection can check it
	// against (clicperso=N) conditions on /con sections.
	_gameVars["clicperso"] = persoId;
	// New conversation: forget the last cam so the first line is free to pick
	// any 2..5.  Mirrors the original engine, where `DAT_00772908` (prev cam)
	// is consulted within a dialog but starts cleared at session entry.
	_lastDialCam = 0;
	// Capture the camera direction at the moment of the click; every default
	// per-line angle in this session is computed relative to this so the
	// hero's reverse-shot flip doesn't leak into the next NPC line.
	_dialBaseAlpha = _omni3dMan.getAlpha();
	_dialBaseAlphaValid = true;

	// atlantis.exe's click-on-perso handler (FUN_00425828) branches on the
	// perso's enabled-subject count (byte at 0x602f10 + (persoId-512)*0xE1):
	//   count > 0  -> open the topic-selection menu of enabled subjects;
	//   count == 0 -> no menu at all: just play the first /con(clicperso=N)
	//                 section whose conditions pass (the jbe 0x425abe path).
	// enableSuj/disableSuj are the ONLY things that ever change that count
	// (verified: those are the sole callers of the inc/dec at 0x602f10), so
	// "has a non-empty record in _sujEnabled" is exactly the exe's count>0.
	// Chapters 1-3 list every talkable perso in their /INIT enableSuj
	// commands; CHAPI004.CON has none — each chapter-4 NPC owns a single
	// /con(clicperso=N)/suj1 section and is meant to play directly on click.
	bool persoHasMenu = false;
	for (const SujState &ss : _sujEnabled)
		if (ss.persoId == persoId && !ss.sujs.empty()) { persoHasMenu = true; break; }

	// atlantis.exe's FUN_00425828 plays exactly ONE /con section per click and
	// returns: both of its FUN_0042844c (section player) call sites jump
	// straight to the function's exit label.  It never re-scans the section
	// list, so a /set in the line just played (e.g. broche30=1) cannot satisfy
	// and chain into a second section.  interactNPC() mirrors that: it collects
	// the candidate section(s), plays at most one, and returns — the topic menu
	// reappears only on the next click, exactly as in the original.  The
	// do/while(false) is a single pass whose early-out breaks share the cleanup.
	do {
		if (shouldAbort())
			break;

		// For each enabled subject, collect the first unplayed matching section.
		Common::Array<ConSection *> options;
		Common::Array<int> seenSujs;

		for (ConSection &sec : _conScript.sections()) {
			if (!sec.isCon) continue;
			// A /con section belongs to this NPC if any of its clicperso
			// conditions names it.  Test the conditions, not the single
			// sec.clicPerso field, so a `(clicperso=A)|(clicperso=B)` section
			// is offered for BOTH A and B — the field records only the last.
			// Sections with NO clicperso condition (e.g. CHAPI013 section
			// 92: `/con(vue=126)&(ObjetEnMain=261)&(angle...)&(wsprframe...)`
			// -- "use bottle on the priest aide at the right pose") are
			// allowed for any click, gated solely on their other conds.
			bool hasClicPerso = false;
			bool forThisPerso = false;
			// Also skip sections whose ONLY click constraint is a ClicZone
			// (those belong to runZoneConSections, not the per-NPC scan).
			bool hasClicZone = false;
			for (const ConCondition &cc : sec.conditions) {
				if (cc.key == "clicperso") {
					hasClicPerso = true;
					if (cc.value == persoId) forThisPerso = true;
				} else if (cc.key == "cliczone") {
					hasClicZone = true;
				}
			}
			if (hasClicPerso && !forThisPerso) continue;
			if (!hasClicPerso && hasClicZone) continue;
			debugC(2, kDebugScript, "interactNPC: checking section %d clicPerso=%d played=%d needsItem=%d sujId=%d",
			      sec.id, persoId, sec.played, sec.needsItem, sec.sujId);
			if (sec.played) { debugC(2, kDebugScript, "  -> skip: already played"); continue; }
			// `/con*` is NOT "needs item in hand" -- atlantis.exe
			// FUN_004281f8 (line 22141) treats the `*` as a depth-gate
			// override that makes the section always reachable, never a
			// hard "object required" prerequisite.  Sections that really
			// need a held item spell it out: every CHAPI*.CON `/con*`
			// that requires the inventory either carries an explicit
			// `(ObjetEnMain=N)` clause (CHAPI015 sections 196 / 197 /
			// 203-cut-rope, CHAPI013 section 178) or an `(invent<N>=...)`
			// clause (CHAPI015 sections 193 / 195).  Sections like
			// CHAPI015 `/203 /con*(ClicPerso=537)&(flagtemp2=1)` (the
			// Meljanz escape) deliberately fire with NO item held: the
			// player has just used /set(ObjetEnMain=0) when cutting the
			// rope.  The old auto-skip stranded the player at the inn
			// balcony unless they manually re-equipped the knife from
			// the toolbar (which wrongly opened the door anyway because
			// evalSelSection reads ObjetEnMain from selectedObject, not
			// the gameVar).  Drop the filter; let evalSelSection's
			// condition evaluation be the only gate.

			// Evaluate all /con conditions (objetenmain, game vars, etc.).
			if (!evalSelSection(sec, 0, (int)_currentPlaceId)) { debugC(2, kDebugScript, "  -> skip: conditions failed"); continue; }

			// A perso with no enabled subject is not menu-driven: the first
			// condition-passing /con(clicperso=N) section just plays — the
			// exe's count==0 branch.  A sujId-0 section (a scripted greeting
			// or give-object trigger) always plays directly the same way.
			//
			// An automatic section preempts the subject menu entirely: drop
			// any topic options already collected so it plays on its own.
			// Otherwise a sujId-0 section found *after* some sujId>0 topics
			// (e.g. perso 523's give-brooch section 78, which follows topic
			// section 75 in CHAPI005.CON) would be appended to the topic list
			// and handed to showSubjectMenu(), which has no icon for sujId 0
			// and renders that cell as a blank/duplicated border.
			if (sec.sujId == 0 || !persoHasMenu) {
				options.clear();
				options.push_back(&sec);
				break;
			}

			// Multi-sujid sections (`/suj1,2`): the section is available
			// when ANY listed subject is enabled.  The menu uses the
			// FIRST id as the icon and "already-seen" key; subsequent
			// ids are just additional fire conditions.  CHAPI005
			// section 113 (the game-over branch after GardePech1's
			// warning) depends on this -- a /suj1,2 stays available
			// when the player has disabled only one of the two.
			bool alreadySeen = false;
			for (int s : seenSujs) { if (s == sec.sujId) { alreadySeen = true; break; } }
			if (alreadySeen) { debugC(2, kDebugScript, "  -> skip: sujId=%d already seen", sec.sujId); continue; }

			bool anyEnabled = false;
			for (int s : sec.sujIds) {
				if (isSujEnabled(persoId, s)) { anyEnabled = true; break; }
			}
			if (!anyEnabled && sec.sujIds.empty() &&
			    isSujEnabled(persoId, sec.sujId)) {
				anyEnabled = true;   // legacy fallback if sujIds list empty
			}
			if (!anyEnabled) {
				debugC(2, kDebugScript, "  -> skip: none of sujIds enabled for perso %d", persoId);
				continue;
			}

			options.push_back(&sec);
			seenSujs.push_back(sec.sujId);
		}

		if (options.empty()) {
			debugC(1, kDebugScript, "interactNPC: persoId=%d no options found", persoId);
			break;
		}

		int choice;
		if (options.size() == 1 && (options[0]->sujId == 0 || !persoHasMenu)) {
			// A single automatic (sujId 0) section — a scripted greeting or
			// reaction — or any section of a perso with no enabled subject,
			// plays directly with no subject menu (exe FUN_00425828 count==0).
			choice = 0;
		} else {
			// One or more player-selectable subjects (sujId > 0): show the
			// subject menu, even for a single subject — the original always
			// lets the player choose the topic (or right-click to leave).
			choice = showSubjectMenu(options);
		}

		if (choice < 0 || shouldAbort()) break;

		ConSection *sec = options[(uint)choice];
		sec->played = true;
		playConSection(*sec);
	} while (false);

	_gameVars["clicperso"] = 0;
	_dialBaseAlphaValid = false;
}

void CryOmni3DEngine_Atlantis::decodeSprite(Common::SeekableReadStream &stream,
        SubjSprite &spr, const uint16 *colorTable) {
	// Per-row offset table: h x uint32, relative to the pixel-data base.
	Common::Array<uint32> rowOff(spr.h);
	for (uint16 r = 0; r < spr.h; r++)
		rowOff[r] = stream.readUint32LE();
	const int64 pixBase = stream.pos();

	spr.pixels.resize((uint)spr.w * spr.h);
	spr.blend.resize((uint)spr.w * spr.h);
	for (uint i = 0; i < spr.pixels.size(); i++) {
		spr.pixels[i] = kSprTransp;
		spr.blend[i]  = kSprNoBlend;
	}

	for (uint16 ry = 0; ry < spr.h; ry++) {
		stream.seek(pixBase + rowOff[ry]);
		int rx = 0;
		while (rx < spr.w && !stream.eos()) {
			byte b = stream.readByte();
			int btype = (b >> 6) & 0x3;
			int cnt   = b & 0x3F;
			if (btype == 0) {
				rx += cnt;  // transparent run
			} else if (btype == 1) {
				// Opaque run: cnt colour-index bytes.
				for (int i = 0; i < cnt && rx < spr.w; i++, rx++) {
					uint16 col = colorTable[stream.readByte()];
					if (col == kSprTransp) col = 0x0000;
					spr.pixels[ry * spr.w + rx] = col;
				}
			} else {
				// Blend run: cnt (factor, index) pairs.  The colour-table
				// entry is already premultiplied by factor/32 in the data
				// (atlantis.exe FUN_0041047f); keep the factor so drawing
				// can add the framebuffer's (32-factor)/32 share.
				for (int i = 0; i < cnt && rx < spr.w; i++, rx++) {
					byte factor = stream.readByte() & 0x1f;
					uint16 col  = colorTable[stream.readByte()];
					if (col == kSprTransp) col = 0x0000;
					spr.pixels[ry * spr.w + rx] = col;
					spr.blend[ry * spr.w + rx]  = factor;
				}
			}
		}
	}
}

void CryOmni3DEngine_Atlantis::loadSubjectSprites() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    openBigFileStream(kFileTypeSprite2D, "SUJETS.SPR"));
	if (!stream) {
		warning("Cannot open SPRITE\\2D\\SUJETS.SPR");
		return;
	}

	// flags word: bit 2 = has 256-entry color table stored as RGB555
	uint16 flags = stream->readUint16LE();

	uint16 colorTable[256];
	if (flags & 0x4) {
		// Color table is stored in RGB555 format; convert to RGB565.
		// Cannot use g_system->getScreenFormat() here — called before initGraphics().
		// Green is 5 bits in RGB555 but 6 bits in RGB565: shift g left 6 (= g*2) so it
		// fills bits [10:6] rather than [9:5], giving ~double the brightness.
		for (int i = 0; i < 256; i++) {
			uint16 rgb555 = stream->readUint16LE();
			uint16 r = (rgb555 >> 10) & 0x1f;
			uint16 g = (rgb555 >> 5)  & 0x1f;
			uint16 b =  rgb555        & 0x1f;
			colorTable[i] = (r << 11) | (g << 6) | b;
		}
	} else {
		memset(colorTable, 0, sizeof(colorTable));
	}
	if (flags & 0x8) {
		// 512-entry table: skip the extra 256 entries (already read 256 above)
		stream->skip(256 * 2);
	}

	uint16 count = stream->readUint16LE();
	if (count == 0 || count > 256) {
		warning("loadSubjectSprites: unexpected count %u", count);
		return;
	}

	// Sprite data section starts here (after flags + color table + count).
	// Offsets in the sprite table are relative to this position.
	const int64 sprDataBase = stream->pos();  // = 516 bytes in

	Common::Array<uint32> offsets(count);
	for (uint16 i = 0; i < count; i++)
		offsets[i] = stream->readUint32LE();

	_subjSprites.resize(count);

	for (uint16 si = 0; si < count; si++) {
		if (stream->seek(sprDataBase + offsets[si]) == false) {
			warning("loadSubjectSprites: seek failed for sprite %u", si);
			continue;
		}

		SubjSprite &spr = _subjSprites[si];
		spr.w    = stream->readUint16LE();
		spr.h    = stream->readUint16LE();
		spr.xoff = stream->readSint16LE();
		spr.yoff = stream->readSint16LE();

		if (spr.w == 0 || spr.h == 0 || spr.w > 640 || spr.h > 480)
			continue;

		decodeSprite(*stream, spr, colorTable);
	}

	debugC(1, kDebugScript, "loadSubjectSprites: loaded %u sprites from SUJETS.SPR", count);
}

void CryOmni3DEngine_Atlantis::loadCursorSprites() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    openBigFileStream(kFileTypeSprite2D, "CURSEURS.SPR"));
	if (!stream) {
		warning("loadCursorSprites: cannot open SPRITE\\2D\\CURSEURS.SPR — using fallback cursors");
		return;
	}

	uint16 flags = stream->readUint16LE();

	uint16 colorTable[256];
	memset(colorTable, 0, sizeof(colorTable));
	if (flags & 0x4) {
		for (int i = 0; i < 256; i++) {
			uint16 rgb555 = stream->readUint16LE();
			uint16 r = (rgb555 >> 10) & 0x1f;
			uint16 g = (rgb555 >> 5)  & 0x1f;
			uint16 b =  rgb555        & 0x1f;
			uint16 rgb565 = (uint16)((r << 11) | (g << 6) | b);
			// Remap the transparency sentinel to near-black to avoid false keying.
			colorTable[i] = (rgb565 == kWarpCursorKey) ? 0x0000u : rgb565;
		}
	}
	if (flags & 0x8)
		stream->skip(512);

	uint16 count = stream->readUint16LE();
	if (count == 0 || count > 256) {
		warning("loadCursorSprites: unexpected sprite count %u", count);
		return;
	}

	const int64 sprDataBase = stream->pos();
	Common::Array<uint32> offsets(count);
	for (uint16 i = 0; i < count; i++)
		offsets[i] = stream->readUint32LE();

	_cursorSprites.resize(count);

	for (uint16 si = 0; si < count; si++) {
		if (!stream->seek(sprDataBase + offsets[si])) {
			warning("loadCursorSprites: seek failed for sprite %u", si);
			continue;
		}
		CursorSpr &spr = _cursorSprites[si];
		spr.w    = stream->readUint16LE();
		spr.h    = stream->readUint16LE();
		spr.xoff = stream->readSint16LE();
		spr.yoff = stream->readSint16LE();

		if (spr.w == 0 || spr.h == 0 || spr.w > 64 || spr.h > 64)
			continue;

		Common::Array<uint32> rowOff(spr.h);
		for (uint16 r = 0; r < spr.h; r++)
			rowOff[r] = stream->readUint32LE();
		const int64 pixBase = stream->pos();

		spr.pixels.resize(spr.w * spr.h, kWarpCursorKey);
		spr.blend.resize(spr.w * spr.h, kSprNoBlend);

		for (uint16 ry = 0; ry < spr.h; ry++) {
			stream->seek(pixBase + rowOff[ry]);
			int rx = 0;
			while (rx < (int)spr.w && !stream->eos()) {
				byte b = stream->readByte();
				int btype = (b >> 6) & 0x3;
				int cnt   = b & 0x3F;
				if (btype == 0) {
					rx += cnt;
				} else if (btype == 1) {
					for (int i = 0; i < cnt && rx < (int)spr.w; i++, rx++) {
						byte idx = stream->readByte();
						spr.pixels[ry * spr.w + rx] = colorTable[idx];
					}
				} else {
					for (int i = 0; i < cnt && rx < (int)spr.w; i++, rx++) {
						byte factor = stream->readByte() & 0x1f;
						byte idx = stream->readByte();
						spr.pixels[ry * spr.w + rx] = colorTable[idx];
						spr.blend[ry * spr.w + rx]  = factor;
					}
				}
			}
		}
	}

	debugC(1, kDebugScript, "loadCursorSprites: loaded %u sprites from CURSEURS.SPR", count);
}

void CryOmni3DEngine_Atlantis::loadObjectSprites() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    openBigFileStream(kFileTypeSprite2D, "OBJETS1.SPR"));
	if (!stream) {
		warning("loadObjectSprites: cannot open SPRITE\\2D\\OBJETS1.SPR");
		return;
	}

	uint16 flags = stream->readUint16LE();

	uint16 colorTable[256];
	memset(colorTable, 0, sizeof(colorTable));
	if (flags & 0x4) {
		for (int i = 0; i < 256; i++) {
			uint16 rgb555 = stream->readUint16LE();
			uint16 r = (rgb555 >> 10) & 0x1f;
			uint16 g = (rgb555 >> 5)  & 0x1f;
			uint16 b =  rgb555        & 0x1f;
			colorTable[i] = (r << 11) | (g << 6) | b;
		}
	}
	if (flags & 0x8)
		stream->skip(256 * 2);

	uint16 count = stream->readUint16LE();
	if (count == 0 || count > 256) {
		warning("loadObjectSprites: unexpected count %u", count);
		return;
	}

	const int64 sprDataBase = stream->pos();

	Common::Array<uint32> offsets(count);
	for (uint16 i = 0; i < count; i++)
		offsets[i] = stream->readUint32LE();

	_objSprites.resize(count);

	for (uint16 si = 0; si < count; si++) {
		if (!stream->seek(sprDataBase + offsets[si])) {
			warning("loadObjectSprites: seek failed for sprite %u", si);
			continue;
		}

		SubjSprite &spr = _objSprites[si];
		spr.w    = stream->readUint16LE();
		spr.h    = stream->readUint16LE();
		spr.xoff = stream->readSint16LE();
		spr.yoff = stream->readSint16LE();

		if (spr.w == 0 || spr.h == 0 || spr.w > 640 || spr.h > 480)
			continue;

		decodeSprite(*stream, spr, colorTable);
	}

	debugC(1, kDebugScript, "loadObjectSprites: loaded %u sprites from OBJETS1.SPR", count);
}

void CryOmni3DEngine_Atlantis::loadInventoryBarSprite() {
	// OBJETS0.SPR holds the single 599x82 inventory-bar background sprite.
	// File format is identical to OBJETS1.SPR / SPRMENU.SPR: same SPR header,
	// 256 RGB555 color-table entries, optional skipped extra table, then a
	// sprite count (always 1 here) and the offset table.  We decode exactly
	// the same RLE rows as loadObjectSprites().
	_inventoryBarSprite = SubjSprite();
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    openBigFileStream(kFileTypeSprite2D, "OBJETS0.SPR"));
	if (!stream) {
		warning("loadInventoryBarSprite: cannot open SPRITE\\2D\\OBJETS0.SPR");
		return;
	}

	uint16 flags = stream->readUint16LE();

	uint16 colorTable[256];
	memset(colorTable, 0, sizeof(colorTable));
	if (flags & 0x4) {
		for (int i = 0; i < 256; i++) {
			uint16 rgb555 = stream->readUint16LE();
			uint16 r = (rgb555 >> 10) & 0x1f;
			uint16 g = (rgb555 >> 5)  & 0x1f;
			uint16 b =  rgb555        & 0x1f;
			colorTable[i] = (r << 11) | (g << 6) | b;
		}
	}
	if (flags & 0x8)
		stream->skip(256 * 2);

	uint16 count = stream->readUint16LE();
	if (count != 1) {
		warning("loadInventoryBarSprite: expected 1 sprite, got %u", count);
		return;
	}

	const int64 sprDataBase = stream->pos();
	uint32 spr0Off = stream->readUint32LE();

	if (!stream->seek(sprDataBase + spr0Off)) {
		warning("loadInventoryBarSprite: seek failed");
		return;
	}

	SubjSprite &spr = _inventoryBarSprite;
	spr.w    = stream->readUint16LE();
	spr.h    = stream->readUint16LE();
	spr.xoff = stream->readSint16LE();
	spr.yoff = stream->readSint16LE();
	if (spr.w == 0 || spr.h == 0 || spr.w > 1024 || spr.h > 256) {
		warning("loadInventoryBarSprite: bad dims %ux%u", spr.w, spr.h);
		spr = SubjSprite();
		return;
	}

	decodeSprite(*stream, spr, colorTable);

	debugC(1, kDebugScript, "loadInventoryBarSprite: loaded %ux%u sprite from OBJETS0.SPR (hotspot %d,%d)",
	      spr.w, spr.h, spr.xoff, spr.yoff);
}

Graphics::Surface *CryOmni3DEngine_Atlantis::loadTGA(FileType ft, const char *name) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(openBigFileStream(ft, name));
	if (!stream) {
		warning("loadTGA: cannot open %s", name);
		return nullptr;
	}

	// TGA header (18 bytes): all game TGAs are type 2 (uncompressed true-color), 640x480, depth=15
	byte  idLen    = stream->readByte();
	/* cmType = */ stream->readByte();
	byte  imgType  = stream->readByte();
	stream->skip(9); // color-map spec (5) + x/y origin (4)
	uint16 w       = stream->readUint16LE();
	uint16 h       = stream->readUint16LE();
	byte   depth   = stream->readByte();
	stream->readByte(); // image descriptor
	stream->skip(idLen); // optional ID field

	if (imgType != 2 || depth != 15 || w == 0 || h == 0) {
		warning("loadTGA: unexpected format in %s (type=%u depth=%u %ux%u)", name, imgType, depth, w, h);
		return nullptr;
	}

	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::Surface *surf = new Graphics::Surface();
	surf->create(w, h, fmt);

	for (uint16 y = 0; y < h; y++) {
		// TGA origin is bottom-left by default (image_descriptor bit5 = 0)
		uint16 *row = (uint16 *)surf->getBasePtr(0, h - 1 - y);
		for (uint16 x = 0; x < w; x++) {
			uint16 px = stream->readUint16LE();
			uint8 r = ((px >> 10) & 0x1f) << 3;
			uint8 g = ((px >>  5) & 0x1f) << 3;
			uint8 b = ( px        & 0x1f) << 3;
			row[x] = (uint16)fmt.RGBToColor(r, g, b);
		}
	}

	return surf;
}

void CryOmni3DEngine_Atlantis::blitMenuSprite(Graphics::ManagedSurface &dst, uint idx, int hx, int hy) const {
	if (idx >= _menuSprites.size() || _menuSprites[idx].pixels.empty())
		return;
	const SubjSprite &s = _menuSprites[idx];
	int baseX = hx - s.xoff;
	int baseY = hy - s.yoff;
	for (int sy = 0; sy < (int)s.h; sy++) {
		int dy = baseY + sy;
		if (dy < 0 || dy >= dst.h)
			continue;
		for (int sx = 0; sx < (int)s.w; sx++) {
			int dx = baseX + sx;
			if (dx < 0 || dx >= dst.w)
				continue;
			uint sidx = sy * s.w + sx;
			uint16 pix = s.pixels[sidx];
			if (pix == kSprTransp)
				continue;
			uint16 *dp = (uint16 *)dst.getBasePtr(dx, dy);
			if (s.blend[sidx] != kSprNoBlend)
				pix = blendSprPixel565(pix, *dp, s.blend[sidx]);
			*dp = pix;
		}
	}
}

void CryOmni3DEngine_Atlantis::loadMenuSprites() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    openBigFileStream(kFileTypeSprite2D, "SPRMENU.SPR"));
	if (!stream) {
		warning("loadMenuSprites: cannot open SPRITE\\2D\\SPRMENU.SPR");
		return;
	}

	uint16 flags = stream->readUint16LE();

	uint16 colorTable[256];
	memset(colorTable, 0, sizeof(colorTable));
	if (flags & 0x4) {
		for (int i = 0; i < 256; i++) {
			uint16 rgb555 = stream->readUint16LE();
			uint16 r = (rgb555 >> 10) & 0x1f;
			uint16 g = (rgb555 >> 5)  & 0x1f;
			uint16 b =  rgb555        & 0x1f;
			colorTable[i] = (r << 11) | (g << 6) | b;
		}
	}
	if (flags & 0x8)
		stream->skip(256 * 2);

	uint16 count = stream->readUint16LE();
	if (count == 0 || count > 256) {
		warning("loadMenuSprites: unexpected count %u", count);
		return;
	}

	const int64 sprDataBase = stream->pos();

	Common::Array<uint32> offsets(count);
	for (uint16 i = 0; i < count; i++)
		offsets[i] = stream->readUint32LE();

	_menuSprites.resize(count);

	for (uint16 si = 0; si < count; si++) {
		if (!stream->seek(sprDataBase + offsets[si])) {
			warning("loadMenuSprites: seek failed for sprite %u", si);
			continue;
		}

		SubjSprite &spr = _menuSprites[si];
		spr.w    = stream->readUint16LE();
		spr.h    = stream->readUint16LE();
		spr.xoff = stream->readSint16LE();
		spr.yoff = stream->readSint16LE();

		if (spr.w == 0 || spr.h == 0 || spr.w > 640 || spr.h > 480)
			continue;

		decodeSprite(*stream, spr, colorTable);
	}

}

void CryOmni3DEngine_Atlantis::loadFontMaxSprites() {
	// Default and hover glyph sets share the same SPR format.  FONTMAX/FONTMAX2
	// are the gold menu font; FONTMIN/FONTMIN2 the smaller red player-name font.
	loadFontSpriteSet("FONTMAX.SPR",  _fontMaxSprites);
	loadFontSpriteSet("FONTMAX2.SPR", _fontMaxHoverSprites);
	loadFontSpriteSet("FONTMIN.SPR",  _fontMinSprites);
	loadFontSpriteSet("FONTMIN2.SPR", _fontMin2Sprites);
}

void CryOmni3DEngine_Atlantis::loadCreditFontSprites() {
	// Two glyph banks for the credits roll.  Same SPR format as FONTMAX.
	loadFontSpriteSet("CREDBLAN.SPR", _creditBlanSprites);
	loadFontSpriteSet("CREDBLEU.SPR", _creditBleuSprites);
}

void CryOmni3DEngine_Atlantis::loadFontSpriteSet(const char *sprName,
        Common::Array<SubjSprite> &out) {
	out.clear();
	Common::ScopedPtr<Common::SeekableReadStream> stream(
	    openBigFileStream(kFileTypeSprite2D, sprName));
	if (!stream) {
		warning("loadFontSpriteSet: cannot open SPRITE\\2D\\%s", sprName);
		return;
	}

	uint16 flags = stream->readUint16LE();

	uint16 colorTable[256];
	memset(colorTable, 0, sizeof(colorTable));
	if (flags & 0x4) {
		for (int i = 0; i < 256; i++) {
			uint16 rgb555 = stream->readUint16LE();
			uint16 r = (rgb555 >> 10) & 0x1f;
			uint16 g = (rgb555 >> 5)  & 0x1f;
			uint16 b =  rgb555        & 0x1f;
			colorTable[i] = (r << 11) | (g << 6) | b;
		}
	}
	if (flags & 0x8)
		stream->skip(256 * 2);

	uint16 count = stream->readUint16LE();
	if (count == 0 || count > 256) {
		warning("loadFontSpriteSet(%s): unexpected count %u", sprName, count);
		return;
	}

	const int64 sprDataBase = stream->pos();

	Common::Array<uint32> offsets(count);
	for (uint16 i = 0; i < count; i++)
		offsets[i] = stream->readUint32LE();

	out.resize(count);

	for (uint16 si = 0; si < count; si++) {
		if (!stream->seek(sprDataBase + offsets[si])) {
			warning("loadFontSpriteSet(%s): seek failed for sprite %u", sprName, si);
			continue;
		}

		SubjSprite &spr = out[si];
		spr.w    = stream->readUint16LE();
		spr.h    = stream->readUint16LE();
		spr.xoff = stream->readSint16LE();
		spr.yoff = stream->readSint16LE();

		if (spr.w == 0 || spr.h == 0 || spr.w > 640 || spr.h > 480)
			continue;

		decodeSprite(*stream, spr, colorTable);
	}
}

void CryOmni3DEngine_Atlantis::drawInventoryIcon(Graphics::ManagedSurface &dst,
        const Object *obj, const Common::Rect &r) {
	if (_objSprites.empty() || obj == nullptr)
		return;

	// Find this object's index in _objects via pointer arithmetic.
	if (_objects.empty())
		return;
	ptrdiff_t idx = obj - &_objects.front();
	if (idx < 0 || (uint)idx >= _objSprites.size())
		return;

	const SubjSprite &spr = _objSprites[(uint)idx];
	if (spr.pixels.empty())
		return;

	// Center sprite within the slot rect.
	int cx = (r.left + r.right)  / 2;
	int cy = (r.top  + r.bottom) / 2;
	// xoff/yoff = hotspot (center of sprite); compute top-left corner.
	int dx = cx - spr.xoff;
	int dy = cy - spr.yoff;

	// Clip ONLY to the destination surface, never to the slot rect.  The slot
	// is 50x40 but OBJETS1.SPR icons run up to 60x60 (e.g. sprites 33/44), so
	// clipping to the slot would trim the top/bottom of most items.  The
	// original engine's icon blit (atlantis.exe FUN_0041047f) clips against the
	// global render-target window, not a per-slot box: the icon draws full size
	// centred on the slot, overflowing into the bar's vertical margin (the 70px
	// column spacing — slot centres 70px apart — leaves room horizontally for
	// the ≤60px-wide icons).  `r` here is used only to centre the icon.
	for (int y = 0; y < (int)spr.h; y++) {
		int sy = dy + y;
		if (sy < 0 || sy >= (int)dst.h) continue;
		for (int x = 0; x < (int)spr.w; x++) {
			int sx = dx + x;
			if (sx < 0 || sx >= (int)dst.w) continue;
			uint sidx = y * spr.w + x;
			uint16 pix = spr.pixels[sidx];
			if (pix == kSprTransp) continue;
			uint16 *dp = (uint16 *)dst.getBasePtr(sx, sy);
			if (spr.blend[sidx] != kSprNoBlend)
				pix = blendSprPixel565(pix, *dp, spr.blend[sidx]);
			*dp = pix;
		}
	}
}

void CryOmni3DEngine_Atlantis::drawGivenObject(Graphics::Surface &dst) const {
	if (_dialogGivenObjSprite < 0 || (uint)_dialogGivenObjSprite >= _objSprites.size())
		return;
	const SubjSprite &spr = _objSprites[(uint)_dialogGivenObjSprite];
	if (spr.pixels.empty())
		return;
	// Centre horizontally; sit near the bottom, above the subtitle strip.
	// (The original draws the given object's icon centre-bottom on every
	// dialog frame via DAT_0049619c; the exact Y is not recoverable from the
	// decompiler, so anchor the icon's bottom a fixed gap above the screen
	// edge.)
	const int cx = dst.w / 2;
	const int objBottom = dst.h - 56;          // leave room for subtitles below
	const int cy = objBottom - ((int)spr.h - spr.yoff);  // hotspot Y for that bottom
	const int dx = cx - spr.xoff;
	const int dy = cy - spr.yoff;
	for (int y = 0; y < (int)spr.h; y++) {
		int sy = dy + y;
		if (sy < 0 || sy >= dst.h) continue;
		for (int x = 0; x < (int)spr.w; x++) {
			int sx = dx + x;
			if (sx < 0 || sx >= dst.w) continue;
			uint sidx = y * spr.w + x;
			uint16 pix = spr.pixels[sidx];
			if (pix == kSprTransp) continue;
			uint16 *dp = (uint16 *)dst.getBasePtr(sx, sy);
			if (spr.blend[sidx] != kSprNoBlend)
				pix = blendSprPixel565(pix, *dp, spr.blend[sidx]);
			*dp = pix;
		}
	}
}

void CryOmni3DEngine_Atlantis::drawSubjectSprite(Graphics::Surface &dst,
        uint sprIdx, int cx, int cy) const {
	if (sprIdx >= _subjSprites.size())
		return;
	const SubjSprite &spr = _subjSprites[sprIdx];
	if (spr.pixels.empty())
		return;

	// cx,cy = center point; xoff,yoff = hotspot offset from top-left
	int dx = cx - spr.xoff;
	int dy = cy - spr.yoff;

	for (int y = 0; y < (int)spr.h; y++) {
		int sy = dy + y;
		if (sy < 0 || sy >= dst.h) continue;
		for (int x = 0; x < (int)spr.w; x++) {
			int sx = dx + x;
			if (sx < 0 || sx >= dst.w) continue;
			uint sidx = y * spr.w + x;
			uint16 pix = spr.pixels[sidx];
			if (pix == kSprTransp) continue;
			uint16 *dp = (uint16 *)dst.getBasePtr(sx, sy);
			if (spr.blend[sidx] != kSprNoBlend)
				pix = blendSprPixel565(pix, *dp, spr.blend[sidx]);
			*dp = pix;
		}
	}
}

// Subject-choice menu, reproduced from atlantis.exe's subject grid (draw at
// FUN_004261a0, hit-test at FUN_004211f0 — both confirmed against the
// decompilation and a dynamic CreateFileA/blit trace of the original):
//
//  - Each subject is two sprites: the border (SUJETS.SPR sprite 0) drawn at
//    the cell's top-left, then the icon (sprite = sujId) at the cell centre,
//    i.e. icon anchor = border anchor + (25, 25).
//  - Cells are 50x50; 4 rows per column; columns step 50 px in X from X=450.
//  - For <=4 subjects the single column is bottom-anchored
//    (startY = 330 - 50*N); for >=5 the grid starts at Y=180.
//  - The original draws the icons statically: there is NO hover highlight and
//    NO on-screen "exit" affordance — a right-click cancels.
//
// Returns the chosen option index, or -1 on cancel.
int CryOmni3DEngine_Atlantis::showSubjectMenu(const Common::Array<ConSection *> &options) {
	const int kScreenW = 640, kScreenH = 480;
	const uint nOpts = options.size();
	if (nOpts == 0)
		return -1;

	const int kBorderX  = 450;  // exe 0x1c2 — column-0 cell origin X
	const int kColStepX = 50;
	const int kRowStepY = 50;
	const int kCell     = 50;   // exe FUN_004211f0 hit-test cell (0x32)
	const int kIconDX   = 25;   // icon anchor = border (cell top-left) + (25,25)
	const int kIconDY   = 25;
	const int kPerCol   = 4;

	// Column start Y: a single bottom-anchored column for <=4 subjects, a
	// fixed grid from Y=180 once a second column is needed.
	const int startY = (nOpts < 5) ? (int)(330 - 50 * nOpts) : 180;

	// Border-sprite anchor (= cell top-left) for each subject.
	struct IconLayout { int bx, by; uint optIdx; };
	Common::Array<IconLayout> layout;
	for (uint i = 0; i < nOpts; i++) {
		int col = (int)i / kPerCol;
		int row = (int)i % kPerCol;
		layout.push_back({kBorderX + col * kColStepX, startY + row * kRowStepY, i});
	}

	Graphics::PixelFormat fmt = g_system->getScreenFormat();
	Graphics::ManagedSurface frame(kScreenW, kScreenH, fmt);

	showMouse(true);
	setArrowCursor();

	// Drain the button press that opened the menu (the player just clicked the
	// NPC) so it is not consumed as an immediate — and missing — selection,
	// which would close the menu the instant it appeared.
	while (getCurrentMouseButton() != 0 && !shouldAbort()) {
		pollEvents();
		g_system->delayMillis(10);
	}

	int result = -1;
	bool done = false;
	while (!done && !shouldAbort()) {
		pollEvents();

		// Panoramic background → frame buffer.
		if (_warpLoaded) {
			const Graphics::Surface *bg = _omni3dMan.getSurface();
			if (bg)
				frame.blitFrom(*bg);
			else
				frame.fillRect(Common::Rect(kScreenW, kScreenH), 0);
		} else {
			frame.fillRect(Common::Rect(kScreenW, kScreenH), 0);
		}

		Graphics::Surface *rawPtr = frame.surfacePtr();

		// Each subject: border (sprite 0) first, then the icon on top — the
		// original draws every icon identically, with no hover highlight.
		for (const IconLayout &il : layout) {
			drawSubjectSprite(*rawPtr, 0, il.bx, il.by);
			int sujId = options[il.optIdx]->sujId;
			uint sprIdx = (sujId > 0 && (uint)sujId < _subjSprites.size())
			              ? (uint)sujId : 0;
			drawSubjectSprite(*rawPtr, sprIdx, il.bx + kIconDX, il.by + kIconDY);
		}

		g_system->copyRectToScreen(frame.getPixels(), frame.pitch, 0, 0, kScreenW, kScreenH);
		g_system->updateScreen();

		if (getCurrentMouseButton() == 1) {
			Common::Point click = getMousePos();
			while (getCurrentMouseButton() != 0 && !shouldAbort()) {
				pollEvents();
				g_system->delayMillis(10);
			}
			// 50x50 hit cell anchored at each subject's border (top-left).
			for (const IconLayout &il : layout) {
				if (click.x >= il.bx && click.x < il.bx + kCell &&
				    click.y >= il.by && click.y < il.by + kCell) {
					result = (int)il.optIdx;
					break;
				}
			}
			done = true;
		} else if (getCurrentMouseButton() == 2) {
			done = true;   // right-click cancels
		}

		g_system->delayMillis(10);
	}

	showMouse(false);
	return result;
}

// Build the HNM filename for a dialog line's background video.
// speaker + params="camN" → "<SPEAKER_UPPER>N.HNM"; params="off"/empty → no video.
void CryOmni3DEngine_Atlantis::loadDialInitData() {
	Common::ScopedPtr<Common::SeekableReadStream> s(
	    openBigFileStream(kFileTypeDialog, "INITDIAL.TXT"));
	if (!s) {
		warning("loadDialInitData: cannot open DIALOG\\INITDIAL.TXT");
		return;
	}

	uint32 sz = (uint32)s->size();
	Common::Array<char> buf(sz + 1);
	s->read(buf.data(), sz);
	buf[sz] = '\0';

	const char *p = buf.data();
	while (*p) {
		while (*p && (*p == '\r' || *p == '\n' || *p == ' ' || *p == '\t')) p++;
		if (!*p) break;

		Common::Array<Common::String> tok;
		while (*p && *p != '\r' && *p != '\n') {
			while (*p && (*p == ' ' || *p == '\t')) p++;
			if (!*p || *p == '\r' || *p == '\n') break;
			const char *te = p;
			while (*te && *te != ' ' && *te != '\t' && *te != '\r' && *te != '\n') te++;
			tok.push_back(Common::String(p, (uint32)(te - p)));
			p = te;
		}

		if (tok.empty() || tok[0].equalsIgnoreCase("FIN")) break;
		// Format: persoId unk anim3DC anim3DA animM s3d cam2 cam3 cam4 cam5 [spf2..spf5]
		if (tok.size() < 10) continue;

		int persoId = atoi(tok[0].c_str());
		if (persoId <= 0) continue;

		// Detect the player character (hero) by the s3d field being "hero".
		if (tok[5].equalsIgnoreCase("hero"))
			_heroPersoId = persoId;

		DialInitEntry entry;
		for (int i = 0; i < 4; i++)
			entry.camUBB[i] = tok[6 + i];  // cam2..cam5 basenames

		// Column 4 (0-indexed) is the animM basename — the per-character
		// mouth vertex-animation file at DIALOG\PERSO\<base>.3DA.  The
		// trailing "m" in the basename is a Cryo convention (e.g.
		// "heroan1m" → HEROAN1M.3DA).  "none" is used as a sentinel for
		// characters that don't ship a mouth file.
		if (!tok[4].equalsIgnoreCase("none"))
			entry.animMBase = tok[4];

		// Column 5 (0-indexed) is the s3d basename — the per-character
		// scene-graph mesh at DIALOG\PERSO\<base>.S3D.  Multiple persoIds
		// share a mesh (every "Garde" persoId points at "garde"), so the
		// 20 .S3D files on disc cover the ~67 INITDIAL entries.  Used by
		// F3dcMesh to load the triangle topology that maps vertices in
		// the animM pool into drawable triangles.
		if (!tok[5].equalsIgnoreCase("none"))
			entry.s3dBase = tok[5];

		// Column 2 (0-indexed) is the anim3DC basename — the per-character
		// body file at DIALOG\PERSO\<base>.3DC.  Despite the name it also
		// carries the dialog mouth MESH: an "fvi" scene-graph node with the
		// bind-pose vertices and the triangle topology, decoded by
		// F3dcMouthMesh.
		if (!tok[2].equalsIgnoreCase("none"))
			entry.anim3DCBase = tok[2];

		// Columns 10..13 (when present) carry the per-cam SPF overlay basenames.
		// Rows lacking these columns leave the SPF slots empty; "none" tokens
		// also map to empty per the original sentinel convention used for camUBB.
		// Examples:
		//   persoId 567 (Sorciere):    none vispf1c3 vispf1c4 vispf1c5
		//   persoId 667 (EnnemiPilot): none bapan1k3 bapan1k4 bapan1k5
		for (int i = 0; i < 4; i++) {
			if (10 + i < (int)tok.size() && !tok[10 + i].equalsIgnoreCase("none"))
				entry.camSPF[i] = tok[10 + i];
		}

		_dialInitData[persoId] = entry;
	}

	debugC(1, kDebugScript, "loadDialInitData: %u entries from INITDIAL.TXT, heroPersoId=%d",
	      _dialInitData.size(), _heroPersoId);
}

// Per-line camera picker.  Re-implements the random-cam loop from
// atlantis.exe.c FUN_0042844c, lines 22460-22470:
//
//   while (cam == prevCam || INITDIAL[persoId].camUBB[cam-2] is "none") {
//       cam = (rand() & 3) + 2;        // 2..5
//   }
//
// When the CON directive supplies an explicit `camN` (`explicitCam` != 0),
// the original honours it directly; we do the same but still validate
// against `_dialInitData`, falling through to the first available cam if
// the requested one is unavailable.  Returns 2..5.  `_lastDialCam` is
// updated by the caller after the line plays so a fallback failure
// (e.g. no valid cams at all) doesn't poison the next pick.
int CryOmni3DEngine_Atlantis::pickDialCam(int persoId, int explicitCam) {
	// Look up which cam variants exist for this speaker.  Treat "none" /
	// empty as unavailable.
	bool avail[4] = { false, false, false, false };
	int availCount = 0;
	Common::HashMap<int, DialInitEntry>::const_iterator it =
	    _dialInitData.find(persoId);
	if (it != _dialInitData.end()) {
		for (int i = 0; i < 4; i++) {
			const Common::String &s = it->_value.camUBB[i];
			if (!s.empty() && !s.equalsIgnoreCase("none")) {
				avail[i] = true;
				availCount++;
			}
		}
	}

	// Explicit cam from `[Speaker, X, Y, camN]`: honour it if valid+available,
	// otherwise fall through to the random pick below.
	if (explicitCam >= 2 && explicitCam <= 5) {
		if (availCount == 0 || avail[explicitCam - 2])
			return explicitCam;
	}

	// No INITDIAL data at all → keep deterministic cam2 close-up.
	if (availCount == 0)
		return 2;

	// Random pick — same `(rand & 3) + 2` shape as the original, rejecting
	// previous-cam and unavailable cams.  If the only available cam is the
	// previous one (avoid impossible), we still return it.
	int cam = 2;
	for (int tries = 0; tries < 16; tries++) {
		cam = (int)_dialRng.getRandomNumber(3) + 2;  // 2..5
		if (cam == _lastDialCam && availCount > 1)
			continue;
		if (avail[cam - 2])
			return cam;
	}
	// Fallback: first available cam (favouring something other than last).
	for (int c = 2; c <= 5; c++)
		if (avail[c - 2] && c != _lastDialCam)
			return c;
	for (int c = 2; c <= 5; c++)
		if (avail[c - 2])
			return c;
	return 2;
}

void CryOmni3DEngine_Atlantis::playConSection(ConSection &sec) {
	// The cursor is hidden for the duration of dialog playback; remember its
	// prior visibility so it is restored to exactly that when the section ends.
	bool cursorWasVisible = showMouse(false);

	// Drain the input gesture that triggered us before any line starts.
	// Departure /sel sections (e.g. zone 146 → Seth's "off" line) fire while
	// the player still holds down the click that initiated the navigation;
	// the per-line wait loop reads that held button as "skip this line" and
	// aborts immediately, swallowing the subtitle and audio.  Consume the
	// held button + any buffered keypresses here so the skip gesture only
	// counts when the player clicks/presses *during* playback.
	while (getCurrentMouseButton() != 0 && !shouldAbort()) {
		pollEvents();
		g_system->delayMillis(10);
	}
	clearKeys();

	// Execute items in source order so cinematics and music play at the right moment
	// relative to dialog lines (CON files freely interleave /set commands with dialog).
	int lineIdx = 0;
	// GereGameOver hand-off: a /set(gameover=N) command (N != 0) ends the
	// section immediately (atlantis.exe FUN_0042844c breaks its line loop the
	// moment the GameOver variable becomes non-zero) and the lines that follow
	// are handed to the game-over screen instead of being played as ordinary
	// dialog.  Captured here, run after the loop.
	int            pendingGameOver = 0;
	Common::String gameOverText;
	char           gameOverTrack   = 'A';
	// A /set(chapter=N) item reloads _conScript, freeing `sec` and every
	// section the caller is iterating.  Remember the chapter on entry so the
	// item loop can stop the instant that happens (see the check below).
	const uint entryChapter = _currentCONChapter;
	for (uint itemIdx = 0; itemIdx < sec.items.size(); ++itemIdx) {
		const ConSectionItem &item = sec.items[itemIdx];
		if (shouldAbort()) break;
		if (item.isSet) {
			// /set(gameover=N): N != 0 triggers the game-over screen.
			if (scumm_strnicmp(item.setCmd.c_str(), "gameover=", 9) == 0 &&
			    atoi(item.setCmd.c_str() + 9) != 0) {
				execConSetCommand(item.setCmd);
				pendingGameOver = atoi(item.setCmd.c_str() + 9);
				// The narrator commentary is the first dialog line after the
				// command; its APC track letter is its line index within the
				// section (atlantis.exe GereGameOver counts '[' lines from the
				// section header to the first [Narratrice] line).
				for (uint j = itemIdx + 1; j < sec.items.size(); ++j) {
					if (!sec.items[j].isSet) {
						gameOverText  = sec.items[j].line.text;
						gameOverTrack = (char)('A' + lineIdx);
						break;
					}
				}
				break;
			}
			const uint placeBeforeSet = _nextPlaceId;
			execConSetCommand(item.setCmd);
			// A /set(chapter=N) command reloaded _conScript: `sec`, `item`
			// and the section list are now freed memory.  Stop at once —
			// re-reading sec.items below would touch the dangling buffer.
			// The new chapter's init /set commands have already run inside
			// loadConScript(); any trailing items of this section are
			// abandoned, exactly as the original engine drops the rest of a
			// section once its chapter advance fires.
			if (_currentCONChapter != entryChapter)
				break;
			// A /set(vue=N) inside a dialog section moves the conversation
			// to a new place.  vue= is normally deferred to the game loop's
			// doPlaceChange(), but a dialog section can place vue= *before*
			// its closing lines (e.g. CHAPI005 section 78 — the talk follows
			// the fisherman from place 139 to vue=145 before its last line).
			// Those trailing lines must composite over the destination
			// cyclo, not the one the dialog opened on, so swap the clean
			// cyclo now.  _nextPlaceId stays set: the real place change —
			// transition video, arrival /sel sections — still runs through
			// the normal path once the section ends.
			if (_nextPlaceId != uint(-1) && _nextPlaceId != placeBeforeSet) {
				bool laterLine = false;
				for (uint j = itemIdx + 1; j < sec.items.size(); ++j)
					if (!sec.items[j].isSet) { laterLine = true; break; }
				const AtlantisPlace *destPlace = laterLine
				    ? _wam.findPlaceById((uint16)_nextPlaceId) : nullptr;
				if (destPlace && loadCycloHNM(destPlace->cycloHnm))
					debugC(1, kDebugScript, "playConSection: vue=%u mid-section — clean "
					         "cyclo swapped for trailing dialog lines",
					      _nextPlaceId);
			}
		} else {
			Common::String vid;
			// Resolve the on-screen character from the line's speaker name
			// via the exe's speaker → persoId table.  sec.clicPerso (the NPC
			// the player clicked) is deliberately not used: a dialog freely
			// interleaves lines from several characters, so only the speaker
			// name identifies who is actually talking on this line.
			int persoId = resolveSpeakerPersoId(item.line.speaker);
			// Pick the camera shot.  When the CON directive has an explicit
			// `camN` we honour it; otherwise pickDialCam() randomises in 2..5
			// the way the original engine does (avoiding the previous cam
			// and any cam whose UBB resource is missing).  Both the FOV
			// (zoom) and the UBB filename are keyed off the chosen cam.
			int explicitCam = 0;
			{
				const char *camTag = strstr(item.line.params.c_str(), "cam");
				if (camTag) sscanf(camTag, "cam%d", &explicitCam);
				if (explicitCam < 2 || explicitCam > 5) explicitCam = 0;
			}
			int camNum = pickDialCam(persoId, explicitCam);
			Common::String spfBase;
			Common::String animMBase;
			Common::String s3dBase;
			Common::String anim3DCBase;
			// Skip per-perso UBB / SPF lookup for any off-camera spelling
			// (`off`, `off0`, `off1`) -- the line plays as voiceover with no
			// head shot, so the resource basenames are irrelevant.
			const bool _offForLookup = item.line.params.equalsIgnoreCase("off")
			                       || item.line.params.equalsIgnoreCase("off0")
			                       || item.line.params.equalsIgnoreCase("off1");
			if (persoId > 0 && !_offForLookup) {
				Common::HashMap<int, DialInitEntry>::const_iterator it =
				    _dialInitData.find(persoId);
				if (it != _dialInitData.end()) {
					Common::String fn = it->_value.camUBB[camNum - 2];
					if (!fn.empty() && !fn.equalsIgnoreCase("none")) {
						fn.toUppercase();
						vid = Common::String("PERSO\\") + fn + ".UBB";
					}
					// Per-cam SPF overlay (INITDIAL columns 10..13).  Parser
					// already strips "none" to empty, so a non-empty basename
					// here means we have an overlay to composite.
					spfBase = it->_value.camSPF[camNum - 2];
					// Per-character mouth vertex animation (INITDIAL column 5).
					// Single basename across all cams — the mouth mesh moves
					// in 3D and the camera projects whichever cam is chosen.
					animMBase = it->_value.animMBase;
					// Per-character mesh topology (INITDIAL column 6).  Holds
					// the triangle indices the future rasteriser will use to
					// turn the animM vertex pool into solid polygons.
					s3dBase = it->_value.s3dBase;
					// Per-character body file (INITDIAL column 2) — carries
					// the dialog mouth mesh (verts + triangle topology) in
					// its "fvi" node, decoded by F3dcMouthMesh.
					anim3DCBase = it->_value.anim3DCBase;
				}
			}
			// Remember for the next line so consecutive lines don't repeat.
			_lastDialCam = camNum;
			// Effective per-line angle, matching the original engine
			// (atlantis.exe.c FUN_0042844c lines 22497–22541):
			//
			//   • Explicit `[Speaker, X, Y, camN]` (hasAngle):
			//       alpha = X°, beta = Y°   (line 22543)
			//   • `[Speaker, off]` (off-camera narrator):
			//       alpha = current + FOV°, beta = 0  (line 22516–22522)
			//   • Bare / camN-only lines:
			//       alpha = current,        beta = 0  (line 22536)
			//
			// Common to all three: beta is always normalised — either set
			// explicitly or snapped to 0 (horizon).  That's the only
			// automatic motion the original applies for un-annotated
			// lines.  There is *no* "rotate-to-face-the-speaker" fallback
			// in the original; the natural per-character motion comes
			// from authored X,Y on the lead line of each dialog branch
			// (~24% of lines), with FPS-mode aiming doing the rest (the
			// player is already pointing at the NPC by the time the
			// click registers, so preserving alpha lands on-target).
			bool effHasAngle = item.line.hasAngle;
			int  effAngleX   = item.line.angleX;
			int  effAngleY   = item.line.angleY;
			// Off-camera marker: the dialog directive tail token can be `off`,
			// `off0`, or `off1`.  atlantis.exe FUN_0042844c (lines 22418-22431)
			// sets three distinct flag globals (DAT_004960b4/b8/bc) for the
			// three spellings, but every consumer downstream treats "any flag
			// non-zero" the same way -- suppress the UBB head, skip the per-cam
			// FOV change, leave the panorama where the player was facing.  So
			// from the renderer's view, off / off0 / off1 are interchangeable.
			// Match all three here (the old code only matched the bare "off",
			// silently treating `[Narratrice,off0]` lines as on-camera and
			// flipping the cyclo for a head that wasn't going to render).
			const bool isOffCam = item.line.params.equalsIgnoreCase("off")
			                   || item.line.params.equalsIgnoreCase("off0")
			                   || item.line.params.equalsIgnoreCase("off1");
			const bool isHero  = (persoId == _heroPersoId && _heroPersoId > 0);
			const char *defaultSource = "";
			if (!effHasAngle && _warpLoaded && !isOffCam) {
				// Faithful default: preserve alpha, zero beta.
				//   - Hero speakers (reverse-shot of the player character):
				//     rotate 180° so the cyclo behind Seth's talking head is
				//     the half of the panorama opposite the NPC, mimicking an
				//     over-the-shoulder cut.  The NPC's own lines keep alpha
				//     unchanged so the player sees the NPC framed directly.
				//   - "off" speakers (voiceover / self-talk, e.g. Seth's
				//     departure-zone lines): the original game does NOT
				//     rotate the camera — the panorama stays exactly where
				//     the player was facing.  Handled by the !isOffCam guard
				//     above so we skip default-angle injection entirely; the
				//     line plays with no view change.
				//
				// The "current" alpha here is the player's click-time facing,
				// captured into _dialBaseAlpha at interactNPC entry — *not*
				// _omni3dMan.getAlpha(), which already reflects the previous
				// line's reverse-shot flip and would leak into the next NPC
				// line.  For non-NPC entry points (e.g. /sel-triggered
				// dialog), fall back to the live alpha.
				const double baseAlpha = _dialBaseAlphaValid
				                       ? _dialBaseAlpha
				                       : _omni3dMan.getAlpha();
				const double curAlphaDeg = baseAlpha * 180.0 / M_PI;
				double effDegX = curAlphaDeg;
				if (isHero) {
					effDegX = curAlphaDeg + 180.0;
				}
				double dx = effDegX - 360.0 * floor(effDegX / 360.0);
				effHasAngle = true;
				effAngleX   = (int)(dx + 0.5);
				effAngleY   = 0;
				defaultSource = isHero ? " [default: hero reverse-shot 180°]"
				                       : " [default: beta=0, alpha preserved]";
			}

			// Suppress the per-cam FOV change for off-camera variants
			// (`off` / `off0` / `off1`).  The original engine gates the
			// FUN_0045179a(camFovDeg) call at FUN_0042844c:22497 on the
			// off-flags all being zero — for self-talk/voiceover lines the
			// panorama stays at the navigation default 75.137° FOV.
			// Passing camNum=0 lands outside playSingleConLine's `2..5`
			// range check, so the FOV change is skipped while the UBB
			// dialog video (if any) still plays.
			const int effCamNum = isOffCam ? 0 : camNum;
			// Off-cam lines display no head, so they also get no SPF, no
			// mouth animation, and no mesh load — drop any basenames the
			// INITDIAL lookup found.
			const Common::String effSpfBase   = isOffCam ? Common::String() : spfBase;
			const Common::String effAnimMBase = isOffCam ? Common::String() : animMBase;
			const Common::String effS3dBase   = isOffCam ? Common::String() : s3dBase;
			const Common::String effAnim3DCBase = isOffCam ? Common::String() : anim3DCBase;
			debugC(2, kDebugScript, "playConSection: speaker='%s' params='%s' persoId=%d "
			         "cam=%d vid='%s' spf='%s' mouth='%s' s3d='%s' 3dc='%s' angle=(%d,%d)%s%s",
			      item.line.speaker.c_str(), item.line.params.c_str(), persoId,
			      effCamNum, vid.c_str(), effSpfBase.c_str(),
			      effAnimMBase.c_str(), effS3dBase.c_str(), effAnim3DCBase.c_str(),
			      effAngleX, effAngleY,
			      effHasAngle ? "" : " (none)",
			      defaultSource);
			playSingleConLine(sec.id, (char)('A' + lineIdx++),
			                  item.line.text, vid,
			                  effHasAngle, effAngleX, effAngleY,
			                  effCamNum, effSpfBase, effAnimMBase, effS3dBase,
			                  effAnim3DCBase,
			                  /*interactiveDialog=*/sec.isCon);
		}
	}

	// Game-over screen (GereGameOver): show the image + narrator commentary,
	// then reload the chapter checkpoint.  Run after the loop so the click/key
	// that ended the last ordinary dialog line has already been drained.
	if (pendingGameOver != 0) {
		showGameOver(sec.id, pendingGameOver, gameOverText, gameOverTrack);
		// The original restores GameOver to its pre-section value on section
		// exit (atlantis.exe FUN_0042844c, LAB_00428c92) — 0 in every chapter.
		_gameVars["gameover"] = 0;

		// A game-over is not a spot the player can recover from in place: it
		// is gated on a *persistent* CON variable (VARIAS.CON indices 0-6 —
		// here FlagChp8), and the section that kills the player stays
		// reachable while that variable keeps its fatal value.  The dialog
		// chain that leads here is also marked played, so simply returning
		// would strand the NPC with no options at all (the reported bug).
		//
		// The exe offers no in-place escape: a chapter change (FUN_00427634)
		// zeroes only the transient vars 7+, and GereGameOver (FUN_00426590)
		// just resets a few loop flags.  The one thing that rolls a
		// persistent variable back is loading a save — and the game
		// autosaves exactly once per chapter, on entry (FUN_00427634 ->
		// FUN_0042d6b8).  So the original's game-over necessarily reloads the
		// current chapter's checkpoint; mirror that here by routing through
		// the engine's existing load-game abort path.
		int  ckSlot = (_currentPlayer >= 0)
		            ? episodeSaveSlot((uint)_currentPlayer, _currentCONChapter)
		            : -1;
		uint reload = (ckSlot >= 0 && saveSlotExists(ckSlot))
		            ? (uint)ckSlot + 1
		            : (_currentPlayer >= 0
		               ? playerResumeSave((uint)_currentPlayer) : 0);
		if (reload != 0) {
			debugC(1, kDebugScript, "showGameOver: reloading chapter checkpoint (slot %u)", reload);
			_loadedSave   = reload;
			_abortCommand = kAbortLoadGame;
		} else {
			// No checkpoint exists yet (a game-over before the first
			// autosave): the only state-clearing fallback is a fresh start.
			warning("showGameOver: no checkpoint to reload; restarting game");
			_abortCommand = kAbortNewGame;
		}
	}

	// Restore the cursor to whatever it was before the dialog.
	showMouse(cursorWasVisible);

	// /go N — section chain: after this section's body finishes, play
	// section N in continuation.  Sections like CHAPI012 /169 have BOTH a
	// dialog body ([Garde185] "Hey, you! Companion!") AND a trailing
	// /go170 — the body MUST play before /go fires the next section,
	// otherwise the "Hey, you" voice-over is skipped and the game jumps
	// straight to the game-over screen.  Skip the chain if the body's
	// /set(chapter=N) just reparsed _conScript (the gotoId would now name
	// the wrong section in the new chapter's pool) or if a game-over
	// reload is pending (loadGame will replay the chapter from scratch).
	if (sec.gotoId > 0 && !shouldAbort() && _currentCONChapter == entryChapter &&
	    pendingGameOver == 0 && _abortCommand == kAbortNoAbort) {
		for (ConSection &target : _conScript.sections()) {
			if (target.id == sec.gotoId) {
				target.played = true;
				playConSection(target);
				break;
			}
		}
	}

	// Final-line skip drain.  playSingleConLine treats a click/key during
	// audio or the trailing subtitle as "advance the line"; on the LAST
	// line of a section the click is therefore still held when we return
	// to handleWarp.  handleWarp's warp loop reads getCurrentMouseButton()
	// as the press signal (it's a level test, not an edge test), so the
	// held button fires a phantom click at the camera's new angle -- the
	// directive `[Lascoyt,190,0,cam5]` snaps alpha to face Lascoyt, and
	// the centre-screen hit-test then dispatches whatever zone happens to
	// sit at that angle.  CHAPI015 reproduces this every time: the
	// timer-fired Lascoyt welcome ends, the held skip-button hits zone
	// 152 (the dining room exit), and section 190's
	// /sel(departVue>151)&(departvue<168)&(FlagTemp1=0) -- "Grab him!" --
	// fires immediately as game-over.  Drain the button here so the next
	// outer click really is a fresh press.
	while (getCurrentMouseButton() != 0 && !shouldAbort()) {
		pollEvents();
		g_system->delayMillis(10);
	}
	clearKeys();
}

// Single dialog line: stream a UBB talking-head video over the warp panorama,
// play the matching APC speech track, overlay the subtitle text.
//
// All geometry is derived from the data: the UBB header (dimensions, frame
// count, frame delay) drives the composite, the screen pixel format drives
// the CLUT8→RGB conversion, and the FONTMAX glyph table drives subtitle
// metrics.  Nothing is hardcoded to 640×480 or RGB565.
//
// The animated dialog mouth mesh is rasterised by F3dcMouthRenderer
// (f3dc_renderer.cpp).
//
// Layout mirrors the original (atlantis.exe FUN_0042ee88 + FUN_00430178):
//   • UBB frame is decoded into a CLUT8 buffer by HNMDecoder.
//   • Each pixel is colour-keyed (palette idx 0 = transparent) and converted
//     to the screen pixel format.  Where transparent, the warp panorama
//     shows through (ScummVM enhancement; the original simply opaquely
//     overwrites the backbuffer).
//   • The subtitle is drawn directly over the composite at the bottom-left,
//     with a 1-pixel drop shadow for legibility.  The original draws white
//     text at (5, 472) with no opaque backing — same anchor, derived from
//     composite height.
void CryOmni3DEngine_Atlantis::playSingleConLine(int sectionId, char track,
                                                  const Common::String &text,
                                                  const Common::String &videoName,
                                                  bool hasAngle,
                                                  int angleX, int angleY,
                                                  int camNum,
                                                  const Common::String &spfBaseName,
                                                  const Common::String &animMBase,
                                                  const Common::String &s3dBase,
                                                  const Common::String &anim3DCBase,
                                                  bool interactiveDialog) {
	const Graphics::PixelFormat fmt = g_system->getScreenFormat();
	const int screenW = (int)g_system->getWidth();
	const int screenH = (int)g_system->getHeight();

	// Per-cam FOV (zoom).  Source: atlantis.exe DGROUP, VA 0x00496080,
	// referenced at code 22514/22535 as `*(float *)(s_4_apc_00496078 +
	// cam*4)` and passed to FUN_0045179a as the horizontal FOV in degrees.
	// Indexing: cam2..cam5 → table[0..3].  cam2 = tight close-up,
	// cam3 = medium, cam4/5 = wide.
	//
	// Save the current FOV so the panorama view restores to its
	// navigation default after the line finishes.  This matches the
	// original engine's FUN_0045179a(view, 75.137, 50.0) calls at
	// lines 22550 / 22601 that restore the default FOV after a dialog.
	static const float kCamFovDeg[4] = {20.68f, 30.0f, 48.0f, 48.26f};
	const double savedHFov = _omni3dMan.getHFov();
	bool fovChanged = false;
	if (_warpLoaded && camNum >= 2 && camNum <= 5) {
		const double camFov = (double)kCamFovDeg[camNum - 2] * M_PI / 180.0;
		_omni3dMan.setHFov(camFov);
		fovChanged = true;
		debugC(2, kDebugScript, "playSingleConLine: cam%d FOV=%.2f° applied",
		      camNum, (double)kCamFovDeg[camNum - 2]);
	}

	// Swap the panorama source to the *clean* cyclo (no NPC sprite overlay)
	// whenever this line should hide the scene standing behind it.
	// _warpSurface carries the NPC sprites composited onto the cyclo (via
	// compositeNPCSprites + recompositeSpriteLayer); two cases must hide them:
	//
	//   • interactiveDialog — a /con dialog the player clicked into: the
	//     original keeps the scene clean for the whole conversation,
	//     including its off-camera reply lines, so the sprites don't flicker
	//     in and out between shots.
	//   • any line that renders a talking-head shot (a UBB is present): the
	//     close-up head sits right over the speaker, so the speaker's own
	//     standing sprite would otherwise show *behind his own head*.  This
	//     is exactly the Creon throne-room arrival — CHAPI011 /sel section
	//     125 fires CONAN1C2.UBB, and although it is a /sel (not /con) it
	//     still needs the sprites hidden.
	//
	// The remaining case keeps _warpSurface: a /sel-triggered voiceover with
	// NO head shot (`[Seth,off]`/`[Lascoyt,off]`, CHAPI011 sections 159/160 —
	// the passing-by remark before chapter 12) must leave the scene's standing
	// NPCs visible.  Restore the warped source on exit either way.
	const bool hasHeadShot = !videoName.empty();
	const Graphics::Surface *cleanSource =
	    ((interactiveDialog || hasHeadShot) &&
	     _warpLoaded && _cycloSurface.getPixels())
	    ? &_cycloSurface : nullptr;
	bool sourceSwapped = false;
	if (cleanSource) {
		_omni3dMan.setSourceSurface(cleanSource);
		sourceSwapped = true;
		debugC(2, kDebugScript, "playSingleConLine: source swapped to clean cyclo");
	}

	// Per-line panorama angle (data-driven).
	//
	// The original engine's CON dialog directive `[Speaker, X, Y, camN]`
	// supplies two optional numeric tokens that the engine writes directly
	// into the cyclo view's alpha/beta — see atlantis.exe.c lines 22381
	// (X/Y → angle conversion) and 22543 (FUN_00451915 alpha/beta store).
	//
	// X and Y are authored in **degrees** — the same unit used by the
	// `/set(anglewarpx=N)` CON command (line ~1707).  Empirically, sampling
	// CHAPI001/003/011/054 yields X values clustered in the 0–360 range
	// (e.g. 120, 180, 210, 269, 292, 350) which only makes sense as
	// degrees, not panorama-pixel coordinates (which would span 0–2048).
	// Y is small (typically 0–4) — vertical tilt in degrees.
	//
	// When the directive carries no numeric tokens the view is left exactly
	// where the player aimed it (matching original behaviour for
	// `[Speaker, camN]` and `[Speaker]` lines).
	if (_warpLoaded && hasAngle) {
		const double alpha = ((double)angleX) * M_PI / 180.0;
		const double beta  = ((double)angleY) * M_PI / 180.0;
		_omni3dMan.setAlpha(alpha);
		_omni3dMan.setBeta(beta);
		_gameVars["anglewarpx"] = (int)(_omni3dMan.getAlpha() * 180.0 / M_PI);
		_gameVars["anglewarpy"] = (int)(_omni3dMan.getBeta()  * 180.0 / M_PI);
		debugC(2, kDebugScript, "playSingleConLine: angle applied X=%d° Y=%d° "
		         "→ alpha=%.3f rad beta=%.3f rad", angleX, angleY, alpha, beta);
	}

#ifdef USE_HNM
	Video::HNMDecoder *bgDecoder = nullptr;
	Graphics::Surface ubbConvFrame;
	// Cleared per line so a leftover palette from a previous video can't
	// leak through if the new stream starts mid-stream without a 'PL' chunk.
	byte ubbPal[256 * 3];
	memset(ubbPal, 0, sizeof(ubbPal));

	// Engine-faithful UBB compositor (no SKIP/RUN distinction available).
	// The original engine seeds the dialog buffer with a cyclo perspective
	// warp each frame and the IV codec's opcode stream tells it which
	// pixel runs to leave (SKIP -> cyclo) and which to write (RUN ->
	// palette[idx]).  ScummVM's Video::HNMDecoder collapses the opcode
	// stream into a flat CLUT8 buffer, and HNM5 motion compensation reads
	// from the previous-frame buffer for *most* opcodes — not just opcode
	// 0x20 — so a sentinel-init trick can't recover the SKIP/RUN
	// distinction without poisoning every motion-compensated block (see
	// project_ubb_compositor memory, offline-render verification at
	// /tmp/seq_strip_*.png).  Until we add an opt-in tracking mask to the
	// shared HNMDecoder (option B), we paint every idx as palette[idx]:
	// idx 0 resolves to palette[0]=black, matching the artist's authored
	// hair colour and the un-enhanced original look.


	// Optional per-cam SPF overlay — the dialog mouth animation that the
	// original engine composites on top of the talking head every UBB frame.
	// The UBB stream has the mouth region painted as palette index 0 (the
	// canonical Cryo chroma key); the SPF fills it in.  Files live alongside
	// the UBB at `DIALOG\PERSO\<base>.SPF` (24 of them on the shipped CD),
	// NOT under `SPRITE\UBB\` (that prefix holds the 205 transition-video
	// SPFs used by playTransitionVideo for navigation walk-ons).  Only ~24
	// dialog cams have a mouth SPF — for the rest, the mouth is baked into
	// the UBB itself, so a missing file is normal and not a warning.
	Common::SeekableReadStream *spfStream = nullptr;
	uint spfFrameCount = 0;
	// SPF screen bbox — the rectangle the scarf/veil actually paints
	// into on screen.  The dialog SPF frame headers we sampled
	// (BAPAN1K3.SPF, GRDAN1C5.SPF) have ZEROED (x1, y_bot, x2, y_top)
	// fields, unlike navigation SPFs.  They position themselves
	// entirely via ROW_SEP opcodes in the flat-buffer pixel stream, so
	// the bbox only emerges from rendering.  Probe by compositing
	// frame[0] onto a scratch surface seeded with a sentinel colour
	// the SPF can never emit, then scan for non-sentinel pixels.
	int spfBboxX1 = -1, spfBboxY1 = -1, spfBboxX2 = -1, spfBboxY2 = -1;
	if (!spfBaseName.empty()) {
		Common::String spfFile = spfBaseName;
		spfFile.toUppercase();
		Common::String spfPath = Common::String("PERSO\\") + spfFile + ".SPF";
		spfStream = openBigFileStream(kFileTypeDialog, spfPath);
		if (spfStream) {
			if (spfStream->seek(4))
				spfFrameCount = spfStream->readUint32LE();
			// Composite frame[0] onto a scratch RGB565 surface seeded
			// with 0xFFFE.  SPF palettes emit RGB565 colours through
			// rgb565ToSurfaceColor (sprite_renderer.cpp:327); 0xFFFE is
			// reserved (the kSprTransp transparent marker is 0x0001 and
			// pure white is 0xFFFF), so any pixel left at 0xFFFE was
			// untouched and any other value was painted by the SPF.
			Graphics::PixelFormat probeFmt(2, 5, 6, 5, 0, 11, 5, 0, 0);
			Graphics::Surface probe;
			probe.create(640, 480, probeFmt);
			const uint16 sentinel = 0xFFFE;
			uint16 *probePix = (uint16 *)probe.getPixels();
			for (int i = 0; i < 640 * 480; i++) probePix[i] = sentinel;
			compositeSpfFrame(spfStream, 0, probe);
			for (int py = 0; py < 480; py++) {
				const uint16 *row = (const uint16 *)probe.getBasePtr(0, py);
				for (int px = 0; px < 640; px++) {
					if (row[px] != sentinel) {
						if (spfBboxX1 < 0 || px < spfBboxX1) spfBboxX1 = px;
						if (spfBboxX2 < 0 || px > spfBboxX2) spfBboxX2 = px;
						if (spfBboxY1 < 0 || py < spfBboxY1) spfBboxY1 = py;
						if (spfBboxY2 < 0 || py > spfBboxY2) spfBboxY2 = py;
					}
				}
			}
			probe.free();
			spfStream->seek(0);
		}
		debugC(1, kDebugScript, "playSingleConLine: SPF overlay '%s' %s "
		         "(%u frames, screen bbox [%d,%d]-[%d,%d])",
		      spfPath.c_str(),
		      spfStream ? "loaded" : "absent (mouth baked in UBB)",
		      spfFrameCount,
		      spfBboxX1, spfBboxY1, spfBboxX2, spfBboxY2);
	}

	// Per-character mouth vertex animation (.3DA M, F3DC subfmt 2) — the
	// per-frame lip-sync visemes the original engine morphs into the mouth
	// mesh.  F3dcMouthRenderer drives the rendered geometry from these.
	F3dcMouthAnim mouthAnim;
	int32 mouthMinX = 0, mouthMinY = 0, mouthMaxX = 0, mouthMaxY = 0;
	int32 mouthMinZ = 0, mouthMaxZ = 0;
	if (!animMBase.empty()) {
		Common::String mFile = animMBase;
		mFile.toUppercase();
		Common::String mPath = Common::String("PERSO\\") + mFile + ".3DA";
		Common::ScopedPtr<Common::SeekableReadStream> ms(
		    openBigFileStream(kFileTypeDialog, mPath));
		if (ms && mouthAnim.loadFromStream(*ms)) {
			mouthAnim.getBounds(mouthMinX, mouthMinY, mouthMinZ,
			                    mouthMaxX, mouthMaxY, mouthMaxZ);
			debugC(1, kDebugScript, "playSingleConLine: mouth anim '%s' loaded "
			         "(%u frames, %u verts, X[%d..%d] Y[%d..%d] Z[%d..%d])",
			      mPath.c_str(),
			      mouthAnim.frameCount(), mouthAnim.verticesPerFrame(),
			      mouthMinX, mouthMaxX, mouthMinY, mouthMaxY,
			      mouthMinZ, mouthMaxZ);
		} else {
			debugC(1, kDebugScript, "playSingleConLine: mouth anim '%s' unavailable",
			      mPath.c_str());
		}
	}

	// Per-character head-sway / skeleton animation (.3DA A, F3DC subfmt 4).
	// Same basename as the mouth anim but with the trailing "M" replaced by
	// "A" (e.g. BADPAN1M.3DA → BADPAN1A.3DA).  Prototype: feeds a per-K1
	// model-space translation into F3dcMouthRenderer to roughly track the
	// head's motion in the UBB video.  See project_f3dc_head_sway memory.
	F3dcSmallAnim smallAnim;
	if (!animMBase.empty()) {
		Common::String aFile = animMBase;
		aFile.toUppercase();
		const uint sz = aFile.size();
		if (sz >= 2 && aFile[sz - 1] == 'M') {
			aFile.setChar('A', sz - 1);
			Common::String aPath = Common::String("PERSO\\") + aFile + ".3DA";
			Common::ScopedPtr<Common::SeekableReadStream> as(
			    openBigFileStream(kFileTypeDialog, aPath));
			if (as && smallAnim.loadFromStream(*as)) {
				debugC(1, kDebugScript, "playSingleConLine: head-sway anim '%s' loaded "
				         "(%u bones)", aPath.c_str(), smallAnim.boneCount());
			} else {
				debugC(1, kDebugScript, "playSingleConLine: head-sway anim '%s' unavailable",
				      aPath.c_str());
			}
		}
	}

	// Per-character scene-graph mesh file (DIALOG\PERSO\<base>.S3D).  Its
	// tail holds the per-camera look-at transforms F3dcMouthRenderer uses
	// to project the mouth mesh onto the talking-head video.
	F3dcMesh mesh;
	if (!s3dBase.empty()) {
		Common::String sFile = s3dBase;
		sFile.toUppercase();
		Common::String sPath = Common::String("PERSO\\") + sFile + ".S3D";
		Common::ScopedPtr<Common::SeekableReadStream> ss(
		    openBigFileStream(kFileTypeDialog, sPath));
		if (ss && mesh.loadFromStream(*ss)) {
			uint32 roots[4];
			mesh.getRootFileOffsets(roots);
			debugC(1, kDebugScript, "playSingleConLine: mesh '%s' loaded "
			         "(countA=%u, roots 0x%x 0x%x 0x%x 0x%x)",
			      sPath.c_str(), mesh.headerCountA(),
			      roots[0], roots[1], roots[2], roots[3]);
		} else {
			debugC(1, kDebugScript, "playSingleConLine: mesh '%s' unavailable", sPath.c_str());
		}
	}

	// Per-character dialog mouth MESH (verts + triangle topology) from the
	// .3DC body file's "fvi" node.  This is the real geometry that fills
	// the UBB chroma-key mouth hole.
	F3dcMouthMesh mouthMesh;
	if (!anim3DCBase.empty()) {
		Common::String cFile = anim3DCBase;
		cFile.toUppercase();
		Common::String cPath = Common::String("PERSO\\") + cFile + ".3DC";
		Common::ScopedPtr<Common::SeekableReadStream> cs(
		    openBigFileStream(kFileTypeDialog, cPath));
		if (cs && mouthMesh.loadFromStream(*cs)) {
			debugC(1, kDebugScript, "playSingleConLine: mouth mesh '%s' loaded "
			         "(%u verts, %u triangles)",
			      cPath.c_str(), mouthMesh.vertexCount(),
			      mouthMesh.triangleCount());
		} else {
			debugC(1, kDebugScript, "playSingleConLine: mouth mesh '%s' unavailable",
			      cPath.c_str());
		}
	}

	// Per-character face texture (DIALOG\MAPS\<base>.3DM).  The dialog mouth
	// mesh is texture-mapped with this 256x256 image; its basename is
	// embedded in the .3DC header, exposed as candidate strings.
	F3dcTextureMap mouthTex;
	if (mouthMesh.isLoaded()) {
		const Common::Array<Common::String> &cands =
		    mouthMesh.textureMapCandidates();
		for (uint ci = 0; ci < cands.size() && !mouthTex.isLoaded(); ci++) {
			Common::String tFile = cands[ci];
			tFile.toUppercase();
			Common::String tPath = Common::String("MAPS\\") + tFile + ".3DM";
			Common::ScopedPtr<Common::SeekableReadStream> ts(
			    openBigFileStream(kFileTypeDialog, tPath));
			if (ts && mouthTex.loadFromStream(*ts))
				debugC(1, kDebugScript, "playSingleConLine: face texture '%s' loaded",
				      tPath.c_str());
		}
		if (!mouthTex.isLoaded())
			debugC(1, kDebugScript, "playSingleConLine: no .3DM face texture "
			         "(%u candidates) — mouth flat-shaded", cands.size());
	}

	// Animated dialog mouth-mesh renderer — projects the per-character .3DC
	// mesh through the .S3D look-at camera and rasterises the lip-sync
	// .3DA-M visemes, texture-mapped, into the UBB chroma-key mouth hole.
	F3dcMouthRenderer mouthRenderer;
	mouthRenderer.init(mouthMesh, mesh, mouthAnim,
	                   mouthTex.isLoaded() ? &mouthTex : nullptr, camNum,
	                   smallAnim.isLoaded() ? &smallAnim : nullptr);

	if (!videoName.empty()) {
		debugC(1, kDebugScript, "playSingleConLine: trying DIALOG\\%s", videoName.c_str());
		Common::SeekableReadStream *vs = openBigFileStream(kFileTypeDialog, videoName);
		if (!vs) {
			debugC(1, kDebugScript, "playSingleConLine: file not found in BigFile");
		} else {
			// UBB2 is 8-bit paletted; HNMDecoder rejects CLUT8 streams when
			// initialised with an RGB target, so we decode into CLUT8 and
			// convert per-pixel below.  This also lets us colour-key idx 0.
			Graphics::PixelFormat clut8 = Graphics::PixelFormat::createFormatCLUT8();
			bgDecoder = new Video::HNMDecoder(clut8, true);  // loop=true
			if (!bgDecoder->loadStream(vs)) {
				debugC(1, kDebugScript, "playSingleConLine: HNMDecoder::loadStream failed");
				delete bgDecoder;
				bgDecoder = nullptr;
			} else {
				// Composite surface tracks the decoder's intrinsic resolution.
				ubbConvFrame.create(bgDecoder->getWidth(), bgDecoder->getHeight(), fmt);
				debugC(1, kDebugScript, "playSingleConLine: video loaded OK (%ux%u)",
				      bgDecoder->getWidth(), bgDecoder->getHeight());
				// Atlantis dialog UBBs ship a FUN_00450b08 opcode stream
				// in the IA chunks alongside the IV (CLUT8) chunks; the
				// opcode stream carries the SKIP/RUN distinction that
				// drives the cyclo bleed-through.  Opt in so the decoder
				// stops discarding IA as audio.  See project_ubb_compositor.
				bgDecoder->setCaptureOpcodeStream(true);
				bgDecoder->start();
			}
		}
	}
#endif

	// Subtitle pre-wrap.  Width budget is the composite width less symmetric
	// margins (5 px on either side, matching the original's x=5 origin).
	const int subLeftMargin  = 5;
	const int subRightMargin = 5;
	int subWidthBudget = screenW - subLeftMargin - subRightMargin;
#ifdef USE_HNM
	if (ubbConvFrame.getPixels())
		subWidthBudget = ubbConvFrame.w - subLeftMargin - subRightMargin;
#endif
	if (subWidthBudget < 32) subWidthBudget = 32;

	// Subtitles render in a proportional sans-serif font, matching the original
	// (Arial via GDI TextOutA, atlantis.exe FUN_0041aee4) -- NOT the gold
	// FONTMAX sprite font, which is the menu font.  kBigGUIFont is ScummVM's
	// built-in proportional Helvetica (Arial-like), compiled in so it works on
	// the no-FreeType/no-theme Windows build too.
	const Graphics::Font *guiFont =
	    FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	if (!guiFont)
		guiFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	Common::Array<Common::String> subLines;
	int subLineH = 0;
	const bool useFontMax = false;
	// Subtitle toggle: when disabled in ScummVM options, leave subLines
	// empty so nothing is rendered.  Voice playback further down is not
	// affected -- it runs through Audio::Mixer::kSpeechSoundType and obeys
	// the standard speech volume / speech-mute settings via the mixer.
	const bool drawSubs = showSubtitles();
	if (drawSubs && !text.empty()) {
		if (useFontMax) {
			// Greedy whitespace word-wrap using FONTMAX advance widths.
			subLineH = 24;  // FONTMAX glyph height upper bound (data-driven below)
			int maxGlyphH = 0;
			for (uint si = 0; si < _fontMaxSprites.size(); ++si)
				if ((int)_fontMaxSprites[si].h > maxGlyphH)
					maxGlyphH = _fontMaxSprites[si].h;
			if (maxGlyphH > 0) subLineH = maxGlyphH + 2;

			Common::String line, word;
			for (uint i = 0; i <= text.size(); ++i) {
				char c = (i < text.size()) ? text[i] : ' ';
				if (c == ' ' || c == '\r' || c == '\n') {
					if (!word.empty()) {
						Common::String cand = line.empty() ? word : line + " " + word;
						if (fontMaxStringWidth(cand) > subWidthBudget && !line.empty()) {
							subLines.push_back(line);
							line = word;
						} else {
							line = cand;
						}
						word.clear();
					}
					if (c == '\r' || c == '\n') {
						if (!line.empty()) { subLines.push_back(line); line.clear(); }
					}
				} else {
					word += c;
				}
			}
			if (!line.empty()) subLines.push_back(line);
		} else if (guiFont) {
			subLineH = guiFont->getFontHeight() + 2;
			guiFont->wordWrapText(text, subWidthBudget, subLines);
		}
	}

	// Render-on-screen color for subtitles: pure white, mirroring the
	// original (0xffff in RGB565 is white).  Shadow is opaque black.
	const uint32 subColor    = fmt.RGBToColor(255, 255, 255);
	const uint32 subShadow   = fmt.RGBToColor(0,   0,   0);

	// Tracks whether the no-video / no-panorama fallback has already cleared
	// the backbuffer on this call.  Function-local (not `static`) so the
	// engine remains re-entrant — each line restarts from "not cleared".
	bool emptyBackbufferCleared = false;

	// SPF overlay frame index: advances once per *fresh* UBB frame so the
	// i-th SPF frame composites on top of the i-th UBB frame, mirroring the
	// original engine's shared frame counter at FUN_0042f9e8 + 0x358.  When
	// the SPF runs short, compositeSpfFrame() clamps to its last frame.
	uint spfFrameIdx = 0;

	auto drawFrame = [&]() {
		// 1. Underlay: warp panorama (if loaded) or cleared backbuffer.
		const Graphics::Surface *warpView =
		    _warpLoaded ? _omni3dMan.getSurface() : nullptr;

#ifdef USE_HNM
		if (bgDecoder && ubbConvFrame.getPixels()) {
			// Drive frame timing from the decoder's natural rate (HNM
			// regularFrameDelayMs ≈ 50 ms = 20 FPS for UBB2 dialog).
			const Graphics::Surface *f = nullptr;
			if (bgDecoder->needsUpdate())
				f = bgDecoder->decodeNextFrame();

			if (f) {
				if (bgDecoder->hasDirtyPalette())
					memcpy(ubbPal, bgDecoder->getPalette(), 256 * 3);

				// Seed ubbConvFrame with the cyclo panorama (mirrors
				// FUN_00451d8b in atlantis.exe).  The SKIP opcodes in
				// the IA stream leave dst untouched, so this seed shows
				// through wherever the artist wanted transparency
				// (cinematic letterbox).  Falls back to black if the
				// warp panorama is absent or mismatched.
				if (warpView && warpView->w == ubbConvFrame.w &&
				                warpView->h == ubbConvFrame.h &&
				                warpView->format == fmt) {
					const uint copyRowBytes = ubbConvFrame.w * fmt.bytesPerPixel;
					for (int row = 0; row < ubbConvFrame.h; row++)
						memcpy(ubbConvFrame.getBasePtr(0, row),
						       warpView->getBasePtr(0, row), copyRowBytes);
				} else {
					ubbConvFrame.fillRect(
					    Common::Rect(ubbConvFrame.w, ubbConvFrame.h), 0);
				}

				// Per-pixel composite via FUN_00450b08 port.  The IA
				// chunk's opcode stream drives the SKIP/RUN decision
				// per pixel run; ScummVM's HNM5-decoded CLUT8 buffer
				// supplies palette indices for the RUN positions.
				// Format-only support for the original's shade blend
				// (shade<0xff): we always paint at full intensity for
				// simplicity — visual impact is a slight loss of
				// darkening on dimly-lit edges, not a correctness bug.
				const int fw = f->w, fh = f->h;
				const byte *srcBase = (const byte *)f->getBasePtr(0, 0);
				uint32 opSize = 0;
				const byte *opcodes = bgDecoder->getOpcodeStream(opSize);

				// Build a 16-bit RGB565 palette LUT from the CLUT8
				// palette for O(1) per-RUN-pixel lookup.
				uint16 palLUT[256];
				for (int i = 0; i < 256; i++)
					palLUT[i] = (uint16)fmt.RGBToColor(ubbPal[i*3+0],
					                                  ubbPal[i*3+1],
					                                  ubbPal[i*3+2]);

				if (opcodes && opSize >= 4 &&
				    fmt.bytesPerPixel == 2) {
					// Walk the FUN_00450b08 opcode stream.
					const byte *op = opcodes;
					const byte *opEnd = opcodes + opSize;
					uint32 pos = 0;                            // pixel offset into frame
					const uint32 frameN = (uint32)fw * (uint32)fh;
					uint16 *dst16 = (uint16 *)ubbConvFrame.getPixels();
					while (op + 2 <= opEnd && pos < frameN) {
						const byte type = op[0];
						const byte runHi = op[1];
						if (type == 0) {
							// SKIP: 4-byte op, runlen in high 24 bits.
							if (op + 4 > opEnd) break;
							const uint32 runlen = ((uint32)op[1]) |
							                      ((uint32)op[2] << 8) |
							                      ((uint32)op[3] << 16);
							if (runlen == 0) break;            // terminator
							pos += runlen;
							op += 4;
						} else if (type == 1) {
							// RUN with a single shade byte (op[2]).
							// shade>>3 == 0x1f -> full intensity (solid
							// palette colour); a lower shade is an edge
							// pixel the original alpha-blends against the
							// background (the cyclo seed in dst), so the
							// head edge is anti-aliased instead of leaving
							// a hard outline.  Mirrors FUN_00450b08.
							if (op + 3 > opEnd) break;
							const uint32 runlen = runHi;
							const byte *src = srcBase + pos;
							const uint shade = (uint)(op[2] >> 3);
							if (shade >= 0x1f) {
								for (uint32 i = 0; i < runlen && pos + i < frameN; i++)
									dst16[pos + i] = palLUT[src[i]];
							} else {
								for (uint32 i = 0; i < runlen && pos + i < frameN; i++)
									dst16[pos + i] = blendUbbPixel565(
									    palLUT[src[i]], dst16[pos + i], shade);
							}
							pos += runlen;
							op += 3;
						} else if (type == 2) {
							// RUN with per-pixel shade: runHi pixels, then
							// runHi shade bytes.  Each pixel uses its own
							// shade; 0x1f is solid, lower values blend with
							// the background (FUN_00450b08 type-2 path).
							if (op + 2 + runHi > opEnd) break;
							const uint32 runlen = runHi;
							const byte *src = srcBase + pos;
							const byte *shadeBytes = op + 2;
							for (uint32 i = 0; i < runlen && pos + i < frameN; i++) {
								const uint shade = (uint)(shadeBytes[i] >> 3);
								dst16[pos + i] = (shade >= 0x1f)
								    ? palLUT[src[i]]
								    : blendUbbPixel565(palLUT[src[i]],
								                       dst16[pos + i], shade);
							}
							pos += runlen;
							op += 2 + runlen;
						} else {
							warning("playSingleConLine: unknown UBB opcode 0x%02x at +%u",
							        type, (uint)(op - opcodes));
							break;
						}
					}
				} else {
					// No opcode stream available (or unsupported pixel
					// format) — fall back to the flat composite so the
					// head is at least visible.
					const uint bpp = fmt.bytesPerPixel;
					const int fp = f->pitch;
					for (int fy = 0; fy < fh; fy++) {
						const byte *src = srcBase + fy * fp;
						byte *dst = (byte *)ubbConvFrame.getBasePtr(0, fy);
						for (int fx = 0; fx < fw; fx++, dst += bpp) {
							const uint32 c = palLUT[src[fx]];
							if (bpp == 2)       *(uint16 *)dst = (uint16)c;
							else if (bpp == 4)  *(uint32 *)dst = c;
							else                memcpy(dst, &c, bpp);
						}
					}
				}

				// 1d. Dialog mouth-mesh render — drawn AFTER the UBB so
				//     the mesh overpaints whatever the UBB drew in the
				//     mouth area.  The remaining "green border" artefact
				//     at the mesh silhouette is still under investigation
				//     (see project_f3dc_renderer memory) — the original
				//     engine somehow filters it but neither bit-0/bit-1
				//     triangle type, backface culling, nor BFS-classified
				//     chroma compositing reproduces that filter.  For now,
				//     keep the working render until further dynamic trace.
				if (mouthRenderer.isReady())
					mouthRenderer.renderFrame(ubbConvFrame, spfFrameIdx, 0);

				// 1e. Per-cam SPF overlay (INITDIAL columns 10..13).  The
				//     original engine composites the matching SPF frame on
				//     top of every UBB frame in the dialog playback loop at
				//     FUN_0042f9e8 (atlantis.exe.c:26609-26734), sharing the
				//     UBB's frame counter.  We mirror that exactly, plus a
				//     modulo so the SPF LOOPS when the UBB outlives it —
				//     compositeSpfFrame clamps to the last frame on its own,
				//     which freezes the scarf/veil while the head keeps
				//     talking (visible bug on EnnemiPilot whose UBB runs
				//     30+ frames against a much shorter SPF).
				//     Drawn last, on top of the UBB head, so extra body
				//     parts (e.g. the Ennemi Pilot's scarf) overlay it.
				if (spfStream) {
					const uint sfi = (spfFrameCount > 0)
					    ? (spfFrameIdx % spfFrameCount)
					    : spfFrameIdx;
					compositeSpfFrame(spfStream, sfi, ubbConvFrame);
				}

				// 1f. "Object just received" icon: an NPC that gave Seth an
				//     item this dialog shows its icon centre-bottom over the
				//     head, on top of everything (the original DAT_0049619c
				//     overlay).  Cleared when the dialog ends.
				if (fmt.bytesPerPixel == 2)
					drawGivenObject(ubbConvFrame);

				// Advance the shared SPF / mouth-anim frame counter once
				// per fresh UBB frame.  Same one-to-one mapping the
				// original engine uses (atlantis.exe.c FUN_0042f9e8:26710).
				++spfFrameIdx;
			}
		} else
#endif
		if (warpView) {
			// No video — keep the panorama on-screen behind the subtitle.  If
			// an NPC gave Seth an item this dialog, overlay its icon
			// centre-bottom on a copy of the panorama first (the DAT_0049619c
			// overlay still shows on off-camera/voiceover lines).
			if (_dialogGivenObjSprite >= 0 && fmt.bytesPerPixel == 2) {
				Graphics::Surface tmp;
				tmp.create(warpView->w, warpView->h, fmt);
				tmp.copyFrom(*warpView);
				drawGivenObject(tmp);
				g_system->copyRectToScreen(tmp.getPixels(), tmp.pitch,
				                           0, 0, tmp.w, tmp.h);
				tmp.free();
			} else {
				g_system->copyRectToScreen(warpView->getPixels(), warpView->pitch,
				                           0, 0, warpView->w, warpView->h);
			}
		} else {
			// No video and no panorama — clear once per playSingleConLine call.
			if (!emptyBackbufferCleared) {
				Graphics::ManagedSurface clr(screenW, screenH, fmt);
				clr.fillRect(Common::Rect(screenW, screenH), 0);
				g_system->copyRectToScreen(clr.getPixels(), clr.pitch,
				                           0, 0, screenW, screenH);
				emptyBackbufferCleared = true;
			}
		}

		// 2. Subtitle overlay, drawn directly onto the composite (no opaque
		//    backing bar — matching the original).  Anchored to the bottom
		//    of the *composite* surface so videos with non-480 height work.
#ifdef USE_HNM
		Graphics::ManagedSurface compose;
		bool composeOwnsPixels = false;
		int blitX = 0, blitY = 0, blitW = 0, blitH = 0;

		if (bgDecoder && ubbConvFrame.getPixels()) {
			// Snapshot the conv frame into a ManagedSurface so FONTMAX /
			// GUI font helpers can blit through the standard interface.
			compose.create(ubbConvFrame.w, ubbConvFrame.h, fmt);
			composeOwnsPixels = true;
			const uint rowBytes = ubbConvFrame.w * fmt.bytesPerPixel;
			for (int row = 0; row < ubbConvFrame.h; row++)
				memcpy(compose.getBasePtr(0, row),
				       ubbConvFrame.getBasePtr(0, row), rowBytes);
			blitW = ubbConvFrame.w;
			blitH = ubbConvFrame.h;
			// Centre the composite if it's smaller than the screen.
			blitX = (screenW - blitW) / 2;
			blitY = (screenH - blitH) / 2;
			if (blitX < 0) blitX = 0;
			if (blitY < 0) blitY = 0;
		}
#endif

		// Bottom-anchored subtitle, 6 px gap from bottom edge.
		int subAreaH = subLines.size() * subLineH;
		int subBaseY = 0;
#ifdef USE_HNM
		int subSurfH = composeOwnsPixels ? compose.h : screenH;
#else
		int subSurfH = screenH;
#endif
		subBaseY = subSurfH - subAreaH - 6;
		if (subBaseY < 0) subBaseY = 0;

#ifdef USE_HNM
		if (composeOwnsPixels) {
			if (useFontMax) {
				for (uint i = 0; i < subLines.size(); ++i) {
					int by = subBaseY + (int)i * subLineH + subLineH - 4;
					// 1-px black drop shadow for legibility against the head.
					drawFontMaxText(compose, subLines[i],
					                subLeftMargin + 1, by + 1);
					drawFontMaxText(compose, subLines[i],
					                subLeftMargin,     by);
				}
			} else if (guiFont) {
				for (uint i = 0; i < subLines.size(); ++i) {
					int by = subBaseY + (int)i * subLineH;
					guiFont->drawString(&compose, subLines[i],
					    subLeftMargin + 1, by + 1, subWidthBudget, subShadow,
					    Graphics::kTextAlignLeft);
					guiFont->drawString(&compose, subLines[i],
					    subLeftMargin, by, subWidthBudget, subColor,
					    Graphics::kTextAlignLeft);
				}
			}
			g_system->copyRectToScreen(compose.getPixels(), compose.pitch,
			                           blitX, blitY, blitW, blitH);
			compose.free();
		} else
#endif
		{
			// No composite surface (no video) — text on the warp/cleared
			// backbuffer via a small overlay surface, same shadow scheme.
			if (!subLines.empty() && subLineH > 0) {
				Graphics::ManagedSurface overlay(screenW, subAreaH + 4, fmt);
				if (warpView) {
					// Pull the matching strip from the panorama so the text
					// keeps the original backdrop pixels (no opaque bar).
					int srcY = screenH - subAreaH - 6;
					if (srcY < 0) srcY = 0;
					int rowsAvail = warpView->h - srcY;
					if (rowsAvail > overlay.h) rowsAvail = overlay.h;
					const uint rowBytes = warpView->w * fmt.bytesPerPixel;
					for (int row = 0; row < rowsAvail; row++)
						memcpy(overlay.getBasePtr(0, row),
						       warpView->getBasePtr(0, srcY + row), rowBytes);
				} else {
					overlay.fillRect(Common::Rect(overlay.w, overlay.h), 0);
				}
				if (useFontMax) {
					for (uint i = 0; i < subLines.size(); ++i) {
						int by = (int)i * subLineH + subLineH - 4;
						drawFontMaxText(overlay, subLines[i],
						                subLeftMargin + 1, by + 1);
						drawFontMaxText(overlay, subLines[i],
						                subLeftMargin,     by);
					}
				} else if (guiFont) {
					for (uint i = 0; i < subLines.size(); ++i) {
						int by = (int)i * subLineH;
						guiFont->drawString(&overlay, subLines[i],
						    subLeftMargin + 1, by + 1, subWidthBudget, subShadow,
						    Graphics::kTextAlignLeft);
						guiFont->drawString(&overlay, subLines[i],
						    subLeftMargin, by, subWidthBudget, subColor,
						    Graphics::kTextAlignLeft);
					}
				}
				int dy = screenH - subAreaH - 6;
				if (dy < 0) dy = 0;
				g_system->copyRectToScreen(overlay.getPixels(), overlay.pitch,
				                           0, dy, overlay.w, overlay.h);
				overlay.free();
			}
		}
		g_system->updateScreen();
	};

	// Linger: scale with subtitle length (visible time ≈ 80 ms/char,
	// minimum 1.5 s) so longer lines aren't cut short when no audio exists.
	int lingerMs = 1500;
	if (!text.empty())
		lingerMs = MAX(1500, (int)text.size() * 80);

	// One click (or key) skips only one line: drain a still-held skip input
	// carried over from skipping the previous line, so this line's playback
	// watches for a fresh press instead of consuming the old one.
	while (getCurrentMouseButton() != 0 && !shouldAbort()) {
		pollEvents();
		g_system->delayMillis(10);
	}
	clearKeys();

	Common::String apcName = Common::String::format("ATA%04d%c.APC", sectionId, track);
	Common::SeekableReadStream *apcFile = openBigFileStream(kFileTypeSound, apcName);

	if (!apcFile) {
		for (int ms = 0; ms < lingerMs && !shouldAbort(); ms += 20) {
			drawFrame();
			pollEvents();
			if (checkKeysPressed(1, Common::KEYCODE_SPACE) || getCurrentMouseButton() != 0) break;
			g_system->delayMillis(20);
		}
#ifdef USE_HNM
		delete bgDecoder;
		delete spfStream;
		ubbConvFrame.free();
#endif
		if (fovChanged) _omni3dMan.setHFov(savedHFov);
		if (sourceSwapped) _omni3dMan.setSourceSurface(&_warpSurface);
		return;
	}

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
		Audio::SoundHandle handle;
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, apc);
		debugC(1, kDebugScript, "playSingleConLine: %s", apcName.c_str());

		while (!shouldAbort() && _mixer->isSoundHandleActive(handle)) {
			drawFrame();
			pollEvents();
			if (checkKeysPressed(1, Common::KEYCODE_SPACE) || getCurrentMouseButton() != 0) {
				_mixer->stopHandle(handle);
				break;
			}
			g_system->delayMillis(10);
		}
		_mixer->stopHandle(handle);
	}
	delete apcFile;

	// Subtitle linger after audio ends — fixed 500 ms tail matches the
	// original game's "fade-out grace" before the next line starts.
	for (int ms = 0; ms < 500 && !shouldAbort(); ms += 20) {
		drawFrame();
		pollEvents();
		if (checkKeysPressed(1, Common::KEYCODE_SPACE) || getCurrentMouseButton() != 0) break;
		g_system->delayMillis(20);
	}
#ifdef USE_HNM
	delete bgDecoder;
	delete spfStream;
	ubbConvFrame.free();
#endif
	if (fovChanged) _omni3dMan.setHFov(savedHFov);
	if (sourceSwapped) _omni3dMan.setSourceSurface(&_warpSurface);
}

} // namespace Atlantis
} // namespace CryOmni3D
