/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/rect.h"

#include "mohawk/mohawk.h"
#include "mohawk/resource.h"
#include "mohawk/sound.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/transition_xfer.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

ZoombiniTransitionXfer::ZoombiniTransitionXfer(MohawkEngine_Zoombini *vm) : ZoombiniTransition(vm, ZoombiniPageType::kXfer) {
	_useFadeEffect = true;
}

ZoombiniTransitionXfer::~ZoombiniTransitionXfer() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniTransitionXfer::getScriptSoundPriorityRanges() const {
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
	};
	return kRanges;
}

void ZoombiniTransitionXfer::open() {
	openArchive(ZMB_MHK_XFER);
}

void ZoombiniTransitionXfer::computeXferRoute() {
	// Determine xfer view (0-5) from source SI page.
	// Each page records its source SI page first.
	// It then calls @ref MohawkEngine_Zoombini::setNextPage() with @ref ZoombiniPageType::kXfer.
	ZmbSrcPageKind src = _vm->_xferSrcPage;
	const ZmbXferRouteInfo *routeInfo = ZmbXferRouteInfo::getZmbXferRouteInfo(src);
	if (!routeInfo) {
		error("xfer: unknown source page kind %d", static_cast<int>(src));
		return;
	}
	_nextPageType = static_cast<ZoombiniPageType>(routeInfo->destPage);
	_vm->_state->markXferContainerArrival(src, *routeInfo);

	switch (src) {
	case ZmbSrcPageKind::kPicker_01: // Picker -> Bridge (Route from Picker)
		_xferView = XferSceneId::kScene0_FromIsle;
		_xferBackgroundResId = kResBackground5000_Route0_FromIsle;
		_xferShapesId = kResShapes5100_Route0_FromIsle;
		_xferScrbCount = 9;
		break;
	case ZmbSrcPageKind::kBridge_02: // Bridge -> Tunnels (Route 1 Path 2)
		_xferView = XferSceneId::kScene1_BigBadHungry;
		_xferBackgroundResId = kResBackground1000_Route1_BigBadHungry;
		_xferShapesId = kResShapes1100_Route1_BigBadHungry;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kTunnels_03: // Tunnels -> Pizza (Route 1 Path 3)
		_xferView = XferSceneId::kScene1_BigBadHungry;
		_xferBackgroundResId = kResBackground1000_Route1_BigBadHungry;
		_xferShapesId = kResShapes1100_Route1_BigBadHungry;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kPizza_04: // Pizza -> Basecamp 1 (Route 1 Path 4)
		_xferView = XferSceneId::kScene1_BigBadHungry;
		_xferBackgroundResId = kResBackground1000_Route1_BigBadHungry;
		_xferShapesId = kResShapes1100_Route1_BigBadHungry;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kBC1North_05: // Basecamp 1 north exit -> Ferry (Route 2 Path 1)
		_xferView = XferSceneId::kScene2_WhosBayou;
		_xferBackgroundResId = kResBackground2000_Route2_WhosBayou;
		_xferShapesId = kResShapes2100_Route2_WhosBayou;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kFerry_07: // Ferry -> Lilly (Route 2 Path 2)
		_xferView = XferSceneId::kScene2_WhosBayou;
		_xferBackgroundResId = kResBackground2000_Route2_WhosBayou;
		_xferShapesId = kResShapes2100_Route2_WhosBayou;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kLilly_08: // Lilly -> Slides (Route 2 Path 3)
		_xferView = XferSceneId::kScene2_WhosBayou;
		_xferBackgroundResId = kResBackground2000_Route2_WhosBayou;
		_xferShapesId = kResShapes2100_Route2_WhosBayou;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kSlides_09: // Slides -> Basecamp 2 (Route 2 Path 4)
		_xferView = XferSceneId::kScene2_WhosBayou;
		_xferBackgroundResId = kResBackground2000_Route2_WhosBayou;
		_xferShapesId = kResShapes2100_Route2_WhosBayou;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kBC1South_06: // Basecamp 1 south exit -> Fleens (Route 3 Path 1)
		_xferView = XferSceneId::kScene3_DeepDarkForest;
		_xferBackgroundResId = kResBackground3000_Route3_DeepDarkForest;
		_xferShapesId = kResShapes3100_Route3_DeepDarkForest;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kFleens_10: // Fleens -> Hotel (Route 3 Path 2)
		_xferView = XferSceneId::kScene3_DeepDarkForest;
		_xferBackgroundResId = kResBackground3000_Route3_DeepDarkForest;
		_xferShapesId = kResShapes3100_Route3_DeepDarkForest;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kHotel_11: // Hotel -> Net (Route 3 Path 3)
		_xferView = XferSceneId::kScene3_DeepDarkForest;
		_xferBackgroundResId = kResBackground3000_Route3_DeepDarkForest;
		_xferShapesId = kResShapes3100_Route3_DeepDarkForest;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kNet_12: // Net -> Basecamp 2 (Route 3 Path 4)
		_xferView = XferSceneId::kScene3_DeepDarkForest;
		_xferBackgroundResId = kResBackground3000_Route3_DeepDarkForest;
		_xferShapesId = kResShapes3100_Route3_DeepDarkForest;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kBC2_13: // Basecamp 2 -> Caves (Route 4 Path 1)
		_xferView = XferSceneId::kScene4_MountainOfDespair;
		_xferBackgroundResId = kResBackground4000_Route4_MountainOfDespair;
		_xferShapesId = kResShapes4100_Route4_MountainOfDespair;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kCaves_14: // Caves -> Smoke (Route 4 Path 2)
		_xferView = XferSceneId::kScene4_MountainOfDespair;
		_xferBackgroundResId = kResBackground4000_Route4_MountainOfDespair;
		_xferShapesId = kResShapes4100_Route4_MountainOfDespair;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kSmoke_15: // Smoke -> Maze (Route 4 Path 3)
		_xferView = XferSceneId::kScene4_MountainOfDespair;
		_xferBackgroundResId = kResBackground4000_Route4_MountainOfDespair;
		_xferShapesId = kResShapes4100_Route4_MountainOfDespair;
		_xferScrbCount = 3;
		break;
	case ZmbSrcPageKind::kMaze_16: // Maze -> Town (Route to Town)
		_xferView = XferSceneId::kScene5_ToTown;
		_xferBackgroundResId = kResBackground6000_Route5_ToTown;
		_xferShapesId = kResShapes6100_Route5_ToTown;
		_xferScrbCount = 9;
		break;
	default:
		error("xfer: unknown source SI page %d", static_cast<int>(src));
		break;
	}
}

