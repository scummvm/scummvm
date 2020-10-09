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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ngi/ngi.h"

#include "ngi/messages.h"
#include "ngi/statics.h"
#include "ngi/gameloader.h"
#include "ngi/interaction.h"
#include "ngi/motion.h"
#include "ngi/input.h"

#include "ngi/constants.h"

namespace NGI {

void global_messageHandler_KickStucco() {
	Movement *mov = g_nmi->_aniMan->getMovementById(MV_MAN_HMRKICK);
	int end = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();
	bool flip = false;

	for (int i = 0; i < end; i++) {
		ExCommand *ex = mov->getDynamicPhaseByIndex(i)->getExCommand();

		if (ex && ex->_messageKind == 35 && ex->_messageNum == SND_CMN_015) {
			if (flip) {
				ex->_messageNum = SND_CMN_055;
			} else {
				ex->_messageNum = SND_CMN_054;
				flip = true;
			}
		}
	}

	mov = g_nmi->_aniMan->getMovementById(MV_MAN_HMRKICK_COINLESS);
	end = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();
	flip = false;

	for (int i = 0; i < end; i++) {
		ExCommand *ex = mov->getDynamicPhaseByIndex(i)->getExCommand();

		if (ex && ex->_messageKind == 35 && ex->_messageNum == SND_CMN_015) {
			if (flip) {
				ex->_messageNum = SND_CMN_055;
			} else {
				ex->_messageNum = SND_CMN_054;
				flip = true;
			}
		}
	}
}

void global_messageHandler_KickMetal() {
	Movement *mov = g_nmi->_aniMan->getMovementById(MV_MAN_HMRKICK);
	int end = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();

	for (int i = 0; i < end; i++) {
		ExCommand *ex = mov->getDynamicPhaseByIndex(i)->getExCommand();

		if (ex && ex->_messageKind == 35) {
			if (ex->_messageNum == SND_CMN_054 || ex->_messageNum == SND_CMN_055)
				ex->_messageNum = SND_CMN_015;
		}
	}

	mov = g_nmi->_aniMan->getMovementById(MV_MAN_HMRKICK_COINLESS);
	end = mov->_currMovement ? mov->_currMovement->_dynamicPhases.size() : mov->_dynamicPhases.size();

	for (int i = 0; i < end; i++) {
		ExCommand *ex = mov->getDynamicPhaseByIndex(i)->getExCommand();

		if (ex && ex->_messageKind == 35) {
			if (ex->_messageNum == SND_CMN_054 || ex->_messageNum == SND_CMN_055)
				ex->_messageNum = SND_CMN_015;
		}
	}
}

int global_messageHandler1(ExCommand *cmd) {
	debugC(5, kDebugEvents, "global_messageHandler1: %d %d", cmd->_messageKind, cmd->_messageNum);

	if (cmd->_excFlags & 0x10000) {
		if (cmd->_messageNum == MV_MAN_TOLADDER)
			cmd->_messageNum = MV_MAN_TOLADDER2;
		if (cmd->_messageNum == MV_MAN_STARTLADDER)
			cmd->_messageNum = MV_MAN_STARTLADDER2;
		if (cmd->_messageNum == MV_MAN_GOLADDER)
			cmd->_messageNum = MV_MAN_GOLADDER2;
		if (cmd->_messageNum == MV_MAN_STOPLADDER)
			cmd->_messageNum = MV_MAN_STOPLADDER2;
	}

	if (g_nmi->_inputDisabled) {
		if (cmd->_messageKind == 17) {
			switch (cmd->_messageNum) {
			case 29:
			case 30:
			case 36:
			case 106:
				cmd->_messageKind = 0;
				break;
			default:
				break;
			}
		}
	} else if (cmd->_messageKind == 17) {
		switch (cmd->_messageNum) {
		case MSG_MANSHADOWSON:
			g_nmi->_aniMan->_shadowsOn = 1;
			break;
		case MSG_HMRKICK_STUCCO:
			global_messageHandler_KickStucco();
			break;
		case MSG_MANSHADOWSOFF:
			g_nmi->_aniMan->_shadowsOn = 0;
			break;
		case MSG_DISABLESAVES:
			g_nmi->disableSaves(cmd);
			break;
		case MSG_ENABLESAVES:
			g_nmi->enableSaves();
			break;
		case MSG_HMRKICK_METAL:
			global_messageHandler_KickMetal();
			break;
		case 29: // left mouse
			if (g_nmi->_inventoryScene) {
				if (getGameLoaderInventory()->handleLeftClick(cmd))
					cmd->_messageKind = 0;
			}
			break;
		case 107: // right mouse
			if (getGameLoaderInventory()->getSelectedItemId()) {
				getGameLoaderInventory()->unselectItem(0);
				cmd->_messageKind = 0;
			}
			break;
		case 36: // keydown
			g_nmi->defHandleKeyDown(cmd->_param);

			switch (cmd->_param) {
			case '\x1B': // ESC
				if (g_nmi->_currentScene) {
					getGameLoaderInventory()->unselectItem(0);
					g_nmi->openMainMenu();
					cmd->_messageKind = 0;
				}
				break;
			case 't':
				g_nmi->stopAllSounds();
				cmd->_messageKind = 0;
				break;
			case 'u':
				g_nmi->toggleMute();
				cmd->_messageKind = 0;
				break;
			case ' ':
				if (getGameLoaderInventory()->getIsLocked()) {
					if (getGameLoaderInventory()->getIsInventoryOut()) {
						getGameLoaderInventory()->setIsLocked(0);
					}
				} else {
					getGameLoaderInventory()->slideOut();
					getGameLoaderInventory()->setIsLocked(1);
				}
				break;
			case '\t':
				// Demo has no map
				if (g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS)
					break;

				if (g_nmi->_flgCanOpenMap)
					g_nmi->openMap();
				cmd->_messageKind = 0;
				break;
			case Common::KEYCODE_F1:
				if (g_nmi->_flgCanOpenMap)
					g_nmi->openHelp();
				cmd->_messageKind = 0;
				break;
			case '8':
				{
					int num = 32;
					for (uint i = 0; i < g_nmi->_gameLoader->_sc2array[num]._picAniInfos.size(); i++) {
						debug("pic %d, %d:", num, i);
						g_nmi->_gameLoader->_sc2array[num]._picAniInfos[i].print();
					}
				}
				break;
			default:
				break;
			}
			break;
		case 33:
			if (!g_nmi->_inventoryScene)
				break;

			int invItem;

			if (g_nmi->_updateFlag && (invItem = g_nmi->_inventory->getHoveredItem(&g_nmi->_mouseScreenPos))) {
				g_nmi->_cursorId = PIC_CSR_ITN;
				if (!g_nmi->_currSelectedInventoryItemId && !g_nmi->_aniMan->_movement &&
					!(g_nmi->_aniMan->_flags & 0x100) && g_nmi->_aniMan->isIdle()) {
					int st = g_nmi->_aniMan->_statics->_staticsId;
					ExCommand *newex = 0;

					if (st == ST_MAN_RIGHT) {
						newex = new ExCommand(g_nmi->_aniMan->_id, 1, rMV_MAN_LOOKUP, 0, 0, 0, 1, 0, 0, 0);
					} else if (st == (0x4000 | ST_MAN_RIGHT)) {
						newex = new ExCommand(g_nmi->_aniMan->_id, 1, MV_MAN_LOOKUP, 0, 0, 0, 1, 0, 0, 0);
					}

					if (newex) {
						newex->_param = g_nmi->_aniMan->_odelay;
						newex->_excFlags |= 3;
						newex->postMessage();
					}
				}

				if (g_nmi->_currSelectedInventoryItemId != invItem)
					g_nmi->playSound(SND_CMN_070, 0);

				g_nmi->_currSelectedInventoryItemId = invItem;
				g_nmi->setCursor(g_nmi->_cursorId);
				break;
			}
			if (g_nmi->_updateCursorCallback)
				g_nmi->_updateCursorCallback();

			g_nmi->_currSelectedInventoryItemId = 0;
			g_nmi->setCursor(g_nmi->_cursorId);
			break;
		case 65: // open map
			if (cmd->_field_2C == 11 && cmd->_z == ANI_INV_MAP && g_nmi->_flgCanOpenMap)
				g_nmi->openMap();
			break;
		default:
			break;
		}
	}

	if (cmd->_messageKind == 56) {
		getGameLoaderInventory()->rebuildItemRects();

		ExCommand *newex = new ExCommand(0, 35, SND_CMN_031, 0, 0, 0, 1, 0, 0, 0);

		newex->_z = 1;
		newex->_excFlags |= 3;
		newex->postMessage();

		return 1;
	} else if (cmd->_messageKind == 57) {
		getGameLoaderInventory()->rebuildItemRects();

		return 1;
	}

	return 0;
}

void staticANIObjectCallback(int *arg) {
	(*arg)--;
}

int global_messageHandler2(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	int res = 0;
	StaticANIObject *ani;

	switch (cmd->_messageNum) {
	case 0x44c8:
		error("0x44c8");
		// Unk3_sub_4477A0(&unk3, _parentId, _z != 0);
		break;

	case 28:
		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (ani)
			ani->_priority = cmd->_z;
		break;

	case 25:
		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (ani) {
			if (cmd->_z) {
				ani->setFlags40(true);
				ani->_callback2 = staticANIObjectCallback;
			} else {
				ani->setFlags40(false);
				ani->_callback2 = 0; // Really NULL
			}
		}
		break;

	case 26:
		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (ani) {
			Movement *mov = ani->_movement;
			if (mov)
				mov->_currDynamicPhase->_field_68 = 0;
		}
		break;

	default:
#if 0
		// We never put anything into _defMsgArray
		while (::iterator it = g_nmi->_defMsgArray.begin(); it != g_nmi->_defMsgArray.end(); ++it)
			if (((ExCommand *)*it)->_field_24 == _messageNum) {
				((ExCommand *)*it)->firef34(v13);
				res = 1;
			}
#endif

		//debug_msg(_messageNum);

		if (!g_nmi->_soundEnabled || cmd->_messageNum != 33 || g_nmi->_currSoundListCount <= 0)
			return res;

		for (int snd = 0; snd < g_nmi->_currSoundListCount; snd++) {
			SoundList *s = g_nmi->_currSoundList1[snd];
			int ms = s->getCount();
			for (int i = 0; i < ms; i++) {
				s->getSoundByIndex(i).setPanAndVolumeByStaticAni();
			}
		}
	}

	return res;
}

int global_messageHandler3(ExCommand *cmd) {
	int result = 0;

	if (cmd->_messageKind == 17) {
		switch (cmd->_messageNum) {
		case 29:
		case 30:
		case 31:
		case 32:
		case 36:
			if (g_nmi->_inputDisabled)
				cmd->_messageKind = 0;
			break;
		default:
			break;
		}
	}

	StaticANIObject *ani, *ani2;

	switch (cmd->_messageKind) {
	case 17:
		switch (cmd->_messageNum) {
		case 61:
			debugC(0, kDebugEvents, "preload: { %d, %d },", cmd->_parentId, cmd->_param);
			return g_nmi->_gameLoader->preloadScene(cmd->_parentId, cmd->_param);
		case 62:
			return g_nmi->_gameLoader->gotoScene(cmd->_parentId, cmd->_param);
		case 64:
			if (g_nmi->_currentScene && g_nmi->_msgObjectId2
					&& (!(cmd->_param & 4) || g_nmi->_msgObjectId2 != cmd->_z || g_nmi->_msgId != cmd->_invId)) {
				ani = g_nmi->_currentScene->getStaticANIObject1ById(g_nmi->_msgObjectId2, g_nmi->_msgId);
				if (ani) {
					ani->_flags &= 0xFF7F;
					ani->_flags &= 0xFEFF;
					ani->deleteFromGlobalMessageQueue();
				}
			}
			g_nmi->_msgX = 0;
			g_nmi->_msgY = 0;
			g_nmi->_msgObjectId2 = 0;
			g_nmi->_msgId = 0;
			if ((cmd->_param & 1) || (cmd->_param & 2)) {
				g_nmi->_msgX = cmd->_x;
				g_nmi->_msgY = cmd->_y;
			}
			if (cmd->_param & 4) {
				g_nmi->_msgObjectId2 = cmd->_z;
				g_nmi->_msgId = cmd->_invId;
			}
			return result;
		case 29:
			if (g_nmi->_gameLoader->_interactionController->_flag24 && g_nmi->_currentScene) {
				ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				ani2 = g_nmi->_currentScene->getStaticANIObject1ById(g_nmi->_gameLoader->_field_FA, -1);

				if (ani) {
					if (g_nmi->_msgObjectId2 == ani->_id && g_nmi->_msgId == ani->_odelay) {
						cmd->_messageKind = 0;
						return result;
					}
					if (canInteractAny(ani2, ani, cmd->_param)) {
						handleObjectInteraction(ani2, ani, cmd->_param);
						return 1;
					}
				} else {
					int id = g_nmi->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
					PictureObject *pic = g_nmi->_currentScene->getPictureObjectById(id, 0);
					if (pic) {
						if (g_nmi->_msgObjectId2 == pic->_id && g_nmi->_msgId == pic->_odelay) {
							cmd->_messageKind = 0;
							return result;
						}
						if (!ani2 || canInteractAny(ani2, pic, cmd->_param)) {
							if (!ani2 || (ani2->isIdle() && !(ani2->_flags & 0x80) && !(ani2->_flags & 0x100)))
								handleObjectInteraction(ani2, pic, cmd->_param);
							return 1;
						}
					}
				}
			}
			if (getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->_isEnabled && cmd->_param <= 0) {
				if (g_nmi->_msgX != cmd->_sceneClickX || g_nmi->_msgY != cmd->_sceneClickY) {
					ani = g_nmi->_currentScene->getStaticANIObject1ById(g_nmi->_gameLoader->_field_FA, -1);
					if (!ani || (ani->isIdle() && !(ani->_flags & 0x80) && !(ani->_flags & 0x100))) {
						debugC(0, kDebugPathfinding, "WWW 1");
						result = startWalkTo(g_nmi->_gameLoader->_field_FA, -1, cmd->_sceneClickX, cmd->_sceneClickY, 0);
						if (result) {
							ExCommand *ex = new ExCommand(g_nmi->_gameLoader->_field_FA, 17, 64, 0, 0, 0, 1, 0, 0, 0);

							ex->_param = 1;
							ex->_excFlags |= 3;
							ex->_x = cmd->_sceneClickX;
							ex->_y = cmd->_sceneClickY;
							ex->postMessage();
						}
					}
				} else {
					cmd->_messageKind = 0;
				}
			}
			return result;
		default:
			return result;
		}
	case 58:
		g_nmi->setCursor(cmd->_param);
		return result;
	case 59:
		setInputDisabled(1);
		return result;
	case 60:
		setInputDisabled(0);
		return result;
	case 56:
		if (cmd->_field_2C) {
			ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
			if (ani) {
				getGameLoaderInventory()->addItem2(ani);
				result = 1;
			}
		} else {
			result = 1;
			getGameLoaderInventory()->addItem(cmd->_parentId, 1);
		}
		getGameLoaderInventory()->rebuildItemRects();
		return result;
	case 57:
		if (cmd->_field_2C) {
			if (!cmd->_invId) {
				getGameLoaderInventory()->removeItem2(g_nmi->_currentScene, cmd->_parentId, cmd->_x, cmd->_y, cmd->_z);
				getGameLoaderInventory()->rebuildItemRects();
				return 1;
			}
			ani = g_nmi->_currentScene->getStaticANIObject1ById(g_nmi->_gameLoader->_field_FA, -1);
			if (ani) {
				getGameLoaderInventory()->removeItem2(g_nmi->_currentScene, cmd->_parentId, ani->_ox + cmd->_x, ani->_oy + cmd->_y, ani->_priority + cmd->_z);
				getGameLoaderInventory()->rebuildItemRects();
				return 1;
			}
		} else {
			getGameLoaderInventory()->removeItem(cmd->_parentId, 1);
		}
		getGameLoaderInventory()->rebuildItemRects();
		return 1;
	case 55:
		if (g_nmi->_currentScene) {
			GameObject *obj;
			if (cmd->_z)
				obj = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_x, cmd->_y);
			else
				obj = g_nmi->_currentScene->getPictureObjectById(cmd->_x, cmd->_y);
			handleObjectInteraction(g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param), obj, cmd->_invId);
			result = 1;
		}
		return result;
	case 51:
		return startWalkTo(cmd->_parentId, cmd->_param, cmd->_x, cmd->_y, cmd->_invId);
	case 52:
		return doSomeAnimation(cmd->_parentId, cmd->_param, cmd->_invId);
	case 53:
		return doSomeAnimation2(cmd->_parentId, cmd->_param);
	case 63:
		if (cmd->_objtype == kObjTypeObjstateCommand) {
			ObjstateCommand *c = static_cast<ObjstateCommand *>(cmd);
			result = 1;
			g_nmi->setObjectState(c->_objCommandName.c_str(), c->_value);
		}
		return result;
	default:
		return result;
	}
}

