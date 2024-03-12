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

#include "engines/nancy/action/datarecords.h"
#include "engines/nancy/action/inventoryrecords.h"
#include "engines/nancy/action/navigationrecords.h"
#include "engines/nancy/action/soundrecords.h"
#include "engines/nancy/action/miscrecords.h"

#include "engines/nancy/action/autotext.h"
#include "engines/nancy/action/conversation.h"
#include "engines/nancy/action/interactivevideo.h"
#include "engines/nancy/action/overlay.h"
#include "engines/nancy/action/secondaryvideo.h"
#include "engines/nancy/action/secondarymovie.h"

#include "engines/nancy/action/puzzle/assemblypuzzle.h"
#include "engines/nancy/action/puzzle/bballpuzzle.h"
#include "engines/nancy/action/puzzle/bulpuzzle.h"
#include "engines/nancy/action/puzzle/bombpuzzle.h"
#include "engines/nancy/action/puzzle/collisionpuzzle.h"
#include "engines/nancy/action/puzzle/cubepuzzle.h"
#include "engines/nancy/action/puzzle/hamradiopuzzle.h"
#include "engines/nancy/action/puzzle/leverpuzzle.h"
#include "engines/nancy/action/puzzle/mazechasepuzzle.h"
#include "engines/nancy/action/puzzle/mouselightpuzzle.h"
#include "engines/nancy/action/puzzle/orderingpuzzle.h"
#include "engines/nancy/action/puzzle/overridelockpuzzle.h"
#include "engines/nancy/action/puzzle/passwordpuzzle.h"
#include "engines/nancy/action/puzzle/peepholepuzzle.h"
#include "engines/nancy/action/puzzle/raycastpuzzle.h"
#include "engines/nancy/action/puzzle/riddlepuzzle.h"
#include "engines/nancy/action/puzzle/rippedletterpuzzle.h"
#include "engines/nancy/action/puzzle/rotatinglockpuzzle.h"
#include "engines/nancy/action/puzzle/safedialpuzzle.h"
#include "engines/nancy/action/puzzle/setplayerclock.h"
#include "engines/nancy/action/puzzle/sliderpuzzle.h"
#include "engines/nancy/action/puzzle/soundequalizerpuzzle.h"
#include "engines/nancy/action/puzzle/spigotpuzzle.h"
#include "engines/nancy/action/puzzle/tangrampuzzle.h"
#include "engines/nancy/action/puzzle/telephone.h"
#include "engines/nancy/action/puzzle/towerpuzzle.h"
#include "engines/nancy/action/puzzle/turningpuzzle.h"
#include "engines/nancy/action/puzzle/twodialpuzzle.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"

