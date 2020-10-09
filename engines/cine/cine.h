/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CINE_CINE_H
#define CINE_CINE_H


#include "common/scummsys.h"
#include "common/file.h"
#include "common/util.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"

#include "engines/engine.h"

#include "cine/texte.h"
#include "cine/rel.h"
#include "cine/script.h"
#include "cine/part.h"
#include "cine/prc.h"
#include "cine/msg.h"
#include "cine/bg.h"
#include "cine/pal.h"
#include "cine/gfx.h"
#include "cine/anim.h"
#include "cine/bg_list.h"
#include "cine/various.h"
#include "cine/console.h"
#include "cine/sound.h"
#include "cine/detection.h"

//#define DUMP_SCRIPTS

/**
 * This is the namespace of the Cine engine.
 *
 * Status of this engine:
 *
 * This enigne has 2 generations Cinematique evo.1 and Cinematique evo.2
 * first generation is fairly complete, and second one is under development
 *
 * Cinematique evo.1 status:
 * The engine supports Future Wars and is basically complete with support of
 * all known game variants. Based on Yaz0r's engine.
 *
 * Cinematique evo.2 status:
 * This generation supports Operation Stealth, originally developed by Yaz0r for
 * French variant of the game which heared to be completable.
 * Later the work was renewed as part of GSoC'08, by Kari Salminen, but it has not
 * yet been finished. The game is not completable.
 *
 *
 * Games using this engine:
 *
 * Cinematique evo.1
 * - Future Wars
 *
 * Cinematique evo.2
 * - Operation Stealth
 *
 */
namespace Cine {

struct SeqListElement;

struct VolumeResource {
	char name[10];
	uint32 pNamesList;
	int16 diskNum;
	int32 sizeOfNamesList;
};

typedef Common::HashMap<Common::String, Common::Array<VolumeResource> > StringToVolumeResourceArrayHashMap;

class CineConsole;

class CineEngine : public Engine {

protected:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

	void shutdown();

	bool initGame();

public:
	CineEngine(OSystem *syst, const CINEGameDescription *gameDesc);
	~CineEngine() override;

	void syncSoundSettings() override;

	bool mayHave256Colors() const;
	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	bool loadSaveDirectory();
	void makeSystemMenu();
	int scummVMSaveLoadDialog(bool isSave);
	int modifyGameSpeed(int speedChange);
	void setDefaultGameSpeed();
	uint32 getTimerDelay() const;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	virtual Common::String getSaveStateName(int slot) const override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;

	const CINEGameDescription *_gameDescription;
	Common::File _partFileHandle;

	Common::RandomSource _rnd;

	StringToVolumeResourceArrayHashMap _volumeEntriesMap;

	TextHandler _textHandler;

	bool _restartRequested;

private:
	void initialize();
	void showSplashScreen();
	void resetEngine();
	bool checkSaveHeaderData(const ChunkHeader& hdr);
	bool loadPlainSaveFW(Common::SeekableReadStream &in, CineSaveGameFormat saveGameFormat, uint32 version);	
	bool loadVersionedSaveFW(Common::SeekableReadStream &in);
	bool loadVersionedSaveOS(Common::SeekableReadStream &in);
	bool makeLoad(const Common::String &saveName);
	void writeSaveHeader(Common::OutSaveFile &out, uint32 headerId);
	void makeSaveFW(Common::OutSaveFile &out);
	void makeSaveOS(Common::OutSaveFile &out);
	void makeSave(const Common::String &saveFileName, uint32 playtime,
		Common::String desc, bool isAutosave);
	void mainLoop(int bootScriptIdx);
	void readVolCnf();
	Common::String getTargetSaveStateName(Common::String target, int slot) const;

	bool _preLoad;
	int _timerDelayMultiplier;

public:
	// TODO: These are pseudo-global vars
	// They better belong to appropriate classes
	Common::Array<AnimData> _animDataTable;
	Common::List<BGIncrust> _bgIncrustList;
	Common::StringArray _messageTable;
	Common::Array<ObjectStruct> _objectTable;
	Common::List<overlay> _overlayList;
	Common::Array<PalEntry> _palArray;
	Common::Array<PartBuffer> _partBuffer;
	ScriptList _globalScripts;
	ScriptList _objectScripts;
	RawObjectScriptArray _relTable; ///< Object script bytecode table

	/**
	 * Global variables.
	 * 255 of these are saved, but there's one more that's used for bypassing the copy protection.
	 * In CineEngine::mainLoop(int bootScriptIdx) there's this code: globalVars[VAR_BYPASS_PROTECTION] = 0;
	 * And as VAR_BYPASS_PROTECTION is 255 that's why we're allocating one more than we otherwise would.
	 */
	ScriptVars _globalVars;
	RawScriptArray _scriptTable; ///< Table of script bytecode

	Common::Array<int16> _zoneData;
	Common::Array<uint16> _zoneQuery; ///< Only exists in Operation Stealth

	Common::List<SeqListElement> _seqList;

	Common::String _commandBuffer;
	Common::Array<Common::KeyState> _keyInputList;
};

extern CineEngine *g_cine;
extern Sound *g_sound;

#define BOOT_PRC_NAME "AUTO00.PRC"
#define BOOT_SCRIPT_INDEX 1
#define COPY_PROT_FAIL_PRC_NAME "L201.ANI"

enum {
	// Both FW and OS
	VAR_MOUSE_X_POS = 249,
	VAR_MOUSE_Y_POS = 250,
	// FW only
	VAR_MOUSE_X_MODE = 253,
	VAR_MOUSE_Y_MODE = 251,
	// OS only
	VAR_MOUSE_X_POS_2ND = 251, // Many times used in conjunction with VAR_MOUSE_X_POS
	VAR_MOUSE_Y_POS_2ND = 252, // Many times used in conjunction with VAR_MOUSE_Y_POS
	VAR_BYPASS_PROTECTION = 255,
	VAR_LOW_MEMORY = 0
};

enum {
	MOUSE_CURSOR_NORMAL = 0,
	MOUSE_CURSOR_DISK,
	MOUSE_CURSOR_CROSS
};

enum {
	kCineDebugScript    = 1 << 0,
	kCineDebugPart      = 1 << 1,
	kCineDebugSound     = 1 << 2,
	kCineDebugCollision = 1 << 3
};

enum {
	kCmpEQ = (1 << 0),
	kCmpGT = (1 << 1),
	kCmpLT = (1 << 2)
};


} // End of namespace Cine

#endif
