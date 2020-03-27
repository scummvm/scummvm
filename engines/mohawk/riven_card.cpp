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

#include "mohawk/riven_card.h"

#include "mohawk/cursors.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_stacks/aspit.h"
#include "mohawk/riven_video.h"

#include "mohawk/resource.h"
#include "mohawk/riven.h"

#include "common/memstream.h"

namespace Mohawk {

RivenCard::RivenCard(MohawkEngine_Riven *vm, uint16 id) :
	_vm(vm),
	_id(id),
	_hoveredHotspot(nullptr),
	_pressedHotspot(nullptr) {
	loadCardResource(id);
	loadHotspots(id);
	loadCardPictureList(id);
	loadCardSoundList(id);
	loadCardMovieList(id);
	loadCardHotspotEnableList(id);
	loadCardWaterEffectList(id);
	applyPatches(id);
}

RivenCard::~RivenCard() {
	for (uint i = 0; i < _hotspots.size(); i++) {
		delete _hotspots[i];
	}

	_vm->_gfx->clearWaterEffect();
	_vm->_gfx->clearFliesEffect();
	_vm->_video->closeVideos();
}

void RivenCard::loadCardResource(uint16 id) {
	Common::SeekableReadStream *inStream = _vm->getResource(ID_CARD, id);

	_name = inStream->readSint16BE();
	_zipModePlace = inStream->readUint16BE();
	_scripts = _vm->_scriptMan->readScripts(inStream);

	delete inStream;
}

void RivenCard::applyPatches(uint16 id) {
	uint32 globalId = _vm->getStack()->getCardGlobalId(id);

	if (globalId == 0x2A3BC) {
		applyPropertiesPatch8EB7(globalId, "jladder", 3);
	} else if (globalId == 0x8EB7) {
		applyPropertiesPatch8EB7(globalId, "jgate", 3);
	}
	applyPropertiesPatch2E76(globalId);

	// Apply script patches
	for (uint i = 0; i < _scripts.size(); i++) {
		_scripts[i].script->applyCardPatches(_vm, globalId, _scripts[i].type, 0xFFFF);
	}

	applyPropertiesPatch22118(globalId);
	applyPropertiesPatchE2E(globalId);
	applyPropertiesPatch1518D(globalId);
	applyPropertiesPatch2B414(globalId);
}

void RivenCard::applyPropertiesPatch8EB7(uint32 globalId, const Common::String &var, uint16 hotspotId) {
	// On Jungle Island on the back side of the "beetle" gate, the forward hotspot
	// is always enabled, preventing keyboard navigation from automatically opening
	// the gate.
	// We patch the card so that the forward opcode is enabled only when the gate is open.
	//
	// New hotspot enable entries:
	// == Hotspot enable 5 ==
	// hotspotId: 3
	// enabled: 1
	//
	// == Hotspot enable 6 ==
	// hotspotId: 3
	// enabled: 0
	//
	// Additional load script fragment:
	// switch (jgate) {
	// case 0:
	//     activateBLST(6);
	//     break;
	// case 1:
	//     activateBLST(5);
	//     break;

	HotspotEnableRecord forwardEnabled;
	forwardEnabled.index = _hotspotEnableList.back().index + 1;
	forwardEnabled.hotspotId = hotspotId;
	forwardEnabled.enabled = 1;
	_hotspotEnableList.push_back(forwardEnabled);

	HotspotEnableRecord forwardDisabled;
	forwardDisabled.index = _hotspotEnableList.back().index + 1;
	forwardDisabled.hotspotId = hotspotId;
	forwardDisabled.enabled = 0;
	_hotspotEnableList.push_back(forwardDisabled);

	uint16 jGateVariable = _vm->getStack()->getIdFromName(kVariableNames, var);
	uint16 patchData[] = {
			1, // Command count in script
			kRivenCommandSwitch,
			2, // Unused
			jGateVariable,
			2, // Branches count

			0, // jgate == 0 branch (gate closed)
			1, // Command count in sub-script
			kRivenCommandActivateBLST,
			1, // Argument count
			forwardDisabled.index,

			1, // jgate == 1 branch (gate open)
			1, // Command count in sub-script
			kRivenCommandActivateBLST,
			1, // Argument count
			forwardEnabled.index
	};

	RivenScriptPtr patchScript = _vm->_scriptMan->readScriptFromData(patchData, ARRAYSIZE(patchData));

	// Append the patch to the existing script
	RivenScriptPtr loadScript = getScript(kCardLoadScript);
	loadScript += patchScript;

	debugC(kRivenDebugPatches, "Applied fix always enabled forward hotspot in card %x", globalId);
}

void RivenCard::applyPropertiesPatch2E76(uint32 globalId) {
	// In Gehn's office, after having encountered him once before and coming back
	// with the trap book, the draw update script of card 1 tries to switch to
	// card 2 while still loading card 1. Switching cards is not allowed during
	// draw update scripts, resulting in an use after free crash.
	//
	// Here we backport the fix that has been made in the DVD version to the CD version.
	//
	// Script before patch:
	// == Script 1 ==
	// type: CardUpdate
	// switch (agehn) {
	// case 1:
	//   switch (atrapbook) {
	//     case 1:
	//       obutton = 1;
	//       transition(16);
	//       switchCard(2);
	//       break;
	//   }
	// break;
	// case 2:
	//   activatePLST(5);
	//   break;
	// case 3:
	//   activatePLST(5);
	//   break;
	// }
	//
	//
	// Script after patch:
	// == Script 1 ==
	// type: CardUpdate
	// switch (agehn) {
	// case 1:
	//   switch (atrapbook) {
	//     case 1:
	//       obutton = 1;
	//       activatePLST(6);
	//       break;
	//   }
	//   break;
	// case 2:
	//   activatePLST(5);
	//   break;
	// case 3:
	//   activatePLST(5);
	//   break;
	// }
	//
	// == Script 2 ==
	// type: CardEnter
	// switch (agehn) {
	// case 1:
	//   switch (atrapbook) {
	//     case 1:
	//       transition(16);
	//       switchCard(2);
	//       break;
	//     }
	//   break;
	// }
	if (globalId == 0x2E76 && !_vm->isGameVariant(GF_DVD)) {
		uint16 aGehnVariable = _vm->getStack()->getIdFromName(kVariableNames, "agehn");
		uint16 aTrapBookVariable = _vm->getStack()->getIdFromName(kVariableNames, "atrapbook");
		uint16 patchData[] = {
				1, // Command count in script
				kRivenCommandSwitch,
				2, // Unused
				aGehnVariable,
				1, // Branches count

				1, // agehn == 1 branch
				1, // Command count in sub-script
				kRivenCommandSwitch,
				2, // Unused
				aTrapBookVariable,
				1, // Branches count

				1, // atrapbook == 1 branch
				2, // Command count in sub-script
				kRivenCommandTransition,
				1, // Argument count
				kRivenTransitionBlend,
				kRivenCommandChangeCard,
				1, // Argument count
				2  // Card id
		};

		// Add the new script to the list
		RivenTypedScript patchScript;
		patchScript.type = kCardEnterScript;
		patchScript.script = _vm->_scriptMan->readScriptFromData(patchData, ARRAYSIZE(patchData));
		_scripts.push_back(patchScript);

		// Add a black picture to the card's list to be able to use it in the second part of the patch
		Picture blackPicture;
		blackPicture.index = 6;
		blackPicture.id = 117;
		blackPicture.rect = Common::Rect(608, 392);
		_pictureList.push_back(blackPicture);

		debugC(kRivenDebugPatches, "Applied invalid card change during screen update (1/2) to card %x", globalId);
		// The second part of this patch is in the script patches
	}
}

void RivenCard::applyPropertiesPatch22118(uint32 globalId) {
	// On Temple Island, near the steam valve closest to the bridge to Boiler island,
	// the background sound on the view offering a view to the bridge does
	// not properly reflect the valve's position.
	//
	// The sound is always that of steam going through the pipe when the bridge is
	// down. When the valve points up, the sound should be that of steam escaping
	// through the top of the pipe.
	//
	// Script before patch:
	// == Script 0 ==
	// type: CardLoad
	// switch (bbigbridge) {
	//   case 0:
	//     switch (tbookvalve) {
	//       case 0:
	//         activatePLST(2);
	//         activateSLST(1);
	//         break;
	//     }
	//     break;
	// }
	// switch (bbigbridge) {
	//   case 0:
	//     switch (tbookvalve) {
	//       case 1:
	//         activatePLST(2);
	//         activateSLST(2);
	//         break;
	//     }
	//     break;
	// }
	// switch (bbigbridge) {
	//   case 1:
	//     switch (tbookvalve) {
	//       case 0:
	//         activatePLST(1);
	//         activateSLST(2);
	//         break;
	//     }
	//     break;
	// }
	// switch (bbigbridge) {
	//   case 1:
	//     switch (tbookvalve) {
	//       case 1:
	//         activatePLST(1);
	//         activateSLST(2);
	//         break;
	//     }
	//     break;
	// }
	//
	//
	// Script after patch:
	// == Script 0 ==
	// type: CardLoad
	// switch (bbigbridge) {
	//   case 0:
	//     switch (tbookvalve) {
	//       case 0:
	//         activatePLST(2);
	//         break;
	//     }
	//     break;
	// }
	// switch (bbigbridge) {
	//   case 0:
	//     switch (tbookvalve) {
	//       case 1:
	//         activatePLST(2);
	//         break;
	//     }
	//     break;
	// }
	// switch (bbigbridge) {
	//   case 1:
	//     switch (tbookvalve) {
	//       case 0:
	//         activatePLST(1);
	//         break;
	//     }
	//     break;
	// }
	// switch (bbigbridge) {
	//   case 1:
	//     switch (tbookvalve) {
	//       case 1:
	//         activatePLST(1);
	//         break;
	//     }
	//     break;
	// }
	// switch (tbookvalve) {
	//   case 0:
	//     activateSLST(1);
	//     break;
	//   case 1:
	//     activateSLST(2);
	//     break;
	// }
	if (globalId == 0x22118) {
		uint16 tBookValveVariable = _vm->getStack()->getIdFromName(kVariableNames, "tbookvalve");
		uint16 patchData[] = {
				1, // Command count in script
				kRivenCommandSwitch,
				2, // Unused
				tBookValveVariable,
				2, // Branches count

				0, // tbookvalve == 0 branch (steam escaping at the top of the pipe)
				1, // Command count in sub-script
				kRivenCommandActivateSLST,
				1, // Argument count
				1, // Steam leaking sound id

				1, // tbookvalve == 1 branch (steam going to the left pipe)
				1, // Command count in sub-script
				kRivenCommandActivateSLST,
				1, // Argument count
				2, // Steam in pipe sound id
		};

		RivenScriptPtr patchScript = _vm->_scriptMan->readScriptFromData(patchData, ARRAYSIZE(patchData));

		// Append the patch to the existing script
		RivenScriptPtr loadScript = getScript(kCardLoadScript);
		loadScript += patchScript;

		debugC(kRivenDebugPatches, "Applied incorrect steam sounds (2/2) to card %x", globalId);
	}
}

void RivenCard::applyPropertiesPatchE2E(uint32 globalId) {
	if (!_vm->isGameVariant(GF_25TH))
		return;

	// The main menu in the Myst 25th anniversary version is patched to include new items:
	//   - Save game
	if (globalId == 0xE2E) {
		moveHotspot(   22, Common::Rect(470, 175, 602, 190)); // Setup
		moveHotspot(   16, Common::Rect(470, 201, 602, 216)); // New game
		addMenuHotspot(23, Common::Rect(470, 227, 602, 242), 3, RivenStacks::ASpit::kExternalRestoreGame, "xarestoregame");
		addMenuHotspot(24, Common::Rect(470, 256, 602, 271), 4, RivenStacks::ASpit::kExternalSaveGame,    "xaSaveGame");
		addMenuHotspot(25, Common::Rect(470, 283, 602, 300), 5, RivenStacks::ASpit::kExternalResume,      "xaResumeGame");
		addMenuHotspot(26, Common::Rect(470, 309, 602, 326), 6, RivenStacks::ASpit::kExternalOptions,     "xaOptions");
		addMenuHotspot(27, Common::Rect(470, 335, 602, 352), 7, RivenStacks::ASpit::kExternalQuit,        "xademoquit");
		_vm->getStack()->registerName(kExternalCommandNames,    RivenStacks::ASpit::kExternalNewGame,     "xaNewGame");
	}
}

void RivenCard::applyPropertiesPatch1518D(uint32 globalId) {
	// Inside Jungle Island's dome, when looking at the open book,
	// stepping back from the stand and then looking at the book
	// again, the book closing animation would play again.
	//
	// Comparing the scripts for the Jungle dome and the other domes
	// shows a small portion of script is missing.
	// The following patch adds it back so the jungle dome script
	// matches the other domes.
	//
	// Added script part:
	//   == Script 2 ==
	//   [...]
	//   type: CardEnter
	//   switch (jbook) {
	//   case 2:
	//     playMovieBlocking(1);
	//     jbook = 0;
	//     refreshCard();
	//     break;
	//   }
	if (globalId == 0x1518D) {
		uint16 jBookVariable = _vm->getStack()->getIdFromName(kVariableNames, "jbook");
		uint16 patchData[] = {
		        1, // Command count in script
		        kRivenCommandSwitch,
		        2, // Unused
		        jBookVariable,
		        1, // Branches count

		        2, // jbook == 2 branch
		        3, // Command count in sub-script

		        kRivenCommandPlayMovieBlocking,
		        1, // Argument count
		        1, // Video id

		        kRivenCommandSetVariable,
		        2, // Argument count
		        jBookVariable,
		        0, // Variable value

		        kRivenCommandRefreshCard,
		        0 // Argument count
		};

		RivenScriptPtr patchScript = _vm->_scriptMan->readScriptFromData(patchData, ARRAYSIZE(patchData));

		// Append the patch to the existing script
		RivenScriptPtr loadScript = getScript(kCardEnterScript);
		loadScript += patchScript;

		debugC(kRivenDebugPatches, "Applied jungle book close loop to card %x", globalId);
	}
}

void RivenCard::applyPropertiesPatch2B414(uint32 globalId) {
	//
	// When seated in the Jungle Island's gallows control throne,
	// the right lever opens or closes the gallows' floor.
	// The issue is that it is possible to click on the position
	// where the lever would be when the gallow's floor in the
	// opposite state as it currently is. And thus to trigger the
	// corresponding sequence. That is to say for example closing
	// the floor when it is already closed.
	//
	// We simply add the missing script instructions to make
	// the open and close hotspots mutually exclusive.
	//
	// Added script part:
	//   == Script 0 ==
	//     type: CardLoad
	//     [...]
	//   switch (jgallows) {
	//     case 0:
	//       activateBLST(1);
	//       activateBLST(4);
	//       break;
	//     case 1:
	//       activateBLST(2);
	//       activateBLST(3);
	//       break;
	//   }
	if (globalId == 0x2B414) {
		HotspotEnableRecord openGallowsEnabled;
		openGallowsEnabled.index = 1;
		openGallowsEnabled.hotspotId = 8;
		openGallowsEnabled.enabled = 1;
		_hotspotEnableList.push_back(openGallowsEnabled);

		HotspotEnableRecord openGallowsDisabled;
		openGallowsDisabled.index = 2;
		openGallowsDisabled.hotspotId = 8;
		openGallowsDisabled.enabled = 0;
		_hotspotEnableList.push_back(openGallowsDisabled);

		HotspotEnableRecord closeGallowsEnabled;
		closeGallowsEnabled.index = 3;
		closeGallowsEnabled.hotspotId = 9;
		closeGallowsEnabled.enabled = 1;
		_hotspotEnableList.push_back(closeGallowsEnabled);

		HotspotEnableRecord closeGallowsDisabled;
		closeGallowsDisabled.index = 4;
		closeGallowsDisabled.hotspotId = 9;
		closeGallowsDisabled.enabled = 0;
		_hotspotEnableList.push_back(closeGallowsDisabled);

		uint16 jGallowsVariable = _vm->getStack()->getIdFromName(kVariableNames, "jgallows");
		uint16 patchData[] = {
		        1, // Command count in script
		        kRivenCommandSwitch,
		        2, // Unused
		        jGallowsVariable,
		        2, // Branches count

		        0, // jgallows == 0 branch
		        2, // Command count in sub-script

		        kRivenCommandActivateBLST,
		        1, // Argument count
		        openGallowsEnabled.index,

		        kRivenCommandActivateBLST,
		        1, // Argument count
		        closeGallowsDisabled.index,

		        1, // jgallows == 1 branch
		        2, // Command count in sub-script

		        kRivenCommandActivateBLST,
		        1, // Argument count
		        openGallowsDisabled.index,

		        kRivenCommandActivateBLST,
		        1, // Argument count
		        closeGallowsEnabled.index,

		};

		RivenScriptPtr patchScript = _vm->_scriptMan->readScriptFromData(patchData, ARRAYSIZE(patchData));

		// Append the patch to the existing script
		RivenScriptPtr loadScript = getScript(kCardLoadScript);
		loadScript += patchScript;

		debugC(kRivenDebugPatches, "Applied missing jgallows hotspot enable / disable to card %x", globalId);
	}
}

void RivenCard::moveHotspot(uint16 blstId, const Common::Rect &position) {
	RivenHotspot *hotspot = getHotspotByBlstId(blstId);
	if (!hotspot) {
		warning("Could not find hotspot with blstId %d", blstId);
		return;
	}

	hotspot->setRect(position);
}

void RivenCard::addMenuHotspot(uint16 blstId, const Common::Rect &position, uint16 index,
                               uint16 externalCommandNameId, const char *externalCommandName) {
	RivenHotspot *existingHotspot = getHotspotByBlstId(blstId);
	if (existingHotspot) {
		moveHotspot(blstId, position);
		return; // Don't add the hotspot if it already exists
	}

	// Add the external command id => name mapping if it is missing
	int16 existingCommandNameId = _vm->getStack()->getIdFromName(kExternalCommandNames, externalCommandName);
	if (existingCommandNameId < 0) {
		_vm->getStack()->registerName(kExternalCommandNames, externalCommandNameId, externalCommandName);
	} else {
		externalCommandNameId = existingCommandNameId;
	}

	uint16 patchData[] = {
			blstId,
			0xFFFF,           // name
			(uint16) position.left,
			(uint16) position.top,
			(uint16) position.right,
			(uint16) position.bottom,
			0,                // u0
			kRivenMainCursor, // cursor
			index,
			0xFFFF,           // transition offset
			0,                // flags
			2,                // script count

			kMouseDownScript,          // script type
			1,                         // command count
			kRivenCommandRunExternal,  // command type
			2,                         // argument count
			externalCommandNameId,
			0,                         // external argument count

			kMouseInsideScript,        // script type
			1,                         // command count
			kRivenCommandChangeCursor, // command type
			1,                         // argument count
			kRivenOpenHandCursor       // cursor
		};

	// Script data is expected to be in big endian
	for (uint i = 0; i < ARRAYSIZE(patchData); i++) {
			patchData[i] = TO_BE_16(patchData[i]);
		}

	// Add the new hotspot to the existing ones
	Common::MemoryReadStream patchStream((const byte *)(patchData), ARRAYSIZE(patchData) * sizeof(uint16));
	RivenHotspot *newHotspot = new RivenHotspot(_vm, &patchStream);
	_hotspots.push_back(newHotspot);
}

void RivenCard::enter(bool unkMovies) {
	setCurrentCardVariable();

	_vm->_activatedPLST = false;
	_vm->_activatedSLST = false;

	_vm->_gfx->beginScreenUpdate();
	runScript(kCardLoadScript);
	defaultLoadScript();

	initializeZipMode();
	_vm->_gfx->applyScreenUpdate(true);

	if (_vm->_showHotspots) {
		drawHotspotRects();
	}

	runScript(kCardEnterScript);
}

void RivenCard::initializeZipMode() {
	if (_zipModePlace) {
		_vm->addZipVisitedCard(_id, _name);
	}

	// Check if a zip mode hotspot is enabled by checking the name/id against the ZIPS records.
	for (uint32 i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->isZip()) {
			if (_vm->_vars["azip"] != 0) {
				// Check if a zip mode hotspot is enabled by checking the name/id against the ZIPS records.
				Common::String hotspotName = _hotspots[i]->getName();
				bool visited = _vm->isZipVisitedCard(hotspotName);

				_hotspots[i]->enable(visited);
			} else // Disable the hotspot if zip mode is disabled
				_hotspots[i]->enable(false);
		}
	}
}

