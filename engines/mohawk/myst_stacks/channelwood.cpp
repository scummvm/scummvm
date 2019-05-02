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
#include "mohawk/myst_stacks/channelwood.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {
namespace MystStacks {

Channelwood::Channelwood(MohawkEngine_Myst *vm) :
		MystScriptParser(vm, kChannelwoodStack),
		_state(vm->_gameState->_channelwood),
		_valveVar(0),
		_siriusDrawerState(0),
		_doorOpened(0),
		_leverPulled(false),
		_leverAction(nullptr) {
	setupOpcodes();
}

Channelwood::~Channelwood() {
}

void Channelwood::setupOpcodes() {
	// "Stack-Specific" Opcodes
	REGISTER_OPCODE(100, Channelwood, o_bridgeToggle);
	REGISTER_OPCODE(101, Channelwood, o_pipeExtend);
	REGISTER_OPCODE(102, Channelwood, o_drawImageChangeCardAndVolume);
	REGISTER_OPCODE(104, Channelwood, o_waterTankValveOpen);
	REGISTER_OPCODE(105, Channelwood, o_leverStartMove);
	REGISTER_OPCODE(106, Channelwood, o_leverEndMove);
	REGISTER_OPCODE(107, Channelwood, o_leverMoveFail);
	REGISTER_OPCODE(108, Channelwood, o_leverMove);
	REGISTER_OPCODE(109, Channelwood, o_stairsDoorToggle);
	REGISTER_OPCODE(110, Channelwood, o_valveHandleMove1);
	REGISTER_OPCODE(111, Channelwood, o_valveHandleMoveStart1);
	REGISTER_OPCODE(112, Channelwood, o_valveHandleMoveStop);
	REGISTER_OPCODE(113, Channelwood, o_valveHandleMove2);
	REGISTER_OPCODE(114, Channelwood, o_valveHandleMoveStart2);
	REGISTER_OPCODE(115, Channelwood, o_valveHandleMove3);
	REGISTER_OPCODE(116, Channelwood, o_valveHandleMoveStart3);
	REGISTER_OPCODE(117, Channelwood, o_hologramMonitor);
	REGISTER_OPCODE(118, Channelwood, o_drawerOpen);
	REGISTER_OPCODE(119, Channelwood, o_hologramTemple);
	REGISTER_OPCODE(120, Channelwood, o_leverElev3StartMove);
	REGISTER_OPCODE(121, Channelwood, o_leverElev3EndMove);
	REGISTER_OPCODE(122, Channelwood, o_waterTankValveClose);
	REGISTER_OPCODE(123, Channelwood, o_executeMouseUp);
	REGISTER_OPCODE(124, Channelwood, o_leverEndMoveWithSound);
	REGISTER_OPCODE(125, Channelwood, o_pumpLeverMove);
	REGISTER_OPCODE(126, Channelwood, o_pumpLeverEndMove);
	REGISTER_OPCODE(127, Channelwood, o_elevatorMovies);
	REGISTER_OPCODE(128, Channelwood, o_leverEndMoveResumeBackground);
	REGISTER_OPCODE(129, Channelwood, o_soundReplace);

	// "Init" Opcodes
	REGISTER_OPCODE(201, Channelwood, o_lever_init);
	REGISTER_OPCODE(202, Channelwood, o_pipeValve_init);
	REGISTER_OPCODE(203, Channelwood, o_drawer_init);

	// "Exit" Opcodes
	REGISTER_OPCODE(300, Channelwood, NOP);
}

void Channelwood::disablePersistentScripts() {

}

void Channelwood::runPersistentScripts() {

}

uint16 Channelwood::getVar(uint16 var) {
	switch(var) {
	case 1: // Water Pump Bridge Raised
		return _state.waterPumpBridgeState;
	case 2: // Lower Walkway to Upper Walkway Elevator Raised
		return _state.elevatorState;
	case 3: // Water Flowing To Lower Walkway To Upper Walkway Elevator
		return (_state.waterValveStates & 0xc0) == 0xc0;
	case 4: // Water Flowing To Book Room Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb0 && _state.pipeState) ? 1 : 0;
	case 5: // Lower Walkway to Upper Walkway Spiral Stair Lower Door State
		return _state.stairsLowerDoorState;
	case 6: // Pipe Bridge Extended
		return _state.pipeState;
	case 7: // Water Flowing To Water Pump For Bridge
		return ((_state.waterValveStates & 0xe2) == 0x82 || (_state.waterValveStates & 0xf4) == 0xa0) ? 1 : 0;
	case 8: // Water Tank Valve
		return (_state.waterValveStates & 0x80) ? 1 : 0;
	case 9: // State of First Water Valve
		return (_state.waterValveStates & 0x40) ? 1 : 0;
	case 10: // State of Second Water Valve
		return (_state.waterValveStates & 0x20) ? 1 : 0;
	case 11: // State of Right Third Water Valve
		return (_state.waterValveStates & 0x10) ? 1 : 0;
	case 12: // State of Right-Right Fourth Water Valve
		return (_state.waterValveStates & 0x08) ? 1 : 0;
	case 13: // State of Right-Left Fourth Water Valve
		return (_state.waterValveStates & 0x04) ? 1 : 0;
	case 14: // State of Left Third Water Valve
		return (_state.waterValveStates & 0x02) ? 1 : 0;
	case 15: // Water Flowing To Lower Walkway To Upper Walkway Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb8) ? 1 : 0;
	case 16: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		return _state.stairsUpperDoorState;
	case 17: // Achenar's Holoprojector Selection
		return _state.holoprojectorSelection;
	case 18: // Sirrus's Room Bed Drawer Open
		return _siriusDrawerState;
	case 19: // Sound - Water Tank Valve
		return (_state.waterValveStates & 0x80) ? 1 : 0;
	case 20: // Sound - First Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xc0) == 0x80) ? 1 : 0;
	case 21: // Sound - Second Water Valve Water Flowing To Right
		return ((_state.waterValveStates & 0xe0) == 0xa0) ? 1 : 0;
	case 22: // Sound - Right Third Water Valve Water Flowing To Right
		return ((_state.waterValveStates & 0xf0) == 0xb0) ? 1 : 0;
	case 23: // Sound - Right Third Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xf0) == 0xa0) ? 1 : 0;
	case 24: // Sound - Second Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xe0) == 0x80) ? 1 : 0;
	case 25: // Sound - Right-Right Fourth Valve Water Flowing To Left (To Pipe Bridge)
		return ((_state.waterValveStates & 0xf8) == 0xb0) ? 1 : 0;
	case 26: // Sound - Right-Left Fourth Valve Water Flowing To Right (To Pipe Down Tree)
		return ((_state.waterValveStates & 0xf4) == 0xa4) ? 1 : 0;
	case 27: // Sound - Right-Left Fourth Valve Water Flowing To Left (To Pipe Fork)
		return ((_state.waterValveStates & 0xf4) == 0xa0) ? 1 : 0;
	case 28: // Sound - Left Third Water Valve Flowing To Right (To Pipe Fork)
		return ((_state.waterValveStates & 0xe2) == 0x82) ? 1 : 0;
	case 29: // Sound - Left Third Water Valve Flowing To Left (To Pipe In Water)
		return ((_state.waterValveStates & 0xe2) == 0x80) ? 1 : 0;
	case 30: // Door State
		return _doorOpened;
	case 31: // Water flowing in pipe fork ?
		// 0 -> keep sound.
		// 1 -> not flowing.
		// 2 --> flowing.
		if ((_state.waterValveStates & 0xe2) == 0x82)	// From left.
			return 2;
		if ((_state.waterValveStates & 0xf4) == 0xa0)	// From right.
			return 1;

		return 0;
	case 32: // Sound - Water Flowing in Pipe to Book Room Elevator
		if ((_state.waterValveStates & 0xf8) == 0xb0)
			return _state.pipeState ? 2 : 1;

		return 0;
	case 33: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		if (_state.stairsUpperDoorState) {
			if (_tempVar == 1)
				return 2;
			else
				return 1;
		} else {
			return 0;
		}
	case 102: // Sirrus's Desk Drawer / Red Page State
		if (_siriusDrawerState) {
			if(!(_globals.redPagesInBook & 16) && (_globals.heldPage != kRedChannelwoodPage))
				return 2; // Drawer Open, Red Page Present
			else
				return 1; // Drawer Open, Red Page Taken
		} else {
			return 0; // Drawer Closed
		}
	case 103: // Blue Page Present
		return !(_globals.bluePagesInBook & 16) && (_globals.heldPage != kBlueChannelwoodPage);
	default:
		return MystScriptParser::getVar(var);
	}
}

void Channelwood::toggleVar(uint16 var) {
	switch(var) {
	case 1: // Water Pump Bridge Raised
		_state.waterPumpBridgeState ^= 1;
		break;
	case 6: // Pipe Bridge Extended
		_state.pipeState ^= 1;
		break;
	case 16: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		_state.stairsUpperDoorState ^= 1;
		break;
	case 102: // Red page
		if (!(_globals.redPagesInBook & 16)) {
			if (_globals.heldPage == kRedChannelwoodPage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kRedChannelwoodPage;
		}
		break;
	case 103: // Blue page
		if (!(_globals.bluePagesInBook & 16)) {
			if (_globals.heldPage == kBlueChannelwoodPage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kBlueChannelwoodPage;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool Channelwood::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 2: // Lower Walkway to Upper Walkway Elevator Raised
		if (_state.elevatorState != value) {
			_state.elevatorState = value;
			refresh = true;
		}
		break;
	case 5: // Lower Walkway to Upper Walkway Spiral Stair Lower Door State
		if (_state.stairsLowerDoorState != value) {
			_state.stairsLowerDoorState = value;
			refresh = true;
		}
		break;
	case 9:
		refresh = pipeChangeValve(value, 0x40);
		break;
	case 10:
		refresh = pipeChangeValve(value, 0x20);
		break;
	case 11:
		refresh = pipeChangeValve(value, 0x10);
		break;
	case 12:
		refresh = pipeChangeValve(value, 0x08);
		break;
	case 13:
		refresh = pipeChangeValve(value, 0x04);
		break;
	case 14:
		refresh = pipeChangeValve(value, 0x02);
		break;
	case 18: // Sirrus's Room Bed Drawer Open
		if (_siriusDrawerState != value) {
			_siriusDrawerState = value;
			refresh = true;
		}
		break;
	case 30: // Door opened
		_doorOpened = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

bool Channelwood::pipeChangeValve(bool open, uint16 mask) {
	if (open) {
		if (!(_state.waterValveStates & mask)) {
			_state.waterValveStates |= mask;
			return true;
		}
	} else {
		if (_state.waterValveStates & mask) {
			_state.waterValveStates &= ~mask;
			return true;
		}
	}

	return false;
}

void Channelwood::o_bridgeToggle(uint16 var, const ArgumentsArray &args) {
	VideoEntryPtr bridge = _vm->playMovie("bridge", kChannelwoodStack);
	bridge->moveTo(292, 203);

	// Toggle bridge state
	if (_state.waterPumpBridgeState)
		bridge->setBounds(Audio::Timestamp(0, 3050, 600), Audio::Timestamp(0, 6100, 600));
	else
		bridge->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 3050, 600));

	_vm->waitUntilMovieEnds(bridge);
}

void Channelwood::o_pipeExtend(uint16 var, const ArgumentsArray &args) {
	uint16 soundId = args[0];
	debugC(kDebugScript, "\tsoundId: %d", soundId);

	_vm->_sound->playEffect(soundId);
	VideoEntryPtr pipe = _vm->playMovie("pipebrid", kChannelwoodStack);
	pipe->moveTo(267, 170);

	// Toggle pipe state
	if (_state.pipeState)
		pipe->setBounds(Audio::Timestamp(0, 3040, 600), Audio::Timestamp(0, 6080, 600));
	else
		pipe->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 3040, 600));

	_vm->waitUntilMovieEnds(pipe);
	_vm->_sound->resumeBackground();
}

void Channelwood::o_drawImageChangeCardAndVolume(uint16 var, const ArgumentsArray &args) {
	uint16 imageId = args[0];
	uint16 cardId = args[1];
	uint16 volume = args.size() == 3 ? args[2] : 0;

	debugC(kDebugScript, "\timageId: %d", imageId);
	debugC(kDebugScript, "\tcardId: %d", cardId);

	_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
	_vm->wait(200);

	_vm->changeToCard(cardId, kTransitionPartToLeft);

	if (volume) {
		_vm->_sound->changeBackgroundVolume(volume);
	}
}


void Channelwood::o_waterTankValveOpen(uint16 var, const ArgumentsArray &args) {
	Common::Rect rect = getInvokingResource<MystArea>()->getRect();

	for (uint i = 0; i < 2; i++)
		for (uint16 imageId = 3601; imageId >= 3595; imageId--) {
			_vm->_gfx->copyImageToScreen(imageId, rect);
			_vm->doFrame();
		}

	pipeChangeValve(true, 0x80);
}

void Channelwood::o_leverStartMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
	lever->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_leverPulled = false;
}

void Channelwood::o_leverMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	if (lever->pullLeverV()) {
		if (!_leverPulled) {
			_leverPulled = true;
			_leverAction->handleMouseUp();
		}
	} else {
		_leverPulled = false;
	}
}

