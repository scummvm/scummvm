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
#include "mohawk/riven_scripts.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/rect.h"

namespace Mohawk {

struct MohawkGameDescription;
class MohawkArchive;
class RivenGraphics;
class RivenConsole;
class RivenSaveLoad;
class RivenOptionsDialog;
class RivenStack;
class RivenCard;
class RivenHotspot;
class RivenSoundManager;
class RivenInventory;

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

// Engine Debug Flags
enum {
	kRivenDebugScript   = (1 << 0)
};

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

	VideoManager *_video;
	RivenSoundManager *_sound;
	RivenGraphics *_gfx;
	Common::RandomSource *_rnd;
	RivenScriptManager *_scriptMan;
	RivenInventory *_inventory;

	GUI::Debugger *getDebugger();

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool hasFeature(EngineFeature f) const;

	typedef Common::Functor0<void> TimerProc;

#define TIMER(cls, method) \
		new Common::Functor0Mem<void, cls>(this, &cls::method)

	void doVideoTimer(VideoHandle handle, bool force);
	void doFrame();

private:
	MohawkArchive *_extrasFile; // We need a separate handle for the extra data
	RivenConsole *_console;
	RivenSaveLoad *_saveLoad;
	RivenOptionsDialog *_optionsDialog;
	InstallerArchive _installerArchive;

	// Stack/Card-related functions and variables
	RivenCard *_card;
	RivenStack *_stack;

	// Hotspot related functions and variables
	bool _showHotspots;

	// Variables
	void initVars();

	// Timer
	Common::SharedPtr<TimerProc> _timerProc;
	uint32 _timerTime;

	void pauseEngineIntern(bool) override;
public:
	// Stack/card/script funtions
	RivenStack *constructStackById(uint16 id);
	void changeToCard(uint16 dest);
	void changeToStack(uint16);
	void refreshCard();
	RivenCard *getCard() const { return _card; }
	RivenStack *getStack() const { return _stack; }

	// Hotspot functions/variables
	Common::Array<ZipMode> _zipModeData;
	void updateCurrentHotspot();
	void addZipVisitedCard(uint16 cardId, uint16 cardNameId);
	bool isZipVisitedCard(const Common::String &hotspotName) const;

	// Variables
	RivenVariableMap _vars;
	uint32 &getStackVar(uint32 index);

	// Miscellaneous
	Common::SeekableReadStream *getExtrasResource(uint32 tag, uint16 id);
	bool _activatedPLST;
	bool _activatedSLST;
	void runLoadDialog();
	void delayAndUpdate(uint32 ms);

	// Timer
	void installTimer(TimerProc *proc, uint32 time);
	void checkTimer();
	void removeTimer();
};

} // End of namespace Mohawk

#endif