RivenScriptPtr RivenCard::getScript(uint16 scriptType) const {
	for (uint16 i = 0; i < _scripts.size(); i++)
		if (_scripts[i].type == scriptType) {
			return _scripts[i].script;
		}

	return RivenScriptPtr();
}

void RivenCard::runScript(uint16 scriptType) {
	RivenScriptPtr script = getScript(scriptType);
	_vm->_scriptMan->runScript(script, false);
}

uint16 RivenCard::getId() const {
	return _id;
}

void RivenCard::defaultLoadScript() {
	// Activate the first picture list if none have been activated
	if (!_vm->_activatedPLST)
		drawPicture(1);

	// Activate the first sound list if none have been activated
	if (!_vm->_activatedSLST)
		playSound(1);
}

void RivenCard::loadCardPictureList(uint16 id) {
	Common::SeekableReadStream* plst = _vm->getResource(ID_PLST, id);
	uint16 recordCount = plst->readUint16BE();
	_pictureList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		Picture &picture = _pictureList[i];
		picture.index = plst->readUint16BE();
		picture.id = plst->readUint16BE();
		picture.rect.left = plst->readUint16BE();
		picture.rect.top = plst->readUint16BE();
		picture.rect.right = plst->readUint16BE();
		picture.rect.bottom = plst->readUint16BE();
	}

	delete plst;
}

