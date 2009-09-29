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

const Common::String dialoguePath("ROZH");

static double real_to_double(byte real[6]);

Game::Game(DraciEngine *vm) : _vm(vm) {
	unsigned int i;
	
	BArchive *initArchive = _vm->_initArchive;
	const BAFile *file;
	
	// Read in persons
	file = initArchive->getFile(5);
	Common::MemoryReadStream personData(file->_data, file->_length);
	
	unsigned int numPersons = file->_length / personSize;
	_persons = new Person[numPersons];
	
	for (i = 0; i < numPersons; ++i) {
		_persons[i]._x = personData.readUint16LE();
		_persons[i]._y = personData.readUint16LE();
		_persons[i]._fontColour = personData.readByte();
	}

	// Read in dialogue offsets
	file = initArchive->getFile(4);
	Common::MemoryReadStream dialogueData(file->_data, file->_length);
	
	unsigned int numDialogues = file->_length / sizeof(uint16);
	_dialogueOffsets = new uint[numDialogues];	
	
	unsigned int curOffset;
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
	unsigned int numVariables = file->_length / sizeof (int16);

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
	unsigned int numObjects = file->_length;
	
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

		// Whenever the top-level loop is entered, it should not finish unless
		// the exit is triggered by a script
		_shouldExitLoop = false;

		// If the scheduled room differs from the current one, do a room change
		if (_newRoom != _currentRoom._roomNum) {

			// Set the first two variables to the new room / gate
			// Before setting these variables we have to convert the values to 
			// 1-based indexing because this is how everything is stored in the data files
			_variables[0] = _newGate + 1;
			_variables[1] = _newRoom + 1;

			// If the new room is the map room, set the appropriate coordinates
			// for the dragon in the persons array
			if (_newRoom == _info._mapRoom) {
				_persons[kDragonObject]._x = 160;
				_persons[kDragonObject]._y = 0;
			}

			setLoopSubstatus(kSubstatusOrdinary);
	
			// Do the actual change
			changeRoom(_newRoom);
			
			// Set the current room / gate to the new value
			_currentRoom._roomNum = _newRoom;
			_currentGate = _newGate;

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
 		}

		// Mimic the original engine by setting the loop status to Ordinary before
		// entering the main loop
		setLoopStatus(kStatusOrdinary);

		loop();
	}
}

void Game::init() {
	_shouldQuit = false;
	_shouldExitLoop = false;
	_scheduledPalette = 0;

	_animUnderCursor = kOverlayImage;

	_currentItem = kNoItem;
	_itemUnderCursor = kNoItem;

	_vm->_mouse->setCursorType(kNormalCursor);

	_loopStatus = kStatusOrdinary;
	_objUnderCursor = kOverlayImage;

	// Set the inventory to empty initially
	memset(_inventory, kNoItem, kInventorySlots * sizeof (int));

	// Initialize animation for object / room titles
	Animation *titleAnim = _vm->_anims->addText(kTitleText, true);
	Text *title = new Text("", _vm->_smallFont, kTitleColour, 0, 0);
	titleAnim->addFrame(title);

	// Initialize animation for speech text
	Animation *speechAnim = _vm->_anims->addText(kSpeechText, true);
	Text *speech = new Text("", _vm->_bigFont, kFontColour1, 0, 0);
	speechAnim->addFrame(speech);

	// Initialize inventory animation
	const BAFile *f = _vm->_iconsArchive->getFile(13);
	Animation *inventoryAnim = _vm->_anims->addAnimation(kInventorySprite, 255, false);
	Sprite *inventorySprite = new Sprite(f->_data, f->_length, 0, 0, true);
	inventoryAnim->addFrame(inventorySprite);
	inventoryAnim->setRelative((kScreenWidth - inventorySprite->getWidth()) / 2,
								(kScreenHeight - inventorySprite->getHeight()) / 2);

	for (uint i = 0; i < kDialogueLines; ++i) {
		_dialogueAnims[i] = _vm->_anims->addText(kDialogueLinesID - i, true);
		Text *dialogueLine = new Text("", _vm->_smallFont, kLineInactiveColour, 0, 0);
		_dialogueAnims[i]->addFrame(dialogueLine);

		_dialogueAnims[i]->setZ(254);
		_dialogueAnims[i]->setRelative(1, 
			kScreenHeight - (i + 1) * _vm->_smallFont->getFontHeight());

		Text *text = reinterpret_cast<Text *>(_dialogueAnims[i]->getFrame());
		text->setText("");
	}

	for (uint i = 0; i < _info._numItems; ++i) {
		loadItem(i);
	}

	loadObject(kDragonObject);
	
	const GameObject *dragon = getObject(kDragonObject);
	debugC(4, kDraciLogicDebugLevel, "Running init program for the dragon object...");
	_vm->_script->run(dragon->_program, dragon->_init);

	_currentRoom._roomNum = _info._startRoom;
	_currentGate = 0;

	_newRoom = _currentRoom._roomNum;
	_newGate = _currentGate;

	// Before setting these variables we have to convert the values to 1-based indexing
	// because this is how everything is stored in the data files
	_variables[0] = _currentGate + 1;
	_variables[1] = _currentRoom._roomNum + 1;

	changeRoom(_currentRoom._roomNum);
	runGateProgram(_currentGate);
}

