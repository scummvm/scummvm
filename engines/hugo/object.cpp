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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/object.h"
#include "hugo/display.h"
#include "hugo/file.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"
#include "hugo/text.h"
#include "hugo/inventory.h"
#include "hugo/mouse.h"

namespace Hugo {

ObjectHandler::ObjectHandler(HugoEngine *vm) : _vm(vm), _objects(0), _uses(0) {
	_numObj = 0;
	_objCount = 0;
	_usesSize = 0;
	memset(_objBound, '\0', sizeof(overlay_t));
	memset(_boundary, '\0', sizeof(overlay_t));
	memset(_overlay,  '\0', sizeof(overlay_t));
	memset(_ovlBase,  '\0', sizeof(overlay_t));
}

ObjectHandler::~ObjectHandler() {
}

byte ObjectHandler::getBoundaryOverlay(uint16 index) const {
	return _boundary[index];
}

byte ObjectHandler::getObjectBoundary(uint16 index) const {
	return _objBound[index];
}

byte ObjectHandler::getBaseBoundary(uint16 index) const {
	return _ovlBase[index];
}

byte ObjectHandler::getFirstOverlay(uint16 index) const {
	return _overlay[index];
}

bool ObjectHandler::isCarried(int objIndex) const {
	return _objects[objIndex].carriedFl;
}

void ObjectHandler::setCarry(int objIndex, bool val) {
	_objects[objIndex].carriedFl = val;
}

void ObjectHandler::setVelocity(int objIndex, int8 vx, int8 vy) {
	_objects[objIndex].vx = vx;
	_objects[objIndex].vy = vy;
}

void ObjectHandler::setPath(int objIndex, path_t pathType, int16 vxPath, int16 vyPath) {
	_objects[objIndex].pathType = pathType;
	_objects[objIndex].vxPath = vxPath;
	_objects[objIndex].vyPath = vyPath;
}

/**
 * Save sequence number and image number in given object
 */
void ObjectHandler::saveSeq(object_t *obj) {
	debugC(1, kDebugObject, "saveSeq");

	bool found = false;
	for (int i = 0; !found && (i < obj->seqNumb); i++) {
		seq_t *q = obj->seqList[i].seqPtr;
		for (int j = 0; !found && (j < obj->seqList[i].imageNbr); j++) {
			if (obj->currImagePtr == q) {
				found = true;
				obj->curSeqNum = i;
				obj->curImageNum = j;
			} else {
				q = q->nextSeqPtr;
			}
		}
	}
}

/**
 * Set up cur_seq_p from stored sequence and image number in object
 */
void ObjectHandler::restoreSeq(object_t *obj) {
	debugC(1, kDebugObject, "restoreSeq");

	seq_t *q = obj->seqList[obj->curSeqNum].seqPtr;
	for (int j = 0; j < obj->curImageNum; j++)
		q = q->nextSeqPtr;
	obj->currImagePtr = q;
}

/**
 * If status.objid = -1, pick up objid, else use status.objid on objid,
 * if objid can't be picked up, use it directly
 */
void ObjectHandler::useObject(int16 objId) {
	debugC(1, kDebugObject, "useObject(%d)", objId);

	const char *verb;                               // Background verb to use directly
	int16 inventObjId = _vm->_inventory->getInventoryObjId();
	object_t *obj = &_objects[objId];               // Ptr to object
	if (inventObjId == -1) {
		// Get or use objid directly
		if ((obj->genericCmd & TAKE) || obj->objValue)  // Get collectible item
			sprintf(_vm->_line, "%s %s", _vm->_text->getVerb(_vm->_take, 0), _vm->_text->getNoun(obj->nounIndex, 0));
		else if (obj->cmdIndex != 0)                // Use non-collectible item if able
			sprintf(_vm->_line, "%s %s", _vm->_text->getVerb(_vm->_parser->getCmdDefaultVerbIdx(obj->cmdIndex), 0), _vm->_text->getNoun(obj->nounIndex, 0));
		else if ((verb = _vm->_parser->useBG(_vm->_text->getNoun(obj->nounIndex, 0))) != 0)
			sprintf(_vm->_line, "%s %s", verb, _vm->_text->getNoun(obj->nounIndex, 0));
		else
			return;                                 // Can't use object directly
	} else {
		// Use status.objid on objid
		// Default to first cmd verb
		sprintf(_vm->_line, "%s %s %s", _vm->_text->getVerb(_vm->_parser->getCmdDefaultVerbIdx(_objects[inventObjId].cmdIndex), 0),
			                       _vm->_text->getNoun(_objects[inventObjId].nounIndex, 0),
			                       _vm->_text->getNoun(obj->nounIndex, 0));

		// Check valid use of objects and override verb if necessary
		for (uses_t *use = _uses; use->objId != _numObj; use++) {
			if (inventObjId == use->objId) {
				// Look for secondary object, if found use matching verb
				bool foundFl = false;

				for (target_t *target = use->targets; target->nounIndex != 0; target++)
					if (target->nounIndex == obj->nounIndex) {
						foundFl = true;
						sprintf(_vm->_line, "%s %s %s", _vm->_text->getVerb(target->verbIndex, 0),
							                       _vm->_text->getNoun(_objects[inventObjId].nounIndex, 0),
							                       _vm->_text->getNoun(obj->nounIndex, 0));
					}

				// No valid use of objects found, print failure string
				if (!foundFl) {
					// Deselect dragged icon if inventory not active
					if (_vm->_inventory->getInventoryState() != kInventoryActive)
						_vm->_screen->resetInventoryObjId();
					Utils::notifyBox(_vm->_text->getTextData(use->dataIndex));
					return;
				}
			}
		}
	}

	if (_vm->_inventory->getInventoryState() == kInventoryActive) // If inventory active, remove it
		_vm->_inventory->setInventoryState(kInventoryUp);

	_vm->_screen->resetInventoryObjId();

	_vm->_parser->lineHandler();                    // and process command
}

/**
 * Return object index of the topmost object under the cursor, or -1 if none
 * Objects are filtered if not "useful"
 */
int16 ObjectHandler::findObject(uint16 x, uint16 y) {
	debugC(3, kDebugObject, "findObject(%d, %d)", x, y);

	int16     objIndex = -1;                        // Index of found object
	uint16    y2Max = 0;                            // Greatest y2
	object_t *obj = _objects;
	// Check objects on screen
	for (int i = 0; i < _numObj; i++, obj++) {
		// Object must be in current screen and "useful"
		if (obj->screenIndex == *_vm->_screen_p && (obj->genericCmd || obj->objValue || obj->cmdIndex)) {
			seq_t *curImage = obj->currImagePtr;
			// Object must have a visible image...
			if (curImage != 0 && obj->cycling != kCycleInvisible) {
				// If cursor inside object
				if (x >= (uint16)obj->x && x <= obj->x + curImage->x2 && y >= (uint16)obj->y && y <= obj->y + curImage->y2) {
					// If object is closest so far
					if (obj->y + curImage->y2 > y2Max) {
						y2Max = obj->y + curImage->y2;
						objIndex = i;               // Found an object!
					}
				}
			} else {
				// ...or a dummy object that has a hotspot rectangle
				if (curImage == 0 && obj->vxPath != 0 && !obj->carriedFl) {
					// If cursor inside special rectangle
					if ((int16)x >= obj->oldx && (int16)x < obj->oldx + obj->vxPath && (int16)y >= obj->oldy && (int16)y < obj->oldy + obj->vyPath) {
						// If object is closest so far
						if (obj->oldy + obj->vyPath - 1 > (int16)y2Max) {
							y2Max = obj->oldy + obj->vyPath - 1;
							objIndex = i;           // Found an object!
						}
					}
				}
			}
		}
	}
	return objIndex;
}

/**
 * Issue "Look at <object>" command
 * Note special case of swapped hero image
 */
void ObjectHandler::lookObject(object_t *obj) {
	debugC(1, kDebugObject, "lookObject");

	if (obj == _vm->_hero)
		// Hero swapped - look at other
		obj = &_objects[_vm->_heroImage];

	_vm->_parser->command("%s %s", _vm->_text->getVerb(_vm->_look, 0), _vm->_text->getNoun(obj->nounIndex, 0));
}

/**
 * Free all object images, uses and ObjArr (before exiting)
 */
void ObjectHandler::freeObjects() {
	debugC(1, kDebugObject, "freeObjects");

	if (_vm->_hero != 0 && _vm->_hero->seqList[0].seqPtr != 0) {
		// Free all sequence lists and image data
		for (int16 i = 0; i < _numObj; i++) {
			object_t *obj = &_objects[i];
			for (int16 j = 0; j < obj->seqNumb; j++) {
				seq_t *seq = obj->seqList[j].seqPtr;
				seq_t *next;
				if (seq == 0) // Failure during database load
					break;
				if (seq->imagePtr != 0) {
					free(seq->imagePtr);
					seq->imagePtr = 0;
				}
				seq = seq->nextSeqPtr;
				while (seq != obj->seqList[j].seqPtr) {
					if (seq->imagePtr != 0) {
						free(seq->imagePtr);
						seq->imagePtr = 0;
					}
					next = seq->nextSeqPtr;
					free(seq);
					seq = next;
				}
				free(seq);
			}
		}
	}

	if (_uses) {
		for (int16 i = 0; i < _usesSize; i++)
			free(_uses[i].targets);
		free(_uses);
	}

	for(int16 i = 0; i < _objCount; i++) {
		free(_objects[i].stateDataIndex);
		_objects[i].stateDataIndex = 0;
	}

	free(_objects);
	_objects = 0;
}

/**
 * Compare function for the quicksort.  The sort is to order the objects in
 * increasing vertical position, using y+y2 as the baseline
 * Returns -1 if ay2 < by2 else 1 if ay2 > by2 else 0
 */
int ObjectHandler::y2comp(const void *a, const void *b) {
	debugC(6, kDebugObject, "y2comp");

	const object_t *p1 = &HugoEngine::get()._object->_objects[*(const byte *)a];
	const object_t *p2 = &HugoEngine::get()._object->_objects[*(const byte *)b];

	if (p1 == p2)
		// Why does qsort try the same indexes?
		return 0;

	if (p1->priority == kPriorityBackground)
		return -1;

	if (p2->priority == kPriorityBackground)
		return 1;

	if (p1->priority == kPriorityForeground)
		return 1;

	if (p2->priority == kPriorityForeground)
		return -1;

	int ay2 = p1->y + p1->currImagePtr->y2;
	int by2 = p2->y + p2->currImagePtr->y2;

	return ay2 - by2;
}

/**
 * Return TRUE if object being carried by hero
 */
bool ObjectHandler::isCarrying(uint16 wordIndex) {
	debugC(1, kDebugObject, "isCarrying(%d)", wordIndex);

	for (int i = 0; i < _numObj; i++) {
		if ((wordIndex == _objects[i].nounIndex) && _objects[i].carriedFl)
			return true;
	}
	return false;
}

/**
 * Describe any takeable objects visible in this screen
 */
void ObjectHandler::showTakeables() {
	debugC(1, kDebugObject, "showTakeables");

	for (int j = 0; j < _numObj; j++) {
		object_t *obj = &_objects[j];
		if ((obj->cycling != kCycleInvisible) &&
		    (obj->screenIndex == *_vm->_screen_p) &&
		    (((TAKE & obj->genericCmd) == TAKE) || obj->objValue)) {
			Utils::notifyBox(Common::String::format("You can also see:\n%s.", _vm->_text->getNoun(obj->nounIndex, LOOK_NAME)));
		}
	}
}

/**
 * Find a clear space around supplied object that hero can walk to
 */
bool ObjectHandler::findObjectSpace(object_t *obj, int16 *destx, int16 *desty) {
	debugC(1, kDebugObject, "findObjectSpace(obj, %d, %d)", *destx, *desty);

	seq_t *curImage = obj->currImagePtr;
	int16 y = obj->y + curImage->y2 - 1;

	bool foundFl = true;
	// Try left rear corner
	for (int16 x = *destx = obj->x + curImage->x1; x < *destx + kHeroMaxWidth; x++) {
		if (checkBoundary(x, y))
			foundFl = false;
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->x + curImage->x2 - kHeroMaxWidth + 1; x <= obj->x + (int16)curImage->x2; x++) {
			if (checkBoundary(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try left front corner
		foundFl = true;
		y += 2;
		for (int16 x = *destx = obj->x + curImage->x1; x < *destx + kHeroMaxWidth; x++) {
			if (checkBoundary(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->x + curImage->x2 - kHeroMaxWidth + 1; x <= obj->x + (int16)curImage->x2; x++) {
			if (checkBoundary(x, y))
				foundFl = false;
		}
	}

	*desty = y;
	return foundFl;
}

void ObjectHandler::readUse(Common::ReadStream &in, uses_t &curUse) {
	curUse.objId = in.readSint16BE();
	curUse.dataIndex = in.readUint16BE();
	uint16 numSubElem = in.readUint16BE();
	curUse.targets = (target_t *)malloc(sizeof(target_t) * numSubElem);
	for (int j = 0; j < numSubElem; j++) {
		curUse.targets[j].nounIndex = in.readUint16BE();
		curUse.targets[j].verbIndex = in.readUint16BE();
	}
}
/**
 * Load _uses from Hugo.dat
 */
void ObjectHandler::loadObjectUses(Common::ReadStream &in) {
	uses_t tmpUse;
	tmpUse.targets = 0;

	//Read _uses
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_usesSize = numElem;
			_uses = (uses_t *)malloc(sizeof(uses_t) * numElem);
		}

		for (int i = 0; i < numElem; i++) {
			if (varnt == _vm->_gameVariant)
				readUse(in, _uses[i]);
			else {
				readUse(in, tmpUse);
				free(tmpUse.targets);
				tmpUse.targets = 0;
			}
		}
	}
}

void ObjectHandler::readObject(Common::ReadStream &in, object_t &curObject) {
	curObject.nounIndex = in.readUint16BE();
	curObject.dataIndex = in.readUint16BE();
	uint16 numSubElem = in.readUint16BE();

	if (numSubElem == 0)
		curObject.stateDataIndex = 0;
	else
		curObject.stateDataIndex = (uint16 *)malloc(sizeof(uint16) * numSubElem);
	for (int j = 0; j < numSubElem; j++)
		curObject.stateDataIndex[j] = in.readUint16BE();

	curObject.pathType = (path_t) in.readSint16BE();
	curObject.vxPath = in.readSint16BE();
	curObject.vyPath = in.readSint16BE();
	curObject.actIndex = in.readUint16BE();
	curObject.seqNumb = in.readByte();
	curObject.currImagePtr = 0;

	if (curObject.seqNumb == 0) {
		curObject.seqList[0].imageNbr = 0;
		curObject.seqList[0].seqPtr = 0;
	}

	for (int j = 0; j < curObject.seqNumb; j++) {
		curObject.seqList[j].imageNbr = in.readUint16BE();
		curObject.seqList[j].seqPtr = 0;
	}

	curObject.cycling = (cycle_t)in.readByte();
	curObject.cycleNumb = in.readByte();
	curObject.frameInterval = in.readByte();
	curObject.frameTimer = in.readByte();
	curObject.radius = in.readByte();
	curObject.screenIndex = in.readByte();
	curObject.x = in.readSint16BE();
	curObject.y = in.readSint16BE();
	curObject.oldx = in.readSint16BE();
	curObject.oldy = in.readSint16BE();
	curObject.vx = in.readByte();
	curObject.vy = in.readByte();
	curObject.objValue = in.readByte();
	curObject.genericCmd = in.readSint16BE();
	curObject.cmdIndex = in.readUint16BE();
	curObject.carriedFl = (in.readByte() != 0);
	curObject.state = in.readByte();
	curObject.verbOnlyFl = (in.readByte() != 0);
	curObject.priority = in.readByte();
	curObject.viewx = in.readSint16BE();
	curObject.viewy = in.readSint16BE();
	curObject.direction = in.readSint16BE();
	curObject.curSeqNum = in.readByte();
	curObject.curImageNum = in.readByte();
	curObject.oldvx = in.readByte();
	curObject.oldvy = in.readByte();
}
/**
 * Load ObjectArr from Hugo.dat
 */
void ObjectHandler::loadObjectArr(Common::ReadStream &in) {
	debugC(6, kDebugObject, "loadObject(&in)");
	object_t tmpObject;
	tmpObject.stateDataIndex = 0;

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();

		if (varnt == _vm->_gameVariant) {
			_objCount = numElem;
			_objects = (object_t *)malloc(sizeof(object_t) * numElem);
		}

		for (int i = 0; i < numElem; i++) {
			if (varnt == _vm->_gameVariant)
				readObject(in, _objects[i]);
			else {
				// Skip over uneeded objects.
				readObject(in, tmpObject);
				free(tmpObject.stateDataIndex);
				tmpObject.stateDataIndex = 0;
			}
		}
	}
}

/**
 * Set the screenindex property of the carried objets to the given screen
 * number
 */
void ObjectHandler::setCarriedScreen(int screenNum) {
	for (int i = kHeroIndex + 1; i < _numObj; i++) {// Any others
		if (isCarried(i))                           // being carried
			_objects[i].screenIndex = screenNum;
	}
}

/**
 * Load _numObj from Hugo.dat
 */
void ObjectHandler::loadNumObj(Common::ReadStream &in) {
	int numElem;

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant)
			_numObj = numElem;
	}
}