void RivenCard::drawPicture(uint16 index, bool queue) {
	if (index > 0 && index <= _pictureList.size()) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, kRivenCommandActivatePLST, 1, index);
		_vm->_scriptMan->runScript(script, queue);
	}
}

RivenCard::Picture RivenCard::getPicture(uint16 index) const {
	for (uint16 i = 0; i < _pictureList.size(); i++) {
		if (_pictureList[i].index == index) {
			return _pictureList[i];
		}
	}

	error("Could not find picture %d in card %d", index, _id);
}

void RivenCard::loadCardSoundList(uint16 id) {
	Common::SeekableReadStream *slstStream = _vm->getResource(ID_SLST, id);

	uint16 recordCount = slstStream->readUint16BE();
	_soundList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		SLSTRecord &slstRecord = _soundList[i];
		slstRecord.index = slstStream->readUint16BE();

		uint16 soundCount = slstStream->readUint16BE();

		slstRecord.soundIds.resize(soundCount);
		for (uint16 j = 0; j < soundCount; j++)
			slstRecord.soundIds[j] = slstStream->readUint16BE();

		slstRecord.fadeFlags = slstStream->readUint16BE();
		slstRecord.loop = slstStream->readUint16BE();
		slstRecord.globalVolume = slstStream->readUint16BE();
		slstRecord.u0 = slstStream->readUint16BE();			// Unknown

		if (slstRecord.u0 > 1)
			warning("slstRecord.u0: %d non-boolean", slstRecord.u0);

		slstRecord.suspend = slstStream->readUint16BE();

		if (slstRecord.suspend != 0)
			warning("slstRecord.suspend: %d non-zero", slstRecord.suspend);

		slstRecord.volumes.resize(soundCount);
		slstRecord.balances.resize(soundCount);
		slstRecord.u2.resize(soundCount);

		for (uint16 j = 0; j < soundCount; j++)
			slstRecord.volumes[j] = slstStream->readUint16BE();

		for (uint16 j = 0; j < soundCount; j++)
			slstRecord.balances[j] = slstStream->readSint16BE();	// negative = left, 0 = center, positive = right

		for (uint16 j = 0; j < soundCount; j++) {
			slstRecord.u2[j] = slstStream->readUint16BE();		// Unknown

			if (slstRecord.u2[j] != 255 && slstRecord.u2[j] != 256)
				warning("slstRecord.u2[%d]: %d not 255 or 256", j, slstRecord.u2[j]);
		}
	}

	delete slstStream;
}

