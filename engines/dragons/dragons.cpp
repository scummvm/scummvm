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
#include "engines/util.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "common/error.h"
#include "actor.h"
#include "actorresource.h"
#include "background.h"
#include "bigfile.h"
#include "cursor.h"
#include "dragonflg.h"
#include "dragonimg.h"
#include "dragonini.h"
#include "dragonobd.h"
#include "dragonrms.h"
#include "dragonvar.h"
#include "dragons.h"
#include "inventory.h"
#include "scene.h"
#include "screen.h"
#include "sequenceopcodes.h"
#include "scriptopcodes.h"

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
	_nextUpdatetime = 0;
	_flags = 0;
	_unkFlags1 = 0;
	_sequenceOpcodes = new SequenceOpcodes(this);
	_scriptOpcodes = NULL;
	_engine = this;
	run_func_ptr_unk_countdown_timer = 0;
	data_8006a3a0_flag = 0;
	data_800633fa = 0;
	data_8006f3a8 = 0;
	_data_either_5_or_0 = 0;
	_inventory = new Inventory(this);
	_cursor = new Cursor(this);

	_leftMouseButtonUp = false;
	_rightMouseButtonUp = false;
}

DragonsEngine::~DragonsEngine() {
	delete _sequenceOpcodes;
	delete _scriptOpcodes;
}

void DragonsEngine::updateEvents() {
	Common::Event event;
	_leftMouseButtonUp = false;
	_rightMouseButtonUp = false;
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
				break;
			case Common::EVENT_RBUTTONUP:
				_rightMouseButtonUp = true;
				break;
			default:
				break;
		}
	}
}

Common::Error DragonsEngine::run() {
	_screen = new Screen();
	_bigfileArchive = new BigfileArchive("bigfile.dat", Common::Language::EN_ANY);
	_dragonFLG = new DragonFLG(_bigfileArchive);
	_dragonIMG = new DragonIMG(_bigfileArchive);
	_dragonOBD = new DragonOBD(_bigfileArchive);
	_dragonRMS = new DragonRMS(_bigfileArchive, _dragonOBD);
	_dragonVAR = new DragonVAR(_bigfileArchive);
	_dragonINIResource = new DragonINIResource(_bigfileArchive);
	ActorResourceLoader *actorResourceLoader = new ActorResourceLoader(_bigfileArchive);
	_actorManager = new ActorManager(actorResourceLoader);
	_scriptOpcodes = new ScriptOpcodes(this, _dragonFLG);
	_scene = new Scene(this, _screen, _scriptOpcodes, _bigfileArchive, _actorManager, _dragonRMS, _dragonINIResource);
	_flags = 0x1046;
	_flags &= 0x1c07040;
	_flags |= 0x26;

	_cursor->init(_actorManager, _dragonINIResource);
	_inventory->init(_actorManager);

	uint16 sceneId = 0x12;
	_dragonINIResource->getFlickerRecord()->sceneId = sceneId; //TODO
	_sceneId1 = sceneId;
	_scene->loadScene(sceneId, 0x1e);

	_scene->draw();
	_screen->updateScreen();

	gameLoop();

	delete _scene;
	delete _actorManager;
	delete _backgroundResourceLoader;
	delete _dragonFLG;
	delete _dragonIMG;
	delete _dragonRMS;
	delete _dragonVAR;
	delete _bigfileArchive;
	delete _screen;

	debug("Ok");
	return Common::kNoError;
}