// Select a route-level system PCM track that combines background audio with optional Narrator speech.
// No-voice variants remain in the same Speech mixer category as the narrated variants.
int16 ZoombiniTransitionXfer::selectXferSoundId() const {
	// Use a local page-flag copy so the SFX group lookup cannot increment the stored state counter.
	ZmbStateFile::PageFlag pageFlag = _vm->_state->getCurrentState().getPageFlagFromPageType(_nextPageType);
	const ZmbSfxGroupFlags sfxGroupFlags = _vm->_state->getSfxGroupFlagsFromPageFlag(pageFlag, _nextPageType);
	const int16 routeLevel = _vm->_state->readPageRouteLevel(_nextPageType) + 1;

	if (_xferView == XferSceneId::kScene0_FromIsle) { // XFER_0 - FROM ISLE (Bridge): SND 20094-20099
		switch (sfxGroupFlags) {
		case ZmbSfxGroupFlags::kRandom_00:
			if (2 <= routeLevel && routeLevel <= 3) {
				// Higher route level: 6 choices including hard voice
				switch (_vm->_rnd->getRandomNumber(1, 6)) {
				case 1:
					return kSysResSound20094_Route0_BridgeVoiceA;
				case 2:
					return kSysResSound20095_Route0_BridgeVoiceB;
				case 3:
					return kSysResSound20096_Route0_BridgeVoiceC;
				case 4:
					return kSysResSound20097_Route0_BridgeVoiceD;
				case 5:
					return kSysResSound20098_Route0_BridgeHard; // hard voice
				default:
					return kSysResSound20099_Route0_BridgeNoVoice; // no-voice
				}
			} else {
				// Low route level: 5 choices, no hard voice
				switch (_vm->_rnd->getRandomNumber(1, 5)) {
				case 1:
					return kSysResSound20094_Route0_BridgeVoiceA;
				case 2:
					return kSysResSound20095_Route0_BridgeVoiceB;
				case 3:
					return kSysResSound20096_Route0_BridgeVoiceC;
				case 4:
					return kSysResSound20097_Route0_BridgeVoiceD;
				default:
					return kSysResSound20099_Route0_BridgeNoVoice; // no-voice
				}
			}
			break;
		case ZmbSfxGroupFlags::kEasy_01:
			return kSysResSound20094_Route0_BridgeVoiceA;
		case ZmbSfxGroupFlags::kHardFirstTrigger_02:
		case ZmbSfxGroupFlags::kHardSecondTrigger_12:
			return kSysResSound20098_Route0_BridgeHard; // hard voice
		case ZmbSfxGroupFlags::kPractice_05:
			if (2 <= routeLevel && routeLevel <= 3)
				return kSysResSound20098_Route0_BridgeHard;
			return kSysResSound20094_Route0_BridgeVoiceA;
		default:
			break;
		}
	} else if (_xferView == XferSceneId::kScene1_BigBadHungry) { // XFER_1 - BIG BAD HUNGRY: switches on destination puzzle
		switch (_nextPageType) {
		case ZoombiniPageType::kBasecamp1:
			// BC1 destination: random no-voice pick from caves/pizza no-voice
			if (!_vm->_rnd->getRandomBool())
				return kSysResSound20009_Route1_TunnelsNoVoice;
			return kSysResSound20012_Route1_PizzaNoVoice;
		case ZoombiniPageType::kTunnels:
			// Group 1 and both dedicated Group 2 visit selectors use SND 20008.
			// The random group selects SND 20007, SND 20008, or the silent option.
			// Practice uses SND 20007.
			switch (sfxGroupFlags) {
			case ZmbSfxGroupFlags::kEasy_01:
			case ZmbSfxGroupFlags::kHardFirstTrigger_02:
			case ZmbSfxGroupFlags::kHardSecondTrigger_12:
				return kSysResSound20008_Route1_TunnelsVoiceB;
			case ZmbSfxGroupFlags::kRandom_00:
				switch (_vm->_rnd->getRandomNumber(1, 3)) {
				case 1:
					return kSysResSound20007_Route1_TunnelsVoiceA;
				case 2:
					return kSysResSound20008_Route1_TunnelsVoiceB;
				default:
					return kSysResSound20009_Route1_TunnelsNoVoice; // no-voice
				}
			default:
				return kSysResSound20007_Route1_TunnelsVoiceA;
			}
			break;
		case ZoombiniPageType::kPizza:
			switch (sfxGroupFlags) {
			case ZmbSfxGroupFlags::kEasy_01:
				return kSysResSound20010_Route1_PizzaVoiceA;
			case ZmbSfxGroupFlags::kHardFirstTrigger_02:
				return kSysResSound20011_Route1_PizzaVoiceB;
			case ZmbSfxGroupFlags::kPractice_05:
				if (routeLevel < 2)
					return kSysResSound20010_Route1_PizzaVoiceA;
				return kSysResSound20011_Route1_PizzaVoiceB;
			case ZmbSfxGroupFlags::kHardSecondTrigger_12:
				return kSysResSound20011_Route1_PizzaVoiceB;
			case ZmbSfxGroupFlags::kRandom_00:
			default:
				if (routeLevel < 2) {
					if (!_vm->_rnd->getRandomBool())
						return kSysResSound20010_Route1_PizzaVoiceA;
					return kSysResSound20012_Route1_PizzaNoVoice;
				} else {
					if (!_vm->_rnd->getRandomBool())
						return kSysResSound20011_Route1_PizzaVoiceB;
					return kSysResSound20012_Route1_PizzaNoVoice;
				}
			}
			break;
		default:
			break;
		}
	} else if (_xferView == XferSceneId::kScene2_WhosBayou) { // XFER_2 - WHO'S BAYOU: switches on destination puzzle
		switch (_nextPageType) {
		case ZoombiniPageType::kBasecamp2:
			// BC2 via bayou: random no-voice from ferry/lilly/slides
			switch (_vm->_rnd->getRandomNumber(1, 3)) {
			case 1:
				return kSysResSound20016_Route2_FerryNoVoice; // ferry no-voice
			case 2:
				return kSysResSound20020_Route2_LillyNoVoice; // lilly no-voice
			default:
				return kSysResSound20024_Route2_SlidesNoVoice; // slides no-voice
			}
			break;
		case ZoombiniPageType::kFerry:
			switch (sfxGroupFlags) {
			case ZmbSfxGroupFlags::kRandom_00:
				if (2 <= routeLevel) {
					// Higher level: 4 choices including hard
					switch (_vm->_rnd->getRandomNumber(1, 4)) {
					case 1:
						return kSysResSound20013_Route2_FerryVoiceA;
					case 2:
						return kSysResSound20014_Route2_FerryVoiceB;
					case 3:
						return kSysResSound20015_Route2_FerryHard; // hard
					default:
						return kSysResSound20016_Route2_FerryNoVoice; // no-voice
					}
				} else {
					switch (_vm->_rnd->getRandomNumber(1, 3)) {
					case 1:
						return kSysResSound20013_Route2_FerryVoiceA;
					case 2:
						return kSysResSound20014_Route2_FerryVoiceB;
					default:
						return kSysResSound20016_Route2_FerryNoVoice; // no-voice
					}
				}
				break;
			case ZmbSfxGroupFlags::kEasy_01:
			case ZmbSfxGroupFlags::kPractice_05:
				return kSysResSound20014_Route2_FerryVoiceB;
			case ZmbSfxGroupFlags::kHardFirstTrigger_02:
			case ZmbSfxGroupFlags::kHardSecondTrigger_12:
				return kSysResSound20015_Route2_FerryHard; // hard
			default:
				break;
			}
			break;
		case ZoombiniPageType::kLilly:
			switch (sfxGroupFlags) {
			case ZmbSfxGroupFlags::kRandom_00:
				if (2 <= routeLevel) {
					switch (_vm->_rnd->getRandomNumber(1, 4)) {
					case 1:
						return kSysResSound20017_Route2_LillyVoiceA;
					case 2:
						return kSysResSound20018_Route2_LillyVoiceB;
					case 3:
						return kSysResSound20019_Route2_LillyHard; // hard
					default:
						return kSysResSound20020_Route2_LillyNoVoice; // no-voice
					}
				} else {
					switch (_vm->_rnd->getRandomNumber(1, 3)) {
					case 1:
						return kSysResSound20017_Route2_LillyVoiceA;
					case 2:
						return kSysResSound20018_Route2_LillyVoiceB;
					default:
						return kSysResSound20020_Route2_LillyNoVoice;
					}
				}
				break;
			case ZmbSfxGroupFlags::kEasy_01:
				return kSysResSound20018_Route2_LillyVoiceB;
			case ZmbSfxGroupFlags::kHardFirstTrigger_02:
			case ZmbSfxGroupFlags::kHardSecondTrigger_12:
				return kSysResSound20019_Route2_LillyHard;
			case ZmbSfxGroupFlags::kPractice_05:
				if (routeLevel < 2)
					return kSysResSound20018_Route2_LillyVoiceB;
				return kSysResSound20019_Route2_LillyHard;
			default:
				break;
			}
			break;
		case ZoombiniPageType::kSlides:
			if (sfxGroupFlags == ZmbSfxGroupFlags::kRandom_00) {
				switch (_vm->_rnd->getRandomNumber(1, 3)) {
				case 1:
					return kSysResSound20021_Route2_SlidesVoiceA;
				case 2:
					return kSysResSound20022_Route2_SlidesVoiceB;
				default:
					return kSysResSound20024_Route2_SlidesNoVoice;
				}
			} else {
				return kSysResSound20022_Route2_SlidesVoiceB;
			}
			break;
		default:
			break;
		}
	} else if (_xferView == XferSceneId::kScene3_DeepDarkForest) { // XFER_3 - DEEP DARK FOREST: switches on destination puzzle
		switch (_nextPageType) {
		case ZoombiniPageType::kBasecamp2:
			// BC2 via forest: random no-voice from fleens/hotel/net
			switch (_vm->_rnd->getRandomNumber(1, 3)) {
			case 1:
				return kSysResSound20028_Route3_FleensNoVoice; // fleens no-voice
			case 2:
				return kSysResSound20031_Route3_HotelNoVoice; // hotel no-voice
			default:
				return kSysResSound20034_Route3_NetNoVoice; // net no-voice
			}
			break;
		case ZoombiniPageType::kFleens:
			switch (sfxGroupFlags) {
			case ZmbSfxGroupFlags::kRandom_00:
				if (routeLevel == 1 || routeLevel == 3) {
					// Low difficulty levels: 3 choices, no hard
					switch (_vm->_rnd->getRandomNumber(1, 3)) {
					case 1:
						return kSysResSound20025_Route3_FleensVoiceA;
					case 2:
						return kSysResSound20026_Route3_FleensVoiceB;
					default:
						return kSysResSound20028_Route3_FleensNoVoice;
					}
				} else {
					// Higher: 4 choices including hard
					switch (_vm->_rnd->getRandomNumber(1, 4)) {
					case 1:
						return kSysResSound20025_Route3_FleensVoiceA;
					case 2:
						return kSysResSound20026_Route3_FleensVoiceB;
					case 3:
						return kSysResSound20027_Route3_FleensHard;
					default:
						return kSysResSound20028_Route3_FleensNoVoice;
					}
				}
				break;
			case ZmbSfxGroupFlags::kEasy_01:
			case ZmbSfxGroupFlags::kPractice_05:
				return kSysResSound20026_Route3_FleensVoiceB;
			case ZmbSfxGroupFlags::kHardFirstTrigger_02:
			case ZmbSfxGroupFlags::kHardSecondTrigger_12:
				return kSysResSound20026_Route3_FleensVoiceB;
			default:
				break;
			}
			break;
		case ZoombiniPageType::kHotel:
			if (sfxGroupFlags == ZmbSfxGroupFlags::kRandom_00) {
				switch (_vm->_rnd->getRandomNumber(1, 3)) {
				case 1:
					return kSysResSound20029_Route3_HotelVoiceA;
				case 2:
					return kSysResSound20030_Route3_HotelVoiceB;
				default:
					return kSysResSound20031_Route3_HotelNoVoice; // no-voice
				}
			} else {
				return kSysResSound20030_Route3_HotelVoiceB;
			}
			break;
		case ZoombiniPageType::kNet:
			if (sfxGroupFlags == ZmbSfxGroupFlags::kRandom_00) {
				switch (_vm->_rnd->getRandomNumber(1, 3)) {
				case 1:
					return kSysResSound20032_Route3_NetVoiceA;
				case 2:
					return kSysResSound20033_Route3_NetVoiceB;
				default:
					return kSysResSound20034_Route3_NetNoVoice; // no-voice
				}
			} else {
				return kSysResSound20033_Route3_NetVoiceB;
			}
			break;
		default:
			break;
		}
	} else if (_xferView == XferSceneId::kScene4_MountainOfDespair) { // XFER_4 - MOUNTAIN OF DESPAIR: switches on destination puzzle
		switch (_nextPageType) {
		case ZoombiniPageType::kCaves:
			if (sfxGroupFlags == ZmbSfxGroupFlags::kRandom_00) {
				switch (_vm->_rnd->getRandomNumber(1, 3)) {
				case 1:
					return kSysResSound20035_Route4_CavesVoiceA;
				case 2:
					return kSysResSound20036_Route4_CavesVoiceB;
				default:
					return kSysResSound20037_Route4_CavesNoVoice; // no-voice
				}
			} else {
				return kSysResSound20036_Route4_CavesVoiceB;
			}
			break;
		case ZoombiniPageType::kSmoke:
			switch (sfxGroupFlags) {
			case ZmbSfxGroupFlags::kRandom_00:
				if (2 <= routeLevel) {
					switch (_vm->_rnd->getRandomNumber(1, 4)) {
					case 1:
						return kSysResSound20000_Route4_SmokeVoiceA;
					case 2:
						return kSysResSound20001_Route4_SmokeVoiceB;
					case 3:
						return kSysResSound20002_Route4_SmokeHard;
					default:
						return kSysResSound20003_Route4_SmokeNoVoice;
					}
				} else {
					switch (_vm->_rnd->getRandomNumber(1, 3)) {
					case 1:
						return kSysResSound20000_Route4_SmokeVoiceA;
					case 2:
						return kSysResSound20001_Route4_SmokeVoiceB;
					default:
						return kSysResSound20003_Route4_SmokeNoVoice; // no-voice
					}
				}
				break;
			case ZmbSfxGroupFlags::kEasy_01:
				return kSysResSound20002_Route4_SmokeHard;
			case ZmbSfxGroupFlags::kHardFirstTrigger_02:
			case ZmbSfxGroupFlags::kPractice_05:
			case ZmbSfxGroupFlags::kHardSecondTrigger_12:
				if (routeLevel < 2)
					return kSysResSound20001_Route4_SmokeVoiceB;
				return kSysResSound20002_Route4_SmokeHard;
			default:
				break;
			}
			break;
		case ZoombiniPageType::kMaze:
			if (sfxGroupFlags == ZmbSfxGroupFlags::kRandom_00) {
				switch (_vm->_rnd->getRandomNumber(1, 3)) {
				case 1:
					return kSysResSound20004_Route4_MazeVoiceA;
				case 2:
					return kSysResSound20005_Route4_MazeVoiceB;
				default:
					return kSysResSound20006_Route4_MazeNoVoice; // no-voice
				}
			} else {
				return kSysResSound20005_Route4_MazeVoiceB;
			}
			break;
		default:
			break;
		}
	} else if (_xferView == XferSceneId::kScene5_ToTown) { // XFER_5 - TO TOWN
		// Easy and practice select SND 20100 directly. Every other group still
		// consumes the authored 1-4 selector before converging on SND 20100.
		if (sfxGroupFlags != ZmbSfxGroupFlags::kEasy_01 &&
			sfxGroupFlags != ZmbSfxGroupFlags::kPractice_05)
			(void)_vm->_rnd->getRandomNumber(1, 4);
		return kSysResSound20100_Route5_TownVoice;
	}

	return 0;
}

