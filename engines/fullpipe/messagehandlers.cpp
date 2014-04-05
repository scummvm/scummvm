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

#include "fullpipe/fullpipe.h"

#include "fullpipe/messages.h"
#include "fullpipe/statics.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/interaction.h"
#include "fullpipe/motion.h"
#include "fullpipe/input.h"

#include "fullpipe/constants.h"

namespace Fullpipe {

void global_messageHandler_KickStucco() {
	warning("STUB: global_messageHandler_KickStucco()");
}

void global_messageHandler_KickMetal() {
	warning("STUB: global_messageHandler_KickMetal()");
}

int global_messageHandler1(ExCommand *cmd) {
	debug(5, "global_messageHandler1: %d %d", cmd->_messageKind, cmd->_messageNum);

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

	if (g_fp->_inputDisabled) {
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
			g_fp->_aniMan->_shadowsOn = 1;
			break;
		case MSG_HMRKICK_STUCCO:
			global_messageHandler_KickStucco();
			break;
		case MSG_MANSHADOWSOFF:
			g_fp->_aniMan->_shadowsOn = 0;
			break;
		case MSG_DISABLESAVES:
			g_fp->disableSaves(cmd);
			break;
		case MSG_ENABLESAVES:
			g_fp->enableSaves();
			break;
		case MSG_HMRKICK_METAL:
			global_messageHandler_KickMetal();
			break;
		case 29: // left mouse
			if (g_fp->_inventoryScene) {
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
			g_fp->defHandleKeyDown(cmd->_keyCode);

			switch (cmd->_keyCode) {
			case '\x1B': // ESC
				if (g_fp->_currentScene) {
					getGameLoaderInventory()->unselectItem(0);
					g_fp->openMainMenu();
					cmd->_messageKind = 0;
				}
				break;
			case 't':
				g_fp->stopAllSounds();
				cmd->_messageKind = 0;
				break;
			case 'u':
				g_fp->toggleMute();
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
				if (g_fp->_flgCanOpenMap)
					g_fp->openMap();
				cmd->_messageKind = 0;
				break;
			case 'p':
				if (g_fp->_flgCanOpenMap)
					g_fp->openHelp();
				cmd->_messageKind = 0;
				break;
			default:
				break;
			}
			break;
		case 33:
			if (!g_fp->_inventoryScene)
				break;

			int invItem;

			if (g_fp->_updateFlag && (invItem = g_fp->_inventory->getHoveredItem(&g_fp->_mouseScreenPos))) {
				g_fp->_cursorId = PIC_CSR_ITN;
				if (!g_fp->_currSelectedInventoryItemId && !g_fp->_aniMan->_movement &&
					!(g_fp->_aniMan->_flags & 0x100) && g_fp->_aniMan->isIdle()) {
					int st = g_fp->_aniMan->_statics->_staticsId;
					ExCommand *newex = 0;

					if (st == ST_MAN_RIGHT) {
						newex = new ExCommand(g_fp->_aniMan->_id, 1, rMV_MAN_LOOKUP, 0, 0, 0, 1, 0, 0, 0);
					} else if (st == (0x4000 | ST_MAN_RIGHT)) {
						newex = new ExCommand(g_fp->_aniMan->_id, 1, MV_MAN_LOOKUP, 0, 0, 0, 1, 0, 0, 0);
					}

					if (newex) {
						newex->_keyCode = g_fp->_aniMan->_okeyCode;
						newex->_excFlags |= 3;
						newex->postMessage();
					}
				}

				if (g_fp->_currSelectedInventoryItemId != invItem)
					g_fp->playSound(SND_CMN_070, 0);

				g_fp->_currSelectedInventoryItemId = invItem;
				g_fp->setCursor(g_fp->_cursorId);
				break;
			}
			if (g_fp->_updateCursorCallback)
				g_fp->_updateCursorCallback();

			g_fp->_currSelectedInventoryItemId = 0;
			g_fp->setCursor(g_fp->_cursorId);
			break;
		case 65: // open map
			if (cmd->_field_2C == 11 && cmd->_field_14 == ANI_INV_MAP && g_fp->_flgCanOpenMap)
				g_fp->openMap();
			break;
		default:
			break;
		}
	}

	if (cmd->_messageKind == 56) {
		getGameLoaderInventory()->rebuildItemRects();

		ExCommand *newex = new ExCommand(0, 35, SND_CMN_031, 0, 0, 0, 1, 0, 0, 0);

		newex->_field_14 = 1;
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
		// Unk3_sub_4477A0(&unk3, _parentId, _field_14 != 0);
		break;

	case 28:
		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (ani)
			ani->_priority = cmd->_field_14;
		break;

	case 25:
		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (ani) {
			if (cmd->_field_14) {
				ani->setFlags40(true);
				ani->_callback2 = staticANIObjectCallback;
			} else {
				ani->setFlags40(false);
				ani->_callback2 = 0;
			}
		}
		break;

	case 26:
		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (ani) {
			Movement *mov = ani->_movement;
			if (mov)
				mov->_currDynamicPhase->_field_68 = 0;
		}
		break;

	default:
#if 0
		// We never put anything into _defMsgArray
		while (::iterator it = g_fp->_defMsgArray.begin(); it != g_fp->_defMsgArray.end(); ++it)
			if (((ExCommand *)*it)->_field_24 == _messageNum) {
				((ExCommand *)*it)->firef34(v13);
				res = 1;
			}
#endif

		//debug_msg(_messageNum);

		if (!g_fp->_soundEnabled || cmd->_messageNum != 33 || g_fp->_currSoundListCount <= 0)
			return res;

		for (int snd = 0; snd < g_fp->_currSoundListCount; snd++) {
			SoundList *s = g_fp->_currSoundList1[snd];
		    int ms = s->getCount();
			for (int i = 0; i < ms; i++) {
				s->getSoundByIndex(i)->setPanAndVolumeByStaticAni();
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
			if (g_fp->_inputDisabled)
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
			debug(0, "preload: { %d, %d },", cmd->_parentId, cmd->_keyCode);
			return g_fp->_gameLoader->preloadScene(cmd->_parentId, cmd->_keyCode);
		case 62:
			return g_fp->_gameLoader->gotoScene(cmd->_parentId, cmd->_keyCode);
		case 64:
			if (g_fp->_currentScene && g_fp->_msgObjectId2
					&& (!(cmd->_keyCode & 4) || g_fp->_msgObjectId2 != cmd->_field_14 || g_fp->_msgId != cmd->_field_20)) {
				ani = g_fp->_currentScene->getStaticANIObject1ById(g_fp->_msgObjectId2, g_fp->_msgId);
				if (ani) {
					ani->_flags &= 0xFF7F;
					ani->_flags &= 0xFEFF;
					ani->deleteFromGlobalMessageQueue();
				}
			}
			g_fp->_msgX = 0;
			g_fp->_msgY = 0;
			g_fp->_msgObjectId2 = 0;
			g_fp->_msgId = 0;
			if ((cmd->_keyCode & 1) || (cmd->_keyCode & 2)) {
				g_fp->_msgX = cmd->_x;
				g_fp->_msgY = cmd->_y;
			}
			if (cmd->_keyCode & 4) {
				g_fp->_msgObjectId2 = cmd->_field_14;
				g_fp->_msgId = cmd->_field_20;
			}
			return result;
		case 29:
			if (!g_fp->_currentScene)
				return result;

			if (g_fp->_gameLoader->_interactionController->_flag24) {
				ani = g_fp->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				ani2 = g_fp->_currentScene->getStaticANIObject1ById(g_fp->_gameLoader->_field_FA, -1);
				if (ani) {
					if (g_fp->_msgObjectId2 == ani->_id && g_fp->_msgId == ani->_okeyCode) {
						cmd->_messageKind = 0;
						return result;
					}
					if (canInteractAny(ani2, ani, cmd->_keyCode)) {
						handleObjectInteraction(ani2, ani, cmd->_keyCode);
						return 1;
					}
				} else {
					int id = g_fp->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
					PictureObject *pic = g_fp->_currentScene->getPictureObjectById(id, 0);
					if (pic) {
						if (g_fp->_msgObjectId2 == pic->_id && g_fp->_msgId == pic->_okeyCode) {
							cmd->_messageKind = 0;
							return result;
						}
						if (!ani2 || canInteractAny(ani2, pic, cmd->_keyCode)) {
							if (!ani2 || (ani2->isIdle() && !(ani2->_flags & 0x80) && !(ani2->_flags & 0x100)))
								handleObjectInteraction(ani2, pic, cmd->_keyCode);
							return 1;
						}
					}
				}
			}
			if (getSc2MctlCompoundBySceneId(g_fp->_currentScene->_sceneId)->_isEnabled && cmd->_keyCode <= 0) {
				if (g_fp->_msgX != cmd->_sceneClickX || g_fp->_msgY != cmd->_sceneClickY) {
					ani = g_fp->_currentScene->getStaticANIObject1ById(g_fp->_gameLoader->_field_FA, -1);
					if (!ani || (ani->isIdle() && !(ani->_flags & 0x80) && !(ani->_flags & 0x100))) {
						result = startWalkTo(g_fp->_gameLoader->_field_FA, -1, cmd->_sceneClickX, cmd->_sceneClickY, 0);
						if (result) {
							ExCommand *ex = new ExCommand(g_fp->_gameLoader->_field_FA, 17, 64, 0, 0, 0, 1, 0, 0, 0);

							ex->_keyCode = 1;
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
		g_fp->setCursor(cmd->_keyCode);
		return result;
	case 59:
		setInputDisabled(1);
		return result;
	case 60:
		setInputDisabled(0);
		return result;
	case 56:
		if (cmd->_field_2C) {
			ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
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
			if (!cmd->_field_20) {
				getGameLoaderInventory()->removeItem2(g_fp->_currentScene, cmd->_parentId, cmd->_x, cmd->_y, cmd->_field_14);
				getGameLoaderInventory()->rebuildItemRects();
				return 1;
			}
			ani = g_fp->_currentScene->getStaticANIObject1ById(g_fp->_gameLoader->_field_FA, -1);
			if (ani) {
				getGameLoaderInventory()->removeItem2(g_fp->_currentScene, cmd->_parentId, ani->_ox + cmd->_x, ani->_oy + cmd->_y, ani->_priority + cmd->_field_14);
				getGameLoaderInventory()->rebuildItemRects();
				return 1;
			}
		} else {
			getGameLoaderInventory()->removeItem(cmd->_parentId, 1);
		}
		getGameLoaderInventory()->rebuildItemRects();
		return 1;
	case 55:
		if (g_fp->_currentScene) {
			GameObject *obj;
			if (cmd->_field_14)
				obj = g_fp->_currentScene->getStaticANIObject1ById(cmd->_x, cmd->_y);
			else
				obj = g_fp->_currentScene->getPictureObjectById(cmd->_x, cmd->_y);
			handleObjectInteraction(g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode), obj, cmd->_field_20);
			result = 1;
		}
		return result;
	case 51:
		return startWalkTo(cmd->_parentId, cmd->_keyCode, cmd->_x, cmd->_y, cmd->_field_20);
	case 52:
		return doSomeAnimation(cmd->_parentId, cmd->_keyCode, cmd->_field_20);
	case 53:
		return doSomeAnimation2(cmd->_parentId, cmd->_keyCode);
	case 63:
		if (cmd->_objtype == kObjTypeObjstateCommand) {
			ObjstateCommand *c = (ObjstateCommand *)cmd;
			result = 1;
			g_fp->setObjectState(c->_objCommandName, c->_value);
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
		MessageQueue *mq = new MessageQueue(g_fp->_currentScene->getMessageQueueById(cmd->_messageNum), cmd->_parId, 0);

		if (cmd->_excFlags & 1)
			mq->_flag1 = 1;
		else
			mq->_flag1 = 0;

		mq->sendNextCommand();
		break;
	}
	case 2:
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->trySetMessageQueue(cmd->_messageNum, cmd->_parId);
		break;

	case 1: {
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		int flags = cmd->_field_14;
		if (flags <= 0)
			flags = -1;

		if (cmd->_excFlags & 1)
			ani->startAnim(cmd->_messageNum, 0, flags);
		else
			ani->startAnim(cmd->_messageNum, cmd->_parId, flags);

		break;
	}
	case 8:
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->startAnimEx(cmd->_messageNum, cmd->_parId, -1, -1);
		break;

	case 20: {
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		int flags = cmd->_field_14;
		if (flags <= 0)
			flags = -1;

		ExCommand2 *cmd2 = (ExCommand2 *)cmd;

		if (cmd->_excFlags & 1) {
			ani->startAnimSteps(cmd->_messageNum, 0, cmd->_x, cmd->_y, cmd2->_points, cmd2->_pointsSize >> 3, flags);
		} else {
			ani->startAnimSteps(cmd->_messageNum, cmd->_parId, cmd->_x, cmd->_y, cmd2->_points, cmd2->_pointsSize >> 3, flags);
		}
		break;
	}
	case 21:
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->queueMessageQueue(0);
		ani->playIdle();
		break;
	case 9:
		// Nop in original
		break;
	case 3:
		g_fp->_currentScene->_y = cmd->_messageNum - cmd->_messageNum % g_fp->_scrollSpeed;
		break;

	case 4:
		g_fp->_currentScene->_x = cmd->_messageNum - cmd->_messageNum % g_fp->_scrollSpeed;
		break;

	case 19: {
		if (!g_fp->_currentScene)
			break;
		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
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
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->_flags |= 4;
		ani->changeStatics2(cmd->_messageNum);
		break;

	case 6:
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->hide();
		break;

	case 27:
		if (!g_fp->_currentScene || g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode) == 0) {
			ani = g_fp->accessScene(cmd->_field_20)->getStaticANIObject1ById(cmd->_parentId, -1);
			if (ani) {
				ani = new StaticANIObject(ani);
				g_fp->_currentScene->addStaticANIObject(ani, 1);
			}
		}

		// fall through
	case 5:
		if (g_fp->_currentScene)
			ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);

		if (!ani)
			break;

		if (cmd->_field_14 >= 0)
			ani->_priority = cmd->_field_14;

		ani->show1(cmd->_x, cmd->_y, cmd->_messageNum, cmd->_parId);
		break;

	case 10:
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		if (cmd->_field_14 >= 0)
			ani->_priority = cmd->_field_14;

		ani->show2(cmd->_x, cmd->_y, cmd->_messageNum, cmd->_parId);
		break;

	case 7: {
		if (!g_fp->_currentScene->_picObjList.size())
			break;

		int offX = g_fp->_scrollSpeed * (cmd->_x / g_fp->_scrollSpeed);
		int offY = g_fp->_scrollSpeed * (cmd->_y / g_fp->_scrollSpeed);

		if (cmd->_messageNum) {
			g_fp->_currentScene->_x = offX - g_fp->_sceneRect.left;
			g_fp->_currentScene->_y = offY - g_fp->_sceneRect.top;

			if (cmd->_field_24) {
				g_fp->_currentScene->_messageQueueId = cmd->_parId;
			}
		} else {
			g_fp->_sceneRect.translate(offX - g_fp->_sceneRect.left, offY - g_fp->_sceneRect.top);

			g_fp->_currentScene->_x = 0;
			g_fp->_currentScene->_y = 0;

			g_fp->_currentScene->updateScrolling2();
		}
		break;
	}
	case 34:
		if (!g_fp->_currentScene)
			break;

		ani = g_fp->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->_flags = cmd->_messageNum | (ani->_flags & ~cmd->_field_14);

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

int MovGraph_messageHandler(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	if (cmd->_messageNum != 33)
		return 0;

	StaticANIObject *ani = g_fp->_currentScene->getStaticANIObject1ById(g_fp->_gameLoader->_field_FA, -1);

	if (!getSc2MctlCompoundBySceneId(g_fp->_currentScene->_sceneId))
		return 0;

	if (getSc2MctlCompoundBySceneId(g_fp->_currentScene->_sceneId)->_objtype != kObjTypeMovGraph || !ani)
		return 0;

	MovGraph *gr = (MovGraph *)getSc2MctlCompoundBySceneId(g_fp->_currentScene->_sceneId);

	MovGraphLink *link = 0;
	double mindistance = 1.0e10;
	Common::Point point;

	for (ObList::iterator i = gr->_links.begin(); i != gr->_links.end(); ++i) {
		point.x = ani->_ox;
		point.y = ani->_oy;

		double dst = gr->calcDistance(&point, (MovGraphLink *)(*i), 0);
		if (dst >= 0.0 && dst < mindistance) {
			mindistance = dst;
			link = (MovGraphLink *)(*i);
		}
	}

	int top;

	if (link) {
		MovGraphNode *node = link->_movGraphNode1;

		double sq = (ani->_oy - node->_y) * (ani->_oy - node->_y) + (ani->_ox - node->_x) * (ani->_ox - node->_x);
		int off = (node->_field_14 >> 16) & 0xFF;
		double off2 = ((link->_movGraphNode2->_field_14 >> 8) & 0xff) - off;

		top = off + (int)(sqrt(sq) * off2 / link->_distance);
	} else {
		top = (gr->calcOffset(ani->_ox, ani->_oy)->_field_14 >> 8) & 0xff;
	}

	if (ani->_movement) {
		ani->_movement->_currDynamicPhase->_rect->top = 255 - top;
		return 0;
	}

	if (ani->_statics)
		ani->_statics->_rect->top = 255 - top;

	return 0;
}

} // End of namespace Fullpipe