void DragonsEngine::gameLoop() {
	_counter = 0;
	bit_flags_8006fbd8 = 0;
	while (!shouldQuit()) {
		updateHandler();
		updateEvents();

		if (getCurrentSceneId() != 2) {
			_sceneId1 = getCurrentSceneId();
		}

		_counter++;
		DragonINI *flickerIni = _dragonINIResource->getFlickerRecord();
		if (_counter >= 1200 && flickerIni->actor->resourceID == 0xe) { // 0xe == flicker.act
			Actor *actor = flickerIni->actor;
			actor->_sequenceID2 = 2;
			flickerIni->field_20_actor_field_14 = 2;

			actor->updateSequence(getINI(0xc2)->sceneId == 1 ? 0x30 : 2);
			_counter = 0;
			setFlags(Dragons::ENGINE_FLAG_80000000);
		}

		if (_flags & Dragons::ENGINE_FLAG_80000000) {
			if (flickerIni->actor->flags & Dragons::ACTOR_FLAG_4) {
				_counter = 0;
				clearFlags(Dragons::ENGINE_FLAG_80000000);
			}
		}

		if (bit_flags_8006fbd8 == 0) {
			setFlags(Dragons::ENGINE_FLAG_8);
		}

		if (flickerIni->sceneId == getCurrentSceneId()) {
			uint16 id = getIniFromImg();
			if (id != 0) {
				// 0x80026cac
				error("todo 0x80026cac run script");
			} else {
				// 0x80026d34
				// $s4_1 = 0;
			}
		} else {
			// 0x80026d34
			// $s4_1 = 0;
		}

		// 0x80026d38
		_cursor->updateINIUnderCursor();

		if (_rightMouseButtonUp && !isFlagSet(ENGINE_FLAG_20000000) && !isFlagSet(ENGINE_FLAG_400)) {
			_cursor->selectPreviousCursor();
		}

		// Action input
		if (_leftMouseButtonUp && !isFlagSet(ENGINE_FLAG_20000000) && !isFlagSet(ENGINE_FLAG_400)) {
			//TODO
		}

		runINIScripts();

		_scene->draw();
		_screen->updateScreen();
		wait();
	}
}



/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void DragonsEngine::game_loop()