void ZoombiniTransitionXfer::skipToCompletion() {
	// Once the pending target is set, commit immediately instead of waiting for the sound to finish.
	_closureFrame = 1;
	_xferSoundRes = ZmbResource();
}

void ZoombiniTransitionXfer::setBackgroundBitmap() {
	computeXferRoute();
	// Sound selection precedes every setup-time position and Snoid seed draw.
	_xferSoundRes = ZmbResource(ZmbResource::kSystem, selectXferSoundId());
	_vm->_gfx->setPalette(_xferBackgroundResId);
	_vm->_gfx->drawBackground(_xferBackgroundResId);
}

void ZoombiniTransitionXfer::loadFeatures() {
	// Render the page in registration order and keep manual links persistent.
	// This hides idle snoids behind dock rock 5100, then moves each walker in front of it at the second SCRS event 0,
	// independently of position.
	_manualZOrder = true;

	// Disable fidget and idle animations during the transition.
	_vm->_fidgetThreshold = 0;
	_vm->resetFidgetActivity();

	// Load environment SCRBs, zoombinis, sub-feature, sound, text.
	const ZmbResource xferShapes(ZmbResource::kPage, _xferShapesId);

	// Load environment SCRBs.
	// XFER_0/XFER_5: 9 SCRBs (5100-5108 / 6100-6108); each loops with event-trigger flags.
	// 0x01188000 (LOOP_ANIM | DEFER_ANIM | PLAY_ONCE | DEFER_RENDER)
	// XFER_1-4: 3 SCRBs from xferShapes:
	//   [0] main overlay: 0x0C10C000 (NO_DIRTY_MERGE | LOOP_ANIM | PLAY_ONCE | OVERLAY | REGION_TRACK)
	//   [1],[2] static shapes: flags = 0
	const uint32 kEnvScrbFlags = ZmbFeature::FLAG_00008000_LOOP_ANIM |
								 ZmbFeature::FLAG_00080000_DEFER_ANIM |
								 ZmbFeature::FLAG_00100000_PLAY_ONCE |
								 ZmbFeature::FLAG_01000000_DEFER_RENDER;

	const uint16 xferScene = static_cast<uint16>(_xferView);
	const bool isMidRoute = (static_cast<uint16>(XferSceneId::kScene1_BigBadHungry) <= xferScene &&
							 xferScene <= static_cast<uint16>(XferSceneId::kScene4_MountainOfDespair));
	const bool isToTown = (_xferView == XferSceneId::kScene5_ToTown);
	const bool isFromIsle = (_xferView == XferSceneId::kScene0_FromIsle);

	// Initialize callback state.
	_completionCounter = 0;
	_bodyArrangementOverride = 0;
	_linkTargetScrbId = 0;
	_finalEnvScrbId = 0;
	_envOneShotScrbId = 0;
	_envOneShotAvailable = false;
	_xfer5EventScrbId = 0;
	_xfer5DisplayedTownCount = _vm->_state->getCurrentState()._zmbStoredTownCount;
	_xfer5TownCountNeedsBake = true;
	_xfer5TownCountRefreshPending = false;
	_xfer5ForegroundFeatures[0] = nullptr;
	_xfer5ForegroundFeatures[1] = nullptr;
	_xfer5ForegroundActivationPending = false;
	_midRouteWalkerCount = 0;
	_midRouteWalkerActivationPending = false;
	_midRouteInitialCompositeRendered = false;
	for (int i = 0; i < 16; i++)
		_midRouteWalkers[i] = nullptr;
	for (int i = 0; i < 4; i++)
		_envScrbIds[i] = 0;
	for (int i = 0; i < 2; i++)
		_envEventTriggerFlags[i] = false;

	// -----------------------------------------------------------------------
	// Phase 1: pre-snoid environment SCRBs
	// -----------------------------------------------------------------------
	if (isFromIsle) {
		// XFER_0: loads animated SCRBs 5102-5108 BEFORE snoids (they render behind),
		// then static overlays 5100-5101 AFTER snoids (they render in front).
		const int16 environmentScrbIds[] = {
			kResScrb5102_Route0_EventDirtFallLeft,
			kResScrb5103_Route0_EventDirtFallRight,
			kResScrb5104_Route0_RandomBirdFlight1,
			kResScrb5105_Route0_RandomBirdFlight2,
			kResScrb5106_Route0_RandomBirdFlight3,
			kResScrb5107_Route0_RandomBirdFlight4,
			kResScrb5108_Route0_DirtCollapse};
		for (uint16 i = 0; i < ARRAYSIZE(environmentScrbIds); i++)
			loadScrbFeature(xferShapes, environmentScrbIds[i], 6, kEnvScrbFlags);

		// The original four-entry random runner table contains SCRBs 5104-5107.
		// SCRBs 5102 and 5103 belong exclusively to Snoid events 10 and 11.
		_envScrbIds[0] = kResScrb5104_Route0_RandomBirdFlight1;
		_envScrbIds[1] = kResScrb5105_Route0_RandomBirdFlight2;
		_envScrbIds[2] = kResScrb5106_Route0_RandomBirdFlight3;
		_envScrbIds[3] = kResScrb5107_Route0_RandomBirdFlight4;

		_envOneShotScrbId = kResScrb5108_Route0_DirtCollapse;
		_envOneShotAvailable = true;

		// One-shot flags for events 10-11 (SCRBs 5102-5103).
		_envEventTriggerFlags[0] = true; // event 10 -> SCRB 5102
		_envEventTriggerFlags[1] = true; // event 11 -> SCRB 5103

		// Load SHPL 3200.
		_useSmallSnoids = true;
	} else if (isToTown) {
		// In XFER_5, SCRB 6108, 6105, and 6104 appear behind the Snoids.
		// SCRB 6100-6103 and 6106-6107 appear in front and are loaded below.
		loadScrbFeature(xferShapes, kResScrb6108_Route5_FarBackgroundAnimation, 6, kEnvScrbFlags);
		ZmbFeature::EventHooks townCountHooks;
		townCountHooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniTransitionXfer::xfer5TownCount_render));
		townCountHooks.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniTransitionXfer::xfer5TownCount_onPostRender));
		loadScrbFeature(xferShapes, kResScrb6105_Route5_TownCount, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES, townCountHooks);
		loadScrbFeature(xferShapes, kResScrb6104_Route5_MidBackground, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);

		_finalEnvScrbId = kResScrb6108_Route5_FarBackgroundAnimation;

		_linkTargetScrbId = kResScrb6104_Route5_MidBackground;

		_xfer5EventScrbId = kResScrb6105_Route5_TownCount;
	} else {
		// XFER_1-4: main overlay SCRB with patch hook + sub-feature go before snoids.
		const uint32 kMainScrbFlags = ZmbFeature::FLAG_00004000_NO_DIRTY_MERGE |
									  ZmbFeature::FLAG_00008000_LOOP_ANIM |
									  ZmbFeature::FLAG_00100000_PLAY_ONCE |
									  ZmbFeature::FLAG_04000000_OVERLAY |
									  ZmbFeature::FLAG_08000000_REGION_TRACK;

		// The route-specific sub-feature SCRB draws the route path.
		// Its pre-render callback selects the active band, then its render callback flood-fills that band.
		computeRoutePathBand();
		computeRoutePathColorLevel();
		buildPuzzleCompletionArray();
		_routePathCounter = 0;
		_routePathNextFrame = getCurrentFrameCounter();
		_routePathPixels = nullptr;

		int16 mainScrbId = 0;
		int16 routePathScrbId = 0;
		switch (_xferView) {
		case XferSceneId::kScene1_BigBadHungry:
			mainScrbId = kResScrb1100_Route1_MainOverlay;
			routePathScrbId = kResScrb1200_Route1_RoutePath;
			break;
		case XferSceneId::kScene2_WhosBayou:
			mainScrbId = kResScrb2100_Route2_MainOverlay;
			routePathScrbId = kResScrb2200_Route2_RoutePath;
			break;
		case XferSceneId::kScene3_DeepDarkForest:
			mainScrbId = kResScrb3100_Route3_MainOverlay;
			routePathScrbId = kResScrb3200_Route3_RoutePath;
			break;
		case XferSceneId::kScene4_MountainOfDespair:
			mainScrbId = kResScrb4100_Route4_MainOverlay;
			routePathScrbId = kResScrb4200_Route4_RoutePath;
			break;
		default:
			error("xfer: invalid mid-route view %d", static_cast<int>(_xferView));
			break;
		}

		// Main SCRB with route view shape remapping callback.
		ZmbFeature::EventHooks mainScrbHooks;
		mainScrbHooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniTransitionXfer::routeView_updateSlots));
		loadScrbFeature(xferShapes, mainScrbId, 6, kMainScrbFlags, mainScrbHooks);

		ZmbFeature::EventHooks routePathHooks;
		routePathHooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniTransitionXfer::routePath_selectBand));
		routePathHooks.setRenderFunc(static_cast<ZmbFeature::OnRenderFunc>(&ZoombiniTransitionXfer::routePath_onPostRender));
		_routePathFeature = loadScrbFeature(ZmbResource(ZmbResource::kPage, routePathScrbId), routePathScrbId, 4, ZmbFeature::FLAG_04000000_OVERLAY, routePathHooks);
		// Register this runner with a timed pre-render callback so it remains render-active
		// while @ref ZoombiniTransitionXfer::routePath_onPostRender() mutates pixels.
		if (_routePathFeature)
			_routePathFeature->activateRender();
	}

	// -----------------------------------------------------------------------
	// Snoids -- loaded from active pack (set by preceding page's save/cleanup).
	// -----------------------------------------------------------------------
	ZmbStateActivePack &pack = _vm->_state->getCurrentState()._zmbPackActive;

	// Reset the shared walk/drag lock before loading walkers.
	_vm->_walkersInProgress = 0;

	Common::Point startPositions[16];
	const uint16 startPositionCount = ARRAYSIZE(startPositions);
	for (uint16 i = 0; i < startPositionCount; i++) {
		if (isFromIsle) {
			startPositions[i] = Common::Point(200, 235);
		} else if (isMidRoute) {
			startPositions[i] = Common::Point(-22, 445);
		} else {
			const int16 randY = static_cast<int16>(6 * _vm->_rnd->getRandomNumber(3) + 282);
			startPositions[i] = Common::Point(-22, randY);
		}
	}

	Common::Array<ZmbSnoid *> loadedSnoids;
	_xferSnoidCount = loadDynamicSnoidsFromPack(pack, startPositions, startPositionCount, false, &loadedSnoids);

	for (ZmbSnoid *snoid : loadedSnoids) {
		if (_useSmallSnoids) {
			// Load SHPL 3200.
			// @ref ZmbSnoid::_useSmallShapeRegs activates the small-table path.
			// @ref ZmbSnoid::updateWalkHotspots() then uses that path.
			// It also activates the small REGS render path.
			snoid->setResource(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap3200_SnoidSmall));
			snoid->setupSmallIdleHotspots();
		} else {
			snoid->setupIdleHotspots();
		}

		if (isMidRoute) {
			// Keep the registration order until the first static composite has rendered.
			// The stagger dispatcher then traverses this array in reverse.
			if (_midRouteWalkerCount < ARRAYSIZE(_midRouteWalkers)) {
				_midRouteWalkers[_midRouteWalkerCount] = snoid;
				_midRouteWalkerCount += 1;
			}
		}
	}
	_midRouteWalkerActivationPending = isMidRoute && 0 < _midRouteWalkerCount;

	// -----------------------------------------------------------------------
	// Phase 2: post-snoid features (rendered in front of snoids)
	// -----------------------------------------------------------------------
	if (isFromIsle) {
		// XFER_0: static SCRBs 5100-5101 loaded AFTER snoids. With the page's
		// Z-sort disabled through @ref ZoombiniPage::_manualZOrder,
		// registration order alone places them in front of the idle snoid stack.
		// They are the final flags-0 features registered in this branch.
		loadScrbFeature(xferShapes, kResScrb5100_Route0_DockRock, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
		loadScrbFeature(xferShapes, kResScrb5101_Route0_DockForeground, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);

		_linkTargetScrbId = kResScrb5100_Route0_DockRock;
	} else if (isToTown) {
		// 6100-6103 static foreground, 6106-6107 animated foreground -- above snoid walkers.
		const int16 foregroundScrbIds[] = {
			kResScrb6100_Route5_Foreground1,
			kResScrb6101_Route5_Foreground2,
			kResScrb6102_Route5_Foreground3,
			kResScrb6103_Route5_Foreground4};
		for (uint16 i = 0; i < ARRAYSIZE(foregroundScrbIds); i++)
			loadScrbFeature(xferShapes, foregroundScrbIds[i], 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);

		// SCRBs 6106 and 6107 omit LOOP_ANIM but retain DEFER_ANIM, PLAY_ONCE, and DEFER_RENDER.
		const uint32 kEnvScrbFlagsNoLoop = ZmbFeature::FLAG_00080000_DEFER_ANIM |
										   ZmbFeature::FLAG_00100000_PLAY_ONCE |
										   ZmbFeature::FLAG_01000000_DEFER_RENDER;
		ZmbFeature *fg6106 = loadScrbFeature(xferShapes, kResScrb6106_Route5_BirdFlyby, 6, kEnvScrbFlagsNoLoop);
		ZmbFeature *fg6107 = loadScrbFeature(xferShapes, kResScrb6107_Route5_BeeFlight, 6, kEnvScrbFlagsNoLoop);
		_xfer5ForegroundFeatures[0] = fg6106;
		_xfer5ForegroundFeatures[1] = fg6107;
		_xfer5ForegroundActivationPending = true;
	} else if (isMidRoute) {
		// Render shapes 1 and 2 as static overlapping edges above the walker overlay.
		int16 foregroundEdge1Id = 0;
		int16 foregroundEdge2Id = 0;
		switch (_xferView) {
		case XferSceneId::kScene1_BigBadHungry:
			foregroundEdge1Id = kResScrb1101_Route1_ForegroundEdge1;
			foregroundEdge2Id = kResScrb1102_Route1_ForegroundEdge2;
			break;
		case XferSceneId::kScene2_WhosBayou:
			foregroundEdge1Id = kResScrb2101_Route2_ForegroundEdge1;
			foregroundEdge2Id = kResScrb2102_Route2_ForegroundEdge2;
			break;
		case XferSceneId::kScene3_DeepDarkForest:
			foregroundEdge1Id = kResScrb3101_Route3_ForegroundEdge1;
			foregroundEdge2Id = kResScrb3102_Route3_ForegroundEdge2;
			break;
		case XferSceneId::kScene4_MountainOfDespair:
			foregroundEdge1Id = kResScrb4101_Route4_ForegroundEdge1;
			foregroundEdge2Id = kResScrb4102_Route4_ForegroundEdge2;
			break;
		default:
			error("xfer: invalid mid-route view %d", static_cast<int>(_xferView));
			break;
		}
		loadScrbFeature(xferShapes, foregroundEdge1Id, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
		loadScrbFeature(xferShapes, foregroundEdge2Id, 0, ZmbFeature::FLAG_00000000_TYPE_SHAPES);
	}

	// Use a fixed auto-close timer; walker completion does not close the page.
	_closureFrame = getCurrentFrameCounter() + 300;

	// Both SCRS-driven views start at frame zero. The first post-render
	// controller pass therefore fires immediately and assigns the next delay.
	if (isFromIsle) {
		_scrsNextTriggerFrame = 0;
		// Feet variants select SCRS 5200-5204 from this base.
		_scrsResIdBase = kResScrs5199_Route0_WalkBase;
	} else if (isToTown) {
		// First trigger fires immediately.
		_scrsNextTriggerFrame = 0;
		// Feet variants select SCRS 6200-6204 from this base.
		_scrsResIdBase = kResScrs6199_Route5_WalkBase;
	}

	// Materialize the complete registration-order scene before starting the
	// deferred environment features or the mid-route walkers.
	renderFeatures();

	// Play the composite Xfer PCM track selected before background setup.
	// Every variant, including the tracks without Narrator speech, uses the Speech volume category.
	// Selected sound IDs 20000-20100 are in @ref ZmbResource::kSystem, not XFER.MHK.
	if (_xferSoundRes.hasId() && _vm->hasResource(ID_SND, _xferSoundRes))
		_vm->_sound->playSound(_xferSoundRes, Audio::Mixer::kSpeechSoundType);

	// Draw route name text for mid-route views (XFER_1-4) with palette 10 foreground and palette 45 outline.
	// Bake it into the persistent background so it survives later shape-screen redraws.
	//
	// Drawing into @ref ZoombiniGraphics::kShapeScreen is temporary because that screen is rebuilt every frame.
	// The route name would therefore appear for at most one frame.
	// Drawing into @ref ZoombiniGraphics::kBackScreen bakes the text into the persistent background bitmap.
	if (static_cast<uint16>(XferSceneId::kScene1_BigBadHungry) <= xferScene &&
		xferScene <= static_cast<uint16>(XferSceneId::kScene4_MountainOfDespair)) {
		//   Route 1 (BigBadHungry):    left=43,  top=54,  right=226, bottom=107
		//   Route 2 (WhosBayou):       left=371, top=33,  right=613, bottom=65
		//   Route 3 (DeepDarkForest):  left=127, top=29,  right=299, bottom=81
		//   Route 4 (MountainDespair): left=135, top=29,  right=323, bottom=82
		const uint32 textKey = static_cast<uint32>(ZoombiniText::Key::kRoute1) + xferScene - 1;
		const Common::Rect &textRect = _routeTextRects[xferScene - 1];

		ZoombiniGraphics::TextConf tc;
		tc._fontUsage = ZoombiniFontUsage::kFontTitle;
		tc._outlineEffect = true;
		tc._textPalette = ZoombiniGraphics::kColor0A_White;    // palette #10 (fg)
		tc._outlinePalette = ZoombiniGraphics::kColor2D_Black; // palette #45 (shadow)
		tc._hAlign = Graphics::kTextAlignCenter;
		tc._vAlign = Graphics::kTextAlignCenter;
		tc._wordWrap = false;
		_vm->_gfx->drawText(ZoombiniGraphics::kBackScreen, textKey, textRect, tc);
	}

	if (_xfer5ForegroundActivationPending) {
		activateXfer5EnvironmentalFeatures();
		_xfer5ForegroundActivationPending = false;
	}

	if (_midRouteWalkerActivationPending && _midRouteInitialCompositeRendered)
		activateMidRouteWalkers();
}

void ZoombiniTransitionXfer::activateXfer5EnvironmentalFeatures() {
	// The first static render pass consumes the deferred SCRB load state.
	// Start the one-shots afterward in scene order: SCRB 6107, then SCRB 6106.
	ZmbFeature *beeFlight = _xfer5ForegroundFeatures[1];
	if (beeFlight) {
		beeFlight->initValues();
		beeFlight->activateAnimate();
		beeFlight->activateRender();
	}

	ZmbFeature *birdFlyby = _xfer5ForegroundFeatures[0];
	if (birdFlyby) {
		birdFlyby->initValues();
		birdFlyby->activateAnimate();
		birdFlyby->activateRender();
	}

	addExternalDirtyRect(Common::Rect(0, 0, 640, 480));
}

void ZoombiniTransitionXfer::activateMidRouteWalkers() {
	// All XFER_1-4 walkers begin at the same destination X coordinate.
	// Stable sorting therefore preserves registration order, and reverse traversal starts the last registered walker first.
	const uint32 frameBase = getCurrentFrameCounter();
	uint16 staggerIdx = 0;
	for (int walkerIdx = static_cast<int>(_midRouteWalkerCount) - 1; 0 <= walkerIdx; walkerIdx--) {
		ZmbSnoid *snoid = _midRouteWalkers[walkerIdx];
		if (!snoid || !snoid->_packIsOccupied || snoid->getAnimState() != kSnoidAnimState000_Idle)
			continue;

		snoid->setAnimTargetPos(Common::Point(670, 445));
		snoid->setAnimState(kSnoidAnimState010_ArrivalMotion, nullptr);
		snoid->setDelayUntilFrame(frameBase + staggerIdx * 90);
		staggerIdx += 1;
	}

	_midRouteWalkerActivationPending = false;
}

void ZoombiniTransitionXfer::onAnimFrame() {
	ZoombiniPage::onAnimFrame();

	if (isClosed())
		return;

	if (_xferView == XferSceneId::kScene5_ToTown) {
		if (_xfer5TownCountRefreshPending) {
			_xfer5TownCountNeedsBake = true;
			reloadXfer5TownCount();
			_xfer5TownCountRefreshPending = false;
		}

		for (int featureIdx = 0; featureIdx < 2; featureIdx++) {
			ZmbFeature *feature = _xfer5ForegroundFeatures[featureIdx];
			if (feature && !feature->hasAnimEndCallbackFired())
				addExternalDirtyRect(Common::Rect(0, 0, 640, 480));
		}
	}

	// -----------------------------------------------------------------------
	// Auto-close every view after the 300-frame deadline and sound completion.
	// -----------------------------------------------------------------------
	if (0 < _closureFrame && _closureFrame < getCurrentFrameCounter()) {
		if (!_vm->_sound->isSoundPlaying(_xferSoundRes)) {
			close();
			return;
		}
	}

	// -----------------------------------------------------------------------
	// Completion counter check + SCRS periodic triggers.
	// Completion check fires first; view-specific branches follow.
	// -----------------------------------------------------------------------
	if (_scrsNextTriggerFrame < getCurrentFrameCounter()) {
		// Activate the final environment SCRB after five Snoids complete.
		if (4 < _completionCounter) {
			_completionCounter = -1; // Disable further counting
			if (_finalEnvScrbId != 0)
				activateEnvScrb(_finalEnvScrbId);
			// The timer-based auto-close handles the transition.
		}

		// -------------------------------------------------------------------
		// XFER_0: periodic SCRS trigger -- start one snoid's animation per interval.
		// Timer: 30 * rand(3,6) = 90-180 frames between triggers.
		// 60% chance: trigger next idle snoid to play SCRS 5200.
		// 40% chance (after first trigger): trigger random env SCRB animation.
		// -------------------------------------------------------------------
		if (_xferView == XferSceneId::kScene0_FromIsle) {
			_scrsNextTriggerFrame = getCurrentFrameCounter() + 30 * _vm->_rnd->getRandomNumber(3, 6);

			int16 chance = _vm->_rnd->getRandomNumber(1, 100);
			if (chance <= 40 && _scrsTriggerPhase1) {
				// 40% chance (only after first snoid trigger): env SCRB activation.
				// 0-3 = Random env SCRB, 4 = one-shot.
				int16 envIdx = _vm->_rnd->getRandomNumber(0, 4);
				if (envIdx < 4) {
					// Activate one of the four bird-flight SCRBs (5104-5107).
					// These are transient because DEFER_RENDER remains set,
					// so it hides after playing.
					if (_envScrbIds[envIdx] != 0)
						activateEnvScrb(_envScrbIds[envIdx], false);
				} else {
					// For environment index 4, play the one-shot dirt-collapse SCRB 5108 and preserve its final frame.
					if (_envOneShotAvailable && _envOneShotScrbId != 0) {
						_envOneShotAvailable = false;
						activateEnvScrb(_envOneShotScrbId, true);
					}
				}
			} else {
				// 60% chance (or 100% if first trigger): trigger next idle snoid SCRS.
				_scrsTriggerPhase1 = true;

				if (_scrsTriggerIdx < _xferSnoidCount) {
					uint16 snoidId = getDynamicPackSnoidId(_scrsTriggerIdx);
					ZmbSnoid *snoid = getSnoid(snoidId);
					if (snoid) {
						// Face right before playback.
						// A direction left over from a prior cycle can otherwise flip the sprite mid-transition.
						snoid->setFacingLeft(false);
						// Feet traits 1-5 select walk SCRS 5200-5204.
						int16 scrsResId = static_cast<int16>(_scrsResIdBase + snoid->_trait._feet);
						snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsResId), ZmbScrsCompletionMode::kHide, ZmbScrsPlaybackMode::kReject);
					}
					_scrsTriggerIdx += 1;
				}
			}
		}

		// -------------------------------------------------------------------
		// XFER_5: periodic SCRS trigger -- same structure as XFER_0.
		// Timer: 40 * rand(3,6) = 120-240 frames between triggers.
		// 100% snoid triggers (no env SCRB split), using SCRS 6200.
		// SCRS pool 1 maps 6200-6204 to state 8 and the general five-layer renderer,
		// not the state-9/tBMP 3100 normal-script renderer.
		// -------------------------------------------------------------------
		if (_xferView == XferSceneId::kScene5_ToTown) {
			_scrsNextTriggerFrame = getCurrentFrameCounter() + 40 * _vm->_rnd->getRandomNumber(3, 6);

			if (_scrsTriggerIdx < _xferSnoidCount) {
				uint16 snoidId = getDynamicPackSnoidId(_scrsTriggerIdx);
				ZmbSnoid *snoid = getSnoid(snoidId);
				if (snoid) {
					// Face right before starting the walk.
					snoid->setFacingLeft(false);
					// Feet traits 1-5 select walk SCRS 6200-6204.
					int16 scrsResId = static_cast<int16>(_scrsResIdBase + snoid->_trait._feet);
					snoid->startScrsPlayback(ZmbResource(ZmbResource::kPage, scrsResId), ZmbScrsCompletionMode::kHide, ZmbScrsPlaybackMode::kReject);
				}
				_scrsTriggerIdx += 1;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Deliberate input adaptation: one click commits the pending destination and exits immediately.
// The auto-close timer is the only competing completion path here.
// A two-stage click/SCRB handshake is therefore unnecessary.
// ---------------------------------------------------------------------------
ZmbEventHandleResult ZoombiniTransitionXfer::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;

	if (!isClosed()) {
		skipToCompletion();
		return ZmbEventHandleResult::kConsumed;
	}
	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniTransitionXfer::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbdRepeat;

	// Enhanced shortcuts skip the transition without waiting for the auto-close timer or sound to finish.
	if (_vm->useEnhancedKbdShortcuts() && !isClosed()) {
		switch (kbd.keycode) {
		case Common::KEYCODE_SPACE:
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_ESCAPE:
			skipToCompletion();
			return ZmbEventHandleResult::kConsumed;
		default:
			break;
		}
	}
	return ZmbEventHandleResult::kPassthrough;
}

// ---------------------------------------------------------------------------
// Activate a deferred environment SCRB feature by ID.
// For features with DEFER_ANIM | DEFER_RENDER, this starts their animation.
// ---------------------------------------------------------------------------
void ZoombiniTransitionXfer::activateEnvScrb(int16 scrbId, bool persistAfterPlay) {
	ZmbFeature *feature = _scrbFeatures.find(scrbId);
	if (!feature)
		return;

	// Random re-triggers of SCRB 5104-5107 fire only while the feature is not already animating.
	if (!persistAfterPlay && feature->isAnimateActivated())
		return;

	if (persistAfterPlay) {
		// Drop DEFER_RENDER before the PLAY_ONCE cycle.
		// The frozen final frame then remains visible,
		// preserving the collapsed-dirt aftermath when its area is repainted.
		feature->removeFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER);
	}

	feature->initValues();
	feature->activateAnimate();
	feature->activateRender();
	const Common::Rect &dirtyRect = feature->getZSortRect();
	addExternalDirtyRect(dirtyRect.isEmpty() ? Common::Rect(0, 0, 640, 480) : dirtyRect);
}

ZmbRenderResult ZoombiniTransitionXfer::xfer5TownCount_render(ZmbFeature *feature) {
	if (!_xfer5TownCountNeedsBake)
		return ZmbRenderResult::kSkipped;

	return blitShapes(feature);
}

void ZoombiniTransitionXfer::xfer5TownCount_onPostRender(ZmbFeature *feature) {
	Common::Rect signRect = feature->getClickRect();
	if (signRect.isEmpty())
		return;

	Common::Rect textRect = signRect;
	textRect.left += 16;
	textRect.top += 8;

	Common::U32String text = _vm->_text->getLocalizedString(ZoombiniText::kXferVillePopulation);
	text += Common::U32String::format(text.lastChar() == ' ' ? "%d" : " %d", _xfer5DisplayedTownCount);

	ZoombiniGraphics::TextConf tc;
	tc._outlineEffect = true;
	tc._textPalette = 0xD1;
	tc._outlinePalette = 0x70;
	tc._hAlign = Graphics::kTextAlignLeft;
	tc._vAlign = Graphics::kTextAlignStart;

	_vm->_gfx->drawText(ZoombiniGraphics::kShapeScreen, text, textRect, tc);

	signRect.clip(Common::Rect(0, 0, 640, 480));
	Graphics::Surface *shapeScreen = _vm->_gfx->getScreen(ZoombiniGraphics::kShapeScreen);
	Graphics::Surface *backScreen = _vm->_gfx->getScreen(ZoombiniGraphics::kBackScreen);
	backScreen->copyRectToSurface(*shapeScreen, signRect.left, signRect.top, signRect);
	feature->deactivateRender();
	_xfer5TownCountNeedsBake = false;
}

void ZoombiniTransitionXfer::reloadXfer5TownCount() {
	ZmbFeature *feature = _scrbFeatures.find(_xfer5EventScrbId);
	if (!feature)
		return;

	// Population events restart the persistent sign runner even when its previous
	// refresh is still active. Each crossing must materialize the latest count.
	feature->initValues();
	feature->activateAnimate();
	feature->activateRender();
	const Common::Rect &dirtyRect = feature->getZSortRect();
	addExternalDirtyRect(dirtyRect.isEmpty() ? Common::Rect(0, 0, 640, 480) : dirtyRect);
}

// ---------------------------------------------------------------------------
// Handle SCRS event codes during playback.
// Called from the script engine when a SCRS frame terminator carries an event code.
// @param feature The Snoid feature that fired the event.
// @param eventCode Adjusted event code (raw byte - 1).
// A value of -1 marks the end of the animation.
// ---------------------------------------------------------------------------
void ZoombiniTransitionXfer::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	// Only XFER_0 and XFER_5 use SCRS-driven animation with callbacks.
	if (_xferView != XferSceneId::kScene0_FromIsle && _xferView != XferSceneId::kScene5_ToTown)
		return;

	// When the completion counter activates the final environment SCRB,
	// it also installs this page as the runner's frame-event callback.
	// Frame event 30 from XFER_5 SCRB 6108 then triggers the page transition.
	// This lets the to-town animation finish before the page closes, rather than racing the 300-frame automatic close.
	if (eventCode == 30 && _finalEnvScrbId != 0 && feature == _scrbFeatures.find(_finalEnvScrbId)) {
		_closureFrame = 1;
		_xferSoundRes = ZmbResource();
		return;
	}

	// The feature must be a snoid for body arrangement and visibility operations.
	ZmbSnoid *snoid = dynamic_cast<ZmbSnoid *>(feature);

	if (26 < eventCode) {
		// ---------------------------------------------------------------
		// Event 50 is XFER_5-only and activates the display's environment SCRB.
		// ---------------------------------------------------------------
		if (eventCode == 50) {
			_xfer5DisplayedTownCount += 1;
			if (_xfer5EventScrbId != 0) {
				_xfer5TownCountRefreshPending = true;
			}
		}
		// ---------------------------------------------------------------
		// Events 240-243: Set pending body arrangement override.
		// The pending arrangement is applied on the next event 0.
		// ---------------------------------------------------------------
		else if (kAnimEvent240_BodyArrangePendFirst <= eventCode && eventCode <= kAnimEvent243_BodyArrangePendLast) {
			_bodyArrangementOverride = eventCode - (kAnimEvent240_BodyArrangePendFirst - 1); // 1-4
		}
		// ---------------------------------------------------------------
		// Events 250-253: Direct body arrangement change.
		// ---------------------------------------------------------------
		else if (kAnimEvent250_BodyArrangeDirectFirst <= eventCode && eventCode <= kAnimEvent253_BodyArrangeDirectLast) {
			if (snoid)
				snoid->setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(eventCode - kAnimEvent250_BodyArrangeDirectFirst));
		}
	} else if (eventCode == 26) {
		// ---------------------------------------------------------------
		// Event 26: Animation complete -- reset body arrangement, link, count.
		// Reset to front arrangement.
		// Re-link the Snoid before the 5100 (XFER_0) or 6104 (XFER_5) runner.
		// This remains stable because page Z-sorting is disabled.
		// ---------------------------------------------------------------
		if (snoid) {
			snoid->setTraitLayout(ZmbScriptDecoder::TraitLayout::kSlotOrder00);

			if (_linkTargetScrbId != 0) {
				ZmbFeature *linkTarget = _scrbFeatures.find(_linkTargetScrbId);
				if (linkTarget)
					manualLinkBefore(snoid, linkTarget);
			}
		}

		if (0 <= _completionCounter)
			_completionCounter += 1;
	} else if (eventCode == kAnimEventM1_End) {
		// End-of-animation (PLAY_ONCE completion). No special handling needed.
	} else if (eventCode == 0) {
		// ---------------------------------------------------------------
		// Event 0: Toggle facing, apply pending arrangement, inc cycle.
		// The event toggles the facing-left state, not render visibility.
		// The walk zig-zags down the cliff ledges and flips direction at these keyframes.
		// Hiding the snoid would stop its animation state machine, leaving the SCRS at frame 0.
		// On the second XFER_0 cycle, relink the walker after the 5100 dock rock.
		// The page has no z-sort, so the link remains in effect.
		// For walk scripts 5200-5204, that event occurs at the first switchback,
		// when the walker emerges from behind the dock rock.
		// ---------------------------------------------------------------
		if (snoid) {
			// Toggle facing direction (walk turns at ledge switchbacks).
			snoid->setFacingLeft(!snoid->isFacingLeft());

			// Apply pending body arrangement override (set by events 240-243).
			if (_bodyArrangementOverride != 0) {
				snoid->setTraitLayout(static_cast<ZmbScriptDecoder::TraitLayout>(_bodyArrangementOverride - 1));
				_bodyArrangementOverride = 0;
			}

			// Increment per-snoid SCRS cycle counter.
			snoid->_scrsAnimCycleCount += 1;

			if (_xferView == XferSceneId::kScene0_FromIsle && snoid->_scrsAnimCycleCount == 2 &&
				_linkTargetScrbId != 0) {
				ZmbFeature *linkTarget = _scrbFeatures.find(_linkTargetScrbId);
				if (linkTarget) {
					manualLinkAfter(snoid, linkTarget);
					// The promotion changes which pixels win in the overlap
					// area without moving either feature -- repaint it.
					addExternalDirtyRect(snoid->getZSortRect());
				}
			}
		}
	} else if (10 <= eventCode && eventCode <= 11) {
		// ---------------------------------------------------------------
		// Events 10-11: One-shot env SCRB activation (XFER_0 only).
		// Event 10 -> SCRB 5102, Event 11 -> SCRB 5103.
		// ---------------------------------------------------------------
		if (_xferView == XferSceneId::kScene0_FromIsle) {
			uint16 flagIdx = eventCode - 10;
			if (flagIdx < 2 && _envEventTriggerFlags[flagIdx]) {
				_envEventTriggerFlags[flagIdx] = false;
				// Activate the corresponding environment script and preserve its final frame.
				int16 envScrbId;
				if (flagIdx == 0)
					envScrbId = kResScrb5102_Route0_EventDirtFallLeft;
				else
					envScrbId = kResScrb5103_Route0_EventDirtFallRight;
				activateEnvScrb(envScrbId, true);
			}
		}
	}
}

