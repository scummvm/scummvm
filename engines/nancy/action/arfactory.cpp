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
#include "engines/nancy/action/primaryvideo.h"
#include "engines/nancy/action/secondaryvideo.h"
#include "engines/nancy/action/secondarymovie.h"
#include "engines/nancy/action/staticbitmapanim.h"
#include "engines/nancy/action/orderingpuzzle.h"
#include "engines/nancy/action/rotatinglockpuzzle.h"
#include "engines/nancy/action/telephone.h"
#include "engines/nancy/action/sliderpuzzle.h"
#include "engines/nancy/action/passwordpuzzle.h"
#include "engines/nancy/action/leverpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

ActionRecord *ActionManager::createActionRecord(uint16 type) {
	type -= 0xA;
	switch (type) {
	case 0x00:
		return new Hot1FrSceneChange();
	case 0x01:
		return new HotMultiframeSceneChange();
	case 0x02:
		return new SceneChange();
	case 0x03:
		return new HotMultiframeMultisceneChange();
	case 0x04:
		return new Hot1FrExitSceneChange();
	case 0x0A:
		return new PaletteThisScene();
	case 0x0B:
		return new PaletteNextScene();
	case 0x1E:
		return new LightningOn();
	case 0x28:
		return new PlayPrimaryVideoChan0();
	case 0x29:
		return new PlaySecondaryVideo(0);
	case 0x2A:
		return new PlaySecondaryVideo(1);
	case 0x2B:
		return new PlaySecondaryMovie();
	case 0x2C:
		return new PlayStaticBitmapAnimation(false); // PlayStaticBitmapAnimation
	case 0x2D:
		return new PlayStaticBitmapAnimation(true); // PlayIntStaticBitmapAnimation
	case 0x32:
		return new MapCall();
	case 0x33:
		return new MapCallHot1Fr();
	case 0x34:
		return new MapCallHotMultiframe();
	case 0x41:
		return new TextBoxWrite();
	case 0x42:
		return new TextBoxClear();
	case 0x5A:
		return new BumpPlayerClock();
	case 0x5B:
		return new SaveContinueGame();
	case 0x5C:
		return new TurnOffMainRendering();
	case 0x5D:
		return new TurnOnMainRendering();
	case 0x5E:
		return new ResetAndStartTimer();
	case 0x5F:
		return new StopTimer();
	case 0x60:
		return new EventFlagsMultiHS();
	case 0x61:
		return new EventFlags();
	case 0x62:
		return new OrderingPuzzle();
	case 0x63:
		return new LoseGame();
	case 0x64:
		return new PushScene();
	case 0x65:
		return new PopScene();
	case 0x66:
		return new WinGame();
	case 0x67:
		return new DifficultyLevel();
	case 0x68:
		return new RotatingLockPuzzle();
	case 0x69:
		return new LeverPuzzle();
	case 0x6A:
		return new Telephone();
	case 0x6B:
		return new SliderPuzzle();
	case 0x6C:
		return new PasswordPuzzle();
	case 0x6E:
		return new AddInventoryNoHS();
	case 0x6F:
		return new RemoveInventoryNoHS();
	case 0x70:
		return new ShowInventoryItem();
	case 0x8C:
		return new PlayDigiSoundAndDie();
	case 0x8D:
		return new PlayDigiSoundAndDie();
	case 0x8E:
		return new PlaySoundPanFrameAnchorAndDie();
	case 0x8F:
		return new PlaySoundMultiHS();
	case 0x96:
		return new HintSystem();
	default:
		error("Action Record type %i is invalid!", type+0xA);
		return nullptr;
	}
}

} // End of namespace Action
} // End of namespace Nancy