void Game::loop() {

	Surface *surface = _vm->_screen->getSurface();

	do {

	debugC(4, kDraciLogicDebugLevel, "loopstatus: %d, loopsubstatus: %d", 
		_loopStatus, _loopSubstatus);

		_vm->handleEvents();

		// Fetch mouse coordinates			
		int x = _vm->_mouse->getPosX();
		int y = _vm->_mouse->getPosY();

		if (_loopStatus == kStatusDialogue && _loopSubstatus == kSubstatusOrdinary) {

			Text *text;
			for (int i = 0; i < kDialogueLines; ++i) {
				text = reinterpret_cast<Text *>(_dialogueAnims[i]->getFrame());
				
				if (_animUnderCursor == _dialogueAnims[i]->getID()) {
					text->setColour(kLineActiveColour);
				} else {
					text->setColour(kLineInactiveColour);
				}
			}
	
			if (_vm->_mouse->lButtonPressed() || _vm->_mouse->rButtonPressed()) {
				_shouldExitLoop = true;
				_vm->_mouse->lButtonSet(false);
				_vm->_mouse->rButtonSet(false);
			}
		}

		if(_vm->_mouse->isCursorOn()) {

			// Fetch the dedicated objects' title animation / current frame
			Animation *titleAnim = _vm->_anims->getAnimation(kTitleText);
			Text *title = reinterpret_cast<Text *>(titleAnim->getFrame());

			updateCursor();
			updateTitle();

			if (_loopStatus == kStatusOrdinary && _loopSubstatus == kSubstatusOrdinary) {

				if (_vm->_mouse->lButtonPressed()) {
					_vm->_mouse->lButtonSet(false);				
					
					if (_currentItem != kNoItem) {
						putItem(_currentItem, 0);
						_currentItem = kNoItem;
						updateCursor();
					} else {
						if (_objUnderCursor != kObjectNotFound) {
							GameObject *obj = &_objects[_objUnderCursor];
		
							_vm->_mouse->cursorOff();
							titleAnim->markDirtyRect(surface);
							title->setText("");
							_objUnderCursor = kObjectNotFound;

							if (!obj->_imLook) {
								if (obj->_lookDir == -1) {
									walkHero(x, y);
								} else {
									walkHero(obj->_lookX, obj->_lookY);
								}
							}

							_vm->_script->run(obj->_program, obj->_look);
							_vm->_mouse->cursorOn();
						} else {
							walkHero(x, y);
						}
					}
				}

				if (_vm->_mouse->rButtonPressed()) {
					_vm->_mouse->rButtonSet(false);

					if (_objUnderCursor != kObjectNotFound) {
						GameObject *obj = &_objects[_objUnderCursor];

						if (_vm->_script->testExpression(obj->_program, obj->_canUse)) {
							_vm->_mouse->cursorOff();
							titleAnim->markDirtyRect(surface);
							title->setText("");
							_objUnderCursor = kObjectNotFound;

							if (!obj->_imUse) {
								if (obj->_useDir == -1) {
									walkHero(x, y);
								} else {
									walkHero(obj->_useX, obj->_useY);
								}
							}

							_vm->_script->run(obj->_program, obj->_use);
							_vm->_mouse->cursorOn();
						} else {
							walkHero(x, y);
						}
					} else {
						if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
							_vm->_mouse->cursorOff();
							titleAnim->markDirtyRect(surface);
							title->setText("");


							_vm->_script->run(_currentRoom._program, _currentRoom._use);
							_vm->_mouse->cursorOn();
						} else {
							walkHero(x, y);
						}
					}
				}
			}

			if (_loopStatus == kStatusInventory && _loopSubstatus == kSubstatusOrdinary) {
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
						const GPL2Program &program = _items[_itemUnderCursor]._program;
						const int lookOffset = _items[_itemUnderCursor]._look;

						_vm->_script->run(program, lookOffset);
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
							const GPL2Program &program = _items[_itemUnderCursor]._program;
							const int canUseOffset = _items[_itemUnderCursor]._canUse;
							const int useOffset = _items[_itemUnderCursor]._use;

							if (_vm->_script->testExpression(program, canUseOffset)) {
								_vm->_script->run(program, useOffset);
							}
						}
						updateCursor();
					}
				}
			}		
		}		
		
		debugC(5, kDraciLogicDebugLevel, "Anim under cursor: %d", _animUnderCursor); 

		// Handle character talking (if there is any)
		if (_loopSubstatus == kSubstatusTalk) {
			Animation *speechAnim = _vm->_anims->getAnimation(kSpeechText); 			
			Text *speechFrame = reinterpret_cast<Text *>(speechAnim->getFrame());

			uint speechDuration = kBaseSpeechDuration + 
								  speechFrame->getLength() * kSpeechTimeUnit / 
							 	  (128 / 16 + 1);

			// If the current speech text has expired or the user clicked a mouse button, 
			// advance to the next line of text			
			if (_vm->_mouse->lButtonPressed() || 
				_vm->_mouse->rButtonPressed() || 
				(_vm->_system->getMillis() - _speechTick) >= speechDuration) {

				_shouldExitLoop = true;
				_vm->_mouse->lButtonSet(false);
				_vm->_mouse->rButtonSet(false);
			}
		}

		// This returns true if we got a signal to quit the game
		if (shouldQuit())
			return;

		// Advance animations and redraw screen
		_vm->_anims->drawScene(surface);
		_vm->_screen->copyToScreen();
		_vm->_system->delayMillis(20);

		// HACK: Won't be needed once the game loop is implemented properly
		_shouldExitLoop = _shouldExitLoop || (_newRoom != _currentRoom._roomNum && 
						(_loopStatus == kStatusOrdinary || _loopStatus == kStatusGate));

	} while (!shouldExitLoop());
}