{
	/**
	bool bVar1;
	DragonINI *pDVar2;
	uint uVar3;
	uint32_t uVar4;
	uint actorId;
	int iVar5;
	ushort uVar6;
	ushort uVar7;
	uint actorId_00;
	void *buffer;
	uint16_t sequenceId;
	DragonINI *pDVar8;
	ushort *puVar9;
	ScriptOpCall local_30;

	_data_either_5_or_0 = 0;
	bit_flags_8006fbd8 = 0;
	_counter = 0;
	setFlags(ENGINE_FLAG_8);
	actorId = 0;

	while(!shouldQuit()) {
		LAB_80026a74:
		wait();
		pDVar2 = dragon_ini_pointer;
		iVar5 = data_80063940_const_c3_maybe;
		if ((uint)currentSceneId != _const_value_2) {
			sceneId_1 = currentSceneId;
		}
		_counter++;
		if (0x4af < _counter) {
			pDVar8 = _dragonINIResource->getFlickerRecord(); //dragon_ini_pointer + (uint)dragon_ini_maybe_flicker_control;
			if (pDVar8->actor->resourceID == 0xe) {
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
		if (bit_flags_8006fbd8 == 0) {
			setFlags(ENGINE_FLAG_8);
		}
		if (dragon_ini_pointer[(uint)dragon_ini_maybe_flicker_control].sceneId_maybe == currentSceneId) {
			uVar3 = ipt_img_file_related();
			actorId_00 = uVar3 & 0xffff;
			if (actorId_00 == 0) goto LAB_80026d34;
			if (actorId_00 != (actorId & 0xffff)) {
				buffer = (void *)((int)dragon_Obd_Offset + dragon_Opt_Offset[actorId_00 - 1].obdOffset);
				local_30.code = (void *)((int)buffer + 8);
				uVar4 = read_int32(buffer);
				local_30.codeEnd = (void *)(uVar4 + (int)local_30.code);
				actorId = run_script_field8_eq_4(&local_30);
				if ((actorId & 0xffff) != 0) {
					local_30.codeEnd =
							(void *)((uint)*(ushort *)((int)local_30.code + 2) +
									 (int)(void *)((int)local_30.code + 4));
					local_30.code = (void *)((int)local_30.code + 4);
					run_script(&local_30);
					_counter = 0;
				}
			}
		}
		else {
			LAB_80026d34:
			uVar3 = 0;
		}
		actorId = updateINIUnderCursor();
		if (((actorId & 0xffff) == 0) ||
			((dragon_ini_pointer[(uint)_cursor->_iniUnderCursor].actorId & 0x4000) != 0)) {
			_data_either_5_or_0 = 0;
		}
		else {
			_data_either_5_or_0 = 5;
		}
		actorId = CheckButtonMapPress_CycleDown(0);
		if (((actorId & 0xffff) != 0) && ((engine_flags_maybe & 0x20000400) == 0)) {
			uVar6 = _cursor->_sequenceID - 1;
			if ((uVar6 == 0) && ((_inventory->getType() == 1 || (_inventory->getType() == 2)))) {
				uVar6 = _cursor->_sequenceID - 2;
			}
			_cursor->_sequenceID = uVar6;
			if ((_cursor->_sequenceID == 3) && (_inventory->getType() == 1)) {
				_cursor->_sequenceID = 1;
			}
			if (_cursor->_sequenceID == 2) {
				_cursor->_sequenceID = 1;
			}
			if (_cursor->_sequenceID == 0xffff) {
				uVar6 = 5;
				if (DAT_8006f3a8 == 0) {
					uVar6 = 4;
				}
				LAB_80026fb0:
				_cursor->_sequenceID = uVar6;
			}
			_counter = 0;
			actorId = uVar3;
			continue;
		}
		actorId = CheckButtonMapPress_CycleUp(0);
		if (((actorId & 0xffff) != 0) && ((engine_flags_maybe & 0x20000400) == 0)) {
			_cursor->_sequenceID = _cursor->_sequenceID + 1;
			if (DAT_8006f3a8 == 0) {
				bVar1 = _cursor->_sequenceID < 5;
			}
			else {
				bVar1 = _cursor->_sequenceID < 6;
			}
			if (!bVar1) {
				_cursor->_sequenceID = 0;
			}
			if ((_cursor->_sequenceID == 0) && ((_inventory->getType() == 1 || (_inventory->getType() == 2)))) {
				_cursor->_sequenceID = 1;
			}
			if (_cursor->_sequenceID == 2) {
				if (_inventory->getType() == 1) {
					_cursor->_sequenceID = 4;
				}
				uVar6 = 3;
				if (_cursor->_sequenceID == 2) goto LAB_80026fb0;
			}
			_counter = 0;
			actorId = uVar3;
			continue;
		}
		if (bit_flags_8006fbd8 == 3) {
			bit_flags_8006fbd8 = 0;
			if ((dragon_ini_pointer[(uint)dragon_ini_maybe_flicker_control].sceneId_maybe == currentSceneId)
				&& (actorId = (uint)dragon_ini_pointer[(uint)dragon_ini_maybe_flicker_control].actorId,
					actors[actorId].﻿_sequenceID2 != 0xffff)) {
				uVar6 = DAT_800728c0;
				if (_cursor->_sequenceID != 5) {
					uVar6 = DAT_80072890;
				}
				actors[actorId].﻿_sequenceID2 = dragon_ini_pointer[(uint)uVar6 - 1].field_0x20;
			}
			works_with_obd_data_1();
			if (((uint)currentSceneId == DAT_80063a70) && (dragon_ini_pointer[DAT_80063b0c].field_0x2 != 0))
			{
				engine_flags_maybe = engine_flags_maybe & 0xfffffff7;
			}
			else {
				engine_flags_maybe = engine_flags_maybe | 8;
			}
			_counter = 0;
			actorId = uVar3;
			continue;
		}
		if (_inventory->getType() != 1) {
			if (_inventory->getType() < 2) {
				if (_inventory->getType() == 0) {
					actorId = CheckButtonMapPress_InventoryBag(0);
					if ((((actorId & 0xffff) != 0) && ((engine_flags_maybe & 0x20000400) == 0)) &&
						((bit_flags_8006fbd8 & 3) != 1)) {
						sequenceId = dragon_Var_Offset[dragon_var_index_const_7];
						uVar7 = inventory_old_showing_value;
						inventory_old_showing_value = _inventory->getType();
						joined_r0x800271d0:
						_inventory->setType(inventory_old_showing_value);
						if (sequenceId == 1) {
							LAB_800279f4:
							inventory_old_showing_value = uVar7;
							FUN_8003130c();
							actorId = uVar3;
						}
						else {
							_counter = 0;
							_inventory->setType(1);
							actor_related_80030e88();
							joined_r0x80027a38:
							if (DAT_8006f3a8 == 0) {
								_cursor->_sequenceID = 1;
								actorId = uVar3;
							}
							else {
								_cursor->_sequenceID = 5;
								actorId = uVar3;
							}
						}
						goto LAB_80026a74;
					}
					actorId = CheckButtonMapPress_Action(0);
					uVar6 = _inventory->getType();
					if (((actorId & 0xffff) != 0) && ((engine_flags_maybe & 8) != 0)) {
						_counter = 0;
						if ((_cursor->_iniUnderCursor & 0x8000) != 0) {
							if (_cursor->_iniUnderCursor == 0x8002) {
								LAB_80027294:
								uVar7 = 0;
								if (DAT_8006f3a8 == 0) {
									if ((bit_flags_8006fbd8 & 3) != 1) {
										sequenceId = dragon_Var_Offset[dragon_var_index_const_7];
										uVar7 = inventory_old_showing_value;
										inventory_old_showing_value = _inventory->getType();
										goto joined_r0x800271d0;
									}
								}
								else {
									actorId = 0;
									do {
										if (unkArray_uint16[actorId] == 0) break;
										uVar7 = uVar7 + 1;
										actorId = (uint)uVar7;
									} while (uVar7 < 0x29);
									if (uVar7 < 0x29) {
										_cursor->_sequenceID = 1;
										ContinueGame?();
										uVar6 = DAT_8006f3a8;
										DAT_8006f3a8 = 0;
										_cursor->_iniUnderCursor = 0;
										unkArray_uint16[(uint)uVar7] = uVar6;
										actorId = uVar3;
										goto LAB_80026a74;
									}
								}
							}
							else {
								if (_cursor->_iniUnderCursor != 0x8001) goto LAB_80027ab4;
								if (inventorySequenceId == 0) goto LAB_80027294;
							}
							if ((_cursor->_iniUnderCursor == 0x8001) && (inventorySequenceId == 1)) {
								_inventory->setType(2);
								inventory_old_showing_value = uVar6;
								FUN_80038890();
								actorId = uVar3;
								goto LAB_80026a74;
							}
						}
						LAB_80027ab4:
						_counter = 0;
						DAT_80072890 = _cursor->_iniUnderCursor;
						if (_cursor->_sequenceID < 5) {
							DATA_800728b0_cursor_seqID = _cursor->_sequenceID;
							FUN_8002837c();
							if (bit_flags_8006fbd8 != 0) {
								engine_flags_maybe = engine_flags_maybe & 0xfffffff7;
							}
						}
						else {
							DATA_800728b0_cursor_seqID = _cursor->_sequenceID;
							FUN_8002837c();
							if (bit_flags_8006fbd8 != 0) {
								engine_flags_maybe = engine_flags_maybe & 0xfffffff7;
							}
							DAT_800728c0 = DAT_80072890;
							DAT_80072890 = DAT_8006f3a8;
						}
					}
				}
			}
			else {
				if (_inventory->getType() == 2) {
					actorId = CheckButtonMapPress_InventoryBag(0);
					uVar6 = _inventory->getType();
					if (((actorId & 0xffff) != 0) && ((engine_flags_maybe & 0x20000400) == 0)) {
						uVar7 = inventory_old_showing_value;
						if (dragon_Var_Offset[dragon_var_index_const_7] == 1) goto LAB_800279f4;
						_counter = 0;
						_inventory->setType(1);
						inventory_old_showing_value = uVar6;
						actor_related_80030e88();
						goto joined_r0x80027a38;
					}
					actorId = CheckButtonMapPress_Action(0);
					if (((actorId & 0xffff) != 0) && ((engine_flags_maybe & 8) != 0)) goto LAB_80027ab4;
				}
			}
			LAB_80027b58:
			run_ini_scripts();
			actorId = uVar3;
			goto LAB_80026a74;
		}
		actorId = CheckButtonMapPress_InventoryBag(0);
		if ((actorId & 0xffff) != 0) {
			_counter = 0;
			LAB_80027970:
			FUN_80031480();
			uVar6 = inventory_old_showing_value;
			inventory_old_showing_value = _inventory->getType();
			actorId = uVar3;
			_inventory->setType(uVar6);
			goto LAB_80026a74;
		}
		actorId = CheckButtonMapPress_Action(0);
		uVar6 = _inventory->getType();
		if (((actorId & 0xffff) != 0) && ((engine_flags_maybe & 8) != 0)) {
			_counter = 0;
			if ((_cursor->_iniUnderCursor & 0x8000) != 0) {
				if (_cursor->_iniUnderCursor == 0x8001) {
					FUN_80031480();
					_inventory->setType(0);
					if (inventory_old_showing_value == 2) {
						FUN_80038994();
					}
				}
				else {
					if (_cursor->_iniUnderCursor != 0x8002) goto LAB_8002790c;
					FUN_80031480();
					_inventory->setType(2);
					if (inventory_old_showing_value != 2) {
						FUN_80038890();
					}
				}
				inventory_old_showing_value = uVar6;
				actorId = uVar3;
				goto LAB_80026a74;
			}
			if (_cursor->_iniUnderCursor != 0) {
				actorId_00 = 0;
				if ((_cursor->_sequenceID != 4) && (_cursor->_sequenceID != 2)) {
					DATA_800728b0_cursor_seqID = _cursor->_sequenceID;
					DAT_80072890 = _cursor->_iniUnderCursor;
					if (4 < _cursor->_sequenceID) {
						DAT_80072890 = DAT_8006f3a8;
						DAT_800728c0 = _cursor->_iniUnderCursor;
					}
					engine_flags_maybe = engine_flags_maybe & 0xfffffff7;
					FUN_8002837c();
					goto LAB_8002790c;
				}
				if (_cursor->_iniUnderCursor != unkArray_uint16[0]) {
					actorId = 1;
					do {
						actorId_00 = actorId;
						actorId = actorId_00 + 1;
					} while (_cursor->_iniUnderCursor != unkArray_uint16[actorId_00 & 0xffff]);
				}
				puVar9 = unkArray_uint16 + (actorId_00 & 0xffff);
				iVar5 = (actorId_00 & 0xffff) + 0x17;
				*puVar9 = DAT_8006f3a8;
				DAT_8007283c = actors[iVar5].﻿_sequenceID;
				actors[iVar5].flags = actors[iVar5].flags & 0xffbf;
				DAT_8006f3a8 = _cursor->_iniUnderCursor;
				_cursor->_sequenceID = 5;
				actorId = uVar3;
				if (*puVar9 != 0) {
					actorId = actorId_00 + 0x17 & 0xffff;
					actors[actorId].flags = 0;
					actors[actorId].﻿priorityLayer_maybe = 0;
					actors[actorId].field_0xe = 0x100;
					actor_update_sequenceID
							(actorId,dragon_ini_pointer[(uint)*puVar9 - 1].field_1a_flags_maybe * 2 + 10);
					actors[actorId].flags = actors[actorId].flags | 0x3c0;
					actors[actorId].﻿priorityLayer_maybe = 6;
					actorId = uVar3;
				}
				goto LAB_80026a74;
			}
			uVar6 = 0;
			if (DAT_8006f3a8 == 0) goto LAB_80027b58;
			actorId = 0;
			do {
				iVar5 = actorId + 0x17;
				if (((((int)(short)actors[iVar5].x_pos + -0x10 <= (int)cursor_x_var) &&
					  ((int)cursor_x_var < (int)(short)actors[iVar5].x_pos + 0x10)) &&
					 ((int)(short)actors[iVar5].y_pos + -0xc <= (int)cursor_y_var)) &&
					(actorId = (uint)uVar6, (int)cursor_y_var < (int)(short)actors[iVar5].y_pos + 0xc)) break;
				uVar6 = uVar6 + 1;
				actorId = (uint)uVar6;
			} while (uVar6 < 0x29);
			if (actorId != 0x29) {
				actorId_00 = (uint)(ushort)(uVar6 + 0x17);
				unkArray_uint16[actorId] = DAT_8006f3a8;
				actors[actorId_00].flags = 0;
				actors[actorId_00].﻿priorityLayer_maybe = 0;
				actors[actorId_00].field_0xe = 0x100;
				DAT_8006f3a8 = 0;
				actor_update_sequenceID
						(actorId_00,
						 dragon_ini_pointer[(uint)unkArray_uint16[actorId] - 1].field_1a_flags_maybe * 2 +
						 10);
				uVar6 = _cursor->_sequenceID;
				actors[actorId_00].flags = actors[actorId_00].flags | 0x3c0;
				actors[actorId_00].﻿priorityLayer_maybe = 6;
				if (uVar6 == 5) {
					_cursor->_sequenceID = 4;
				}
			}
		}
		LAB_8002790c:
		if ((DAT_8006f3a8 == 0) ||
			(((ushort)(cursor_x_var - 10U) < 300 && ((ushort)(cursor_y_var - 10U) < 0xb4))))
			goto LAB_80027b58;
		_cursor->_sequenceID = 5;
		delayFunction(2);
		goto LAB_80027970;
	}
	 **/
}


