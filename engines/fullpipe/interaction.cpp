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

#include "fullpipe/fullpipe.h"

#include "fullpipe/interaction.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/statics.h"
#include "fullpipe/motion.h"

namespace Fullpipe {

int handleObjectInteraction(StaticANIObject *subject, GameObject *object, int invId) {
	return getGameLoaderInteractionController()->handleInteraction(subject, object, invId);
}

bool canInteractAny(GameObject *obj1, GameObject *obj2, int invId) {
	int sceneId = 0;

	if (g_fp->_currentScene)
		sceneId = g_fp->_currentScene->_sceneId;

	InteractionController *intC = getGameLoaderInteractionController();
	for (ObList::iterator i = intC->_interactions.begin(); i != intC->_interactions.end(); ++i) {
		Interaction *intr = (Interaction *)*i;

		if (intr->_sceneId > 0 && intr->_sceneId != sceneId)
			break;

		if (invId == -3) {
			invId = getGameLoaderInventory()->getSelectedItemId();
		}
		if (intr->canInteract(obj1, obj2, invId))
			return true;
	}
	return false;
}

InteractionController::~InteractionController() {
	warning("STUB: InteractionController::~InteractionController()");
}

bool InteractionController::load(MfcArchive &file) {
	debug(5, "InteractionController::load()");

	return _interactions.load(file);
}

int static_compSceneId = 0;

bool InteractionController::compareInteractions(const void *p1, const void *p2) {
	const Interaction *i1 = (const Interaction *)p1;
	const Interaction *i2 = (const Interaction *)p2;

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
	if (i2->_objectId3 == -1)
		return true;

	if (i1->_objectId3 == i2->_objectId3)
		return true;

	if (i1->_objectId3 == -1 || i1->_objectId3 == -2)
		return false;

	return true;
}

void InteractionController::sortInteractions(int sceneId) {
	static_compSceneId = sceneId;

	Common::sort(_interactions.begin(), _interactions.end(), InteractionController::compareInteractions);
}

bool InteractionController::handleInteraction(StaticANIObject *subj, GameObject *obj, int invId) {
	if (subj) {
		if (!subj->isIdle() || (subj->_flags & 0x100))
			return false;
	}

	if (!_interactions.size())
		return false;

	Interaction *inter = 0;
	Interaction *previnter = 0;
	int dur = 0;
	int mindur = 0xFFFF;

	MessageQueue *mq;
	ExCommand *ex;

	for (ObList::iterator i = _interactions.begin(); i != _interactions.end(); ++i) {
		Interaction *cinter = (Interaction *)*i;

		if (!cinter->canInteract(subj, obj, invId))
			continue;

		if ((inter || cinter->_objectId2) && (!obj || cinter->_objectId3 != obj->_id)) {
			if (cinter->_messageQueue)
				cinter->_messageQueue->calcDuration(subj);

			PicAniInfo aniInfo;

			obj->getPicAniInfo(&aniInfo);

			if (cinter->_staticsId1) {
				StaticANIObject *ani = (StaticANIObject *)obj;
				ani->_messageQueueId = 0;
				ani->changeStatics2(cinter->_staticsId1);
			}
			int xpos = cinter->_xOffs + obj->_ox;
			int ypos = cinter->_yOffs + obj->_oy;

			obj->setPicAniInfo(&aniInfo);

			if (abs(xpos - subj->_ox) > 1 || abs(ypos - subj->_oy) > 1) {
				mq = getSc2MctlCompoundBySceneId(g_fp->_currentScene->_sceneId)->doWalkTo(subj, xpos, ypos, 1, cinter->_staticsId2);
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

	if (!inter->_objectId2) {
		StaticANIObject *ani = (StaticANIObject *)obj;

		if (!ani->isIdle())
			return false;

		if (ani->_flags & 0x100)
			return false;

		if (!inter->_staticsId1 || !(inter->_flags & 1))
			goto LABEL_38;

		if (ani->_movement || ani->_statics == 0 || ani->_statics->_staticsId != inter->_staticsId1) {
			mq = ani->changeStatics1(inter->_staticsId1);
			if (!mq)
				return false;

			ex = new ExCommand((subj ? subj->_id : 0), 55, 0, 0, 0, 0, 1, 0, 0, 0);
			ex->_x = obj->_id;
			ex->_y = obj->_okeyCode;
			ex->_keyCode = subj ? subj->_okeyCode : 0;
			ex->_excFlags = 3;
			ex->_field_14 = (obj->_objtype != kObjTypePictureObject);
			ex->_field_20 = invId;
			mq->addExCommandToEnd(ex);

			if (mq->_isFinished) {
				mq->_isFinished = 0;
				ani->queueMessageQueue(mq);
			}
		} else {
			if (ani->getMessageQueue())
				ani->queueMessageQueue(0);
LABEL_38:
			if (inter->_messageQueue) {
				mq = new MessageQueue(inter->_messageQueue, 0, 1);
				mq->changeParam28ForObjectId(ani->_id, -1, ani->_okeyCode);

				if (!mq->chain(0))
					return false;
			}
		}
		return true;
	}

	if (obj && !subj)
		return true;

	if (!obj || inter->_objectId3 == obj->_id) {
		if (subj) {
			if (inter->_messageQueue) {
				if (subj->isIdle()) {
					mq = new MessageQueue(inter->_messageQueue, 0, 1);

					if (!mq->chain(subj)) {
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
			StaticANIObject *ani = (StaticANIObject *)obj;

			ani->queueMessageQueue(0);

			if (inter->_staticsId1)
				ani->changeStatics2(inter->_staticsId1);

			if (!(inter->_flags & 0x10000))
				obj->_flags |= 0x80;
		}

		if (!inter->_messageQueue)
			return false;

		subj->setOXY(inter->_xOffs + obj->_ox, inter->_yOffs + obj->_oy);

		mq = new MessageQueue(inter->_messageQueue, 0, 1);
		mq->changeParam28ForObjectId(obj->_id, -1, obj->_okeyCode);
		mq->_flags |= 1;

		if (!(inter->_flags & 0x10000)) {
			ex = new ExCommand(obj->_id, 34, 0x80, 0, 0, 0, 1, 0, 0, 0);
			ex->_keyCode = obj->_okeyCode;
			ex->_field_14 = 0x100;
			ex->_messageNum = 0;
			ex->_excFlags = 3;
			mq->addExCommandToEnd(ex);
		}

		ex = new ExCommand(obj->_id, 34, 0x100, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = obj->_okeyCode;
		ex->_field_14 = 0x100;
		ex->_messageNum = 0;
		ex->_excFlags = 3;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(subj->_id, 34, 0x100, 0, 0, 0, 1, 0, 0, 0);
		ex->_keyCode = subj->_okeyCode;
		ex->_field_14 = 0x100;
		ex->_messageNum = 0;
		ex->_excFlags = 3;
		mq->addExCommandToEnd(ex);

		ex = new ExCommand(subj->_id, 17, 0x40, 0, 0, 0, 1, 0, 0, 0);
		ex->_excFlags |= 3;
		ex->_keyCode = 0;
		mq->addExCommandToEnd(ex);

		if (!mq->chain(subj)) {
			delete mq;

			return false;
		}

		subj->_flags |= 1;
		obj->_flags |= 1;
	} else {
		bool someFlag = false;
		PicAniInfo aniInfo;

		obj->getPicAniInfo(&aniInfo);

		if (obj->_objtype == kObjTypeStaticANIObject && inter->_staticsId1) {
			StaticANIObject *ani = (StaticANIObject *)obj;

			ani->_messageQueueId = 0;
			ani->changeStatics2(inter->_staticsId1);
		}

		int xpos = inter->_yOffs + obj->_ox;
		int ypos = inter->_yOffs + obj->_oy;

		obj->setPicAniInfo(&aniInfo);

		if (abs(xpos - subj->_ox) > 1 || abs(ypos - subj->_oy) > 1
				|| (inter->_staticsId2 != 0 && (subj->_statics == 0 || subj->_statics->_staticsId != inter->_staticsId2))) {
			mq = getSc2MctlCompoundBySceneId(g_fp->_currentScene->_sceneId)->method34(subj, xpos, ypos, 1, inter->_staticsId2);

			if (!mq)
				return false;

			ex = new ExCommand(subj->_id, 55, 0, 0, 0, 0, 1, 0, 0, 0);
			ex->_x = obj->_id;
			ex->_y = obj->_okeyCode;
			ex->_keyCode = subj->_okeyCode;
			ex->_excFlags = 3;
			ex->_field_20 = invId;
			ex->_field_14 = (obj->_objtype != kObjTypePictureObject);
			mq->addExCommandToEnd(ex);

			someFlag = true;

			ex = new ExCommand(subj->_id, 17, 0x40, 0, 0, 0, 1, 0, 0, 0);
			ex->_x = xpos;
			ex->_y = ypos;
			ex->_excFlags |= 3;
			ex->_keyCode = 6;
			ex->_field_14 = obj->_id;
			ex->_field_20 = obj->_okeyCode;
			ex->postMessage();
		}

		if (!inter->_staticsId1 || !(inter->_flags & 1))
			return true;

		StaticANIObject *ani = (StaticANIObject *)obj;

		if (!ani->isIdle())
			return false;

		if (ani->getMessageQueue())
			ani->queueMessageQueue(0);

		if (!ani->_statics || ani->_statics->_staticsId != inter->_staticsId1 || ani->_movement) {
			mq = ani->changeStatics1(inter->_staticsId1);

			if (!mq)
				return false;

			if (someFlag) {
				if (!(inter->_flags & 0x10000)) {
					if (mq->_isFinished) {
						ani->_flags |= 0x80u;
					} else {
						ex = new ExCommand(ani->_id, 34, 0x80, 0, 0, 0, 1, 0, 0, 0);
						ex->_field_14 = 0x80;
						ex->_keyCode = ani->_okeyCode;
						ex->_excFlags = 3;
						mq->addExCommandToEnd(ex);
					}
				}
				ex = new ExCommand(ani->_id, 34, 0x100, 0, 0, 0, 1, 0, 0, 0);
				ex->_keyCode = ani->_okeyCode;
				ex->_field_14 = 0x100;
				ex->_excFlags = 3;
				mq->addExCommandToEnd(ex);
			} else {
				ex = new ExCommand(subj->_id, 55, 0, 0, 0, 0, 1, 0, 0, 0);
				ex->_x = ani->_id;
				ex->_y = ani->_okeyCode;
				ex->_keyCode = subj->_okeyCode;
				ex->_excFlags = 2;
				ex->_field_14 = (obj->_objtype != kObjTypePictureObject);
				ex->_field_20 = invId;
				mq->addExCommandToEnd(ex);

				if (!mq->_isFinished)
					return true;

				mq->_isFinished = 0;
				ani->queueMessageQueue(mq);
			}
		} else {
			obj->_flags |= 1;

			if (inter->_flags & 0x10000)
				return true;

			obj->_flags |= 0x80;
		}
	}

	return true;
}

Interaction *InteractionController::getInteractionByObjectIds(int obId, int obId2, int obId3) {
	for (ObList::iterator i = _interactions.begin(); i != _interactions.end(); ++i) {
		Interaction *intr = (Interaction *)*i;

		if (intr->_objectId1 == obId && intr->_objectId2 == obId2 && intr->_objectId3 == obId3)
			return intr;
	}

	return 0;
}

Interaction::Interaction() {
	_objectId1 = 0;
	_objectId2 = 0;
	_staticsId1 = 0;
	_objectId3 = 0;
	_objectState2 = 0;
	_objectState1 = 0;
	_messageQueue = 0;
	_flags = 0;
	_yOffs = 0;
	_xOffs = 0;
	_staticsId2 = 0;
	_field_28 = 0;
	_sceneId = -1;
	_actionName = 0;
}

Interaction::~Interaction() {
	warning("STUB: Interaction::~Interaction()");
}

bool Interaction::load(MfcArchive &file) {
	debug(5, "Interaction::load()");

	_objectId1 = file.readUint16LE();
	_objectId2 = file.readUint16LE();
	_staticsId1 = file.readUint16LE();
	_staticsId2 = file.readUint16LE();
	_objectId3 = file.readUint16LE();
	_objectState2 = file.readUint32LE();
	_objectState1 = file.readUint32LE();
	_xOffs = file.readUint32LE();
	_yOffs = file.readUint32LE();
	_sceneId = file.readUint32LE();
	_flags = file.readUint32LE();
	_actionName = file.readPascalString();

	_messageQueue = (MessageQueue *)file.readClass();

	return true;
}

bool Interaction::canInteract(GameObject *obj1, GameObject *obj2, int invId) {
	if (_sceneId > 0 && g_fp->_currentScene && g_fp->_currentScene->_sceneId != _sceneId)
		return false;

	if (_flags & 0x20000)
		return false;

	if (!obj2)
		return false;

	if (obj2->_id != _objectId1)
		return false;

	if ((_flags & 8) && (_flags & 1)) {
		if (obj2->_objtype != kObjTypeStaticANIObject)
			return false;

		StaticANIObject *st = (StaticANIObject *)obj2;

		if (!st->_statics)
			return false;

		if (st->_statics->_staticsId != _staticsId1) {
			if (_staticsId1)
				return false;
		}
	}

	if ((_objectId3 != invId && _objectId3 != -1 && _objectId3 != -2) || (!invId && _objectId3 == -2))
		return false;

	if (_objectState1) {
		if (_flags & 0x10) {
			if ((g_fp->getObjectState(obj1->getName()) & _objectState1) == 0)
				return false;
		} else {
			if (g_fp->getObjectState(obj1->getName()) != _objectState1)
				return false;
		}
	}

	if (_objectState2) {
		if (_flags & 0x10) {
			if ((g_fp->getObjectState(obj2->getName()) & _objectState2) == 0)
				return false;
		} else {
			if (g_fp->getObjectState(obj2->getName()) != _objectState2)
				return false;
		}
	}

	if (_objectId2 && (!obj1 || _objectId2 != obj1->_id))
		return false;

	return true;
}

bool Interaction::isOverlapping(StaticANIObject *subj, GameObject *obj) {
	StaticANIObject *ani = (StaticANIObject *)obj;

	if (abs(_xOffs + obj->_ox - subj->_ox) <= 1
		&& abs(obj->_oy + _yOffs - subj->_oy) <= 1) {
		if (!_staticsId2 || (subj->_statics != 0 && subj->_statics->_staticsId == _staticsId2)) {
			if (!_staticsId1 || !(_flags & 1) || (ani->_statics != 0 && ani->_statics->_staticsId == _staticsId1))
				return true;
		}
	}
	return false;
}

bool EntranceInfo::load(MfcArchive &file) {
	debug(5, "EntranceInfo::load()");

	_sceneId = file.readUint32LE();
	_field_4 = file.readUint32LE();
	_messageQueueId = file.readUint32LE();
	file.read(_gap_C, 292); // FIXME, Ugh
	_field_130 = file.readUint32LE();

	return true;
}

} // End of namespace Fullpipe
