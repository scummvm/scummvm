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

#ifndef FULLPIPE_INTERACTION_H
#define FULLPIPE_INTERACTION_H

#include "fullpipe/utils.h"

namespace Fullpipe {

class GameObject;
class MessageQueue;
class StaticANIObject;

int handleObjectInteraction(StaticANIObject *subject, GameObject *object, int invId);
bool canInteractAny(GameObject *obj1, GameObject *obj2, int invId);


class Interaction : public CObject {
 public:
	int16 _objectId1;
	int16 _objectId2;
	int16 _objectId3;
	int16 _staticsId1;
	int16 _staticsId2;
	int _objectState1;
	int _objectState2;
	int _xOffs;
	int _yOffs;
	MessageQueue *_messageQueue;
	int _sceneId;
	int _field_28;
	int _flags;
	char *_actionName;

 public:
	Interaction();
	virtual ~Interaction();

	virtual bool load(MfcArchive &file);
	bool canInteract(GameObject *obj1, GameObject *obj2, int invId);
	bool isOverlapping(StaticANIObject *subj, GameObject *obj);
};

class InteractionController : public CObject {
 public:
	ObList _interactions;
	int16 _field_20;
	bool _flag24;

 private:
	static bool compareInteractions(const void *p1, const void *p2);

 public:
	InteractionController() : _field_20(0), _flag24(true) {}
	virtual ~InteractionController();

	virtual bool load(MfcArchive &file);

	void enableFlag24() { _flag24 = true; }
	void disableFlag24() { _flag24 = false; }

	void sortInteractions(int sceneId);

	bool handleInteraction(StaticANIObject *subj, GameObject *obj, int invId);

	Interaction *getInteractionByObjectIds(int obId, int obId2, int obId3);
};

struct EntranceInfo {
	int32 _sceneId;
	int32 _field_4;
	int32 _messageQueueId;
	byte _gap_C[292]; // FIXME
	int32 _field_130;

	bool load(MfcArchive &file);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_INTERACTION_H */
