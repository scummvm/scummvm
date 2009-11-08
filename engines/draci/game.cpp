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
 * $URL$
 * $Id$
 *
 */

#include "common/stream.h"

#include "draci/draci.h"
#include "draci/game.h"
#include "draci/barchive.h"
#include "draci/script.h"
#include "draci/animation.h"

#include <cmath>

namespace Draci {

static const Common::String dialoguePath("ROZH");

static double real_to_double(byte real[6]);

enum {
	kWalkingMapOverlayColour = 2,
	kWalkingShortestPathOverlayColour = 120,
	kWalkingObliquePathOverlayColour = 73
};

Game::Game(DraciEngine *vm) : _vm(vm), _walkingState(vm) {
	uint i;

	BArchive *initArchive = _vm->_initArchive;
	const BAFile *file;

	// Read in persons
	file = initArchive->getFile(5);
	Common::MemoryReadStream personData(file->_data, file->_length);

	const int personSize = sizeof(uint16) * 2 + sizeof(byte);
	uint numPersons = file->_length / personSize;
	_persons = new Person[numPersons];

	for (i = 0; i < numPersons; ++i) {
		_persons[i]._x = personData.readUint16LE();
		_persons[i]._y = personData.readUint16LE();
		_persons[i]._fontColour = personData.readByte();
	}

	// Read in dialogue offsets
	file = initArchive->getFile(4);
	Common::MemoryReadStream dialogueData(file->_data, file->_length);

	uint numDialogues = file->_length / sizeof(uint16);
	_dialogueOffsets = new uint[numDialogues];

	uint curOffset;
	for (i = 0, curOffset = 0; i < numDialogues; ++i) {
		_dialogueOffsets[i] = curOffset;
		curOffset += dialogueData.readUint16LE();
	}

	_dialogueVars = new int[curOffset];
	memset(_dialogueVars, 0, sizeof (int) * curOffset);

	// Read in game info
	file = initArchive->getFile(3);
	Common::MemoryReadStream gameData(file->_data, file->_length);

	_info._startRoom = gameData.readByte() - 1;
	_info._mapRoom = gameData.readByte() - 1;
	_info._numObjects = gameData.readUint16LE();
	_info._numItems = gameData.readUint16LE();
	_info._numVariables = gameData.readByte();
	_info._numPersons = gameData.readByte();
	_info._numDialogues = gameData.readByte();
	_info._maxItemWidth = gameData.readUint16LE();
	_info._maxItemHeight = gameData.readUint16LE();
	_info._musicLength = gameData.readUint16LE();
	_info._crc[0] = gameData.readUint16LE();
	_info._crc[1] = gameData.readUint16LE();
	_info._crc[2] = gameData.readUint16LE();
	_info._crc[3] = gameData.readUint16LE();

	_info._numDialogueBlocks = curOffset;

	// Read in variables
	file = initArchive->getFile(2);
	uint numVariables = file->_length / sizeof (int16);

	_variables = new int[numVariables];
	Common::MemoryReadStream variableData(file->_data, file->_length);

	for (i = 0; i < numVariables; ++i) {
		_variables[i] = variableData.readUint16LE();
	}

	// Read in item icon status
	file = initArchive->getFile(1);
	uint numItems = file->_length;
	_itemStatus = new byte[numItems];
	memcpy(_itemStatus, file->_data, numItems);
	_items = new GameItem[numItems];

	// Read in object status
	file = initArchive->getFile(0);
	uint numObjects = file->_length;

	_objects = new GameObject[numObjects];
	Common::MemoryReadStream objStatus(file->_data, file->_length);

	for (i = 0; i < numObjects; ++i) {
		byte tmp = objStatus.readByte();

		// Set object visibility
		_objects[i]._visible = tmp & (1 << 7);

		// Set object location
		_objects[i]._location = (~(1 << 7) & tmp) - 1;
	}

	assert(numDialogues == _info._numDialogues);
	assert(numPersons == _info._numPersons);
	assert(numVariables == _info._numVariables);
	assert(numObjects == _info._numObjects);
	assert(numItems == _info._numItems);

	// Deallocate all cached files, because we have copied them into our own data structures.
	initArchive->clearCache();
}

void Game::start() {
	while (!shouldQuit()) {
		if (enterNewRoom()) {
			// Call the outer loop doing all the hard job.
			loop(kOuterLoop, false);
		}
	}
}

void Game::init() {
	setQuit(false);
	setExitLoop(false);
	setIsReloaded(false);
	_scheduledPalette = 0;
	_fadePhases = _fadePhase = 0;
	setEnableQuickHero(true);
	setWantQuickHero(false);
	setEnableSpeedText(true);
	setLoopStatus(kStatusGate);
	setLoopSubstatus(kOuterLoop);

	_animUnderCursor = kOverlayImage;

	_currentItem = kNoItem;
	_itemUnderCursor = kNoItem;

	_vm->_mouse->setCursorType(kHighlightedCursor);	// anything different from kNormalCursor

	_objUnderCursor = kOverlayImage;
        
	// Set the inventory to empty initially
	memset(_inventory, kNoItem, kInventorySlots * sizeof(int));

	// Initialize animation for object / room titles
	Animation *titleAnim = _vm->_anims->addText(kTitleText, true);
	Text *title = new Text("", _vm->_smallFont, kTitleColour, 0, 0, 0);
	titleAnim->addFrame(title, NULL);

	// Initialize animation for speech text
	Animation *speechAnim = _vm->_anims->addText(kSpeechText, true);
	Text *speech = new Text("", _vm->_bigFont, kFontColour1, 0, 0, 0);
	speechAnim->addFrame(speech, NULL);

	// Initialize inventory animation.  _iconsArchive is never flushed.
	const BAFile *f = _vm->_iconsArchive->getFile(13);
	Animation *inventoryAnim = _vm->_anims->addAnimation(kInventorySprite, 255, false);
	Sprite *inventorySprite = new Sprite(f->_data, f->_length, 0, 0, true);
	inventoryAnim->addFrame(inventorySprite, NULL);
	inventoryAnim->setRelative((kScreenWidth - inventorySprite->getWidth()) / 2,
	                           (kScreenHeight - inventorySprite->getHeight()) / 2);

	for (uint i = 0; i < kDialogueLines; ++i) {
		_dialogueAnims[i] = _vm->_anims->addText(kDialogueLinesID - i, true);
		Text *dialogueLine = new Text("", _vm->_smallFont, kLineInactiveColour, 0, 0, 0);
		_dialogueAnims[i]->addFrame(dialogueLine, NULL);

		_dialogueAnims[i]->setZ(254);
		_dialogueAnims[i]->setRelative(1,
		                      kScreenHeight - (i + 1) * _vm->_smallFont->getFontHeight());

		Text *text = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());
		text->setText("");
	}

	for (uint i = 0; i < _info._numItems; ++i) {
		loadItem(i);
	}

	loadObject(kDragonObject);

	const GameObject *dragon = getObject(kDragonObject);
	debugC(4, kDraciLogicDebugLevel, "Running init program for the dragon object...");
	_vm->_script->run(dragon->_program, dragon->_init);