void RivenCard::playSound(uint16 index, bool queue) {
	if (index > 0 && index <= _soundList.size()) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, kRivenCommandActivateSLST, 1, index);
		_vm->_scriptMan->runScript(script, queue);
	}
}

SLSTRecord RivenCard::getSound(uint16 index) const {
	for (uint16 i = 0; i < _soundList.size(); i++) {
		if (_soundList[i].index == index) {
			return _soundList[i];
		}
	}

	error("Could not find sound %d in card %d", index, _id);
}

void RivenCard::overrideSound(uint16 index, uint16 withIndex) {
	_soundList[index].soundIds = _soundList[withIndex].soundIds;
}

void RivenCard::loadHotspots(uint16 id) {
	Common::SeekableReadStream *inStream = _vm->getResource(ID_HSPT, id);

	uint16 hotspotCount = inStream->readUint16BE();
	_hotspots.resize(hotspotCount);

	uint32 globalId = _vm->getStack()->getCardGlobalId(id);
	for (uint16 i = 0; i < hotspotCount; i++) {
		_hotspots[i] = new RivenHotspot(_vm, inStream);
		_hotspots[i]->applyPropertiesPatches(globalId);
		_hotspots[i]->applyScriptPatches(globalId);
	}

	delete inStream;
}

void RivenCard::drawHotspotRects() {
	for (uint16 i = 0; i < _hotspots.size(); i++)
		_vm->_gfx->drawRect(_hotspots[i]->getRect(), _hotspots[i]->isEnabled());
}

