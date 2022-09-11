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
#include "gui/message.h"
#include "common/config-manager.h"
#include "common/keyboard.h"
#include "common/language.h"
#include "common/translation.h"
#include "engines/util.h"
#include "graphics/thumbnail.h"
#include "common/error.h"
#include "dragons/actor.h"
#include "dragons/actorresource.h"
#include "dragons/background.h"
#include "dragons/bigfile.h"
#include "dragons/cursor.h"
#include "dragons/credits.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonimg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/dragonrms.h"
#include "dragons/dragonvar.h"
#include "dragons/dragons.h"
#include "dragons/font.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"
#include "dragons/screen.h"
#include "dragons/sequenceopcodes.h"
#include "dragons/scriptopcodes.h"
#include "dragons/bag.h"
#include "dragons/talk.h"
#include "dragons/sound.h"
#include "dragons/strplayer.h"

namespace Dragons {

#define DRAGONS_TICK_INTERVAL 17

static DragonsEngine *_engine = nullptr;

DragonsEngine *getEngine() {
	return _engine;
}

DragonsEngine::DragonsEngine(OSystem *syst, const ADGameDescription *desc) : Engine(syst) {
	_language = desc->language;
	_bigfileArchive = nullptr;
	_dragonRMS = nullptr;
	_backgroundResourceLoader = nullptr;
	_screen = nullptr;
	_sequenceOpcodes = new SequenceOpcodes(this);
	_scriptOpcodes = nullptr;
	_engine = this;
	_inventory = new Inventory(this);
	_cursor = new Cursor(this);
	_credits = nullptr;
	_talk = nullptr;
	_fontManager = nullptr;
	_strPlayer = nullptr;
	_sceneUpdateFunction = nullptr;
	_vsyncUpdateFunction = nullptr;

	_dragonOBD = nullptr;
	_dragonImg = nullptr;
	_actorManager = nullptr;
	_dragonINIResource = nullptr;
	_scene = nullptr;
	_sound = nullptr;
	_isLoadingDialogAudio = false;
	_sceneId1 = 0;
	_dragonFLG = nullptr;
	_dragonVAR = nullptr;
	_flickerIdleCounter = 0;
	_loadingScreenState = nullptr;

	_leftMouseButtonUp = false;
	_leftMouseButtonDown = false;
	_rightMouseButtonUp = false;
	_iKeyUp = false;
	_downKeyDown = false;
	_downKeyUp = false;
	_upKeyDown = false;
	_upKeyUp = false;
	_enterKeyUp = false;
	_leftKeyDown = false;
	_leftKeyUp = false;
	_rightKeyDown = false;
	_rightKeyUp = false;
	_wKeyDown = false;
	_aKeyDown = false;
	_sKeyDown = false;
	_dKeyDown = false;
	_oKeyDown = false;
	_pKeyDown = false;
	_mouseWheel = MOUSE_WHEEL_NO_EVENT;

	_debugMode = false;
	_isGamePaused = false;
	_inMenu = false;

	_bit_flags_8006fbd8 = 0;

	reset();
}

DragonsEngine::~DragonsEngine() {
	delete _sequenceOpcodes;
	delete _scriptOpcodes;
	delete _cursor;
}

void DragonsEngine::updateEvents() {
	Common::Event event;
	_leftMouseButtonUp = false;
	_rightMouseButtonUp = false;
	_iKeyUp = false;
	_downKeyUp = false;
	_upKeyUp = false;
	_enterKeyUp = false;
	_leftKeyUp = false;
	_rightKeyUp = false;
	_mouseWheel = MOUSE_WHEEL_NO_EVENT;
	while (_eventMan->pollEvent(event)) {
//		_input->processEvent(event);
		switch (event.type) {
		case Common::EVENT_QUIT:
			quitGame();
			break;
		case Common::EVENT_MOUSEMOVE:
			_cursor->updatePosition(event.mouse.x, event.mouse.y);
			break;
		case Common::EVENT_WHEELDOWN:
			_mouseWheel = MOUSE_WHEEL_DOWN;
			break;
		case Common::EVENT_WHEELUP:
			_mouseWheel = MOUSE_WHEEL_UP;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (event.customType == Dragons::kDragonsActionLeft) {
				_leftKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionRight) {
				_rightKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionUp) {
				_upKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionDown) {
				_downKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionSquare) {
				_aKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionTriangle) {
				_wKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionCircle) {
				_dKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionCross) {
				_sKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionL1) {
				_oKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionR1) {
				_pKeyDown = true;
			} else if (event.customType == Dragons::kDragonsActionSelect) {
				_leftMouseButtonDown = true;
			} else if (event.customType == Dragons::kDragonsActionDebugGfx) {
				_debugMode = !_debugMode;
			}
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			if (event.customType == Dragons::kDragonsActionLeft) {
				_leftKeyUp = true;
				_leftKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionRight) {
				_rightKeyUp = true;
				_rightKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionUp) {
				_upKeyUp = true;
				_upKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionDown) {
				_downKeyUp = true;
				_downKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionSquare) {
				_aKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionTriangle) {
				_wKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionCircle) {
				_dKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionCross) {
				_sKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionL1) {
				_oKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionR1) {
				_pKeyDown = false;
			} else if (event.customType == Dragons::kDragonsActionSelect) {
				_leftMouseButtonUp = true;
				_leftMouseButtonDown = false;
			} else if (event.customType == Dragons::kDragonsActionChangeCommand) {
				_rightMouseButtonUp = true;
			} else if (event.customType == Dragons::kDragonsActionInventory) {
				_iKeyUp = true;
			} else if (event.customType == Dragons::kDragonsActionEnter) {
				_enterKeyUp = true;
			} else if (event.customType == Dragons::kDragonsActionQuit) {
				quitGame();
			}
			break;
		default:
			break;
		}
	}
}

Common::Error DragonsEngine::run() {
	if(!checkAudioVideoFiles()) {
		return Common::kNoGameDataFoundError;
	}

	_screen = new Screen();
	_bigfileArchive = new BigfileArchive(this, "bigfile.dat");
	_talk = new Talk(this, _bigfileArchive);
	_dragonFLG = new DragonFLG(_bigfileArchive);
	_dragonImg = new DragonImg(_bigfileArchive);
	_dragonOBD = new DragonOBD(_bigfileArchive);
	_dragonRMS = new DragonRMS(_bigfileArchive, _dragonOBD);
	_dragonVAR = new DragonVAR(_bigfileArchive);
	_dragonINIResource = new DragonINIResource(_bigfileArchive);
	_fontManager = new FontManager(this, _screen, _bigfileArchive);
	_credits = new Credits(this, _fontManager, _bigfileArchive);
	ActorResourceLoader *actorResourceLoader = new ActorResourceLoader(_bigfileArchive);
	_actorManager = new ActorManager(actorResourceLoader);
	_scriptOpcodes = new ScriptOpcodes(this, _dragonFLG);
	_backgroundResourceLoader = new BackgroundResourceLoader(_bigfileArchive, _dragonRMS);
	_scene = new Scene(this, _screen, _scriptOpcodes, _actorManager, _dragonRMS, _dragonINIResource, _backgroundResourceLoader);

	_sound = new SoundManager(this, _bigfileArchive, _dragonRMS);
	_strPlayer = new StrPlayer(this, _screen);

	syncSoundSettings();

	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	} else {
		_strPlayer->playVideo("crystald.str");
		_strPlayer->playVideo("illusion.str");
		_strPlayer->playVideo("labintro.str");

		init();
		mainMenu();
		loadScene(0);
	}

	if (!shouldQuit()) {
		_scene->draw();
		_screen->updateScreen();

		gameLoop();
	}

	delete _scene;
	delete _actorManager;
	delete _backgroundResourceLoader;
	delete _dragonFLG;
	delete _dragonImg;
	delete _dragonRMS;
	delete _dragonVAR;
	delete _fontManager;
	delete _bigfileArchive;
	delete _screen;
	delete _sound;
	delete _strPlayer;

	debug("Ok");
	return Common::kNoError;
}

uint16 DragonsEngine::ipt_img_file_related() {
	DragonINI *flicker = _dragonINIResource->getFlickerRecord();
	assert(flicker);

	int16 tileX = flicker->actor->_x_pos / 32;
	int16 tileY = flicker->actor->_y_pos / 8;

	for (int i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if ((ini->sceneId == getCurrentSceneId()) && (ini->flags == 0)) {
			Img *img = _dragonImg->getImg(ini->imgId);
			if ((img->x <= tileX) && (((tileX <= img->x + img->w && (img->y <= tileY)) && (tileY <= img->y + img->h)))) {
				return i + 1;
			}
		}
	}
	return 0;
}