	// Make sure we enter the right room in start().
	setRoomNum(kNoEscRoom);
	rememberRoomNumAsPrevious();
	scheduleEnteringRoomUsingGate(_info._startRoom, 0);
	_pushedNewRoom = _pushedNewGate = -1;
}

void Game::handleOrdinaryLoop(int x, int y) {
	// During the normal game-play, in particular not when
	// running the init-scripts, enable interactivity.
	if (_loopSubstatus != kOuterLoop) {
		return;
	}

	if (_vm->_mouse->lButtonPressed()) {
		_vm->_mouse->lButtonSet(false);

		if (_currentItem != kNoItem) {
			putItem(_currentItem, 0);
			_currentItem = kNoItem;
			updateOrdinaryCursor();
		} else {
			if (_objUnderCursor != kObjectNotFound) {
				const GameObject *obj = &_objects[_objUnderCursor];

				_walkingState.setCallback(&obj->_program, obj->_look);

				if (obj->_imLook || !_currentRoom._heroOn) {
					_walkingState.callback();
				} else {
					if (obj->_lookDir == kDirectionLast) {
						walkHero(x, y, obj->_lookDir);
					} else {
						walkHero(obj->_lookX, obj->_lookY, obj->_lookDir);
					}
				}
			} else {
				_walkingState.setCallback(NULL, 0);
				walkHero(x, y, kDirectionLast);
			}
		}
	}

	if (_vm->_mouse->rButtonPressed()) {
		_vm->_mouse->rButtonSet(false);

		if (_objUnderCursor != kObjectNotFound) {
			const GameObject *obj = &_objects[_objUnderCursor];

			if (_vm->_script->testExpression(obj->_program, obj->_canUse)) {
				_walkingState.setCallback(&obj->_program, obj->_use);

				if (obj->_imUse || !_currentRoom._heroOn) {
					_walkingState.callback();
				} else {
					if (obj->_useDir == kDirectionLast) {
						walkHero(x, y, obj->_useDir);
					} else {
						walkHero(obj->_useX, obj->_useY, obj->_useDir);
					}
				}
			} else {
				_walkingState.setCallback(NULL, 0);
				walkHero(x, y, kDirectionLast);
			}
		} else {
			if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
				_walkingState.setCallback(&_currentRoom._program, _currentRoom._use);
				_walkingState.callback();
			} else {
				_walkingState.setCallback(NULL, 0);
				walkHero(x, y, kDirectionLast);
			}
		}
	}
}

void Game::handleInventoryLoop() {
	if (_loopSubstatus != kOuterLoop) {
		return;
	}
	if (_inventoryExit) {
		inventoryDone();
	}

	// If we are in inventory mode, all the animations except game items'
	// images will necessarily be paused so we can safely assume that any
	// animation under the cursor (a value returned by
	// AnimationManager::getTopAnimationID()) will be an item animation or
	// an overlay, for which we check. Item animations have their IDs
	// calculated by offseting their itemID from the ID of the last "special"
	// animation ID. In this way, we obtain its itemID.
	if (_animUnderCursor != kOverlayImage && _animUnderCursor != kInventorySprite) {
		_itemUnderCursor = kInventoryItemsID - _animUnderCursor;
	} else {
		_itemUnderCursor = kNoItem;
	}

	// If the user pressed the left mouse button
	if (_vm->_mouse->lButtonPressed()) {
		_vm->_mouse->lButtonSet(false);

		// If there is an inventory item under the cursor and we aren't
		// holding any item, run its look GPL program
		if (_itemUnderCursor != kNoItem && _currentItem == kNoItem) {
			const GameItem *item = &_items[_itemUnderCursor];

			_vm->_script->run(item->_program, item->_look);
		// Otherwise, if we are holding an item, try to place it inside the
		// inventory
		} else if (_currentItem != kNoItem) {
			// FIXME: This should place the item in the nearest inventory slot,
			// not the first one available
			putItem(_currentItem, 0);

			// Remove it from our hands
			_currentItem = kNoItem;
		}
	} else if (_vm->_mouse->rButtonPressed()) {
		_vm->_mouse->rButtonSet(false);

		// If we right-clicked outside the inventory, close it
		if (_animUnderCursor != kInventorySprite && _itemUnderCursor == kNoItem) {
			inventoryDone();

		// If there is an inventory item under our cursor
		} else if (_itemUnderCursor != kNoItem) {
			// Again, we have two possibilities:

			// The first is that there is no item in our hands.
			// In that case, just take the inventory item from the inventory.
			if (_currentItem == kNoItem) {
				_currentItem = _itemUnderCursor;
				removeItem(_itemUnderCursor);

			// The second is that there *is* an item in our hands.
			// In that case, run the canUse script for the inventory item
			// which will check if the two items are combinable and, finally,
			// run the use script for the item.
			} else {
				const GameItem *item = &_items[_itemUnderCursor];

				if (_vm->_script->testExpression(item->_program, item->_canUse)) {
					_vm->_script->run(item->_program, item->_use);
				}
			}
			updateInventoryCursor();
		}
	}
}

void Game::handleDialogueLoop() {
	if (_loopSubstatus != kInnerDuringDialogue) {
		return;
	}

	Text *text;
	for (int i = 0; i < kDialogueLines; ++i) {
		text = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());

		if (_animUnderCursor == _dialogueAnims[i]->getID()) {
			text->setColour(kLineActiveColour);
		} else {
			text->setColour(kLineInactiveColour);
		}
	}

	if (_vm->_mouse->lButtonPressed() || _vm->_mouse->rButtonPressed()) {
		setExitLoop(true);
		_vm->_mouse->lButtonSet(false);
		_vm->_mouse->rButtonSet(false);
	}
}

void Game::advanceAnimationsAndTestLoopExit() {
	// Fade the palette if requested
	if (_fadePhase > 0 && (_vm->_system->getMillis() - _fadeTick) >= kFadingTimeUnit) {
		_fadeTick = _vm->_system->getMillis();
		--_fadePhase;
		const byte *startPal = _currentRoom._palette >= 0 ? _vm->_paletteArchive->getFile(_currentRoom._palette)->_data : NULL;
		const byte *endPal = getScheduledPalette() >= 0 ? _vm->_paletteArchive->getFile(getScheduledPalette())->_data : NULL;
		_vm->_screen->interpolatePalettes(startPal, endPal, 0, kNumColours, _fadePhases - _fadePhase, _fadePhases);
		if (_fadePhase == 0) {
			if (_loopSubstatus == kInnerWhileFade) {
				setExitLoop(true);
			}
			// Rewrite the palette index of the current room.  This
			// is necessary when two fadings are called after each
			// other, such as in the intro.
			_currentRoom._palette = getScheduledPalette();
		}
	}

	// Handle character talking (if there is any)
	if (_loopSubstatus == kInnerWhileTalk) {
		// If the current speech text has expired or the user clicked a mouse button,
		// advance to the next line of text
		if ((getEnableSpeedText() && (_vm->_mouse->lButtonPressed() || _vm->_mouse->rButtonPressed())) ||
			(_vm->_system->getMillis() - _speechTick) >= _speechDuration) {

			setExitLoop(true);
		}
		_vm->_mouse->lButtonSet(false);
		_vm->_mouse->rButtonSet(false);
	}

	// A script has scheduled changing the room (either triggered
	// by the user clicking on something or run at the end of a
	// gate script in the intro).
	if ((_loopStatus == kStatusOrdinary || _loopStatus == kStatusGate) && _newRoom != getRoomNum()) {
		setExitLoop(true);
	}

	// This returns true if we got a signal to quit the game
	if (shouldQuit()) {
		setExitLoop(true);
	}

	// Walk the hero.  The WalkingState class handles everything including
	// proper timing.
	bool walkingFinished = false;
	if (_walkingState.isActive()) {
		walkingFinished = !_walkingState.continueWalking();
	}

	// Advance animations (this may also call setExitLoop(true) in the
	// callbacks) and redraw screen
	_vm->_anims->drawScene(_vm->_screen->getSurface());
	_vm->_screen->copyToScreen();
	_vm->_system->delayMillis(20);

	// If the hero has arrived at his destination, after even the last
	// phase was correctly animated, run the callback.
	if (walkingFinished) {
		bool exitLoop = false;
		if (_loopSubstatus == kInnerUntilExit) {
			// The callback may run another inner loop (for
			// example, a dialogue).  Reset the loop
			// substatus temporarily to the outer one.
			exitLoop = true;
			setLoopSubstatus(kOuterLoop);
		}
		debugC(2, kDraciWalkingDebugLevel, "Finished walking");
		_walkingState.callback();	// clears callback pointer first
		if (exitLoop) {
			debugC(3, kDraciWalkingDebugLevel, "Exiting from the inner loop");
			setExitLoop(true);
			setLoopSubstatus(kInnerUntilExit);
		}
	}
}