namespace Nancy {
namespace Action {

ActionRecord *ActionManager::createActionRecord(uint16 type, Common::SeekableReadStream *recordStream) {
	switch (type) {
	case 10:
		return new Hot1FrSceneChange(CursorManager::kHotspot);
	case 11:
		return new HotMultiframeSceneChange(CursorManager::kHotspot);
	case 12:
		return new SceneChange();
	case 13:
		return new HotMultiframeMultisceneChange();
	case 14:
		return new Hot1FrSceneChange(CursorManager::kExit);
	case 15:
		return new Hot1FrSceneChange(CursorManager::kMoveForward);
	case 16:
		return new Hot1FrSceneChange(CursorManager::kMoveBackward);
	case 17:
		return new Hot1FrSceneChange(CursorManager::kMoveUp);
	case 18:
		return new Hot1FrSceneChange(CursorManager::kMoveDown);
	case 19:
		return new HotMultiframeSceneChange(CursorManager::kMoveForward);
	case 20:
		if (g_nancy->getGameType() == kGameTypeVampire) {
			return new PaletteThisScene();
		} else {
			return new HotMultiframeSceneChange(CursorManager::kMoveUp);
		}
	case 21:
		if (g_nancy->getGameType() == kGameTypeVampire) {
			return new PaletteNextScene();
		} else {
			return new HotMultiframeSceneChange(CursorManager::kMoveDown);
		}
	case 22:
		return new Hot1FrSceneChange(CursorManager::kMoveLeft);
	case 23:
		return new Hot1FrSceneChange(CursorManager::kMoveRight);
	case 24:
		return new HotMultiframeMultisceneCursorTypeSceneChange();
	case 25: {
		// Weird case; instead of storing the cursor id, they instead chose to store
		// an AR id corresponding to one of the directional Hot1FrSceneChange variants.
		// Thus, we need to scan the incoming chunk and make another call to createActionRecord().
		// This is not the most elegant solution, but it works :)
		assert(recordStream);
		uint16 innerID = recordStream->readUint16LE();
		Hot1FrSceneChange *newRec = dynamic_cast<Hot1FrSceneChange *>(createActionRecord(innerID));
		assert(newRec);
		newRec->_isTerse = true;
		return newRec;
	}
	case 26:
		return new InteractiveVideo();
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
	case 52:
		return new PlaySecondaryVideo();
	case 53:
		return new PlaySecondaryMovie();
	case 54:
		if (g_nancy->getGameType() <= kGameTypeNancy1) {
			return new Overlay(false); // PlayStaticBitmapAnimation
		} else {
			return new Overlay(true);
		}
	case 55:
		if (g_nancy->getGameType() <= kGameTypeNancy1) {
			return new Overlay(true); // PlayIntStaticBitmapAnimation
		} else if (g_nancy->getGameType() >= kGameTypeNancy7) {
			return new OverlayStaticTerse();
		}
		return nullptr;
	case 56:
		if (g_nancy->getGameType() <= kGameTypeNancy6) {
			return new ConversationVideo();
		} else {
			return new OverlayAnimTerse();
		}
		return nullptr;
	case 57:
		return new ConversationCel();
	case 58:
		return new ConversationSound();
	case 59:
		return new ConversationCelT();
	case 60:
		if (g_nancy->getGameType() <= kGameTypeNancy5) {
			// Only used in tvd and nancy1
			return new MapCall();
		} else {
			return new ConversationSoundT();
		}
	case 61:
		if (g_nancy->getGameType() <= kGameTypeNancy5) {
			// Only used in tvd and nancy1
			return new MapCallHot1Fr();
		} else {
			return new Autotext();
		}
	case 62:
		if (g_nancy->getGameType() <= kGameTypeNancy7) {
			return new MapCallHotMultiframe(); // TVD/nancy1 only
		} else {
			return new ConversationCelTerse(); // nancy8 and up
		}
	case 63:
		return new ConversationSoundTerse();
	case 65:
		return new TableIndexOverlay();
	case 66:
		return new TableIndexPlaySound();
	case 67:
		return new TableIndexSetValueHS();
	case 68:
		return new TextScroll(false);
	case 70:
		return new TextScroll(true); // AutotextEntryList
	case 71:
		return new ModifyListEntry(ModifyListEntry::kAdd);
	case 72:
		return new ModifyListEntry(ModifyListEntry::kDelete);
	case 73:
		return new ModifyListEntry(ModifyListEntry::kMark);
	case 75:
		return new TextBoxWrite();
	case 76:
		return new TextboxClear();
	case 77:
		return new SetValue();
	case 78:
		return new SetValueCombo();
	case 79:
		return new ValueTest();
	case 97:
		return new EventFlags(true);
	case 98:
		return new EventFlagsMultiHS(true, true);
	case 99:
		return new EventFlagsMultiHS(true);
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
		return new EventFlagsMultiHS(false);
	case 107:
		return new EventFlags();
	case 108:
		if (g_nancy->getGameType() <= kGameTypeNancy6) {
			return new OrderingPuzzle(OrderingPuzzle::kOrdering);
		} else {
			return new GotoMenu();
		}
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
		return new Telephone(false);
	case 117:
		return new SliderPuzzle();
	case 118:
		return new PasswordPuzzle();
	case 119:
		if (g_nancy->getGameType() >= kGameTypeNancy7) {
			// This got moved in nancy7
			return new OrderingPuzzle(OrderingPuzzle::kOrdering);
		}
		return nullptr;
	case 120:
		return new AddInventoryNoHS();
	case 121:
		return new RemoveInventoryNoHS();
	case 122:
		return new ShowInventoryItem();
	case 123:
		return new InventorySoundOverride();
	case 124:
		return new EnableDisableInventory();
	case 125:
		return new PopInvViewPriorScene();
	case 126:
		return new GoInvViewScene();
	case 140:
		return new SetVolume();
	case 150:
		return new PlaySound();
	case 151:
		if (g_nancy->getGameType() <= kGameTypeNancy6)  {
			return new PlaySound(); // PlayStreamSound
		} else {
			return new PlayRandomSoundTerse();
		}
	case 152:
		return new PlaySoundFrameAnchor();
	case 153:
		return new PlaySoundMultiHS();
	case 154:
		return new StopSound();
	case 155:
		return new StopSound(); // StopAndUnloadSound, but we always unload
	case 157:
		return new PlaySoundCC();
	case 158:
		return new PlayRandomSound();
	case 159:
		return new PlaySoundTerse();
	case 160:
		return new HintSystem();
	case 161:
		return new PlaySoundEventFlagTerse();
	case 170:
		return new SetPlayerClock();
	case 200:
		return new SoundEqualizerPuzzle();
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
	case 206:
		return new RaycastPuzzle();
	case 207:
		return new TangramPuzzle();
	case 208:
		return new OrderingPuzzle(OrderingPuzzle::PuzzleType::kPiano);
	case 209:
		return new TurningPuzzle();
	case 210:
		return new SafeDialPuzzle();
	case 211:
		return new CollisionPuzzle(CollisionPuzzle::PuzzleType::kCollision);
	case 212:
		return new OrderingPuzzle(OrderingPuzzle::PuzzleType::kOrderItems);
	case 213:
		return new CollisionPuzzle(CollisionPuzzle::PuzzleType::kTileMove);
	case 214:
		return new OrderingPuzzle(OrderingPuzzle::PuzzleType::kKeypad);
	case 215:
		return new MazeChasePuzzle();
	case 216:
		return new PeepholePuzzle();
	case 217:
		return new MouseLightPuzzle();
	case 218:
		return new BulPuzzle();
	case 219:
		return new BBallPuzzle();
	case 220:
		return new TwoDialPuzzle();
	case 221:
		return new HamRadioPuzzle();
	case 222:
		return new AssemblyPuzzle();
	case 223:
		return new CubePuzzle();
	case 224:
		return new OrderingPuzzle(OrderingPuzzle::kKeypadTerse);
	case 225:
		return new SpigotPuzzle();
	case 230:
		return new Telephone(true);
	default:
		return nullptr;
	}
}

} // End of namespace Action
} // End of namespace Nancy
