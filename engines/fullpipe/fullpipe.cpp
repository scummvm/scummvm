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
 */

#include "base/plugins.h"

#include "common/archive.h"
#include "common/config-manager.h"

#include "engines/util.h"

#include "fullpipe/fullpipe.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/objects.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/messagequeue.h"

namespace Fullpipe {

FullpipeEngine *g_fullpipe = 0;

FullpipeEngine::FullpipeEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource("fullpipe");

	_gameProjectVersion = 0;
	_pictureScale = 8;
	_scrollSpeed = 0;
	_currSoundListCount = 0;

	_currArchive = 0;

	_soundEnabled = true;
	_flgSoundList = true;

	_inputController = 0;
	_inputDisabled = false;

	_needQuit = false;

	_aniMan = 0;
	_aniMan2 = 0;
	_currentScene = 0;
	_scene2 = 0;

	_globalMessageQueueList = 0;

	g_fullpipe = this;
}

FullpipeEngine::~FullpipeEngine() {
	delete _rnd;
	delete _globalMessageQueueList;
}

void FullpipeEngine::initialize() {
	_globalMessageQueueList = new GlobalMessageQueueList;
}

Common::Error FullpipeEngine::run() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	// Initialize backend
	initGraphics(800, 600, true, &format);

	_backgroundSurface.create(800, 600, format);

	initialize();

	_isSaveAllowed = false;

	loadGam("fullpipe.gam");

	return Common::kNoError;
}

void FullpipeEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyState = event.kbd.keycode;
			break;
		case Common::EVENT_KEYUP:
			_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_QUIT:
			_needQuit = true;
			break;
		default:
			break;
		}
	}
}