// ============================================================
// Route Path Flood-Fill Implementation (XFER_1-4 only)
// ============================================================

// Seed coordinates are stored as consecutive X/Y pairs for each route view and progress level.
static const int16 kRoutePathSeeds[] = {
	// XFER_1 bands 1-4
	3,
	105,
	130,
	146,
	1,
	2,
	6,
	60,
	// XFER_2 bands 1-4
	42,
	194,
	1,
	106,
	1,
	1,
	1,
	4,
	// XFER_3 bands 1-4
	1,
	1,
	1,
	1,
	1,
	53,
	102,
	162,
	// XFER_4 bands 1-4
	1,
	12,
	57,
	154,
	1,
	1,
	2,
	109,
};

// Overlapping route-view slot tables.
// The second view begins at index 2 of this 22-entry storage.
// @ref ZoombiniTransitionXfer::kRouteViewSlotTable provides both pre-slot and band-slot lookups.
static const int16 kRouteViewSlotTable[22] = {
	0, 0,                 // [0..1] (XFER_1 pre-slot padding)
	1, 2, 3, 4,           // [2..5] / [0..3]
	4, 5, 6, 7,           // [6..9] / [4..7]
	11, 4, 8, 9, 10, 16,  // [10..15] / [8..13]
	11, 12, 13, 14, 15, 3 // [16..21] / [14..19]
};

