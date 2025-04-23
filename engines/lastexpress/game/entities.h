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

#ifndef LASTEXPRESS_ENTITIES_H
#define LASTEXPRESS_ENTITIES_H

/*
	Entities
	--------

	The entities structure contains 40 Entity_t structures for each character

*/

#include "lastexpress/entities/entity.h"

#include "lastexpress/shared.h"

#include "common/rect.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class Sequence;

class Entities : Common::Serializable {
public:
	Entities(LastExpressEngine *engine);
	~Entities() override;

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &ser) override;
	void savePositions(Common::Serializer &ser);
	void saveCompartments(Common::Serializer &ser);

	void setup(bool isFirstChapter, CharacterIndex entity);
	void setupChapter(ChapterIndex chapter);
	void reset();

	// Update & drawing

	/**
	 * Reset an character state
	 *
	 * @param character character index
	 * @note remember to call the function pointer (we do not pass it our implementation)
	 */
	void resetState(CharacterIndex entity);
	void updateFields() const;
	void updateSequences() const;
	void updateCallbacks();

	CharacterIndex canInteractWith(const Common::Point &point) const;
	bool compare(CharacterIndex entity1, CharacterIndex entity2) const;

	/**
	 * Update an character current sequence frame (and related fields)
	 *
	 * @param character character index
	 */
	void updateFrame(CharacterIndex entity) const;
	void updatePositionEnter(CharacterIndex entity, CarIndex car, PositionOld position);
	void updatePositionExit(CharacterIndex entity, CarIndex car, PositionOld position);
	void enterCompartment(CharacterIndex entity, ObjectIndex compartment, bool useCompartment1 = false);
	void exitCompartment(CharacterIndex entity, ObjectIndex compartment, bool useCompartment1 = false);

	// Sequences
	void drawSequenceLeft(CharacterIndex index, const char *sequence) const;
	void drawSequenceRight(CharacterIndex index, const char *sequence) const;
	void clearSequences(CharacterIndex index) const;

	bool updateEntity(CharacterIndex entity, CarIndex car, EntityPosition position) const;
	bool hasValidFrame(CharacterIndex entity) const;

	// Accessors
	Entity *get(CharacterIndex entity);
	EntityData::EntityCallData *getData(CharacterIndex entity) const;
	int getPosition(CarIndex car, PositionOld position) const;
	int getCompartments(int index) const;
	int getCompartments1(int index) const;

	// Scene
	void loadSceneFromEntityPosition(CarIndex car, EntityPosition position, bool alternate = false) const;

	//////////////////////////////////////////////////////////////////////////
	// Checks
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Query if 'character' is inside a compartment
	 *
	 * @param character   The character.
	 * @param car      The car.
	 * @param position The position.
	 *
	 * @return true if inside the compartment, false if not.
	 */
	bool isInsideCompartment(CharacterIndex entity, CarIndex car, EntityPosition position) const;

	bool checkFields2(ObjectIndex object) const;

	/**
	 * Query if 'character' is in compartment cars.
	 *
	 * @param character The character.
	 *
	 * @return true if in compartment cars, false if not.
	 */
	bool isInsideCompartments(CharacterIndex entity) const;

	/**
	 * Query if the player is in the specified position
	 *
	 * @param car       The car.
	 * @param position  The position.
	 * @return true if player is in that position, false if not.
	 */
	bool isPlayerPosition(CarIndex car, PositionOld position) const;

	/**
	 * Query if 'character' is inside a train car
	 *
	 * @param character The character.
	 * @param car    The car.
	 *
	 * @return true if inside a train car, false if not.
	 */
	bool isInsideTrainCar(CharacterIndex entity, CarIndex car) const;

	/**
	 * Query if 'character' is in green car entrance.
	 *
	 * @param character The character.
	 *
	 * @return true if in the green car entrance, false if not.
	 */
	bool isInGreenCarEntrance(CharacterIndex entity) const;

	/**
	 * Query if the player is in a specific car
	 *
	 * @param car The car.
	 *
	 * @return true if player is in the car, false if not.
	 */
	bool isPlayerInCar(CarIndex car) const;

	/**
	 * Query if 'character' is going in the up or down direction.
	 *
	 * @param character The character.
	 *
	 * @return true if direction is up or down, false if not.
	 */
	bool isDirectionUpOrDown(CharacterIndex entity) const;

	/**
	 * Query if the distance between the two entities is less 'distance'
	 *
	 * @param entity1  The first character.
	 * @param entity2  The second character.
	 * @param distance The distance.
	 *
	 * @return true if the distance between entities is less than 'distance', false if not.
	 */
	bool isDistanceBetweenEntities(CharacterIndex entity1, CharacterIndex entity2, uint distance) const;

	bool checkFields10(CharacterIndex entity) const;

	/**
	 * Query if there is somebody in the restaurant or salon.
	 *
	 * @return true if somebody is in the restaurant or salon, false if not.
	 */
	bool isSomebodyInsideRestaurantOrSalon() const;

	/**
	 * Query if 'character' is in the salon.
	 *
	 * @param character The character.
	 *
	 * @return true if in the salon, false if not.
	 */
	bool isInSalon(CharacterIndex entity) const;

	/**
	 * Query if 'character' is in the restaurant.
	 *
	 * @param character The character.
	 *
	 * @return true if in the restaurant, false if not.
	 */
	bool isInRestaurant(CharacterIndex entity) const;

	/**
	 * Query if 'character' is in Kronos salon.
	 *
	 * @param character The character.
	 *
	 * @return true if in Kronos salon, false if not.
	 */
	bool isInKronosSalon(CharacterIndex entity) const;

	/**
	 * Query if the player is outside Alexei window.
	 *
	 * @return true if outside alexei window, false if not.
	 */
	bool isOutsideAlexeiWindow() const;

	/**
	 * Query if the player is outside Anna window.
	 *
	 * @return true if outside anna window, false if not.
	 */
	bool isOutsideAnnaWindow() const;

	/**
	 * Query if 'character' is in the kitchen.
	 *
	 * @param character The character.
	 *
	 * @return true if in the kitchen, false if not.
	 */
	bool isInKitchen(CharacterIndex entity) const;

	/**
	 * Query if nobody is in a compartment at that position.
	 *
	 * @param car      The car.
	 * @param position The position.
	 *
	 * @return true if nobody is in a compartment, false if not.
	 */
	bool isNobodyInCompartment(CarIndex car, EntityPosition position) const;

	bool checkFields19(CharacterIndex entity, CarIndex car, EntityPosition position) const;

	/**
	 * Query if 'character' is in the baggage car entrance.
	 *
	 * @param character The character.
	 *
	 * @return true if in the baggage car entrance, false if not.
	 */
	bool isInBaggageCarEntrance(CharacterIndex entity) const;

	/**
	 * Query if 'character' is in the baggage car.
	 *
	 * @param character The character.
	 *
	 * @return true if in the baggage car, false if not.
	 */
	bool isInBaggageCar(CharacterIndex entity) const;

	/**
	 * Query if 'character' is in Kronos sanctum.
	 *
	 * @param character The character.
	 *
	 * @return true if in Kronos sanctum, false if not.
	 */
	bool isInKronosSanctum(CharacterIndex entity) const;

	/**
	 * Query if 'character' is in Kronos car entrance.
	 *
	 * @param character The character.
	 *
	 * @return true if in Kronos car entrance, false if not.
	 */
	bool isInKronosCarEntrance(CharacterIndex entity) const;

	/**
	 * Check distance from position.
	 *
	 * @param character   The character.
	 * @param position The position.
	 * @param distance The distance.
	 *
	 * @return true if distance is bigger, false otherwise.
	 */
	bool checkDistanceFromPosition(CharacterIndex entity, EntityPosition position, int distance) const;

	/**
	 * Query if 'character' is walking opposite to player.
	 *
	 * @param character The character.
	 *
	 * @return true if walking opposite to player, false if not.
	 */
	bool isWalkingOppositeToPlayer(CharacterIndex entity) const;

	/**
	 * Query if 'character' is female.
	 *
	 * @param character The character.
	 *
	 * @return true if female, false if not.
	 */
	static bool isFemale(CharacterIndex entity);

	/**
	 * Query if 'character' is married.
	 *
	 * @param character The character.
	 *
	 * @return true if married, false if not.
	 */
	static bool isMarried(CharacterIndex entity);