void Channelwood::o_leverMoveFail(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	if (lever->pullLeverV()) {
		if (!_leverPulled) {
			_leverPulled = true;
			uint16 soundId = lever->getList2(0);
			if (soundId)
				_vm->_sound->playEffect(soundId);
		}
	} else {
		_leverPulled = false;
	}
}

void Channelwood::o_leverEndMove(uint16 var, const ArgumentsArray &args) {
	// Get current lever frame
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	// Release lever
	lever->releaseLeverV();

	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);

	_vm->refreshCursor();
}

void Channelwood::o_leverEndMoveResumeBackground(uint16 var, const ArgumentsArray &args) {
	_vm->_sound->resumeBackground();
	o_leverEndMove(var, args);
}

void Channelwood::o_leverEndMoveWithSound(uint16 var, const ArgumentsArray &args) {
	o_leverEndMove(var, args);

	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);
}

void Channelwood::o_leverElev3StartMove(uint16 var, const ArgumentsArray &args) {
	_vm->_gfx->copyImageToScreen(3970, Common::Rect(544, 333));
	_vm->doFrame();
	o_leverStartMove(var, args);
}

void Channelwood::o_leverElev3EndMove(uint16 var, const ArgumentsArray &args) {
	o_leverEndMove(var, args);
	_vm->_gfx->copyImageToScreen(3265, Common::Rect(544, 333));
	_vm->doFrame();
	_vm->_sound->playEffect(5265);
}