void Game::loop(LoopSubstatus substatus, bool shouldExit) {
	// Can run both as an outer and inner loop.  In both mode it updates
	// the screen according to the timer.  It the outer mode (kOuterLoop)
	// it also reacts to user events.  In the inner mode (all kInner*
	// enums), the loop runs until its stopping condition, possibly
	// stopping earlier if the user interrupts it, however no other user
	// intervention is allowed.
	assert(getLoopSubstatus() == kOuterLoop);
	setLoopSubstatus(substatus);
	setExitLoop(shouldExit);

	// Always enter the first pass of the loop, even if shouldExitLoop() is
	// true, exactly to ensure to make at least one pass.
	do {
		debugC(4, kDraciLogicDebugLevel, "loopstatus: %d, loopsubstatus: %d",
			_loopStatus, _loopSubstatus);

		_vm->handleEvents();
		if (isReloaded()) {
			// Cannot continue with the same animation objects,
			// because the real data structures of the game have
			// completely been changed.
			break;
		}

		if (_vm->_mouse->isCursorOn()) {
			// Find animation under cursor and the game object
			// corresponding to it
			int x = _vm->_mouse->getPosX();
			int y = _vm->_mouse->getPosY();
			_animUnderCursor = _vm->_anims->getTopAnimationID(x, y);
			_objUnderCursor = getObjectWithAnimation(_animUnderCursor);
			debugC(5, kDraciLogicDebugLevel, "Anim under cursor: %d", _animUnderCursor);

			switch (_loopStatus) {
			case kStatusOrdinary:
				updateOrdinaryCursor();
				updateTitle(x, y);
				handleOrdinaryLoop(x, y);
				break;
			case kStatusInventory:
				updateInventoryCursor();
				updateTitle(x, y);
				handleInventoryLoop();
				break;
			case kStatusDialogue:
				handleDialogueLoop();
				break;
			case kStatusGate: ;
				// cannot happen when isCursonOn; added for completeness
			}

			// TODO: Handle main menu
		}

		advanceAnimationsAndTestLoopExit();

	} while (!shouldExitLoop());

	setLoopSubstatus(kOuterLoop);
	setExitLoop(false);
}

void Game::updateOrdinaryCursor() {
	// Fetch mouse coordinates
	bool mouseChanged = false;

	// If there is no game object under the cursor, try using the room itself
	if (_objUnderCursor == kObjectNotFound) {
		if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
			if (_currentItem == kNoItem) {
				_vm->_mouse->setCursorType(kHighlightedCursor);
			} else {
				_vm->_mouse->loadItemCursor(_currentItem, true);
			}
			mouseChanged = true;
		}
	// If there *is* a game object under the cursor, update the cursor image
	} else {
		const GameObject *obj = &_objects[_objUnderCursor];

		// If there is no walking direction set on the object (i.e. the object
		// is not a gate / exit), test whether it can be used and, if so,
		// update the cursor image (highlight it).
		if (obj->_walkDir == 0) {
			if (_vm->_script->testExpression(obj->_program, obj->_canUse)) {
				if (_currentItem == kNoItem) {
					_vm->_mouse->setCursorType(kHighlightedCursor);
				} else {
					_vm->_mouse->loadItemCursor(_currentItem, true);
				}
				mouseChanged = true;
			}
		// If the walking direction *is* set, the game object is a gate, so update
		// the cursor image to the appropriate arrow.
		} else {
			_vm->_mouse->setCursorType((CursorType)obj->_walkDir);
			mouseChanged = true;
		}
	}
	// Load the appropriate cursor (item image if an item is held or ordinary cursor
	// if not)
	if (!mouseChanged) {
		if (_currentItem == kNoItem) {
			_vm->_mouse->setCursorType(kNormalCursor);
		} else {
			_vm->_mouse->loadItemCursor(_currentItem, false);
		}
	}
}

void Game::updateInventoryCursor() {
	// Fetch mouse coordinates
	bool mouseChanged = false;

	if (_itemUnderCursor != kNoItem) {
		const GameItem *item = &_items[_itemUnderCursor];

		if (_vm->_script->testExpression(item->_program, item->_canUse)) {
			if (_currentItem == kNoItem) {
				_vm->_mouse->setCursorType(kHighlightedCursor);
			} else {
				_vm->_mouse->loadItemCursor(_currentItem, true);
			}
			mouseChanged = true;
		}
	}
	if (!mouseChanged) {
		if (_currentItem == kNoItem) {
			_vm->_mouse->setCursorType(kNormalCursor);
		} else {
			_vm->_mouse->loadItemCursor(_currentItem, false);
		}
	}
}

void Game::updateTitle(int x, int y) {
	// Fetch current surface and height of the small font (used for titles)
	Surface *surface = _vm->_screen->getSurface();
	const int smallFontHeight = _vm->_smallFont->getFontHeight();

	// Fetch the dedicated objects' title animation / current frame
	Animation *titleAnim = _vm->_anims->getAnimation(kTitleText);
	Text *title = reinterpret_cast<Text *>(titleAnim->getCurrentFrame());

	// Mark dirty rectangle to delete the previous text
	titleAnim->markDirtyRect(surface);

	if (_loopStatus == kStatusInventory) {
		// If there is no item under the cursor, delete the title.
		// Otherwise, show the item's title.
		if (_itemUnderCursor == kNoItem) {
			title->setText("");
		} else {
			const GameItem *item = &_items[_itemUnderCursor];
			title->setText(item->_title);
		}
	} else {
		// If there is no object under the cursor, delete the title.
		// Otherwise, show the object's title.
		if (_objUnderCursor == kObjectNotFound) {
			title->setText("");
		} else {
			const GameObject *obj = &_objects[_objUnderCursor];
			title->setText(obj->_title);
		}
	}

	// Move the title to the correct place (just above the cursor)
	int newX = surface->centerOnX(x, title->getWidth());
	int newY = surface->putAboveY(y - smallFontHeight / 2, title->getHeight());
	titleAnim->setRelative(newX, newY);

	// If we are currently playing the title, mark it dirty so it gets updated.
	// Otherwise, start playing the title animation.
	if (titleAnim->isPlaying()) {
		titleAnim->markDirtyRect(surface);
	} else {
		_vm->_anims->play(titleAnim->getID());
	}
}