/**
 * Restore all sequences
 */
void ObjectHandler::restoreAllSeq() {
	// Restore ptrs to currently loaded objects
	for (int i = 0; i < _numObj; i++)
		restoreSeq(&_objects[i]);
}

/**
 * Save objects
 */
void ObjectHandler::saveObjects(Common::WriteStream *out) {
	for (int i = 0; i < _numObj; i++) {
		// Save where curr_seq_p is pointing to
		saveSeq(&_objects[i]);

		out->writeByte(_objects[i].pathType);
		out->writeSint16BE(_objects[i].vxPath);
		out->writeSint16BE(_objects[i].vyPath);
		out->writeByte(_objects[i].cycling);
		out->writeByte(_objects[i].cycleNumb);
		out->writeByte(_objects[i].frameTimer);
		out->writeByte(_objects[i].screenIndex);
		out->writeSint16BE(_objects[i].x);
		out->writeSint16BE(_objects[i].y);
		out->writeSint16BE(_objects[i].oldx);
		out->writeSint16BE(_objects[i].oldy);
		out->writeSByte(_objects[i].vx);
		out->writeSByte(_objects[i].vy);
		out->writeByte(_objects[i].objValue);
		out->writeByte((_objects[i].carriedFl) ? 1 : 0);
		out->writeByte(_objects[i].state);
		out->writeByte(_objects[i].priority);
		out->writeSint16BE(_objects[i].viewx);
		out->writeSint16BE(_objects[i].viewy);
		out->writeSint16BE(_objects[i].direction);
		out->writeByte(_objects[i].curSeqNum);
		out->writeByte(_objects[i].curImageNum);
		out->writeSByte(_objects[i].oldvx);
		out->writeSByte(_objects[i].oldvy);
	}
}