void Game::updateCursor() {

	// Fetch mouse coordinates			
	int x = _vm->_mouse->getPosX();
	int y = _vm->_mouse->getPosY();

	// Find animation under cursor
	_animUnderCursor = _vm->_anims->getTopAnimationID(x, y);

	// If we are inside a dialogue, all we need is to update the ID of the current
	// animation under the cursor. This enables us to update the currently selected
	// dialogue line (by recolouring it) but still leave the cursor unupdated when
	// over background objects.
	if (_loopStatus == kStatusDialogue)
		return;

	// If we are in inventory mode, we do a different kind of updating that handles
	// inventory items and return early
	if (_loopStatus == kStatusInventory && _loopSubstatus == kSubstatusOrdinary) {

		if (_currentItem == kNoItem) {
			_vm->_mouse->setCursorType(kNormalCursor);
		} else {
			_vm->_mouse->loadItemCursor(_currentItem);
		}

		if (_itemUnderCursor != kNoItem) {
			const GPL2Program &program = _items[_itemUnderCursor]._program;
			const int canUseOffset = _items[_itemUnderCursor]._canUse;

			if (_vm->_script->testExpression(program, canUseOffset)) {
				if (_currentItem == kNoItem) {
					_vm->_mouse->setCursorType(kHighlightedCursor);				
				} else {
					_vm->_mouse->loadItemCursor(_currentItem, true);			
				}
			}
		}

		return;
	}

	// Find the game object under the cursor
	// (to be more precise, one that corresponds to the animation under the cursor)
	int curObject = getObjectWithAnimation(_animUnderCursor);

	// Update the game object under the cursor
	_objUnderCursor = curObject;
	if (_objUnderCursor != _oldObjUnderCursor) {
		_oldObjUnderCursor = _objUnderCursor;
	}

	// Load the appropriate cursor (item image if an item is held or ordinary cursor
	// if not)
	if (_currentItem == kNoItem) {
		_vm->_mouse->setCursorType(kNormalCursor);
	} else {
		_vm->_mouse->loadItemCursor(_currentItem);
	}

	// TODO: Handle main menu

	// If there is no game object under the cursor, try using the room itself
	if (_objUnderCursor == kObjectNotFound) {					

		if (_vm->_script->testExpression(_currentRoom._program, _currentRoom._canUse)) {
			if (_currentItem == kNoItem) {
				_vm->_mouse->setCursorType(kHighlightedCursor);
			} else {
				_vm->_mouse->loadItemCursor(_currentItem, true);
			}
		}
	// If there *is* a game object under the cursor, update the cursor image
	} else {
		GameObject *obj = &_objects[_objUnderCursor];
		
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
			}
		// If the walking direction *is* set, the game object is a gate, so update
		// the cursor image to the appropriate arrow.
		} else {
			_vm->_mouse->setCursorType((CursorType)obj->_walkDir);
		}
	}
}

