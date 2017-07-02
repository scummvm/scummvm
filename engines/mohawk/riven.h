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

#ifndef MOHAWK_RIVEN_H
#define MOHAWK_RIVEN_H

#include "mohawk/installer_archive.h"
#include "mohawk/mohawk.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_scripts.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/rect.h"

namespace Mohawk {

struct MohawkGameDescription;
class MohawkArchive;
class RivenGraphics;
class RivenExternal;
class RivenConsole;
class RivenSaveLoad;
class RivenOptionsDialog;
class RivenCard;
class RivenHotspot;
class RivenSoundManager;

// Riven Stack Types
enum {
	kStackUnknown = 0, // Default value for ReturnStackID
	kStackOspit = 1,   // 233rd Age (Gehn's Office)
	kStackPspit = 2,   // Prison Island
	kStackRspit = 3,   // Temple Island
	kStackTspit = 4,   // Rebel Age (Tay)
	kStackBspit = 5,   // Book-Making Island
	kStackGspit = 6,   // Garden Island
	kStackJspit = 7,   // Jungle Island
	kStackAspit = 8,   // Main Menu, Books, Setup

	kStackFirst = kStackOspit,
	kStackLast = kStackAspit
};

// NAME Resource ID's
enum {
	kCardNames = 1,
	kHotspotNames = 2,
	kExternalCommandNames = 3,
	kVariableNames = 4,
	kStackNames = 5
};

enum RivenTransitionSpeed {
	kRivenTransitionSpeedNone = 5000,
	kRivenTransitionSpeedFastest = 5001,
	kRivenTransitionSpeedNormal = 5002,
	kRivenTransitionSpeedBest = 5003
};

// Rects for the inventory object positions (initialized in
// MohawkEngine_Riven's constructor).
extern Common::Rect *g_atrusJournalRect1;
extern Common::Rect *g_atrusJournalRect2;
extern Common::Rect *g_cathJournalRect2;
extern Common::Rect *g_atrusJournalRect3;
extern Common::Rect *g_cathJournalRect3;
extern Common::Rect *g_trapBookRect3;
extern Common::Rect *g_demoExitRect;

struct ZipMode {
	Common::String name;
	uint16 id;
	bool operator== (const ZipMode& z) const;
};

typedef Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> RivenVariableMap;

class MohawkEngine_Riven : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_Riven(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_Riven();

	RivenSoundManager *_sound;
	RivenGraphics *_gfx;
	RivenExternal *_externalScriptHandler;
	Common::RandomSource *_rnd;
	RivenScriptManager *_scriptMan;

	RivenCard *_card;

	GUI::Debugger *getDebugger();

	bool canLoadGameStateCurrently() { return !(getFeatures() & GF_DEMO); }
	bool canSaveGameStateCurrently() { return !(getFeatures() & GF_DEMO); }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool hasFeature(EngineFeature f) const;

	typedef void (*TimerProc)(MohawkEngine_Riven *vm);

	void doVideoTimer(VideoHandle handle, bool force);

private:
	MohawkArchive *_extrasFile; // We need a separate handle for the extra data
	RivenConsole *_console;
	RivenSaveLoad *_saveLoad;
	RivenOptionsDialog *_optionsDialog;
	InstallerArchive _installerArchive;

	// Stack/Card-related functions and variables
	uint16 _curStack;
	void handleEvents();

	// Stack resource names
	RivenNameList _varNames;
	RivenNameList _externalCommandNames;
	RivenNameList _hotspotNames;
	RivenNameList _cardNames;
	RivenNameList _stackNames;

	// Hotspot related functions and variables
	void checkInventoryClick();
	bool _showHotspots;

	// Variables
	void initVars();

	// Timer
	TimerProc _timerProc;
	uint32 _timerTime;

	// Miscellaneous
	bool _gameOver;
	bool _ignoreNextMouseUp;
	void checkSunnerAlertClick();

public:
	// Stack/card/script funtions
	void changeToCard(uint16 dest);
	void changeToStack(uint16);
	void refreshCard();
	Common::String getName(uint16 nameResource, uint16 nameID);
	int16 getIdFromName(uint16 nameResource, const Common::String &name);
	Common::String getStackName(uint16 stack) const;
	RivenCard *getCurCard() const { return _card; }
	uint16 getCurStack() const { return _curStack; }
	uint16 matchRMAPToCard(uint32);
	uint32 getCurCardRMAP();

	// Hotspot functions/variables
	Common::Array<ZipMode> _zipModeData;
	void updateCurrentHotspot();
	void addZipVisitedCard(uint16 cardId, uint16 cardNameId);
	bool isZipVisitedCard(const Common::String &hotspotName) const;

	// Variables
	RivenVariableMap _vars;
	uint32 &getStackVar(uint32 index);

	// Miscellaneous
	void setGameOver() { _gameOver = true; }
	void ignoreNextMouseUp() { _ignoreNextMouseUp = true; }
	Common::SeekableReadStream *getExtrasResource(uint32 tag, uint16 id);
	bool _activatedPLST;
	bool _activatedSLST;
	void runLoadDialog();
	void delayAndUpdate(uint32 ms);

	// Timer
	void installTimer(TimerProc proc, uint32 time);
	void installCardTimer();
	void checkTimer();
	void removeTimer();
};

} // End of namespace Mohawk

#endif