void DragonsEngine::updateHandler() {
	data_8006a3a0_flag |= 0x40;
	//TODO logic here

	updateActorSequences();

	_cursor->updateVisibility();
	_inventory->updateVisibility();

	//TODO logic here
	for (uint16 i = 0; i < 0x17; i++) {
		Actor *actor = _actorManager->getActor(i);
		if (actor->flags & Dragons::ACTOR_FLAG_40) {
			if (!(actor->flags & Dragons::ACTOR_FLAG_100)) {
				int16 priority = _scene->getPriorityAtPosition(Common::Point(actor->x_pos, actor->y_pos));
				DragonINI *flicker = _dragonINIResource->getFlickerRecord();
				if (flicker && _scene->contains(flicker) && flicker->actor->_actorID == i) {
					if (priority < 8 || priority == 0x10) {
						actor->priorityLayer = priority;
					}
				} else {
					if (priority != -1) {
						actor->priorityLayer = priority;
					}
				}

				if (actor->priorityLayer >= 0x11) {
					actor->priorityLayer = 0;
				}

				if (actor->priorityLayer >= 9) {
					actor->priorityLayer -= 8;
				}
			}

			if (actor->sequenceTimer != 0) {
				actor->sequenceTimer--;
			}
		}
	}

	if (_flags & Dragons::ENGINE_FLAG_80) {
		for (uint16 i = 0x17; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
			Actor *actor = _actorManager->getActor(i);
			if (actor->sequenceTimer != 0) {
				actor->sequenceTimer--;
			}
		}
	}

	if (isFlagSet(ENGINE_FLAG_4)) {
		updatePathfindingActors();
	}

	// TODO 0x8001bed0

	// 0x8001c294
	if (!(_unkFlags1 & ENGINE_UNK1_FLAG_8)) {
		//TODO ReadPad();
	}

	if (isFlagSet(ENGINE_FLAG_20)) {
		engineFlag0x20UpdateFunction();
	}

	//TODO vsync update function

	// TODO data_8006a3a0 logic. @ 0x8001c2f4

	data_8006a3a0_flag &= ~0x40;
}