int Game::getObjectWithAnimation(int animID) const {
	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];

		for (uint j = 0; j < obj->_anim.size(); ++j) {
			if (obj->_anim[j] == animID) {
				return i;
			}
		}
	}

	return kObjectNotFound;
}

void Game::removeItem(int itemID) {
	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] == itemID) {
			_inventory[i] = kNoItem;
			_vm->_anims->stop(kInventoryItemsID - itemID);
			break;
		}
	}
}

void Game::putItem(int itemID, int position) {
	if (itemID == kNoItem)
		return;

	if (position >= 0 &&
		position < kInventoryLines * kInventoryColumns &&
		(_inventory[position] == kNoItem || _inventory[position] == itemID)) {
		_inventory[position] = itemID;
	} else {
		for (position = 0; position < kInventorySlots; ++position) {
			if (_inventory[position] == kNoItem) {
				_inventory[position] = itemID;
				break;
			}
		}
	}

	const int line = position / kInventoryColumns + 1;
	const int column = position % kInventoryColumns + 1;

	const int anim_id = kInventoryItemsID - itemID;
	Animation *anim = _vm->_anims->getAnimation(anim_id);
	if (!anim) {
		anim = _vm->_anims->addItem(anim_id, false);
		// _itemImagesArchive is never flushed.
		const BAFile *img = _vm->_itemImagesArchive->getFile(2 * itemID);
		Sprite *sp = new Sprite(img->_data, img->_length, 0, 0, true);
		anim->addFrame(sp, NULL);
	}
	Drawable *frame = anim->getCurrentFrame();

	const int x = kInventoryX +
	              (column * kInventoryItemWidth) -
	              (kInventoryItemWidth / 2) -
	              (frame->getWidth() / 2);

	const int y = kInventoryY +
	              (line * kInventoryItemHeight) -
	              (kInventoryItemHeight / 2) -
	              (frame->getHeight() / 2);

	debug(2, "itemID: %d position: %d line: %d column: %d x: %d y: %d", itemID, position, line, column, x, y);

	anim->setRelative(x, y);

	// If we are in inventory mode, we need to play the item animation, immediately
	// upon returning it to its slot but *not* in other modes because it should be
	// invisible then (along with the inventory)
	if (_loopStatus == kStatusInventory && _loopSubstatus == kOuterLoop) {
		_vm->_anims->play(anim_id);
	}
}

void Game::inventoryInit() {
	// Pause all "background" animations
	_vm->_anims->pauseAnimations();

	// Draw the inventory and the current items
	inventoryDraw();

	// Turn cursor on if it is off
	_vm->_mouse->cursorOn();

	// Set the appropriate loop status
	setLoopStatus(kStatusInventory);

	// TODO: This will be used for exiting the inventory automatically when the mouse
	// is outside it for some time
	_inventoryExit = false;
}

void Game::inventoryDone() {
	_vm->_mouse->cursorOn();
	setLoopStatus(kStatusOrdinary);

	_vm->_anims->unpauseAnimations();

	_vm->_anims->stop(kInventorySprite);

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] != kNoItem) {
			_vm->_anims->stop(kInventoryItemsID - _inventory[i]);
		}
	}

	// Reset item under cursor
	_itemUnderCursor = kNoItem;
}

void Game::inventoryDraw() {
	_vm->_anims->play(kInventorySprite);

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] != kNoItem) {
			_vm->_anims->play(kInventoryItemsID - _inventory[i]);
		}
	}
}

void Game::inventoryReload() {
	// Make sure all items are loaded into memory (e.g., after loading a
	// savegame) by re-putting them on the same spot in the inventory.
	for (uint i = 0; i < kInventorySlots; ++i) {
		putItem(_inventory[i], i);
	}
}

void Game::dialogueMenu(int dialogueID) {
	int oldLines, hit;

	char tmp[5];
	sprintf(tmp, "%d", dialogueID+1);
	Common::String ext(tmp);
	_dialogueArchive = new BArchive(dialoguePath + ext + ".dfw");

	debugC(4, kDraciLogicDebugLevel, "Starting dialogue (ID: %d, Archive: %s)",
	    dialogueID, (dialoguePath + ext + ".dfw").c_str());

	_currentDialogue = dialogueID;
	oldLines = 255;
	dialogueInit(dialogueID);

	do {
		_dialogueExit = false;
		hit = dialogueDraw();

		debugC(7, kDraciLogicDebugLevel,
			"hit: %d, _lines[hit]: %d, lastblock: %d, dialogueLines: %d, dialogueExit: %d",
			hit, _lines[hit], _lastBlock, _dialogueLinesNum, _dialogueExit);

		if ((!_dialogueExit) && (hit != -1) && (_lines[hit] != -1)) {
			if ((oldLines == 1) && (_dialogueLinesNum == 1) && (_lines[hit] == _lastBlock)) {
				break;
			}
			_currentBlock = _lines[hit];
			runDialogueProg(_dialogueBlocks[_lines[hit]]._program, 1);
		} else {
			break;
		}
		_lastBlock = _lines[hit];
		_dialogueVars[_dialogueOffsets[dialogueID] + _lastBlock] += 1;
		_dialogueBegin = false;
		oldLines = _dialogueLinesNum;

	} while (!_dialogueExit);

	dialogueDone();
	_currentDialogue = kNoDialogue;
}

int Game::dialogueDraw() {
	_dialogueLinesNum = 0;
	int i = 0;
	int ret = 0;

	Animation *anim;
	Text *dialogueLine;

	while ((_dialogueLinesNum < 4) && (i < _blockNum)) {
		GPL2Program blockTest;
		blockTest._bytecode = _dialogueBlocks[i]._canBlock;
		blockTest._length = _dialogueBlocks[i]._canLen;
		debugC(3, kDraciLogicDebugLevel, "Testing dialogue block %d", i);
		if (_vm->_script->testExpression(blockTest, 1)) {
			anim = _dialogueAnims[_dialogueLinesNum];
			dialogueLine = reinterpret_cast<Text *>(anim->getCurrentFrame());
			dialogueLine->setText(_dialogueBlocks[i]._title);

			dialogueLine->setColour(kLineInactiveColour);
			_lines[_dialogueLinesNum] = i;
			_dialogueLinesNum++;
		}
		++i;
	}

	for (i = _dialogueLinesNum; i < kDialogueLines; ++i) {
		_lines[i] = -1;
		anim = _dialogueAnims[i];
		dialogueLine = reinterpret_cast<Text *>(anim->getCurrentFrame());
		dialogueLine->setText("");
	}

	if (_dialogueLinesNum > 1) {
		// Call the game loop to enable interactivity until the user
		// selects his choice.  _animUnderCursor will be set.
		_vm->_mouse->cursorOn();
		loop(kInnerDuringDialogue, false);
		_vm->_mouse->cursorOff();

		bool notDialogueAnim = true;
		for (uint j = 0; j < kDialogueLines; ++j) {
			if (_dialogueAnims[j]->getID() == _animUnderCursor) {
				notDialogueAnim = false;
				break;
			}
		}

		if (notDialogueAnim) {
			ret = -1;
		} else {
			ret = _dialogueAnims[0]->getID() - _animUnderCursor;
		}
	} else {
		ret = _dialogueLinesNum - 1;
	}

	for (i = 0; i < kDialogueLines; ++i) {
		dialogueLine = reinterpret_cast<Text *>(_dialogueAnims[i]->getCurrentFrame());
		_dialogueAnims[i]->markDirtyRect(_vm->_screen->getSurface());
		dialogueLine->setText("");
	}

	return ret;
}

