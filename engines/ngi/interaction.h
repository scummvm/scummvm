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

#ifndef NGI_INTERACTION_H
#define NGI_INTERACTION_H

#include "ngi/utils.h"

namespace NGI {

class GameObject;
class MessageQueue;
class AniObject;

int handleObjectInteraction(AniObject *subject, GameObject *object, int invId);
bool canInteractAny(GameObject *obj1, GameObject *obj2, int invId);


class Interact : public CObject {
 public:
	int16 _objectId;
	int16 _heroId;
	int16 _cursorObjectId;
	int16 _objectStaticId;
	int16 _heroStaticId;
	int _objectState1;
	int _objectState2;
	int _heroOffsetX;
	int _heroOffsetY;
	MessageQueue *_messageQueue;
	int _sceneId;
	int _field_28;
	uint _flags;
	Common::String _actionName;

 public:
	Interact();
	~Interact() override;

	bool load(MfcArchive &file) override;
	bool canRun(GameObject *obj1, GameObject *obj2, int invId);
	bool isOverlapping(AniObject *subj, GameObject *obj);
};

class InteractionController : public CObject {
	friend bool canInteractAny(GameObject *obj1, GameObject *obj2, int invId);

public:
	typedef ObList<Interact> InteractionList;
	bool _enabled;

 private:
	InteractionList _interactions;
	static bool compareInteractions(const Interact *i1, const Interact *i2);

 public:
	InteractionController() : _enabled(true) {}
	~InteractionController() override;

	bool load(MfcArchive &file) override;

	void enable() { _enabled = true; }
	void disable() { _enabled = false; }

	void sortInteractions(int sceneId);

	bool handleInteraction(AniObject *subj, GameObject *obj, int invId);

	Interact *findInteraction(int obId, int obId2, int obId3);
};

struct Entrance {
	int32 _sceneId;
	int32 _id;
	int32 _queueId;
	int32 _field_130;

	bool load(MfcArchive &file);
};

} // End of namespace NGI

#endif /* NGI_INTERACTION_H */
