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

#include "engines/nancy/action/puzzle/angletosspuzzle.h"
#include "engines/nancy/action/puzzle/arcadepuzzle.h"
#include "engines/nancy/action/puzzle/assemblypuzzle.h"
#include "engines/nancy/action/puzzle/bballpuzzle.h"
#include "engines/nancy/action/puzzle/beadpuzzle.h"
#include "engines/nancy/action/puzzle/boardgamepuzzle.h"
#include "engines/nancy/action/puzzle/bulpuzzle.h"
#include "engines/nancy/action/puzzle/bombpuzzle.h"
#include "engines/nancy/action/puzzle/cardgamepuzzle.h"
#include "engines/nancy/action/puzzle/collisionpuzzle.h"
#include "engines/nancy/action/puzzle/cubepuzzle.h"
#include "engines/nancy/action/puzzle/cuttingpuzzle.h"
#include "engines/nancy/action/puzzle/dotconnectpuzzle.h"
#include "engines/nancy/action/puzzle/gridmappuzzle.h"
#include "engines/nancy/action/puzzle/matchpuzzle.h"
#include "engines/nancy/action/puzzle/hamradiopuzzle.h"
#include "engines/nancy/action/puzzle/leverpuzzle.h"
#include "engines/nancy/action/puzzle/magnetmazepuzzle.h"
#include "engines/nancy/action/puzzle/mazechasepuzzle.h"
#include "engines/nancy/action/puzzle/memorypuzzle.h"
#include "engines/nancy/action/puzzle/mindpuzzle.h"
#include "engines/nancy/action/puzzle/minigolfpuzzle.h"
#include "engines/nancy/action/puzzle/mirrorlightpuzzle.h"
#include "engines/nancy/action/puzzle/mouselightpuzzle.h"
#include "engines/nancy/action/puzzle/multibuildpuzzle.h"
#include "engines/nancy/action/puzzle/onebuildpuzzle.h"
#include "engines/nancy/action/puzzle/orderingpuzzle.h"
#include "engines/nancy/action/puzzle/overridelockpuzzle.h"
#include "engines/nancy/action/puzzle/passwordpuzzle.h"
#include "engines/nancy/action/puzzle/peepholepuzzle.h"
#include "engines/nancy/action/puzzle/quizpuzzle.h"
#include "engines/nancy/action/puzzle/raycastpuzzle.h"
#include "engines/nancy/action/puzzle/riddlepuzzle.h"
#include "engines/nancy/action/puzzle/rippedletterpuzzle.h"
#include "engines/nancy/action/puzzle/rotatinglockpuzzle.h"
#include "engines/nancy/action/puzzle/safedialpuzzle.h"
#include "engines/nancy/action/puzzle/setplayerclock.h"
#include "engines/nancy/action/puzzle/sliderpuzzle.h"
#include "engines/nancy/action/puzzle/sortpuzzle.h"
#include "engines/nancy/action/puzzle/soundequalizerpuzzle.h"
#include "engines/nancy/action/puzzle/soundmatchpuzzle.h"
#include "engines/nancy/action/puzzle/spigotpuzzle.h"
#include "engines/nancy/action/puzzle/tangrampuzzle.h"
#include "engines/nancy/action/puzzle/telephone.h"
#include "engines/nancy/action/puzzle/towerpuzzle.h"
#include "engines/nancy/action/puzzle/turningpuzzle.h"
#include "engines/nancy/action/puzzle/twodialpuzzle.h"
#include "engines/nancy/action/puzzle/typingquizpuzzle.h"
#include "engines/nancy/action/puzzle/whalesurvivorpuzzle.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"

