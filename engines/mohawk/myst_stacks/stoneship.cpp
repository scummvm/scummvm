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
#include "mohawk/myst_areas.h"
#include "mohawk/myst_card.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_state.h"
#include "mohawk/myst_sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/stoneship.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {
namespace MystStacks {

Stoneship::Stoneship(MohawkEngine_Myst *vm) :
		MystScriptParser(vm, kStoneshipStack),
		_state(vm->_gameState->_stoneship) {
	setupOpcodes();

	_tunnelRunning = false;
	_tunnelNextTime = 0;
	_tunnelAlarmSound = 0;
	_tunnelImagesCount = 0;

	_state.lightState = 0;
	_state.generatorDepletionTime = 0;
	_state.generatorDuration = 0;
	_cabinMystBookPresent = 0;
	_siriusDrawerDrugsOpen = 0;
	_chestDrawersOpen = 0;
	_chestAchenarBottomDrawerClosed = 1;

	_brotherDoorOpen = 0;

	// Drop key
	if (_state.trapdoorKeyState == 1)
		_state.trapdoorKeyState = 2;

	// Power is not available when loading
	if (_state.sideDoorOpened)
		_state.generatorPowerAvailable = 2;
	else
		_state.generatorPowerAvailable = 0;

	_batteryCharging = false;
	_batteryDepleting = false;
	_batteryNextTime = 0;
	_batteryLastCharge = 0;
	_batteryGaugeRunning = false;
	_batteryGauge = nullptr;

	_hologramTurnedOn = 0;
	_hologramDisplay = nullptr;
	_hologramSelection = nullptr;
	_hologramDisplayPos = 0;

	_telescopeRunning = false;
	_telescopePosition = 0;
	_telescopePanorama = 0;
	_telescopeOldMouse = 0;
	_telescopeLighthouseOff = 0;
	_telescopeLighthouseOn = 0;
	_telescopeLighthouseState = false;
	_telescopeNexTime = 0;

	_cloudOrbMovie = nullptr;
	_cloudOrbSound = 0;
	_cloudOrbStopSound = 0;
}

Stoneship::~Stoneship() {
}

void Stoneship::setupOpcodes() {
	// "Stack-Specific" Opcodes
	REGISTER_OPCODE(100, Stoneship, o_pumpTurnOff);
	REGISTER_OPCODE(101, Stoneship, o_brotherDoorOpen);
	REGISTER_OPCODE(102, Stoneship, o_cabinBookMovie);
	REGISTER_OPCODE(103, Stoneship, o_drawerOpenSirius);
	REGISTER_OPCODE(104, Stoneship, o_drawerClose);
	REGISTER_OPCODE(105, Stoneship, o_telescopeStart);
	REGISTER_OPCODE(106, Stoneship, o_telescopeMove);
	REGISTER_OPCODE(107, Stoneship, o_telescopeStop);
	REGISTER_OPCODE(108, Stoneship, o_generatorStart);
	REGISTER_OPCODE(109, Stoneship, NOP);
	REGISTER_OPCODE(110, Stoneship, o_generatorStop);
	REGISTER_OPCODE(111, Stoneship, o_drawerOpenAchenar);
	REGISTER_OPCODE(112, Stoneship, o_hologramPlayback);
	REGISTER_OPCODE(113, Stoneship, o_hologramSelectionStart);
	REGISTER_OPCODE(114, Stoneship, o_hologramSelectionMove);
	REGISTER_OPCODE(115, Stoneship, o_hologramSelectionStop);
	REGISTER_OPCODE(116, Stoneship, o_compassButton);
	REGISTER_OPCODE(117, Stoneship, o_chestValveVideos);
	REGISTER_OPCODE(118, Stoneship, o_chestDropKey);
	REGISTER_OPCODE(119, Stoneship, o_trapLockOpen);
	REGISTER_OPCODE(120, Stoneship, o_sideDoorsMovies);
	REGISTER_OPCODE(121, Stoneship, o_cloudOrbEnter);
	REGISTER_OPCODE(122, Stoneship, o_cloudOrbLeave);
	REGISTER_OPCODE(125, Stoneship, o_drawerCloseOpened);

	// "Init" Opcodes
	REGISTER_OPCODE(200, Stoneship, o_hologramDisplay_init);
	REGISTER_OPCODE(201, Stoneship, o_hologramSelection_init);
	REGISTER_OPCODE(202, Stoneship, o_battery_init);
	REGISTER_OPCODE(203, Stoneship, o_tunnelEnter_init);
	REGISTER_OPCODE(204, Stoneship, o_batteryGauge_init);
	REGISTER_OPCODE(205, Stoneship, o_tunnel_init);
	REGISTER_OPCODE(206, Stoneship, o_tunnelLeave_init);
	REGISTER_OPCODE(207, Stoneship, o_chest_init);
	REGISTER_OPCODE(208, Stoneship, o_telescope_init);
	REGISTER_OPCODE(209, Stoneship, o_achenarDrawers_init);
	REGISTER_OPCODE(210, Stoneship, o_cloudOrb_init);

	// "Exit" Opcodes
	REGISTER_OPCODE(300, Stoneship, NOP);
}

void Stoneship::disablePersistentScripts() {
	_batteryCharging = false;
	_batteryDepleting = false;
	_batteryGaugeRunning = false;
	_telescopeRunning = false;
}

void Stoneship::runPersistentScripts() {
	if (_batteryCharging)
		chargeBattery_run();

	if (_telescopeRunning)
		telescope_run();

	if (_batteryGaugeRunning)
		batteryGauge_run();

	if (_batteryDepleting)
		batteryDeplete_run();

	if (_tunnelRunning)
		tunnel_run();
}

uint16 Stoneship::getVar(uint16 var) {
	switch(var) {
	case 0: // Water Drained From Lighthouse / Right Button Of Pump
		return _state.pumpState == 4;
	case 1: // Water Drained From Tunnels To Brothers' Rooms / Middle Button Of Pump
		return _state.pumpState == 2;
	case 2: // Water Drained From Ship Cabin Tunnel / Left Button Of Pump
		return _state.pumpState == 1;
	case 3: // Lighthouse Chest Floating
		return _state.pumpState != 4 && !_state.chestValveState && !_state.chestWaterState;
	case 4: // Lighthouse State - Close Up
		if (_state.pumpState == 4) {
			return 1; // Drained
		} else {
			if (_state.chestValveState || _state.chestWaterState)
				return 0; // Flooded
			else
				return 2; // Flooded, Chest Floating
		}
	case 5: // Lighthouse Trapdoor State
		return _state.trapdoorState;
	case 6: // Chest valve state
		return _state.chestValveState;
	case 7: // Lighthouse Chest Unlocked
		return _state.chestOpenState;
	case 8: // Lighthouse Chest Key Position
		return _state.trapdoorKeyState;
	case 11: // Lighthouse Key State
		if (_state.chestOpenState) {
			if (_state.trapdoorKeyState == 1)
				return 1;
			else if (_state.trapdoorKeyState == 2)
				return 2;
			else
				return 3;
		} else {
			return 0;
		}
	case 12: // Trapdoor can be unlocked
		return _state.trapdoorKeyState == 1 && _state.trapdoorState == 2;
	case 13: // State Of Tunnels To Brothers' Rooms - Close Up
		if (_state.generatorPowerAvailable != 1) {
			if (_state.pumpState != 2)
				return 0; // Dark, Flooded
			else
				return 1; // Dark, Drained
		} else {
			if (_state.pumpState != 2)
				return 2; // Lit, Flooded
			else
				return 3; // Lit, Drained
		}
	case 14: // State Of Tunnels lights To Brothers' Rooms - Far
		return _state.generatorPowerAvailable;
	case 15: // Side Door in Tunnels To Brother's Rooms Open
		if (_state.generatorPowerAvailable == 1)
			return _state.sideDoorOpened;
		else
			return 0;
	case 16: // Ship Chamber Light State
		return _state.lightState;
	case 17: // Sirrus' Room Drawer with Drugs Open
		return _siriusDrawerDrugsOpen;
	case 18: // Brother Room Door Open
		return _brotherDoorOpen;
	case 19: // Brother Room Door State
		if (_brotherDoorOpen) {
			if (_state.lightState)
				return 2; // Open, Light On
			else
				return 1; // Open, Light Off
		} else {
			return 0; // Closed
		}
	case 20: // Ship Chamber Table/Book State
		return _cabinMystBookPresent;
	case 21: // Brothers Rooms' Chest Of Drawers Drawer State
		return _chestDrawersOpen;
	case 28: // Telescope Angle Position
		return 0;
	case 29: // Achenar's Room Rose/Skull Hologram Button Lit
		return _hologramTurnedOn;
	case 30: // Light State in Tunnel to Compass Rose Room
		if (_state.generatorPowerAvailable == 1) {
			if (_state.lightState)
				return 0;
			else
				return 1;
		} else {
			return 2;
		}
	case 31: // Lighthouse Lamp Room Battery Pack Indicator Light
		return batteryRemainingCharge() >= 10;
	case 32: // Lighthouse Lamp Room Battery Pack Meter Level
		return 0;
	case 33: // State of Side Door in Tunnels to Compass Rose Room
		if (_state.sideDoorOpened)
			return 2;
		else
			return _state.generatorPowerAvailable == 1;
	case 34: // Achenar's Room Drawer with Torn Note Closed
		return _chestAchenarBottomDrawerClosed;
	case 35: // Sirrus' Room Drawer #4 (Bottom) Open and Red Page State
		if (_chestDrawersOpen == 4)
			return getVar(102);
		else
			return 2;
	case 36: // Ship Chamber Door State
		if (_tempVar) {
			if (_state.lightState)
				return 2; // Open, Light On
			else
				return 1; // Open, Light Off
		} else {
			return 0; // Closed
		}
	case 102: // Red page
		return !(_globals.redPagesInBook & 8) && (_globals.heldPage != kRedStoneshipPage);
	case 103: // Blue page
		return !(_globals.bluePagesInBook & 8) && (_globals.heldPage != kBlueStoneshipPage);
	default:
		return MystScriptParser::getVar(var);
	}
}

void Stoneship::toggleVar(uint16 var) {
	switch(var) {
	case 0: // Water Drained From Lighthouse / Right Button Of Pump
		if (_state.pumpState == 4)
			_state.pumpState = 0;
		else
			_state.pumpState = 4;
		break;
	case 1: // Water Drained From Tunnels To Brothers' Rooms / Middle Button Of Pump
		if (_state.pumpState == 2)
			_state.pumpState = 0;
		else
			_state.pumpState = 2;
		break;
	case 2: // Water Drained From Ship Cabin Tunnel / Left Button Of Pump
		if (_state.pumpState == 1)
			_state.pumpState = 0;
		else
			_state.pumpState = 1;
		break;
	case 6: // Chest valve state
		_state.chestValveState = (_state.chestValveState + 1) % 2;
		break;
	case 8:  // Lighthouse Chest Key Position
		if (_state.trapdoorKeyState) {
			if (_state.trapdoorKeyState == 1)
				_state.trapdoorKeyState = 2;
			else
				_state.trapdoorKeyState = 1;
		}
		break;
	case 10: // Chest water state
		_state.chestWaterState = 0;
		break;
	case 11:
		if (_state.chestOpenState)
			_state.trapdoorKeyState = _state.trapdoorKeyState != 1;
		break;
	case 20: // Ship Chamber Table/Book State
		_cabinMystBookPresent = (_cabinMystBookPresent + 1) % 2;
		break;
	case 29: // Achenar's Room Rose/Skull Hologram Button Lit
		_hologramTurnedOn = (_hologramTurnedOn + 1) % 2;
		break;
	case 102: // Red page
		if (!(_globals.redPagesInBook & 8)) {
			if (_globals.heldPage == kRedStoneshipPage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kRedStoneshipPage;
		}
		break;
	case 103: // Blue page
		if (!(_globals.bluePagesInBook & 8)) {
			if (_globals.heldPage == kBlueStoneshipPage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kBlueStoneshipPage;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool Stoneship::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 5: // Lighthouse Trapdoor State
		_state.trapdoorState = value;
		break;
	case 7:
		if (_state.chestOpenState != value)
			_state.chestOpenState = value;
		break;
	case 8: // Lighthouse Chest Key Position
		_state.trapdoorKeyState = value;
		break;
	case 15: // Side Door in Tunnels To Brother's Rooms Open
		if (_state.sideDoorOpened != value) {
			if (!value && _state.generatorPowerAvailable == 2)
				_state.generatorPowerAvailable = 0;
			_state.sideDoorOpened = value;
			refresh = true;
		}
		break;
	case 17: // Sirrus' Room Drawer with Drugs Open
		if (_siriusDrawerDrugsOpen != value) {
			_siriusDrawerDrugsOpen = value;
			refresh = true;
		}
		break;
	case 18: // Brother Room Door Open
		if (_brotherDoorOpen != value) {
			_brotherDoorOpen = value;
			refresh = true;
		}
		break;
	case 21: // Brothers Rooms' Chest Of Drawers Drawer State
		if (_chestDrawersOpen != value) {
			_chestDrawersOpen = value;
			refresh = true;
		}
		break;
	case 29: // Achenar's Room Rose/Skull Hologram Button Lit
		_hologramTurnedOn = value;
		break;
	case 34: // Achenar's Room Drawer with Torn Note Closed
		_chestAchenarBottomDrawerClosed = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

void Stoneship::o_pumpTurnOff(uint16 var, const ArgumentsArray &args) {
	if (_state.pumpState) {
		uint16 buttonVar = 0;

		switch (_state.pumpState) {
		case 1:
			buttonVar = 2;
			break;
		case 2:
			buttonVar = 1;
			break;
		case 4:
			buttonVar = 0;
			break;
		default:
			warning("Incorrect pump state");
		}

		for (uint i = 0; i < _vm->getCard()->_resources.size(); i++) {
			MystArea *resource = _vm->getCard()->_resources[i];
			if (resource->hasType(kMystAreaImageSwitch) && resource->getImageSwitchVar() == buttonVar) {
				static_cast<MystAreaImageSwitch *>(resource)->drawConditionalDataToScreen(0, true);
				break;
			}
		}
	}
}

void Stoneship::o_brotherDoorOpen(uint16 var, const ArgumentsArray &args) {
	_brotherDoorOpen = 1;
	_vm->getCard()->redrawArea(19, 0);
	animatedUpdate(args, 5);
}

void Stoneship::o_cabinBookMovie(uint16 var, const ArgumentsArray &args) {
	uint16 startTime = args[0];
	uint16 endTime = args[1];

	VideoEntryPtr book = _vm->playMovie("bkroom", kStoneshipStack);
	book->moveTo(159, 99);
	book->setBounds(Audio::Timestamp(0, startTime, 600), Audio::Timestamp(0, endTime, 600));

	_vm->waitUntilMovieEnds(book);
}

void Stoneship::o_drawerOpenSirius(uint16 var, const ArgumentsArray &args) {
	MystAreaImageSwitch *drawer = _vm->getCard()->getResource<MystAreaImageSwitch>(args[0]);

	if (drawer->getImageSwitchVar() == 35) {
		drawer->drawConditionalDataToScreen(getVar(102), 0);
	} else {
		drawer->drawConditionalDataToScreen(0, 0);
	}

	TransitionType transition = kTransitionTopToBottom;
	if (args.size() == 2 && args[1])
		transition = kTransitionCopy;

	_vm->_gfx->runTransition(transition, drawer->getRect(), 25, 5);
}

void Stoneship::o_drawerClose(uint16 var, const ArgumentsArray &args) {
	drawerClose(args[0]);
}

void Stoneship::o_telescopeStart(uint16 var, const ArgumentsArray &args) {
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();
	_telescopeOldMouse = mouse.x;
	_vm->_cursor->setCursor(700);
}

void Stoneship::o_telescopeMove(uint16 var, const ArgumentsArray &args) {
	MystAreaDrag *display = getInvokingResource<MystAreaDrag>();
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	// Compute telescope position
	_telescopePosition = (_telescopePosition - (mouse.x - _telescopeOldMouse) / 2 + 3240) % 3240;
	_telescopeOldMouse = mouse.x;

	// Copy image to screen
    Common::Rect src = Common::Rect(_telescopePosition, 0, _telescopePosition + 112, 112);
    _vm->_gfx->copyImageSectionToScreen(_telescopePanorama, src, display->getRect());

    // Draw lighthouse
    telescopeLighthouseDraw();
}

void Stoneship::o_telescopeStop(uint16 var, const ArgumentsArray &args) {
	_vm->refreshCursor();
}

void Stoneship::o_generatorStart(uint16 var, const ArgumentsArray &args) {
	MystAreaDrag *handle = getInvokingResource<MystAreaDrag>();

	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);

	if (_state.generatorDuration)
		_state.generatorDuration -= _vm->getTotalPlayTime() - _state.generatorDepletionTime;

	// Start charging the battery
	_batteryDepleting = false;
	_batteryCharging = true;
	_batteryNextTime = _vm->getTotalPlayTime() + 1000;

	// Start handle movie
	MystAreaVideo *movie = static_cast<MystAreaVideo *>(handle->getSubResource(0));
	movie->playMovie();

	soundId = handle->getList2(0);
	if (soundId)
		_vm->_sound->playEffect(soundId, true);
}

void Stoneship::o_generatorStop(uint16 var, const ArgumentsArray &args) {
	_batteryCharging = false;

	if (_state.generatorDuration) {
		// Clip battery power
		if (_state.generatorDuration > 600000)
			_state.generatorDuration = 600000;

		// Start depleting power
		_state.generatorDepletionTime = _vm->getTotalPlayTime() + _state.generatorDuration;
		_state.generatorPowerAvailable = 1;
		_batteryDepleting = true;
		_batteryNextTime = _vm->getTotalPlayTime() + 60000;
	}

	// Pause handle movie
	MystAreaDrag *handle = getInvokingResource<MystAreaDrag>();
	MystAreaVideo *movie = static_cast<MystAreaVideo *>(handle->getSubResource(0));
	movie->pauseMovie(true);

	uint16 soundId = handle->getList3(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);
}

void Stoneship::chargeBattery_run() {
	uint32 time = _vm->getTotalPlayTime();

	if (time > _batteryNextTime) {
		_batteryNextTime = time + 1000;
		_state.generatorDuration += 30000;
	}
}

uint16 Stoneship::batteryRemainingCharge() {
	uint32 time = _vm->getTotalPlayTime();

	if (_state.generatorDepletionTime > time) {
		return (_state.generatorDepletionTime - time) / 7500;
	} else {
		return 0;
	}
}

void Stoneship::batteryDeplete_run() {
	uint32 time = _vm->getTotalPlayTime();

	if (time > _batteryNextTime) {
		if (_state.generatorDuration > 60000) {
			_state.generatorDuration -= 60000;
			_batteryNextTime = time + 60000;
		} else { // Battery depleted
			_state.generatorDuration = 0;
			_state.generatorDepletionTime = 0;

			if (_state.sideDoorOpened)
				_state.generatorPowerAvailable = 2;
			else
				_state.generatorPowerAvailable = 0;

			_batteryDepleting = false;
		}
	}
}

void Stoneship::o_drawerOpenAchenar(uint16 var, const ArgumentsArray &args) {
	MystAreaImageSwitch *drawer = _vm->getCard()->getResource<MystAreaImageSwitch>(args[0]);
	drawer->drawConditionalDataToScreen(0, 0);
	_vm->_gfx->runTransition(kTransitionTopToBottom, drawer->getRect(), 25, 5);
}

void Stoneship::o_hologramPlayback(uint16 var, const ArgumentsArray &args) {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	uint16 startPoint = args[0];
	uint16 endPoint = args[1];
	// uint16 direction = args[2];

	_hologramDisplay->setBlocking(false);
	VideoEntryPtr displayMovie = _hologramDisplay->playMovie();

	if (_hologramTurnedOn) {
		if (_hologramDisplayPos)
			endPoint = _hologramDisplayPos;
		displayMovie->setBounds(Audio::Timestamp(0, startPoint, 600), Audio::Timestamp(0, endPoint, 600));
	} else {
		displayMovie->setBounds(Audio::Timestamp(0, startPoint, 600), Audio::Timestamp(0, endPoint, 600));
	}

	_vm->waitUntilMovieEnds(displayMovie);
}

void Stoneship::o_hologramSelectionStart(uint16 var, const ArgumentsArray &args) {
	//_vm->_cursor->setCursor(0);
}

void Stoneship::o_hologramSelectionMove(uint16 var, const ArgumentsArray &args) {
	MystAreaDrag *handle = getInvokingResource<MystAreaDrag>();
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		int16 position = mouse.x - 143;
		position = CLIP<int16>(position, 0, 242);

		// Draw handle movie frame
		uint16 selectionPos = position * 1500 / 243;

		VideoEntryPtr handleMovie = _hologramSelection->playMovie();
		_vm->_video->drawVideoFrame(handleMovie, Audio::Timestamp(0, selectionPos, 600));

		_hologramDisplayPos = position * 1450 / 243 + 350;

		// Draw display movie frame
		if (_hologramTurnedOn) {
			_hologramDisplay->setBlocking(false);
			VideoEntryPtr displayMovie = _hologramDisplay->playMovie();
			_vm->_video->drawVideoFrame(displayMovie, Audio::Timestamp(0, _hologramDisplayPos, 600));
		}
	}
}

void Stoneship::o_hologramSelectionStop(uint16 var, const ArgumentsArray &args) {
	_vm->refreshCursor();
}

void Stoneship::o_compassButton(uint16 var, const ArgumentsArray &args) {
	// Used on Card 2111 (Compass Rose)
	// Called when Button Clicked.
	uint16 correctButton = args[0];

	if (correctButton) {
		// Correct Button -> Light On Logic
		_state.lightState = 1;
	} else {
		// Wrong Button -> Power Failure Logic
		_state.generatorPowerAvailable = 2;
		_state.lightState = 0;
		_state.generatorDepletionTime = 0;
		_state.generatorDuration = 0;

		_batteryDepleting = false;
	}

	o_redrawCard(var, args);
}

void Stoneship::o_chestValveVideos(uint16 var, const ArgumentsArray &args) {
	_vm->_sound->playEffect(2132);

	if (_state.chestValveState) {
		// Valve closing
		VideoEntryPtr valve = _vm->playMovie("ligspig", kStoneshipStack);
		valve->moveTo(97, 267);
		valve->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 350, 600));

		_vm->waitUntilMovieEnds(valve);
	} else if (_state.chestWaterState) {
		// Valve opening, spilling water
		VideoEntryPtr valve = _vm->playMovie("ligspig", kStoneshipStack);
		valve->moveTo(97, 267);
		valve->setBounds(Audio::Timestamp(0, 350, 600), Audio::Timestamp(0, 650, 600));

		_vm->waitUntilMovieEnds(valve);

		_vm->_sound->playEffect(3132);

		for (uint i = 0; i < 25; i++) {
			valve = _vm->playMovie("ligspig", kStoneshipStack);
			valve->moveTo(97, 267);
			valve->setBounds(Audio::Timestamp(0, 650, 600), Audio::Timestamp(0, 750, 600));
			_vm->waitUntilMovieEnds(valve);
		}

		_vm->_sound->resumeBackground();
	} else {
		// Valve opening
		VideoEntryPtr valve = _vm->playMovie("ligspig", kStoneshipStack);
		valve->moveTo(97, 267);
		valve->seek(Audio::Timestamp(0, 350, 600));
		valve->setRate(-1);

		_vm->waitUntilMovieEnds(valve);
	}
}

void Stoneship::o_chestDropKey(uint16 var, const ArgumentsArray &args) {
	// If holding Key to Lamp Room Trapdoor, drop to bottom of
	// Lighthouse...
	if (_state.trapdoorKeyState == 1) {
		_vm->setMainCursor(_savedCursorId);
		_state.trapdoorKeyState = 2;
	}
}

void Stoneship::o_trapLockOpen(uint16 var, const ArgumentsArray &args) {
	VideoEntryPtr lock = _vm->playMovie("openloc", kStoneshipStack);
	lock->moveTo(187, 71);
	lock->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 750, 600));
	_vm->waitUntilMovieEnds(lock);