void FullpipeEngine::initObjectStates() {
	setLevelStates();

	setObjectState(sO_Dude, getObjectEnumState(sO_Dude, sO_NotCarryingEgg));
	setObjectState(sO_EggCracker, getObjectEnumState(sO_EggCracker, sO_NotCrackedEggs));
	setObjectState(sO_GuvTheDrawer, getObjectEnumState(sO_GuvTheDrawer, sO_Awaken));
	setObjectState(sO_EggGulper, getObjectEnumState(sO_EggGulper, sO_First));
	setObjectState(sO_EggGulperGaveCoin, getObjectEnumState(sO_EggGulperGaveCoin, sO_No));
	setObjectState(sO_Jar_4, getObjectEnumState(sO_Jar_4, sO_OnTheSpring));
	setObjectState(sO_GulpedEggs, getObjectEnumState(sO_GulpedEgg, sO_NotPresent));

	setSwallowedEggsState();

	setObjectState(sO_WeirdWacko, getObjectEnumState(sO_WeirdWacko, sO_InGlasses));
	setObjectState(sO_TumyTrampie, getObjectEnumState(sO_TumyTrampie, sO_Drinking));
	setObjectState(sO_StairsUp_8, getObjectEnumState(sO_StairsUp_8, sO_NotBroken));
	setObjectState(sO_HareTheNooksiter, getObjectEnumState(sO_HareTheNooksiter, sO_WithHandle));
	setObjectState(sO_Elephantine, getObjectEnumState(sO_Elephantine, sO_WithBoot));
	setObjectState(sO_Fly_12, 0);
	setObjectState(sO_ClockAxis, getObjectEnumState(sO_ClockAxis, sO_NotAvailable));
	setObjectState(sO_ClockHandle, getObjectEnumState(sO_ClockHandle, sO_In_7));
	setObjectState(sO_BigMumsy, getObjectEnumState(sO_BigMumsy, sO_Sleeping));
	setObjectState(sO_CoinSlot_1, getObjectEnumState(sO_CoinSlot_1, sO_Empty));
	setObjectState(sO_FriesPit, getObjectEnumState(sO_FriesPit, sO_WithApple));
	setObjectState(sO_Jug, getObjectEnumState(sO_Jug, sO_Blocked));
	setObjectState(sO_RightStairs_9, getObjectEnumState(sO_RightStairs_9, sO_ClosedShe));
	setObjectState(sO_Pipe_9, getObjectEnumState(sO_Pipe_9, sO_WithJug));
	setObjectState(sO_Inflater, getObjectEnumState(sO_Inflater, sO_WithGum));
	setObjectState(sO_Swingie, getObjectEnumState(sO_Swingie, sO_Swinging));
	setObjectState(sO_DudeJumped, getObjectEnumState(sO_DudeJumped, sO_No));
	setObjectState(sO_Bridge, getObjectEnumState(sO_Bridge, sO_Convoluted));
	setObjectState(sO_Guardian, getObjectEnumState(sO_Guardian, sO_OnRight));
	setObjectState(sO_Grandma, getObjectEnumState(sO_Grandma, sO_In_14));
	setObjectState(sO_Boot_15, getObjectEnumState(sO_Boot_15, sO_NotPresent));
	setObjectState(sO_LeftPipe_15, getObjectEnumState(sO_LeftPipe_15, sO_OpenedShe));
	setObjectState(sO_Pedestal_16, getObjectEnumState(sO_Pedestal_16, sO_IsFree));
	setObjectState(sO_Cup, getObjectEnumState(sO_Cup, sO_InSmokeRoom));
	setObjectState(sO_Pedestal_17, getObjectEnumState(sO_Pedestal_17, sO_IsFree));
	setObjectState(sO_UsherHand, getObjectEnumState(sO_UsherHand, sO_WithoutCoin));
	setObjectState(sO_RightPipe_17, getObjectEnumState(sO_RightPipe_17, sO_ClosedShe));
	setObjectState(sO_Fly_17, 1);
	setObjectState(sO_DudeSwinged, 0);
	setObjectState(sO_Girl, getObjectEnumState(sO_Girl, sO_Swinging));
	setObjectState(sO_Sugar, getObjectEnumState(sO_Sugar, sO_Present));
	setObjectState(sO_Janitors, getObjectEnumState(sO_Janitors, sO_Together));
	setObjectState(sO_Bag_22, getObjectEnumState(sO_Bag_22, sO_NotFallen));
	setObjectState(sO_Grandpa, getObjectEnumState(sO_Grandpa, sO_InSock));
	setObjectState(sO_CoinSlot_22, getObjectEnumState(sO_CoinSlot_22, sO_Empty));
	setObjectState(sO_UpperHatch_23, getObjectEnumState(sO_UpperHatch_23, sO_Closed));
	setObjectState(sO_LowerHatch_23, getObjectEnumState(sO_LowerHatch_23, sO_Closed));
	setObjectState(sO_Lever_23, getObjectEnumState(sO_Lever_23, sO_NotTaken));
	setObjectState(sO_LeverHandle_23, getObjectEnumState(sO_LeverHandle_23, sO_WithoutStool));
	setObjectState(sO_LowerPipe_21, getObjectEnumState(sO_LowerPipe_21, sO_ClosedShe));
	setObjectState(sO_StarsDown_24, getObjectEnumState(sO_StarsDown_24, sO_OpenedShe));
	setObjectState(sO_Hatch_26, getObjectEnumState(sO_Hatch_26, sO_Closed));
	setObjectState(sO_Sock_26, getObjectEnumState(sO_Sock_26, sO_NotHanging));
	setObjectState(sO_LeftPipe_26, getObjectEnumState(sO_LeftPipe_26, sO_ClosedShe));
	setObjectState(sO_Valve1_26, getObjectEnumState(sO_Valve1_26, sO_Opened));
	setObjectState(sO_Valve2_26, getObjectEnumState(sO_Valve2_26, sO_Closed));
	setObjectState(sO_Valve3_26, getObjectEnumState(sO_Valve3_26, sO_Closed));
	setObjectState(sO_Valve4_26, getObjectEnumState(sO_Valve4_26, sO_Closed));
	setObjectState(sO_Valve5_26, getObjectEnumState(sO_Valve5_26, sO_Opened));
	setObjectState(sO_Pool, getObjectEnumState(sO_Pool, sO_Overfull));
	setObjectState(sO_Plank_25, getObjectEnumState(sO_Plank_25, sO_NearDudesStairs));
	setObjectState(sO_Driver, getObjectEnumState(sO_Driver, sO_WithSteering));
	setObjectState(sO_Janitress, getObjectEnumState(sO_Janitress, sO_WithMop));
	setObjectState(sO_LeftPipe_29, getObjectEnumState(sO_LeftPipe_29, sO_ClosedShe));
	setObjectState(sO_LeftPipe_30, getObjectEnumState(sO_LeftPipe_30, sO_ClosedShe));
	setObjectState(sO_Leg, getObjectEnumState(sO_Leg, sO_ShowingHeel));
	setObjectState(sO_Tub, getObjectEnumState(sO_Tub, sO_EmptyShe));
	setObjectState(sO_Cactus, getObjectEnumState(sO_Cactus, sO_NotGrown));
	setObjectState(sO_Fireman, getObjectEnumState(sO_Fireman, sO_WithHose));
	setObjectState(sO_Cube, getObjectEnumState(sO_Cube, sO_In_33));
	setObjectState(sO_MommyOfHandle_32, getObjectEnumState(sO_MommyOfHandle_32, sO_WithoutHandle));
	setObjectState(sO_Pedestal_33, getObjectEnumState(sO_Pedestal_33, sO_IsFree));
	setObjectState(sO_Valve_34, getObjectEnumState(sO_Valve_34, sO_WithNothing));
	setObjectState(sO_Stool_34, getObjectEnumState(sO_Stool_34, sO_WithoutDrawer));
	setObjectState(sO_Plank_34, getObjectEnumState(sO_Plank_34, sO_Passive));
	setObjectState(sO_Hatch_34, getObjectEnumState(sO_Hatch_34, sO_Closed));
	setObjectState(sO_Valve_35, getObjectEnumState(sO_Valve_35, sO_TurnedOff));
	setObjectState(sO_Carpet_35, getObjectEnumState(sO_Carpet_35, sO_CannotTake));
	setObjectState(sO_CoinSlot_35, getObjectEnumState(sO_CoinSlot_35, sO_WithCoin));
	setObjectState(sO_BellyInflater, getObjectEnumState(sO_BellyInflater, sO_WithCork));
	setObjectState(sO_Jawcrucnher, getObjectEnumState(sO_Jawcrucnher, sO_WithoutCarpet));
	setObjectState(sO_Guard_1, getObjectEnumState(sO_Guard_1, sO_On));
	setObjectState(sO_Gurad_2, getObjectEnumState(sO_Gurad_2, sO_On));
	setObjectState(sO_Guard_3, getObjectEnumState(sO_Guard_3, sO_On));
	setObjectState(sO_Bottle_38, getObjectEnumState(sO_Bottle_38, sO_OnTheTable));
	setObjectState(sO_Boss, getObjectEnumState(sO_Boss, sO_WithHammer));
}

