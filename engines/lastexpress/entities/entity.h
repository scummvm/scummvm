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

#ifndef LASTEXPRESS_ENTITY_H
#define LASTEXPRESS_ENTITY_H

#include "lastexpress/shared.h"

#include "lastexpress/sound/sound.h"

#include "lastexpress/helpers.h"

#include "common/array.h"
#include "common/func.h"
#include "common/serializer.h"
#include "common/textconsole.h"

namespace LastExpress {

class LastExpressEngine;
class Sequence;
class SequenceFrame;
struct SavePoint;

class EntityData : Common::Serializable {
public:

	struct EntityParameters : Common::Serializable{
		virtual ~EntityParameters() {}
		virtual Common::String toString() = 0;

		virtual void update(uint32 index) = 0;

		virtual void saveLoadWithSerializer(Common::Serializer &s) = 0;
	};

	struct EntityParametersIIII : EntityParameters {
		uint param1;
		uint param2;
		uint param3;
		uint param4;
		uint param5;
		uint param6;
		uint param7;
		uint param8;

		EntityParametersIIII() {
			param1 = 0;
			param2 = 0;
			param3 = 0;
			param4 = 0;
			param5 = 0;
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		bool hasNonNullParameter() {
			return param1 || param2 || param3 || param4 || param5 || param6 || param7 || param8;
		}

		Common::String toString() {
			return Common::String::format("IIII: %d %d %d %d %d %d %d %d\n", param1, param2, param3, param4, param5, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIIII::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			case 2: param3 = 1; break;
			case 3: param4 = 1; break;
			case 4: param5 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncAsUint32LE(param3);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSIII : EntityParameters {
		char seq[12];
		uint param4;
		uint param5;
		uint param6;
		uint param7;
		uint param8;

		EntityParametersSIII() {
			memset(&seq, 0, 12);
			param4 = 0;
			param5 = 0;
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("SIII: %s %d %d %d %d %d\n", seq, param4, param5, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersSIII::update] Invalid index (was: %d)", index);

			case 3: param4 = 1; break;
			case 4: param5 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSIIS : EntityParameters {
		char seq1[12];
		uint param4;
		uint param5;
		char seq2[12];

		EntityParametersSIIS() {
			memset(&seq1, 0, 12);
			param4 = 0;
			param5 = 0;
			memset(&seq2, 0, 12);
		}

		Common::String toString() {
			return Common::String::format("SIIS: %s %d %d %s\n", seq1, param4, param5, seq2);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersSIIS::update] Invalid index (was: %d)", index);

			case 3: param4 = 1; break;
			case 4: param5 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq1, 12);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncBytes((byte *)&seq2, 12);
		}
	};

	struct EntityParametersISSI : EntityParameters {
		uint param1;
		char seq1[12];
		char seq2[12];
		uint param8;

		EntityParametersISSI() {
			param1 = 0;
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("ISSI: %d %s %s %d\n", param1, seq1, seq2, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersISSI::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersISII : EntityParameters {
		uint param1;
		char seq[12];
		uint param5;
		uint param6;
		uint param7;
		uint param8;

		EntityParametersISII() {
			param1 = 0;
			memset(&seq, 0, 12);
			param5 = 0;
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("ISII: %d %s %d %d %d %d\n", param1, seq, param5, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersISII::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 4: param5 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param5);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSSII : EntityParameters {
		char seq1[12];
		char seq2[12];
		uint param7;
		uint param8;

		EntityParametersSSII() {
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("SSII: %s %s %d %d\n", seq1, seq2, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersSSII::update] Invalid index (was: %d)", index);

			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersSSS : EntityParameters {
		char seq1[12];
		char seq2[12];
		char seq3[8];

		EntityParametersSSS() {
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
			memset(&seq3, 0, 8);
		}

		Common::String toString() {
			return Common::String::format("SSS: %s %s %s\n", seq1, seq2, seq3);
		}

		void update(uint32) {
			error("[EntityParametersSSS::update] Cannot update this type of parameters");
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
			s.syncBytes((byte *)&seq3, 8);
		}
	};

	struct EntityParametersIISS : EntityParameters {
		uint param1;
		uint param2;
		char seq1[12];
		char seq2[12];

		EntityParametersIISS() {
			param1 = 0;
			param2 = 0;
			memset(&seq1, 0, 12);
			memset(&seq2, 0, 12);
		}

		Common::String toString() {
			return Common::String::format("IISS: %d %d %s %s\n", param1, param2, seq1, seq2);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIISS::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncBytes((byte *)&seq1, 12);
			s.syncBytes((byte *)&seq2, 12);
		}
	};

	struct EntityParametersIISI : EntityParameters {
		uint param1;
		uint param2;
		char seq[12];
		uint param6;
		uint param7;
		uint param8;

		EntityParametersIISI() {
			param1 = 0;
			param2 = 0;
			memset(&seq, 0, 12);
			param6 = 0;
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("IISI: %d %d %s %d %d %d\n", param1, param2, seq, param6, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIISI::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			case 5: param6 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param6);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersIIIS : EntityParameters {
		uint param1;
		uint param2;
		uint param3;
		char seq[12];
		uint param7;
		uint param8;

		EntityParametersIIIS() {
			param1 = 0;
			param2 = 0;
			param3 = 0;
			memset(&seq, 0, 12);
			param7 = 0;
			param8 = 0;
		}

		Common::String toString() {
			return Common::String::format("IIIS: %d %d %d %s %d %d\n", param1, param2, param3, seq, param7, param8);
		}

		void update(uint32 index) {
			switch (index) {
			default:
				error("[EntityParametersIIIS::update] Invalid index (was: %d)", index);

			case 0: param1 = 1; break;
			case 1: param2 = 1; break;
			case 2: param3 = 1; break;
			case 6: param7 = 1; break;
			case 7: param8 = 1; break;
			}
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncAsUint32LE(param3);
			s.syncBytes((byte *)&seq, 12);
			s.syncAsUint32LE(param7);
			s.syncAsUint32LE(param8);
		}
	};

	struct EntityParametersI5S : EntityParameters {
		uint param1;
		uint param2;
		uint param3;
		uint param4;
		uint param5;
		char seq[12];

		EntityParametersI5S() {
			param1 = 0;
			param2 = 0;
			param3 = 0;
			param4 = 0;
			param5 = 0;
			memset(&seq, 0, 12);
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsUint32LE(param1);
			s.syncAsUint32LE(param2);
			s.syncAsUint32LE(param3);
			s.syncAsUint32LE(param4);
			s.syncAsUint32LE(param5);
			s.syncBytes((byte *)&seq, 12);
		}
	};

	struct EntityCallParameters : Common::Serializable {
		EntityParameters *parameters[4];

		EntityCallParameters() {
			// We default to int parameters
			for (int i = 0; i < 4; i++)
				parameters[i] = new EntityParametersIIII();
		}

		~EntityCallParameters() {
			clear();
		}

		void clear() {
			for (int i = 0; i < 4; i++)
				SAFE_DELETE(parameters[i]);
		}

		// Serializable
		void saveLoadWithSerializer(Common::Serializer &s) {
			for (uint i = 0; i < ARRAYSIZE(parameters); i++)
				parameters[i]->saveLoadWithSerializer(s);
		}
	};

	struct EntityCallData : Common::Serializable {
		byte callbacks[16];
		byte currentCall;
		EntityPosition entityPosition;      // word
		Location location;                  // word
		CarIndex car;                       // word
		byte field_497;
		EntityIndex entity;                 // byte
		InventoryItem inventoryItem;        // byte
		EntityDirection direction;          // byte
		int16 field_49B;
		int16 currentFrame;
		int16 currentFrame2;
		int16 field_4A1;
		int16 field_4A3;
		ClothesIndex clothes;               // byte
		Position position;
		CarIndex car2;                      // byte
		bool doProcessEntity;               // byte
		bool field_4A9;	                    // byte
		bool field_4AA;                     // byte
		EntityDirection directionSwitch;
		Common::String sequenceName;        // char[13]
		Common::String sequenceName2;       // char[13]
		Common::String sequenceNamePrefix;  // char[7]
		Common::String sequenceNameCopy;    // char[13]
		SequenceFrame *frame;
		SequenceFrame *frame1;
		Sequence *sequence;
		Sequence *sequence2;
		Sequence *sequence3;

		/**
		 * Default constructor.
		 */
		EntityCallData() {
			memset(&callbacks, 0, 16 * sizeof(byte));
			currentCall = 0;
			entityPosition = kPositionNone;
			location = kLocationOutsideCompartment;
			car = kCarNone;
			field_497 = 0;
			entity = kEntityPlayer;
			inventoryItem = kItemNone;
			direction = kDirectionNone;
			field_49B = 0;
			currentFrame = 0;
			currentFrame2 = 0;
			field_4A1 = 0;
			field_4A3 = 30;
			clothes = kClothesDefault;
			position = 0;
			car2 = kCarNone;
			doProcessEntity = false;
			field_4A9 = false;
			field_4AA = false;
			directionSwitch = kDirectionNone;
			frame = NULL;
			frame1 = NULL;
			sequence = NULL;
			sequence2 = NULL;
			sequence3 = NULL;
		}

		~EntityCallData();

		/**
		 * Convert this object into a string representation.
		 *
		 * @return A string representation of this object.
		 */
		Common::String toString() {
			Common::String str = "";

			str += Common::String::format("Entity position: %d    - Location: %d       - Car: %d\n", entityPosition, location, car);
			str += Common::String::format("Entity: %d             - Item: %d          - Direction: %d\n", entity, inventoryItem, direction);
			str += Common::String::format("Clothes: %d            - Position: %d      - Direction switch: %d\n", clothes, position, directionSwitch);
			str += "\n";
			str += Common::String::format("field_497: %02d        - field_49B: %i     - field_4A1: %i\n", field_497, field_49B, field_4A1);
			str += Common::String::format("field_4A9: %02d        - field_4AA: %i     - Car 2: %d\n", field_4A9, field_4AA, car2);
			str += "\n";
			str += "Sequence: " + sequenceName + "                 - Sequence 2: " + sequenceName2 + "\n";
			str += "Sequence prefix: " + sequenceNamePrefix + "    - Sequence copy: " + sequenceNameCopy + "\n";
			str += Common::String::format("Current frame: %i    - Current frame 2: %i       - Process entity: %d\n", currentFrame, currentFrame2, doProcessEntity);
			str += "\n";
			str += Common::String::format("Current call: %d\n", currentCall);
			str += Common::String::format("Functions: %d %d %d %d %d %d %d %d\n", callbacks[0], callbacks[1], callbacks[2], callbacks[3], callbacks[4], callbacks[5], callbacks[6], callbacks[7]);
			str += Common::String::format("Callbacks: %d %d %d %d %d %d %d %d\n", callbacks[8], callbacks[9], callbacks[10], callbacks[11], callbacks[12], callbacks[13], callbacks[14], callbacks[15]);

			return str;
		}

		// Serializable
		void saveLoadWithSerializer(Common::Serializer &s);
	};

	EntityData() {}

	template<class T>
	void resetCurrentParameters() {
		EntityCallParameters *params = &_parameters[_data.currentCall];
		params->clear();

		for (int i = 0; i < 4; i++)
			params->parameters[i] = new T();
	}

	EntityCallData 	  *getCallData() { return &_data; }

	EntityParameters  *getParameters(uint callback, byte index) const;
	EntityParameters  *getCurrentParameters(byte index = 0) { return getParameters(_data.currentCall, index); }

	int 			   getCallback(uint callback) const;
	int				   getCurrentCallback() { return getCallback(_data.currentCall); }
	void 			   setCallback(uint callback, byte index);
	void 			   setCurrentCallback(uint index) { setCallback(_data.currentCall, index); }

	void               updateParameters(uint32 index) const;

	// Serializable
	void 			   saveLoadWithSerializer(Common::Serializer &ser);

private:

	EntityCallData 		 _data;
	EntityCallParameters _parameters[9];
};

class Entity : Common::Serializable {
public:

	typedef Common::Functor1<const SavePoint&, void> Callback;

	Entity(LastExpressEngine *engine, EntityIndex index);
	virtual ~Entity();

	// Accessors
	EntityData *getParamData() { return _data; }
	EntityData::EntityCallData *getData() { return _data->getCallData(); }

	// Callbacks
	byte getCallback() { return _data->getCallback(_data->getCallData()->currentCall + 8); }
	void setCallback(byte index) { _data->setCallback(_data->getCallData()->currentCall + 8, index); getData()->currentCall++; }

	// Setup
	void setup(ChapterIndex index);

	virtual void setup_chapter1() = 0;
	virtual void setup_chapter2() = 0;
	virtual void setup_chapter3() = 0;
	virtual void setup_chapter4() = 0;
	virtual void setup_chapter5() = 0;

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser) { _data->saveLoadWithSerializer(ser); }

	void nullfunction(const SavePoint &savepoint) {}

protected:
	LastExpressEngine *_engine;

	EntityIndex				  _entityIndex;
	EntityData 				 *_data;
	Common::Array<Callback *> _callbacks;

	/**
	 * Saves the game
	 *
	 * @param savepoint The savepoint
	 *                   - SavegameType
	 *                   - EventIndex
	 */
	void savegame(const SavePoint &savepoint);

	/**
	 * Play sound
	 *
	 * @param savepoint The savepoint
	 *                    - Sound filename
	 * @param resetItem true to reset item.
	 * @param flag      sound flag
	 */
	void playSound(const SavePoint &savepoint, bool resetItem = false, SoundFlag flag = kFlagInvalid);

	/**
	 * Draws the entity
	 *
	 * @param savepoint 	 The savepoint
	 *                         - Sequence
	 *                         - ExcuseMe flag
	 * @param handleExcuseMe true to handle excuseMeCath action
	 */
	void draw(const SavePoint &savepoint, bool handleExcuseMe = false);

	/**
	 * Draws the entity along with another one
	 *
	 * @param savepoint The savepoint.
	 *                    - Sequence 1
	 *                    - Sequence 2
	 *                    - EntityIndex
	 */
	void draw2(const SavePoint &savepoint);

	/**
	 * Updates parameter 2 using ticks value
	 *
	 * @param savepoint The savepoint
	 *                    - Number of ticks to add
	 */
	void updateFromTicks(const SavePoint &savepoint);

	/**
	 * Updates parameter 2 using time value
	 *
	 * @param savepoint The savepoint.
	 *                    - Time to add
	 */
	void updateFromTime(const SavePoint &savepoint);

	/**
	 * Resets an entity
	 *
	 * @param savepoint    The savepoint.
	 * @param resetClothes true to reset clothes.
	 * @param resetItem    true to reset inventoryItem to kItemInvalid
	 */
	void reset(const SavePoint &savepoint, bool resetClothes = false, bool resetItem = false);

	/**
	 * Process callback action when the entity direction is not kDirectionRight
	 *
	 * @param savepoint The savepoint.
	 */
	void callbackActionOnDirection(const SavePoint &savepoint);

	/**
	 * Process callback action when somebody is standing in the restaurant or salon.
	 *
	 * @param savepoint The savepoint.
	 */
	void callbackActionRestaurantOrSalon(const SavePoint &savepoint);

	/**
	 * Updates the entity
	 *
	 * @param savepoint 	 The savepoint.
	 *                        - CarIndex
	 *                        - EntityPosition
	 * @param handleExcuseMe true to handle the kActionExcuseMe/kActionExcuseMeCath actions.
	 */
	void updateEntity(const SavePoint &savepoint, bool handleExcuseMe = false);

	/**
	 * Call a specific savepoint (or draw sequence in default case)
	 *
	 * @param savepoint 	 The savepoint.
	 *                         - Sequence to draw in default case
	 *                         - EntityIndex
	 *                         - ActionIndex
	 *                         - Sequence for the savepoint
	 * @param handleExcuseMe true to handle excuse me.
	 */
	void callSavepoint(const SavePoint &savepoint, bool handleExcuseMe = false);

	/**
	 * Handles entering/exiting a compartment.
	 *
	 * @param savepoint   The savepoint.
	 * @param position1   The first position.
	 * @param position2   The second position.
	 * @param car 		  The car.
	 * @param compartment The compartment.
	 * @param alternate   true to use the alternate version of SceneManager::loadSceneFromObject()
	 */
	void enterExitCompartment(const SavePoint &savepoint, EntityPosition position1 = kPositionNone, EntityPosition position2 = kPositionNone, CarIndex car = kCarNone, ObjectIndex compartment = kObjectNone, bool alternate = false, bool updateLocation = false);

	/**
	 * Updates the position
	 *
	 * @param savepoint 	 The savepoint
	 *                        - Sequence name
	 *                        - CarIndex
	 *                        - Position
	 * @param handleExcuseMe true to handle excuseMe actions
	 */
	void updatePosition(const SavePoint &savepoint, bool handleExcuseMe = false);
};


} // End of namespace LastExpress

#endif // LASTEXPRESS_ENTITY_H