// Build the per-puzzle completion array from game state.
// Apply the route-slot fix-up and set @ref ZoombiniTransitionXfer::_routeProgressLevel for shape selection.
void ZoombiniTransitionXfer::buildPuzzleCompletionArray() {
	const ZmbStateFile &state = _vm->_state->getCurrentState();

	// First loop: read per-SI-page completion level.
	// SI indices align with the completion slots consumed by route-slot lookups,
	// @ref ZoombiniTransitionXfer::kRouteViewSlotTable, and @ref ZoombiniTransitionXfer::routeView_updateSlots().
	for (int i = 0; i <= 16; i++) {
		if (_vm->_state->inPracticeMode()) {
			_puzzleCompletionArr[i] = static_cast<int8>(CLIP<uint16>(_vm->_state->_practiceLevel, 0, 4));
		} else {
			_puzzleCompletionArr[i] = static_cast<int8>(readPuzzleLevelFlag(state, static_cast<ZmbSrcPageKind>(i)));
		}
	}

	// Determine global route slot index.
	switch (_nextPageType) {
	case ZoombiniPageType::kBridge:
		_routeSlotIndex = 1;
		break;
	case ZoombiniPageType::kTunnels:
		_routeSlotIndex = 2;
		break;
	case ZoombiniPageType::kPizza:
		_routeSlotIndex = 3;
		break;
	case ZoombiniPageType::kBasecamp1:
		_routeSlotIndex = 4;
		break;
	case ZoombiniPageType::kFerry:
		_routeSlotIndex = 5;
		break;
	case ZoombiniPageType::kLilly:
		_routeSlotIndex = 6;
		break;
	case ZoombiniPageType::kSlides:
		_routeSlotIndex = 7;
		break;
	case ZoombiniPageType::kBasecamp2:
		if (_vm->_xferSrcPage == ZmbSrcPageKind::kSlides_09)
			_routeSlotIndex = 11;
		else
			_routeSlotIndex = 16;
		break;
	case ZoombiniPageType::kFleens:
		_routeSlotIndex = 8;
		break;
	case ZoombiniPageType::kHotel:
		_routeSlotIndex = 9;
		break;
	case ZoombiniPageType::kNet:
		_routeSlotIndex = 10;
		break;
	case ZoombiniPageType::kCaves:
		_routeSlotIndex = 12;
		break;
	case ZoombiniPageType::kSmoke:
		_routeSlotIndex = 13;
		break;
	case ZoombiniPageType::kMaze:
		_routeSlotIndex = 14;
		break;
	case ZoombiniPageType::kTown:
		_routeSlotIndex = 15;
		break;
	default:
		_routeSlotIndex = 0;
		break;
	}

	if (_routeSlotIndex == 0)
		return;

	// The destination slot uses the current route-path color level.
	_puzzleCompletionArr[_routeSlotIndex] = static_cast<int8>(_routePathColorLevel - 1);
	if (_puzzleCompletionArr[_routeSlotIndex] < 1) {
		if (_routeProgressLevel < 0)
			_routeProgressLevel = static_cast<int16>(_routePathColorLevel - 1);
		_puzzleCompletionArr[_routeSlotIndex] = -1;
	}

	// Map the source SI page to its predecessor's completion slot.
	// Restore a negative predecessor to level 1 so the route view shows the departure as completed.
	// The current SI enum uses Tunnels=03 and Caves=14.
	int16 shuffledId = -1;
	switch (_vm->_xferSrcPage) {
	case ZmbSrcPageKind::kPicker_01:
		shuffledId = 0;
		break;
	case ZmbSrcPageKind::kBridge_02:
		shuffledId = 1;
		break;
	case ZmbSrcPageKind::kTunnels_03:
		shuffledId = 2;
		break;
	case ZmbSrcPageKind::kPizza_04:
		shuffledId = 3;
		break;
	case ZmbSrcPageKind::kBC1North_05:
		shuffledId = 4;
		break;
	case ZmbSrcPageKind::kFerry_07:
		shuffledId = 5;
		break;
	case ZmbSrcPageKind::kLilly_08:
		shuffledId = 6;
		break;
	case ZmbSrcPageKind::kSlides_09:
		shuffledId = 7;
		break;
	case ZmbSrcPageKind::kBC1South_06:
		shuffledId = 4;
		break;
	case ZmbSrcPageKind::kFleens_10:
		shuffledId = 8;
		break;
	case ZmbSrcPageKind::kHotel_11:
		shuffledId = 9;
		break;
	case ZmbSrcPageKind::kNet_12:
		shuffledId = 10;
		break;
	case ZmbSrcPageKind::kBC2_13:
		shuffledId = 11;
		break;
	case ZmbSrcPageKind::kCaves_14:
		shuffledId = 12;
		break;
	case ZmbSrcPageKind::kSmoke_15:
		shuffledId = 13;
		break;
	case ZmbSrcPageKind::kMaze_16:
		shuffledId = 14;
		break;
	default:
		break;
	}

	if (0 <= shuffledId && shuffledId < 17 && _puzzleCompletionArr[shuffledId] < 0)
		_puzzleCompletionArr[shuffledId] = 1;

	_routeProgressLevel = _puzzleCompletionArr[_routeSlotIndex];
	_puzzleCompletionArr[_routeSlotIndex] = -1;
}

