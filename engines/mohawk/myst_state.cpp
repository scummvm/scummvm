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

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_state.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/thumbnail.h"

namespace Mohawk {

MystSaveMetadata::MystSaveMetadata() {
	saveDay = 0;
	saveMonth = 0;
	saveYear = 0;
	saveHour = 0;
	saveMinute = 0;
	totalPlayTime = 0;
	autoSave = false;
}

bool MystSaveMetadata::sync(Common::Serializer &s) {
	static const Common::Serializer::Version kCurrentVersion = 2;

	if (!s.syncVersion(kCurrentVersion)) {
		return false;
	}

	s.syncAsByte(saveDay);
	s.syncAsByte(saveMonth);
	s.syncAsUint16LE(saveYear);
	s.syncAsByte(saveHour);
	s.syncAsByte(saveMinute);
	s.syncString(saveDescription);
	s.syncAsUint32LE(totalPlayTime);
	s.syncAsByte(autoSave, 2);

	return true;
}

const int MystGameState::kAutoSaveSlot = 0;

MystGameState::MystGameState(MohawkEngine_Myst *vm, Common::SaveFileManager *saveFileMan) :
		_vm(vm),
		_saveFileMan(saveFileMan) {

	reset();
}

void MystGameState::reset() {
	// Most of the variables are zero at game start.
	memset(&_globals, 0, sizeof(_globals));
	memset(&_myst, 0, sizeof(_myst));
	memset(&_channelwood, 0, sizeof(_channelwood));
	memset(&_mechanical, 0, sizeof(_mechanical));
	memset(&_selenitic, 0, sizeof(_selenitic));
	memset(&_stoneship, 0, sizeof(_stoneship));
	memset(&_mystReachableZipDests, 0, sizeof(_mystReachableZipDests));
	memset(&_channelwoodReachableZipDests, 0, sizeof(_channelwoodReachableZipDests));
	memset(&_mechReachableZipDests, 0, sizeof(_mechReachableZipDests));
	memset(&_seleniticReachableZipDests, 0, sizeof(_seleniticReachableZipDests));
	memset(&_stoneshipReachableZipDests, 0, sizeof(_stoneshipReachableZipDests));

	// Unknown
	_globals.u0 = 2;
	// Current Age / Stack - Start in Myst
	_globals.currentAge = kMystStart;
	_globals.heldPage = kNoPage;
	_globals.u1 = 1;
	_globals.ending = kDniNotVisited;

	// Library Bookcase Door - Default to Up
	_myst.libraryBookcaseDoor = 1;
	// Dock Imager Numeric Selection - Default to 67
	_myst.imagerSelection = 67;
	// Dock Imager Active - Default to Active
	_myst.imagerActive = 1;
	// Stellar Observatory Lights - Default to On
	_myst.observatoryLights = 1;
	// First day of month
	_myst.observatoryDaySetting = 1;
	// Stellar Observatory sliders
	_myst.observatoryDaySlider = 90;
	_myst.observatoryMonthSlider = 90;
	_myst.observatoryYearSlider = 90;
	_myst.observatoryTimeSlider = 90;

	// Lighthouse Trapdoor State - Default to Locked
	_stoneship.trapdoorState = 2;
	// Lighthouse Chest Water State - Default to Full
	_stoneship.chestWaterState = 1;
}

MystGameState::~MystGameState() {
}

bool MystGameState::load(int slot) {
	if (!loadState(slot)) {
		return false;
	}

	loadMetadata(slot);

	// Set Channelwood elevator state to down, because we start on the lower level
	_channelwood.elevatorState = 0;

	// Switch us back to the intro stack, to the linking book
	_vm->changeToStack(kIntroStack, 5, 0, 0);

	// Set our default cursor
	_vm->_cursor->showCursor();
	if (_globals.heldPage == kNoPage)
		_vm->setMainCursor(kDefaultMystCursor);
	else if (_globals.heldPage < kRedLibraryPage) //A blue page is held
		_vm->setMainCursor(kBluePageCursor);
	else if (_globals.heldPage < kWhitePage) //A red page is held
		_vm->setMainCursor(kRedPageCursor);
	else
		_vm->setMainCursor(kWhitePageCursor);

	return true;
}

bool MystGameState::loadState(int slot) {
	Common::String filename = buildSaveFilename(slot);
	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(filename);
	if (!loadFile) {
		return false;
	}

	debugC(kDebugSaveLoad, "Loading game from '%s'", filename.c_str());

	// First, let's make sure we're using a saved game file from this version of Myst
	// By checking length of file...
	int32 size = loadFile->size();
	if (size != 664 && size != 601) {
		warning("Incompatible saved game version");
		delete loadFile;
		return false;
	}

	Common::Serializer s(loadFile, nullptr);
	syncGameState(s, size == 664);
	delete loadFile;

	return true;
}

void MystGameState::loadMetadata(int slot) {
	// Open the metadata file
	Common::String filename = buildMetadataFilename(slot);
	Common::InSaveFile *metadataFile = _vm->getSaveFileManager()->openForLoading(filename);
	if (!metadataFile) {
		return;
	}

	debugC(kDebugSaveLoad, "Loading metadata from '%s'", filename.c_str());

	Common::Serializer m(metadataFile, nullptr);

	// Read the metadata file
	if (_metadata.sync(m)) {
		_vm->setTotalPlayTime(_metadata.totalPlayTime);
	}

	delete metadataFile;
}

bool MystGameState::save(int slot, const Common::String &desc, const Graphics::Surface *thumbnail, bool autoSave) {
	if (!saveState(slot)) {
		return false;
	}

	updateMetadateForSaving(desc, autoSave);

	return saveMetadata(slot, thumbnail);
}

bool MystGameState::saveState(int slot) {
	// Make sure we have the right extension
	Common::String filename = buildSaveFilename(slot);
	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(filename);
	if (!saveFile) {
		return false;
	}

	debugC(kDebugSaveLoad, "Saving game to '%s'", filename.c_str());

	Common::Serializer s(nullptr, saveFile);
	syncGameState(s, _vm->isGameVariant(GF_ME));
	saveFile->finalize();
	delete saveFile;

	return true;
}

Common::String MystGameState::buildSaveFilename(int slot) {
	return Common::String::format("myst-%03d.mys", slot);
}

Common::String MystGameState::buildMetadataFilename(int slot) {
	return Common::String::format("myst-%03d.mym", slot);
}

void MystGameState::updateMetadateForSaving(const Common::String &desc, bool autoSave) {
	// Update save creation info
	TimeDate t;
	g_system->getTimeAndDate(t);
	_metadata.saveYear = t.tm_year + 1900;
	_metadata.saveMonth = t.tm_mon + 1;
	_metadata.saveDay = t.tm_mday;
	_metadata.saveHour = t.tm_hour;
	_metadata.saveMinute = t.tm_min;
	_metadata.saveDescription = desc;
	_metadata.totalPlayTime = _vm->getTotalPlayTime();
	_metadata.autoSave = autoSave;
}

bool MystGameState::saveMetadata(int slot, const Graphics::Surface *thumbnail) {
	// Write the metadata to a separate file so that the save files
	// are still compatible with the original engine
	Common::String metadataFilename = buildMetadataFilename(slot);
	Common::OutSaveFile *metadataFile = _saveFileMan->openForSaving(metadataFilename);
	if (!metadataFile) {
		return false;
	}

	// Save the metadata
	Common::Serializer m(nullptr, metadataFile);
	_metadata.sync(m);

	// Append a thumbnail
	if (thumbnail) {
		Graphics::saveThumbnail(*metadataFile, *thumbnail);
	} else {
		Graphics::saveThumbnail(*metadataFile);
	}

	metadataFile->finalize();
	delete metadataFile;

	return true;
}

SaveStateDescriptor MystGameState::querySaveMetaInfos(int slot) {
	SaveStateDescriptor desc;
	desc.setWriteProtectedFlag(slot == kAutoSaveSlot);

	// Open the save file
	Common::String filename = buildSaveFilename(slot);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(filename);
	if (!saveFile) {
		return desc;
	}
	delete saveFile;

	// There is a save in the slot
	desc.setSaveSlot(slot);

	// Open the metadata file
	filename = buildMetadataFilename(slot);
	Common::InSaveFile *metadataFile = g_system->getSavefileManager()->openForLoading(filename);
	if (!metadataFile) {
		return desc;
	}

	Common::Serializer m(metadataFile, nullptr);

	// Read the metadata file
	Mohawk::MystSaveMetadata metadata;
	if (!metadata.sync(m)) {
		delete metadataFile;
		return desc;
	}

	// Set the save description
	desc.setDescription(metadata.saveDescription);
	desc.setSaveDate(metadata.saveYear, metadata.saveMonth, metadata.saveDay);
	desc.setSaveTime(metadata.saveHour, metadata.saveMinute);
	desc.setPlayTime(metadata.totalPlayTime);
	desc.setAutosave(metadata.autoSave);
	if (metadata.autoSave) // Allow non-saves to be deleted, but not autosaves
		desc.setDeletableFlag(slot != kAutoSaveSlot);

	Graphics::Surface *thumbnail;
	if (!Graphics::loadThumbnail(*metadataFile, thumbnail)) {
		delete metadataFile;
		return desc;
	}
	desc.setThumbnail(thumbnail);

	delete metadataFile;

	return desc;
}

Common::String MystGameState::querySaveDescription(int slot) {
	// Open the metadata file
	Common::String filename = buildMetadataFilename(slot);
	Common::InSaveFile *metadataFile = g_system->getSavefileManager()->openForLoading(filename);
	if (!metadataFile) {
		return "";
	}

	Common::Serializer m(metadataFile, nullptr);

	// Read the metadata file
	Mohawk::MystSaveMetadata metadata;
	if (!metadata.sync(m)) {
		delete metadataFile;
		return "";
	}

	delete metadataFile;

	return metadata.saveDescription;
}

void MystGameState::syncGameState(Common::Serializer &s, bool isME) {
	// Globals first
	s.syncAsUint16LE(_globals.u0);
	s.syncAsUint16LE(_globals.currentAge);
	s.syncAsUint16LE(_globals.heldPage);
	s.syncAsUint16LE(_globals.u1);
	s.syncAsUint16LE(_globals.transitions);
	s.syncAsUint16LE(_globals.zipMode);
	s.syncAsUint16LE(_globals.redPagesInBook);
	s.syncAsUint16LE(_globals.bluePagesInBook);

	// Onto Myst
	if (isME) {
		s.syncAsUint32LE(_myst.cabinMarkerSwitch);
		s.syncAsUint32LE(_myst.clockTowerMarkerSwitch);
		s.syncAsUint32LE(_myst.dockMarkerSwitch);
		s.syncAsUint32LE(_myst.poolMarkerSwitch);
		s.syncAsUint32LE(_myst.gearsMarkerSwitch);
		s.syncAsUint32LE(_myst.generatorMarkerSwitch);
		s.syncAsUint32LE(_myst.observatoryMarkerSwitch);
		s.syncAsUint32LE(_myst.rocketshipMarkerSwitch);
	} else {
		s.syncAsByte(_myst.cabinMarkerSwitch);
		s.syncAsByte(_myst.clockTowerMarkerSwitch);
		s.syncAsByte(_myst.dockMarkerSwitch);
		s.syncAsByte(_myst.poolMarkerSwitch);
		s.syncAsByte(_myst.gearsMarkerSwitch);
		s.syncAsByte(_myst.generatorMarkerSwitch);
		s.syncAsByte(_myst.observatoryMarkerSwitch);
		s.syncAsByte(_myst.rocketshipMarkerSwitch);
	}

	s.syncAsUint16LE(_myst.greenBookOpenedBefore);
	s.syncAsUint16LE(_myst.shipFloating);
	s.syncAsUint16LE(_myst.cabinValvePosition);
	s.syncAsUint16LE(_myst.clockTowerHourPosition);
	s.syncAsUint16LE(_myst.clockTowerMinutePosition);
	s.syncAsUint16LE(_myst.gearsOpen);
	s.syncAsUint16LE(_myst.clockTowerBridgeOpen);
	s.syncAsUint16LE(_myst.generatorBreakers);
	s.syncAsUint16LE(_myst.generatorButtons);
	s.syncAsUint16LE(_myst.generatorVoltage);
	s.syncAsUint16LE(_myst.libraryBookcaseDoor);
	s.syncAsUint16LE(_myst.imagerSelection);
	s.syncAsUint16LE(_myst.imagerActive);
	s.syncAsUint16LE(_myst.imagerWaterErased);
	s.syncAsUint16LE(_myst.imagerMountainErased);
	s.syncAsUint16LE(_myst.imagerAtrusErased);
	s.syncAsUint16LE(_myst.imagerMarkerErased);
	s.syncAsUint16LE(_myst.towerRotationAngle);
	s.syncAsUint16LE(_myst.courtyardImageBoxes);
	s.syncAsUint16LE(_myst.cabinPilotLightLit);
	s.syncAsUint16LE(_myst.observatoryDaySetting);
	s.syncAsUint16LE(_myst.observatoryLights);
	s.syncAsUint16LE(_myst.observatoryMonthSetting);
	s.syncAsUint16LE(_myst.observatoryTimeSetting);
	s.syncAsUint16LE(_myst.observatoryYearSetting);
	s.syncAsUint16LE(_myst.observatoryDayTarget);
	s.syncAsUint16LE(_myst.observatoryMonthTarget);
	s.syncAsUint16LE(_myst.observatoryTimeTarget);
	s.syncAsUint16LE(_myst.observatoryYearTarget);
	s.syncAsUint16LE(_myst.cabinSafeCombination);
	s.syncAsUint16LE(_myst.treePosition);
	s.syncAsUint32LE(_myst.treeLastMoveTime);

	for (int i = 0; i < 5; i++)
		s.syncAsUint16LE(_myst.rocketSliderPosition[i]);

	s.syncAsUint16LE(_myst.observatoryDaySlider);
	s.syncAsUint16LE(_myst.observatoryMonthSlider);
	s.syncAsUint16LE(_myst.observatoryYearSlider);
	s.syncAsUint16LE(_myst.observatoryTimeSlider);

	// Channelwood
	if (isME) {
		s.syncAsUint32LE(_channelwood.waterPumpBridgeState);
		s.syncAsUint32LE(_channelwood.elevatorState);
		s.syncAsUint32LE(_channelwood.stairsLowerDoorState);
		s.syncAsUint32LE(_channelwood.pipeState);
	} else {
		s.syncAsByte(_channelwood.waterPumpBridgeState);
		s.syncAsByte(_channelwood.elevatorState);
		s.syncAsByte(_channelwood.stairsLowerDoorState);
		s.syncAsByte(_channelwood.pipeState);
	}

	s.syncAsUint16LE(_channelwood.waterValveStates);
	s.syncAsUint16LE(_channelwood.holoprojectorSelection);
	s.syncAsUint16LE(_channelwood.stairsUpperDoorState);

	// Mechanical

	if (isME)
		s.syncAsUint32LE(_mechanical.achenarCrateOpened);
	else
		s.syncAsByte(_mechanical.achenarCrateOpened);

	s.syncAsUint16LE(_mechanical.achenarPanelState);
	s.syncAsUint16LE(_mechanical.sirrusPanelState);
	s.syncAsUint16LE(_mechanical.staircaseState);
	s.syncAsUint16LE(_mechanical.elevatorRotation);

	for (int i = 0; i < 4; i++)
		s.syncAsUint16LE(_mechanical.codeShape[i]);

	// Selenitic

	if (isME) {
		s.syncAsUint32LE(_selenitic.emitterEnabledWater);
		s.syncAsUint32LE(_selenitic.emitterEnabledVolcano);
		s.syncAsUint32LE(_selenitic.emitterEnabledClock);
		s.syncAsUint32LE(_selenitic.emitterEnabledCrystal);
		s.syncAsUint32LE(_selenitic.emitterEnabledWind);
		s.syncAsUint32LE(_selenitic.soundReceiverOpened);
		s.syncAsUint32LE(_selenitic.tunnelLightsSwitchedOn);
	} else {
		s.syncAsByte(_selenitic.emitterEnabledWater);
		s.syncAsByte(_selenitic.emitterEnabledVolcano);
		s.syncAsByte(_selenitic.emitterEnabledClock);
		s.syncAsByte(_selenitic.emitterEnabledCrystal);
		s.syncAsByte(_selenitic.emitterEnabledWind);
		s.syncAsByte(_selenitic.soundReceiverOpened);
		s.syncAsByte(_selenitic.tunnelLightsSwitchedOn);
	}

	s.syncAsUint16LE(_selenitic.soundReceiverCurrentSource);

	for (byte i = 0; i < 5; i++)
		s.syncAsUint16LE(_selenitic.soundReceiverPositions[i]);

	for (byte i = 0; i < 5; i++)
		s.syncAsUint16LE(_selenitic.soundLockSliderPositions[i]);

	// Stoneship

	if (isME) {
		s.syncAsUint32LE(_stoneship.lightState);
	} else {
		s.syncAsByte(_stoneship.lightState);
	}

	s.syncAsUint16LE(_stoneship.sideDoorOpened);
	s.syncAsUint16LE(_stoneship.pumpState);
	s.syncAsUint16LE(_stoneship.trapdoorState);
	s.syncAsUint16LE(_stoneship.chestWaterState);
	s.syncAsUint16LE(_stoneship.chestValveState);
	s.syncAsUint16LE(_stoneship.chestOpenState);
	s.syncAsUint16LE(_stoneship.trapdoorKeyState);
	s.syncAsUint32LE(_stoneship.generatorDuration);
	s.syncAsUint16LE(_stoneship.generatorPowerAvailable);
	s.syncAsUint32LE(_stoneship.generatorDepletionTime);

	// D'ni
	s.syncAsUint16LE(_globals.ending);

	// Already visited zip destinations

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_mystReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_channelwoodReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_mechReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_seleniticReachableZipDests[i]);

