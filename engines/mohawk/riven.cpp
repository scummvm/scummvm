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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
 
#include "common/config-manager.h"
#include "common/events.h"
#include "common/keyboard.h" 

#include "mohawk/graphics.h"
#include "mohawk/riven.h"
#include "mohawk/riven_external.h"
#include "mohawk/riven_saveload.h"

namespace Mohawk {

MohawkEngine_Riven::MohawkEngine_Riven(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	_showHotspots = false;
	_cardData.hasData = false;
	_gameOver = false;
	_activatedSLST = false;
	_extrasFile = NULL;

	// Attempt to let game run from the CD's
	// NOTE: assets2 contains higher quality audio than assets1
	SearchMan.addSubDirectoryMatching(_gameDataDir, "all");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "exe");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "assets2");
}

MohawkEngine_Riven::~MohawkEngine_Riven() {
	delete _gfx;
	delete _console;
	delete _externalScriptHandler;
	delete _extrasFile;
	delete _saveLoad;
	delete[] _vars;
	delete _loadDialog;
	delete _optionsDialog;
	_cardData.scripts.clear();
}

Common::Error MohawkEngine_Riven::run() {
	MohawkEngine::run();

	_gfx = new RivenGraphics(this);
	_console = new RivenConsole(this);
	_saveLoad = new RivenSaveLoad(this, _saveFileMan);
	_externalScriptHandler = new RivenExternal(this);
	_loadDialog = new GUI::SaveLoadChooser("Load Game:", "Load");
	_loadDialog->setSaveMode(false);
	_optionsDialog = new RivenOptionsDialog(this);

	initVars();

	// Open extras.mhk for common images (non-existant in the demo)
	if (!(getFeatures() & GF_DEMO)) {
		_extrasFile = new MohawkFile();
		_extrasFile->open("extras.mhk");
	}

	// Start at main cursor
	_gfx->changeCursor(kRivenMainCursor);

	// Load game from launcher/command line if requested
	if (ConfMan.hasKey("save_slot") && !(getFeatures() & GF_DEMO)) {
		uint32 gameToLoad = ConfMan.getInt("save_slot");
		Common::StringList savedGamesList = _saveLoad->generateSaveGameList();
		if (gameToLoad > savedGamesList.size())
			error ("Could not find saved game");
		_saveLoad->loadGame(savedGamesList[gameToLoad]);
		// HACK: The save_slot variable is saved to the disk! We don't want this!
		ConfMan.removeKey("save_slot", ConfMan.getActiveDomainName());
		ConfMan.flushToDisk();
	} else { // Otherwise, start us off at aspit's card 1
        changeToStack(aspit);
		changeToCard(1);
	}

	Common::Event event;
	while (!_gameOver) {
		bool needsUpdate = _gfx->runScheduledWaterEffects();

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					_mousePos = event.mouse;
					checkHotspotChange();
					
					// Check to show the inventory
					if (_mousePos.y >= 392)
						_gfx->showInventory();
					else
						_gfx->hideInventory();
					
					needsUpdate = true;
					break;
				case Common::EVENT_LBUTTONDOWN:
					if (_curHotspot >= 0) {
						runHotspotScript(_curHotspot, kMouseDownScript);
						//scheduleScript(_hotspots[_curHotspot].script, kMouseMovedPressedReleasedScript);
					}
					break;
				case Common::EVENT_LBUTTONUP:
					if (_curHotspot >= 0) {
						runHotspotScript(_curHotspot, kMouseUpScript);
						//scheduleScript(_hotspots[_curHotspot].script, kMouseMovedPressedReleasedScript);
					} else
						checkInventoryClick();
					break;
				case Common::EVENT_KEYDOWN:
					switch (event.kbd.keycode) {
						case Common::KEYCODE_d:
							if (event.kbd.flags & Common::KBD_CTRL) {
								_console->attach();
								_console->onFrame();
							}
							break;
						case Common::KEYCODE_SPACE:
							pauseGame();
							break;
						case Common::KEYCODE_F4:
							_showHotspots = !_showHotspots;
							if (_showHotspots) {
								for (uint16 i = 0; i < _hotspotCount; i++)
									_gfx->drawRect(_hotspots[i].rect, _hotspots[i].enabled);
								needsUpdate = true;
							} else {
								changeToCard();
							}
							break;
						case Common::KEYCODE_F5:
							runDialog(*_optionsDialog);
							updateZipMode();
							break;
						case Common::KEYCODE_ESCAPE:
							if (getFeatures() & GF_DEMO) {
								if (_curStack != aspit)
									changeToStack(aspit);
								changeToCard(1);
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}

		if (_curHotspot >= 0) {
			runHotspotScript(_curHotspot, kMouseInsideScript);
			//scheduleScript(_hotspots[_curHotspot].script, kMouseMovedPressedReleasedScript);
		}

		if (shouldQuit()) {
			if (_eventMan->shouldRTL() && (getFeatures() & GF_DEMO) && !(_curStack == aspit && _curCard == 12)) {
				if (_curStack != aspit)
					changeToStack(aspit);
				changeToCard(12);
				_eventMan->resetRTL();
				continue;
			}			
			return Common::kNoError;
		}

		// Update the screen if we need to
		if (needsUpdate)
			_system->updateScreen();

		// Cut down on CPU usage
		_system->delayMillis(10);
	}

	return Common::kNoError;
}

// Stack/Card-Related Functions

void MohawkEngine_Riven::changeToStack(uint16 n) {
	// The endings are in reverse order because of the way the 1.02 patch works.
	// The only "Data3" file is j_Data3.mhk from that patch. Patch files have higher
	// priorities over the regular files and are therefore loaded and checked first.
	static const char *endings[] = { "_Data3.mhk", "_Data2.mhk", "_Data1.mhk", "_Data.mhk" };

	// Don't change stack to the current stack (if the files are loaded)
	if (_curStack == n && !_mhk.empty())
		return;
		
	_curStack = n;

	// Clear the old stack files out
	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];
	_mhk.clear();

	// Get the prefix character for the destination stack
	char prefix = getStackName(_curStack)[0];
	
	// Load any file that fits the patterns
	for (int i = 0; i < ARRAYSIZE(endings); i++) {
		Common::String filename = Common::String(prefix) + endings[i];
		if (Common::File::exists(filename)) {
			MohawkFile *mhk = new MohawkFile();
			mhk->open(filename);
			_mhk.push_back(mhk);
		}
	}
	
	// Make sure we have loaded files
	if (_mhk.empty())
		error("Could not load stack %s", getStackName(_curStack).c_str());
	
	// Stop any currently playing sounds and load the new sound file too
	_sound->stopAllSLST();
	_sound->loadRivenSounds(_curStack);
}

