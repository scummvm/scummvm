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

namespace Dragons {

#define DRAGONS_TICK_INTERVAL 17

static DragonsEngine *_engine = nullptr;

DragonsEngine *getEngine() {
	return _engine;
}

DragonsEngine::DragonsEngine(OSystem *syst) : Engine(syst) {
	_bigfileArchive = NULL;
	_dragonRMS = NULL;
	_backgroundResourceLoader = NULL;
	_screen = NULL;
	_sequenceOpcodes = new SequenceOpcodes(this);
	_scriptOpcodes = NULL;
	_engine = this;
	_inventory = new Inventory(this);
	_cursor = new Cursor(this);
	_credits = NULL;
	_talk = NULL;
	_fontManager = NULL;
	_sceneUpdateFunction = NULL;
	_vsyncUpdateFunction = NULL;

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
	_bigfileArchive = new BigfileArchive("bigfile.dat", Common::EN_ANY);
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

	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	} else {
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

	debug("Ok");
	return Common::kNoError;
}

uint16 DragonsEngine::ipt_img_file_related() {
    DragonINI *flicker = _dragonINIResource->getFlickerRecord();
	assert(flicker);

	int16 tileX = flicker->actor->_x_pos / 32;
	int16 tileY = flicker->actor->_y_pos / 8;

	for (int i = 0;i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if ((ini->sceneId == getCurrentSceneId()) && (ini->field_1a_flags_maybe == 0)) {
			Img *img = _dragonImg->getImg(ini->field_2);
			if ((img->x <= tileX) && (((tileX <= img->x + img->w && (img->y <= tileY)) && (tileY <= img->y + img->h)))) {
				return i + 1;
			}
		}
	}
	return 0;
}

void DragonsEngine::gameLoop() {
	uint uVar3;
	uint actorId;
	uint16 uVar6;
	uint16 uVar7;
	uint actorId_00;
	uint16 sequenceId;
	DragonINI *pDVar8;

	_cursor->_cursorActivationSeqOffset = 0;
	_bit_flags_8006fbd8 = 0;
	_counter = 0;
	setFlags(ENGINE_FLAG_8);
	actorId = 0;

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

		_counter++;
		if (0x4af < _counter) {
			pDVar8 = _dragonINIResource->getFlickerRecord();
			if (pDVar8->actor->_resourceID == 0xe) {
				pDVar8->actor->_sequenceID2 = 2;
				pDVar8->field_20_actor_field_14 = 2;
				if (getINI(0xc2)->field_1e == 1) {
					sequenceId = 0x30;
				}
				else {
					sequenceId = 2;
				}
				pDVar8->actor->updateSequence(sequenceId);
				_counter = 0;
				setFlags(ENGINE_FLAG_80000000);
			}
		}
		if (isFlagSet(ENGINE_FLAG_80000000)
			&& _dragonINIResource->getFlickerRecord()->actor->isFlagSet(ACTOR_FLAG_4)) {
			_counter = 0;
			clearFlags(ENGINE_FLAG_80000000);
		}
		if (_bit_flags_8006fbd8 == 0) {
			setFlags(ENGINE_FLAG_8);
		}
		if (_dragonINIResource->getFlickerRecord()->sceneId == getCurrentSceneId()) {
			uVar3 = ipt_img_file_related();
			actorId_00 = uVar3 & 0xffff;
			if (actorId_00 == 0) goto LAB_80026d34;
			if (actorId_00 != (actorId & 0xffff)) {
				byte *obd = _dragonOBD->getFromOpt(actorId_00 - 1);
				ScriptOpCall scriptOpCall(obd + 8, READ_LE_UINT32(obd));

				if (_scriptOpcodes->runScript4(scriptOpCall)) {
					scriptOpCall._codeEnd = scriptOpCall._code + 4 + READ_LE_UINT16(scriptOpCall._code + 2);
					scriptOpCall._code += 4;
					_scriptOpcodes->runScript(scriptOpCall);
					_counter = 0;
				}
			}
		}
		else {
			LAB_80026d34:
			uVar3 = 0;
		}

		if (_cursor->updateINIUnderCursor() == 0 ||
			(!(_cursor->_iniUnderCursor & 0x8000) && (getINI(_cursor->_iniUnderCursor - 1)->field_1a_flags_maybe & 0x4000) != 0)) { //TODO check this. This logic looks a bit strange.
			_cursor->_cursorActivationSeqOffset = 0;
		}
		else {
			_cursor->_cursorActivationSeqOffset = 5;
		}

		if (_rightMouseButtonUp && isInputEnabled()) {
			_cursor->selectPreviousCursor();
			_counter = 0;
			actorId = uVar3;
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
//			_counter = 0;
//			actorId = uVar3;
//			continue;
//		}

		if (_bit_flags_8006fbd8 == 3) {
			_bit_flags_8006fbd8 = 0;
			DragonINI *flicker = _dragonINIResource->getFlickerRecord();
			if (flicker->sceneId == getCurrentSceneId() && flicker->actor->_sequenceID2 != -1) {
				uVar6 = _scriptOpcodes->_scriptTargetINI;
				if (_cursor->_sequenceID != 5) {
					uVar6 = _cursor->_data_80072890;
				}
				if (uVar6 > 0) {
					flicker->actor->_sequenceID2 = getINI(uVar6 - 1)->field_e;
				}
			}

			performAction();
			if ((getCurrentSceneId() == 0x1d) && (getINI(0x178)->field_12 != 0)) //cave of dilemma
			{
				clearFlags(ENGINE_FLAG_8);
			} else {
				setFlags(ENGINE_FLAG_8);
			}
			_counter = 0;
			actorId = uVar3;
			continue;
		}
		if (_inventory->getType() != 1) {
			if (_inventory->getType() < 2) {
				if (_inventory->getType() == 0) {
					if ((checkForInventoryButtonRelease() && isInputEnabled()) &&
						((_bit_flags_8006fbd8 & 3) != 1)) {
						sequenceId = _dragonVAR->getVar(7);
						uVar7 = _inventory->_old_showing_value;
						_inventory->_old_showing_value = _inventory->getType();
						joined_r0x800271d0:
						_inventory->setType(_inventory->_old_showing_value);
						if (sequenceId == 1) {
							LAB_800279f4:
							_inventory->_old_showing_value = uVar7;
							_inventory->inventoryMissing();
							actorId = uVar3;
						}
						else {
							_counter = 0;
							_inventory->setType(1);
							_inventory->openInventory();
							joined_r0x80027a38:
							if (_cursor->_iniItemInHand == 0) {
								_cursor->_sequenceID = 1;
								actorId = uVar3;
							}
							else {
								_cursor->_sequenceID = 5;
								actorId = uVar3;
							}
						}
						continue;
					}
					uVar6 = _inventory->getType();
					if (checkForActionButtonRelease() && isFlagSet(ENGINE_FLAG_8)) {
						_counter = 0;
						if ((_cursor->_iniUnderCursor & 0x8000) != 0) {
							if (_cursor->_iniUnderCursor == 0x8002) {
								LAB_80027294:
								if (_cursor->_iniItemInHand == 0) {
									if ((_bit_flags_8006fbd8 & 3) != 1) {
										sequenceId = _dragonVAR->getVar(7);
										uVar7 = _inventory->_old_showing_value;
										_inventory->_old_showing_value = _inventory->getType();
										goto joined_r0x800271d0;
									}
								}
								else {
									if (_inventory->addItem(_cursor->_iniItemInHand)) {
										_cursor->_sequenceID = 1;
										waitForFrames(1);
										_cursor->_iniItemInHand = 0;
										_cursor->_iniUnderCursor = 0;
										actorId = uVar3;
										continue;
									}
								}
							}
							else {
								if (_cursor->_iniUnderCursor != 0x8001) goto LAB_80027ab4;
								if (_inventory->getSequenceId() == 0) goto LAB_80027294;
							}
							if ((_cursor->_iniUnderCursor == 0x8001) && (_inventory->getSequenceId() == 1)) {
								_inventory->setType(2);
								_inventory->_old_showing_value = uVar6;
								FUN_80038890();
								actorId = uVar3;
								continue;
							}
						}
						LAB_80027ab4:
						_counter = 0;
						_cursor->_data_80072890 = _cursor->_iniUnderCursor;
						if (_cursor->_sequenceID < 5) {
							_cursor->_data_800728b0_cursor_seqID = _cursor->_sequenceID;
							walkFlickerToObject();
							if (_bit_flags_8006fbd8 != 0) {
								clearFlags(ENGINE_FLAG_8);
							}
						}
						else {
							_cursor->_data_800728b0_cursor_seqID = _cursor->_sequenceID;
							walkFlickerToObject();
							if (_bit_flags_8006fbd8 != 0) {
								clearFlags(ENGINE_FLAG_8);
							}
							_scriptOpcodes->_scriptTargetINI = _cursor->_data_80072890;
							_cursor->_data_80072890 = _cursor->_iniItemInHand;
						}
					}
				}
			}
			else {
				if (_inventory->getType() == 2) {
					uVar6 = _inventory->getType();
					if (checkForInventoryButtonRelease() && isInputEnabled()) {
						uVar7 = _inventory->_old_showing_value;
						if (_dragonVAR->getVar(7) == 1) goto LAB_800279f4;
						_counter = 0;
						_inventory->setType(1);
						_inventory->_old_showing_value = uVar6;
						_inventory->openInventory();
						goto joined_r0x80027a38;
					}
					if (checkForActionButtonRelease() && isFlagSet(ENGINE_FLAG_8)) goto LAB_80027ab4;
				}
			}
			LAB_80027b58:
			runINIScripts();
			actorId = uVar3;
			continue;
		}
		if (checkForInventoryButtonRelease()) {
			_counter = 0;
			LAB_80027970:
			_inventory->closeInventory();
			uVar6 = _inventory->_old_showing_value;
			_inventory->_old_showing_value = _inventory->getType();
			actorId = uVar3;
			_inventory->setType(uVar6);
			continue;
		}
		uVar6 = _inventory->getType();
		if (checkForActionButtonRelease() && isFlagSet(ENGINE_FLAG_8)) {
			_counter = 0;
			if ((_cursor->_iniUnderCursor & 0x8000) != 0) {
				if (_cursor->_iniUnderCursor == 0x8001) {
					_inventory->closeInventory();
					_inventory->setType(0);
					if (_inventory->_old_showing_value == 2) {
						FUN_80038994();
					}
				}
				else {
					if (_cursor->_iniUnderCursor != 0x8002) goto LAB_8002790c;
					_inventory->closeInventory();
					_inventory->setType(2);
					if (_inventory->_old_showing_value != 2) {
						FUN_80038890();
					}
				}
				_inventory->_old_showing_value = uVar6;
				actorId = uVar3;
				continue;
			}
			if (_cursor->_iniUnderCursor != 0) {
				if ((_cursor->_sequenceID != 4) && (_cursor->_sequenceID != 2)) {
					_cursor->_data_800728b0_cursor_seqID = _cursor->_sequenceID;
					_cursor->_data_80072890 = _cursor->_iniUnderCursor;
					if (4 < _cursor->_sequenceID) {
						_cursor->_data_80072890 = _cursor->_iniItemInHand;
						_scriptOpcodes->_scriptTargetINI = _cursor->_iniUnderCursor;
					}
					clearFlags(ENGINE_FLAG_8);
					walkFlickerToObject();
					goto LAB_8002790c;
				}
				Actor *actor = _inventory->getInventoryItemActor(_cursor->_iniUnderCursor);
				uint16 tmpId = _cursor->_iniItemInHand;
				_inventory->replaceItem(_cursor->_iniUnderCursor, _cursor->_iniItemInHand);
				_cursor->data_8007283c = actor->_sequenceID;
				actor->clearFlag(ACTOR_FLAG_40);
				_cursor->_iniItemInHand = _cursor->_iniUnderCursor;
				_cursor->_sequenceID = 5;
				actorId = uVar3;
				if (tmpId != 0) {
					actor->_flags = 0;
					actor->_priorityLayer = 0;
					actor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
					actor->updateSequence(getINI(tmpId - 1)->field_8 * 2 + 10);
					actor->setFlag(ACTOR_FLAG_40);
					actor->setFlag(ACTOR_FLAG_80);
					actor->setFlag(ACTOR_FLAG_100);
					actor->setFlag(ACTOR_FLAG_200);
					actor->_priorityLayer = 6;
					actorId = uVar3;
				}
				continue;
			}
			if (_cursor->_iniItemInHand == 0) goto LAB_80027b58;
			//drop item back into inventory
			if (_inventory->addItemIfPositionIsEmpty(_cursor->_iniItemInHand, _cursor->_x, _cursor->_y)) {
				Actor *invActor = _inventory->getInventoryItemActor(_cursor->_iniItemInHand);
				invActor->_flags = 0;
				invActor->_priorityLayer = 0;
				invActor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
				invActor->updateSequence(
						 getINI(_cursor->_iniItemInHand - 1)->field_8 * 2 + 10);
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
		LAB_8002790c:
		if ((_cursor->_iniItemInHand == 0) ||
			(((uint16)(_cursor->_x - 10U) < 300 && ((uint16)(_cursor->_y - 10U) < 0xb4))))
			goto LAB_80027b58;
		_cursor->_sequenceID = 5;
		waitForFrames(2);
		goto LAB_80027970;
	}
}


void DragonsEngine::updateHandler() {
	_videoFlags |= 0x40;
	//TODO logic here

	updateActorSequences();

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
		if (ini->sceneId == currentSceneId && ini->field_1a_flags_maybe == 0) {
			Img *img = _dragonImg->getImg(ini->field_2);
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
		if (ini->field_1a_flags_maybe & INI_FLAG_10) {
			ini->field_1a_flags_maybe &= ~INI_FLAG_10;
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
/*
void DragonsEngine::engineFlag0x20UpdateFunction() {
{
	uint16 uVar1;
	uint16 uVar2;
	DragonINI *pDVar3;
	DragonINI *pDVar4;
	uint16 uVar5;
	uint actorId;

	DragonINI *flickerINI = _dragonINIResource->getFlickerRecord();
	uint16 currentSceneId = _scene->getSceneId();


//	uVar1 = flickerINI->actorId;
//	actorId = (uint)uVar1;
//	uVar5 = dragon_ini_pointer[dragon_ini_const__2 + -1].field_0x1c;
	if (flickerINI == NULL) {
		//LAB_80027d40:
//		if ((flickerINI->sceneId == currentSceneId)
//			&& (uVar5 != 0xffff)) {
//			actors[(uint)uVar5]._sequenceID = 8;
//			actors[(uint)uVar5]._priorityLayer_maybe = 0;
//		}
	}
	else {
		if (flickerINI->sceneId == currentSceneId) {
			if ((flickerINI == NULL) || flickerINI->actor->isFlagSet(ACTOR_FLAG_10)) {
				if ((flickerINI->sceneId == currentSceneId)
					&& (uVar5 != 0xffff)) {
//					actors[(uint)uVar5]._sequenceID = 8;
//					actors[(uint)uVar5]._priorityLayer_maybe = 0;
				}
			} else {
				if ((_bit_flags_8006fbd8 & 2) == 0) {
					_bit_flags_8006fbd8 = _bit_flags_8006fbd8 | 2;
				}
//				if (((((actors[actorId]._flags & 0x2000) == 0) && ((actors[actorId]._flags & 4) != 0)) &&
//					 (actors[actorId]._sequenceID2 != actors[actorId]._sequenceID)) &&
//				(actors[actorId]._sequenceID2 != -1)) {
//					actor_update_sequenceID(actorId,actors[actorId]._sequenceID2);
//				}
			}

		} else {
			//actors[(uint)uVar5]._priorityLayer_maybe = 0;
		}

	}


	LAB_80027db4:
	uVar1 = num_ini_records_maybe;
	pDVar3 = dragon_ini_pointer;
	uVar2 = currentSceneId;
	if ((inventoryType == 0) && (uVar5 = 0, num_ini_records_maybe != 0)) {
		actorId = 0;
		do {
			pDVar4 = pDVar3 + actorId;
			if (((-1 < (int)((uint)pDVar4->field_0x10 << 0x10)) && (pDVar4->sceneId_maybe == uVar2)) &&
				(pDVar4->field_0x10 = pDVar4->field_0x10 - 1, (int)((uint)pDVar4->field_0x10 << 0x10) < 0))
			{
				pDVar4->field_1a_flags_maybe = pDVar4->field_1a_flags_maybe | 0x10;
			}
			uVar5 = uVar5 + 1;
			actorId = (uint)uVar5;
		} while (uVar5 < uVar1);
	}
	if (_run_func_ptr_unk_countdown_timer != 0) {
		_run_func_ptr_unk_countdown_timer = _run_func_ptr_unk_countdown_timer - 1;
	}
	return (uint)_run_func_ptr_unk_countdown_timer;
} */

//TODO the logic in this function doesn't match the original. It should be redone.
void DragonsEngine::engineFlag0x20UpdateFunction() {
	if (_flags & ENGINE_FLAG_20) {
		if ((_flags & (ENGINE_FLAG_80000000 | Dragons::ENGINE_FLAG_8)) == 8) {
			_cursor->update();
		}
		//TODO 0x80027be4

		uint16 currentSceneId = _scene->getSceneId();
		DragonINI *flickerINI = _dragonINIResource->getFlickerRecord();


//	uVar1 = flickerINI->actorId;
//	actorId = (uint)uVar1;
//	uVar5 = dragon_ini_pointer[dragon_ini_const__2 + -1].field_0x1c;
		if (flickerINI == NULL) {
			//LAB_80027d40:
			//error("LAB_80027d40"); //TODO is this logic required?
//		if ((flickerINI->sceneId == currentSceneId)
//			&& (uVar5 != 0xffff)) {
//			actors[(uint)uVar5]._sequenceID = 8;
//			actors[(uint)uVar5]._priorityLayer_maybe = 0;
//		}
		}
		else {
			if (flickerINI->sceneId == currentSceneId) {
				if (flickerINI->actor->isFlagSet(ACTOR_FLAG_10)) {
					if (_inventory->isActorSet()) {
						_inventory->setActorSequenceId(8);
						_inventory->setPriority(0);
					}
				} else {
					if ((_bit_flags_8006fbd8 & 2) == 0) {
						_bit_flags_8006fbd8 = _bit_flags_8006fbd8 | 2;
					}
					if (flickerINI->actor->isFlagClear(ACTOR_FLAG_2000)
					&& flickerINI->actor->isFlagSet(ACTOR_FLAG_4)
					&& flickerINI->actor->_sequenceID2 != -1
					&& flickerINI->actor->_sequenceID2 != flickerINI->actor->_sequenceID) {
						flickerINI->actor->updateSequence(flickerINI->actor->_sequenceID2);
					}
				}
			} else {
				_inventory->setPriority(0); //TODO I don't think this is quite right.
			}

		}

		// 0x80027db8
		if (!_inventory->isVisible()) {
			for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
				DragonINI *ini = getINI(i);
				if (ini->field_10 >= 0 && ini->sceneId == currentSceneId) {
					ini->field_10--;
					if (ini->field_10 < 0) {
						ini->field_1a_flags_maybe |= INI_FLAG_10;
					}
				}
			}
		}

		if (_run_func_ptr_unk_countdown_timer != 0) {
			_run_func_ptr_unk_countdown_timer--;
		}
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
	local_58_code = NULL;
	local_58_codeEnd = NULL;

	uVar6 = 0;
	_scriptOpcodes->_data_80071f5c = 0;

	assert(_cursor->_data_80072890 > 0);
	byte *obd = _dragonOBD->getFromOpt(_cursor->_data_80072890 - 1);


	ScriptOpCall local_48(obd + 8, READ_LE_UINT32(obd));
	pvVar7 = local_48._code;
	pvVar8 = local_48._codeEnd;

	uVar4 = _cursor->executeScript(local_48, 1);
	if (_cursor->_data_800728b0_cursor_seqID > 4) {
		_scriptOpcodes->_data_80071f5c = 0;

		obd = _dragonOBD->getFromOpt(_scriptOpcodes->_scriptTargetINI - 1);
		_scriptOpcodes->_scriptTargetINI = _cursor->_data_80072890;

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
	if (local_58_code != NULL && local_58_codeEnd != NULL) {
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
	}
	else {
		_scriptOpcodes->_data_80071f5c--;
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

void DragonsEngine::FUN_80038890() {
	error("FUN_80038890"); //TODO
}

void DragonsEngine::walkFlickerToObject()
{
	uint16 targetX;
	uint16 targetY;
	uint uVar7;
	uint uVar8;
	DragonINI *targetINI;
	DragonINI *flickerINI;

	flickerINI = _dragonINIResource->getFlickerRecord();
	if (flickerINI->sceneId == getCurrentSceneId()) {
		if (_cursor->_data_80072890 != 0) {

			if (!(READ_LE_UINT16(_dragonOBD->getFromOpt(_cursor->_data_80072890 - 1) + 4) & 8)
			&& (_inventory->getType() == 0) && !isFlagSet(ENGINE_FLAG_200000)) {
				targetINI = getINI(_cursor->_data_80072890 - 1);
				if ((targetINI->field_1a_flags_maybe & 1) == 0) {
					if (targetINI->actorResourceId == -1) {
						return;
					}
					Img *img = _dragonImg->getImg(targetINI->field_2);
					targetX = img->field_a;
					targetY = img->field_c;
				}
				else {
					targetX = targetINI->actor->_x_pos;
					targetY = targetINI->actor->_y_pos;
				}
				flickerINI->actor->_walkSpeed = 0x10000;
				if (flickerINI->field_20_actor_field_14 == -1) {
					flickerINI->actor->setFlag(ACTOR_FLAG_800);
				}
				flickerINI->actor->startWalk((int)(((uint)targetX + (uint)targetINI->field_1c) * 0x10000) >> 0x10,
													(int)(((uint)targetY + (uint)targetINI->field_1e) * 0x10000) >> 0x10,0);
				_bit_flags_8006fbd8 = 1;
				return;
			}
			if (isFlagSet(ENGINE_FLAG_200000)) {
				_bit_flags_8006fbd8 = 3;
				return;
			}
			flickerINI = _dragonINIResource->getFlickerRecord();
			if (flickerINI != NULL && flickerINI->actor != NULL) {
				flickerINI->actor->clearFlag(ACTOR_FLAG_10);
				flickerINI->actor->setFlag(ACTOR_FLAG_4);
				targetINI = getINI(_cursor->_data_80072890 - 1);
				flickerINI->field_20_actor_field_14 = targetINI->field_e;
				flickerINI->actor->_sequenceID2 = targetINI->field_e;
			}
			_bit_flags_8006fbd8 = 3;
			return;
		}
		if (_inventory->getType() == 0 && !isFlagSet(ENGINE_FLAG_200000)) {
			uVar7 = (uint)(uint16)_cursor->_x;
			uVar8 = (uint)(uint16)_cursor->_y;
			flickerINI->actor->_walkSpeed = 0x10000;
			flickerINI->actor->startWalk(
					(int)((uVar7 + (uint)_scene->_camera.x) * 0x10000) >> 0x10,
					(int)((uVar8 + (uint)_scene->_camera.y) * 0x10000) >> 0x10,0);
		}
	}
	else {
		if (_cursor->_data_80072890 != 0) {
			_bit_flags_8006fbd8 = 3;
			return;
		}
	}
	_bit_flags_8006fbd8 = 0;
	return;
}

void DragonsEngine::FUN_80038994() {
	error("FUN_80038994"); //TODO
}

void DragonsEngine::reset_screen_maybe() {
	_videoFlags &= ~0x10;
	//TODO
}

bool DragonsEngine::canLoadGameStateCurrently() {
	return isInputEnabled();
}

bool DragonsEngine::canSaveGameStateCurrently() {
	return isInputEnabled() && _inventory->getType() != 1;
}

bool DragonsEngine::hasFeature(Engine::EngineFeature f) const {
	return
		// TODO (f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void DragonsEngine::loadScene(uint16 sceneId) {
	_flags = 0x1046;
	_flags &= 0x1c07040;
	_flags |= 0x26;
	_unkFlags1 = 0;

	clearFlags(ENGINE_FLAG_1000_TEXT_ENABLED); //TODO wire this up to subtitle config.

	_scriptOpcodes->_scriptTargetINI = 0; //TODO this should be reset in scriptopcode.
	_cursor->init(_actorManager, _dragonINIResource);
	_inventory->init(_actorManager, _backgroundResourceLoader, new Bag(_bigfileArchive, _screen), _dragonINIResource);
	_talk->init();

	_screen->loadPalette(1, _cursor->getPalette());
	setupPalette1();

	_screen->loadPalette(2, _cursor->getPalette());
	_screen->loadPalette(4, _cursor->getPalette());
	_screen->updatePaletteTransparency(4, 1, 0xff, true);

	// TODO FUN_80017010_update_actor_texture_maybe();
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
	_data_800633fa = 0;

	for (int i = 0; i < 8; i++) {
		_paletteCyclingTbl[i].paletteType = 0;
		_paletteCyclingTbl[i].startOffset = 0;
		_paletteCyclingTbl[i].endOffset = 0;
		_paletteCyclingTbl[i].updateInterval = 0;
		_paletteCyclingTbl[i].updateCounter = 0;
	}

	setSceneUpdateFunction(NULL);
}

void DragonsEngine::runSceneUpdaterFunction() {
	if ((isFlagSet(ENGINE_FLAG_20) && (_run_func_ptr_unk_countdown_timer == 0)) &&
		(_run_func_ptr_unk_countdown_timer = 1, _sceneUpdateFunction != NULL)) {
		_sceneUpdateFunction();
	}
}

void DragonsEngine::setSceneUpdateFunction(void (*newUpdateFunction)()) {
	_sceneUpdateFunction = newUpdateFunction;
}

void DragonsEngine::setVsyncUpdateFunction(void (*newUpdateFunction)()) {
	_vsyncUpdateFunction = newUpdateFunction;
}

void DragonsEngine::seedRandom(int32 seed) {
		_randomState = seed * -0x2b0e2b0f;
}

uint32 DragonsEngine::shuffleRandState()
{
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
	_screen->updatePaletteTransparency(1,0x40,0x7f,true);
}

bool DragonsEngine::isDebugMode() {
	return _debugMode;
}

bool DragonsEngine::isVsyncUpdaterFunctionRunning() {
	return _vsyncUpdateFunction != NULL;
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
					}
					else {
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
				}
				else {
					_paletteCyclingTbl[loopIndex].updateCounter = _paletteCyclingTbl[loopIndex].updateCounter + -1;
				}
			}
		}
	}
}

void (*DragonsEngine::getSceneUpdateFunction())() {
	return _sceneUpdateFunction;
}

} // End of namespace Dragons