// Set the route-band position 1-4 by matching the destination puzzle slot against the route slot table,
// using the source SI page that started this transition.
void ZoombiniTransitionXfer::computeRoutePathBand() {
	switch (_vm->_xferSrcPage) {
	// Route 1 -- Big Bad Hungry
	case ZmbSrcPageKind::kBridge_02:
		_routePathBand = 2;
		break; // dest Tunnels
	case ZmbSrcPageKind::kTunnels_03:
		_routePathBand = 3;
		break; // dest Pizza
	case ZmbSrcPageKind::kPizza_04:
		_routePathBand = 4;
		break; // dest BC1
	// Route 2 -- Who's Bayou
	case ZmbSrcPageKind::kBC1North_05:
		_routePathBand = 1;
		break; // dest Ferry
	case ZmbSrcPageKind::kFerry_07:
		_routePathBand = 2;
		break; // dest Lilly
	case ZmbSrcPageKind::kLilly_08:
		_routePathBand = 3;
		break; // dest Slides
	case ZmbSrcPageKind::kSlides_09:
		_routePathBand = 4;
		break; // dest BC2
	// Route 3 -- Deep Dark Forest
	case ZmbSrcPageKind::kBC1South_06:
		_routePathBand = 1;
		break; // dest Fleens
	case ZmbSrcPageKind::kFleens_10:
		_routePathBand = 2;
		break; // dest Hotel
	case ZmbSrcPageKind::kHotel_11:
		_routePathBand = 3;
		break; // dest Net
	case ZmbSrcPageKind::kNet_12:
		_routePathBand = 4;
		break; // dest BC2
	// Route 4 -- Mountain of Despair
	case ZmbSrcPageKind::kBC2_13:
		_routePathBand = 1;
		break; // dest Caves
	case ZmbSrcPageKind::kCaves_14:
		_routePathBand = 2;
		break; // dest Smoke
	case ZmbSrcPageKind::kSmoke_15:
		_routePathBand = 3;
		break; // dest Maze
	default:
		_routePathBand = 1;
		break;
	}
}