/**
 * Restore objects
 */
void ObjectHandler::restoreObjects(Common::SeekableReadStream *in) {
	for (int i = 0; i < _numObj; i++) {
		_objects[i].pathType = (path_t) in->readByte();
		_objects[i].vxPath = in->readSint16BE();
		_objects[i].vyPath = in->readSint16BE();
		_objects[i].cycling = (cycle_t) in->readByte();
		_objects[i].cycleNumb = in->readByte();
		_objects[i].frameTimer = in->readByte();
		_objects[i].screenIndex = in->readByte();
		_objects[i].x = in->readSint16BE();
		_objects[i].y = in->readSint16BE();
		_objects[i].oldx = in->readSint16BE();
		_objects[i].oldy = in->readSint16BE();
		_objects[i].vx = in->readSByte();
		_objects[i].vy = in->readSByte();
		_objects[i].objValue = in->readByte();
		_objects[i].carriedFl = (in->readByte() == 1);
		_objects[i].state = in->readByte();
		_objects[i].priority = in->readByte();
		_objects[i].viewx = in->readSint16BE();
		_objects[i].viewy = in->readSint16BE();
		_objects[i].direction = in->readSint16BE();
		_objects[i].curSeqNum = in->readByte();
		_objects[i].curImageNum = in->readByte();
		_objects[i].oldvx = in->readSByte();
		_objects[i].oldvy = in->readSByte();
	}
}