int global_messageHandler4(ExCommand *cmd) {
	StaticANIObject *ani = 0;

	switch (cmd->_messageKind) {
	case 18: {
		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(cmd->_messageNum), cmd->_parId, 0);

		if (cmd->_excFlags & 1)
			mq->_flag1 = 1;
		else
			mq->_flag1 = 0;

		mq->sendNextCommand();
		break;
	}
	case 2:
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		ani->trySetMessageQueue(cmd->_messageNum, cmd->_parId);
		break;

	case 1: {
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		int flags = cmd->_z;
		if (flags <= 0)
			flags = -1;

		if (cmd->_excFlags & 1)
			ani->startAnim(cmd->_messageNum, 0, flags);
		else
			ani->startAnim(cmd->_messageNum, cmd->_parId, flags);

		break;
	}
	case 8:
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		ani->startAnimEx(cmd->_messageNum, cmd->_parId, -1, -1);
		break;

	case 20: {
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		int flags = cmd->_z;
		if (flags <= 0)
			flags = -1;

		if (cmd->_objtype == kObjTypeExCommand2) {
			ExCommand2 *cmd2 = static_cast<ExCommand2 *>(cmd);

			if (cmd->_excFlags & 1) {
				ani->startAnimSteps(cmd->_messageNum, 0, cmd->_x, cmd->_y, cmd2->_points, flags);
			} else {
				ani->startAnimSteps(cmd->_messageNum, cmd->_parId, cmd->_x, cmd->_y, cmd2->_points, flags);
			}
		}
		break;
	}
	case 21:
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		ani->queueMessageQueue(0);
		ani->playIdle();
		break;
	case 9:
		// Nop in original
		break;
	case 3:
		g_nmi->_currentScene->_y = cmd->_messageNum - cmd->_messageNum % g_nmi->_scrollSpeed;
		break;

	case 4:
		g_nmi->_currentScene->_x = cmd->_messageNum - cmd->_messageNum % g_nmi->_scrollSpeed;
		break;

	case 19: {
		if (!g_nmi->_currentScene)
			break;
		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		MessageQueue *mq = ani->getMessageQueue();
		MessageQueue *mq2 = ani->changeStatics1(cmd->_messageNum);

		if (!mq2 || !mq2->getExCommandByIndex(0) || !mq)
			break;

		mq2->_parId = mq->_id;
		mq2->_flag1 = (cmd->_field_24 == 0);
		break;
	}
	case 22:
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		ani->_flags |= 4;
		ani->changeStatics2(cmd->_messageNum);
		break;

	case 6:
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		ani->hide();
		break;

	case 27:
		if (!g_nmi->_currentScene || g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param) == 0) {
			ani = g_nmi->accessScene(cmd->_invId)->getStaticANIObject1ById(cmd->_parentId, -1);
			if (ani) {
				ani = new StaticANIObject(ani);
				g_nmi->_currentScene->addStaticANIObject(ani, 1);
			}
		}

		// fall through
	case 5:
		if (g_nmi->_currentScene)
			ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);

		if (!ani)
			break;

		if (cmd->_z >= 0)
			ani->_priority = cmd->_z;

		ani->show1(cmd->_x, cmd->_y, cmd->_messageNum, cmd->_parId);
		break;

	case 10:
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		if (cmd->_z >= 0)
			ani->_priority = cmd->_z;

		ani->show2(cmd->_x, cmd->_y, cmd->_messageNum, cmd->_parId);
		break;

	case 7: {
		if (!g_nmi->_currentScene->_picObjList.size())
			break;

		int offX = g_nmi->_scrollSpeed * (cmd->_x / g_nmi->_scrollSpeed);
		int offY = g_nmi->_scrollSpeed * (cmd->_y / g_nmi->_scrollSpeed);

		if (cmd->_messageNum) {
			g_nmi->_currentScene->_x = offX - g_nmi->_sceneRect.left;
			g_nmi->_currentScene->_y = offY - g_nmi->_sceneRect.top;

			if (cmd->_field_24) {
				g_nmi->_currentScene->_messageQueueId = cmd->_parId;
			}
		} else {
			g_nmi->_sceneRect.translate(offX - g_nmi->_sceneRect.left, offY - g_nmi->_sceneRect.top);

			g_nmi->_currentScene->_x = 0;
			g_nmi->_currentScene->_y = 0;

			g_nmi->_currentScene->updateScrolling2();
		}
		break;
	}
	case 34:
		if (!g_nmi->_currentScene)
			break;

		ani = g_nmi->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_param);
		if (!ani)
			break;

		ani->_flags = cmd->_messageNum | (ani->_flags & ~cmd->_z);

		break;

	case 35:
		global_messageHandler_handleSound(cmd);
		break;

	case 11:
	case 12:
		break;
	default:
		return 0;
		break;
	}

	return 1;
}