// Riven uses some hacks to change stacks for linking books
// Otherwise, script command 27 changes stacks
struct RivenSpecialChange {
	byte startStack;
	uint32 startCardRMAP;
	byte targetStack;
	uint32 targetCardRMAP;
} rivenSpecialChange[] = {
	{ aspit,  0x1f04, ospit,  0x44ad },		// Trap Book
	{ bspit, 0x1c0e7, ospit,  0x2e76 },		// Dome Linking Book
	{ gspit, 0x111b1, ospit,  0x2e76 },		// Dome Linking Book
	{ jspit, 0x28a18, rspit,   0xf94 },		// Tay Linking Book
	{ jspit, 0x26228, ospit,  0x2e76 },		// Dome Linking Book
	{ ospit,  0x5f0d, pspit,  0x3bf0 },		// Return from 233rd Age
	{ ospit,  0x470a, jspit, 0x1508e },		// Return from 233rd Age
	{ ospit,  0x5c52, gspit, 0x10bea },		// Return from 233rd Age
	{ ospit,  0x5d68, bspit, 0x1adfd },		// Return from 233rd Age
	{ ospit,  0x5e49, tspit,   0xe78 },		// Return from 233rd Age
	{ pspit,  0x4108, ospit,  0x2e76 },		// Dome Linking Book
	{ rspit,  0x32d8, jspit, 0x1c474 },		// Return from Tay
	{ tspit, 0x21b69, ospit,  0x2e76 }		// Dome Linking Book
};

