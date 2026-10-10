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
#include "engines/nancy/action/autotextscroll.h"
#include "engines/nancy/action/cameraaction.h"
#include "engines/nancy/action/conversation.h"
#include "engines/nancy/action/interactivevideo.h"
#include "engines/nancy/action/overlay.h"
#include "engines/nancy/action/secondaryvideo.h"
#include "engines/nancy/action/secondarymovie.h"
#include "engines/nancy/action/typewritertext.h"
#include "engines/nancy/action/wolfcommands.h"

#include "engines/nancy/action/puzzle/adjustpuzzle.h"
#include "engines/nancy/action/puzzle/analogclock.h"
#include "engines/nancy/action/puzzle/angletosspuzzle.h"
#include "engines/nancy/action/puzzle/arcadepuzzle.h"
#include "engines/nancy/action/puzzle/assemblypuzzle.h"
#include "engines/nancy/action/puzzle/bballpuzzle.h"
#include "engines/nancy/action/puzzle/beadpuzzle.h"
#include "engines/nancy/action/puzzle/blockingpuzzle.h"
#include "engines/nancy/action/puzzle/blockspuzzle.h"
#include "engines/nancy/action/puzzle/boardgamepuzzle.h"
#include "engines/nancy/action/puzzle/buildpuzzle.h"
#include "engines/nancy/action/puzzle/bulpuzzle.h"
#include "engines/nancy/action/puzzle/bombpuzzle.h"
#include "engines/nancy/action/puzzle/booksortpuzzle.h"
#include "engines/nancy/action/puzzle/cardgamepuzzle.h"
#include "engines/nancy/action/puzzle/chesschatpuzzle.h"
#include "engines/nancy/action/puzzle/collisionpuzzle.h"
#include "engines/nancy/action/puzzle/cookingpuzzle.h"
#include "engines/nancy/action/puzzle/cubepuzzle.h"
#include "engines/nancy/action/puzzle/cuttingpuzzle.h"
#include "engines/nancy/action/puzzle/dancepuzzle.h"
#include "engines/nancy/action/puzzle/decoderpuzzle.h"
#include "engines/nancy/action/puzzle/dialsequencepuzzle.h"
#include "engines/nancy/action/puzzle/dotconnectpuzzle.h"
#include "engines/nancy/action/puzzle/dragsurfacepuzzle.h"
#include "engines/nancy/action/puzzle/escapepuzzle.h"
#include "engines/nancy/action/puzzle/flashmemorypuzzle.h"
#include "engines/nancy/action/puzzle/drivingpuzzle.h"
#include "engines/nancy/action/puzzle/dropsortpuzzle.h"
#include "engines/nancy/action/puzzle/escapegridpuzzle.h"
#include "engines/nancy/action/puzzle/fishingpuzzle.h"
#include "engines/nancy/action/puzzle/foxgeesepuzzle.h"
#include "engines/nancy/action/puzzle/goldbergpuzzle.h"
#include "engines/nancy/action/puzzle/gridmappuzzle.h"
#include "engines/nancy/action/puzzle/magicboxpuzzle.h"
#include "engines/nancy/action/puzzle/matchpuzzle.h"
#include "engines/nancy/action/puzzle/hamradiopuzzle.h"
#include "engines/nancy/action/puzzle/hangmanpuzzle.h"
#include "engines/nancy/action/puzzle/keypadphone.h"
#include "engines/nancy/action/puzzle/lettergridpuzzle.h"
#include "engines/nancy/action/puzzle/leverpuzzle.h"
#include "engines/nancy/action/puzzle/magnetmazepuzzle.h"
#include "engines/nancy/action/puzzle/mazechasepuzzle.h"
#include "engines/nancy/action/puzzle/memorypuzzle.h"
#include "engines/nancy/action/puzzle/meterpuzzle.h"
#include "engines/nancy/action/puzzle/minesweeperpuzzle.h"
#include "engines/nancy/action/puzzle/mindpuzzle.h"
#include "engines/nancy/action/puzzle/minigolfpuzzle.h"
#include "engines/nancy/action/puzzle/mirrorlightpuzzle.h"
#include "engines/nancy/action/puzzle/mouselightpuzzle.h"
#include "engines/nancy/action/puzzle/multibuildpuzzle.h"
#include "engines/nancy/action/puzzle/necklacepuzzle.h"
#include "engines/nancy/action/puzzle/onebuildpuzzle.h"
#include "engines/nancy/action/puzzle/orderingpuzzle.h"
#include "engines/nancy/action/puzzle/overridelockpuzzle.h"
#include "engines/nancy/action/puzzle/pachinkopuzzle.h"
#include "engines/nancy/action/puzzle/paintpuzzle.h"
#include "engines/nancy/action/puzzle/passwordpuzzle.h"
#include "engines/nancy/action/puzzle/peepholepuzzle.h"
#include "engines/nancy/action/puzzle/pegspuzzle.h"
#include "engines/nancy/action/puzzle/quizpuzzle.h"
#include "engines/nancy/action/puzzle/raycastpuzzle.h"
#include "engines/nancy/action/puzzle/riddlepuzzle.h"
#include "engines/nancy/action/puzzle/rippedletterpuzzle.h"
#include "engines/nancy/action/puzzle/rotatinglockpuzzle.h"
#include "engines/nancy/action/puzzle/rowmatchpuzzle.h"
#include "engines/nancy/action/puzzle/safedialpuzzle.h"
#include "engines/nancy/action/puzzle/scalepuzzle.h"
#include "engines/nancy/action/puzzle/scopapuzzle.h"
#include "engines/nancy/action/puzzle/setplayerclock.h"
#include "engines/nancy/action/puzzle/sentrypuzzle.h"
#include "engines/nancy/action/puzzle/stakeoutpuzzle.h"
#include "engines/nancy/action/puzzle/sequencelockpuzzle.h"
#include "engines/nancy/action/puzzle/sewingmachinepuzzle.h"
#include "engines/nancy/action/puzzle/shovelmeasurepuzzle.h"
#include "engines/nancy/action/puzzle/skeeballpuzzle.h"
#include "engines/nancy/action/puzzle/sliderpuzzle.h"
#include "engines/nancy/action/puzzle/snowmobilepuzzle.h"
#include "engines/nancy/action/puzzle/sortpuzzle.h"
#include "engines/nancy/action/puzzle/soundequalizerpuzzle.h"
#include "engines/nancy/action/puzzle/soundmatchpuzzle.h"
#include "engines/nancy/action/puzzle/spigotpuzzle.h"
#include "engines/nancy/action/puzzle/stepobjectspuzzle.h"
#include "engines/nancy/action/puzzle/tangrampuzzle.h"
#include "engines/nancy/action/puzzle/telephone.h"
#include "engines/nancy/action/puzzle/tilecyclepuzzle.h"
#include "engines/nancy/action/puzzle/towerpuzzle.h"
#include "engines/nancy/action/puzzle/turningpuzzle.h"
#include "engines/nancy/action/puzzle/twodialpuzzle.h"
#include "engines/nancy/action/puzzle/typingquizpuzzle.h"
#include "engines/nancy/action/puzzle/weightsortpuzzle.h"
#include "engines/nancy/action/puzzle/whackitpuzzle.h"
#include "engines/nancy/action/puzzle/wolfmazepuzzle.h"
#include "engines/nancy/action/puzzle/whalesurvivorpuzzle.h"
#include "engines/nancy/action/puzzle/wordfindpuzzle.h"

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
			return new SceneChange();	// Moved from 12
	case 11:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kHotspot);
		else
			return new Hot1FrSceneChange(CursorManager::kNormal, true, true);
	case 12:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new SceneChange();
		else
			return new HotMultiframeSceneChange(CursorManager::kNormal, true);
	case 13:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeMultiSceneChange();
		else
			return new Hot1FrSceneChange(CursorManager::kHotspot);
	case 14:
		return new Hot1FrSceneChange(CursorManager::kExit);
	case 15:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new Hot1FrSceneChange(CursorManager::kMoveForward);
		else
			return new SceneChange(SceneChange::kWithoutFrame);
	case 16:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new Hot1FrSceneChange(CursorManager::kMoveBackward);
		else
			return new SceneChange(SceneChange::kWithFrame);
	case 17:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new Hot1FrSceneChange(CursorManager::kMoveUp);
		else
			return new SceneChangeWithExtras(SceneChangeWithExtras::kSingleFlag);
	case 18:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new Hot1FrSceneChange(CursorManager::kMoveDown);
		else
			return new SceneChangeWithExtras(SceneChangeWithExtras::kFlagList);
	case 19:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kMoveForward);
		else if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new Hot1FrSceneChange(CursorManager::kMoveLeft);		// Moved from 22
		else
			return new Hot1FrSceneChange(CursorManager::kNormal, true);
	case 20:
		if (g_nancy->getGameType() == kGameTypeVampire)
			return new PaletteThisScene();
		else if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kMoveUp);
		else if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new Hot1FrSceneChange(CursorManager::kMoveRight);	// Moved from 23
		else
			return new SceneChangeWithExtras(SceneChangeWithExtras::kHotspots);
	case 21:
		if (g_nancy->getGameType() == kGameTypeVampire)
			return new PaletteNextScene();
		else if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeSceneChange(CursorManager::kMoveDown);
		else if (g_nancy->getGameType() <= kGameTypeNancy16)
			return new HotSingleFrameSceneChange();
		else
			return new SceneChangeWithExtras(SceneChangeWithExtras::kStreamName);
	case 22:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new Hot1FrSceneChange(CursorManager::kMoveLeft);
		else
			return new HotMultiframeSceneChange(CursorManager::kHotspot);		// Moved from 11
	case 23:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new Hot1FrSceneChange(CursorManager::kMoveRight);
		else
			return new HotMultiframeSceneChange(CursorManager::kMoveForward);	// Moved from 19
	case 24:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new HotMultiframeMultiSceneCursorTypeSceneChange();
		else
			return new HotMultiframeSceneChange(CursorManager::kMoveUp);		// Moved from 20
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
		} else if (g_nancy->getGameType() <= kGameTypeNancy15) {
			return new HotMultiframeSceneChange(CursorManager::kMoveDown);		// Moved from 21
		} else if (g_nancy->getGameType() <= kGameTypeNancy16) {
			return new SceneChangeWithExtras(SceneChangeWithExtras::kFlagList | SceneChangeWithExtras::kHotspots);
		} else {
			// Nancy17 added the name of the stream to change the scene of
			return new SceneChangeWithExtras(SceneChangeWithExtras::kStreamName | SceneChangeWithExtras::kFlagList | SceneChangeWithExtras::kHotspots);
		}
	}
	case 26:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new InteractiveVideo();
		else if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new HotMultiframeMultiSceneChange();	// Moved from 13
		else if (g_nancy->getGameType() == kGameTypeNancy16)
			return nullptr;	// InvCursorAR, unused
		else
			return new BeginStream();
	case 27:	// Nancy10
		if (g_nancy->getGameType() <= kGameTypeNancy12)
			return new HotMultiframeMultiSceneCursorTypeSceneChange();	// Moved from 24
		else if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new HotMultiframeInvTypeSceneChange();
		else if (g_nancy->getGameType() <= kGameTypeNancy16)
			return new HideCursor();
		else
			return new EndStream();
	case 28:	// Nancy10
		if (g_nancy->getGameType() <= kGameTypeNancy16)
			return new InteractiveVideo();	// Moved from 26
		else
			return new HeldItemFlags();	// Moved from 26
	case 29:	// Nancy10
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new ControlUIItems();
		else if (g_nancy->getGameType() <= kGameTypeNancy16)
			return new UIControl();
		else
			return new HideCursor();	// Moved from 27
	case 30:	// Nancy11
		return new StopPlayerScrolling();
	case 31:	// Nancy11
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new StartPlayerScrolling();
		else
			return new UTurn();
	case 32:	// Nancy10
		return new UIPopupPrepScene();
	case 34:	// Nancy17
		return new UIControl();	// Moved from 29
	case 35:	// Nancy12
		return new ConversationInfoCheck();
	case 36:	// Nancy12
		return new ConversationGoodbye();
	case 37:	// Nancy17
		return new CollectionControl();
	case 38:	// Nancy17
		return new CollectionEdit();
	case 39:	// Nancy17
		return new CollectionCheck();
	case 40:
		if (g_nancy->getGameType() <= kGameTypeNancy1)
			return new LightningOn(); // Only used in TVD
		else if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new SpecialEffect();
		else if (g_nancy->getGameType() <= kGameTypeNancy16)
			return new Fader();
		else
			return new FadeOverlay();
	case 41:	// Nancy13
		return new PlaySecondaryMovie(PlaySecondaryMovie::kSecondaryMovieTerse);
	case 42:	// Nancy14
	case 43:	// Nancy14
	case 45:	// Nancy11
		return new PlaySecondaryMovie(PlaySecondaryMovie::kRandomMovie);
	case 44:	// Nancy14
		return new PlaySecondaryMovie(PlaySecondaryMovie::kMovieWithVolume);
	case 46:	// Nancy11
		return new PlayRandomMovieControl();
	case 47:	// Nancy14
		return new PlaySecondaryMovie(PlaySecondaryMovie::kInteractiveMovie);
	case 48:	// Nancy16
		return new PlaySecondaryMovie(PlaySecondaryMovie::kMovieWithRandomPosition);
	case 50:
		return new ConversationVideo();
	case 51:
		return new PlaySecondaryVideo();
	case 52:
		if (g_nancy->getGameType() <= kGameTypeNancy14)
			return new PlaySecondaryVideo();
		else
			return new OverlayMultiframeTerse();
	case 53:
		if (g_nancy->getGameType() <= kGameTypeNancy13)
			return new PlaySecondaryMovie(PlaySecondaryMovie::kSecondaryMovie);
		else
			return new RolloverOverlay();
	case 54:
		if (g_nancy->getGameType() <= kGameTypeNancy1)
			return new Overlay(Overlay::kStaticAnimation);
		else if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new Overlay(Overlay::kInterruptibleAnimation);
		else
			return new OverlayRandomColor();
	case 55:
		if (g_nancy->getGameType() <= kGameTypeNancy1)
			return new Overlay(Overlay::kInterruptibleAnimation);
		else
			return new OverlayStaticTerse();
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
		if (g_nancy->getGameType() <= kGameTypeNancy5)
			return new MapCall();	// Only used in TVD and nancy1
		else
			return new ConversationSoundT();
	case 61:
		if (g_nancy->getGameType() <= kGameTypeNancy5)
			return new MapCallHot1Fr();	// Only used in TVD and nancy1
		else
			return new Autotext();
	case 62:
		if (g_nancy->getGameType() <= kGameTypeNancy7)
			return new MapCallHotMultiframe(); // Only used in TVD and nancy1
		else
			return new ConversationCelTerse();
	case 63:
		return new ConversationSoundTerse();
	case 65:
		return new TableIndexOverlay();
	case 66:
		return new TableIndexPlaySound();
	case 67:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new TableIndexSetValueHS();
		else
			return new Autotext();		// Moved from 61
	case 68:
		if (g_nancy->getGameType() <= kGameTypeNancy11)
			return new TextScroll(TextScroll::kTextScroll);
		else
			return new TextLineOverlay();
	case 69:	// Nancy11
		return new TimerControl();
	case 70:
		return new TextScroll(TextScroll::kAutotextEntryList);
	case 71:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new ModifyListEntry(ModifyListEntry::kAdd);
		else
			return new AddJournalEntry();
	case 72:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new ModifyListEntry(ModifyListEntry::kDelete);
		else
			return new AddTasklistEntry();
	case 73:
		return new ModifyListEntry(ModifyListEntry::kMark);
	case 74:	// Nancy10 only
		return new FrameTextBox(FrameTextBox::kFullBox);
	case 75:
		if (g_nancy->getGameType() <= kGameTypeNancy9)
			return new TextBoxWrite(TextBoxWrite::kTextBoxWrite);
		else
			return new FrameTextBox(FrameTextBox::kNormalBox);
	case 76:
		return new TextboxClear();
	case 77:
		return new SetValue();
	case 78:
		return new SetValueCombo();
	case 79:
		return new ValueTest();
	case 81:	// Nancy11
		return new TextBoxWrite(TextBoxWrite::kAutotextWrite);
	case 90:	// Nancy16
		return new EventFlags(EventFlags::kEventFlags);
	case 91:	// Nancy16
		return new EventFlagsMultiHS(EventFlagsMultiHS::kCursorHS);
	case 92:	// Nancy17
		return new SetRandomFlag();
	case 93:	// Nancy17
		return new OnSceneChange();
	case 94:	// Nancy12
		return new EventFlagsMultiHS(EventFlagsMultiHS::kMultiHS);	// Moved from 106
	case 95:	// Nancy12
		return new EventFlags(EventFlags::kEventFlags);	// Moved from 107
	case 96:	// Nancy11
		return new RandomizeEventFlags();
	case 97:
		return new EventFlags(EventFlags::kEventFlagsTerse);
	case 98:
		return new EventFlagsMultiHS(EventFlagsMultiHS::kHSTerse);
	case 99:
		return new EventFlagsMultiHS(EventFlagsMultiHS::kCursorHS);
	case 100:
		return new BumpPlayerClock();
	case 101:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new SaveContinueGame();
		else
			return new BumpPlayerClock();		// Moved from 100
	case 102:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new TurnOffMainRendering();
		else
			return new SaveContinueGame();		// Moved from 101
	case 103:
		return new TurnOnMainRendering();
	case 104:
		return new ResetAndStartTimer();
	case 105:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new StopTimer();
		else
			return new MakeScreenFile();		// Moved from 141
	case 106:
		return new EventFlagsMultiHS(EventFlagsMultiHS::kMultiHS);
	case 107:
		return new EventFlags(EventFlags::kEventFlags);
	case 108:
		if (g_nancy->getGameType() <= kGameTypeNancy6)
			return new OrderingPuzzle(OrderingPuzzle::kOrdering);
		else if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new GotoMenu();
		else
			return new ClearSavedData();
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
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new RotatingLockPuzzle();
		else
			return new SaveNamedGame();
	case 115:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new LeverPuzzle();
		else
			return new LoadNamedGame();
	case 116:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new Telephone(Telephone::kTelephone);
		else
			return new KeypadPhone();
	case 117:
		return new SliderPuzzle();
	case 118:
		return new PasswordPuzzle();
	case 119:	// Nancy7
		return new OrderingPuzzle(OrderingPuzzle::kOrdering);
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
	case 127:	// Nancy17
		return new AddPhoneEntry();
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
	case 133:	// Nancy14 - only used for that one
		return new CameraAction();
	case 134:	// Nancy15
		// Switches the active player character (Nancy / Frank / Joe), the
		// dual-protagonist mechanic new to The Creature of Kapu Cave
		return new PlayChar();
	case 140:
		if (g_nancy->getGameType() <= kGameTypeNancy11)
			return new SetVolume();			// Moved to 149 in Nancy9, empty slot in Nancy9-11
		else
			return new SetPlayerClock();	// Moved from 170
	case 141:	// Nancy12
		return new MakeScreenFile();	// Moved from 148
	case 143:	// Nancy14
		return new ConcatSound();
	case 144:	// Nancy14
		return new MultiSound();
	case 145:	// Nancy13
		return new PlaySound(); // Moved from 150
	case 146:	// Nancy13
		return new FadeSoundToSilence(); // Moved from 147
	case 147:	// Nancy11
		if (g_nancy->getGameType() <= kGameTypeNancy12)
			return new FadeSoundToSilence();
		else
			return new SetVolume();				// Moved from 148
	case 148:	// Nancy11
		if (g_nancy->getGameType() <= kGameTypeNancy11)
			return new MakeScreenFile();
		else if (g_nancy->getGameType() <= kGameTypeNancy12)
			return new SetVolume();			// Moved from 149
		else
			return new StopSound();			// Moved from 154
	case 149:	// Nancy9
		if (g_nancy->getGameType() <= kGameTypeNancy11)
			return new SetVolume();					// Moved from 140
		else if (g_nancy->getGameType() <= kGameTypeNancy12)
			return new PlaySoundEventFlagTerse();	// Moved from 161
		else
			return new StopSound();					// Moved from 155
	case 150:
		if (g_nancy->getGameType() <= kGameTypeNancy13)
			return new PlaySound();
		else
			return new SetMovieVolume();
	case 151:
		if (g_nancy->getGameType() <= kGameTypeNancy6)
			return new PlaySound(); // PlayStreamSound
		else
			return new PlayRandomSoundTerse();
	case 152:
		return new PlaySoundFrameAnchor();
	case 153:
		return new PlaySoundMultiHS();
	case 154:
	case 155: // StopAndUnloadSound, but we always unload
		return new StopSound();
	case 156:	// Nancy11
		if (g_nancy->getGameType() <= kGameTypeNancy16)
			return new Update3DSound();
		else
			return new RotatingLockPuzzle();	// Moved from 114
	case 157:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new PlaySoundCC();
		else
			return new KeypadPhone();	// Moved from 116
	case 158:
		return new PlayRandomSound();
	case 159:	// Nancy7
		if (g_nancy->getGameType() <= kGameTypeNancy13)
			return new PlaySoundTerse();
		else
			return new LetterGridPuzzle();
	case 160:
		if (g_nancy->getGameType() <= kGameTypeNancy11)
			return new HintSystem();
		else if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new DrivingPuzzle(DrivingPuzzle::kDriving);
		else
			return new SnowmobilePuzzle(SnowmobilePuzzle::kDriving);
	case 161:	// Nancy11
		if (g_nancy->getGameType() <= kGameTypeNancy11)
			return new PlaySoundEventFlagTerse();
		else
			return new MinigolfPuzzle();
	// -- Nancy 12 new puzzles/action records --
	case 162:
		return new SewingMachinePuzzle();
	case 163:
		return new MirrorLightPuzzle();
	case 164:
		return new BoardGamePuzzle();
	case 165:
		return new MindPuzzle();
	case 166:
		if (g_nancy->getGameType() <= kGameTypeNancy13)
			return new OneBuildPuzzle();	// Moved from 234
		else
			return new BuildPuzzle();
	case 167:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new DrivingPuzzle(DrivingPuzzle::kChase);
		else
			return new SnowmobilePuzzle(SnowmobilePuzzle::kChase);
	case 168:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new Set3DSoundListenerPosition();
		else
			return new SentryStartPosition();
	// -- Nancy 13 new/relocated puzzles (types 169-176) --
	case 169:
		return new StepObjectsPuzzle();
	case 170:
		if (g_nancy->getGameType() <= kGameTypeNancy12)
			return new SetPlayerClock();	// Moved to 140 in Nancy12, unused slot in Nancy12
		else
			return new WordFindPuzzle();
	case 171:
		if (g_nancy->getGameType() <= kGameTypeNancy16)
			return new TurningPuzzle();	// Moved from 209
		else
			return new TileCyclePuzzle();
	case 172:
		return new BlocksPuzzle();
	case 173:
		return new PegsPuzzle();
	case 174:
		return new ScalePuzzle();
	case 175:
		return new PachinkoPuzzle();
	case 176:
		return new DropSortPuzzle();
	// -- Nancy14 new puzzles (types 177-182) --
	case 177:
		return new HangmanPuzzle();
	case 178:
		return new AdjustPuzzle();
	case 179:
		return new MeterPuzzle();
	case 180:
		return new BlockingPuzzle();
	case 181:
		return new PaintPuzzle();
	case 182:
		return new DecoderPuzzle();
	// -- Nancy15 new puzzles (types 183-185) --
	case 183:
		return new MagicBoxPuzzle();
	case 184:
		return new EscapeGridPuzzle();
	case 185:
		return new WeightSortPuzzle();
	// -- Nancy16 new puzzles (types 186-199) --
	case 186:
		return new FoxGeesePuzzle();
	case 187:	// Nancy17
		return new DialSequencePuzzle();
	case 188: // Nancy18
		return new ChessChatPuzzle();
	case 189:
		return new QuizPuzzle();		// Moved from 231
	case 192:
		return new RowMatchPuzzle();
	case 193:
		return new FishingPuzzle();
	case 194:
		return new CookingPuzzle();
	case 195:
		return new MinesweeperPuzzle();
	case 196:
		return new AnalogClock();
	case 197:
		return new AutotextScroll();	// Used for the end credits
	case 190:	// Nancy18
		return new SequenceLockPuzzle();
	case 191:	// BeadPuzzle, moved from 243, unused in Nancy16
		return nullptr;
	case 198:
		return new WolfCommands();
	case 199:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new WolfMazePuzzle();	// Driven by the commands given through 198
		else
			return new AutotextScroll(true);
	case 200:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new SoundEqualizerPuzzle();
		else
			return new Rain();
	case 201:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new TowerPuzzle();
		else
			return new BookSortPuzzle();
	case 202:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new BombPuzzle();
		else
			return new FlashMemoryPuzzle();
	case 203:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new RippedLetterPuzzle();
		else
			return new WhackItPuzzle();
	case 204:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new OverrideLockPuzzle();
		else
			return new EscapePuzzle();
	case 205:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new RiddlePuzzle();
		else
			return new ShovelMeasurePuzzle();
	case 206:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new RaycastPuzzle();
		else
			return new SkeeBallPuzzle();
	case 207:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new TangramPuzzle();
		else
			return new DragSurfacePuzzle();
	case 208:
		if (g_nancy->getGameType() <= kGameTypeNancy15)
			return new OrderingPuzzle(OrderingPuzzle::PuzzleType::kPiano);
		else
			return new GoldbergPuzzle();
	case 209:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new TurningPuzzle();
		else
			return new TypewriterText();
	case 210:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new SafeDialPuzzle();
		else
			return new SentryPuzzle();
	case 211:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new CollisionPuzzle(CollisionPuzzle::PuzzleType::kCollision);
		else
			return new ScopaPuzzle();
	case 212:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new OrderingPuzzle(OrderingPuzzle::PuzzleType::kOrderItems);
		else
			return new StakeOutPuzzle();
	case 213:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new CollisionPuzzle(CollisionPuzzle::PuzzleType::kTileMove);
		else
			return new GraphChase();
	case 214:
		if (g_nancy->getGameType() <= kGameTypeNancy17)
			return new OrderingPuzzle(OrderingPuzzle::PuzzleType::kKeypad);
		else
			return new DancePuzzle();
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
		return new Telephone(Telephone::kNewPhone);
	case 231:
		return new QuizPuzzle();
	case 232:
		return new AngleTossPuzzle();
	// -- Nancy 9 and up --
	case 233:
		return new SoundMatchPuzzle();
	case 234:
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
		if (g_nancy->getGameType() <= kGameTypeNancy14)
			return new BeadPuzzle();
		else
			return new NecklacePuzzle();
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