RivenHotspot *RivenCard::getHotspotContainingPoint(const Common::Point &point) const {
	RivenHotspot *hotspot = nullptr;
	for (uint16 i = 0; i < _hotspots.size(); i++)
		if (_hotspots[i]->isEnabled() && _hotspots[i]->containsPoint(point)) {
			hotspot = _hotspots[i];
		}

	return hotspot;
}

Common::Array<RivenHotspot *> RivenCard::getHotspots() const {
	return _hotspots;
}

RivenHotspot *RivenCard::getHotspotByName(const Common::String &name, bool optional) const {
	int16 nameId = _vm->getStack()->getIdFromName(kHotspotNames, name);

	for (uint i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->getNameId() == nameId && nameId != -1) {
			return _hotspots[i];
		}
	}

	if (optional) {
		return nullptr;
	} else {
		error("Card %d does not have an hotspot named %s", _id, name.c_str());
	}
}

RivenHotspot *RivenCard::getHotspotByBlstId(const uint16 blstId) const {
	for (uint i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->getBlstId() == blstId) {
			return _hotspots[i];
		}
	}

	return nullptr;
}

void RivenCard::loadCardHotspotEnableList(uint16 id) {
	Common::SeekableReadStream *blst = _vm->getResource(ID_BLST, id);

	uint16 recordCount = blst->readUint16BE();
	_hotspotEnableList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		HotspotEnableRecord &record = _hotspotEnableList[i];
		record.index = blst->readUint16BE();
		record.enabled = blst->readUint16BE();
		record.hotspotId = blst->readUint16BE();
	}

	delete blst;
}

void RivenCard::activateHotspotEnableRecord(uint16 index) {
	for (uint16 i = 0; i < _hotspotEnableList.size(); i++) {
		const HotspotEnableRecord &record = _hotspotEnableList[i];
		if (record.index == index) {
			RivenHotspot *hotspot = getHotspotByBlstId(record.hotspotId);
			hotspot->enable(record.enabled == 1);
			break;
		}
	}
}

void RivenCard::loadCardWaterEffectList(uint16 id) {
	Common::SeekableReadStream *flst = _vm->getResource(ID_FLST, id);

	uint16 recordCount = flst->readUint16BE();
	_waterEffectList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		WaterEffectRecord &record = _waterEffectList[i];
		record.index = flst->readUint16BE();
		record.sfxeId = flst->readUint16BE();
		record.u0 = flst->readUint16BE();

		if (record.u0 != 0) {
			warning("FLST u0 non-zero");
		}
	}

	delete flst;
}

void RivenCard::activateWaterEffect(uint16 index) {
	for (uint16 i = 0; i < _waterEffectList.size(); i++) {
		const WaterEffectRecord &record = _waterEffectList[i];
		if (record.index == index) {
			_vm->_gfx->scheduleWaterEffect(record.sfxeId);
			break;
		}
	}
}

