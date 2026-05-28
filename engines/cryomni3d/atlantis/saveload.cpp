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
 * along with this program.  If not, see <http://www.gnu.org/licenses.
 *
 */

#include "common/debug.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/system.h"

#include "cryomni3d/atlantis/engine.h"

namespace CryOmni3D {
namespace Atlantis {

// Fixed save-file block layout (1792 bytes):
//   20  — save description (kSaveDescriptionLen, null-padded)
//   12  — dummy padding (3 × uint32, currently zeros)
//  200  — dialog variables (reserved, all zero)
//   80  — inventory (20 × uint32BE object index, uint(-1) = empty)
//    4  — current chapter (uint32BE)
//   32  — current WAM filename (null-padded, may be a sub-WAM)
//    4  — current place ID (uint32BE)
//    8  — camera alpha (doubleBE)
//    8  — camera beta (doubleBE)
// 1024  — place states (256 × uint32BE)
//  400  — game variables (100 × uint32BE)
//
// Extended block appended after the fixed block (magic 'ATLS', version 3):
//   4  — magic 0x41544C53 ('ATLS')
//   4  — version (uint32BE, currently 3)
//   4  — hiddenPersos count N (uint32BE)
//   N×4 — hidden perso IDs (int32BE each)
//   4  — sujEnabled entries M (uint32BE)
//   M × (4 + 4 + K×4) — persoId (int32BE), numSujs K (uint32BE), suj IDs (int32BE each)
//   4  — gameVars count G (uint32BE)
//   G × (4 + keyLen + 4) — keyLen (uint32BE), key bytes (no null), value (int32BE)
//   4  — CON script chapter (uint32BE) — version 2+ only
//   4  — selected music track (uint32BE) — version 3+ only
static const int32 kSaveSize = 1792;
static const uint kWAMNameLen = 32;
static const uint32 kExtMagic = 0x41544C53u;  // 'ATLS'
static const uint32 kExtVersion = 3;

void CryOmni3DEngine_Atlantis::saveGame(uint saveNum, const Common::String &saveName) {
	Common::String saveFileName = getSaveStateName((int)saveNum - 1);

	Common::OutSaveFile *out = _saveFileMan->openForSaving(saveFileName);
	if (!out) {
		warning("saveGame: cannot open '%s' for writing", saveFileName.c_str());
		return;
	}

	// Save description (20 bytes)
	char saveNameC[kSaveDescriptionLen + 1];
	memset(saveNameC, 0, sizeof(saveNameC));
	strncpy(saveNameC, saveName.c_str(), kSaveDescriptionLen);
	out->write(saveNameC, kSaveDescriptionLen);

	// Dummy padding
	out->writeUint32LE(0);
	out->writeUint32BE(0);
	out->writeUint32BE(0);

	// Dialog variables (not used; write zeros)
	for (uint i = 0; i < 200; i++)
		out->writeByte(0);

	// Inventory (20 slots × uint32BE)
	for (Inventory::const_iterator it = _inventory.begin(); it != _inventory.end(); ++it) {
		uint objId = uint(-1);
		if (*it != nullptr)
			objId = (uint)(*it - _objects.begin());
		out->writeUint32BE(objId);
	}

	// Chapter
	out->writeUint32BE(_currentChapter);

	// Current WAM filename (32 bytes, null-padded)
	char wamNameC[kWAMNameLen];
	memset(wamNameC, 0, sizeof(wamNameC));
	strncpy(wamNameC, _currentWAMName.c_str(), kWAMNameLen - 1);
	out->write(wamNameC, kWAMNameLen);

	// Current place ID
	out->writeUint32BE(_currentPlaceId);

	// Camera angles
	out->writeDoubleBE(_omni3dMan.getAlpha());
	out->writeDoubleBE(_omni3dMan.getBeta());

	// Place states (256 × uint32BE)
	for (uint i = 0; i < 256; i++) {
		uint32 state = (i < _placeStates.size()) ? _placeStates[i].state : 0;
		out->writeUint32BE(state);
	}

	// Game variables (100 × uint32BE)
	for (uint i = 0; i < 100; i++) {
		uint val = (i < _gameVariables.size()) ? _gameVariables[i] : 0;
		out->writeUint32BE(val);
	}

	// Extended block: CON script state.
	out->writeUint32BE(kExtMagic);
	out->writeUint32BE(kExtVersion);

	// Hidden persos
	out->writeUint32BE((uint32)_hiddenPersos.size());
	for (int pid : _hiddenPersos)
		out->writeSint32BE(pid);

	// Enabled subjects per NPC
	out->writeUint32BE((uint32)_sujEnabled.size());
	for (const SujState &ss : _sujEnabled) {
		out->writeSint32BE(ss.persoId);
		out->writeUint32BE((uint32)ss.sujs.size());
		for (int s : ss.sujs)
			out->writeSint32BE(s);
	}

	// Named game variables — only the 7 persistent ones (VARIAS.CON idx 0-6).
	// The rest are chapter-transient and the original never saves them.
	uint32 nPersist = 0;
	for (const auto &kv : _gameVars)
		if (isPersistentGameVar(kv._key))
			nPersist++;
	out->writeUint32BE(nPersist);
	for (const auto &kv : _gameVars) {
		if (!isPersistentGameVar(kv._key))
			continue;
		uint32 klen = (uint32)kv._key.size();
		out->writeUint32BE(klen);
		out->write(kv._key.c_str(), klen);
		out->writeSint32BE(kv._value);
	}

	// CON script chapter (ext v2).  loadConScript() clears _gameVars, so the
	// active chapter is not reliably present among the named variables above —
	// persist it explicitly so loadGame restores the correct CHAPI*.CON.
	out->writeUint32BE(_currentCONChapter);

	// Selected music track (ext v3).  The startmusik/stopmusik CON commands
	// own _musicTrackId; it is not a game variable, so persist it explicitly
	// or a reloaded save runs in silence until the next startmusik.
	out->writeUint32BE((uint32)_musicTrackId);

	out->finalize();
	delete out;
}

bool CryOmni3DEngine_Atlantis::loadGame(uint saveNum) {
	Common::String saveFileName = getSaveStateName((int)saveNum - 1);

	Common::InSaveFile *in = _saveFileMan->openForLoading(saveFileName);
	if (!in) {
		warning("loadGame: cannot open '%s'", saveFileName.c_str());
		return false;
	}

	if (in->size() < kSaveSize) {
		warning("loadGame: '%s' too small (%d bytes, need at least %d)",
		        saveFileName.c_str(), (int)in->size(), kSaveSize);
		delete in;
		return false;
	}

	// Save description (skip)
	char saveNameC[kSaveDescriptionLen];
	in->read(saveNameC, sizeof(saveNameC));

	// Dummy padding
	(void)in->readUint32LE();
	(void)in->readUint32BE();
	(void)in->readUint32BE();

	// Dialog variables (skip — superseded by extended block)
	for (uint i = 0; i < 200; i++)
		(void)in->readByte();

	// Inventory
	for (Inventory::iterator it = _inventory.begin(); it != _inventory.end(); ++it) {
		uint objId = in->readUint32BE();
		if (objId >= _objects.size())
			objId = uint(-1);
		*it = (objId != uint(-1)) ? (_objects.begin() + objId) : nullptr;
	}

	// Chapter
	uint32 chapter = in->readUint32BE();

	// WAM filename
	char wamNameC[kWAMNameLen];
	in->read(wamNameC, kWAMNameLen);
	wamNameC[kWAMNameLen - 1] = '\0';

	// Place, camera angles
	uint32 placeId = in->readUint32BE();
	double alpha = in->readDoubleBE();
	double beta  = in->readDoubleBE();

	// Place states
	uint32 savedPlaceStates[256];
	for (uint i = 0; i < 256; i++)
		savedPlaceStates[i] = in->readUint32BE();

	// Game variables
	for (uint i = 0; i < 100; i++) {
		uint val = in->readUint32BE();
		if (i < _gameVariables.size())
			_gameVariables[i] = val;
	}

	// Extended block (optional — present when file > kSaveSize)
	bool hasExt = (in->size() > kSaveSize);
	Common::Array<int> savedHiddenPersos;
	Common::Array<SujState> savedSujEnabled;
	Common::HashMap<Common::String, int> savedGameVars;
	uint32 savedCONChapter = 0;  // ext v2+: active CHAPI*.CON chapter
	int savedMusicTrack = 0;     // ext v3+: selected music track

	if (hasExt) {
		uint32 magic = in->readUint32BE();
		uint32 ver   = in->readUint32BE();

		if (magic != kExtMagic || ver < 1) {
			warning("loadGame: unrecognised extended block (magic=%08x ver=%u)", magic, ver);
			hasExt = false;
		} else {
			// Hidden persos
			uint32 nHidden = in->readUint32BE();
			for (uint32 i = 0; i < nHidden && !in->eos(); i++)
				savedHiddenPersos.push_back(in->readSint32BE());

			// Enabled subjects
			uint32 nSuj = in->readUint32BE();
			for (uint32 i = 0; i < nSuj && !in->eos(); i++) {
				SujState ss;
				ss.persoId = in->readSint32BE();
				uint32 nK = in->readUint32BE();
				for (uint32 j = 0; j < nK && !in->eos(); j++)
					ss.sujs.push_back(in->readSint32BE());
				savedSujEnabled.push_back(ss);
			}

			// Named game variables
			uint32 nVars = in->readUint32BE();
			for (uint32 i = 0; i < nVars && !in->eos(); i++) {
				uint32 klen = in->readUint32BE();
				if (klen > 256) break;  // sanity
				char kbuf[257];
				in->read(kbuf, klen);
				kbuf[klen] = '\0';
				int32 val = in->readSint32BE();
				savedGameVars[Common::String(kbuf)] = val;
			}

			// CON script chapter (ext v2+).
			if (ver >= 2 && !in->eos())
				savedCONChapter = in->readUint32BE();

			// Selected music track (ext v3+).
			if (ver >= 3 && !in->eos())
				savedMusicTrack = (int)in->readUint32BE();
		}
	}

	delete in;

	// Restore game state
	_currentChapter = chapter;

	// Load the base chapter WAM (sets up chapter-level place state callbacks, loads CON,
	// and runs INIT commands — which establishes baseline _sujEnabled/_gameVars).
	setupChapterWAM((int)chapter);

	// If a sub-WAM was active, load it over the chapter WAM.
	Common::String wamName(wamNameC);
	if (!wamName.empty() && !wamName.equalsIgnoreCase(_currentWAMName))
		setupWAMByName(wamName);

	// Restore the active CON script.  Which CHAPI*.CON is loaded is driven by
	// the CON `chapter=N` command, independent of the WAM chapter used above,
	// so loadGame must reload it explicitly or a save made past chapter 1
	// would resume running chapter 1's script.  The chapter is read from the
	// ext-v2 field (savedCONChapter) — not a game variable, since
	// loadConScript() clears _gameVars.  Do this before the extended block is
	// applied: loadConScript() clears _sujEnabled/_gameVars and re-runs the
	// chapter's INIT block, which the extended-block restore below then
	// overrides with the saved state.
	//
	// ALWAYS re-load — even when the saved chapter matches _currentCONChapter.
	// loadConScript() clears the transient half of _gameVars (every var
	// except the 7 persistent ones the save actually carries), and a game-over
	// reload of the *current* chapter is the canonical case where that matters:
	// without it, post-death values of time2, wsprframe, flagtemp3, flaggarde,
	// etc. survive into the reloaded checkpoint and the next /tim2 tick at the
	// next place re-fires the same death chain on its first frame.
	if (savedCONChapter >= 1) {
		_currentCONChapter = savedCONChapter;
		loadConScript((int)savedCONChapter);
	}

	// Restore camera and place.
	_nextPlaceId = placeId;
	_omni3dMan.setAlpha(alpha);
	_omni3dMan.setBeta(beta);

	// Restore place states.
	for (uint i = 0; i < _placeStates.size() && i < 256; i++)
		_placeStates[i].state = savedPlaceStates[i];

	// NbVisite counters are not yet persisted in the save: start the loaded
	// game with a fresh set so first-visit /sel sections behave predictably.
	for (byte &b : _placeVisits)
		b = 0;

	// Reset every persistent CON variable before the save's values are
	// applied.  setupChapterWAM() -> loadConScript() above carries the 7
	// persistent vars (VARIAS.CON idx 0-6) across, so a var that is live now
	// but ABSENT from the save would otherwise survive the load unchanged.
	// A persistent var missing from the save was 0 when the save was written
	// (saveGame only emits keys that exist in _gameVars), so 0 is its correct
	// restored value.  Without this, a game-over reload restores the very
	// state that triggered it — e.g. FlagChp8 stays 2, so arriving back at
	// place 133 immediately re-fires the death /sel and loops forever.
	{
		Common::Array<Common::String> persistKeys;
		for (const auto &kv : _gameVars)
			if (isPersistentGameVar(kv._key))
				persistKeys.push_back(kv._key);
		for (const Common::String &k : persistKeys)
			_gameVars[k] = 0;
	}

	// Overlay CON script state from extended block (overrides INIT-derived baseline).
	if (hasExt) {
		_hiddenPersos = savedHiddenPersos;
		_sujEnabled   = savedSujEnabled;
		// Restore only the persistent variables; ignore any transient ones a
		// save written by an older build may still contain.
		for (const auto &kv : savedGameVars)
			if (isPersistentGameVar(kv._key))
				_gameVars[kv._key] = kv._value;
	}

	// Restore the music selection last: setupChapterWAM() / loadConScript()
	// above may have run musicStop(); musicUpdate() picks the saved track
	// back up on the next frame.
	_musicTrackId = savedMusicTrack;

	// A load is not itself a chapter transition: drop any autosave armed
	// before the load so the next place change does not redundantly re-save.
	_autosavePending = false;

	// Drop any object held on the cursor.  The held object is transient UI
	// state, not part of the save (ObjetEnMain is a transient CON variable,
	// already cleared by loadConScript) -- the original load path likewise
	// zeroes it (atlantis.exe FUN_0042de5c, _DAT_006cd7e4 = 0).  Without
	// this, a game-over reload leaves the player still carrying whatever was
	// in hand when they died.
	_inventory.deselectObject();

	// The player is now positioned in the loaded episode: point their resume
	// pointer at it so a later relaunch reloads this chapter (and not, say, a
	// further episode that was reached then abandoned).  Mirrors the original
	// load screen writing the picked save's chapter back to the per-player
	// chapter byte (atlantis.exe @ 0x6ce246).
	if (_currentPlayer >= 0 && _currentPlayer < (int)kMaxPlayers &&
	        _currentCONChapter >= 1 && _currentCONChapter < kPlayerSlotStride) {
		_playerChapter[_currentPlayer] = (int)_currentCONChapter;
		savePlayers();
	}

	return true;
}

// --- Player profiles ------------------------------------------------------
// The list of named players is persisted in the save directory as
// "atlantis-players.dat" — a name that cannot collide with the "atlantis.NNNN"
// save-game files.  Each of the kMaxPlayers fixed slots holds a name, or an
// empty string when unused; a slot's index is the player's stable id.

static const char  *kPlayersFile    = "atlantis-players.dat";
static const uint32 kPlayersMagic   = 0x41504c52u;  // 'APLR'
// Version 2 appends a per-player current-chapter array — the resume pointer.
// A version-1 file (no array) still loads; those players fall back to the
// legacy single-autosave slot (see playerResumeSave()).
static const uint32 kPlayersVersion = 2;

void CryOmni3DEngine_Atlantis::loadPlayers() {
	for (uint i = 0; i < kMaxPlayers; i++) {
		_players[i].clear();
		_playerChapter[i] = 0;
	}

	Common::InSaveFile *in = _saveFileMan->openForLoading(kPlayersFile);
	if (!in)
		return;
	uint32 magic   = in->readUint32BE();
	uint32 version = in->readUint32BE();
	if (magic == kPlayersMagic && version >= 1) {
		uint32 count = in->readUint32BE();
		for (uint32 i = 0; i < count && i < kMaxPlayers && !in->eos(); i++) {
			uint16 len = in->readUint16BE();
			Common::String name;
			for (uint16 j = 0; j < len && !in->eos(); j++)
				name += (char)in->readByte();
			_players[i] = name;
		}
		// Version 2+: the per-player resume chapter follows the names.
		if (version >= 2) {
			for (uint32 i = 0; i < count && i < kMaxPlayers && !in->eos(); i++)
				_playerChapter[i] = in->readSint32BE();
		}
	}
	delete in;
}

void CryOmni3DEngine_Atlantis::savePlayers() {
	Common::OutSaveFile *out = _saveFileMan->openForSaving(kPlayersFile, false);
	if (!out) {
		warning("savePlayers: cannot write %s", kPlayersFile);
		return;
	}
	out->writeUint32BE(kPlayersMagic);
	out->writeUint32BE(kPlayersVersion);
	out->writeUint32BE(kMaxPlayers);
	for (uint i = 0; i < kMaxPlayers; i++) {
		out->writeUint16BE((uint16)_players[i].size());
		out->write(_players[i].c_str(), _players[i].size());
	}
	for (uint i = 0; i < kMaxPlayers; i++)
		out->writeSint32BE(_playerChapter[i]);
	out->finalize();
	delete out;
}

void CryOmni3DEngine_Atlantis::deletePlayerSaves(uint player) {
	// Every local slot of the block may hold a per-episode save.
	for (uint i = 0; i < kPlayerSlotStride; i++)
		_saveFileMan->removeSavefile(
		    getSaveStateName(episodeSaveSlot(player, i)));
	if (player < kMaxPlayers)
		_playerChapter[player] = 0;
}

// --- Per-episode saves ----------------------------------------------------
// Atlantis has no manual save: the game keeps one named save per episode and
// rewrites it whenever that episode is (re-)entered.  Chapter N's save lives
// at local slot N of the player's block; the per-player resume chapter
// (_playerChapter) tracks which one a relaunch should reload.

bool CryOmni3DEngine_Atlantis::saveSlotExists(int absSlot) const {
	Common::InSaveFile *in = _saveFileMan->openForLoading(getSaveStateName(absSlot));
	if (!in)
		return false;
	delete in;
	return true;
}

uint CryOmni3DEngine_Atlantis::playerResumeSave(uint player) const {
	if (player >= kMaxPlayers)
		return 0;
	// Preferred: the player's tracked current chapter.
	int ch = _playerChapter[player];
	if (ch >= 1 && ch < (int)kPlayerSlotStride) {
		int slot = episodeSaveSlot(player, (uint)ch);
		if (saveSlotExists(slot))
			return (uint)slot + 1;
	}
	// Fallback: a save written by the pre-episode build sat at local slot 0.
	int legacy = episodeSaveSlot(player, 0);
	if (saveSlotExists(legacy))
		return (uint)legacy + 1;
	return 0;
}

bool CryOmni3DEngine_Atlantis::playerHasSave(uint player) const {
	return playerResumeSave(player) != 0;
}

void CryOmni3DEngine_Atlantis::autosave() {
	// The slot is the chapter number; the description is its EPI.TXT
	// checkpoint name.  Chapters outside a player's block (the end-game
	// sentinels chapter=99/999) and unused chapter numbers — which have no
	// EPI.TXT name — get no checkpoint, matching the original.
	uint chapter = _currentCONChapter;
	if (chapter < 1 || chapter >= kPlayerSlotStride)
		return;
	Common::String name = episodeName(chapter);
	if (name.empty())
		return;
	saveGame((uint)episodeSaveSlot((uint)_currentPlayer, chapter) + 1, name);
	if (_currentPlayer >= 0 && _currentPlayer < (int)kMaxPlayers) {
		_playerChapter[_currentPlayer] = (int)chapter;
		savePlayers();
	}
}

// SPRLIST\EPI.TXT — a CR/LF-separated list of episode checkpoint names,
// terminated by a `/FIN` line.  Parsed once at startup; episodeName() then
// serves chapter N from index N - 1.
void CryOmni3DEngine_Atlantis::loadEpisodeNames() {
	_episodeNames.clear();

	Common::ScopedPtr<Common::SeekableReadStream> s(
	    openBigFileStream(kFileTypeSprite, "EPI.TXT"));
	if (!s) {
		warning("loadEpisodeNames: cannot open SPRLIST\\EPI.TXT");
		return;
	}

	uint32 sz = (uint32)s->size();
	Common::Array<char> buf;
	buf.resize(sz + 1);
	if (sz)
		s->read(buf.data(), sz);
	buf[sz] = '\0';

	Common::String cur;
	for (uint32 i = 0; i <= sz; i++) {
		char c = buf[i];
		if (c == '\n' || c == '\0') {
			if (cur.equalsIgnoreCase("/FIN"))
				break;
			_episodeNames.push_back(cur);
			cur.clear();
		} else if (c != '\r') {
			cur += c;
		}
	}
	debugC(1, kDebugSaveLoad, "loadEpisodeNames: %u episode checkpoint names", _episodeNames.size());
}

} // namespace Atlantis
} // namespace CryOmni3D
