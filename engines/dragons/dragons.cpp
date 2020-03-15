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
#include "common/config-manager.h"
#include "common/keyboard.h"
#include "common/language.h"
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

	_leftMouseButtonUp = false;
	_leftMouseButtonDown = false;
	_rightMouseButtonUp = false;
	_iKeyUp = false;
	_downKeyUp = false;
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

	_debugMode = false;
	_isGamePaused = false;
	_inMenu = false;

	_bit_flags_8006fbd8 = 0;

	reset();
}

DragonsEngine::~DragonsEngine() {
	delete _sequenceOpcodes;
	delete _scriptOpcodes;
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
	while (_eventMan->pollEvent(event)) {
//		_input->processEvent(event);
		switch (event.type) {
		case Common::EVENT_QUIT:
			quitGame();
			break;
		case Common::EVENT_MOUSEMOVE:
			_cursor->updatePosition(event.mouse.x, event.mouse.y);
			break;
		case Common::EVENT_LBUTTONUP:
			_leftMouseButtonUp = true;
			_leftMouseButtonDown = false;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_leftMouseButtonDown = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_rightMouseButtonUp = true;
			break;
		case Common::EVENT_KEYUP:
			if (event.kbd.keycode == Common::KEYCODE_i) {
				_iKeyUp = true;
			} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
				_downKeyUp = true;
			} else if (event.kbd.keycode == Common::KEYCODE_UP) {
				_upKeyUp = true;
			} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
				_enterKeyUp = true;
			} else if (event.kbd.keycode == Common::KEYCODE_LEFT) {
				_leftKeyUp = true;
				_leftKeyDown = false;
			} else if (event.kbd.keycode == Common::KEYCODE_RIGHT) {
				_rightKeyUp = true;
				_rightKeyDown = false;
			} else if (event.kbd.keycode == Common::KEYCODE_w) {
				_wKeyDown = false;
			} else if (event.kbd.keycode == Common::KEYCODE_a) {
				_aKeyDown = false;
			} else if (event.kbd.keycode == Common::KEYCODE_s) {
				_sKeyDown = false;
			} else if (event.kbd.keycode == Common::KEYCODE_d) {
				_dKeyDown = false;
			} else if (event.kbd.keycode == Common::KEYCODE_o) {
				_oKeyDown = false;
			} else if (event.kbd.keycode == Common::KEYCODE_p) {
				_pKeyDown = false;
			}
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_LEFT) {
				_leftKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_RIGHT) {
				_rightKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_w) {
				_wKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_a) {
				_aKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_s) {
				_sKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_d) {
				_dKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_o) {
				_oKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_p) {
				_pKeyDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_TAB) {
				_debugMode = !_debugMode;
			}
			break;
		default:
			break;
		}
	}
}