	_vm->_sound->playEffect(2143);

	lock = _vm->playMovie("openloc", kStoneshipStack);
	lock->moveTo(187, 71);
	lock->setBounds(Audio::Timestamp(0, 750, 600), Audio::Timestamp(0, 10000, 600));
	_vm->waitUntilMovieEnds(lock);

	if (_state.pumpState != 4)
		_vm->_sound->playEffect(4143);
}

void Stoneship::o_sideDoorsMovies(uint16 var, const ArgumentsArray &args) {
	// Used for Cards 2285, 2289, 2247, 2251 (Side Doors in Tunnels Down To Brothers Rooms)
	uint16 movieId = args[0];

	_vm->_cursor->hideCursor();
	_vm->_sound->pauseBackground();

	switch (movieId) {
	case 0:
		// Card 2251
		_vm->playMovieBlocking("tunaup", kStoneshipStack, 149, 161);
		break;
	case 1:
		// Card 2247
		_vm->playMovieBlocking("tunadown", kStoneshipStack, 218, 150);
		break;
	case 2:
		// Card 2289
		_vm->playMovieBlocking("tuncup", kStoneshipStack, 259, 161);
		break;
	case 3:
		// Card 2285
		_vm->playMovieBlocking("tuncdown", kStoneshipStack, 166, 150);
		break;
	default:
		warning("Opcode 120 MovieId Out Of Range");
		break;
	}

	_vm->_sound->resumeBackground();
	_vm->_cursor->showCursor();
}

