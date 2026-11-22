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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ngi/ngi.h"

#include "ngi/interaction.h"
#include "ngi/gameloader.h"
#include "ngi/statics.h"
#include "ngi/motion.h"

namespace NGI {

int handleObjectInteraction(AniObject *subject, GameObject *object, int invId) {
	return getGameLoaderInteractionController()->handleInteraction(subject, object, invId);
}

bool canInteractAny(GameObject *obj1, GameObject *obj2, int invId) {
	int sceneId = 0;

	if (g_nmi->_currentScene)
		sceneId = g_nmi->_currentScene->_sceneId;

	InteractionController *intC = getGameLoaderInteractionController();
	for (InteractionController::InteractionList::iterator i = intC->_interactions.begin(); i != intC->_interactions.end(); ++i) {
		Interact *intr = *i;

		if (intr->_sceneId > 0 && intr->_sceneId != sceneId)
			break;

		if (invId == -3) {
			invId = getGameLoaderInventory()->getSelectedItemId();
		}
		if (intr->canRun(obj1, obj2, invId))
			return true;
	}
	return false;
}

InteractionController::~InteractionController() {
	Common::for_each(_interactions.begin(), _interactions.end(), Common::DefaultDeleter<Interact>());
	removeMessageHandler(124, -1);
}

bool InteractionController::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "InteractionController::load()");

	return _interactions.load(file);
}

int static_compSceneId = 0;

bool InteractionController::compareInteractions(const Interact *i1, const Interact *i2) {
	if (i2->_sceneId < i1->_sceneId) {
		if (i1->_sceneId != static_compSceneId)
			return false;
	}
	if (i2->_sceneId != i1->_sceneId) {
		if (i1->_sceneId > 0 && i2->_sceneId == static_compSceneId)
			return false;
		if (i2->_sceneId != i1->_sceneId)
			return true;
	}
	if (i2->_cursorObjectId == -1)
		return true;

	if (i1->_cursorObjectId == i2->_cursorObjectId)
		return true;

	if (i1->_cursorObjectId == -1 || i1->_cursorObjectId == -2)
		return false;

	return true;
}

void InteractionController::sortInteractions(int sceneId) {
	static_compSceneId = sceneId;

	Common::sort(_interactions.begin(), _interactions.end(), InteractionController::compareInteractions);
}

