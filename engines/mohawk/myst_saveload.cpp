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

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_saveload.h"

#include "common/serializer.h"
#include "common/util.h"

namespace Mohawk {

MystSaveLoad::MystSaveLoad(MohawkEngine_Myst *vm, Common::SaveFileManager *saveFileMan) : _vm(vm), _saveFileMan(saveFileMan) {
	_v = new MystVariables();

	// Most of the variables are zero at game start.
	_v->globals.u0 = 2;
	// Current Age / Stack - Start in Myst
	_v->globals.currentAge = 7;
	_v->globals.u1 = 1;

	// Library Bookcase Door - Default to Up
	_v->myst.libraryBookcaseDoor = 1;
	// Dock Imager Numeric Selection - Default to 67
	_v->myst.imagerSelection = 67;
	// Dock Imager Active - Default to Active
	_v->myst.imagerActive = 1;
	// Stellar Observatory Lights - Default to On
	_v->myst.observatoryLights = 1;

	// Lighthouse Trapdoor State - Default to Locked
	_v->stoneship.trapdoorState = 2;
	// Lighthouse Chest Water State - Default to Full
	_v->stoneship.chestWaterState = 1;
}

MystSaveLoad::~MystSaveLoad() {
	delete _v;
}

Common::StringArray MystSaveLoad::generateSaveGameList() {
	return _saveFileMan->listSavefiles("*.mys");
}

bool MystSaveLoad::loadGame(const Common::String &filename) {
	if (_vm->getFeatures() & GF_DEMO) // Don't load games in the demo
		return false;

	Common::InSaveFile *loadFile = _saveFileMan->openForLoading(filename);
	if (!loadFile)
		return false;

	debugC(kDebugSaveLoad, "Loading game from '%s'", filename.c_str());

	// First, let's make sure we're using a saved game file from this version of Myst
	// By checking length of file...
	int32 size = loadFile->size();
	if ((size == -1)
	    || (size != 664 && (_vm->getFeatures() & GF_ME))
	    || (size != 601 && !(_vm->getFeatures() & GF_ME))) {
		warning("Incompatible saved game version");
		// FIXME: Add Support to load original game saves in ME and vice versa
		delete loadFile;
		return false;
	}

	Common::Serializer s(loadFile, 0);
	syncGameState(s);
	delete loadFile;

	// Set our default cursor
	switch (_v->globals.heldPage) {
	case 2:
		_vm->setMainCursor(kBluePageCursor);
		break;
	case 8:
		_vm->setMainCursor(kRedPageCursor);
		break;
	case 12:
		_vm->setMainCursor(kWhitePageCursor);
		break;
	default:
		_vm->setMainCursor(kDefaultMystCursor);
	}

	return true;
}

bool MystSaveLoad::saveGame(const Common::String &fname) {
	Common::String filename(fname);
	// Make sure we have the right extension
	if (!filename.hasSuffix(".mys") && !filename.hasSuffix(".MYS"))
		filename += ".mys";

	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(filename);
	if (!saveFile)
		return false;

	debugC(kDebugSaveLoad, "Saving game to '%s'", filename.c_str());

	Common::Serializer s(0, saveFile);
	syncGameState(s);
	saveFile->finalize();
	delete saveFile;

	return true;
}

void MystSaveLoad::syncGameState(Common::Serializer &s) {
	// Globals first
	s.syncAsUint16LE(_v->globals.u0);
	s.syncAsUint16LE(_v->globals.currentAge);
	s.syncAsUint16LE(_v->globals.heldPage);
	s.syncAsUint16LE(_v->globals.u1);
	s.syncAsUint16LE(_v->globals.transitions);
	s.syncAsUint16LE(_v->globals.ending);
	s.syncAsUint16LE(_v->globals.redPagesInBook);
	s.syncAsUint16LE(_v->globals.bluePagesInBook);

	// Onto Myst
	if (_vm->getFeatures() & GF_ME) {
		s.syncAsUint32LE(_v->myst.cabinMarkerSwitch);
		s.syncAsUint32LE(_v->myst.clockTowerMarkerSwitch);
		s.syncAsUint32LE(_v->myst.dockMarkerSwitch);
		s.syncAsUint32LE(_v->myst.poolMarkerSwitch);
		s.syncAsUint32LE(_v->myst.gearsMarkerSwitch);
		s.syncAsUint32LE(_v->myst.generatorMarkerSwitch);
		s.syncAsUint32LE(_v->myst.observatoryMarkerSwitch);
		s.syncAsUint32LE(_v->myst.rocketshipMarkerSwitch);
	} else {
		s.syncAsByte(_v->myst.cabinMarkerSwitch);
		s.syncAsByte(_v->myst.clockTowerMarkerSwitch);
		s.syncAsByte(_v->myst.dockMarkerSwitch);
		s.syncAsByte(_v->myst.poolMarkerSwitch);
		s.syncAsByte(_v->myst.gearsMarkerSwitch);
		s.syncAsByte(_v->myst.generatorMarkerSwitch);
		s.syncAsByte(_v->myst.observatoryMarkerSwitch);
		s.syncAsByte(_v->myst.rocketshipMarkerSwitch);
	}

	s.syncAsUint16LE(_v->myst.greenBookState);
	s.syncAsUint16LE(_v->myst.shipState);
	s.syncAsUint16LE(_v->myst.cabinValvePosition);
	s.syncAsUint16LE(_v->myst.clockTowerHourPosition);
	s.syncAsUint16LE(_v->myst.clockTowerMinutePosition);
	s.syncAsUint16LE(_v->myst.gearsOpen);
	s.syncAsUint16LE(_v->myst.clockTowerBridgeOpen);
	s.syncAsUint16LE(_v->myst.generatorBreakers);
	s.syncAsUint16LE(_v->myst.generatorButtons);
	s.syncAsUint16LE(_v->myst.generatorVoltage);
	s.syncAsUint16LE(_v->myst.libraryBookcaseDoor);
	s.syncAsUint16LE(_v->myst.imagerSelection);
	s.syncAsUint16LE(_v->myst.imagerActive);
	s.syncAsUint16LE(_v->myst.u0);
	s.syncAsUint16LE(_v->myst.u1);
	s.syncAsUint16LE(_v->myst.u2);
	s.syncAsUint16LE(_v->myst.u3);
	s.syncAsUint16LE(_v->myst.towerRotationAngle);
	s.syncAsUint16LE(_v->myst.courtyardImageBoxes);
	s.syncAsUint16LE(_v->myst.cabinPilotLightLit);
	s.syncAsUint16LE(_v->myst.observatoryDaySetting);
	s.syncAsUint16LE(_v->myst.observatoryLights);
	s.syncAsUint16LE(_v->myst.observatoryMonthSetting);
	s.syncAsUint16LE(_v->myst.observatoryTimeSetting);
	s.syncAsUint16LE(_v->myst.observatoryYearSetting);
	s.syncAsUint16LE(_v->myst.observatoryDayTarget);
	s.syncAsUint16LE(_v->myst.observatoryMonthTarget);
	s.syncAsUint16LE(_v->myst.observatoryTimeTarget);
	s.syncAsUint16LE(_v->myst.observatoryYearTarget);
	s.syncAsUint16LE(_v->myst.cabinSafeCombination);
	s.syncAsUint16LE(_v->myst.treePosition);
	s.syncAsUint16LE(_v->myst.u4);
	s.syncAsUint16LE(_v->myst.u5);

	for (int i = 0; i < 5; i++)
		s.syncAsUint16LE(_v->myst.rocketSliderPosition[i]);

	s.syncAsUint16LE(_v->myst.u6);
	s.syncAsUint16LE(_v->myst.u7);
	s.syncAsUint16LE(_v->myst.u8);
	s.syncAsUint16LE(_v->myst.u9);

	// Channelwood
	if (_vm->getFeatures() & GF_ME) {
		s.syncAsUint32LE(_v->channelwood.waterPumpBridgeState);
		s.syncAsUint32LE(_v->channelwood.elevatorState);
		s.syncAsUint32LE(_v->channelwood.stairsLowerDoorState);
		s.syncAsUint32LE(_v->channelwood.pipeState);
	} else {
		s.syncAsByte(_v->channelwood.waterPumpBridgeState);
		s.syncAsByte(_v->channelwood.elevatorState);
		s.syncAsByte(_v->channelwood.stairsLowerDoorState);
		s.syncAsByte(_v->channelwood.pipeState);
	}

	s.syncAsUint16LE(_v->channelwood.waterValveStates);
	s.syncAsUint16LE(_v->channelwood.holoprojectorSelection);
	s.syncAsUint16LE(_v->channelwood.stairsUpperDoorState);

	if (_vm->getFeatures() & GF_ME)
		s.skip(4);
	else
		s.skip(1);

	// Mechanical

	s.syncAsUint16LE(_v->mechanical.achenarPanelState);
	s.syncAsUint16LE(_v->mechanical.sirrusPanelState);
	s.syncAsUint16LE(_v->mechanical.staircaseState);
	s.syncAsUint16LE(_v->mechanical.elevatorRotation);

	for (int i = 0; i < 4; i++)
		s.syncAsUint16LE(_v->mechanical.codeShape[i]);

	// Selenitic

	if (_vm->getFeatures() & GF_ME) {
		s.syncAsUint32LE(_v->selenitic.emitterEnabledWater);
		s.syncAsUint32LE(_v->selenitic.emitterEnabledVolcano);
		s.syncAsUint32LE(_v->selenitic.emitterEnabledClock);
		s.syncAsUint32LE(_v->selenitic.emitterEnabledCrystal);
		s.syncAsUint32LE(_v->selenitic.emitterEnabledWind);
		s.syncAsUint32LE(_v->selenitic.soundReceiverOpened);
		s.syncAsUint32LE(_v->selenitic.tunnelLightsSwitchedOn);
	} else {
		s.syncAsByte(_v->selenitic.emitterEnabledWater);
		s.syncAsByte(_v->selenitic.emitterEnabledVolcano);
		s.syncAsByte(_v->selenitic.emitterEnabledClock);
		s.syncAsByte(_v->selenitic.emitterEnabledCrystal);
		s.syncAsByte(_v->selenitic.emitterEnabledWind);
		s.syncAsByte(_v->selenitic.soundReceiverOpened);
		s.syncAsByte(_v->selenitic.tunnelLightsSwitchedOn);
	}

	s.syncAsUint16LE(_v->selenitic.soundReceiverCurrentSource);

	for (byte i = 0; i < 5; i++)
		s.syncAsUint16LE(_v->selenitic.soundReceiverPositions[i]);

	for (byte i = 0; i < 5; i++)
		s.syncAsUint16LE(_v->selenitic.soundLockSliderPositions[i]);

	// Stoneship

	if (_vm->getFeatures() & GF_ME) {
		s.syncAsUint16LE(_v->stoneship.lightState);
		s.syncAsUint16LE(_v->stoneship.u0);
		s.syncAsUint16LE(_v->stoneship.u1);
	} else {
		s.syncAsByte(_v->stoneship.lightState);
		s.syncAsByte(_v->stoneship.u0);
		s.syncAsByte(_v->stoneship.u1);
	}

	s.syncAsUint16LE(_v->stoneship.pumpState);
	s.syncAsUint16LE(_v->stoneship.trapdoorState);
	s.syncAsUint16LE(_v->stoneship.chestWaterState);
	s.syncAsUint16LE(_v->stoneship.chestValveState);
	s.syncAsUint16LE(_v->stoneship.chestOpenState);
	s.syncAsUint16LE(_v->stoneship.trapdoorKeyState);

	for (int i = 0; i < 5; i++)
		s.syncAsUint16LE(_v->stoneship.generatorPowerLevel[i]);

	// D'ni
	s.syncAsUint16LE(_v->dni.outcomeState);

	// Reading unknown region...
	// When Zero Value regions are included, these are 5 blocks of
	// 41 uint16 values.

	for (byte i = 0; i < 31; i++)
		s.syncAsUint16LE(_v->unknownMyst[i]);

	s.skip(20);

	for (byte i = 0; i < 37; i++)
		s.syncAsUint16LE(_v->unknownChannelwood[i]);

	s.skip(8);

	for (byte i = 0; i < 18; i++)
		s.syncAsUint16LE(_v->unknownMech[i]);

	s.skip(46);

	for (byte i = 0; i < 30; i++)
		s.syncAsUint16LE(_v->unknownSelenitic[i]);

	s.skip(22);

	for (byte i = 0; i < 22; i++)
		s.syncAsUint16LE(_v->unknownStoneship[i]);

	s.skip(38);

	if ((_vm->getFeatures() & GF_ME && s.bytesSynced() != 664) || (!(_vm->getFeatures() & GF_ME) && s.bytesSynced() != 601))
		warning("Unexpected File Position 0x%03X At End of Save/Load", s.bytesSynced());
}

void MystSaveLoad::deleteSave(const Common::String &saveName) {
	debugC(kDebugSaveLoad, "Deleting save file \'%s\'", saveName.c_str());
	_saveFileMan->removeSavefile(saveName.c_str());
}

} // End of namespace Mohawk