void DragonsEngine::gameLoop() {
	uint16 prevImgIniId = 0;
	uint16 sequenceId;

	_cursor->_cursorActivationSeqOffset = 0;
	_bit_flags_8006fbd8 = 0;
	_flickerIdleCounter = 0;
	setFlags(ENGINE_FLAG_8);

	while (!shouldQuit()) {
		_scene->draw();
		_screen->updateScreen();
		wait();
		updateHandler();
		updateEvents();
		runSceneUpdaterFunction(); //TODO is this the right place for this logic?

		if (getCurrentSceneId() != 2) {
			_sceneId1 = getCurrentSceneId();
		}

		updateFlickerIdleAnimation();

		if (_bit_flags_8006fbd8 == 0) {
			setFlags(ENGINE_FLAG_8);
		}
		if (_dragonINIResource->getFlickerRecord()->sceneId == getCurrentSceneId()) {
			uint16 imgIniId = ipt_img_file_related();
			if (imgIniId != 0 && imgIniId != (prevImgIniId & 0xffff)) {
				byte *obd = _dragonOBD->getFromOpt(imgIniId - 1);
				ScriptOpCall scriptOpCall(obd + 8, READ_LE_UINT32(obd));

				if (_scriptOpcodes->runScript4(scriptOpCall)) {
					scriptOpCall._codeEnd = scriptOpCall._code + 4 + READ_LE_UINT16(scriptOpCall._code + 2);
					scriptOpCall._code += 4;
					_scriptOpcodes->runScript(scriptOpCall);
					_flickerIdleCounter = 0;
				}
			}
			prevImgIniId = imgIniId;
		} else {
			prevImgIniId = 0;
		}

		if (_cursor->updateINIUnderCursor() == 0 ||
			(!(_cursor->_iniUnderCursor & 0x8000) && (getINI(_cursor->_iniUnderCursor - 1)->flags & 0x4000) != 0)) { //TODO check this. This logic looks a bit strange.
			_cursor->_cursorActivationSeqOffset = 0;
		} else {
			_cursor->_cursorActivationSeqOffset = 5;
		}

		if (_rightMouseButtonUp && isInputEnabled()) {
			_cursor->selectPreviousCursor();
			_flickerIdleCounter = 0;
			continue;
		}
		// TODO implement cycle cursor up.
//		actorId = CheckButtonMapPress_CycleUp(0);
//		if (((actorId & 0xffff) != 0) && isInputEnabled()) {
//			_cursor->_sequenceID = _cursor->_sequenceID + 1;
//			if (_cursor->_iniItemInHand == 0) {
//				bVar1 = _cursor->_sequenceID < 5;
//			}
//			else {
//				bVar1 = _cursor->_sequenceID < 6;
//			}
//			if (!bVar1) {
//				_cursor->_sequenceID = 0;
//			}
//			if ((_cursor->_sequenceID == 0) && ((_inventory->getType() == 1 || (_inventory->getType() == 2)))) {
//				_cursor->_sequenceID = 1;
//			}
//			if (_cursor->_sequenceID == 2) {
//				if (_inventory->getType() == 1) {
//					_cursor->_sequenceID = 4;
//				}
//				uVar6 = 3;
//				if (_cursor->_sequenceID == 2) goto LAB_80026fb0;
//			}
//			_flickerIdleCounter = 0;
//			actorId = uVar3;
//			continue;
//		}

		if (_bit_flags_8006fbd8 == 3) {
			_bit_flags_8006fbd8 = 0;
			DragonINI *flicker = _dragonINIResource->getFlickerRecord();
			if (flicker->sceneId == getCurrentSceneId() && flicker->actor->_direction != -1) {
				int16 iniId = _scriptOpcodes->_scriptTargetINI;
				if (_cursor->_sequenceID != 5) {
					iniId = _cursor->_performActionTargetINI;
				}
				if (iniId > 0) {
					flicker->actor->_direction = getINI(iniId - 1)->direction;
				}
			}

			performAction();
			if ((getCurrentSceneId() == 0x1d) && (getINI(0x178)->objectState != 0)) { //cave of dilemma
				clearFlags(ENGINE_FLAG_8);
			} else {
				setFlags(ENGINE_FLAG_8);
			}
			_flickerIdleCounter = 0;
			continue;
		}
		if (_inventory->getState() != InventoryOpen) {
			if (_inventory->getState() == Closed) {
				if ((checkForInventoryButtonRelease() && isInputEnabled()) &&
					((_bit_flags_8006fbd8 & 3) != 1)) {
					sequenceId = _dragonVAR->getVar(7);
					InventoryState uVar7 = _inventory->_previousState;
					_inventory->_previousState = _inventory->getState();
					_inventory->setState(_inventory->_previousState);
					if (sequenceId == 1) {
						_inventory->_previousState = uVar7;
						_inventory->inventoryMissing();
					} else {
						_flickerIdleCounter = 0;
						_inventory->setState(InventoryOpen);
						_inventory->openInventory();
						if (_cursor->_iniItemInHand == 0) {
							_cursor->_sequenceID = 1;
						} else {
							_cursor->_sequenceID = 5;
						}
					}
					continue;
				}
				InventoryState uVar6 = _inventory->getState();
				if (checkForActionButtonRelease() && isFlagSet(ENGINE_FLAG_8)) {
					_flickerIdleCounter = 0;
					if ((_cursor->_iniUnderCursor & 0x8000) != 0) {
						if (_cursor->_iniUnderCursor == 0x8002) {
							LAB_80027294:
							if (_cursor->_iniItemInHand == 0) {
								if ((_bit_flags_8006fbd8 & 3) != 1) {
									sequenceId = _dragonVAR->getVar(7);
									InventoryState uVar7 = _inventory->_previousState;
									_inventory->_previousState = _inventory->getState();
									_inventory->setState(_inventory->_previousState);
									if (sequenceId == 1) {
										_inventory->_previousState = uVar7;
										_inventory->inventoryMissing();
									} else {
										_flickerIdleCounter = 0;
										_inventory->setState(InventoryOpen);
										_inventory->openInventory();
										if (_cursor->_iniItemInHand == 0) {
											_cursor->_sequenceID = 1;
										} else {
											_cursor->_sequenceID = 5;
										}
									}
									continue;
								}
							} else {
								if (_inventory->addItem(_cursor->_iniItemInHand)) {
									_cursor->_sequenceID = 1;
									waitForFrames(1);
									_cursor->_iniItemInHand = 0;
									_cursor->_iniUnderCursor = 0;
									continue;
								}
							}
						} else {
							if (_cursor->_iniUnderCursor != 0x8001) {
								_flickerIdleCounter = 0;
								_cursor->_performActionTargetINI = _cursor->_iniUnderCursor;
								_cursor->_data_800728b0_cursor_seqID = _cursor->_sequenceID;
								walkFlickerToObject();
								if (_bit_flags_8006fbd8 != 0) {
									clearFlags(ENGINE_FLAG_8);
								}
								if (_cursor->_sequenceID >= 5) {
									_scriptOpcodes->_scriptTargetINI = _cursor->_performActionTargetINI;
									_cursor->_performActionTargetINI = _cursor->_iniItemInHand;
								}
								runINIScripts();
								continue;
							}
							if (_inventory->getSequenceId() == 0) {
								goto LAB_80027294;
							}
						}
						if ((_cursor->_iniUnderCursor == 0x8001) && (_inventory->getSequenceId() == 1)) {
							_inventory->setState(InventionBookOpen);
							_inventory->_previousState = uVar6;
							_inventory->openInventionBook();
							continue;
						}
					}
					_flickerIdleCounter = 0;
					_cursor->_performActionTargetINI = _cursor->_iniUnderCursor;
					_cursor->_data_800728b0_cursor_seqID = _cursor->_sequenceID;
					walkFlickerToObject();
					if (_bit_flags_8006fbd8 != 0) {
						clearFlags(ENGINE_FLAG_8);
					}
					if (_cursor->_sequenceID >= 5) {
						_scriptOpcodes->_scriptTargetINI = _cursor->_performActionTargetINI;
						_cursor->_performActionTargetINI = _cursor->_iniItemInHand;
					}
				}
			}
			if (_inventory->getState() == InventionBookOpen) {
				InventoryState uVar6 = _inventory->getState();
				if (checkForInventoryButtonRelease() && isInputEnabled()) {
					InventoryState uVar7 = _inventory->_previousState;
					if (_dragonVAR->getVar(7) == 1) {
						_inventory->_previousState = uVar7;
						_inventory->inventoryMissing();
						continue;
					}
					_flickerIdleCounter = 0;
					_inventory->setState(InventoryOpen);
					_inventory->_previousState = uVar6;
					_inventory->openInventory();
					if (_cursor->_iniItemInHand == 0) {
						_cursor->_sequenceID = 1;
					} else {
						_cursor->_sequenceID = 5;
					}
					continue;
				}
				if (checkForActionButtonRelease() && isFlagSet(ENGINE_FLAG_8)) {
					_flickerIdleCounter = 0;
					_cursor->_performActionTargetINI = _cursor->_iniUnderCursor;
					_cursor->_data_800728b0_cursor_seqID = _cursor->_sequenceID;
					walkFlickerToObject();
					if (_bit_flags_8006fbd8 != 0) {
						clearFlags(ENGINE_FLAG_8);
					}
					if (_cursor->_sequenceID >= 5) {
						_scriptOpcodes->_scriptTargetINI = _cursor->_performActionTargetINI;
						_cursor->_performActionTargetINI = _cursor->_iniItemInHand;
					}
				}
			}
			runINIScripts();
			continue;
		}
		if (checkForInventoryButtonRelease()) {
			_flickerIdleCounter = 0;
			_inventory->closeInventory();
			_inventory->setPreviousState();
			continue;
		}
		InventoryState uVar6 = _inventory->getState();
		if (checkForActionButtonRelease() && isFlagSet(ENGINE_FLAG_8)) {
			_flickerIdleCounter = 0;
			if ((_cursor->_iniUnderCursor & 0x8000) != 0) {
				if (_cursor->_iniUnderCursor == 0x8001) {
					_inventory->closeInventory();
					_inventory->setState(Closed);
					if (_inventory->_previousState == InventionBookOpen) {
						_inventory->closeInventionBook();
					}
				} else {
					if (_cursor->_iniUnderCursor != 0x8002) {
						if ((_cursor->_iniItemInHand == 0) ||
							(((uint16)(_cursor->_x - 10U) < 300 && ((uint16)(_cursor->_y - 10U) < 0xb4)))) {
							runINIScripts();
						} else {
							_cursor->_sequenceID = 5;
							waitForFrames(2);
							_inventory->closeInventory();
							_inventory->setPreviousState();
						}
						continue;
					}
					_inventory->closeInventory();
					_inventory->setState(InventionBookOpen);
					if (_inventory->_previousState != InventionBookOpen) {
						_inventory->openInventionBook();
					}
				}
				_inventory->_previousState = uVar6;
				continue;
			}
			if (_cursor->_iniUnderCursor != 0) {
				if ((_cursor->_sequenceID != 4) && (_cursor->_sequenceID != 2)) {
					_cursor->_data_800728b0_cursor_seqID = _cursor->_sequenceID;
					_cursor->_performActionTargetINI = _cursor->_iniUnderCursor;
					if (_cursor->_sequenceID >= 4) {
						_cursor->_performActionTargetINI = _cursor->_iniItemInHand;
						_scriptOpcodes->_scriptTargetINI = _cursor->_iniUnderCursor;
					}
					clearFlags(ENGINE_FLAG_8);
					walkFlickerToObject();
					if ((_cursor->_iniItemInHand == 0) ||
						(((uint16)(_cursor->_x - 10U) < 300 && ((uint16)(_cursor->_y - 10U) < 0xb4)))) {
						runINIScripts();
					} else {
						_cursor->_sequenceID = 5;
						waitForFrames(2);
						_inventory->closeInventory();
						_inventory->setPreviousState();
					}
					continue;
				}
				Actor *actor = _inventory->getInventoryItemActor(_cursor->_iniUnderCursor);
				uint16 tmpId = _cursor->_iniItemInHand;
				_inventory->replaceItem(_cursor->_iniUnderCursor, _cursor->_iniItemInHand);
				_cursor->_objectInHandSequenceID = actor->_sequenceID;
				actor->clearFlag(ACTOR_FLAG_40);
				_cursor->_iniItemInHand = _cursor->_iniUnderCursor;
				_cursor->_sequenceID = 5;
				if (tmpId != 0) {
					actor->_flags = 0;
					actor->_priorityLayer = 0;
					actor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
					actor->updateSequence(getINI(tmpId - 1)->inventorySequenceId * 2 + 10);
					actor->setFlag(ACTOR_FLAG_40);
					actor->setFlag(ACTOR_FLAG_80);
					actor->setFlag(ACTOR_FLAG_100);
					actor->setFlag(ACTOR_FLAG_200);
					actor->_priorityLayer = 6;
				}
				continue;
			}
			if (_cursor->_iniItemInHand == 0) {
				runINIScripts();
				continue;
			}
			//drop item back into inventory
			if (_inventory->addItemIfPositionIsEmpty(_cursor->_iniItemInHand, _cursor->_x, _cursor->_y)) {
				Actor *invActor = _inventory->getInventoryItemActor(_cursor->_iniItemInHand);
				invActor->_flags = 0;
				invActor->_priorityLayer = 0;
				invActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
				invActor->updateSequence(getINI(_cursor->_iniItemInHand - 1)->inventorySequenceId * 2 + 10);
				_cursor->_iniItemInHand = 0;
				invActor->setFlag(ACTOR_FLAG_40);
				invActor->setFlag(ACTOR_FLAG_80);
				invActor->setFlag(ACTOR_FLAG_100);
				invActor->setFlag(ACTOR_FLAG_200);
				invActor->_priorityLayer = 6;
				if (_cursor->_sequenceID == 5) {
					_cursor->_sequenceID = 4;
				}
			}
		}
		if ((_cursor->_iniItemInHand == 0) ||
			(((uint16)(_cursor->_x - 10U) < 300 && ((uint16)(_cursor->_y - 10U) < 0xb4)))) {
			runINIScripts();
		} else {
			_cursor->_sequenceID = 5;
			waitForFrames(2);
			_inventory->closeInventory();
			_inventory->setPreviousState();
		}
	}
}