RivenHotspot *RivenCard::getCurHotspot() const {
	return _hoveredHotspot;
}

RivenScriptPtr RivenCard::onMouseDown(const Common::Point &mouse) {
	RivenScriptPtr script = onMouseMove(mouse);
	updateMouseCursor();

	_pressedHotspot = _hoveredHotspot;

	if (_pressedHotspot) {
		script += _pressedHotspot->getScript(kMouseDownScript);
	}

	return script;
}

RivenScriptPtr RivenCard::onMouseUp(const Common::Point &mouse) {
	RivenScriptPtr script = onMouseMove(mouse);

	if (_pressedHotspot && _pressedHotspot == _hoveredHotspot) {
		script += _pressedHotspot->getScript(kMouseUpScript);
	}

	_pressedHotspot = nullptr;

	return script;
}

RivenScriptPtr RivenCard::onMouseMove(const Common::Point &mouse) {
	RivenHotspot *hotspot = getHotspotContainingPoint(mouse);

	RivenScriptPtr script = RivenScriptPtr(new RivenScript());

	// Detect hotspot exit
	if (_hoveredHotspot && (!hotspot || hotspot != _hoveredHotspot)) {
		script += _hoveredHotspot->getScript(kMouseLeaveScript);
	}

	// Detect hotspot entry
	if (hotspot && hotspot != _hoveredHotspot) {
		_hoveredHotspot = hotspot;
		script += _hoveredHotspot->getScript(kMouseEnterScript);
	}

	if (!hotspot) {
		_hoveredHotspot = nullptr;
	}

	return script;
}

RivenScriptPtr RivenCard::onMouseDragUpdate() {
	RivenScriptPtr script;
	if (_pressedHotspot) {
		script = _pressedHotspot->getScript(kMouseDragScript);
	}

	return script;
}

RivenScriptPtr RivenCard::onFrame() {
	return getScript(kCardFrameScript);
}

RivenScriptPtr RivenCard::onMouseUpdate() {
	RivenScriptPtr script(new RivenScript());

	if (_hoveredHotspot) {
		script += _hoveredHotspot->getScript(kMouseInsideScript);
	}

	if (script->empty()) {
		updateMouseCursor();
	}

	// Clear the pressed hotspot, in case we missed the mouse up event
	// because we were running a script when it fired.
	if (_pressedHotspot && _pressedHotspot == _hoveredHotspot) {
		script += _pressedHotspot->getScript(kMouseUpScript);
	}
	_pressedHotspot = nullptr;

	return script;
}

void RivenCard::updateMouseCursor() {
	uint16 cursor;
	if (_hoveredHotspot) {
		cursor = _hoveredHotspot->getMouseCursor();
	} else {
		cursor = kRivenMainCursor;
	}

	_vm->_cursor->setCursor(cursor);
}

void RivenCard::leave() {
	RivenScriptPtr script(new RivenScript());

	if (_pressedHotspot) {
		script += _pressedHotspot->getScript(kMouseUpScript);
		_pressedHotspot = nullptr;
	}

	if (_hoveredHotspot) {
		script += _hoveredHotspot->getScript(kMouseLeaveScript);
		_hoveredHotspot = nullptr;
	}

	script += getScript(kCardLeaveScript);

	_vm->_scriptMan->runScript(script, false);
}

void RivenCard::setCurrentCardVariable() {
	_vm->_vars["currentcardid"] = _id;
}

void RivenCard::dump() const {
	debug("== Card ==");
	debug("id: %d", _id);
	if (_name >= 0) {
		debug("name: %s", _vm->getStack()->getName(kCardNames, _name).c_str());
	} else {
		debug("name: [no name]");
	}
	debug("zipModePlace: %d", _zipModePlace);
	debug("globalId: %x", _vm->getStack()->getCardGlobalId(_id));
	debugN("\n");

	for (uint i = 0; i < _scripts.size(); i++) {
		debug("== Script %d ==", i);
		debug("type: %s", RivenScript::getTypeName(_scripts[i].type));
		_scripts[i].script->dumpScript(0);
		debugN("\n");
	}

	for (uint i = 0; i < _hotspots.size(); i++) {
		debug("== Hotspot %d ==", i);
		_hotspots[i]->dump();
	}

	for (uint i = 0; i < _pictureList.size(); i++) {
		const Common::Rect &rect = _pictureList[i].rect;
		debug("== Picture %d ==", _pictureList[i].index);
		debug("pictureId: %d", _pictureList[i].id);
		debug("rect: (%d, %d, %d, %d)", rect.left, rect.top, rect.right, rect.bottom);
		debugN("\n");
	}

	for (uint i = 0; i < _waterEffectList.size(); i++) {
		debug("== Effect %d ==", _waterEffectList[i].index);
		debug("sfxeId: %d", _waterEffectList[i].sfxeId);
		debug("u0: %d", _waterEffectList[i].u0);
		debugN("\n");
	}

	for (uint i = 0; i < _hotspotEnableList.size(); i++) {
		debug("== Hotspot enable %d ==", _hotspotEnableList[i].index);
		debug("hotspotId: %d", _hotspotEnableList[i].hotspotId);
		debug("enabled: %d", _hotspotEnableList[i].enabled);
		debugN("\n");
	}

	for (uint i = 0; i < _soundList.size(); i++) {
		debug("== Ambient sound list %d ==", _soundList[i].index);
		debug("globalVolume: %d", _soundList[i].globalVolume);
		debug("fadeFlags: %d", _soundList[i].fadeFlags);
		debug("loop: %d", _soundList[i].loop);
		debug("suspend: %d", _soundList[i].suspend);
		debug("u0: %d", _soundList[i].u0);
		for (uint j = 0; j < _soundList[i].soundIds.size(); j++) {
			debug("sound[%d].id: %d", j, _soundList[i].soundIds[j]);
			debug("sound[%d].volume: %d", j, _soundList[i].volumes[j]);
			debug("sound[%d].balance: %d", j, _soundList[i].balances[j]);
			debug("sound[%d].u2: %d", j, _soundList[i].u2[j]);
		}
		debugN("\n");
	}

	for (uint i = 0; i < _movieList.size(); i++) {
		debug("== Movie %d ==", _movieList[i].index);
		debug("movieID: %d", _movieList[i].movieID);
		debug("playbackSlot: %d", _movieList[i].playbackSlot);
		debug("left: %d", _movieList[i].left);
		debug("top: %d", _movieList[i].top);
		debug("lowBoundTime: %d", _movieList[i].lowBoundTime);
		debug("startTime: %d", _movieList[i].startTime);
		debug("highBoundTime: %d", _movieList[i].highBoundTime);
		debug("loop: %d", _movieList[i].loop);
		debug("volume: %d", _movieList[i].volume);
		debug("rate: %d", _movieList[i].rate);
		debugN("\n");
	}
}