void Game::updateTitle() {

	// If we are inside a dialogue, don't update titles
	if (_loopStatus == kStatusDialogue)
		return;

	// Fetch current surface and height of the small font (used for titles)
	Surface *surface = _vm->_screen->getSurface();
	const int smallFontHeight = _vm->_smallFont->getFontHeight();

	// Fetch mouse coordinates			
	int x = _vm->_mouse->getPosX();
	int y = _vm->_mouse->getPosY();

	// Fetch the dedicated objects' title animation / current frame
	Animation *titleAnim = _vm->_anims->getAnimation(kTitleText);
	Text *title = reinterpret_cast<Text *>(titleAnim->getFrame());

	// Mark dirty rectangle to delete the previous text
	titleAnim->markDirtyRect(surface);	
	
	// If there is no object under the cursor, delete the title.
	// Otherwise, show the object's title.
	if (_objUnderCursor == kObjectNotFound) {
		title->setText("");
	} else {
		GameObject *obj = &_objects[_objUnderCursor];	
		title->setText(obj->_title);
	}

	// Move the title to the correct place (just above the cursor)
	int newX = surface->centerOnX(x, title->getWidth());
	int newY = surface->centerOnY(y - smallFontHeight / 2, title->getHeight() * 2);
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
	
		for (uint j = 0; j < obj->_anims.size(); ++j) {
			if (obj->_anims[j] == animID) {
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

	uint i = position;

	if (position >= 0 && 
		position < kInventoryLines * kInventoryColumns &&
		_inventory[position] == kNoItem) {
		_inventory[position] = itemID;
	} else {
		for (i = 0; i < kInventorySlots; ++i) {
			if (_inventory[i] == kNoItem) {
				_inventory[i] = itemID;
				break;		
			}
		}
	}

	const int line = i / kInventoryColumns + 1;
	const int column = i % kInventoryColumns + 1;

	Animation *anim = _vm->_anims->getAnimation(kInventoryItemsID - itemID);
	Drawable *frame = anim->getFrame();

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
	if (_loopStatus == kStatusInventory && _loopSubstatus == kSubstatusOrdinary) {
		_vm->_anims->play(kInventoryItemsID - itemID);
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
	_loopStatus = kStatusInventory;

	// TODO: This will be used for exiting the inventory automatically when the mouse
	// is outside it for some time
	_inventoryExit = false;
}

void Game::inventoryDone() {
	_vm->_mouse->cursorOn();
	_loopStatus = kStatusOrdinary;

	_vm->_anims->unpauseAnimations();

	_vm->_anims->stop(kInventorySprite);

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] != kNoItem) {
			_vm->_anims->stop(kInventoryItemsID - _inventory[i]);
		}
	}

	// Reset item under cursor
	_itemUnderCursor = kNoItem;

	// TODO: Handle main menu
}