const char *DragonsEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String DragonsEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);
	return Common::String::format("%s.%03d", target.c_str(), num);
}

#define DRAGONS_SAVEGAME_VERSION 0

kReadSaveHeaderError
DragonsEngine::readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail) {

	header.version = in->readUint32LE();
	if (header.version > DRAGONS_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--) {
		header.description += (char) in->readByte();
	}

	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		return kRSHEIoError;
	}

	header.flags = in->readUint32LE();

	header.saveDate = in->readUint32LE();
	header.saveTime = in->readUint32LE();
	header.playTime = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
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
	if (!(_flags & Dragons::ENGINE_FLAG_4)) {
		return;
	}

	//TODO ResetRCnt(0xf2000001);

	int16 actorId = _flags & Dragons::ENGINE_FLAG_80 ? (int16) 64 : (int16) 23;

	while (actorId > 0) {
		actorId--;
		Actor *actor = _actorManager->getActor((uint16) actorId);
		if (actorId < 2 && _flags & Dragons::ENGINE_FLAG_40) {
			continue;
		}

		if (actor->flags & Dragons::ACTOR_FLAG_40 &&
			!(actor->flags & Dragons::ACTOR_FLAG_4) &&
			!(actor->flags & Dragons::ACTOR_FLAG_400) &&
			(actor->sequenceTimer == 0 || actor->flags & Dragons::ACTOR_FLAG_1)) {
			debug("Actor[%d] execute sequenceOp", actorId);

			if (actor->flags & Dragons::ACTOR_FLAG_1) {
				actor->resetSequenceIP();
				actor->flags &= 0xeff6; //TODO rewrite using ACTOR_FLAG_nnn
				actor->field_7a = 0;
			}
			//TODO execute sequence Opcode here.
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

uint16 DragonsEngine::getCurrentSceneId() {
	return _scene->getSceneId();
}

void DragonsEngine::setVar(uint16 offset, uint16 value) {
	return _dragonVAR->setVar(offset, value);
}

uint16 DragonsEngine::getIniFromImg() {
	DragonINI *flicker = _dragonINIResource->getFlickerRecord();

	int16 x = flicker->actor->x_pos / 32;
	int16 y = flicker->actor->y_pos / 8;

	uint16 currentSceneId = _scene->getSceneId();

	for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if (ini->sceneId == currentSceneId && ini->field_1a_flags_maybe == 0) {
			IMG *img = _dragonIMG->getIMG(ini->field_2);
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

uint16 DragonsEngine::updateINIUnderCursor() {
	int32 x = (_cursorPosition.x + _scene->_camera.x) / 32;
	int32 y = (_cursorPosition.y + _scene->_camera.y) / 8;

	if (_flags & Dragons::ENGINE_FLAG_10) {

		if (_inventory->getSequenceId() == 0 || _inventory->getSequenceId() == 2) {
//TODO
		} else {

		}
	}

	return 0;
}

void DragonsEngine::runINIScripts() {
	for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = getINI(i);
		if (ini->field_1a_flags_maybe & Dragons::INI_FLAG_10) {
			ini->field_1a_flags_maybe &= ~Dragons::INI_FLAG_10;
			byte *data = _dragonOBD->getFromOpt(i);
			ScriptOpCall scriptOpCall;
			scriptOpCall._code = data + 8;
			scriptOpCall._codeEnd = scriptOpCall._code + READ_LE_UINT32(data);
			uint32 currentFlags = _flags;
			clearFlags(Dragons::ENGINE_FLAG_8);
			_scriptOpcodes->runScript3(scriptOpCall);
			_flags = currentFlags;
		}
	}
}

void DragonsEngine::engineFlag0x20UpdateFunction() {
	if (_flags & Dragons::ENGINE_FLAG_20) {
		if ((_flags & (Dragons::ENGINE_FLAG_80000000 | Dragons::ENGINE_FLAG_8)) == 8) {
			_cursor->update();
		}
		//TODO 0x80027be4

		uint16 currentSceneId = _scene->getSceneId();

		// 0x80027db8
		if (!_inventory->isVisible()) {
			for (uint16 i = 0; i < _dragonINIResource->totalRecords(); i++) {
				DragonINI *ini = getINI(i);
				if (ini->field_10 >= 0 && ini->sceneId == currentSceneId) {
					ini->field_10--;
					if (ini->field_10 < 0) {
						ini->field_1a_flags_maybe |= Dragons::INI_FLAG_10;
					}
				}
			}
		}

		if (run_func_ptr_unk_countdown_timer != 0) {
			run_func_ptr_unk_countdown_timer--;
		}
	} else {
		run_func_ptr_unk_countdown_timer = 1;
	}
}

void DragonsEngine::waitForFrames(uint16 numFrames) {
	for (uint16 i = 0; i < numFrames; i++) {
		wait();
		updateHandler();

		_scene->draw();
		_screen->updateScreen();
		updateEvents();
	}
}

void DragonsEngine::playSound(uint16 soundId) {
	debug(3, "TODO: play sound %d", soundId);
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

} // End of namespace Dragons
