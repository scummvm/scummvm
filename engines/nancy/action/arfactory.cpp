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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/action/recordtypes.h"
#include "engines/nancy/action/conversation.h"
#include "engines/nancy/action/secondaryvideo.h"
#include "engines/nancy/action/secondarymovie.h"
#include "engines/nancy/action/overlay.h"
#include "engines/nancy/action/orderingpuzzle.h"
#include "engines/nancy/action/rotatinglockpuzzle.h"
#include "engines/nancy/action/telephone.h"
#include "engines/nancy/action/sliderpuzzle.h"
#include "engines/nancy/action/passwordpuzzle.h"
#include "engines/nancy/action/leverpuzzle.h"
#include "engines/nancy/action/rippedletterpuzzle.h"
#include "engines/nancy/action/towerpuzzle.h"
#include "engines/nancy/action/riddlepuzzle.h"
#include "engines/nancy/action/overridelockpuzzle.h"
#include "engines/nancy/action/bombpuzzle.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"

namespace Nancy {
namespace Action {

ActionRecord *ActionManager::createActionRecord(uint16 type) {
	switch (type) {
	case 10:
		return new Hot1FrSceneChange();
	case 11:
		return new HotMultiframeSceneChange();
	case 12:
		return new SceneChange();
	case 13:
		return new HotMultiframeMultisceneChange();
	case 14:
		return new Hot1FrExitSceneChange();
	case 20:
		return new PaletteThisScene();
	case 21:
		return new PaletteNextScene();
	case 40:
		if (g_nancy->getGameType() < kGameTypeNancy2) {
			// Only used in TVD
			return new LightningOn();
		} else {
			return new SpecialEffect();
		}		
	case 50:
		return new ConversationVideo(); // PlayPrimaryVideoChan0
	case 51:
		return new PlaySecondaryVideo(0);
	case 52:
		return new PlaySecondaryVideo(1);
	case 53:
		return new PlaySecondaryMovie();
	case 54:
		return new Overlay(false); // PlayStaticBitmapAnimation
	case 55:
		return new Overlay(true); // PlayIntStaticBitmapAnimation
	case 56:
		return new ConversationVideo();
	case 57:
		return new ConversationCel();
	case 58:
		return new ConversationSound();
	case 60:
		return new MapCall();
	case 61:
		return new MapCallHot1Fr();
	case 62:
		return new MapCallHotMultiframe();
	case 75:
		return new TextBoxWrite();
	case 76:
		return new TextBoxClear();
	case 100:
		return new BumpPlayerClock();
	case 101:
		return new SaveContinueGame();
	case 102:
		return new TurnOffMainRendering();
	case 103:
		return new TurnOnMainRendering();
	case 104:
		return new ResetAndStartTimer();
	case 105:
		return new StopTimer();
	case 106:
		return new EventFlagsMultiHS();
	case 107:
		return new EventFlags();
	case 108:
		return new OrderingPuzzle();
	case 109:
		return new LoseGame();
	case 110:
		return new PushScene();
	case 111:
		return new PopScene();
	case 112:
		return new WinGame();
	case 113:
		return new DifficultyLevel();
	case 114:
		return new RotatingLockPuzzle();
	case 115:
		return new LeverPuzzle();
	case 116:
		return new Telephone();
	case 117:
		return new SliderPuzzle();
	case 118:
		return new PasswordPuzzle();
	case 120:
		return new AddInventoryNoHS();
	case 121:
		return new RemoveInventoryNoHS();
	case 122:
		return new ShowInventoryItem();
	case 150:
		return new PlayDigiSoundAndDie();
	case 151:
		return new PlayDigiSoundAndDie();
	case 152:
		return new PlaySoundPanFrameAnchorAndDie();
	case 153:
		return new PlaySoundMultiHS();
	case 160:
		return new HintSystem();
	case 201:
		return new TowerPuzzle();
	case 202:
		return new BombPuzzle();
	case 203:
		return new RippedLetterPuzzle();
	case 204:
		return new OverrideLockPuzzle();
	case 205:
		return new RiddlePuzzle();
	default:
		error("Action Record type %i is invalid!", type);
		return nullptr;
	}
}

} // End of namespace Action
} // End of namespace Nancy