void Game::dialogueInit(int dialogID) {
	_vm->_mouse->setCursorType(kDialogueCursor);

	_blockNum = _dialogueArchive->size() / 3;
	_dialogueBlocks = new Dialogue[_blockNum];

	const BAFile *f;

	for (uint i = 0; i < kDialogueLines; ++i) {
		_lines[i] = 0;
	}

	for (int i = 0; i < _blockNum; ++i) {
		f = _dialogueArchive->getFile(i * 3);
		_dialogueBlocks[i]._canLen = f->_length;
		_dialogueBlocks[i]._canBlock = f->_data;

		f = _dialogueArchive->getFile(i * 3 + 1);

		// The first byte of the file is the length of the string (without the length)
		assert(f->_length - 1 == f->_data[0]);

		_dialogueBlocks[i]._title = Common::String((char *)(f->_data+1), f->_length-1);

		f = _dialogueArchive->getFile(i * 3 + 2);
		_dialogueBlocks[i]._program._bytecode = f->_data;
		_dialogueBlocks[i]._program._length = f->_length;
	}

	for (uint i = 0; i < kDialogueLines; ++i) {
		_vm->_anims->play(_dialogueAnims[i]->getID());
	}

	setLoopStatus(kStatusDialogue);
	_lastBlock = -1;
	_dialogueBegin = true;
}

void Game::dialogueDone() {
	for (uint i = 0; i < kDialogueLines; ++i) {
		_vm->_anims->stop(_dialogueAnims[i]->getID());
	}

	delete _dialogueArchive;
	delete[] _dialogueBlocks;

	setLoopStatus(kStatusOrdinary);
	_vm->_mouse->setCursorType(kNormalCursor);
}

void Game::runDialogueProg(GPL2Program prog, int offset) {
	// Mark last animation
	int lastAnimIndex = _vm->_anims->getLastIndex();

	// Run the dialogue program
	_vm->_script->run(prog, offset);

	deleteAnimationsAfterIndex(lastAnimIndex);
}

int Game::playHeroAnimation(int anim_index) {
	const GameObject *dragon = getObject(kDragonObject);
	const int current_anim_index = playingObjectAnimation(dragon);
	const int animID = dragon->_anim[anim_index];
	Animation *anim = _vm->_anims->getAnimation(animID);

	if (anim_index == current_anim_index) {
		anim->markDirtyRect(_vm->_screen->getSurface());
	} else {
		stopObjectAnimations(dragon);
	}
	positionAnimAsHero(anim);
	if (anim_index == current_anim_index) {
		anim->markDirtyRect(_vm->_screen->getSurface());
	} else {
		_vm->_anims->play(animID);
	}

	return anim->currentFrameNum();
}

void Game::redrawWalkingPath(int id, byte colour, const WalkingPath &path) {
	Animation *anim = _vm->_anims->getAnimation(id);
	Sprite *ov = _walkingMap.newOverlayFromPath(path, colour);
	delete anim->getFrame(0);
	anim->replaceFrame(0, ov, NULL);
	anim->markDirtyRect(_vm->_screen->getSurface());
}

void Game::setHeroPosition(const Common::Point &p) {
	debugC(3, kDraciWalkingDebugLevel, "Jump to x: %d y: %d", p.x, p.y);
	_hero = p;
}

Common::Point Game::findNearestWalkable(int x, int y) const {
	Surface *surface = _vm->_screen->getSurface();
	return _walkingMap.findNearestWalkable(x, y, surface->getDimensions());
}

void Game::walkHero(int x, int y, SightDirection dir) {
	if (!_currentRoom._heroOn) {
		// Nothing to do.  Happens for example in the map.
		return;
	}

	Common::Point target = findNearestWalkable(x, y);

	// Compute the shortest and obliqued path.
	WalkingPath shortestPath, obliquePath;
	_walkingMap.findShortestPath(_hero, target, &shortestPath);
	// TODO: test reachability and react
	_walkingMap.obliquePath(shortestPath, &obliquePath);
	debugC(2, kDraciWalkingDebugLevel, "Walking path lengths: shortest=%d oblique=%d", shortestPath.size(), obliquePath.size());
	if (_vm->_showWalkingMap) {
		redrawWalkingPath(kWalkingShortestPathOverlay, kWalkingShortestPathOverlayColour, shortestPath);
		redrawWalkingPath(kWalkingObliquePathOverlay, kWalkingObliquePathOverlayColour, obliquePath);
	}

	// Start walking.  Walking will be gradually advanced by
	// advanceAnimationsAndTestLoopExit(), which also handles calling the
	// callback and stopping the walk at the end.  If the hero is already
	// walking at this point, this command will cancel the previous path
	// and replace it by the current one (the callback has already been
	// reset by our caller).
	_walkingState.startWalking(_hero, target, Common::Point(x, y), dir,
		_walkingMap.getDelta(), obliquePath);
}

void Game::loadItem(int itemID) {
	const BAFile *f = _vm->_itemsArchive->getFile(itemID * 3);
	Common::MemoryReadStream itemReader(f->_data, f->_length);

	GameItem *item = _items + itemID;

	item->_init = itemReader.readSint16LE();
	item->_look = itemReader.readSint16LE();
	item->_use = itemReader.readSint16LE();
	item->_canUse = itemReader.readSint16LE();
	item->_imInit = itemReader.readByte();
	item->_imLook = itemReader.readByte();
	item->_imUse = itemReader.readByte();

	f = _vm->_itemsArchive->getFile(itemID * 3 + 1);

	// The first byte is the length of the string
	item->_title = Common::String((const char *)f->_data + 1, f->_length - 1);
	assert(f->_data[0] == item->_title.size());

	f = _vm->_itemsArchive->getFile(itemID * 3 + 2);

	item->_program._bytecode = f->_data;
	item->_program._length = f->_length;
}