void Game::inventoryDraw() {

	_vm->_anims->play(kInventorySprite);

	for (uint i = 0; i < kInventorySlots; ++i) {
		if (_inventory[i] != kNoItem) {
			_vm->_anims->play(kInventoryItemsID - _inventory[i]);
		}
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

	} while(!_dialogueExit);

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
			dialogueLine = reinterpret_cast<Text *>(anim->getFrame());
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
		dialogueLine = reinterpret_cast<Text *>(anim->getFrame());
		dialogueLine->setText("");
	}

	_oldObjUnderCursor = kObjectNotFound;

	if (_dialogueLinesNum > 1) {
		_vm->_mouse->cursorOn();
		_shouldExitLoop = false;
		loop();
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
		dialogueLine = reinterpret_cast<Text *>(_dialogueAnims[i]->getFrame());
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

	_loopStatus = kStatusDialogue;
	_lastBlock = -1;
	_dialogueBegin = true;
}

void Game::dialogueDone() {
	for (uint i = 0; i < kDialogueLines; ++i) {
		_vm->_anims->stop(_dialogueAnims[i]->getID());
	}

	_dialogueArchive->closeArchive();

	delete[] _dialogueBlocks;

	_loopStatus = kStatusOrdinary;
	_vm->_mouse->setCursorType(kNormalCursor);
}

void Game::runDialogueProg(GPL2Program prog, int offset) {

	// Mark last animation
	int lastAnimIndex = _vm->_anims->getLastIndex();

	// Run the dialogue program
	_vm->_script->run(prog, offset);
	
	deleteAnimationsAfterIndex(lastAnimIndex);
}

bool Game::isDialogueBegin() const {
	return _dialogueBegin;
}

bool Game::shouldExitDialogue() const {
	return _dialogueExit;
}

void Game::setDialogueExit(bool exit) {
	_dialogueExit = exit;
}

int Game::getDialogueBlockNum() const {
	return _blockNum;
}

int Game::getDialogueVar(int dialogueID) const {
	return _dialogueVars[dialogueID];
}

void Game::setDialogueVar(int dialogueID, int value) {
	_dialogueVars[dialogueID] = value;
}

int Game::getCurrentDialogue() const {
	return _currentDialogue;
}

int Game::getDialogueLastBlock() const {
	return _lastBlock;
}

int Game::getDialogueLinesNum() const {
	return _dialogueLinesNum;	
}

int Game::getDialogueCurrentBlock() const {
	return _currentBlock;	
}

int Game::getCurrentDialogueOffset() const {
	return _dialogueOffsets[_currentDialogue];
}

void Game::walkHero(int x, int y) {

	Surface *surface = _vm->_screen->getSurface();
	
	_hero = _currentRoom._walkingMap.findNearestWalkable(x, y, surface->getRect());

	GameObject *dragon = getObject(kDragonObject); 

	for (uint i = 0; i < dragon->_anims.size(); ++i) {
		_vm->_anims->stop(dragon->_anims[i]);
	}

	debugC(3, kDraciLogicDebugLevel, "Walk to x: %d y: %d", _hero.x, _hero.y);

	// Fetch dragon's animation ID
	// FIXME: Need to add proper walking (this only warps the dragon to position)
	int animID = dragon->_anims[0];

	Animation *anim = _vm->_anims->getAnimation(animID);
	positionAnimAsHero(anim);

	// Play the animation
	_vm->_anims->play(animID);
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

	_currentRoom._music = roomReader.readByte();

	int mapID = roomReader.readByte() - 1;
	loadWalkingMap(mapID);

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

	debugC(4, kDraciLogicDebugLevel, "Music: %d", _currentRoom._music);
	debugC(4, kDraciLogicDebugLevel, "Map: %d", mapID);
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
	
	// Run the init scripts	for room objects
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

	// HACK: Create a visible overlay from the walking map so we can test it
	byte *wlk = new byte[kScreenWidth * kScreenHeight];
	memset(wlk, 255, kScreenWidth * kScreenHeight);

	for (uint i = 0; i < kScreenWidth; ++i) {
		for (uint j = 0; j < kScreenHeight; ++j) {
			if (_currentRoom._walkingMap.isWalkable(i, j)) {
				wlk[j * kScreenWidth + i] = 2;
			}
		}
	}

	Sprite *ov = new Sprite(wlk, kScreenWidth, kScreenHeight, 0, 0, false);

	Animation *map = _vm->_anims->addAnimation(kWalkingMapOverlay, 255, false);
	map->addFrame(ov);
}