void Stoneship::o_cloudOrbEnter(uint16 var, const ArgumentsArray &args) {
	_vm->_sound->playEffect(_cloudOrbSound, true);
	_cloudOrbMovie->playMovie();
}

void Stoneship::o_cloudOrbLeave(uint16 var, const ArgumentsArray &args) {
	_cloudOrbMovie->pauseMovie(true);
	_vm->_sound->playEffect(_cloudOrbStopSound);
	_vm->_gfx->runTransition(kTransitionTopToBottom, getInvokingResource<MystArea>()->getRect(), 4, 0);
}

void Stoneship::o_drawerCloseOpened(uint16 var, const ArgumentsArray &args) {
	uint16 drawerOpen = getVar(var);
	if (drawerOpen)
		drawerClose(args[0] + drawerOpen - 1);
}

void Stoneship::drawerClose(uint16 drawer) {
	_chestDrawersOpen = 0;
	_vm->getCard()->drawBackground();
	_vm->getCard()->drawResourceImages();

	MystArea *res = _vm->getCard()->getResource<MystArea>(drawer);
	_vm->_gfx->runTransition(kTransitionBottomToTop, res->getRect(), 25, 5);
}

void Stoneship::o_hologramDisplay_init(uint16 var, const ArgumentsArray &args) {
	_hologramDisplay = getInvokingResource<MystAreaVideo>();

	_hologramDisplayPos = 0;
}