// Read a puzzle's completion level from game-state flags for a given SI page.
// Return the highest completed difficulty level from 0 through 4 as bit flags.
// State bits 0 through 3 represent levels 1 through 4, respectively.
//
// The level-flag array reserves its leading three bytes; puzzle slots occupy the remaining entries.
// Route-ending pages use their route flags instead.
uint16 ZoombiniTransitionXfer::readPuzzleLevelFlag(const ZmbStateFile &state, ZmbSrcPageKind siPage) {
	byte flag = 0;
	switch (siPage) {
	case ZmbSrcPageKind::kTown_00:
		return 1; // Always level 1
	// Big Bad Hungry puzzle pages.
	case ZmbSrcPageKind::kPicker_01:
	case ZmbSrcPageKind::kBridge_02:
	case ZmbSrcPageKind::kTunnels_03:
		flag = state._pageLevelFlags[static_cast<int16>(siPage) + 2];
		break;
	// BBH route completion flag
	case ZmbSrcPageKind::kPizza_04:
		flag = state._levelFlagRouteBigBadHungry;
		break;
	// Who's Bayou and Deep Dark Forest puzzle pages.
	case ZmbSrcPageKind::kBC1North_05:
	case ZmbSrcPageKind::kBC1South_06:
	case ZmbSrcPageKind::kFerry_07:
	case ZmbSrcPageKind::kLilly_08:
	case ZmbSrcPageKind::kSlides_09:
	case ZmbSrcPageKind::kFleens_10:
		flag = state._pageLevelFlags[static_cast<int16>(siPage) + 1];
		break;
	// WB route completion flag (low nibble)
	case ZmbSrcPageKind::kHotel_11:
		flag = state._levelFlagLoWhosBayouHiDeepDarkForest & 0x0F;
		break;
	// Deep Dark Forest and Mont Despair puzzle pages.
	case ZmbSrcPageKind::kNet_12:
	case ZmbSrcPageKind::kBC2_13:
	case ZmbSrcPageKind::kCaves_14:
		flag = state._pageLevelFlags[static_cast<int16>(siPage)];
		break;
	// MD route completion flag
	case ZmbSrcPageKind::kSmoke_15:
		flag = state._levelFlagRouteMontDespair;
		break;
	// DDF route completion flag (high nibble)
	case ZmbSrcPageKind::kMaze_16:
		flag = (state._levelFlagLoWhosBayouHiDeepDarkForest & 0xF0) >> 4;
		break;
	default:
		return 0;
	}

	// Convert 4-bit flag to highest completed level (0-4).
	uint16 level = 0;
	if (flag & 1)
		level = 1;
	if (flag & 2)
		level = 2;
	if (flag & 4)
		level = 3;
	if (flag & 8)
		level = 4;
	return level;
}

// Set color level 1-4 from the route's puzzle difficulty progression.
//
// For the first puzzle of each route (Bridge, Ferry, Fleens, Caves): use its route difficulty plus one.
//
// Subsequent puzzles use the preceding puzzle's completion level.
// @ref ZoombiniTransitionXfer::readPuzzleLevelFlag() reads that level.
void ZoombiniTransitionXfer::computeRoutePathColorLevel() {
	// Practice mode: all puzzles use the practice difficulty level directly.
	if (_vm->_state->inPracticeMode()) {
		_routePathColorLevel = CLIP<uint16>(_vm->_state->_practiceLevel, 1, 4);
		return;
	}

	const ZmbStateFile &state = _vm->_state->getCurrentState();
	uint16 colorLevel = 0;

	switch (_nextPageType) {
	// ---------------------------------------------------------------
	// First puzzle of each route: use destination's route level + 1.
	// ---------------------------------------------------------------
	case ZoombiniPageType::kBridge:
	case ZoombiniPageType::kFerry:
	case ZoombiniPageType::kFleens:
	case ZoombiniPageType::kCaves:
		colorLevel = _vm->_state->readPageRouteLevel(_nextPageType) + 1;
		break;
	// ---------------------------------------------------------------
	// Route1: predecessor puzzle's completion level.
	// Tunnels <- Picker, Pizza <- Bridge, BC1 <- Tunnels
	// ---------------------------------------------------------------
	case ZoombiniPageType::kTunnels:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kPicker_01);
		break;
	case ZoombiniPageType::kPizza:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kBridge_02);
		break;
	case ZoombiniPageType::kBasecamp1:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kTunnels_03);
		break;
	// ---------------------------------------------------------------
	// Route2: predecessor puzzle's completion level.
	// Lilly <- BC1N, Slides <- BC1S, BC2 <- Ferry (via Slides) or Fleens (via Net)
	// ---------------------------------------------------------------
	case ZoombiniPageType::kLilly:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kBC1North_05);
		break;
	case ZoombiniPageType::kSlides:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kBC1South_06);
		break;
	// ---------------------------------------------------------------
	// Route3: predecessor puzzle's completion level.
	// Hotel <- Lilly, Net <- Slides
	// ---------------------------------------------------------------
	case ZoombiniPageType::kHotel:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kLilly_08);
		break;
	case ZoombiniPageType::kNet:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kSlides_09);
		break;
	// ---------------------------------------------------------------
	// BC2: Slides -> BC2 reads Ferry's flag, Net -> BC2 reads Fleens's flag.
	// ---------------------------------------------------------------
	case ZoombiniPageType::kBasecamp2:
		if (_vm->_xferSrcPage == ZmbSrcPageKind::kSlides_09)
			colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kFerry_07);
		else
			colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kFleens_10);
		break;
	// ---------------------------------------------------------------
	// Route4: predecessor puzzle's completion level.
	// Smoke <- Net, Maze <- BC2
	// ---------------------------------------------------------------
	case ZoombiniPageType::kSmoke:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kNet_12);
		break;
	case ZoombiniPageType::kMaze:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kBC2_13);
		break;
	// ---------------------------------------------------------------
	// Town (XFER_5): Last predecessor = Caves.
	// ---------------------------------------------------------------
	case ZoombiniPageType::kTown:
		colorLevel = readPuzzleLevelFlag(state, ZmbSrcPageKind::kCaves_14);
		break;
	default:
		colorLevel = 1;
		break;
	}

	_routePathColorLevel = CLIP<uint16>(colorLevel, 1, 4);
}

// Pre-render shape callback on the MAIN SCRB: remaps hotspot shape indices
// based on puzzle completion so that completed bands show level-colored
// shape variants and uncompleted bands are hidden.
//
// Shape remapping formula: shapeIdx = slot + 4 * completionLevel
// E.g. band slot 5 at level 1 -> shape 9; at level 2 -> shape 13; etc.
// The tBMP contains pre-colored variants for each band and level combination.
void ZoombiniTransitionXfer::routeView_updateSlots(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	// Determine per-view parameters.
	int16 tableOffset, firstBandSlot, lastSlot;
	switch (_xferView) {
	case XferSceneId::kScene1_BigBadHungry:
		tableOffset = 0;
		firstBandSlot = 5;
		lastSlot = 8;
		break;
	case XferSceneId::kScene2_WhosBayou:
		tableOffset = 5;
		firstBandSlot = 6;
		lastSlot = 9;
		break;
	case XferSceneId::kScene3_DeepDarkForest:
		tableOffset = 10;
		firstBandSlot = 6;
		lastSlot = 9;
		break;
	case XferSceneId::kScene4_MountainOfDespair:
		tableOffset = 15;
		firstBandSlot = 6;
		lastSlot = 9;
		break;
	default:
		return;
	}

	// Build the shape remapping table through lastSlot.
	// Zero removes a hotspot; a positive entry selects its replacement shape.
	int16 shapeRemapTable[10] = {};

	for (int16 slotIdx = 1; slotIdx <= lastSlot; slotIdx++) {
		shapeRemapTable[slotIdx] = 0;

		if (slotIdx < firstBandSlot) {
			// Pre-slot (decoration) hotspot.
			if (tableOffset == 0) {
				// The first route view indexes the completion array directly.
				if (_puzzleCompletionArr[slotIdx])
					shapeRemapTable[slotIdx] = slotIdx;
			} else {
				int16 puzzleIdx = kRouteViewSlotTable[slotIdx + tableOffset];
				if (_puzzleCompletionArr[puzzleIdx]) {
					shapeRemapTable[slotIdx] = slotIdx;
				} else if (puzzleIdx == 11) {
					// Hotel slot -> show if Maze (index 16) was completed.
					if (_puzzleCompletionArr[16])
						shapeRemapTable[slotIdx] = slotIdx;
				} else if (puzzleIdx == 16 && _puzzleCompletionArr[11]) {
					// Maze slot -> show if Hotel (index 11) was completed.
					shapeRemapTable[slotIdx] = slotIdx;
				}
			}
		} else {
			// Band slot.
			int16 bandIdx = kRouteViewSlotTable[slotIdx + tableOffset - firstBandSlot + 2];

			int8 compLevel = _puzzleCompletionArr[bandIdx];
			if (0 < compLevel) {
				// Completed band: remap to level-colored shape variant.
				shapeRemapTable[slotIdx] = slotIdx + 4 * compLevel;
			} else {
				// Not completed or current band.
				_routeProgressLevel = MAX<int16>(_routeProgressLevel, 0);
				if (firstBandSlot == slotIdx) {
					// First band in this view: always use progress level.
					shapeRemapTable[slotIdx] = slotIdx + 4 * _routeProgressLevel;
				} else if (firstBandSlot < slotIdx &&
						   compLevel == -1 &&
						   lastSlot < shapeRemapTable[slotIdx - 1]) {
					// Cascading band: predecessor was also remapped above lastSlot threshold.
					shapeRemapTable[slotIdx] = slotIdx + 4 * _routeProgressLevel;
				}
			}
		}
	}

	// Apply remapping to hotspots.
	Common::Array<ZmbHotspot> remapped;
	for (uint hotspotIdx = 0; hotspotIdx < hotspots.size(); hotspotIdx++) {
		int16 shapeId = hotspots[hotspotIdx]._shapeIdx;
		if (shapeId <= 0 || lastSlot < shapeId)
			continue;
		if (shapeRemapTable[shapeId]) {
			hotspots[hotspotIdx]._shapeIdx = shapeRemapTable[shapeId];
			remapped.push_back(hotspots[hotspotIdx]);
		}
		// Omit entries that have no replacement shape.
	}
	hotspots = remapped;
}