void DragonsEngine::updateHandler() {
	_videoFlags |= 0x40;
	//TODO logic here

	updateActorSequences();

	updateCamera();

	_cursor->updateVisibility();
	_inventory->updateVisibility();

	_actorManager->updateActorDisplayOrder();

	// 0x8001b200
	if (isFlagSet(ENGINE_FLAG_8000) && !_sound->isSpeechPlaying()) {
		//dialog finished playing.
		clearFlags(ENGINE_FLAG_8000);
		_sound->resumeMusic();
	}

	//TODO logic here
	for (uint16 i = 0; i < 0x17; i++) {
		Actor *actor = _actorManager->getActor(i);
		if (actor->_flags & ACTOR_FLAG_40) {
			if (!(actor->_flags & ACTOR_FLAG_100)) {
				int16 priority = _scene->getPriorityAtPosition(Common::Point(actor->_x_pos, actor->_y_pos));
				DragonINI *flicker = _dragonINIResource->getFlickerRecord();
				if (flicker && _scene->contains(flicker) && flicker->actor->_actorID == i) {
					if ((priority >= 0 && priority < 8) || priority == 0x10) {
						actor->_priorityLayer = priority;
					}
				} else {
					if (priority != -1) {
						actor->_priorityLayer = priority;
					}
				}

				if (actor->_priorityLayer >= 0x11) {
					actor->_priorityLayer = 0;
				}

				if (actor->_priorityLayer >= 9) {
					actor->_priorityLayer -= 8;
				}
			}

			if (actor->_sequenceTimer != 0) {
				actor->_sequenceTimer--;
			}
		}
	}

	if (_flags & ENGINE_FLAG_80) {
		for (uint16 i = 0x17; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
			Actor *actor = _actorManager->getActor(i);
			if (actor->_sequenceTimer != 0) {
				actor->_sequenceTimer--;
			}
		}
	}

	if (isFlagSet(ENGINE_FLAG_4)) {
		updatePathfindingActors();
	}

	// TODO 0x8001bed0
	updatePaletteCycling();

	// 0x8001c294
	if (!(_unkFlags1 & ENGINE_UNK1_FLAG_8)) {
		//TODO ReadPad();
	}

	if (isFlagSet(ENGINE_FLAG_20)) {
		engineFlag0x20UpdateFunction();
	}

	runVsyncUpdaterFunction();

	// TODO data_8006a3a0 logic. @ 0x8001c2f4

	_videoFlags &= ~0x40;
}

uint32 DragonsEngine::calulateTimeLeft() {
	uint32 now;

	now = _system->getMillis();

	if (_nextUpdatetime <= now) {
		_nextUpdatetime = now + DRAGONS_TICK_INTERVAL;
		return (0);
	}
	uint32 delay = _nextUpdatetime - now;
	_nextUpdatetime += DRAGONS_TICK_INTERVAL;
	return (delay);
}

void DragonsEngine::wait() {
	_system->delayMillis(calulateTimeLeft());
}

void DragonsEngine::updateActorSequences() {
	if (!(_flags & ENGINE_FLAG_4)) {
		return;
	}

	//TODO ResetRCnt(0xf2000001);

	int16 actorId = _flags & ENGINE_FLAG_80 ? (int16) 64 : (int16) 23;

	while (actorId > 0) {
		actorId--;
		Actor *actor = _actorManager->getActor((uint16) actorId);
		if (actorId < 2 && _flags & ENGINE_FLAG_40) {
			continue;
		}

		if (actor->_flags & ACTOR_FLAG_40 &&
				!(actor->_flags & ACTOR_FLAG_4) &&
				!(actor->_flags & ACTOR_FLAG_400) &&
				(actor->_sequenceTimer == 0 || actor->_flags & ACTOR_FLAG_1)) {
			debug(5, "Actor[%d] execute sequenceOp", actorId);

			if (actor->_flags & ACTOR_FLAG_1) {
				actor->resetSequenceIP();
				//clear flag mask 0xeff6;
				actor->clearFlag(ACTOR_FLAG_1);
				actor->clearFlag(ACTOR_FLAG_8);
				actor->clearFlag(ACTOR_FLAG_1000);
				actor->_field_7a = 0;
			}
			OpCall opCall;
			opCall._result = 1;
			while (opCall._result == 1) {
				opCall._op = (byte) READ_LE_UINT16(actor->_seqCodeIp);
				opCall._code = actor->_seqCodeIp + 2;
				_sequenceOpcodes->execOpcode(actor, opCall);
				actor->_seqCodeIp += opCall._deltaOfs;
			}
		}
	}
}

void DragonsEngine::setFlags(uint32 flags) {
	_flags |= flags;
}

void DragonsEngine::clearFlags(uint32 flags) {
	_flags &= ~flags;
}

uint32 DragonsEngine::getMultipleFlags(uint32 flags) {
	return _flags & flags;
}

uint32 DragonsEngine::getAllFlags() {
	return _flags;
}

void DragonsEngine::setAllFlags(uint32 flags) {
	_flags = flags;
}

void DragonsEngine::setUnkFlags(uint32 flags) {
	_unkFlags1 |= flags;
}

void DragonsEngine::clearUnkFlags(uint32 flags) {
	_unkFlags1 &= ~flags;
}

byte *DragonsEngine::getBackgroundPalette() {
	assert(_scene);
	return _scene->getPalette();
}

bool DragonsEngine::isFlagSet(uint32 flag) {
	return (bool) (_flags & flag);
}

bool DragonsEngine::isUnkFlagSet(uint32 flag) {
	return (bool) (_unkFlags1 & flag);
}

DragonINI *DragonsEngine::getINI(uint32 index) {
	return _dragonINIResource->getRecord(index);
}

uint16 DragonsEngine::getVar(uint16 offset) {
	return _dragonVAR->getVar(offset);
}

uint16 DragonsEngine::getCurrentSceneId() const {
	return _scene->getSceneId();
}

void DragonsEngine::setVar(uint16 offset, uint16 value) {
	return _dragonVAR->setVar(offset, value);
}

uint16 DragonsEngine::getIniFromImg() {
	DragonINI *flicker = _dragonINIResource->getFlickerRecord();

	int16 x = flicker->actor->_x_pos / 32;
	int16 y = flicker->actor->_y_pos / 8;

	uint16 currentSceneId = _scene->getSceneId();

	for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if (ini->sceneId == currentSceneId && ini->flags == 0) {
			Img *img = _dragonImg->getImg(ini->imgId);
			if (x >= img->x &&
				img->x + img->w >= x &&
				y >= img->y &&
				img->h + img->y >= y) {
				return i + 1;
			}
		}

	}
	return 0;
}

void DragonsEngine::runINIScripts() {
	bool isFlag8Set = isFlagSet(ENGINE_FLAG_8);
	for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if (ini->flags & INI_FLAG_10) {
			ini->flags &= ~INI_FLAG_10;
			byte *data = _dragonOBD->getFromOpt(i);
			ScriptOpCall scriptOpCall(data + 8, READ_LE_UINT32(data));
			clearFlags(ENGINE_FLAG_8);
			_scriptOpcodes->runScript3(scriptOpCall);
		}
	}
	if (isFlag8Set) {
		setFlags(ENGINE_FLAG_8);
	}
}

void DragonsEngine::engineFlag0x20UpdateFunction() {
	if (!isFlagSet(ENGINE_FLAG_20)) {
		_run_func_ptr_unk_countdown_timer = 1;
		return;
	}

	if (isFlagSet(ENGINE_FLAG_8) && !isFlagSet(ENGINE_FLAG_8000000)) {
		_cursor->update();
	}

	uint16 currentSceneId = _scene->getSceneId();
	DragonINI *flickerINI = _dragonINIResource->getFlickerRecord();
	DragonINI *ini1 = getINI(1);

	if (flickerINI != nullptr) {
		if (flickerINI->sceneId == currentSceneId) {
			if (flickerINI->actor == nullptr || flickerINI->actor->isFlagSet(ACTOR_FLAG_10)) {
				if (ini1->actor != nullptr) {
					ini1->actor->updateSequence(8);
					ini1->actor->_priorityLayer = 0;
				}
			} else {
				if ((_bit_flags_8006fbd8 & 2u) == 0) {
					_bit_flags_8006fbd8 |= 2u;
				}
				if (flickerINI->actor->isFlagClear(ACTOR_FLAG_2000)
						&& flickerINI->actor->isFlagSet(ACTOR_FLAG_4)
						&& flickerINI->actor->_direction != -1
						&& flickerINI->actor->_direction != flickerINI->actor->_sequenceID) {
					flickerINI->actor->updateSequence(flickerINI->actor->_direction);
				}
				if (ini1->actor != nullptr) {
					ini1->actor->_priorityLayer = 0;
				}
			}
		}
	}

	// 0x80027db8
	if (!_inventory->isOpen()) {
		for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
			DragonINI *ini = getINI(i);
			if (ini->counter >= 0 && ini->sceneId == currentSceneId) {
				ini->counter--;
				if (ini->counter < 0) {
					ini->flags |= INI_FLAG_10;
				}
			}
		}
	}

	if (_run_func_ptr_unk_countdown_timer != 0) {
		_run_func_ptr_unk_countdown_timer--;
	}
}

void DragonsEngine::waitForFrames(uint16 numFrames) {
	for (uint16 i = 0; i < numFrames && !Engine::shouldQuit(); i++) {
		wait();
		updateHandler();

		_scene->draw();
		_screen->updateScreen();
		runSceneUpdaterFunction();
		updateEvents();
	}
}

void DragonsEngine::waitForFramesAllowSkip(uint16 numFrames) {
	for (int i = 0; i < numFrames; i++) {
		waitForFrames(1);
		if (checkForActionButtonRelease()) { // TODO should this be any input?
			return;
		}
	}
}