namespace Nancy {
namespace Action {

ActionRecord *ActionManager::createActionRecord(uint16 type, Common::SeekableReadStream *recordStream) {
	switch (type) {
	case 10:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new Hot1FrSceneChange(CursorManager::kHotspot);
		else
			return new SceneChange();	// Moved from 12 in Nancy10
	case 11:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kHotspot);
		else
			return new Hot1FrSceneChange(CursorManager::kNormal, true, true);
	case 12:
		if (g_nancy->getGameType() <= kGameTypeNancy9) {
			return new SceneChange();
		} else {
			return new HotMultiframeSceneChange(CursorManager::kNormal, true);
		}
	case 13:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeMultiSceneChange();
		else
			return new Hot1FrSceneChange(CursorManager::kHotspot);
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
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kMoveForward);
		else
			return new Hot1FrSceneChange(CursorManager::kMoveLeft);		// Moved from 22 in Nancy10
	case 20:
		if (g_nancy->getGameType() == kGameTypeVampire)
			return new PaletteThisScene();
		else if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kMoveUp);
		else
			return new Hot1FrSceneChange(CursorManager::kMoveRight);	// Moved from 23 in Nancy10
	case 21:
		if (g_nancy->getGameType() == kGameTypeVampire)
			return new PaletteNextScene();
		else if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kMoveDown);
		else
			return new HotSingleFrameSceneChange();
	case 22:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new Hot1FrSceneChange(CursorManager::kMoveLeft);
		else
			return new HotMultiframeSceneChange(CursorManager::kHotspot);		// Moved from 11 in Nancy 10
	case 23:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new Hot1FrSceneChange(CursorManager::kMoveRight);
		else
			return new HotMultiframeSceneChange(CursorManager::kMoveForward);	// Moved from 19 in Nancy 10
	case 24:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeMultiSceneCursorTypeSceneChange();
		else
			return new HotMultiframeSceneChange(CursorManager::kMoveUp);		// Moved from 20 in Nancy 10
	case 25: {
		if (g_nancy->getGameType() <= kGameTypeNancy9) {
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
		} else {
			return new HotMultiframeSceneChange(CursorManager::kMoveDown);		// Moved from 21 in Nancy 10
		}
	}
	case 26:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new InteractiveVideo();
		else
			return new HotMultiframeMultiSceneChange();	// Moved from 13 in Nancy 10
	case 27:
		return new HotMultiframeMultiSceneCursorTypeSceneChange(); // Moved from 24 to 27 in Nancy10
	case 28:
		return new InteractiveVideo();	// Moved from 26 to 28 in Nancy10
	case 29:	// Nancy10
		return new ControlUIItems();
	case 30:	// Nancy11
		return new StopPlayerScrolling();
	case 31:	// Nancy11
		return new StartPlayerScrolling();
	case 32:	// Nancy10
		return new UIPopupPrepScene();
	case 45:	// Nancy11 - random-movie variant of PlaySecondaryMovie
		return new PlaySecondaryMovie(true);
	case 46:	// Nancy11
		return new PlayRandomMovieControl();
	case 40:
		if (g_nancy->getGameType() <= kGameTypeNancy1)
			return new LightningOn();	// Only used in TVD
		else
			return new SpecialEffect();
	case 50:
		return new ConversationVideo(); // PlayPrimaryVideoChan0
	case 51:
	case 52:
		return new PlaySecondaryVideo();
	case 53:
		return new PlaySecondaryMovie();
	case 54:
		if (g_nancy->getGameType() <= kGameTypeNancy1)
			return new Overlay(false); // PlayStaticBitmapAnimation
		else
			return new Overlay(true);
	case 55:
		if (g_nancy->getGameType() <= kGameTypeNancy1)
			return new Overlay(true); // PlayIntStaticBitmapAnimation
		else if (g_nancy->getGameType() >= kGameTypeNancy7)
			return new OverlayStaticTerse();
		else
			return nullptr;
	case 56:
		if (g_nancy->getGameType() <= kGameTypeNancy6)
			return new ConversationVideo();
		else
			return new OverlayAnimTerse();
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
		if (g_nancy->getGameType() <= kGameTypeNancy7)
			return new MapCallHotMultiframe(); // TVD/nancy1 only
		else
			return new ConversationCelTerse(); // nancy8 and up
	case 63:
		return new ConversationSoundTerse();
	case 65:
		return new TableIndexOverlay();
	case 66:
		return new TableIndexPlaySound();
	case 67:
		if (g_nancy->getGameType() >= kGameTypeNancy10)
			return new Autotext();		// Moved from 61 in Nancy 10
		else
			return new TableIndexSetValueHS();
	case 68:
		return new TextScroll(false);
	case 69:	// Nancy11
		return new TimerControl();
	case 70:
		return new TextScroll(true); // AutotextEntryList
	case 71:
		return new ModifyListEntry(ModifyListEntry::kAdd);
	case 72:
		return new ModifyListEntry(ModifyListEntry::kDelete);
	case 73:
		return new ModifyListEntry(ModifyListEntry::kMark);
	case 74:	// Nancy10
		return new FrameTextBox(true);
	case 75:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new TextBoxWrite();
		return new FrameTextBox(false);
	case 76:
		return new TextboxClear();
	case 77:
		return new SetValue();
	case 78:
		return new SetValueCombo();
	case 79:
		return new ValueTest();
	case 81:	// Nancy11
		return new TextBoxWrite(true);
	case 94:
		// Nancy12: moved from 106
		if (g_nancy->getGameType() >= kGameTypeNancy12)
			return new EventFlagsMultiHS(false);
		return nullptr;
	case 95:
		// Nancy12: moved from 107
		if (g_nancy->getGameType() >= kGameTypeNancy12)
			return new EventFlags();
		return nullptr;
	case 96:	// Nancy11
		return new RandomizeEventFlags();
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
		if (g_nancy->getGameType() <= kGameTypeNancy6)
			return new OrderingPuzzle(OrderingPuzzle::kOrdering);
		else
			return new GotoMenu();
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
	case 128:	// Nancy10
		return new CellPhonePopCellSceneFromStack();
	case 129:	// Nancy10
		return new SetCellPhoneBatteryAndSignal();
	case 130:	// Nancy10
		return new ChangeCellPhoneInfo();
	case 131:	// Nancy10
		return new AddSearchLink();
	case 132:	// Nancy12
		return new ResourceUse();
	case 140:
		if (g_nancy->getGameType() >= kGameTypeNancy12)
			return new SetPlayerClock();	// Moved from 170 in Nancy12
		else
			return new SetVolume();			// Legacy SetVolume slot (used up to Nancy8)
	case 141:
		// MakeScreenFile, moved here from 148 in Nancy12.
		// Saves a cropped image of the screen to a bitmap/TGA file.
		// TODO: debug-only feature, not implemented
		return nullptr;
	case 145:
		// Nancy13 renumbered the sound-playing AR block downwards; PlaySound
		// moved here (it was 150 up to Nancy12).
		// TODO: verify the Nancy13 PlaySound on-disk layout matches PlaySound::readData.
		if (g_nancy->getGameType() >= kGameTypeNancy13)
			return new PlaySound();
		return nullptr;
	case 146:
		// Nancy13: FadeSoundToSilence moved here (was 147).
		if (g_nancy->getGameType() >= kGameTypeNancy13)
			return new FadeSoundToSilence();
		return nullptr;
	case 147:
		if (g_nancy->getGameType() >= kGameTypeNancy13)
			return new SetVolume();			// Nancy13: SetVolume moved here (was 148)
		return new FadeSoundToSilence();	// Nancy11
	case 148:
		if (g_nancy->getGameType() >= kGameTypeNancy13)
			return new StopSound();	// Nancy13: StopSound moved here (was 154)
		if (g_nancy->getGameType() >= kGameTypeNancy12)
			return new SetVolume();	// Moved from 149 in Nancy12
		// MakeScreenFile - seems to save a cropped image of the screen in a bitmap file?
		// TODO: Used in Nancy 9, sand castle puzzle. Moved to 141 in Nancy12.
		return nullptr;
	case 149:
		if (g_nancy->getGameType() >= kGameTypeNancy13)
			return new StopSound();	// Nancy13: StopAndUnloadSound moved here (was 155)
		if (g_nancy->getGameType() >= kGameTypeNancy12)
			return new PlaySoundEventFlagTerse();	// Moved from 161 in Nancy12
		else if (g_nancy->getGameType() >= kGameTypeNancy9)
			return new SetVolume();	// Moved from 140 in Nancy9, then to 148 in Nancy12
		else
			return nullptr;
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
	case 156:	// Nancy11
		return new Update3DSound();
	case 157:
		return new PlaySoundCC();
	case 158:
		return new PlayRandomSound();
	case 159:
		return new PlaySoundTerse();
	case 160:
		// In Nancy12 the hint system was removed (the HINT boot chunk is gone) and this
		// slot was reused for a new driving/racing puzzle.
		// TODO: Nancy12 DrivingPuzzle (new), not implemented
		if (g_nancy->getGameType() >= kGameTypeNancy12)
			return nullptr;
		return new HintSystem();
	case 161:
		// PlaySoundEventFlagTerse moved to 149 in Nancy12; this slot was reused for a new puzzle.
		if (g_nancy->getGameType() >= kGameTypeNancy12)
			return new MinigolfPuzzle();
		return new PlaySoundEventFlagTerse();
	// -- Nancy 12 new puzzles/action records --
	case 162:
		// TODO: Nancy12 - sewing machine puzzle.
		return nullptr;
	case 163:
		return new MirrorLightPuzzle();
	case 164:
		return new BoardGamePuzzle();
	case 165:
		return new MindPuzzle();
	case 166:
		// OneBuildPuzzle, moved here from 234 in Nancy12
		return new OneBuildPuzzle();
	case 167:
		// TODO: Nancy12 ChasePuzzle (new), not implemented
		return nullptr;
	case 168:
		return new Set3DSoundListenerPosition();
	// -- Nancy 13 new/relocated puzzles (types 169-176) --
	case 169:
		// StepObjectsPuzzle, new in Nancy13
		// TODO: not yet implemented
		return nullptr;
	case 170:
		if (g_nancy->getGameType() >= kGameTypeNancy13) {
			// WordFindPuzzle, new in Nancy13. This reuses the slot that used
			// to hold SetPlayerClock (which itself moved to 140 in Nancy12).
			// TODO: not yet implemented
			return nullptr;
		}
		// SetPlayerClock lived here up to Nancy11; moved to 140 in Nancy12
		return new SetPlayerClock();
	case 171:
		// TurningPuzzle, moved here from 209 in Nancy13.
		if (g_nancy->getGameType() >= kGameTypeNancy13)
			return new TurningPuzzle();
		return nullptr;
	case 172:
		// BlocksPuzzle, new in Nancy13
		// TODO: not yet implemented
		return nullptr;
	case 173:
		// PegsPuzzle, new in Nancy13
		// TODO: not yet implemented
		return nullptr;
	case 174:
		// Unknown Puzzle, new in Nancy13
		// TODO: not yet implemented
		return nullptr;
	case 175:
		// Unknown Puzzle, new in Nancy13
		// TODO: not yet implemented
		return nullptr;
	case 176:
		// Unknown Puzzle, new in Nancy13
		// TODO: not yet implemented
		return nullptr;
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
	// -- Nancy 8 and up --
	case 226:
		return new CuttingPuzzle();
	case 228:
		return new MatchPuzzle();
	case 229:
		return new ArcadePuzzle();
	case 230:
		return new Telephone(true);
	case 231:
		return new QuizPuzzle();
	case 232:
		return new AngleTossPuzzle();
	// -- Nancy 9 and up --
	case 233:
		return new SoundMatchPuzzle();
	case 234:
		// Moved to 166 in Nancy12
		return new OneBuildPuzzle();
	case 235:
		return new MultiBuildPuzzle();
	case 237:
		return new WhaleSurvivorPuzzle();
	case 238:
		return new MemoryPuzzle();
	// -- Nancy 10 and up --
	case 239:
		return new SortPuzzle();
	case 241:
		return new DotConnectPuzzle();
	case 242:
		return new MagnetMazePuzzle();
	case 243:
		return new BeadPuzzle();
	case 244:
		return new GridMapPuzzle();
	// -- Nancy 11 and up --
	case 245:
		return new TypingQuizPuzzle();
	case 246:
		return new CardGamePuzzle();
	default:
		warning("Unknown action record type %d", type);
		return nullptr;
	}
}

} // End of namespace Action
} // End of namespace Nancy