void MohawkEngine_Riven::changeToCard(uint16 n) {
	bool refreshMode = (n == 0);

	// While this could be run without harm, it doesn't need to be. This should add a speed boost.
	if (!refreshMode) {
		debug (1, "Changing to card %d", n);
		_curCard = n;

		if (!(getFeatures() & GF_DEMO)) {
			for (byte i = 0; i < 13; i++)
				if (_curStack == rivenSpecialChange[i].startStack && _curCard == matchRMAPToCard(rivenSpecialChange[i].startCardRMAP)) {
					changeToStack(rivenSpecialChange[i].targetStack);
					_curCard = matchRMAPToCard(rivenSpecialChange[i].targetCardRMAP);
				}
		}

		if (_cardData.hasData)
			runCardScript(kCardLeaveScript);

		loadCard(_curCard);
	}
	
	// We need to reload hotspots when refreshing, however
	loadHotspots(_curCard);
	
	_gfx->_updatesEnabled = true;
	_gfx->clearWaterEffects();
	_gfx->_activatedPLSTs.clear();
	_video->_mlstRecords.clear();
	_gfx->drawPLST(1);
	_activatedSLST = false;
	
	runCardScript(kCardLoadScript);
	_gfx->updateScreen();
	runCardScript(kCardOpenScript);
	
	// Activate the first sound list if none have been activated
	if (!_activatedSLST)
		_sound->playSLST(1, _curCard);
	
	if (_showHotspots) {
		for (uint16 i = 0; i < _hotspotCount; i++)
			_gfx->drawRect(_hotspots[i].rect, _hotspots[i].enabled);
	}
	
	// Now we need to redraw the cursor if necessary and handle mouse over scripts
	_curHotspot = -1;
	checkHotspotChange();
}

void MohawkEngine_Riven::loadCard(uint16 id) {
	// NOTE: Do not clear the card scripts because it may delete a currently running script!

	Common::SeekableReadStream* inStream = getRawData(ID_CARD, id);

	_cardData.name = inStream->readSint16BE();
	_cardData.zipModePlace = inStream->readUint16BE();
	_cardData.scripts = RivenScript::readScripts(this, inStream);
	_cardData.hasData = true;

	delete inStream;

	if (_cardData.zipModePlace) {
		Common::String cardName = getName(CardNames, _cardData.name);
		if (cardName.empty())
			return;
		ZipMode zip;
		zip.name = cardName;
		zip.id = id;
		if (!(Common::find(_zipModeData.begin(), _zipModeData.end(), zip) != _zipModeData.end()))
			_zipModeData.push_back(zip);
	}
}

