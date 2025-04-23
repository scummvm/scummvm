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

#ifndef LASTEXPRESS_SAVEPOINT_H
#define LASTEXPRESS_SAVEPOINT_H

#include "lastexpress/helpers.h"
#include "lastexpress/shared.h"

#include "common/array.h"
#include "common/list.h"
#include "common/serializer.h"

/*
	Savepoint format
	----------------

	Save point: max: 127 - FIFO list (ie. goes back and overwrites first save point when full)
	    uint32 {4}      - Entity 1
	    uint32 {4}      - Action
	    uint32 {4}      - Entity 2
	    uint32 {4}      - Parameter

	Save point Data
	    uint32 {4}      - Entity 1
	    uint32 {4}      - Action
	    uint32 {4}      - Entity 2
	    uint32 {4}      - function pointer to ??

*/

namespace LastExpress {

class LastExpressEngine;

struct SavePoint {
	CharacterIndex entity1;
	CharacterActions action;
	CharacterIndex entity2;
	union {
		uint32 intValue;
		char charValue[7]; // "MUS%03d" with terminating zero
	} param;

	SavePoint() {
		entity1 = kCharacterCath;
		action = kCharacterActionNone;
		entity2 = kCharacterCath;
		param.intValue = 0;
		param.charValue[6] = 0;
	}

	Common::String toString() {
		return Common::String::format("{ %s - %d - %s - %s }", ENTITY_NAME(entity1), action, ENTITY_NAME(entity2), param.charValue);
	}
};

typedef Common::Functor1<const SavePoint&, void> Callback;

class SavePoints : Common::Serializable {
public:

	struct SavePointData {
		CharacterIndex entity1;
		CharacterActions action;
		CharacterIndex entity2;
		uint32 param;

		SavePointData() {
			entity1 = kCharacterCath;
			action = kCharacterActionNone;
			entity2 = kCharacterCath;
			param = 0;
		}

		Common::String toString() {
			return Common::String::format(" { %s - %d - %s - %d }", ENTITY_NAME(entity1), action, ENTITY_NAME(entity2), param);
		}
	};

	SavePoints(LastExpressEngine *engine);
	~SavePoints() override;

	// Savepoints
	void push(CharacterIndex entity2, CharacterIndex entity1, CharacterActions action, uint32 param = 0);
	void push(CharacterIndex entity2, CharacterIndex entity1, CharacterActions action, const Common::String param);
	void pushAll(CharacterIndex entity, CharacterActions action, uint32 param = 0);
	void process();
	void reset();

	// Data
	void addData(CharacterIndex entity, CharacterActions action, uint32 param);

	// Callbacks
	void setCallback(CharacterIndex index, Callback *callback);
	Callback *getCallback(CharacterIndex entity) const;
	void call(CharacterIndex entity2, CharacterIndex entity1, CharacterActions action, uint32 param = 0) const;
	void call(CharacterIndex entity2, CharacterIndex entity1, CharacterActions action, const Common::String param) const;
	void callAndProcess();

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s) override;

	/**
	 * Convert this object into a string representation.
	 *
	 * @return A string representation of this object.
	 */
	Common::String toString();

	uint32 count() { return _savepoints.size(); }

private:
	static const uint32 _savePointsMaxSize = 128;

	LastExpressEngine *_engine;

	Common::List<SavePoint> _savepoints;    ///< could be a queue, but we need to be able to iterate on the items
	Common::Array<SavePointData> _data;
	Callback *_callbacks[40];

	SavePoint pop();
	bool updateEntityFromData(const SavePoint &point);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SAVEPOINT_H