private:
	static const int _compartmentsCount = 16;
	static const int _positionsCount = 100 * 10;  // 100 positions per train car

	LastExpressEngine       *_engine;
	EntityData              *_header;
	Common::Array<Entity *>  _entities;

	// Compartments & positions
	uint _compartments[_compartmentsCount];
	uint _compartments1[_compartmentsCount];
	uint _positions[_positionsCount];

	void executeCallbacks();
	void incrementDirectionCounter(EntityData::EntityCallData *data) const;
	void processEntity(CharacterIndex entity);

	void drawSequence(CharacterIndex entity, const char *sequence, EntityDirection direction) const;
	void drawSequences(CharacterIndex entity, EntityDirection direction, bool loadSequence) const;
	void loadSequence2(CharacterIndex entity, Common::String sequenceName, Common::String sequenceName2, byte field30, bool loadSequence) const;

	void clearEntitySequenceData(EntityData::EntityCallData *data, EntityDirection direction) const;
	void computeCurrentFrame(CharacterIndex entity) const;
	int16 getCurrentFrame(CharacterIndex entity, Sequence *sequence, EntityPosition position, bool doProcessing) const;
	void processFrame(CharacterIndex entity, bool keepPreviousFrame, bool dontPlaySound);
	void drawNextSequence(CharacterIndex entity) const;
	void updateEntityPosition(CharacterIndex entity) const;
	void copySequenceData(CharacterIndex entity) const;

	bool changeCar(EntityData::EntityCallData *data, CharacterIndex entity, CarIndex car, EntityPosition position, bool increment, EntityPosition newPosition, CarIndex newCar) const;

	void getSequenceName(CharacterIndex entity, EntityDirection direction, Common::String &sequence1, Common::String &sequence2) const;

	void updatePositionsEnter(CharacterIndex entity, CarIndex car, PositionOld position1, PositionOld position2, PositionOld position3, PositionOld position4);
	void updatePositionsExit(CharacterIndex entity, CarIndex car, PositionOld position1, PositionOld position2);

	void resetSequences(CharacterIndex entity) const;

	bool checkPosition(EntityPosition position) const;
	bool checkSequenceFromPosition(CharacterIndex entity) const;
	EntityPosition getEntityPositionFromCurrentPosition() const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ENTITIES_H