void Channelwood::o_pumpLeverMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	if (lever->pullLeverV()) {
		uint16 soundId = lever->getList2(0);
		_vm->_sound->playBackground(soundId, 38400);
	} else {
		uint16 soundId = lever->getList2(1);
		_vm->_sound->playBackground(soundId, 36864);
	}
}

void Channelwood::o_pumpLeverEndMove(uint16 var, const ArgumentsArray &args) {
	o_leverEndMove(var, args);

	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->playBackground(soundId, 36864);
}

void Channelwood::o_stairsDoorToggle(uint16 var, const ArgumentsArray &args) {
	MystAreaVideo *movie = getInvokingResource<MystAreaVideo>();

	if (_state.stairsUpperDoorState) {
		// Close door, play the open movie backwards
		movie->setDirection(-1);
		movie->playMovie();
	} else {
		// Open door
		movie->setDirection(1);
		movie->playMovie();
	}
}

void Channelwood::o_valveHandleMove1(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		// Compute frame to draw
		_tempVar = (mouse.x - 250) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void Channelwood::o_valveHandleMoveStart1(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove1(var, args);
}

void Channelwood::o_valveHandleMoveStop(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();

	// Update state with valve position
	if (_tempVar <= 5)
		setVarValue(_valveVar, 1);
	else
		setVarValue(_valveVar, 0);

	// Play release sound
	uint16 soundId = handle->getList3(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);

	// Redraw valve
	_vm->getCard()->redrawArea(_valveVar);

	// Restore cursor
	_vm->refreshCursor();
}

void Channelwood::o_valveHandleMove2(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		// Compute frame to draw
		_tempVar = handle->getStepsH() - (mouse.x - 234) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void Channelwood::o_valveHandleMoveStart2(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove2(var, args);
}

void Channelwood::o_valveHandleMove3(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		// Compute frame to draw
		_tempVar = handle->getStepsH() - (mouse.x - 250) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void Channelwood::o_valveHandleMoveStart3(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->playEffect(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove3(var, args);
}

void Channelwood::o_hologramMonitor(uint16 var, const ArgumentsArray &args) {
	// Used on Card 3012 (Temple Hologram Monitor)
	uint16 button = args[0]; // 0 to 3

	if (_state.holoprojectorSelection != button || !_vm->_video->isVideoPlaying()) {
		_state.holoprojectorSelection = button;
		_vm->getCard()->redrawArea(17);

		_vm->_video->stopVideos();

		VideoEntryPtr video;

		switch (button) {
		case 0:
			video = _vm->playMovie("monalgh", kChannelwoodStack);
			video->moveTo(227, 70);
			break;
		case 1:
			video = _vm->playMovie("monamth", kChannelwoodStack);
			video->moveTo(227, 70);
			break;
		case 2:
			video = _vm->playMovie("monasirs", kChannelwoodStack);
			video->moveTo(227, 70);
			break;
		case 3:
			video = _vm->playMovie("monsmsg", kChannelwoodStack);
			video->moveTo(226, 68);
			break;
		default:
			warning("Opcode o_hologramMonitor Control Variable Out of Range");
			break;
		}
	}
}

void Channelwood::o_drawerOpen(uint16 var, const ArgumentsArray &args) {
	_siriusDrawerState = 1;
	_vm->getCard()->redrawArea(18, false);
	_vm->getCard()->redrawArea(102, false);
}

void Channelwood::o_hologramTemple(uint16 var, const ArgumentsArray &args) {
	_vm->_sound->pauseBackground();

	// Used on Card 3333 (Temple Hologram)
	switch (_state.holoprojectorSelection) {
	case 0:
		_vm->playMovieBlocking("holoalgh", kChannelwoodStack, 139, 64);
		break;
	case 1:
		_vm->playMovieBlocking("holoamth", kChannelwoodStack, 127, 73);
		break;
	case 2:
		_vm->playMovieBlocking("holoasir", kChannelwoodStack, 139, 64);
		break;
	case 3:
		_vm->playMovieBlocking("holosmsg", kChannelwoodStack, 127, 45);
		break;
	default:
		warning("Opcode o_hologramTemple Control Variable Out of Range");
		break;
	}

	_vm->_sound->resumeBackground();
}

void Channelwood::o_executeMouseUp(uint16 var, const ArgumentsArray &args) {
	// Clear the clicked resource so the mouse up event is not called a second time.
	_vm->getCard()->resetClickedResource();

	MystArea *resource = _vm->getCard()->getResource<MystArea>(args[0]);
	resource->handleMouseUp();
}

void Channelwood::o_waterTankValveClose(uint16 var, const ArgumentsArray &args) {
	Common::Rect rect = getInvokingResource<MystArea>()->getRect();

	for (uint i = 0; i < 2; i++)
		for (uint16 imageId = 3595; imageId <= 3601; imageId++) {
			_vm->_gfx->copyImageToScreen(imageId, rect);
			_vm->doFrame();
		}

	pipeChangeValve(false, 0x80);
}

void Channelwood::o_elevatorMovies(uint16 var, const ArgumentsArray &args) {
	// Used by Card 3262 (Elevator)
	uint16 elevator = args[0];
	uint16 direction = args[1];

	Common::String movie;
	uint16 x;
	uint16 y;

	switch (elevator) {
	case 1:
		x = 214;
		y = 106;
		if (direction == 1)
			movie = "welev1up";
		else
			movie = "welev1dn";
		break;
	case 2:
		x = 215;
		y = 117;
		if (direction == 1)
			movie = "welev2up";
		else
			movie = "welev2dn";
		break;
	case 3:
		x = 213;
		y = 98;
		if (direction == 1)
			movie = "welev3up";
		else
			movie = "welev3dn";
		break;
	default:
		error("Unknown elevator state %d in o_elevatorMovies", elevator);
	}

	_vm->_sound->pauseBackground();
	_vm->playMovieBlocking(movie, kChannelwoodStack, x, y);
	_vm->_sound->resumeBackground();
}

void Channelwood::o_soundReplace(uint16 var, const ArgumentsArray &args) {
	uint16 soundId = args[0];

	if (!_vm->_sound->isEffectPlaying()) {
		_vm->_sound->playEffect(soundId);
	}
}

void Channelwood::o_lever_init(uint16 var, const ArgumentsArray &args) {
	_leverAction = getInvokingResource<MystArea>();
}

void Channelwood::o_pipeValve_init(uint16 var, const ArgumentsArray &args) {
	_valveVar = var;
}

void Channelwood::o_drawer_init(uint16 var, const ArgumentsArray &args) {
	_siriusDrawerState = 0;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