int Game::loadAnimation(uint animNum, uint z) {

	const BAFile *animFile = _vm->_animationsArchive->getFile(animNum);
	Common::MemoryReadStream animationReader(animFile->_data, animFile->_length);	

	uint numFrames = animationReader.readByte();

	// FIXME: handle these properly
	animationReader.readByte(); // Memory logic field, not used
	animationReader.readByte(); // Disable erasing field, not used
	
	bool cyclic = animationReader.readByte();

	animationReader.readByte(); // Relative field, not used

	Animation *anim = _vm->_anims->addAnimation(animNum, z, false);

	anim->setLooping(cyclic);

	for (uint i = 0; i < numFrames; ++i) {
		uint spriteNum = animationReader.readUint16LE() - 1;
		int x = animationReader.readSint16LE();
		int y = animationReader.readSint16LE();
		uint scaledWidth = animationReader.readUint16LE();
		uint scaledHeight = animationReader.readUint16LE();
		byte mirror = animationReader.readByte();
		/* uint sample = */ animationReader.readUint16LE();
		/* uint freq = */ animationReader.readUint16LE();
		uint delay = animationReader.readUint16LE();

		const BAFile *spriteFile = _vm->_spritesArchive->getFile(spriteNum);

		Sprite *sp = new Sprite(spriteFile->_data, spriteFile->_length, x, y, true);

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

		anim->addFrame(sp);
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
	obj->_lookDir = objReader.readByte() - 1;
	obj->_useDir = objReader.readByte() - 1;
	
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
	_currentRoom._walkingMap.load(f->_data, f->_length);
}

GameObject *Game::getObject(uint objNum) {
	return _objects + objNum;
}

uint Game::getNumObjects() const {
	return _info._numObjects;
}

void Game::loadOverlays() {
 	uint x, y, z, num;
 
	const BAFile *overlayHeader;

	overlayHeader = _vm->_roomsArchive->getFile(_currentRoom._roomNum * 4 + 2);
	Common::MemoryReadStream overlayReader(overlayHeader->_data, overlayHeader->_length);
 
 	for (int i = 0; i < _currentRoom._numOverlays; i++) {
 
		num = overlayReader.readUint16LE() - 1;
 		x = overlayReader.readUint16LE();
 		y = overlayReader.readUint16LE();
 		z = overlayReader.readByte();
 
		const BAFile *overlayFile;
		overlayFile = _vm->_overlaysArchive->getFile(num);
 		Sprite *sp = new Sprite(overlayFile->_data, overlayFile->_length, x, y, true);
 
 		_vm->_anims->addOverlay(sp, z);		
	}

	_vm->_overlaysArchive->clearCache();

 	_vm->_screen->getSurface()->markDirty();
}

void Game::changeRoom(uint roomNum) {
	
	debugC(1, kDraciLogicDebugLevel, "Changing to room %d", roomNum);

	// Clear archives
	_vm->_roomsArchive->clearCache();
	_vm->_spritesArchive->clearCache();
	_vm->_paletteArchive->clearCache();
	_vm->_animationsArchive->clearCache();
	_vm->_walkingMapsArchive->clearCache();

	_vm->_screen->clearScreen();

	_vm->_anims->deleteOverlays();
	
	// Delete walking map testing overlay
	_vm->_anims->deleteAnimation(kWalkingMapOverlay);

	int oldRoomNum = _currentRoom._roomNum;

	// TODO: Make objects capable of stopping their own animations
	const GameObject *dragon = getObject(kDragonObject);
	for (uint i = 0; i < dragon->_anims.size(); ++i) {
		_vm->_anims->stop(dragon->_anims[i]);
	}

	for (uint i = 0; i < _info._numObjects; ++i) {
		GameObject *obj = &_objects[i];
		
		if (i != 0 && (obj->_location == oldRoomNum)) {
			for (uint j = 0; j < obj->_anims.size(); ++j) {
					_vm->_anims->deleteAnimation(obj->_anims[j]);
			}
			obj->_anims.clear();
		}
	}

	_currentRoom._roomNum = roomNum;
	loadRoom(roomNum);
	loadOverlays();
}

void Game::runGateProgram(int gate) {

	debugC(6, kDraciLogicDebugLevel, "Running program for gate %d", gate);

	// Set the appropriate loop statu before executing the gate program
	setLoopStatus(kStatusGate);

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
	anim->setZ(_hero.y + 1);

	// Fetch current frame
	Drawable *frame = anim->getFrame();

	// Fetch base dimensions of the frame
	uint height = frame->getHeight();
	uint width = frame->getWidth();

	// We naturally want the dragon to position its feet to the location of the
	// click but sprites are drawn from their top-left corner so we subtract
	// the current height of the dragon's sprite
	Common::Point p = _hero;
	p.x -= (int)(scale * width) / 2;
	p.y -= (int)(scale * height);

	// Since _persons[] is used for placing talking text, we use the non-adjusted x value
	// so the text remains centered over the dragon.
  	_persons[kDragonObject]._x = _hero.x;
	_persons[kDragonObject]._y = p.y;

	// Set the per-animation scaling factor
	anim->setScaleFactors(scale, scale);

	anim->setRelative(p.x, p.y);
}

int Game::getHeroX() const {
	return _hero.x;
}

int Game::getHeroY() const {
	return _hero.y;
}

double Game::getPers0() const {
	return _currentRoom._pers0;
}

double Game::getPersStep() const {
	return _currentRoom._persStep;
}


int Game::getRoomNum() const {
	return _currentRoom._roomNum;
}

void Game::setRoomNum(int room) {
	_newRoom = room;
}

int Game::getGateNum() const {
	return _currentGate;
}

void Game::setGateNum(int gate) {
	_newGate = gate;
}

void Game::setLoopStatus(LoopStatus status) {
	_loopStatus = status;
}

void Game::setLoopSubstatus(LoopSubstatus status) {
	_loopSubstatus = status;
}

LoopStatus Game::getLoopStatus() const {
	return _loopStatus;
}

LoopSubstatus Game::getLoopSubstatus() const {
	return _loopSubstatus;
}

int Game::getVariable(int numVar) const {
	return _variables[numVar];
}

void Game::setVariable(int numVar, int value) {
	_variables[numVar] = value;
}

int Game::getItemStatus(int itemID) const {
	return _itemStatus[itemID];
}

void Game::setItemStatus(int itemID, int status) {
	_itemStatus[itemID] = status;	
}

int Game::getCurrentItem() const {
	return _currentItem;
}

void Game::setCurrentItem(int itemID) {
	_currentItem = itemID;
}

const Person *Game::getPerson(int personID) const {
	return &_persons[personID];
}

void Game::setSpeechTick(uint tick) {
	_speechTick = tick;
}

int Game::getEscRoom() const {
	return _currentRoom._escRoom;
}

void Game::schedulePalette(int paletteID) {
	_scheduledPalette = paletteID;
}

int Game::getScheduledPalette() const {
	return _scheduledPalette;
}

/**
 * The GPL command Mark sets the animation index (which specifies the order in which 
 * animations were loaded in) which is then used by the Release command to delete
 * all animations that have an index greater than the one marked.
 */

int Game::getMarkedAnimationIndex() const {
	return _markedAnimationIndex;
}

void Game::deleteAnimationsAfterIndex(int lastAnimIndex) {
	// Delete all animations loaded after the marked one 
	// (from objects and from the AnimationManager)
	for (uint i = 0; i < getNumObjects(); ++i) {
		GameObject *obj = &_objects[i];

		for (uint j = 0; j < obj->_anims.size(); ++j) {
			Animation *anim;

			anim = _vm->_anims->getAnimation(obj->_anims[j]);
			if (anim != NULL && anim->getIndex() > lastAnimIndex)
				obj->_anims.remove_at(j);
		}
	}

	_vm->_anims->deleteAfterIndex(lastAnimIndex);
}

/**
 * See Game::getMarkedAnimationIndex().
 */

void Game::setMarkedAnimationIndex(int index) {
	_markedAnimationIndex = index;
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


bool WalkingMap::isWalkable(int x, int y) const {

	// Convert to map pixels
	x = x / _deltaX;
	y = y / _deltaY;

	int pixelIndex = _mapWidth * y + x;
	int byteIndex = pixelIndex / 8;
	int mapByte = _data[byteIndex];

	return mapByte & (1 << pixelIndex % 8);
}

/** 
 * @brief For a given point, find a nearest walkable point on the walking map
 *
 * @param startX 	x coordinate of the point
 * @param startY	y coordinate of the point
 *
 * @return A Common::Point representing the nearest walkable point
 *
 *	The algorithm was copied from the original engine for exactness.
 *	TODO: Study this algorithm in more detail so it can be documented properly and
 * 	possibly improved / simplified.
 */
Common::Point WalkingMap::findNearestWalkable(int startX, int startY, Common::Rect searchRect) const {

	// If the starting point is walkable, just return that
	if (searchRect.contains(startX, startY) && isWalkable(startX, startY)) {
		return Common::Point(startX, startY);
	}
	

	int signs[] = { 1, -1 };
	const uint kSignsNum = 2;

	int radius = 0;
	int x, y;
	int dx, dy;
	int prediction;

	// The place where, eventually, the result coordinates will be stored
	int finalX, finalY; 	

	// The algorithm appears to start off with an ellipse with the minor radius equal to
	// zero and the major radius equal to the walking map delta (the number of pixels
	// one map pixel represents). It then uses a heuristic to gradually reshape it into
	// a circle (by shortening the major radius and lengthening the minor one). At each
	// such resizing step, it checks some select points on the ellipse for walkability.
	// It also does the same check for the ellipse perpendicular to it (rotated by 90 degrees).

	while(1) {

		// The default major radius
		radius += _deltaX;

		// The ellipse radii (minor, major) that get resized
		x = 0;
		y = radius;

		// Heuristic variables
		prediction = 1 - radius;
		dx = 3;
		dy = 2 * radius - 2;
		
		do {
			
			// The following two loops serve the purpose of checking the points on the two
			// ellipses for walkability. The signs[] array is there to obliterate the need
			// of writing out all combinations manually.
	
			for (uint i = 0; i < kSignsNum; ++i) {
				finalY = startY + y * signs[i];
	
				for (uint j = 0; j < kSignsNum; ++j) {
					finalX = startX + x * signs[j];

					// If the current point is walkable, return it
					if (searchRect.contains(finalX, finalY) && isWalkable(finalX, finalY)) {
						return Common::Point(finalX, finalY);
					}
				}
			}

			if (x == y) {
				// If the starting point is walkable, just return that
				if (searchRect.contains(finalX, finalY) && isWalkable(finalX, finalY)) {
					return Common::Point(finalX, finalY);
				}
			}

			for (uint i = 0; i < kSignsNum; ++i) {
				finalY = startY + x * signs[i];

				for (uint j = 0; j < kSignsNum; ++j) {
					finalX = startX + y * signs[j];

					// If the current point is walkable, return it
					if (searchRect.contains(finalX, finalY) && isWalkable(finalX, finalY)) {
						return Common::Point(finalX, finalY);
					}
				}
			}

			// If prediction is non-negative, we need to decrease the major radius of the
			// ellipse
			if (prediction >= 0) {
				prediction -= dy;
				dy -= 2 * _deltaX;
				y -= _deltaX;
			}

			// Increase the minor radius of the ellipse and update heuristic variables
			prediction += dx;
			dx += 2 * _deltaX;
			x += _deltaX;

		// If the current ellipse has been reshaped into a circle, 
		// end this loop and enlarge the radius
		} while (x <= y); 
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