void Game::loadRoom(int roomNum) {
	const BAFile *f;
	f = _vm->_roomsArchive->getFile(roomNum * 4);
	Common::MemoryReadStream roomReader(f->_data, f->_length);

	roomReader.readUint32LE(); // Pointer to room program, not used
	roomReader.readUint16LE(); // Program length, not used
	roomReader.readUint32LE(); // Pointer to room title, not used

	// Music will be played by the GPL2 command startMusic when needed.
	setMusicTrack(roomReader.readByte());

	_currentRoom._mapID = roomReader.readByte() - 1;
	_currentRoom._palette = roomReader.readByte() - 1;
	_currentRoom._numOverlays = roomReader.readSint16LE();
	_currentRoom._init = roomReader.readSint16LE();
	_currentRoom._look = roomReader.readSint16LE();
	_currentRoom._use = roomReader.readSint16LE();
	_currentRoom._canUse = roomReader.readSint16LE();
	_currentRoom._imInit = roomReader.readByte();
	_currentRoom._imLook = roomReader.readByte();
	_currentRoom._imUse = roomReader.readByte();
	_currentRoom._mouseOn = roomReader.readByte();
	_currentRoom._heroOn = roomReader.readByte();

	// Read in pers0 and persStep (stored as 6-byte Pascal reals)
	byte real[6];

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
	}

	_currentRoom._pers0 = real_to_double(real);

	for (int i = 5; i >= 0; --i) {
		real[i] = roomReader.readByte();
	}

	_currentRoom._persStep = real_to_double(real);

	_currentRoom._escRoom = roomReader.readByte() - 1;
	_currentRoom._numGates = roomReader.readByte();

	debugC(4, kDraciLogicDebugLevel, "Music: %d", getMusicTrack());
	debugC(4, kDraciLogicDebugLevel, "Map: %d", getMapID());
	debugC(4, kDraciLogicDebugLevel, "Palette: %d", _currentRoom._palette);
	debugC(4, kDraciLogicDebugLevel, "Overlays: %d", _currentRoom._numOverlays);
	debugC(4, kDraciLogicDebugLevel, "Init: %d", _currentRoom._init);
	debugC(4, kDraciLogicDebugLevel, "Look: %d", _currentRoom._look);
	debugC(4, kDraciLogicDebugLevel, "Use: %d", _currentRoom._use);
	debugC(4, kDraciLogicDebugLevel, "CanUse: %d", _currentRoom._canUse);
	debugC(4, kDraciLogicDebugLevel, "ImInit: %d", _currentRoom._imInit);
	debugC(4, kDraciLogicDebugLevel, "ImLook: %d", _currentRoom._imLook);
	debugC(4, kDraciLogicDebugLevel, "ImUse: %d", _currentRoom._imUse);
	debugC(4, kDraciLogicDebugLevel, "MouseOn: %d", _currentRoom._mouseOn);
	debugC(4, kDraciLogicDebugLevel, "HeroOn: %d", _currentRoom._heroOn);
	debugC(4, kDraciLogicDebugLevel, "Pers0: %f", _currentRoom._pers0);
	debugC(4, kDraciLogicDebugLevel, "PersStep: %f", _currentRoom._persStep);
	debugC(4, kDraciLogicDebugLevel, "EscRoom: %d", _currentRoom._escRoom);
	debugC(4, kDraciLogicDebugLevel, "Gates: %d", _currentRoom._numGates);

	// Read in the gates' numbers
	_currentRoom._gates.clear();
	for (uint i = 0; i < _currentRoom._numGates; ++i) {
		_currentRoom._gates.push_back(roomReader.readSint16LE());
	}

	// Add overlays for the walking map and shortest/obliqued paths.
	Animation *map = _vm->_anims->addAnimation(kWalkingMapOverlay, 256, _vm->_showWalkingMap);
	map->addFrame(NULL, NULL);	// rewritten below by loadWalkingMap()

	Animation *sPath = _vm->_anims->addAnimation(kWalkingShortestPathOverlay, 257, _vm->_showWalkingMap);
	Animation *oPath = _vm->_anims->addAnimation(kWalkingObliquePathOverlay, 258, _vm->_showWalkingMap);
	WalkingPath emptyPath;
	Sprite *ov = _walkingMap.newOverlayFromPath(emptyPath, 0);
	sPath->addFrame(ov, NULL);
	ov = _walkingMap.newOverlayFromPath(emptyPath, 0);
	oPath->addFrame(ov, NULL);

	// Load the walking map
	loadWalkingMap(getMapID());

	// Load the room's objects
	for (uint i = 0; i < _info._numObjects; ++i) {
		debugC(7, kDraciLogicDebugLevel,
			"Checking if object %d (%d) is at the current location (%d)", i,
			_objects[i]._location, roomNum);

		if (_objects[i]._location == roomNum) {
			debugC(6, kDraciLogicDebugLevel, "Loading object %d from room %d", i, roomNum);
			loadObject(i);
		}
	}

	// Run the init scripts for room objects
	// We can't do this in the above loop because some objects' scripts reference
	// other objects that may not yet be loaded
	for (uint i = 0; i < _info._numObjects; ++i) {
		if (_objects[i]._location == roomNum) {
			const GameObject *obj = getObject(i);
			debugC(6, kDraciLogicDebugLevel,
				"Running init program for object %d (offset %d)", i, obj->_init);
			_vm->_script->run(obj->_program, obj->_init);
		}
	}

	// Load the room's GPL program and run the init part
	f = _vm->_roomsArchive->getFile(roomNum * 4 + 3);
	_currentRoom._program._bytecode = f->_data;
	_currentRoom._program._length = f->_length;

	debugC(4, kDraciLogicDebugLevel, "Running room init program...");
	_vm->_script->run(_currentRoom._program, _currentRoom._init);

	// Set room palette
	f = _vm->_paletteArchive->getFile(_currentRoom._palette);
	_vm->_screen->setPalette(f->_data, 0, kNumColours);
}

int Game::loadAnimation(uint animNum, uint z) {
	// Make double-sure that an animation isn't loaded more than twice,
	// otherwise horrible things happen in the AnimationManager, because
	// they use a simple link-list without duplicate checking.  This should
	// never happen unless there is a bug in the game, because all GPL2
	// commands are guarded.
	assert(!_vm->_anims->getAnimation(animNum));

	const BAFile *animFile = _vm->_animationsArchive->getFile(animNum);
	Common::MemoryReadStream animationReader(animFile->_data, animFile->_length);

	uint numFrames = animationReader.readByte();

	// The following two flags are ignored by the played.  Memory logic was
	// a hint to the old player whether it should cache the sprites or load
	// them on demand.  We have 1 memory manager and ignore these hints.
	animationReader.readByte();
	// The disable erasing field is just a (poor) optimization flag that
	// turns of drawing the background underneath the sprite.  By reading
	// the source code of the old player, I'm not sure if that would ever
	// have worked.  There are only 6 animations in the game with this flag
	// true.  All of them have just 1 animation phase and they are used to
	// patch a part of the original background by a new sprite.  This
	// should work with the default logic as well---just play this
	// animation on top of the background.  Since the only meaning of the
	// flag was optimization, ignoring should be OK even without dipping
	// into details.
	animationReader.readByte();
	const bool cyclic = animationReader.readByte();
	const bool relative = animationReader.readByte();

	Animation *anim = _vm->_anims->addAnimation(animNum, z, false);

	anim->setLooping(cyclic);

	for (uint i = 0; i < numFrames; ++i) {
		uint spriteNum = animationReader.readUint16LE() - 1;
		int x = animationReader.readSint16LE();
		int y = animationReader.readSint16LE();
		uint scaledWidth = animationReader.readUint16LE();
		uint scaledHeight = animationReader.readUint16LE();
		byte mirror = animationReader.readByte();
		int sample = animationReader.readUint16LE() - 1;
		uint freq = animationReader.readUint16LE();
		uint delay = animationReader.readUint16LE();

		// _spritesArchive is flushed when entering a room.  All
		// scripts in a room are responsible for loading their animations.
		const BAFile *spriteFile = _vm->_spritesArchive->getFile(spriteNum);
		Sprite *sp = new Sprite(spriteFile->_data, spriteFile->_length,
			relative ? 0 : x, relative ? 0 : y, true);

		// Some frames set the scaled dimensions to 0 even though other frames
		// from the same animations have them set to normal values
		// We work around this by assuming it means no scaling is necessary
		if (scaledWidth == 0) {
			scaledWidth = sp->getWidth();
		}

		if (scaledHeight == 0) {
			scaledHeight = sp->getHeight();
		}

		sp->setScaled(scaledWidth, scaledHeight);

		if (mirror)
			sp->setMirrorOn();

		sp->setDelay(delay * 10);

		const SoundSample *sam = _vm->_soundsArchive->getSample(sample, freq);

		anim->addFrame(sp, sam);
		if (relative) {
			anim->makeLastFrameRelative(x, y);
		}
	}

	return animNum;
}