void DragonsEngine::playOrStopSound(uint16 soundId) {
	debug(1, "play sound 0x%x", soundId);

	this->_sound->playOrStopSound(soundId);
}

void DragonsEngine::updatePathfindingActors() {
	for (uint16 i = 0; i < 0x17; i++) {
		Actor *actor = _actorManager->getActor(i);
		actor->walkPath();
	}
}

void DragonsEngine::fadeFromBlack(uint32 flags) {
	if (!isFlagSet(ENGINE_FLAG_40)) {
		return;
	}
	bool isUnkFlag2Set = isUnkFlagSet(ENGINE_UNK1_FLAG_2);

	setUnkFlags(ENGINE_UNK1_FLAG_2);
	clearFlags(ENGINE_FLAG_40);

	//TODO 0x80015a1c implement fading logic here.

	if (!isUnkFlag2Set) {
		clearUnkFlags(ENGINE_UNK1_FLAG_2);
	}
}

void DragonsEngine::fadeFromBlack() {
	fadeFromBlack(0x1f);
}

void DragonsEngine::fadeFromBlackExcludingFont() {
	fadeFromBlack(0x1e);
}

void DragonsEngine::fadeToBlack() {
	fadeToBlack(0x1f);
}

void DragonsEngine::fadeToBlackExcludingFont() {
	fadeToBlack(0x1e);
}

void DragonsEngine::fadeToBlack(uint32 flags) {
	bool isFlag2Set = isUnkFlagSet(ENGINE_UNK1_FLAG_2);
	//TODO
//	do {
//		SetShadeTex(poly_ft4_tbl + uVar2,0);
//		uVar5 = uVar5 + 1;
//		uVar2 = (uint)uVar5;
//	} while (uVar5 < 0x40);
//	uVar5 = 0;
//	uVar2 = 0;
//	do {
//		SetShadeTex(poly_f4_tbl + uVar2,0);
//		uVar1 = unkFlags1;
//		uVar5 = uVar5 + 1;
//		uVar2 = (uint)uVar5;
//	} while (uVar5 < 0xf);
	if (!isFlagSet(ENGINE_FLAG_40)) {
		setUnkFlags(ENGINE_UNK1_FLAG_2);

		//TODO fade out here.

		setFlags(ENGINE_FLAG_40);
		if (!isFlag2Set) {
			clearUnkFlags(ENGINE_UNK1_FLAG_2);
		}
	}
}

void DragonsEngine::performAction() {
	uint uVar1;
	uint16 uVar2;
	uint uVar4;
	uint uVar5;
	uint uVar6;
	byte * pvVar7;
	byte * pvVar8;
	byte *local_58_code;
	byte *local_58_codeEnd;
	bool load_58_result = false;

	uVar2 = _scriptOpcodes->_scriptTargetINI;
	uVar1 = _flags;
	local_58_code = nullptr;
	local_58_codeEnd = nullptr;

	uVar6 = 0;
	_scriptOpcodes->_numDialogStackFramesToPop = 0;

	assert(_cursor->_performActionTargetINI > 0);
	byte *obd = _dragonOBD->getFromOpt(_cursor->_performActionTargetINI - 1);


	ScriptOpCall local_48(obd + 8, READ_LE_UINT32(obd));
	pvVar7 = local_48._code;
	pvVar8 = local_48._codeEnd;

	uVar4 = _cursor->executeScript(local_48, 1);
	if (_cursor->_data_800728b0_cursor_seqID > 4) {
		_scriptOpcodes->_numDialogStackFramesToPop = 0;

		obd = _dragonOBD->getFromOpt(_scriptOpcodes->_scriptTargetINI - 1);
		_scriptOpcodes->_scriptTargetINI = _cursor->_performActionTargetINI;

		ScriptOpCall local_38(obd + 8, READ_LE_UINT32(obd));

		uVar6 = _cursor->executeScript(local_38, 1);
		_scriptOpcodes->_scriptTargetINI = uVar2;

		if ((uVar6 & 0xffff) != 0) {
			local_58_code = local_38._code + 8;
			local_58_codeEnd = local_58_code + READ_LE_UINT16(local_38._code + 6);
		}
	}
	if (((uVar4 & 0xffff) != 0) && ((((uVar4 & 2) == 0 || ((uVar6 & 2) != 0)) || ((uVar6 & 0xffff) == 0)))) {
		local_58_code = local_48._code + 8;
		local_58_codeEnd = local_58_code + READ_LE_UINT16(local_48._code + 6);
	}
	uVar4 = uVar4 & 0xfffd;
	if (local_58_code != nullptr && local_58_codeEnd != nullptr) {
		clearFlags(ENGINE_FLAG_8);
		ScriptOpCall local_58(local_58_code, local_58_codeEnd - local_58_code);
		_scriptOpcodes->runScript(local_58);
		load_58_result = local_58._result;
	}
	if (!load_58_result) {
		if (_cursor->_data_800728b0_cursor_seqID == 3) {
			ScriptOpCall local_58(pvVar7, pvVar8 - pvVar7);
			uVar5 = _talk->talkToActor(local_58);
			uVar4 = uVar4 | uVar5;
		}
		if (((uVar4 & 0xffff) == 0) && ((uVar6 & 0xfffd) == 0)) {
			_talk->flickerRandomDefaultResponse();
		}
	} else {
		_scriptOpcodes->_numDialogStackFramesToPop--;
	}
	_flags |= uVar1 & ENGINE_FLAG_8;
	return;
}

bool DragonsEngine::checkForInventoryButtonRelease() {
	return _iKeyUp;
}

bool DragonsEngine::isInputEnabled() {
	return !isFlagSet(ENGINE_FLAG_20000000) && !isFlagSet(ENGINE_FLAG_400);
}

bool DragonsEngine::isActionButtonPressed() {
	return _leftMouseButtonDown;
}

bool DragonsEngine::isLeftKeyPressed() {
	return _leftKeyDown;
}

bool DragonsEngine::isRightKeyPressed() {
	return _rightKeyDown;
}

bool DragonsEngine::isUpKeyPressed() {
	return _upKeyDown;
}

bool DragonsEngine::isDownKeyPressed() {
	return _downKeyDown;
}

bool DragonsEngine::checkForActionButtonRelease() {
	return _leftMouseButtonUp || _enterKeyUp;
}

void DragonsEngine::walkFlickerToObject() {
	uint16 targetX;
	uint16 targetY;
	uint uVar7;
	uint uVar8;
	DragonINI *targetINI;
	DragonINI *flickerINI;

	flickerINI = _dragonINIResource->getFlickerRecord();
	if (flickerINI->sceneId == getCurrentSceneId()) {
		if (_cursor->_performActionTargetINI != 0) {

			if (!(READ_LE_UINT16(_dragonOBD->getFromOpt(_cursor->_performActionTargetINI - 1) + 4) & 8)
				&& (_inventory->getState() == Closed) && !isFlagSet(ENGINE_FLAG_200000)) {
				targetINI = getINI(_cursor->_performActionTargetINI - 1);
				if ((targetINI->flags & 1) == 0) {
					if (targetINI->actorResourceId == -1) {
						return;
					}
					Img *img = _dragonImg->getImg(targetINI->imgId);
					targetX = img->field_a;
					targetY = img->field_c;
				} else {
					targetX = targetINI->actor->_x_pos;
					targetY = targetINI->actor->_y_pos;
				}
				flickerINI->actor->_walkSpeed = 0x10000;
				if (flickerINI->direction2 == -1) {
					flickerINI->actor->setFlag(ACTOR_FLAG_800);
				}
				flickerINI->actor->startWalk((int)(((uint)targetX + (uint)targetINI->baseXOffset) * 0x10000) >> 0x10,
											 (int)(((uint)targetY + (uint)targetINI->baseYOffset) * 0x10000) >> 0x10, 0);
				_bit_flags_8006fbd8 = 1; //walk to perform an action.
				return;
			}
			if (isFlagSet(ENGINE_FLAG_200000)) {
				_bit_flags_8006fbd8 = 3;
				return;
			}
			flickerINI = _dragonINIResource->getFlickerRecord();
			if (flickerINI != nullptr && flickerINI->actor != nullptr) {
				flickerINI->actor->clearFlag(ACTOR_FLAG_10);
				flickerINI->actor->setFlag(ACTOR_FLAG_4);
				targetINI = getINI(_cursor->_performActionTargetINI - 1);
				flickerINI->direction2 = targetINI->direction;
				flickerINI->actor->_direction = targetINI->direction;
			}
			_bit_flags_8006fbd8 = 3;
			return;
		}
		if (_inventory->getState() == Closed && !isFlagSet(ENGINE_FLAG_200000)) {
			uVar7 = (uint)(uint16)_cursor->_x;
			uVar8 = (uint)(uint16)_cursor->_y;
			flickerINI->actor->_walkSpeed = 0x10000;
			flickerINI->actor->startWalk(
					(int)((uVar7 + (uint)_scene->_camera.x) * 0x10000) >> 0x10,
					(int)((uVar8 + (uint)_scene->_camera.y) * 0x10000) >> 0x10, 0);
		}
	} else {
		if (_cursor->_performActionTargetINI != 0) {
			_bit_flags_8006fbd8 = 3;
			return;
		}
	}
	_bit_flags_8006fbd8 = 0;
	return;
}

void DragonsEngine::reset_screen_maybe() {
	_videoFlags &= ~0x10;
	//TODO
}

bool DragonsEngine::canLoadGameStateCurrently() {
	//player has control and not currently talking to anyone.
	return isInputEnabled() && isFlagSet(ENGINE_FLAG_8) && !isFlagSet(Dragons::ENGINE_FLAG_100);
}

bool DragonsEngine::canSaveGameStateCurrently() {
	return isInputEnabled() && !_inventory->isOpen() && isFlagSet(ENGINE_FLAG_8) && !isFlagSet(Dragons::ENGINE_FLAG_100);
}

bool DragonsEngine::hasFeature(Engine::EngineFeature f) const {
	return
		// TODO (f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void DragonsEngine::init() {
	_flags = 0x1046;
	_flags &= 0x1c07040;
	_flags |= 0x26;
	_unkFlags1 = 0;

	initSubtitleFlag();

	_scriptOpcodes->_scriptTargetINI = 0; //TODO this should be reset in scriptopcode.
	_cursor->init(_actorManager, _dragonINIResource);
	_inventory->init(_actorManager, _backgroundResourceLoader, new Bag(_bigfileArchive, _screen), _dragonINIResource);
	_talk->init();

	_screen->loadPalette(1, _cursor->getPalette());
	setupPalette1();

	_screen->loadPalette(2, _cursor->getPalette());
	_screen->updatePaletteTransparency(2, 1, 5, true);
	_screen->updatePaletteTransparency(2, 0xc1, 0xc3, true);

	_screen->loadPalette(4, _cursor->getPalette());
	_screen->updatePaletteTransparency(4, 1, 0xff, true);
}

