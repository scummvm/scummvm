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

#ifndef TITANIC_TT_NPC_SCRIPT_H
#define TITANIC_TT_NPC_SCRIPT_H

#include "titanic/support/simple_file.h"
#include "titanic/true_talk/tt_script_base.h"
#include "titanic/true_talk/script_support.h"

namespace Titanic {

#define DIALS_ARRAY_COUNT 10

class CGameManager;
class CPetControl;
class TTroomScript;

struct TTnpcData {
private:
	int _array[136];
public:
	TTnpcData();
	int &operator[](int idx) { return _array[idx]; }
	int *getSlot(int idx) { return &_array[16 + idx * 4]; }
	void resetFlags();

	void copyData();
};

class TTnpcScriptBase : public TTscriptBase {
protected:
	int _field54;
	int _val2;
public:
	int _charId;
public:
	TTnpcScriptBase(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4,
		int v5, int v6, int v7);

	/**
	 * Chooses and adds a conversation response based on a specified tag Id.
	 */
	virtual int chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) = 0;

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	virtual int process(const TTroomScript *roomScript, const TTsentence *sentence) = 0;

	virtual int proc8() const = 0;

	/**
	 * Called when the script/id changes
	 */
	virtual ScriptChangedResult scriptChanged(uint id) = 0;

	/**
	 * Called when the script/id changes
	 */
	virtual ScriptChangedResult scriptChanged(const TTroomScript *roomScript, uint id) = 0;

	virtual int proc11() const = 0;
	virtual int proc12() const = 0;

	int charId() const { return _charId; }
};

class TTnpcScript : public TTnpcScriptBase {
private:
	int translateByArray(int id);
protected:
	static TTsentenceEntries *_defaultEntries;
protected:
	Common::Array<TTnpcScriptResponse> _responses;
	int _valuesPerResponse;
	Common::Array<TTscriptRange> _ranges;
	TTscriptMappings _mappings;
	TTsentenceEntries _entries;
	TTtagMappings _tagMappings;
	TTwordEntries _words;
	TThandleQuoteEntries _quotes;
	int _entryCount;
	int _field68;
	int _field6C;
	int _rangeResetCtr;
	int _currentDialNum;
	int _dialDelta;
	int _field7C;
	const char *_itemStringP;
	int _dialValues[DIALS_ARRAY_COUNT];
	TTnpcData _data;
	bool _field2CC;
protected:
	/**
	 * Loads response data for the NPC from the given resource
	 */
	void loadResponses(const char *name, int valuesPerResponse = 1);

	/**
	 * Load ranges data for the NPC from the given resource
	 */
	void loadRanges(const char *name);

	/**
	 * Reset script flags
	 */
	void resetFlags();

	/**
	 * Setup dials
	 */
	void setupDials(int dial1, int dial2, int dial3);

	static int getRoom54(int roomId);

	/**
	 * Perform test on various state values
	 */
	int getValue(int testNum) const;

	/**
	 * Gets a random number between 1 and a given max
	 */
	uint getRandomNumber(int max) const;

	/**
	 * Gets a random number of 0 or 1
	 */
	uint getRandomBit() const {
		return getRandomNumber(2) - 1;
	}

	/**
	 * Returns a dialogue Id by script tag value Id
	 */
	uint getDialogueId(uint tagId);

	/**
	 * Returns a pointer to the PET control
	 */
	static CPetControl *getPetControl(CGameManager *gameManager);

	/**
	 * Adds a new item to the list of number ranges
	 */
	void addRange(uint id, const Common::Array<uint> &values, bool isRandom, bool isSequential);

	/**
	 * Finds an entry in the list of prevoiusly registered number ranges
	 */
	TTscriptRange *findRange(uint id);

	/**
	 * Scans through a list of sentence entries for a matching standardized response
	 */
	int processEntries(const TTsentenceEntries *entries, uint entryCount, const TTroomScript *roomScript, const TTsentence *sentence);

	/**
	 * Scans through a list of sentence entries for a matching standardized response
	 */
	int processEntries(const TTroomScript *roomScript, const TTsentence *sentence) {
		return processEntries(&_entries, _entryCount, roomScript, sentence);
	}

	bool defaultProcess(const TTroomScript *roomScript, const TTsentence *sentence);

	void checkItems(const TTroomScript *roomScript, const TTsentence *sentence);

