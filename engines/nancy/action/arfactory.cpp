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

// TODO put this function in a subclass
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
	case 0x0C:
		return new StartFrameNextScene();
	case 0x14:
		return new StartStopPlayerScrolling(); // TODO
	case 0x15:
		return new StartStopPlayerScrolling(); // TODO
	case 0x1E:
		return new LightningOn();
	case 0x1F:
		return new LightningOff();
	case 0x20:
		return new AmbientLightUp();
	case 0x21:
		return new AmbientLightDown();
	case 0x22:
		return new AmbientLightToTod();
	case 0x23:
		return new AmbientLightToTodOff();
	case 0x24:
		return new FlickerOn();
	case 0x25:
		return new FlickerOff();
	case 0x28:
		return new PlayPrimaryVideoChan0(NancySceneState.getViewport());
	case 0x29:
		return new PlaySecondaryVideo(0, NancySceneState.getViewport());
	case 0x2A:
		return new PlaySecondaryVideo(1, NancySceneState.getViewport());
	case 0x2B:
		return new PlaySecondaryMovie(NancySceneState.getViewport());
	case 0x2C:
		return new PlayStaticBitmapAnimation(false, NancySceneState.getViewport()); // PlayStaticBitmapAnimation
	case 0x2D:
		return new PlayStaticBitmapAnimation(true, NancySceneState.getViewport()); // PlayIntStaticBitmapAnimation
	case 0x32:
		return new MapCall();
	case 0x33:
		return new MapCallHot1Fr();
	case 0x34:
		return new MapCallHotMultiframe();
	case 0x35:
		return new MapLocationAccess();
	case 0x36:
		return new MapLightning();
	case 0x37:
		return new MapLightningOff();
	case 0x38:
		return new MapSound();
	case 0x39:
		return new MapAviOverride();
	case 0x3A:
		return new MapAviOverrideOff();
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
		return new OrderingPuzzle(NancySceneState.getViewport());
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
		return new RotatingLockPuzzle(NancySceneState.getViewport());
	case 0x69:
		return new LeverPuzzle(NancySceneState.getViewport());
	case 0x6A:
		return new Telephone(NancySceneState.getViewport());
	case 0x6B:
		return new SliderPuzzle(NancySceneState.getViewport());
	case 0x6C:
		return new PasswordPuzzle(NancySceneState.getViewport());
	case 0x6E:
		return new AddInventoryNoHS();
	case 0x6F:
		return new RemoveInventoryNoHS();
	case 0x70:
		return new ShowInventoryItem(NancySceneState.getViewport());
	case 0x8C:
		return new PlayDigiSoundAndDie(); // TODO
	case 0x8D:
		return new PlayDigiSoundAndDie(); // TODO
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