void DragonsEngine::loadScene(uint16 sceneId) {
	// TODO fun_80017010_update_actor_texture_maybe();
	if (sceneId > 2) {
		_dragonVAR->setVar(1, 1);
	}

	//if (sceneId > 2) { //TODO remove this restriction to enable intro sequence.
		_scene->setSceneId(2);
		byte *obd = _dragonOBD->getFromSpt(3);
		ScriptOpCall scriptOpCall(obd + 4, READ_LE_UINT32(obd));
		_scriptOpcodes->runScript(scriptOpCall);
	//} else {
//		sceneId = 0x12; // HACK the first scene. TODO remove this
//	}

	_inventory->loadInventoryItemsFromSave();

	if (getINI(0)->sceneId == 0) {
		getINI(0)->sceneId = sceneId; //TODO
	} else {
		_scene->setSceneId(getINI(0)->sceneId);
	}
	_sceneId1 = sceneId;
	_scene->loadScene(sceneId ? sceneId : 0x12, 0x1e);
}

void DragonsEngine::reset() {
	seedRandom(0x1dd); //TODO should we randomise this better? I got this value from a couple of runs in the emulator
	_nextUpdatetime = 0;
	_flags = 0;
	_unkFlags1 = 0;
	_run_func_ptr_unk_countdown_timer = 0;
	_videoFlags = 0;
	_flickerInitialSceneDirection = 0;

	for (int i = 0; i < 8; i++) {
		_paletteCyclingTbl[i].paletteType = 0;
		_paletteCyclingTbl[i].startOffset = 0;
		_paletteCyclingTbl[i].endOffset = 0;
		_paletteCyclingTbl[i].updateInterval = 0;
		_paletteCyclingTbl[i].updateCounter = 0;
	}

	clearSceneUpdateFunction();
}

void DragonsEngine::runSceneUpdaterFunction() {
	if ((isFlagSet(ENGINE_FLAG_20) && (_run_func_ptr_unk_countdown_timer == 0)) &&
			(_run_func_ptr_unk_countdown_timer = 1, _sceneUpdateFunction != nullptr)) {
		_sceneUpdateFunction();
	}
}

void DragonsEngine::setSceneUpdateFunction(void (*newUpdateFunction)()) {
	_sceneUpdateFunction = newUpdateFunction;
}

void DragonsEngine::clearSceneUpdateFunction() {
	setSceneUpdateFunction(nullptr);
}

void DragonsEngine::setVsyncUpdateFunction(void (*newUpdateFunction)()) {
	_vsyncUpdateFunction = newUpdateFunction;
}

void DragonsEngine::seedRandom(int32 seed) {
	_randomState = seed * -0x2b0e2b0f;
}

uint32 DragonsEngine::shuffleRandState() {
	uint32 returnBit;

	returnBit = _randomState & 1;
	_randomState = _randomState >> 1 |
			((_randomState << 0x1e ^ _randomState ^ _randomState << 0x1d ^ _randomState << 0x1b ^
						  _randomState << 0x19) & 0x80000000);
	return returnBit;
}

uint16 DragonsEngine::getRand(uint16 max) {
	uint16 rand = 0;

	for (int i = 0; i < 0x10; i++) {
		rand |= shuffleRandState() << i;
	}
	return rand % max;
}

bool DragonsEngine::checkForDownKeyRelease() {
	return _downKeyUp;
}

bool DragonsEngine::checkForUpKeyRelease() {
	return _upKeyUp;
}

bool DragonsEngine::isSquareButtonPressed() {
	return _aKeyDown;
}

bool DragonsEngine::isTriangleButtonPressed() {
	return _wKeyDown;
}

bool DragonsEngine::isCircleButtonPressed() {
	return _dKeyDown;
}

bool DragonsEngine::isCrossButtonPressed() {
	return _sKeyDown;
}

bool DragonsEngine::isL1ButtonPressed() {
	return _oKeyDown;
}

bool DragonsEngine::isR1ButtonPressed() {
	return _pKeyDown;
}

void DragonsEngine::setupPalette1() {
	byte palette[512];
	memcpy(palette, _cursor->getPalette(), 0x100);
	memcpy(palette + 0x100, _cursor->getPalette(), 0x100);
	_screen->loadPalette(1, palette);
	_screen->updatePaletteTransparency(1, 0x40, 0x7f, true);
}

bool DragonsEngine::isDebugMode() {
	return _debugMode;
}

bool DragonsEngine::isVsyncUpdaterFunctionRunning() {
	return _vsyncUpdateFunction != nullptr;
}

void DragonsEngine::runVsyncUpdaterFunction() {
	if (isVsyncUpdaterFunctionRunning()) {
		_vsyncUpdateFunction();
	}
}

void DragonsEngine::loadCurrentSceneMsf() {
	_sound->loadMsf(getCurrentSceneId());
}

void DragonsEngine::updatePaletteCycling() {
	if (!_isGamePaused) {
		for (int loopIndex = 0; loopIndex < 8 ; loopIndex++) {
			if (_paletteCyclingTbl[loopIndex].updateInterval != 0) {
				if (_paletteCyclingTbl[loopIndex].updateCounter == 0) {
					uint16 *palette = (uint16 *)_screen->getPalette(_paletteCyclingTbl[loopIndex].paletteType);
					int16 uVar14 = _paletteCyclingTbl[loopIndex].startOffset;
					int16 uVar8 = _paletteCyclingTbl[loopIndex].endOffset;
					if (uVar14 < uVar8) {
						uint16 uVar11 = palette[uVar8];
						int uVar15 = uVar8;
						if (uVar14 < uVar8) {
							do {
								uVar8--;
								palette[uVar15] = palette[uVar15 - 1];
								uVar15 = uVar8 & 0xffff;
							} while (_paletteCyclingTbl[loopIndex].startOffset < (uVar8 & 0xffff));
						}
						palette[(uint16)_paletteCyclingTbl[loopIndex].startOffset] = uVar11;
						_paletteCyclingTbl[loopIndex].updateCounter = _paletteCyclingTbl[loopIndex].updateInterval;
					} else {
						if (uVar8 < uVar14) {
							uint16 uVar11 = palette[uVar14];
							uint16 uVar15 = uVar8;
							if (uVar8 < uVar14) {
								do {
									uVar8--;
									palette[uVar15] = palette[uVar15 + 1];
									uVar15 = uVar8 & 0xffff;
								} while ((uVar8 & 0xffff) < _paletteCyclingTbl[loopIndex].startOffset);
							}
							palette[(uint16)_paletteCyclingTbl[loopIndex].endOffset] = uVar11;
							_paletteCyclingTbl[loopIndex].updateCounter =
									_paletteCyclingTbl[loopIndex].updateInterval;
						}
					}
				} else {
					_paletteCyclingTbl[loopIndex].updateCounter = _paletteCyclingTbl[loopIndex].updateCounter + -1;
				}
			}
		}
	}
}