void MohawkEngine_Riven::loadHotspots(uint16 id) {
	// NOTE: Do not clear the hotspots because it may delete a currently running script!

	Common::SeekableReadStream* inStream = getRawData(ID_HSPT, id);

	_hotspotCount = inStream->readUint16BE();
	_hotspots = new RivenHotspot[_hotspotCount];


	for (uint16 i = 0; i < _hotspotCount; i++) {
		_hotspots[i].enabled = true;

		_hotspots[i].blstID = inStream->readUint16BE();
		_hotspots[i].name_resource = inStream->readSint16BE();
		
		int16 left = inStream->readSint16BE();
		int16 top = inStream->readSint16BE();
		int16 right = inStream->readSint16BE();
		int16 bottom = inStream->readSint16BE();
		
		// Riven has some weird hotspots, disable them here
		if (left >= right || top >= bottom) {
			left = top = right = bottom = 0;
			_hotspots[i].enabled = 0;
		}

		_hotspots[i].rect = Common::Rect(left, top, right, bottom);
		
		_hotspots[i].u0 = inStream->readUint16BE();

		if (_hotspots[i].u0 != 0)
			warning("Hotspot %d u0 non-zero", i);

		_hotspots[i].mouse_cursor = inStream->readUint16BE();
		_hotspots[i].index = inStream->readUint16BE();
		_hotspots[i].u1 = inStream->readSint16BE();

		if (_hotspots[i].u1 != -1)
			warning("Hotspot %d u1 not -1", i);

		_hotspots[i].zipModeHotspot = inStream->readUint16BE();
		
		// Read in the scripts now
		_hotspots[i].scripts = RivenScript::readScripts(this, inStream);
	}

	delete inStream;
	updateZipMode();
}

void MohawkEngine_Riven::updateZipMode() {
	// Check if a zip mode hotspot is enabled by checking the name/id against the ZIPS records.

	for (uint32 i = 0; i < _hotspotCount; i++) {
		if (_hotspots[i].zipModeHotspot) {
			if (*matchVarToString("azip") != 0) {
				// Check if a zip mode hotspot is enabled by checking the name/id against the ZIPS records.
				Common::String hotspotName = getName(HotspotNames, _hotspots[i].name_resource);

				bool foundMatch = false;

				if (!hotspotName.empty())
					for (uint16 j = 0; j < _zipModeData.size(); j++)
						if (_zipModeData[j].name == hotspotName) {
							foundMatch = true;
							break;
						}

				_hotspots[i].enabled = foundMatch;
			} else // Disable the hotspot if zip mode is disabled
				_hotspots[i].enabled = false;
		}
	}
}

void MohawkEngine_Riven::checkHotspotChange() {
	uint16 hotspotIndex = 0;
	bool foundHotspot = false;
	for (uint16 i = 0; i < _hotspotCount; i++)
		if (_hotspots[i].enabled && _hotspots[i].rect.contains(_mousePos)) {
			foundHotspot = true;
			hotspotIndex = i;
		}
		
	if (foundHotspot) {
		if (_curHotspot != hotspotIndex) {
			_curHotspot = hotspotIndex;
			_gfx->changeCursor(_hotspots[_curHotspot].mouse_cursor);
		}
	} else {
		_curHotspot = -1;
		_gfx->changeCursor(kRivenMainCursor);
	}
}

Common::String MohawkEngine_Riven::getHotspotName(uint16 hotspot) {
	assert(hotspot < _hotspotCount);
	
	if (_hotspots[hotspot].name_resource < 0)
		return Common::String::emptyString;

	return getName(HotspotNames, _hotspots[hotspot].name_resource);
}

void MohawkEngine_Riven::checkInventoryClick() {
	// Don't even bother. We're not in the inventory portion of the screen.
	if (_mousePos.y < 392)
		return;
		
	// No inventory in the demo or opening screens.
	if (getFeatures() & GF_DEMO || _curStack == aspit)
		return;
		
	// Set the return stack/card id's.
	*matchVarToString("returnstackid") = _curStack;
	*matchVarToString("returncardid") = _curCard;
	
	// See RivenGraphics::showInventory() for an explanation
	// of why only this variable is used.
	bool hasCathBook = *matchVarToString("acathbook") != 0;
	
	// Go to the book if a hotspot contains the mouse
	if (!hasCathBook) {
		if (atrusJournalRectSolo.contains(_mousePos)) {
			_gfx->hideInventory();
			changeToStack(aspit);
			changeToCard(5);
		}
	} else {
		if (atrusJournalRect.contains(_mousePos)) {
			_gfx->hideInventory();
			changeToStack(aspit);
			changeToCard(5);
		} else if (cathJournalRect.contains(_mousePos)) {
			_gfx->hideInventory();
			changeToStack(aspit);
			changeToCard(6);
		} else if (trapBookRect.contains(_mousePos)) {
			_gfx->hideInventory();
			changeToStack(aspit);
			changeToCard(7);
		}
	}
}