bool InteractionController::handleInteraction(AniObject *subj, GameObject *obj, int invId) {
	if (subj) {
		if (!subj->isIdle() || (subj->_flags & 0x100))
			return false;
	}

	if (!_interactions.size())
		return false;

	Interact *inter = nullptr;
	Interact *previnter = nullptr;
	int dur = 0;
	int mindur = 0xFFFF;

	MessageQueue *mq;
	Command *ex;

	for (InteractionList::iterator i = _interactions.begin(); i != _interactions.end(); ++i) {
		Interact *cinter = *i;

		if (!cinter->canRun(subj, obj, invId))
			continue;

		if ((inter || cinter->_heroId) && (!obj || cinter->_cursorObjectId != obj->_id)) {
			if (cinter->_messageQueue)
				cinter->_messageQueue->calcDuration(subj);

			debugC(5, kDebugInteractions, "Interact: %s", transCyrillic(cinter->_actionName));

			PicAniInfo aniInfo;

			obj->getPicAniInfo(aniInfo);

			if (cinter->_objectStaticId && obj->_objtype == kObjTypeStaticANIObject) {
				AniObject *ani = static_cast<AniObject *>(obj);
				ani->_messageQueueId = 0;
				ani->setStatic(cinter->_objectStaticId);
			}
			int xpos = cinter->_heroOffsetX + obj->_x;
			int ypos = cinter->_heroOffsetY + obj->_y;

			obj->setPicAniInfo(aniInfo);

			if (abs(xpos - subj->_x) > 1 || abs(ypos - subj->_y) > 1) {
				debugC(0, kDebugPathfinding, "Calling makeQueue() at [%d, %d]", xpos, ypos);
				mq = getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->makeQueue(subj, xpos, ypos, 1, cinter->_heroStaticId);
				if (mq) {
					dur = mq->calcDuration(subj);
					delete mq;
				} else {
					dur = 0x10000;
				}
				inter = previnter;
			} else {
				dur = 0;
			}
			if (dur < mindur) {
				inter = cinter;
				mindur = dur;
				previnter = cinter;
			}
		} else {
			inter = cinter;
			break;
		}
	}

	if (!inter)
		return false;

	if (!inter->_heroId) {
		if (obj->_objtype == kObjTypeStaticANIObject) {
			AniObject *ani = static_cast<AniObject *>(obj);

			if (inter->_flags & 1) {
				if (!ani->isIdle())
					return false;

				if (ani->_flags & 0x100)
					return false;
			} else if (inter->_objectStaticId != 0) {
				if (ani->_movement || ani->_statics == nullptr || ani->_statics->_staticsId != inter->_objectStaticId) {
					mq = ani->moveToStatic(inter->_objectStaticId);
					if (!mq)
						return false;

					ex = new Command((subj ? subj->_id : 0), 55, 0, 0, 0, 0, 1, 0, 0, 0);
					ex->_x = obj->_id;
					ex->_y = obj->_copyId;
					ex->_param = subj ? subj->_copyId : 0;
					ex->_excFlags = 3;
					ex->_z = (obj->_objtype != kObjTypePictureObject);
					ex->_invId = invId;
					mq->addCommandToEnd(ex);

					if (mq->_isFinished) {
						mq->_isFinished = 0;
						ani->setQueue(mq);
					}

					return true;
				} else {
					if (ani->getMessageQueue())
						ani->setQueue(nullptr);
				}
			}
		}

		if (inter->_messageQueue) {
			mq = new MessageQueue(inter->_messageQueue, 0, 1);
			mq->setObjectCopyForId(obj->_id, -1, obj->_copyId);

			if (!mq->run(nullptr))
				return false;
		}

		return true;
	}

	if (obj && !subj)
		return true;

	if (!obj || inter->_cursorObjectId == obj->_id) {
		if (subj) {
			if (inter->_messageQueue) {
				if (subj->isIdle()) {
					mq = new MessageQueue(inter->_messageQueue, 0, 1);

					if (!mq->run(subj)) {
						delete mq;

						return false;
					}
				}
			}
		}
		return true;
	}

	if (inter->isOverlapping(subj, obj)) {
		if (obj->_objtype == kObjTypeStaticANIObject) {
			AniObject *ani = static_cast<AniObject *>(obj);

			ani->setQueue(nullptr);

			if (inter->_objectStaticId)
				ani->setStatic(inter->_objectStaticId);

			if (!(inter->_flags & 0x10000))
				obj->_flags |= 0x80;
		}

		if (!inter->_messageQueue)
			return false;

		subj->setPos(inter->_heroOffsetX + obj->_x, inter->_heroOffsetY + obj->_y);

		mq = new MessageQueue(inter->_messageQueue, 0, 1);
		mq->setObjectCopyForId(obj->_id, -1, obj->_copyId);
		mq->_flags |= 1;

		if (!(inter->_flags & 0x10000)) {
			ex = new Command(obj->_id, 34, 0x80, 0, 0, 0, 1, 0, 0, 0);
			ex->_param = obj->_copyId;
			ex->_z = 0x80;
			ex->_messageNum = 0;
			ex->_excFlags = 3;
			mq->addCommandToEnd(ex);
		}

		ex = new Command(obj->_id, 34, 0x100, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = obj->_copyId;
		ex->_z = 0x100;
		ex->_messageNum = 0;
		ex->_excFlags = 3;
		mq->addCommandToEnd(ex);

		ex = new Command(subj->_id, 34, 0x100, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = subj->_copyId;
		ex->_z = 0x100;
		ex->_messageNum = 0;
		ex->_excFlags = 3;
		mq->addCommandToEnd(ex);

		ex = new Command(subj->_id, 17, 0x40, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_param = 0;
		mq->addCommandToEnd(ex);

		if (!mq->run(subj)) {
			delete mq;

			return false;
		}

		subj->_flags |= 0x100;
		obj->_flags |= 0x100;
	} else {
		bool someFlag = false;
		PicAniInfo aniInfo;

		obj->getPicAniInfo(aniInfo);

		if (obj->_objtype == kObjTypeStaticANIObject && inter->_objectStaticId) {
			AniObject *ani = static_cast<AniObject *>(obj);

			ani->_messageQueueId = 0;
			ani->setStatic(inter->_objectStaticId);
		}

		int xpos = inter->_heroOffsetX + obj->_x;
		int ypos = inter->_heroOffsetY + obj->_y;

		obj->setPicAniInfo(aniInfo);

		if (abs(xpos - subj->_x) > 1 || abs(ypos - subj->_y) > 1
				|| (inter->_heroStaticId != 0 && (subj->_statics == nullptr || subj->_statics->_staticsId != inter->_heroStaticId))) {
			mq = getSc2MctlCompoundBySceneId(g_nmi->_currentScene->_sceneId)->startMove(subj, xpos, ypos, 1, inter->_heroStaticId);

			if (!mq)
				return false;

			ex = new Command(subj->_id, 55, 0, 0, 0, 0, 1, 0, 0, 0);
			ex->_x = obj->_id;
			ex->_y = obj->_copyId;
			ex->_param = subj->_copyId;
			ex->_excFlags = 3;
			ex->_invId = invId;
			ex->_z = (obj->_objtype != kObjTypePictureObject);
			mq->addCommandToEnd(ex);

			someFlag = true;

			ex = new Command(subj->_id, 17, 0x40, 0, 0, 0, 1, 0, 0, 0);
			ex->_x = xpos;
			ex->_y = ypos;
			ex->_excFlags |= 3;
			ex->_param = 6;
			ex->_z = obj->_id;
			ex->_invId = obj->_copyId;
			ex->postMessage();
		}

		if (!inter->_objectStaticId || !(inter->_flags & 1))
			return true;

		if (obj->_objtype != kObjTypeStaticANIObject)
			return false;

		AniObject *ani = static_cast<AniObject *>(obj);

		if (!ani->isIdle())
			return false;

		if (ani->getMessageQueue())
			ani->setQueue(nullptr);

		if (!ani->_statics || ani->_statics->_staticsId != inter->_objectStaticId || ani->_movement) {
			mq = ani->moveToStatic(inter->_objectStaticId);

			if (!mq)
				return false;

			if (someFlag) {
				if (!(inter->_flags & 0x10000)) {
					if (mq->_isFinished) {
						ani->_flags |= 0x80u;
					} else {
						ex = new Command(ani->_id, 34, 0x80, 0, 0, 0, 1, 0, 0, 0);
						ex->_z = 0x80;
						ex->_param = ani->_copyId;
						ex->_excFlags = 3;
						mq->addCommandToEnd(ex);
					}
				}
				ex = new Command(ani->_id, 34, 0x100, 0, 0, 0, 1, 0, 0, 0);
				ex->_param = ani->_copyId;
				ex->_z = 0x100;
				ex->_excFlags = 3;
				mq->addCommandToEnd(ex);
			} else {
				ex = new Command(subj->_id, 55, 0, 0, 0, 0, 1, 0, 0, 0);
				ex->_x = ani->_id;
				ex->_y = ani->_copyId;
				ex->_param = subj->_copyId;
				ex->_excFlags = 2;
				ex->_z = (obj->_objtype != kObjTypePictureObject);
				ex->_invId = invId;
				mq->addCommandToEnd(ex);

				if (!mq->_isFinished)
					return true;

				mq->_isFinished = 0;
				ani->setQueue(mq);
			}
		} else {
			obj->_flags |= 0x100;

			if (inter->_flags & 0x10000)
				return true;

			obj->_flags |= 0x80;
		}
	}

	return true;
}

Interact *InteractionController::findInteraction(int obId, int obId2, int obId3) {
	for (InteractionList::iterator i = _interactions.begin(); i != _interactions.end(); ++i) {
		Interact *intr = *i;

		if (intr->_objectId == obId && intr->_heroId == obId2 && intr->_cursorObjectId == obId3)
			return intr;
	}

	return nullptr;
}

Interact::Interact() {
	_objectId = 0;
	_heroId = 0;
	_objectStaticId = 0;
	_cursorObjectId = 0;
	_objectState2 = 0;
	_objectState1 = 0;
	_messageQueue = nullptr;
	_flags = 0;
	_heroOffsetY = 0;
	_heroOffsetX = 0;
	_heroStaticId = 0;
	_field_28 = 0;
	_sceneId = -1;
}

Interact::~Interact() {
	if (_messageQueue) {
		while (_messageQueue->getCommandByIndex(0))
			_messageQueue->removeCommand(0, 1);
	}

	delete _messageQueue;
}

bool Interact::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Interact::load()");

	_objectId = file.readUint16LE();
	_heroId = file.readUint16LE();
	_objectStaticId = file.readUint16LE();
	_heroStaticId = file.readUint16LE();
	_cursorObjectId = file.readUint16LE();
	_objectState2 = file.readUint32LE();
	_objectState1 = file.readUint32LE();
	_heroOffsetX = file.readSint32LE();
	_heroOffsetY = file.readSint32LE();
	_sceneId = file.readUint32LE();
	_flags = file.readUint32LE();
	_actionName = file.readPascalString();

	_messageQueue = file.readClass<MessageQueue>();

	return true;
}

bool Interact::canRun(GameObject *obj1, GameObject *obj2, int invId) {
	if (_sceneId > 0 && g_nmi->_currentScene && g_nmi->_currentScene->_sceneId != _sceneId)
		return false;

	if (_flags & 0x20000)
		return false;

	if (!obj2)
		return false;

	if (obj2->_id != _objectId)
		return false;

	if ((_flags & 8) && (_flags & 1)) {
		if (obj2->_objtype != kObjTypeStaticANIObject)
			return false;

		AniObject *st = static_cast<AniObject *>(obj2);

		if (!st->_statics)
			return false;

		if (st->_statics->_staticsId != _objectStaticId) {
			if (_objectStaticId)
				return false;
		}
	}

	if ((_cursorObjectId != invId && _cursorObjectId != -1 && _cursorObjectId != -2) || (!invId && _cursorObjectId == -2))
		return false;

	if (_objectState1) {
		if (_flags & 0x10) {
			if ((g_nmi->getStateId(obj1->getName()) & _objectState1) == 0)
				return false;
		} else {
			if (g_nmi->getStateId(obj1->getName()) != _objectState1)
				return false;
		}
	}

	if (_objectState2) {
		if (_flags & 0x10) {
			if ((g_nmi->getStateId(obj2->getName()) & _objectState2) == 0)
				return false;
		} else {
			if (g_nmi->getStateId(obj2->getName()) != _objectState2)
				return false;
		}
	}

	if (_heroId && (!obj1 || _heroId != obj1->_id))
		return false;

	return true;
}

bool Interact::isOverlapping(AniObject *subj, GameObject *obj) {
	if (abs(_heroOffsetX + obj->_x - subj->_x) <= 1
		&& abs(obj->_y + _heroOffsetY - subj->_y) <= 1) {
		if (!_heroStaticId || (subj->_statics != nullptr && subj->_statics->_staticsId == _heroStaticId)) {
			if (!_objectStaticId || !(_flags & 1))
				return true;

			if (obj->_objtype == kObjTypeStaticANIObject) {
				const AniObject *ani = static_cast<AniObject *>(obj);
				if (ani->_statics != nullptr && ani->_statics->_staticsId == _objectStaticId)
					return true;
			}
		}
	}
	return false;
}

bool Entrance::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Entrance::load()");

	_sceneId = file.readUint32LE();
	_id = file.readUint32LE();
	_queueId = file.readUint32LE();
	file.skip(292); // FIXME: Ugh
	_field_130 = file.readUint32LE();

	return true;
}

} // End of namespace NGI