void Game::loadObject(uint objNum) {
	const BAFile *file;

	file = _vm->_objectsArchive->getFile(objNum * 3);
	Common::MemoryReadStream objReader(file->_data, file->_length);

	GameObject *obj = _objects + objNum;

	obj->_init = objReader.readUint16LE();
	obj->_look = objReader.readUint16LE();
	obj->_use = objReader.readUint16LE();
	obj->_canUse = objReader.readUint16LE();
	obj->_imInit = objReader.readByte();
	obj->_imLook = objReader.readByte();
	obj->_imUse = objReader.readByte();
	obj->_walkDir = objReader.readByte() - 1;
	obj->_z = objReader.readByte();
	objReader.readUint16LE(); // idxSeq field, not used
	objReader.readUint16LE(); // numSeq field, not used
	obj->_lookX = objReader.readUint16LE();
	obj->_lookY = objReader.readUint16LE();
	obj->_useX = objReader.readUint16LE();
	obj->_useY = objReader.readUint16LE();
	obj->_lookDir = static_cast<SightDirection> (objReader.readByte());
	obj->_useDir = static_cast<SightDirection> (objReader.readByte());

	obj->_absNum = objNum;

	file = _vm->_objectsArchive->getFile(objNum * 3 + 1);

	// The first byte of the file is the length of the string (without the length)
	assert(file->_length - 1 == file->_data[0]);

	obj->_title = Common::String((char *)(file->_data+1), file->_length-1);

	file = _vm->_objectsArchive->getFile(objNum * 3 + 2);
	obj->_program._bytecode = file->_data;
	obj->_program._length = file->_length;
}

void Game::loadWalkingMap(int mapID) {
	const BAFile *f;
	f = _vm->_walkingMapsArchive->getFile(mapID);
	_walkingMap.load(f->_data, f->_length);

	Animation *anim = _vm->_anims->getAnimation(kWalkingMapOverlay);
	Sprite *ov = _walkingMap.newOverlayFromMap(kWalkingMapOverlayColour);
	delete anim->getFrame(0);
	anim->replaceFrame(0, ov, NULL);
	anim->markDirtyRect(_vm->_screen->getSurface());
}

void Game::loadOverlays() {
	uint x, y, z, num;

	const BAFile *overlayHeader;

	overlayHeader = _vm->_roomsArchive->getFile(getRoomNum() * 4 + 2);
	Common::MemoryReadStream overlayReader(overlayHeader->_data, overlayHeader->_length);

	for (int i = 0; i < _currentRoom._numOverlays; i++) {
		num = overlayReader.readUint16LE() - 1;
		x = overlayReader.readUint16LE();
		y = overlayReader.readUint16LE();
		z = overlayReader.readByte();

		// _overlaysArchive is flushed when entering a room and this
		// code is called after the flushing has been done.
		const BAFile *overlayFile;
		overlayFile = _vm->_overlaysArchive->getFile(num);
		Sprite *sp = new Sprite(overlayFile->_data, overlayFile->_length, x, y, true);

		_vm->_anims->addOverlay(sp, z);
	}

	_vm->_screen->getSurface()->markDirty();
}

void Game::deleteObjectAnimations() {
	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];

		if (i != 0 && (obj->_location == getPreviousRoomNum())) {
			for (uint j = 0; j < obj->_anim.size(); ++j) {
					_vm->_anims->deleteAnimation(obj->_anim[j]);
			}
			obj->_anim.clear();
		}
	}
}

int Game::playingObjectAnimation(const GameObject *obj) const {
	for (uint i = 0; i < obj->_anim.size(); ++i) {
		const int animID = obj->_anim[i];
		const Animation *anim = _vm->_anims->getAnimation(animID);
		if (anim && anim->isPlaying()) {
			return i;
		}
	}
	return -1;
}

bool Game::enterNewRoom() {
	if (_newRoom == getRoomNum() && !isReloaded()) {
		// If the game has been reloaded, force reloading all animations.
		return true;
	}
	debugC(1, kDraciLogicDebugLevel, "Entering room %d using gate %d", _newRoom, _newGate);

	// TODO: maybe wait till all sounds end instead of stopping them.
	// In any case, make sure all sounds are stopped before we deallocate
	// their memory by clearing the cache.
	_vm->_sound->stopAll();

	// Clear archives
	_vm->_roomsArchive->clearCache();
	_vm->_spritesArchive->clearCache();
	_vm->_paletteArchive->clearCache();
	_vm->_animationsArchive->clearCache();
	_vm->_walkingMapsArchive->clearCache();
	_vm->_soundsArchive->clearCache();
	_vm->_dubbingArchive->clearCache();
	_vm->_overlaysArchive->clearCache();

	_vm->_screen->clearScreen();

	_vm->_anims->deleteOverlays();

	// Delete walking map testing overlay
	_vm->_anims->deleteAnimation(kWalkingMapOverlay);
	_vm->_anims->deleteAnimation(kWalkingShortestPathOverlay);
	_vm->_anims->deleteAnimation(kWalkingObliquePathOverlay);

	// TODO: Make objects capable of stopping their own animations
	const GameObject *dragon = getObject(kDragonObject);
	stopObjectAnimations(dragon);

	// Remember the previous room for returning back from the map.
	rememberRoomNumAsPrevious();
	deleteObjectAnimations();

	// Set the current room to the new value
	_currentRoom._roomNum = _newRoom;

	// Before setting these variables we have to convert the values to 1-based indexing
	// because this is how everything is stored in the data files
	_variables[0] = _newGate + 1;
	_variables[1] = _newRoom + 1;

	// If the new room is the map room, set the appropriate coordinates
	// for the dragon in the persons array
	if (_newRoom == _info._mapRoom) {
		_persons[kDragonObject]._x = 160;
	  	_persons[kDragonObject]._y = 0;
	}

	// Set the appropriate loop status before loading the room
	setLoopStatus(kStatusGate);

	// Make sure the possible walking path from the previous room is
	// cleaned up.  Some rooms (e.g., the map) don't support walking.
	_walkingState.stopWalking();

	// Clean the mouse and animation title.  It gets first updated in
	// loop(), hence if the hero walks during the initialization scripts,
	// the old values would remain otherwise.
	_vm->_mouse->setCursorType(kNormalCursor);
	Animation *titleAnim = _vm->_anims->getAnimation(kTitleText);
	titleAnim->markDirtyRect(_vm->_screen->getSurface());
	Text *title = reinterpret_cast<Text *>(titleAnim->getCurrentFrame());
	title->setText("");

	// Reset the flag allowing to run the scripts.  It may have been turned
	// on by pressing Escape in the intro or in the map room.
	_vm->_script->endCurrentProgram(false);

	loadRoom(_newRoom);
	loadOverlays();

	// Run the program for the gate the dragon came through
	runGateProgram(_newGate);

	// Set cursor state
	// Need to do this after we set the palette since the cursors use it
	if (_currentRoom._mouseOn) {
		debugC(6, kDraciLogicDebugLevel, "Mouse: ON");
		_vm->_mouse->cursorOn();
	} else {
		debugC(6, kDraciLogicDebugLevel, "Mouse: OFF");
		_vm->_mouse->cursorOff();
	}

	// Reset the loop status.
	setLoopStatus(kStatusOrdinary);

	setIsReloaded(false);
	if (_vm->_script->shouldEndProgram()) {
		// Escape pressed during the intro or map animations run in the
		// init scripts.  This flag was turned on to skip the rest of
		// those programs.  Return false to make start() rerun us from
		// the beginning, because the room number has changed.
		return false;
	}
	return true;
}