Common::Error DragonsEngine::run() {
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

	_scene->draw();
	_screen->updateScreen();

	gameLoop();

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
	InventoryState uVar6;
	InventoryState uVar7;
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
				uVar6 = _inventory->getState();
				if (checkForInventoryButtonRelease() && isInputEnabled()) {
					uVar7 = _inventory->_previousState;
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
		uVar6 = _inventory->getState();
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
		clearFlags(ENGINE_FLAG_8000);
	}

	//TODO logic here
	for (uint16 i = 0; i < 0x17; i++) {
		Actor *actor = _actorManager->getActor(i);
		if (actor->_flags & ACTOR_FLAG_40) {
			if (!(actor->_flags & ACTOR_FLAG_100)) {
				int16 priority = _scene->getPriorityAtPosition(Common::Point(actor->_x_pos, actor->_y_pos));
				DragonINI *flicker = _dragonINIResource->getFlickerRecord();
				if (flicker && _scene->contains(flicker) && flicker->actor->_actorID == i) {
					if (priority < 8 || priority == 0x10) {
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
	for (uint16 i = 0; i < numFrames; i++) {
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
	debug("play sound 0x%x", soundId);

	this->_sound->playOrStopSound(soundId);
}

void DragonsEngine::updatePathfindingActors() {
	for (uint16 i = 0; i < 0x17; i++) {
		Actor *actor = _actorManager->getActor(i);
		actor->walkPath();
	}
}

void DragonsEngine::fade_related(uint32 flags) {
	if (!isFlagSet(ENGINE_FLAG_40)) {
		return;
	}
	setUnkFlags(ENGINE_UNK1_FLAG_2);
	clearFlags(ENGINE_FLAG_40);

	//TODO 0x80015a1c
}

void DragonsEngine::call_fade_related_1f() {
	fade_related(0x1f);
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
	return false; // TODO
}

bool DragonsEngine::isDownKeyPressed() {
	return false; // TODO
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
	return isInputEnabled();
}

bool DragonsEngine::canSaveGameStateCurrently() {
	return isInputEnabled() && !_inventory->isOpen();
}

bool DragonsEngine::hasFeature(Engine::EngineFeature f) const {
	return
		// TODO (f == kSupportsRTL) ||
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
					int16 uVar14 = (uint)(uint16)_paletteCyclingTbl[loopIndex].startOffset;
					int16 uVar8 = (uint)(uint16)_paletteCyclingTbl[loopIndex].endOffset;
					if (uVar14 < uVar8) {
						uint16 uVar11 = palette[uVar8];
						int uVar15 = uVar8;
						if (uVar14 < uVar8) {
							do {
								uVar8--;
								palette[uVar15] = palette[uVar15 - 1];
								uVar15 = uVar8 & 0xffff;
							} while ((uint)(uint16)_paletteCyclingTbl[loopIndex].startOffset < (uVar8 & 0xffff));
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
								} while ((uVar8 & 0xffff) < (uint)(uint16)_paletteCyclingTbl[loopIndex].startOffset);
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
	case Common::EN_USA : return 0x4a144;
	case Common::EN_GRB : return 0x4b4fc;
	case Common::DE_DEU : return 0x4af5c;
	case Common::FR_FRA : return 0x4b158;
	default : error("Unable to get font offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint32 DragonsEngine::getSpeechTblOffsetFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA : return 0x4e138;
	case Common::EN_GRB : return 0x4f4f4;
	case Common::DE_DEU : return 0x4f0a4;
	case Common::FR_FRA : return 0x4f2a0;
	default : error("Unable to get speech table offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint32 DragonsEngine::getBigFileInfoTblFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA : return 0x4a238;
	case Common::EN_GRB : return 0x4b5f4;
	case Common::DE_DEU : return 0x4b054;
	case Common::FR_FRA : return 0x4b250;
	default :
		error("Unable to get speech table offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint32 DragonsEngine::getCutscenePaletteOffsetFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA : return 0x5336c;
	case Common::EN_GRB : return 0x54628;
	case Common::DE_DEU : return 0x541d8;
	case Common::FR_FRA : return 0x543d4;
	default :
		error("Unable to get speech table offset from dragon.exe for %s", getLanguageCode(_language));
	}
}

uint32 DragonsEngine::defaultResponseOffsetFromDragonEXE() {
	switch (_language) {
	case Common::EN_USA : return 0x541b0;
	case Common::EN_GRB : return 0x55470;
	case Common::DE_DEU : return 0x55020;
	case Common::FR_FRA : return 0x5521c;
	default :
		error("Unable to get speech table offset from dragon.exe for %s", getLanguageCode(_language));
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
			if (checkForDownKeyRelease()) {
				if (curMenuItem < 2) {
					curMenuItem++;
				} else {
					curMenuItem = 0;
				}
				playOrStopSound(0x8009);
			}

			if (checkForUpKeyRelease()) {
				if (curMenuItem > 0) {
					curMenuItem--;
				} else {
					curMenuItem = 2;
				}
				playOrStopSound(0x8009);
			}
			waitForFrames(1);
		} while (!checkForActionButtonRelease() && !shouldQuit());

		if (curMenuItem == 0) {
			_screen->clearScreen();
			loadingScreen();
			startGame = true;
		} else if (curMenuItem == 1) {
			//TODO options menu
		} else if (curMenuItem == 2) {
			_strPlayer->playVideo("previews.str");
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
			flameYOffset = _loadingScreenState->baseYOffset - flameOffsetTbl[(i + _loadingScreenState->flameOffsetIdx) % 27];
			if (_loadingScreenState->flames[i]->_y_pos >= -0xb) {
				_loadingScreenState->flames[i]->_y_pos = flameYOffset;
			}
			quad = _screen->getFlatQuad(_loadingScreenState->quads[i]);
			if (quad->points[0].y >= -1) {
				quad->points[0].y = flameYOffset + 2;
				quad->points[1].y = flameYOffset + 2;
			}
		}
		_loadingScreenState->flameOffsetIdx = (_loadingScreenState->flameOffsetIdx + 1) % 27;
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

void (*DragonsEngine::getSceneUpdateFunction())() {
	return _sceneUpdateFunction;
}

} // End of namespace Dragons