Common::SeekableReadStream *MohawkEngine_Riven::getExtrasResource(uint32 tag, uint16 id) {
	return _extrasFile->getRawData(tag, id);
}

Common::String MohawkEngine_Riven::getName(uint16 nameResource, uint16 nameID) {
	Common::SeekableReadStream* nameStream = getRawData(ID_NAME, nameResource);
	uint16 fieldCount = nameStream->readUint16BE();
	uint16* stringOffsets = new uint16[fieldCount];
	Common::String name = Common::String::emptyString;
	char c;

	if (nameID < fieldCount) {
		for (uint16 i = 0; i < fieldCount; i++)
			stringOffsets[i] = nameStream->readUint16BE();
		for (uint16 i = 0; i < fieldCount; i++)
			nameStream->readUint16BE();	// Skip unknown values
	
		nameStream->seek(stringOffsets[nameID], SEEK_CUR);
		c = (char)nameStream->readByte();

		while (c) {
			name += c;
			c = (char)nameStream->readByte();
		}
	}

	delete nameStream;
	delete [] stringOffsets;
	return name;
}

uint16 MohawkEngine_Riven::matchRMAPToCard(uint32 rmapCode) {
	uint16 index = 0;
	Common::SeekableReadStream *rmapStream = getRawData(ID_RMAP, 1);

	for (uint16 i = 1; rmapStream->pos() < rmapStream->size(); i++) {
		uint32 code = rmapStream->readUint32BE();
		if (code == rmapCode)
			index = i;
	}

	delete rmapStream;

	if (!index)
		error ("Could not match RMAP code %08x", rmapCode);

	return index - 1;
}

void MohawkEngine_Riven::runCardScript(uint16 scriptType) {
	assert(_cardData.hasData);
	for (uint16 i = 0; i < _cardData.scripts.size(); i++)
		if (_cardData.scripts[i]->getScriptType() == scriptType) {
			_cardData.scripts[i]->runScript();
			break;
		}
}

void MohawkEngine_Riven::runHotspotScript(uint16 hotspot, uint16 scriptType) {
	assert(hotspot < _hotspotCount);
	for (uint16 i = 0; i < _hotspots[hotspot].scripts.size(); i++)
		if (_hotspots[hotspot].scripts[i]->getScriptType() == scriptType) {
			_hotspots[hotspot].scripts[i]->runScript();
			break;
		}
}

void MohawkEngine_Riven::runLoadDialog() {
	runDialog(*_loadDialog);
}

Common::Error MohawkEngine_Riven::loadGameState(int slot) {
	return _saveLoad->loadGame(_saveLoad->generateSaveGameList()[slot]) ? Common::kNoError : Common::kUnknownError;
}

Common::Error MohawkEngine_Riven::saveGameState(int slot, const char *desc) {
	Common::StringList saveList = _saveLoad->generateSaveGameList();

	if ((uint)slot < saveList.size()) 
		_saveLoad->deleteSave(saveList[slot]);
	
	return _saveLoad->saveGame(Common::String(desc)) ? Common::kNoError : Common::kUnknownError;
}

static const char *rivenStackNames[] = {
	"aspit",
	"bspit",
	"gspit",
	"jspit",
	"ospit",
	"pspit",
	"rspit",
	"tspit"
};

Common::String MohawkEngine_Riven::getStackName(uint16 stack) {
	return Common::String(rivenStackNames[stack]);
}

bool ZipMode::operator== (const ZipMode &z) const {
	return z.name == name && z.id == id;
}

}