void FullpipeEngine::setLevelStates() {
	CGameVar *v = _gameLoader->_gameVar->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (v) {
		v->setSubVarAsInt(sO_Level0, 2833);
		v->setSubVarAsInt(sO_Level1, 2754);
		v->setSubVarAsInt(sO_Level2, 2757);
		v->setSubVarAsInt(sO_Level3, 2760);
		v->setSubVarAsInt(sO_Level4, 2763);
		v->setSubVarAsInt(sO_Level5, 2766);
		v->setSubVarAsInt(sO_Level6, 2769);
		v->setSubVarAsInt(sO_Level7, 2772);
		v->setSubVarAsInt(sO_Level8, 2775);
		v->setSubVarAsInt(sO_Level9, 2778);
	}
}

void FullpipeEngine::setSwallowedEggsState() {
	CGameVar *v = _gameLoader->_gameVar->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	_swallowedEgg1 = v->getSubVarByName(sO_Egg1);
	_swallowedEgg2 = v->getSubVarByName(sO_Egg2);
	_swallowedEgg3 = v->getSubVarByName(sO_Egg3);

	_swallowedEgg1->_value.intValue = 0;
	_swallowedEgg2->_value.intValue = 0;
	_swallowedEgg3->_value.intValue = 0;
}

int FullpipeEngine::getObjectEnumState(const char *name, const char *state) {
	CGameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (!var) {
		var = _gameLoader->_gameVar->addSubVarAsInt("OBJSTATES", 0);
	}

	var = var->getSubVarByName(name);
	if (var) {
		var = var->getSubVarByName("ENUMSTATES");
		if (var)
			return var->getSubVarAsInt(state);
	}

	return 0;
}

void FullpipeEngine::setObjectState(const char *name, int state) {
	CGameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (!var) {
		var = _gameLoader->_gameVar->addSubVarAsInt("OBJSTATES", 0);
	}

	var->setSubVarAsInt(name, state);
}

} // End of namespace Fullpipe
