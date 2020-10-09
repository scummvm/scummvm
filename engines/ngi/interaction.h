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

#ifndef NGI_INTERACTION_H
#define NGI_INTERACTION_H

#include "ngi/utils.h"

namespace NGI {

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
	uint _flags;
	Common::String _actionName;

 public:
	Interaction();
	~Interaction() override;

	bool load(MfcArchive &file) override;
	bool canInteract(GameObject *obj1, GameObject *obj2, int invId);
	bool isOverlapping(StaticANIObject *subj, GameObject *obj);
};

class InteractionController : public CObject {
	friend bool canInteractAny(GameObject *obj1, GameObject *obj2, int invId);

public:
	typedef ObList<Interaction> InteractionList;
	bool _flag24;

 private:
	InteractionList _interactions;
	static bool compareInteractions(const Interaction *i1, const Interaction *i2);

 public:
	InteractionController() : _flag24(true) {}
	~InteractionController() override;

	bool load(MfcArchive &file) override;

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
	int32 _field_130;

	bool load(MfcArchive &file);
};

} // End of namespace NGI

#endif /* NGI_INTERACTION_H */