void RivenCard::loadCardMovieList(uint16 id) {
	Common::SeekableReadStream *mlstStream = _vm->getResource(ID_MLST, id);

	uint16 recordCount = mlstStream->readUint16BE();
	_movieList.resize(recordCount);

	for (uint16 i = 0; i < recordCount; i++) {
		MLSTRecord &mlstRecord = _movieList[i];
		mlstRecord.index = mlstStream->readUint16BE();
		mlstRecord.movieID = mlstStream->readUint16BE();
		mlstRecord.playbackSlot = mlstStream->readUint16BE();
		mlstRecord.left = mlstStream->readUint16BE();
		mlstRecord.top = mlstStream->readUint16BE();
		mlstRecord.lowBoundTime = mlstStream->readUint16BE();
		mlstRecord.startTime = mlstStream->readUint16BE();
		mlstRecord.highBoundTime = mlstStream->readUint16BE();
		mlstRecord.loop = mlstStream->readUint16BE();
		mlstRecord.volume = mlstStream->readUint16BE();
		mlstRecord.rate = mlstStream->readUint16BE();

		if (mlstRecord.lowBoundTime != 0)
			warning("lowBoundTime in MLST not 0");

		if (mlstRecord.startTime != 0)
			warning("startTime in MLST not 0");

		if (mlstRecord.highBoundTime != 0xFFFF)
			warning("highBoundTime in MLST not 0xFFFF");

		if (mlstRecord.rate != 1)
			warning("mlstRecord.rate not 1");
	}

	delete mlstStream;
}

MLSTRecord RivenCard::getMovie(uint16 index) const {
	for (uint16 i = 0; i < _movieList.size(); i++) {
		if (_movieList[i].index == index) {
			return _movieList[i];
		}
	}

	error("Could not find movie %d in card %d", index, _id);
}

void RivenCard::playMovie(uint16 index, bool queue) {
	if (index > 0 && index <= _movieList.size()) {
		RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(1, kRivenCommandActivateMLSTAndPlay, 1, index);
		_vm->_scriptMan->runScript(script, queue);
	}
}

RivenScriptPtr RivenCard::onKeyAction(RivenAction keyAction) {
	if (_pressedHotspot) {
		return RivenScriptPtr(new RivenScript());
	}

	static const char *forwardNames[] = {
			"forward", "forward1", "forward2", "forward3",
			"opendoor", "openhatch", "opentrap", "opengate", "opengrate",
			"open", "door", "drop", "go", "enterprison", "exit",
			"forwardleft", "forwardright", nullptr
	};

	static const char *forwardLeftNames [] = { "forwardleft",              nullptr };
	static const char *forwardRightNames[] = { "forwardright",             nullptr };
	static const char *leftNames        [] = { "left",  "afl", "prevpage", nullptr };
	static const char *rightNames       [] = { "right", "afr", "nextpage", nullptr };
	static const char *backNames        [] = { "back",                     nullptr };
	static const char *upNames          [] = { "up",                       nullptr };
	static const char *downNames        [] = { "down",                     nullptr };

	const char **hotspotNames = nullptr;
	switch (keyAction) {
		case kRivenActionMoveForward:
			hotspotNames = forwardNames;
			break;
		case kRivenActionMoveForwardLeft:
			hotspotNames = forwardLeftNames;
			break;
		case kRivenActionMoveForwardRight:
			hotspotNames = forwardRightNames;
			break;
		case kRivenActionMoveLeft:
			hotspotNames = leftNames;
			break;
		case kRivenActionMoveRight:
			hotspotNames = rightNames;
			break;
		case kRivenActionMoveBack:
			hotspotNames = backNames;
			break;
		case kRivenActionLookUp:
			hotspotNames = upNames;
			break;
		case kRivenActionLookDown:
			hotspotNames = downNames;
			break;
		default:
			break;
	}

	if (!hotspotNames) {
		return RivenScriptPtr(new RivenScript());
	}

	RivenHotspot *directionHotspot = findEnabledHotspotByName(hotspotNames);
	if (!directionHotspot) {
		return RivenScriptPtr(new RivenScript());
	}

	_hoveredHotspot = directionHotspot;

	RivenScriptPtr clickScript = directionHotspot->getScript(kMouseDownScript);
	if (!clickScript || clickScript->empty()) {
		clickScript = directionHotspot->getScript(kMouseUpScript);
	}
	if (!clickScript || clickScript->empty()) {
		clickScript = RivenScriptPtr(new RivenScript());
	}

	return clickScript;
}