	/**
	 * Adds a random conversation response
	 */
	bool addRandomResponse(bool flag);

	/**
	 * Updates the current dial with the given delta
	 */
	void updateCurrentDial(bool changeDial);

	bool fn10(bool flag);

	static bool sentence2C(const TTsentence *sentence);

	/**
	 * Gets the True Talk state value
	 */
	bool getStateValue() const;

	/**
	 * Gets the assigned room's room, floor, and elevator number
	 */
	void getAssignedRoom(int *roomNum, int *floorNum, int *elevatorNum) const;

	/**
	 * Uses a porition of the state _array to set up a new response
	 */
	void setResponseFromArray(int index, int id);
public:
	static void init();
	static void deinit();
public:
	TTnpcScript(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4,
		int v5, int v6, int v7);

	void addResponse(int id) override;

	/**
	 * Chooses and adds a conversation response based on a specified tag Id.
	 * This default implementation does a lookup into a list of known tags,
	 * and chooses a random dialogue Id from the available ones for that tag
	 */
	int chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) override;

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	int process(const TTroomScript *roomScript, const TTsentence *sentence) override;

	int proc8() const override;

	/**
	 * Called when the script/id changes
	 */
	ScriptChangedResult scriptChanged(uint id) override {
		return SCR_2;
	}

	/**
	 * Called when the script/id changes
	 */
	ScriptChangedResult scriptChanged(const TTroomScript *roomScript, uint id) override {
		return SCR_2;
	}

	int proc11() const override;
	int proc12() const override;

	/**
	 * Translate a passed Id to a dialogue Id if necessary,
	 * and adds it to the response
	 */
	virtual void selectResponse(int id);

	/**
	 * Handles scanning the word list for a given Id, and if
	 * found adds it to the sentence concept list
	 */
	virtual bool handleWord(uint id) const;

	virtual int handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
		uint tag1, uint tag2, uint remainder);

	/**
	 * Returns true if the NPC's dial region affects quote responses
	 */
	virtual bool isQuoteDialled() const { return false; }

	/**
	 * Given an Id for a previously registered set of random number values,
	 * picks one of the array values and returns it.. depending on flags,
	 * either a random value, or each value in turn
	 */
	virtual uint getRangeValue(uint id);

	/**
	 * Resets the prior used index for the specified range
	 */
	virtual void resetRange(int id);

	/**
	 * Handles updating NPC state based on specified dialogue Ids and dial positions
	 */
	virtual int updateState(uint oldId, uint newId, int index);

	/**
	 * Handles getting a pre-response
	 */
	virtual int preResponse(uint id);

	/**
	 * Returns a bitset of the dials being off or not
	 */
	virtual uint getDialsBitset() const { return 0; }

	virtual const TTscriptMapping *getMapping(int index);
	virtual int doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence);

	/**
	 * Handles any post-response NPC processing
	 */
	virtual void postResponse(int v1, const TTsentenceEntry *entry, const TTroomScript *roomScript, const TTsentence *sentence) {}

	virtual void save(SimpleFile *file);
	virtual void load(SimpleFile *file);
	virtual void saveBody(SimpleFile *file);
	virtual void loadBody(SimpleFile *file);

	/**
	 * Returns the number of range records that are non-random
	 */
	virtual int getRangesCount() const;

	/**
	 * Sets a given dial to be pointing in a specified region (0 to 2)
	 */
	virtual void setDialRegion(int dialNum, int region);

	/**
	 * Sets the value for an NPC's dial
	 */
	virtual void setDial(int dialNum, int value);

	/**
	 * Returns a dial's region number
	 */
	virtual int getDialRegion(int dialNum) const;

	/**
	 * Gets the value for a dial
	 * @param dialNum		Dial number
	 * @param randomizeFlag	If set, introduces a slight random variance so that
	 *		the displayed dial will oscillate randomly around it's real level
	 */
	virtual int getDialLevel(uint dialNum, bool randomizeFlag = true);

	/**
	 * Handles a randomzied response
	 */
	virtual bool randomResponse(uint index);

	virtual uint translateId(uint id) const;

	void preLoad();

	/**
	 * Called with the script and id changes
	 */
	ScriptChangedResult notifyScript(TTroomScript *roomScript, int id) {
		return scriptChanged(roomScript, id);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_TT_NPC_SCRIPT_H */