/**
 * Compute max object score
 */
int ObjectHandler::calcMaxScore() {
	int score = 0;
	for (int i = 0; i < _numObj; i++)
		score += _objects[i].objValue;
	return score;
}

/**
 * Read Object images
 */
void ObjectHandler::readObjectImages() {
	debugC(1, kDebugObject, "readObjectImages");

	for (int i = 0; i < _numObj; i++)
		_vm->_file->readImage(i, &_objects[i]);
}

bool ObjectHandler::checkBoundary(int16 x, int16 y) {
	// Check if Boundary bit set
	return (_boundary[y * kCompLineSize + x / 8] & (0x80 >> x % 8)) != 0;
}

/**
 * Return maximum allowed movement (from zero to vx) such that object does
 * not cross a boundary (either background or another object)
 */
int ObjectHandler::deltaX(const int x1, const int x2, const int vx, int y) const {
// Explanation of algorithm:  The boundaries are drawn as contiguous
// lines 1 pixel wide.  Since DX,DY are not necessarily 1, we must
// detect boundary crossing.  If vx positive, examine each pixel from
// x1 old to x2 new, else x2 old to x1 new, both at the y2 line.
// If vx zero, no need to check.  If vy non-zero then examine each
// pixel on the line segment x1 to x2 from y old to y new.
// Fix from Hugo I v1.5:
// Note the diff is munged in the return statement to cater for a special
// cases arising from differences in image widths from one sequence to
// another.  The problem occurs reversing direction at a wall where the
// new image intersects before the object can move away.  This is cured
// by comparing the intersection with half the object width pos. If the
// intersection is in the other half wrt the intended direction, use the
// desired vx, else use the computed delta.  i.e. believe the desired vx

	debugC(3, kDebugEngine, "deltaX(%d, %d, %d, %d)", x1, x2, vx, y);

	if (vx == 0)
		return 0;                                  // Object stationary

	y *= kCompLineSize;                             // Offset into boundary file
	if (vx > 0) {
		// Moving to right
		for (int i = x1 >> 3; i <= (x2 + vx) >> 3; i++) {// Search by byte
			int b = Utils::firstBit((byte)(_boundary[y + i] | _objBound[y + i]));
			if (b < 8) {   // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1) && (b <= x2 + vx))
					return (b < x1 + ((x2 - x1) >> 1)) ? vx : b - x2 - 1; // return dx
			}
		}
	} else {
		// Moving to left
		for (int i = x2 >> 3; i >= (x1 + vx) >> 3; i--) {// Search by byte
			int b = Utils::lastBit((byte)(_boundary[y + i] | _objBound[y + i]));
			if (b < 8) {    // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1 + vx) && (b <= x2))
					return (b > x1 + ((x2 - x1) >> 1)) ? vx : b - x1 + 1; // return dx
			}
		}
	}
	return vx;
}