RivenHotspot *RivenCard::findEnabledHotspotByName(const char **names) const {
	for (uint i = 0; names[i] != nullptr; i++) {
		RivenHotspot *hotspot = getHotspotByName(names[i], true);
		if (hotspot && hotspot->isEnabled()) {
			return hotspot;
		}
	}

	return nullptr;
}

RivenHotspot::RivenHotspot(MohawkEngine_Riven *vm, Common::ReadStream *stream) :
		_vm(vm) {
	loadFromStream(stream);
}

void RivenHotspot::loadFromStream(Common::ReadStream *stream) {
	_flags = kFlagEnabled;

	_blstID = stream->readUint16BE();
	_nameResource = stream->readSint16BE();

	int16 left = stream->readSint16BE();
	int16 top = stream->readSint16BE();
	int16 right = stream->readSint16BE();
	int16 bottom = stream->readSint16BE();

	// Riven has some invalid rects, disable them here
	// Known weird hotspots:
	// - tspit 371 (DVD: 377), hotspot 4
	if (left >= right || top >= bottom) {
		warning("Invalid hotspot: (%d, %d, %d, %d)", left, top, right, bottom);
		left = top = right = bottom = 0;
		enable(false);
	}

	_rect = Common::Rect(left, top, right, bottom);

	_u0 = stream->readUint16BE();
	_mouseCursor = stream->readUint16BE();
	_index = stream->readUint16BE();
	_transitionOffset = stream->readSint16BE();
	_flags |= stream->readUint16BE();

	// Read in the scripts now
	_scripts = _vm->_scriptMan->readScripts(stream);
}

RivenScriptPtr RivenHotspot::getScript(uint16 scriptType) const {
	for (uint16 i = 0; i < _scripts.size(); i++)
		if (_scripts[i].type == scriptType) {
			return _scripts[i].script;
		}

	return RivenScriptPtr();
}

void RivenHotspot::applyPropertiesPatches(uint32 cardGlobalId) {
	// In Jungle island, one of the bridge hotspots does not have a name
	// This breaks keyboard navigation. Set the proper name.
	if (cardGlobalId == 0x214a0 && _blstID == 9) {
		_nameResource = _vm->getStack()->getIdFromName(kHotspotNames, "forward");
		debugC(kRivenDebugPatches, "Applied missing hotspot name patch to card %x", cardGlobalId);
	}

	// In the lab in Book Making island the card showing one of the doors has
	// two "forward" hotspots. One of them goes backwards. Disable it, and make sure
	// it cannot be found by the keyboard navigation code.
	if (cardGlobalId == 0x1fa79 && _blstID == 3) {
		enable(false);
		_nameResource = -1;
		debugC(kRivenDebugPatches, "Applied disable buggy forward hotspot to card %x", cardGlobalId);
	}

	// On Temple Island, in front of the back door to the rotating room,
	// change the name of the hotspot to look at the bottom of the door to
	// "down" instead of "forwardleft". That way the keyboard navigation
	// does not spoil that you can go below the door.
	// Also make sure the forward keyboard action plays the try to open
	// door animation.
	if (cardGlobalId == 0x87ac && _blstID == 10) {
		_nameResource = _vm->getStack()->getIdFromName(kHotspotNames, "down");
		debugC(kRivenDebugPatches, "Applied change hotspot name to 'down' patch to card %x", cardGlobalId);
	}
	if (cardGlobalId == 0x87ac && _blstID == 12) {
		_nameResource = _vm->getStack()->getIdFromName(kHotspotNames, "opendoor");
		debugC(kRivenDebugPatches, "Applied change hotspot name to 'opendoor' patch to card %x", cardGlobalId);
	}
}

void RivenHotspot::applyScriptPatches(uint32 cardGlobalId) {
	for (uint16 i = 0; i < _scripts.size(); i++) {
		_scripts[i].script->applyCardPatches(_vm, cardGlobalId, _scripts[i].type, _blstID);
	}
}

bool RivenHotspot::isEnabled() const {
	return (_flags & kFlagEnabled) != 0;
}

void RivenHotspot::enable(bool e) {
	if (e) {
		_flags |= kFlagEnabled;
	} else {
		_flags &= ~kFlagEnabled;
	}
}

bool RivenHotspot::isZip() const {
	return (_flags & kFlagZip) != 0;
}

Common::Rect RivenHotspot::getRect() const {
	return _rect;
}

bool RivenHotspot::containsPoint(const Common::Point &point) const {
	return _rect.contains(point);
}

uint16 RivenHotspot::getMouseCursor() const {
	return _mouseCursor;
}

Common::String RivenHotspot::getName() const {
	if (_nameResource < 0)
		return Common::String();

	return _vm->getStack()->getName(kHotspotNames, _nameResource);
}

uint16 RivenHotspot::getIndex() const {
	return _index;
}

uint16 RivenHotspot::getBlstId() const {
	return _blstID;
}

void RivenHotspot::setRect(const Common::Rect &rect) {
	_rect = rect;
}

int16 RivenHotspot::getNameId() const {
	return _nameResource;
}

int16 RivenHotspot::getTransitionOffset() const {
	return _transitionOffset;
}

void RivenHotspot::dump() const {
	debug("index: %d", _index);
	debug("blstId: %d", _blstID);
	debug("name: %s", getName().c_str());
	debug("rect: (%d, %d, %d, %d)", _rect.left, _rect.top, _rect.right, _rect.bottom);
	debug("flags: %d", _flags);
	debug("mouseCursor: %d", _mouseCursor);
	debug("transitionOffset: %d", _transitionOffset);
	debug("u0: %d", _u0);
	debugN("\n");

	for (uint i = 0; i < _scripts.size(); i++) {
		debug("=== Hotspot script %d ===", i);
		debug("type: %s", RivenScript::getTypeName(_scripts[i].type));
		_scripts[i].script->dumpScript(0);
		debugN("\n");
	}
}

} // End of namespace Mohawk