void Stoneship::o_hologramSelection_init(uint16 var, const ArgumentsArray &args) {
	_hologramSelection = getInvokingResource<MystAreaVideo>();
}

void Stoneship::batteryGaugeUpdate() {
	uint16 charge = 0;

	if (_state.generatorDepletionTime) {
		charge = batteryRemainingCharge();
	}

	Common::Rect rect = _batteryGauge->getRect();

	rect.top = rect.bottom - charge;

	_batteryGauge->setRect(rect);
}

void Stoneship::o_battery_init(uint16 var, const ArgumentsArray &args) {
	// Used for Card 2160 (Lighthouse Battery Pack Closeup)
	_batteryGauge = getInvokingResource<MystAreaImageSwitch>();

	batteryGaugeUpdate();
}

void Stoneship::o_tunnelEnter_init(uint16 var, const ArgumentsArray &args) {
	o_tunnel_init(var, args);

	_tunnelRunning = true;
	_tunnelNextTime = _vm->getTotalPlayTime() + 1500;
}

void Stoneship::o_batteryGauge_init(uint16 var, const ArgumentsArray &args) {
	_batteryLastCharge = batteryRemainingCharge();
	_batteryGaugeRunning = true;
}

void Stoneship::batteryGauge_run() {
	uint16 batteryCharge = batteryRemainingCharge();

	if (batteryCharge != _batteryLastCharge) {
		batteryGaugeUpdate();

		_batteryLastCharge = batteryCharge;

		// Redraw card
		_vm->getCard()->drawBackground();
		_vm->getCard()->drawResourceImages();
		_vm->_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
	}
}