void Game::runGateProgram(int gate) {
	debugC(6, kDraciLogicDebugLevel, "Running program for gate %d", gate);

	// Mark last animation
	int lastAnimIndex = _vm->_anims->getLastIndex();

	// Run gate program
	_vm->_script->run(_currentRoom._program, _currentRoom._gates[gate]);

	deleteAnimationsAfterIndex(lastAnimIndex);

	setExitLoop(false);
}

void Game::positionAnimAsHero(Animation *anim) {
	// Calculate scaling factors
	const double scale = getPers0() + getPersStep() * _hero.y;

	// Set the Z coordinate for the dragon's animation
	anim->setZ(_hero.y);

	// Fetch current frame
	Drawable *frame = anim->getCurrentFrame();

	// We naturally want the dragon to position its feet to the location of the
	// click but sprites are drawn from their top-left corner so we subtract
	// the current height of the dragon's sprite
	Common::Point p = _hero;
	p.x -= scummvm_lround(scale * frame->getWidth() / 2);
	p.y -= scummvm_lround(scale * frame->getHeight());

	// Since _persons[] is used for placing talking text, we use the non-adjusted x value
	// so the text remains centered over the dragon.
	_persons[kDragonObject]._x = _hero.x;
	_persons[kDragonObject]._y = p.y;

	// Set the per-animation scaling factor
	anim->setScaleFactors(scale, scale);

	anim->setRelative(p.x, p.y);

	// Clear the animation's shift so that the real sprite stays at place
	// regardless of what the current phase is.  If the animation starts
	// from the beginning, the shift is already [0,0], but if it is in the
	// middle, it may be different.
	anim->clearShift();
}

void Game::positionHeroAsAnim(Animation *anim) {
	// Check out where the hero has moved to by composing the relative
	// shifts of the sprites.
	_hero = anim->getCurrentFramePosition();

	// Update our hero coordinates (don't forget that our control point is
	// elsewhere).
	// TODO: what about rounding errors?
	Drawable *frame = anim->getCurrentFrame();
	_hero.x += scummvm_lround(anim->getScaleX() * frame->getWidth() / 2);
	_hero.y += scummvm_lround(anim->getScaleY() * frame->getHeight());
}

void Game::pushNewRoom() {
	_pushedNewRoom = _newRoom;
	_pushedNewGate = _newGate;
}

void Game::popNewRoom() {
	if (_loopStatus != kStatusInventory && _pushedNewRoom >= 0) {
		scheduleEnteringRoomUsingGate(_pushedNewRoom, _pushedNewGate);
		_pushedNewRoom = _pushedNewGate = -1;
	}
}

void Game::setSpeechTiming(uint tick, uint duration) {
	_speechTick = tick;
	_speechDuration = duration;
}

void Game::shiftSpeechAndFadeTick(int delta) {
	_speechTick += delta;
	_fadeTick += delta;
}

void Game::initializeFading(int phases) {
	_fadePhases = _fadePhase = phases;
	_fadeTick = _vm->_system->getMillis();
}

void Game::deleteAnimationsAfterIndex(int lastAnimIndex) {
	// Delete all animations loaded after the marked one
	// (from objects and from the AnimationManager)
	for (uint i = 0; i < getNumObjects(); ++i) {
		GameObject *obj = &_objects[i];

		for (uint j = 0; j < obj->_anim.size(); ++j) {
			Animation *anim;

			anim = _vm->_anims->getAnimation(obj->_anim[j]);
			if (anim != NULL && anim->getIndex() > lastAnimIndex)
				obj->_anim.remove_at(j--);
		}
	}

	_vm->_anims->deleteAfterIndex(lastAnimIndex);
}

void Game::stopObjectAnimations(const GameObject *obj) {
	for (uint i = 0; i < obj->_anim.size(); ++i) {
		_vm->_anims->stop(obj->_anim[i]);
	}
}

Game::~Game() {
	delete[] _persons;
	delete[] _variables;
	delete[] _dialogueOffsets;
	delete[] _dialogueVars;
	delete[] _objects;
	delete[] _itemStatus;
	delete[] _items;
}

void Game::DoSync(Common::Serializer &s) {
	s.syncAsUint16LE(_currentRoom._roomNum);

	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject& obj = _objects[i];
		s.syncAsSint16LE(obj._location);
		s.syncAsByte(obj._visible);
	}

	for (uint i = 0; i < _info._numItems; ++i) {
		s.syncAsByte(_itemStatus[i]);
	}

	for (int i = 0; i < kInventorySlots; ++i) {
		s.syncAsSint16LE(_inventory[i]);
	}

	for (int i = 0; i < _info._numVariables; ++i) {
		s.syncAsSint16LE(_variables[i]);
	}
	for (uint i = 0; i < _info._numDialogueBlocks; ++i) {
		s.syncAsSint16LE(_dialogueVars[i]);
	}

}

static double real_to_double(byte real[6]) {
	// Extract sign bit
	int sign = real[0] & (1 << 7);

	// Extract exponent and adjust for bias
	int exp = real[5] - 129;

	double mantissa;
	double tmp = 0.0;

	if (real[5] == 0) {
		mantissa = 0.0;
	} else {
		// Process the first four least significant bytes
		for (int i = 4; i >= 1; --i) {
			tmp += real[i];
			tmp /= 1 << 8;
		}

		// Process the most significant byte (remove the sign bit)
		tmp += real[0] & ((1 << 7) - 1);
		tmp /= 1 << 8;

		// Calculate mantissa
		mantissa = 1.0;
		mantissa += 2.0 * tmp;
	}

	// Flip sign if necessary
	if (sign) {
		mantissa = -mantissa;
	}

	// Calculate final value
	return ldexp(mantissa, exp);
}

}