/**
 * Similar to Delta_x, but for movement in y direction.  Special case of
 * bytes at end of line segment; must only count boundary bits falling on
 * line segment.
 */
int ObjectHandler::deltaY(const int x1, const int x2, const int vy, const int y) const {
	debugC(3, kDebugEngine, "deltaY(%d, %d, %d, %d)", x1, x2, vy, y);

	if (vy == 0)
		return 0;                                   // Object stationary

	int inc = (vy > 0) ? 1 : -1;
	for (int j = y + inc; j != (y + vy + inc); j += inc) { //Search by byte
		for (int i = x1 >> 3; i <= x2 >> 3; i++) {
			int b = _boundary[j * kCompLineSize + i] | _objBound[j * kCompLineSize + i];
			if (b != 0) {                           // Any bit set
				// Make sure boundary bits fall on line segment
				if (i == (x2 >> 3))                 // Adjust right end
					b &= 0xff << ((i << 3) + 7 - x2);
				else if (i == (x1 >> 3))            // Adjust left end
					b &= 0xff >> (x1 - (i << 3));
				if (b)
					return j - y - inc;
			}
		}
	}
	return vy;
}

/**
 * Store a horizontal line segment in the object boundary file
 */
void ObjectHandler::storeBoundary(const int x1, const int x2, const int y) {
	debugC(5, kDebugEngine, "storeBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b |= 0xff << ((i << 3) + 7 - x2);
		else if (i == x1 >> 3)                      // Adjust left end
			*b |= 0xff >> (x1 - (i << 3));
		else
			*b = 0xff;
	}
}