// Pre-render shape callback: keep only the target band's hotspot in the
// copy, removing all others so only one shape renders.
void ZoombiniTransitionXfer::routePath_selectBand(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	if (!(1 <= _routePathBand && _routePathBand <= 4)) {
		error("xfer: invalid route-path band %d", _routePathBand);
		return;
	}

	uint16 targetIdx = _routePathBand - 1;
	if (hotspots.size() <= targetIdx) {
		error("xfer: malformed required route-path SCRB hotspot table");
		return;
	}

	// Keep only the target hotspot entry.
	ZmbHotspot target = hotspots[targetIdx];
	hotspots.clear();
	hotspots.push_back(target);
}

// Custom render callback for the path-overlay sub-feature.
// Modify the shape surface with the flood fill, then call @ref ZoombiniPage::blitShapes().
ZmbRenderResult ZoombiniTransitionXfer::routePath_onPostRender(ZmbFeature *feature) {
	if (!feature->isRenderActivated() &&
		feature->hasFlag(ZmbFeature::FLAG_01000000_DEFER_RENDER))
		return ZmbRenderResult::kSkipped;

	// Frame-interval gate.
	uint32 currentFrame = getCurrentFrameCounter();
	if (_routePathNextFrame <= currentFrame) {
		_routePathNextFrame = currentFrame + feature->getFrameInterval();

		if (_routePathCounter == 0) {
			// First active frame: initialize flood-fill grid from shape pixels.
			MohawkSurface *mohawkSurf = _vm->_gfx->findShape(feature->getResource(), _routePathBand);
			if (mohawkSurf) {
				Graphics::Surface *surf = mohawkSurf->getSurface();
				_routePathPixels = static_cast<byte *>(surf->getPixels());
				_routePathWidth = surf->w;
				_routePathHeight = surf->h;
				_routePathPitch = surf->pitch;

				// Seed index: band + 4 * view - 5.
				uint16 seedIdx = _routePathBand + 4 * static_cast<uint16>(_xferView) - 5;
				if (16 <= seedIdx)
					seedIdx = 0;
				int16 seedX = kRoutePathSeeds[seedIdx * 2];
				int16 seedY = kRoutePathSeeds[seedIdx * 2 + 1];

				// Select mark/replace palette indices from XferRoutePathLevelColor.
				// Level 1: "0x2E 0x2F 0x30 0x31" -> Back1/Back2 -> LevelOneColor1/Color2
				// Level 2: "0x30 0x31 0x32 0x33" -> LevelOneColor1/Color2 -> LevelTwoColor1/Color2
				// Level 3: "0x32 0x33 0x34 0x35" -> LevelTwoColor1/Color2 -> LevelThreeColor1/Color2
				// Level 4: "0x34 0x35 0x36 0x37" -> LevelThreeColor1/Color2 -> LevelFourColor1/Color2
				using Clr = ZoombiniGraphics::XferRoutePathLevelColor;
				byte mark1, mark2, replace1, replace2;
				switch (_routePathColorLevel) {
				case 1:
					mark1 = Clr::kRoutePathColor2E_LevelOneBack1;
					mark2 = Clr::kRoutePathColor2F_LevelOneBack2;
					replace1 = Clr::kRoutePathColor30_LevelOneColor1;
					replace2 = Clr::kRoutePathColor31_LevelOneColor2;
					break;
				case 2:
					mark1 = Clr::kRoutePathColor30_LevelOneColor1;
					mark2 = Clr::kRoutePathColor31_LevelOneColor2;
					replace1 = Clr::kRoutePathColor32_LevelTwoColor1;
					replace2 = Clr::kRoutePathColor33_LevelTwoColor2;
					break;
				case 3:
					mark1 = Clr::kRoutePathColor32_LevelTwoColor1;
					mark2 = Clr::kRoutePathColor33_LevelTwoColor2;
					replace1 = Clr::kRoutePathColor34_LevelThreeColor1;
					replace2 = Clr::kRoutePathColor35_LevelThreeColor2;
					break;
				case 4:
					mark1 = Clr::kRoutePathColor34_LevelThreeColor1;
					mark2 = Clr::kRoutePathColor35_LevelThreeColor2;
					replace1 = Clr::kRoutePathColor36_LevelFourColor1;
					replace2 = Clr::kRoutePathColor37_LevelFourColor2;
					break;
				default:
					error("xfer: invalid route path color level: %u", _routePathColorLevel);
					break;
				}

				routePath_initGrid(seedX, seedY, mark1, mark2, replace1, replace2);
			}
		} else {
			routePath_expandFloodFill(_routePathCounter);
		}

		_routePathCounter = MIN<uint32>(_routePathCounter + 7, 1000);
	}

	// Standard blit renders the (now modified) shape pixels.
	ZmbRenderResult result = blitShapes(feature);
	_midRouteInitialCompositeRendered = true;

	if (0 < _routePathRemainingPixels)
		addExternalDirtyRect(feature->getZSortRect());

	return result;
}

// Clear BFS queue, scan shape pixels replacing value 1->mark1 and 2->mark2,
// seed queue[0] with initial coordinates.
void ZoombiniTransitionXfer::routePath_initGrid(int16 seedX, int16 seedY, byte mark1, byte mark2, byte replace1, byte replace2) {
	// Clear BFS queue.
	for (int i = 0; i < kRoutePathQueueSize; i++) {
		_routePathQueueActive[i] = false;
		_routePathQueueX[i] = 0;
		_routePathQueueY[i] = 0;
	}

	_routePathTotalPixels = 0;
	_routePathMark1 = mark1;
	_routePathMark2 = mark2;
	_routePathReplace1 = replace1;
	_routePathReplace2 = replace2;

	// Replace source marker values 1 and 2 with the selected path colors.
	if (_routePathPixels) {
		byte *row = _routePathPixels;
		for (uint16 pathIdx = 0; pathIdx < _routePathHeight; pathIdx++) {
			for (uint16 rowIdx = 0; rowIdx < _routePathWidth; rowIdx++) {
				byte val = row[rowIdx];
				if (val == 1) {
					_routePathTotalPixels += 1;
					row[rowIdx] = mark1;
				}
				if (val == 2) {
					_routePathTotalPixels += 1;
					row[rowIdx] = mark2;
				}
			}
			row += _routePathPitch;
		}
	}

	_routePathRemainingPixels = _routePathTotalPixels;

	// Clamp seed coordinates to grid bounds.
	if (_routePathWidth <= static_cast<uint16>(seedX))
		seedX = _routePathWidth - 1;
	if (_routePathHeight <= static_cast<uint16>(seedY))
		seedY = _routePathHeight - 1;

	// Seed BFS queue slot 0.
	_routePathQueueActive[0] = true;
	_routePathQueueX[0] = seedX;
	_routePathQueueY[0] = seedY;
}

// BFS expansion: process active queue cells, expand to 8 neighbors.
// Repeat until remaining pixels <= target (progress threshold for this frame).
void ZoombiniTransitionXfer::routePath_expandFloodFill(uint32 counter) {
	if (!_routePathPixels || _routePathTotalPixels == 0)
		return;

	// Target remaining count: fewer pixels left as counter approaches 1000.
	uint32 target = _routePathTotalPixels - (counter * _routePathTotalPixels / 1000);
	while (target < _routePathRemainingPixels) {
		uint32 prevRemaining = _routePathRemainingPixels;

		for (int i = 0; i < kRoutePathQueueSize; i++) {
			if (!_routePathQueueActive[i])
				continue;

			_routePathQueueActive[i] = false;
			int16 x = _routePathQueueX[i];
			int16 y = _routePathQueueY[i];

			byte *pixel = _routePathPixels + x + _routePathPitch * y;

			// Expand south (y + 1)
			if (static_cast<uint16>(y + 1) < _routePathHeight) {
				byte *southRow = pixel + _routePathPitch;
				routePath_reserveSlot(y + 1, x, southRow);
				if (0 < x)
					routePath_reserveSlot(y + 1, x - 1, southRow - 1);
				if (static_cast<uint16>(x + 1) < _routePathWidth)
					routePath_reserveSlot(y + 1, x + 1, southRow + 1);
			}

			// Expand north (y - 1)
			// Y > 0 (bounds guard).
			if (0 < y) {
				byte *northRow = pixel - _routePathPitch;
				routePath_reserveSlot(y - 1, x, northRow);
				if (0 < x)
					routePath_reserveSlot(y - 1, x - 1, northRow - 1);
				if (static_cast<uint16>(x + 1) < _routePathWidth)
					routePath_reserveSlot(y - 1, x + 1, northRow + 1);
			}

			// Expand left/right (same row)
			if (0 < x)
				routePath_reserveSlot(y, x - 1, pixel - 1);
			if (static_cast<uint16>(x + 1) < _routePathWidth)
				routePath_reserveSlot(y, x + 1, pixel + 1);
		}

		// If no progress, force remaining to 0 to break the loop.
		if (prevRemaining == _routePathRemainingPixels) {
			_routePathRemainingPixels = 0;
			break;
		}
	}
}

// If *pixel matches mark1 or mark2, find an empty BFS queue slot,
// store (x, y), replace the pixel with the corresponding final color.
void ZoombiniTransitionXfer::routePath_reserveSlot(int16 y, int16 x, byte *pixel) {
	if (*pixel == _routePathMark1) {
		for (int i = 0; i < kRoutePathQueueSize; i++) {
			if (!_routePathQueueActive[i]) {
				_routePathQueueX[i] = x;
				_routePathQueueY[i] = y;
				_routePathQueueActive[i] = true;
				if (0 < _routePathRemainingPixels)
					_routePathRemainingPixels -= 1;
				*pixel = _routePathReplace1;
				return;
			}
		}
	} else if (*pixel == _routePathMark2) {
		for (int i = 0; i < kRoutePathQueueSize; i++) {
			if (!_routePathQueueActive[i]) {
				_routePathQueueX[i] = x;
				_routePathQueueY[i] = y;
				_routePathQueueActive[i] = true;
				if (0 < _routePathRemainingPixels)
					_routePathRemainingPixels -= 1;
				*pixel = _routePathReplace2;
				return;
			}
		}
	}
}

void ZoombiniTransitionXfer::close() {
	// The Xfer loader consumed the serialized active pack,
	// so rebuild it from the transition's real pack runners before opening the destination puzzle.
	// Without this handoff the puzzle receives an empty active pack and creates no Zoombini runners.
	ZmbStateActivePack &activePack = _vm->_state->getCurrentState()._zmbPackActive;
	if (_vm->_state->inPracticeMode() && !_vm->_debugPreserveActivePackOnXferClose) {
		activePack.clearEntries();
		activePack.setSkipOccupiedEntries(true);
		activePack.setSkipUnoccupiedEntries(true);
	} else {
		schedulePackSnoids(true, true);
		saveSnoidsToPack();
	}

	_routePathPixels = nullptr;
	_routePathFeature = nullptr;
	_vm->_debugPreserveActivePackOnXferClose = false;
	_vm->_xferSrcPage = ZmbSrcPageKind::kMinus1; // Reset for next xfer

	// Restore the fidget threshold after the transition.
	_vm->_fidgetThreshold = 64;

	_vm->setNextPage(_nextPageType);
	ZoombiniTransition::close();
}

} // End of namespace Mohawk