void Stoneship::o_tunnel_init(uint16 var, const ArgumentsArray &args) {
		_tunnelImagesCount = args[0];

		assert(_tunnelImagesCount <= 2 && "Too many images");

		for (uint i = 0; i < _tunnelImagesCount; i++) {
			_tunnelImages[i] = args[i + 1];
		}

		_tunnelAlarmSound = args[args.size() - 1];

		debugC(kDebugScript, "\timage count: %d", _tunnelImagesCount);
		debugC(kDebugScript, "\tsoundIdAlarm: %d", _tunnelAlarmSound);
}

void Stoneship::tunnel_run() {
	uint32 time = _vm->getTotalPlayTime();

	if (time > _tunnelNextTime) {
		_tunnelNextTime = time + 1500;
		if (_state.generatorPowerAvailable == 2) {
			// Draw tunnel black
			if (_tunnelImagesCount) {
				_vm->_gfx->copyImageToScreen(_tunnelImages[1], Common::Rect(544, 333));
			}

			_vm->_sound->playEffect(_tunnelAlarmSound);

			// Draw tunnel dark
			if (_tunnelImagesCount) {
				_vm->_gfx->copyImageToScreen(_tunnelImages[0], Common::Rect(544, 333));
			}
		}
	}
}

void Stoneship::o_tunnelLeave_init(uint16 var, const ArgumentsArray &args) {
	_tunnelRunning = false;
}