/**
 * Clear a horizontal line segment in the object boundary file
 */
void ObjectHandler::clearBoundary(const int x1, const int x2, const int y) {
	debugC(5, kDebugEngine, "clearBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

/**
 * Clear a horizontal line segment in the screen boundary file
 * Used to fix some data issues
 */
void ObjectHandler::clearScreenBoundary(const int x1, const int x2, const int y) {
	debugC(5, kDebugEngine, "clearScreenBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_boundary[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

/**
 * An object has collided with a boundary. See if any actions are required
 */
void ObjectHandler::boundaryCollision(object_t *obj) {
	debugC(1, kDebugEngine, "boundaryCollision");

	if (obj == _vm->_hero) {
		// Hotspots only relevant to HERO
		int x;
		if (obj->vx > 0)
			x = obj->x + obj->currImagePtr->x2;
		else
			x = obj->x + obj->currImagePtr->x1;
		int y = obj->y + obj->currImagePtr->y2;

		int16 index = _vm->_mouse->findExit(x, y, obj->screenIndex);
		if (index >= 0)
			_vm->_scheduler->insertActionList(_vm->_mouse->getHotspotActIndex(index));

	} else {
		// Check whether an object collided with HERO
		int dx = _vm->_hero->x + _vm->_hero->currImagePtr->x1 - obj->x - obj->currImagePtr->x1;
		int dy = _vm->_hero->y + _vm->_hero->currImagePtr->y2 - obj->y - obj->currImagePtr->y2;
		// If object's radius is infinity, use a closer value
		int8 radius = obj->radius;
		if (radius < 0)
			radius = kStepDx * 2;
		if ((abs(dx) <= radius) && (abs(dy) <= radius))
			_vm->_scheduler->insertActionList(obj->actIndex);
	}
}

} // End of namespace Hugo