uint32 DragonsEngine::getFontOffsetFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA :
	case Common::RU_RUS : return 0x4a144;
	case Common::EN_GRB : return 0x4b4fc;
	case Common::DE_DEU : return 0x4af5c;
	case Common::FR_FRA : return 0x4b158;
	default : error("Unable to get font offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint32 DragonsEngine::getSpeechTblOffsetFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA :
	case Common::RU_RUS : return 0x4e138;
	case Common::EN_GRB : return 0x4f4f4;
	case Common::DE_DEU : return 0x4f0a4;
	case Common::FR_FRA : return 0x4f2a0;
	default : error("Unable to get speech table offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint16 DragonsEngine::getBigFileTotalRecords() {
	if (_language == Common::EN_USA || _language == Common::EN_GRB || _language == Common::RU_RUS) {
		return 576;
	}
	return 588;
}

uint32 DragonsEngine::getBigFileInfoTblFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA :
	case Common::RU_RUS : return 0x4a238;
	case Common::EN_GRB : return 0x4b5f4;
	case Common::DE_DEU : return 0x4b054;
	case Common::FR_FRA : return 0x4b250;
	default :
		error("Unable to get bigfile info table offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint32 DragonsEngine::getCutscenePaletteOffsetFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA :
	case Common::RU_RUS : return 0x5336c;
	case Common::EN_GRB : return 0x54628;
	case Common::DE_DEU : return 0x541d8;
	case Common::FR_FRA : return 0x543d4;
	default :
		error("Unable to get cutscene palette table offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint32 DragonsEngine::defaultResponseOffsetFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA :
	case Common::RU_RUS : return 0x541b0;
	case Common::EN_GRB : return 0x55470;
	case Common::DE_DEU : return 0x55020;
	case Common::FR_FRA : return 0x5521c;
	default :
		error("Unable to get response offset table offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

void DragonsEngine::updateFlickerIdleAnimation() {
	_flickerIdleCounter++;
	if (_flickerIdleCounter >= 0x4af) {
		DragonINI *flicker = _dragonINIResource->getFlickerRecord();
		if (flicker->actor->_resourceID == 0xe) {
			flicker->actor->_direction = 2;
			flicker->direction2 = 2;
			if (getINI(0xc2)->sceneId == 1) {
				flicker->actor->updateSequence(0x30);
			} else {
				flicker->actor->updateSequence(2);
			}
			_flickerIdleCounter = 0;
			setFlags(ENGINE_FLAG_80000000);
		}
	}
	if (isFlagSet(ENGINE_FLAG_80000000)
		&& _dragonINIResource->getFlickerRecord()->actor->isFlagSet(ACTOR_FLAG_4)) {
		_flickerIdleCounter = 0;
		clearFlags(ENGINE_FLAG_80000000);
	}
}

void DragonsEngine::updateCamera() {
	if (isFlagSet(ENGINE_FLAG_40) && !isUnkFlagSet(ENGINE_UNK1_FLAG_1)) {
		return;
	}

	if (isFlagSet(ENGINE_FLAG_1) && !isUnkFlagSet(ENGINE_UNK1_FLAG_2)) { //TODO original doesn't seem to check for flag 2 here. adding to get cutscenes to work.
		DragonINI *flicker = _dragonINIResource->getFlickerRecord();
		if (flicker && flicker->sceneId != 0) {
			if ((flicker->actor->_x_pos - _scene->_camera.x >= 0x4f)) {
				if (flicker->actor->_x_pos - _scene->_camera.x >= 0xf0) {
					_scene->_camera.x = flicker->actor->_x_pos - 0xf0;
				}
			} else {
				_scene->_camera.x = flicker->actor->_x_pos - 0x50;
			}

			int16 sVar4 = flicker->actor->_y_pos + -0x1e;
			if (((int)flicker->actor->_y_pos - (int)_scene->_camera.y < 0x1e) ||
				(sVar4 = flicker->actor->_y_pos + -0xaa, 0xaa < (int)flicker->actor->_y_pos - (int)_scene->_camera.y)) {
				_scene->_camera.y = sVar4;
			}
		}

		if (_scene->_camera.x < 0) {
			_scene->_camera.x = 0;
		}
		if (_scene->getStageWidth() < _scene->_camera.x + 0x140) {
			_scene->_camera.x = _scene->getStageWidth() - 0x140;
		}
		if (_scene->_camera.y < 0) {
			_scene->_camera.y = 0;
		}
		if (_scene->getStageHeight() < _scene->_camera.y + 200) {
			_scene->_camera.y = _scene->getStageHeight() + -200;
		}
	}
}

void DragonsEngine::initSubtitleFlag() {
	bool showSubtitles = ConfMan.getBool("subtitles");
	if (showSubtitles) {
		clearFlags(ENGINE_FLAG_1000_SUBTITLES_DISABLED);
	} else {
		setFlags(ENGINE_FLAG_1000_SUBTITLES_DISABLED);
	}
}

int centerText(const char *text) {
	return 0x14 - (strlen(text) / 2 + 1);
}

void DragonsEngine::mainMenu() {
	_inMenu = true;
	//TODO need to support other languages.
	const char copyright[6][40] = {
			"Crystal Dynamics is a trademark",
			"of Crystal Dynamics.",
			"Blazing Dragons is a trademark and",
			"copyright of Terry Jones and is",
			"used with permission.",
			"Licensed by Nelvana Marketing Inc."
	};
	const char menuItems[3][40] = {
			"Start",
			"Options",
			"Previews"
	};

	_screen->clearScreen();
	Actor *actor = _actorManager->loadActor(0xd9,0,0,0,3);
	actor->setFlag(ACTOR_FLAG_8000);
	actor->setFlag(ACTOR_FLAG_100);
	actor->setFlag(ACTOR_FLAG_80);

	//TODO fix palette for copyright image.
	_screen->loadPalette(0, _cursor->getPalette()); //actor->_actorResource->getPalette());

	for (int i = 0; i < 6; i++) {
		_fontManager->addAsciiText(centerText(&copyright[i][0]) * 8, (0x12 + i) * 8, &copyright[i][0], strlen(copyright[i]), 1);
	}

	waitForFramesAllowSkip(400);
	_fontManager->clearText();
	actor->updateSequence(1);

	bool startGame = false;
	do {
		uint16 curMenuItem = 0;
		do {
			for (int i = 0; i < 3; i++) {
				_fontManager->addAsciiText((i == 0 ? 17 : 16) * 8, (0x12 + i) * 8, &menuItems[i][0],
										   strlen(menuItems[i]), i == curMenuItem ? 0 : 1);
			}
			if (checkForDownKeyRelease() || checkForWheelDown()) {
				if (curMenuItem < 2) {
					curMenuItem++;
				} else {
					curMenuItem = 0;
				}
				playOrStopSound(0x8009);
			}

			if (checkForUpKeyRelease() || checkForWheelUp()) {
				if (curMenuItem > 0) {
					curMenuItem--;
				} else {
					curMenuItem = 2;
				}
				playOrStopSound(0x8009);
			}
			waitForFrames(1);
		} while (!checkForActionButtonRelease() && !shouldQuit());

		if (!shouldQuit()) {
			if (curMenuItem == 0) {
				_screen->clearScreen();
				loadingScreen();
				startGame = true;
			} else if (curMenuItem == 1) {
				//TODO options menu
			} else if (curMenuItem == 2) {
				_strPlayer->playVideo("previews.str");
			}
		}
	} while (!shouldQuit() && !startGame);

	_inMenu = false;
}

bool DragonsEngine::isInMenu() {
	return _inMenu;
}

void loadingScreenUpdateFunction() {
	getEngine()->loadingScreenUpdate();
}

void DragonsEngine::loadingScreen() {
	const int flamesActorOffset[4] = {2, 0, 3, 1};
	_loadingScreenState = new LoadingScreenState;
	_fontManager->clearText();
	_actorManager->clearActorFlags(2);
	_screen->addFlatQuad(0,0,0x13f,0,0x13f,199,0,199,0x34a2,6,0);
	Actor *actor = _actorManager->loadActor(0,0x82,0,0,6);
	actor->setFlag(ACTOR_FLAG_100);
	actor->setFlag(ACTOR_FLAG_200);
	actor->setFlag(ACTOR_FLAG_80);
	actor = _actorManager->loadActor(0,0x83,0,0,6);
	actor->setFlag(ACTOR_FLAG_100);
	actor->setFlag(ACTOR_FLAG_200);
	actor->setFlag(ACTOR_FLAG_80);

	if (_language == Common::DE_DEU || _language == Common::FR_FRA) {
		actor = _actorManager->loadActor(0,0x84,0,0,6);
		actor->setFlag(ACTOR_FLAG_100);
		actor->setFlag(ACTOR_FLAG_200);
		actor->setFlag(ACTOR_FLAG_80);
		actor = _actorManager->loadActor(0,0x85,0,0,6);
		actor->setFlag(ACTOR_FLAG_100);
		actor->setFlag(ACTOR_FLAG_200);
		actor->setFlag(ACTOR_FLAG_80);
	}

	for (int i = 0; i < 10; i++) {
		actor = _actorManager->loadActor(0,flamesActorOffset[(i % 4)] + 0x7e,i * 0x20 + 0x10,0xbe,6);
		actor->setFlag(ACTOR_FLAG_100);
		actor->setFlag(ACTOR_FLAG_200);
		actor->setFlag(ACTOR_FLAG_80);
		_loadingScreenState->flames[i] = actor;
		_loadingScreenState->baseYOffset = 0xbe;
		int x0 = i * 0x20;
		_loadingScreenState->quads[i] = _screen->addFlatQuad(x0,0,x0 + 0x20,0,x0 + 0x20,199,x0,199,0,7,0);
	}

	setVsyncUpdateFunction(loadingScreenUpdateFunction);
	waitForFramesAllowSkip(400);
	_actorManager->clearActorFlags(2);
	_screen->clearAllFlatQuads();
	setVsyncUpdateFunction(nullptr);
	delete _loadingScreenState;
	_loadingScreenState = nullptr;
}

void DragonsEngine::loadingScreenUpdate() {
	const int16 flameOffsetTbl[26] = {
			3,      4,      5,      6,
			7,      5,      4,      3,
			3,      4,      6,      7,
			6,      5,      5,      6,
			7,      6,      4,      3,
			2,      3,      4,      5,
			6,      5
	};
	FlatQuad *quad;
	int16 flameYOffset;

	if (_loadingScreenState->loadingFlamesUpdateCounter == 0) {
		_loadingScreenState->loadingFlamesUpdateCounter = 4;
		for (int i = 0; i < 10 ; i++) {
			flameYOffset = _loadingScreenState->baseYOffset - flameOffsetTbl[(i + _loadingScreenState->flameOffsetIdx) % 26];
			if (_loadingScreenState->flames[i]->_y_pos >= -0xb) {
				_loadingScreenState->flames[i]->_y_pos = flameYOffset;
			}
			quad = _screen->getFlatQuad(_loadingScreenState->quads[i]);
			if (quad->points[0].y >= -1) {
				quad->points[0].y = flameYOffset + 2;
				quad->points[1].y = flameYOffset + 2;
			}
		}
		_loadingScreenState->flameOffsetIdx = (_loadingScreenState->flameOffsetIdx + 1) % 26;
	} else {
		_loadingScreenState->loadingFlamesUpdateCounter--;
	}

	if (_loadingScreenState->loadingFlamesRiseCounter == 0) {
		_loadingScreenState->loadingFlamesRiseCounter = 1;
		_loadingScreenState->baseYOffset--;
	} else {
		_loadingScreenState->loadingFlamesRiseCounter--;
	}
}

bool DragonsEngine::checkAudioVideoFiles() {
	return validateAVFile("crystald.str") &&
		   validateAVFile("illusion.str") &&
		   validateAVFile("labintro.str") &&
		   validateAVFile("previews.str") &&
		   validateAVFile("dtspeech.xa");
}

bool DragonsEngine::validateAVFile(const char *filename) {
	const byte fileSignature[12] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
	byte buf[12];
	Common::File file;
	bool fileValid = false;

	if (!file.open(filename)) {
		error("Failed to open %s", filename);
	}

	if ((file.size() % 2352) == 0) {
		file.read(buf, 12);
		if (!memcmp(fileSignature, buf, 12)) {
			fileValid = true;
		}
	}

	file.close();

	if(!fileValid) {
		const char* urlForFileExtractionInfo = "https://wiki.scummvm.org/index.php?title=HOWTO-PlayStation_Videos";
		GUIErrorMessageWithURL(Common::U32String::format(_("Error: The file '%s' hasn't been extracted properly.\nPlease refer to the wiki page\n%s for details on how to properly extract the DTSPEECH.XA and *.STR files from your game disc."), filename, urlForFileExtractionInfo), urlForFileExtractionInfo);
	}
	return fileValid;
}

bool DragonsEngine::checkForWheelUp() {
	return _mouseWheel == MOUSE_WHEEL_UP;
}

bool DragonsEngine::checkForWheelDown() {
	return _mouseWheel == MOUSE_WHEEL_DOWN;
}

void DragonsEngine::clearAllText() {
	_fontManager->clearText();
}

void DragonsEngine::syncSoundSettings() {
	Engine::syncSoundSettings();
	if (_sound) {
		_sound->syncSoundSettings();
	}
}

uint16 DragonsEngine::getCursorHandPointerSequenceID() {
	return _language == Common::DE_DEU || _language == Common::FR_FRA ? 0x86 : 0x84;
}

uint32 DragonsEngine::getMiniGame3StartingDialog() {
	switch (_language) {
	case Common::DE_DEU : return 0x5456;
	case Common::FR_FRA : return 0x509C;
	default : break;
	}
	return 0x479A;
}

uint32 DragonsEngine::getMiniGame3PickAHatDialog() {
	switch (_language) {
	case Common::DE_DEU : return 0x2E32E;
	case Common::FR_FRA : return 0x2F180;
	default : break;
	}
	return 0x2958A;
}

uint32 DragonsEngine::getMiniGame3DataOffset() {
	if (_language == Common::DE_DEU || _language == Common::FR_FRA) {
		return 0x265c;
	}
	return 0x4914;
}

uint32 DragonsEngine::getDialogTextId(uint32 textId) {
	switch (_language) {
	case Common::EN_GRB : return getDialogTextIdGrb(textId);
	case Common::DE_DEU : return getDialogTextIdDe(textId);
	case Common::FR_FRA : return getDialogTextIdFr(textId);
	default : break;
	}
	return textId;
}

uint32 DragonsEngine::getDialogTextIdGrb(uint32 textId) {
	switch (textId) {

		//scene 1
	case 0x5ef2 : return 0x5ef0;
	case 0x5ea2 : return 0x5ea0;
	case 0x5ecc : return 0x5eca;

	default: break;
	}
	return textId;
}

uint32 DragonsEngine::getDialogTextIdDe(uint32 textId) {
	switch (textId) {
	//diamond cutscene
	case 0x4294a : return 0x4a584;
	case 0x42A66 : return 0x4a6c4;
	case 0x42AC2 : return 0x4a744;
	case 0x42B56 : return 0x4a7f0;

	//scene 1
	case 0x4e26 : return 0x5c2c;
	case 0x4ea2 : return 0x5ca0;
	case 0x4eec : return 0x5cfa;
	case 0x5000 : return 0x5de4;
	case 0x5074 : return 0x5e78;
	case 0x511c : return 0x5f0e;
	case 0x5138 : return 0x5f28;
	case 0x5152 : return 0x6000;
	case 0x51fc : return 0x60e4;
	case 0x52d6 : return 0x6208;
	case 0x530c : return 0x6244;
	case 0x54dc : return 0x6466;
	case 0x55d4 : return 0x654e;
	case 0x562c : return 0x65a6;
	case 0x5780 : return 0x66f4;
	case 0x581c : return 0x679c;
	case 0x5942 : return 0x6910;
	case 0x5aaa : return 0x6aa8;
	case 0x5afc : return 0x6af4;
	case 0x5b96 : return 0x6bc0;
	case 0x5c4a : return 0x6c66;
	case 0x5dc8 : return 0x6e30;
	case 0x5ef2 : return 0x6f9c;
	case 0x6000 : return 0x7000;
	case 0x7dcc : return 0x92da;
	case 0x60ee : return 0x710e;
	case 0x5de8 : return 0x6e58;
	case 0x5ea2 : return 0x6f46;
	case 0x5ecc : return 0x6f6e;

	// knights saved
	case 0x7854 : return 0x8bb4;
	case 0x78c6 : return 0x8c32;
	case 0x78e8 : return 0x8c56;
	case 0x7a1e : return 0x8d9e;
	case 0x7aba : return 0x8e64;
	case 0x7b60 : return 0x9000;
	case 0x7c20 : return 0x90fa;
	case 0x7c9c : return 0x9196;
	case 0x7cf2 : return 0x91fa;
	case 0x7e1a : return 0x9324;
	case 0x7e96 : return 0x93e2;
	case 0x7f0a : return 0x9464;

	// flame returns
	case 0x8ab2 : return 0xa190;
	case 0x8b40 : return 0xa274;
	case 0x8bb6 : return 0xa2e0;
	case 0x8bd8 : return 0xa304;
	case 0x8c70 : return 0xa3d2;
	case 0x8cd2 : return 0xa450;
	case 0x8e1e : return 0xa5a2;

	// knights saved again
	case 0x9000 : return 0xa806;
	case 0x90de : return 0xa8f8;
	case 0x921c : return 0xaa8e;
	case 0x92aa : return 0xab3c;
	case 0x932c : return 0xabc0;
	case 0x93d6 : return 0xac52;
	case 0x948c : return 0xad16;

	// tournament
	case 0x4c40c : return 0x552f8;
	case 0x4c530 : return 0x55430;
	case 0x4c588 : return 0x5547e;
	case 0x4c6b0 : return 0x5557a;
	case 0x4c6e8 : return 0x555a2;
	case 0x4c814 : return 0x556c4;
	case 0x4c852 : return 0x556fc;

	//for camelhot
	case 0x30DD8 : return 0x36594;

	// castle garden bg dialog
	case 0x22660 : return 0x267ec;
	case 0x226CA : return 0x2687a;
	case 0x22738 : return 0x2690a;
	case 0x22790 : return 0x26972;
	case 0x227E8 : return 0x269e6;
	case 0x2283C : return 0x26a44;
	case 0x228A0 : return 0x26ac4;
	case 0x228EC : return 0x26b1e;

	// knight pool reflection logic
	case 0x23E90 : return 0x28456;
	case 0x23EE6 : return 0x284aa;
	case 0x23F0C : return 0x284e0;
	case 0x23F86 : return 0x2856c;
	case 0x24000 : return 0x285c4;
	case 0x2406A : return 0x2863c;
	case 0x240C2 : return 0x286a0;
	case 0x2411E : return 0x28706;
	case 0x24158 : return 0x28746;
	case 0x241BC : return 0x287a6;
	case 0x241EE : return 0x287d6;
	case 0x24240 : return 0x28832;
	case 0x24286 : return 0x2887e;
	case 0x242B0 : return 0x288ae;
	case 0x2431C : return 0x2891e;
	case 0x2437C : return 0x28994;
	case 0x243B2 : return 0x289dc;
	case 0x2440A : return 0x28a46;
	case 0x24432 : return 0x28a7e;
	case 0x24480 : return 0x28ac8;
	case 0x244EE : return 0x28b40;
	case 0x2453C : return 0x28b9e;

	// Zigmond fraud logic
	case 0x2D000 : return 0x32312;
	case 0x2D044 : return 0x32364;
	case 0x2D0B2 : return 0x323d0;
	case 0x2D0D6 : return 0x323fc;
	case 0x2D152 : return 0x32490;
	case 0x2D1A4 : return 0x324f0;
	case 0x2D20A : return 0x32566;
	case 0x2D27C : return 0x325f0;
	case 0x2D2EC : return 0x32674;
	case 0x2D336 : return 0x326c6;
	case 0x2D3E0 : return 0x3274c;
	case 0x2D456 : return 0x327d2;
	case 0x2D4A8 : return 0x3283c;
	case 0x2D504 : return 0x328a0;

	// Zigmond fraud logic 1
	case 0x2F422 : return 0x348c6;

	// Black dragon on hill
	case 0x325EA : return 0x37f94;
	case 0x3262A : return 0x38000;
	case 0x32686 : return 0x3806c;
	case 0x326D8 : return 0x380fc;
	case 0x3270E : return 0x3814a;
	case 0x32774 : return 0x381a2;
	case 0x32D72 : return 0x38850;

	// Jester in library
	case 0x18502 : return 0x1b7f8;
	case 0x185E0 : return 0x1b900;
	case 0x18596 : return 0x1b8a4;

	// Broken black dragon
	case 0x40802 : return 0x47ec2;
	case 0x40852 : return 0x47f2a;
	case 0x40896 : return 0x47f70;
	case 0x408C0 : return 0x48000;
	case 0x4092A : return 0x48084;

	// Dodo under attack
	case 0x3353A : return 0x39100;
	case 0x335AC : return 0x3917e;
	case 0x335F8 : return 0x391e4;
	case 0x33660 : return 0x39252;
	case 0x336DE : return 0x392da;
	case 0x3375C : return 0x39364;

	// Forest without dodo
	case 0x33EA0 : return 0x39c02;
	case 0x33EFC : return 0x39c98;
	case 0x33F34 : return 0x39cd0;
	case 0x34000 : return 0x39d30;
	case 0x34074 : return 0x39dc4;

	// Angry villagers
	case 0x35946 : return 0x3ba7e;
	case 0x359BC : return 0x3baee;
	case 0x35A38 : return 0x3bb68;
	case 0x35ABC : return 0x3bbd4;
	case 0x35B28 : return 0x3bc68;
	case 0x35B9C : return 0x3bcd4;
	case 0x35C10 : return 0x3bd78;
	case 0x35C80 : return 0x3be0e;
	case 0x35CFA : return 0x3be96;
	case 0x35D64 : return 0x3bf26;

	// Flame bedroom escape
	case 0x10458 : return 0x12ac2;
	case 0x104A0 : return 0x12b16;
	case 0x10500 : return 0x12b72;
	case 0x10550 : return 0x12bb6;
	case 0x10578 : return 0x12be6;

	// Lady of the lake captured
	case 0x490C8 : return 0x51ace;
	case 0x490FC : return 0x51b04;
	case 0x4913A : return 0x51b3e;

	// Men in mines
	case 0x4590A : return 0x4db1c;
	case 0x45994 : return 0x4dbba;
	case 0x459F4 : return 0x4dc2c;
	case 0x45A60 : return 0x4dc92;

	// Moat drained
	case 0x3C97A : return 0x43830;
	case 0x3C9AC : return 0x43870;
	case 0x3C9F8 : return 0x438dc;
	case 0x3CA48 : return 0x43966;

	// Monks at bar
	case 0x37800 : return 0x3db58;
	case 0x37854 : return 0x3dba0;
	case 0x378CA : return 0x3dc1e;
	case 0x39152 : return 0x3f754;
	case 0x3919A : return 0x3f78a;
	case 0x3922C : return 0x3f82a;

	case 0x38F2A : return 0x3f52c;
	case 0x39000 : return 0x3f5a0;
	case 0x39084 : return 0x3f65a;
	case 0x390E8 : return 0x3f6d6;

	case 0x38C68 : return 0x3f25e;
	case 0x38CE2 : return 0x3f2da;
	case 0x38D4E : return 0x3f348;
	case 0x38DC2 : return 0x3f3ba;
	case 0x38E0C : return 0x3f3f4;
	case 0x38E5C : return 0x3f44a;
	case 0x38ED0 : return 0x3f4c0;

	// Mini game 1
	case 0x21312 : return 0x25258;
	case 0x2134C : return 0x252A0;
	case 0x21386 : return 0x252D6;
	case 0x213C0 : return 0x25324;
	case 0x213E2 : return 0x25354;
	case 0x21428 : return 0x253A4;
	case 0x2146C : return 0x253EC;
	case 0x214B4 : return 0x25446;
	case 0x214E4 : return 0x25472;
	case 0x21514 : return 0x254AC;
	case 0x21540 : return 0x254DC;
	case 0x21590 : return 0x25530;
	case 0x215E2 : return 0x2559A;
	case 0x2164E : return 0x25626;
	case 0x216AA : return 0x25690;
	case 0x216D2 : return 0x256D6;
	case 0x217D8 : return 0x257DC;

	// Mini game 2
	case 0x4500 : return 0x50F4;
	case 0x454A : return 0x5164;
	case 0x4576 : return 0x519E;
	case 0x46BC : return 0x532C;
	case 0x4718 : return 0x53B6;
	case 0x475E : return 0x5420;
	case 0x4774 : return 0x5436;

	// Mini game 4
	case 0x49A2 : return 0x5706;
	case 0x4A84 : return 0x580C;
	case 0x4ADE : return 0x5876;
	case 0x4B6A : return 0x5904;
	case 0x4C0C : return 0x59C6;
	case 0x4D50 : return 0x5B24;
	case 0x4DD4 : return 0x5BC6;
	case 0x4DEE : return 0x5BE8;
	case 0x4CC8 : return 0x5A7E;
	case 0x4A56 : return 0x57D4;

	// Mini game 5
	case 0x21DAE : return 0x25E68;
	case 0x21BF0 : return 0x25C34;
	case 0x21E3E : return 0x25F10;
	case 0x475DA : return 0x4FC9E;

	default: break;
	}
	warning("Unhandled textID 0x%x", textId);
	return textId;
}

uint32 DragonsEngine::getDialogTextIdFr(uint32 textId) {
	switch (textId) {
	//diamond cutscene
	case 0x4294a : return 0x4c8b2;
	case 0x42A66 : return 0x4ca14;
	case 0x42AC2 : return 0x4ca76;
	case 0x42B56 : return 0x4cb34;

	//scene 1
	case 0x4e26 : return 0x5844;
	case 0x4ea2 : return 0x58e2;
	case 0x4eec : return 0x5950;
	case 0x5000 : return 0x5a32;
	case 0x5074 : return 0x5ac0;
	case 0x511c : return 0x5b7a;
	case 0x5138 : return 0x5b98;
	case 0x5152 : return 0x5bb4;
	case 0x51fc : return 0x5c6c;
	case 0x52d6 : return 0x5d8a;
	case 0x530c : return 0x5dc8;
	case 0x54dc : return 0x6000;
	case 0x55d4 : return 0x610c;
	case 0x562c : return 0x616a;
	case 0x5780 : return 0x62ba;
	case 0x581c : return 0x6332;
	case 0x5942 : return 0x64b4;
	case 0x5aaa : return 0x660e;
	case 0x5afc : return 0x665e;
	case 0x5b96 : return 0x6752;
	case 0x5c4a : return 0x6838;
	case 0x5dc8 : return 0x69d0;
	case 0x5ef2 : return 0x6b1a;
	case 0x6000 : return 0x6b76;
	case 0x7dcc : return 0x8cc0;
	case 0x60ee : return 0x6c56;
	case 0x5de8 : return 0x69f2;
	case 0x5ea2 : return 0x6ac4;
	case 0x5ecc : return 0x6aec;

	// knights saved
	case 0x7854 : return 0x86d6;
	case 0x78c6 : return 0x8736;
	case 0x78e8 : return 0x8750;
	case 0x7a1e : return 0x88b0;
	case 0x7aba : return 0x8974;
	case 0x7b60 : return 0x8a22;
	case 0x7c20 : return 0x8afe;
	case 0x7c9c : return 0x8b92;
	case 0x7cf2 : return 0x8bda;
	case 0x7e1a : return 0x8d0e;
	case 0x7e96 : return 0x8da6;
	case 0x7f0a : return 0x8e30;

	// flame returns
	case 0x8ab2 : return 0x9ae2;
	case 0x8b40 : return 0x9ba6;
	case 0x8bb6 : return 0x9c16;
	case 0x8bd8 : return 0x9c3c;
	case 0x8c70 : return 0x9ccc;
	case 0x8cd2 : return 0x9d22;
	case 0x8e1e : return 0x9e8c;

	// knights saved again
	case 0x9000 : return 0xa086;
	case 0x90de : return 0xa162;
	case 0x921c : return 0xa2b8;
	case 0x92aa : return 0xa35e;
	case 0x932c : return 0xa400;
	case 0x93d6 : return 0xa4d2;
	case 0x948c : return 0xa5b6;

	// tournament
	case 0x4c40c : return 0x577b4;
	case 0x4c530 : return 0x578be;
	case 0x4c588 : return 0x57908;
	case 0x4c6b0 : return 0x57a26;
	case 0x4c6e8 : return 0x57a4a;
	case 0x4c814 : return 0x57bd6;
	case 0x4c852 : return 0x57c08;

	//for camelhot
	case 0x30DD8 : return 0x37cb8;

	// castle garden bg dialog
	case 0x22660 : return 0x271a4;
	case 0x226CA : return 0x27214;
	case 0x22738 : return 0x27292;
	case 0x22790 : return 0x272ee;
	case 0x227E8 : return 0x27366;
	case 0x2283C : return 0x273c8;
	case 0x228A0 : return 0x2743a;
	case 0x228EC : return 0x274a2;

	// knight pool reflection logic
	case 0x23E90 : return 0x28da8;
	case 0x23EE6 : return 0x28e16;
	case 0x23F0C : return 0x28e42;
	case 0x23F86 : return 0x28ec0;
	case 0x24000 : return 0x28efa;
	case 0x2406A : return 0x28f64;
	case 0x240C2 : return 0x29000;
	case 0x2411E : return 0x29068;
	case 0x24158 : return 0x290a6;
	case 0x241BC : return 0x29114;
	case 0x241EE : return 0x2914a;
	case 0x24240 : return 0x291b2;
	case 0x24286 : return 0x29206;
	case 0x242B0 : return 0x29234;
	case 0x2431C : return 0x292a8;
	case 0x2437C : return 0x29332;
	case 0x243B2 : return 0x2937e;
	case 0x2440A : return 0x293d0;
	case 0x24432 : return 0x293f2;
	case 0x24480 : return 0x29434;
	case 0x244EE : return 0x294a8;
	case 0x2453C : return 0x29500;

	// Zigmond fraud logic
	case 0x2D000 : return 0x33602;
	case 0x2D044 : return 0x33654;
	case 0x2D0B2 : return 0x336d2;
	case 0x2D0D6 : return 0x336f6;
	case 0x2D152 : return 0x337c2;
	case 0x2D1A4 : return 0x33818;
	case 0x2D20A : return 0x338a2;
	case 0x2D27C : return 0x33952;
	case 0x2D2EC : return 0x33a06;
	case 0x2D336 : return 0x33a56;
	case 0x2D3E0 : return 0x33b24;
	case 0x2D456 : return 0x33bd0;
	case 0x2D4A8 : return 0x33c36;
	case 0x2D504 : return 0x33c9e;

	// Zigmond fraud logic 1
	case 0x2F422 : return 0x3608a;

	// Black dragon on hill
	case 0x325EA : return 0x39822;
	case 0x3262A : return 0x3986e;
	case 0x32686 : return 0x398dc;
	case 0x326D8 : return 0x39944;
	case 0x3270E : return 0x39982;
	case 0x32774 : return 0x39a08;
	case 0x32D72 : return 0x3a162;

	// Jester in library
	case 0x18502 : return 0x1b6d6;
	case 0x185E0 : return 0x1b7e2;
	case 0x18596 : return 0x1b790;

	// Broken black dragon
	case 0x40802 : return 0x4a178;
	case 0x40852 : return 0x4a1d2;
	case 0x40896 : return 0x4a212;
	case 0x408C0 : return 0x4a252;
	case 0x4092A : return 0x4a310;

	// Dodo under attack
	case 0x3353A : return 0x3aa2a;
	case 0x335AC : return 0x3aaae;
	case 0x335F8 : return 0x3ab02;
	case 0x33660 : return 0x3ab9c;
	case 0x336DE : return 0x3ac46;
	case 0x3375C : return 0x3acf0;

	// Forest without dodo
	case 0x33EA0 : return 0x3b63a;
	case 0x33EFC : return 0x3b6b2;
	case 0x33F34 : return 0x3b6e8;
	case 0x34000 : return 0x3b76a;
	case 0x34074 : return 0x3b7e2;

	// Angry villagers
	case 0x35946 : return 0x3d476;
	case 0x359BC : return 0x3d4fa;
	case 0x35A38 : return 0x3d570;
	case 0x35ABC : return 0x3d64e;
	case 0x35B28 : return 0x3d6cc;
	case 0x35B9C : return 0x3d71a;
	case 0x35C10 : return 0x3d79e;
	case 0x35C80 : return 0x3d81e;
	case 0x35CFA : return 0x3d8be;
	case 0x35D64 : return 0x3d94c;

	// Flame bedroom escape
	case 0x10458 : return 0x12598;
	case 0x104A0 : return 0x125f4;
	case 0x10500 : return 0x12664;
	case 0x10550 : return 0x12692;
	case 0x10578 : return 0x126d4;

	// Lady of the lake captured
	case 0x490C8 : return 0x53ef8;
	case 0x490FC : return 0x53f42;
	case 0x4913A : return 0x54000;

	// Men in mines
	case 0x4590A : return 0x50000;
	case 0x45994 : return 0x500be;
	case 0x459F4 : return 0x50142;
	case 0x45A60 : return 0x501a0;

	// Moat drained
	case 0x3C97A : return 0x45826;
	case 0x3C9AC : return 0x45862;
	case 0x3C9F8 : return 0x458c0;
	case 0x3CA48 : return 0x4595e;

		// Monks at bar
	case 0x37800 : return 0x3f7d4;
	case 0x37854 : return 0x3f84e;
	case 0x378CA : return 0x3f8b2;
	case 0x39152 : return 0x4153a;
	case 0x3919A : return 0x4157e;
	case 0x3922C : return 0x41616;

	case 0x38F2A : return 0x41388;
	case 0x39000 : return 0x413f4;
	case 0x39084 : return 0x4147e;
	case 0x390E8 : return 0x414d4;

	case 0x38C68 : return 0x41000;
	case 0x38CE2 : return 0x41098;
	case 0x38D4E : return 0x41126;
	case 0x38DC2 : return 0x411b2;
	case 0x38E0C : return 0x41218;
	case 0x38E5C : return 0x41278;
	case 0x38ED0 : return 0x412fa;

	// Mini game 1
	case 0x21312 : return 0x25B60;
	case 0x2134C : return 0x25B98;
	case 0x21386 : return 0x25BD6;
	case 0x213C0 : return 0x25C10;
	case 0x213E2 : return 0x25C38;
	case 0x21428 : return 0x25C7E;
	case 0x2146C : return 0x25CE0;
	case 0x214B4 : return 0x25D36;
	case 0x214E4 : return 0x25D5E;
	case 0x21514 : return 0x25D8A;
	case 0x21540 : return 0x25DCE;
	case 0x21590 : return 0x25E4E;
	case 0x215E2 : return 0x25EB8;
	case 0x2164E : return 0x25F2C;
	case 0x216AA : return 0x25F9C;
	case 0x216D2 : return 0x26000;
	case 0x217D8 : return 0x260F4;

	// Mini game 2
	case 0x4500 : return 0x4D78;
	case 0x454A : return 0x4DCC;
	case 0x4576 : return 0x4E0A;
	case 0x46BC : return 0x4F32;
	case 0x4718 : return 0x5000;
	case 0x475E : return 0x505A;
	case 0x4774 : return 0x5070;

	// Mini game 4
	case 0x49A2 : return 0x5360;
	case 0x4A84 : return 0x5476;
	case 0x4ADE : return 0x54C0;
	case 0x4B6A : return 0x554C;
	case 0x4C0C : return 0x55EA;
	case 0x4D50 : return 0x5742;
	case 0x4DD4 : return 0x57F0;
	case 0x4DEE : return 0x580E;
	case 0x4CC8 : return 0x569C;
	case 0x4A56 : return 0x5438;

	// Mini game 5
	case 0x21DAE : return 0x267C2;
	case 0x21BF0 : return 0x265BC;
	case 0x21E3E : return 0x2686A;
	case 0x475DA : return 0x51F88;

	default: break;
	}
	warning("Unhandled textID 0x%x", textId);
	return textId;
}

void (*DragonsEngine::getSceneUpdateFunction())() {
	return _sceneUpdateFunction;
}

} // End of namespace Dragons