	for (byte i = 0; i < 41; i++)
		s.syncAsUint16LE(_stoneshipReachableZipDests[i]);

	if ((isME && s.bytesSynced() != 664) || (!isME && s.bytesSynced() != 601))
		warning("Unexpected File Position 0x%03X At End of Save/Load", s.bytesSynced());
}

void MystGameState::deleteSave(int slot) {
	Common::String filename = buildSaveFilename(slot);
	Common::String metadataFilename = buildMetadataFilename(slot);

	debugC(kDebugSaveLoad, "Deleting save file \'%s\'", filename.c_str());

	g_system->getSavefileManager()->removeSavefile(filename);
	g_system->getSavefileManager()->removeSavefile(metadataFilename);
}

void MystGameState::addZipDest(MystStack stack, uint16 view) {
	ZipDests *zipDests = nullptr;

	// The demo has no zip dest storage
	if (_vm->isGameVariant(GF_DEMO))
		return;

	// Select stack
	switch (stack) {
	case kChannelwoodStack:
		zipDests = &_channelwoodReachableZipDests;
		break;
	case kMechanicalStack:
		zipDests = &_mechReachableZipDests;
		break;
	case kMystStack:
		zipDests = &_mystReachableZipDests;
		break;
	case kSeleniticStack:
		zipDests = &_seleniticReachableZipDests;
		break;
	case kStoneshipStack:
		zipDests = &_stoneshipReachableZipDests;
		break;
	default:
		error("Stack does not have zip destination storage");
	}

	// Check if not already in list
	int16 firstEmpty = -1;
	bool found = false;
	for (uint i = 0; i < ARRAYSIZE(*zipDests); i++) {
		if (firstEmpty == -1 && (*zipDests)[i] == 0)
			firstEmpty = i;

		if ((*zipDests)[i] == view)
			found = true;
	}

	// Add view to array
	if (!found && firstEmpty >= 0)
		(*zipDests)[firstEmpty] = view;
}

bool MystGameState::isReachableZipDest(MystStack stack, uint16 view) {
	// Zip mode enabled
	if (!ConfMan.getBool("zip_mode"))
		return false;

	// The demo has no zip dest storage
	if (_vm->isGameVariant(GF_DEMO))
		return false;

	// Select stack
	ZipDests *zipDests;
	switch (stack) {
	case kChannelwoodStack:
		zipDests = &_channelwoodReachableZipDests;
		break;
	case kMechanicalStack:
		zipDests = &_mechReachableZipDests;
		break;
	case kMystStack:
		zipDests = &_mystReachableZipDests;
		break;
	case kSeleniticStack:
		zipDests = &_seleniticReachableZipDests;
		break;
	case kStoneshipStack:
		zipDests = &_stoneshipReachableZipDests;
		break;
	default:
		error("Stack does not have zip destination storage");
	}

	// Check if in list
	for (uint i = 0; i < ARRAYSIZE(*zipDests); i++) {
		if ((*zipDests)[i] == view)
			return true;
	}

	return false;
}

} // End of namespace Mohawk