int MovGraph::messageHandler(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	if (cmd->_messageNum != 33)
		return 0;

	StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(g_nmi->_gameLoader->_field_FA, -1);

	if (!getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId))
		return 0;

	if (getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->_objtype != kObjTypeMovGraph || !ani)
		return 0;

	MovGraph *gr = getSc2MovGraphBySceneId(g_nmi->_currentScene->_sceneId);

	MovGraphLink *link = 0;
	double mindistance = 1.0e10;
	Common::Point point;

	for (LinkList::iterator i = gr->_links.begin(); i != gr->_links.end(); ++i) {
		point.x = ani->_ox;
		point.y = ani->_oy;

		double dst = gr->putToLink(&point, *i, 0);
		if (dst >= 0.0 && dst < mindistance) {
			mindistance = dst;
			link = *i;
		}
	}

	int top;

	if (link) {
		MovGraphNode *node = link->_graphSrc;

		double sq = (ani->_oy - node->_y) * (ani->_oy - node->_y) + (ani->_ox - node->_x) * (ani->_ox - node->_x);
		int off = (node->_z >> 16) & 0xFF;
		double off2 = ((link->_graphDst->_z >> 8) & 0xff) - off;

		top = off + (int)(sqrt(sq) * off2 / link->_length);
	} else {
		top = (gr->calcOffset(ani->_ox, ani->_oy)->_z >> 8) & 0xff;
	}

	if (ani->_movement) {
		ani->_movement->_currDynamicPhase->_rect.top = 255 - top;
		return 0;
	}

	if (ani->_statics)
		ani->_statics->_rect.top = 255 - top;

	return 0;
}

} // End of namespace NGI