void Stoneship::o_chest_init(uint16 var, const ArgumentsArray &args) {
	_state.chestOpenState = 0;
}

void Stoneship::o_telescope_init(uint16 var, const ArgumentsArray &args) {
	// Used in Card 2218 (Telescope view)
	_telescopePanorama = args[0];
	_telescopeLighthouseOff = args[1];
	_telescopeLighthouseOn = args[2];
	_telescopePosition = 0;

	_telescopeRunning = true;
	_telescopeLighthouseState = false;
	_telescopeNexTime = _vm->getTotalPlayTime() + 1000;
}

void Stoneship::telescope_run() {
	uint32 time = _vm->getTotalPlayTime();

	if (time > _telescopeNexTime) {

		_telescopeNexTime = time + 1000;
		_telescopeLighthouseState = !_telescopeLighthouseState;

		telescopeLighthouseDraw();
	}
}

void Stoneship::telescopeLighthouseDraw() {
	if (_telescopePosition > 1137 && _telescopePosition < 1294) {
		uint16 imageId = _telescopeLighthouseOff;

		if (_state.generatorPowerAvailable == 1 && _telescopeLighthouseState)
			imageId = _telescopeLighthouseOn;

		Common::Rect src(1205, 0, 1205 + 131, 112);
		src.clip(Common::Rect(_telescopePosition, 0, _telescopePosition + 112, 112));
		src.translate(-1205, 0);
		src.clip(131, 112);

		Common::Rect dest(_telescopePosition, 0, _telescopePosition + 112, 112);
		dest.clip(Common::Rect(1205, 0, 1205 + 131, 112));
		dest.translate(-_telescopePosition, 0);
		dest.clip(112, 112);
		dest.translate(222, 112);

		_vm->_gfx->copyImageSectionToScreen(imageId, src, dest);
	}
}

void Stoneship::o_achenarDrawers_init(uint16 var, const ArgumentsArray &args) {
	// Used for Card 2004 (Achenar's Room Drawers)
	if (!_chestAchenarBottomDrawerClosed) {
		uint16 count1 = args[0];
		for (uint16 i = 0; i < count1; i++) {
			debugC(kDebugScript, "Disable hotspot index %d", args[i + 1]);
			_vm->getCard()->setResourceEnabled(args[i + 1], false);
		}
		uint16 count2 = args[count1 + 1];
		for (uint16 i = 0; i < count2; i++) {
			debugC(kDebugScript, "Enable hotspot index %d", args[i + count1 + 2]);
			_vm->getCard()->setResourceEnabled(args[i + count1 + 2], true);
		}
	}
}

void Stoneship::o_cloudOrb_init(uint16 var, const ArgumentsArray &args) {
	_cloudOrbMovie = getInvokingResource<MystAreaVideo>();
	_cloudOrbSound = args[0];
	_cloudOrbStopSound = args[1];
}

} // End of namespace MystStacks
} // End of namespace Mohawk
