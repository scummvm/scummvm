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

namespace Titanic {

#define DIALS_ARRAY_COUNT 10

enum ScriptArrayFlag { SF_NONE = 0, SF_RANDOM = 1, SF_SEQUENTIAL = 2 };

class CGameManager;
class CPetControl;
class TTroomScript;
class TTsentence;
struct TTsentenceEntry;

struct TTnpcScriptResponse {
	uint _tag;
	uint _values[4];

	/**
	 * Returns the size of the values list plus 1
	 */
	int size() const;
};

struct TTscriptRange {
	uint _id;
	Common::Array<uint> _values;
	TTscriptRange *_nextP;
	uint _priorIndex;
	ScriptArrayFlag _mode;

	TTscriptRange() : _id(0), _nextP(nullptr),
		_priorIndex(0), _mode(SF_NONE) {}
	TTscriptRange(uint id, const Common::Array<uint> &values, bool isRandom, 
		bool isSequential);
};


struct TTsentenceEntry {
	int _field0;
	int _field4;
	CString _string8;
	int _fieldC;
	CString _string10;
	CString _string14;
	CString _string18;
	CString _string1C;
	int _field20;
	CString _string24;
	int _field28;
	int _field2C;
	int _field30;

	TTsentenceEntry() : _field0(0), _field4(0), _fieldC(0),
		_field20(0), _field28(0), _field2C(0), _field30(0) {}

	/**
	* Load an entry from the passed stream, and returns true
	* if an entry was successfully loaded
	*/
	bool load(Common::SeekableReadStream *s);
};

class TTsentenceEntries : public Common::Array<TTsentenceEntry> {
public:
	/**
	* Load a list of entries from the specified resource
	*/
	void load(const CString &resName);
};

struct TTscriptMapping {
	uint _id;
	uint _values[8];

	TTscriptMapping();
};

class TTscriptMappings : public Common::Array<TTscriptMapping> {
public:
	int _valuesPerMapping;

	void load(const char *name, int valuesPerMapping);
};

struct TTtagMapping {
	uint _src, _dest;
	TTtagMapping() : _src(0), _dest(0) {}
	TTtagMapping(uint src, uint dest) : _src(src), _dest(dest) {}
};

class TTtagMappings : public Common::Array<TTtagMapping> {
public:
	void load(const char *name);
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
	virtual int chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) = 0;

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	virtual int process(TTroomScript *roomScript, TTsentence *sentence) = 0;

	virtual int proc8() const = 0;
	virtual int proc9() const = 0;

	/**
	 * Called when the script/id changes
	 */
	virtual ScriptChangedResult scriptChanged(TTscriptBase *roomScript, uint id) = 0;

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
	int _entryCount;
	int _field68;
	int _field6C;
	int _rangeResetCtr;
	int _currentDialNum;
	int _dialDelta;
	int _field7C;
	const char *_itemStringP;
	int _dialValues[DIALS_ARRAY_COUNT];
	int _array[136];
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
	int getValue(int testNum);

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
	int processEntries(const TTsentenceEntries *entries, uint entryCount, TTroomScript *roomScript, TTsentence *sentence);

	/**
	 * Scans through a list of sentence entries for a matching standardized response
	 */
	int processEntries(TTroomScript *roomScript, TTsentence *sentence) {
		return processEntries(&_entries, _entryCount, roomScript, sentence);
	}

	bool defaultProcess(TTroomScript *roomScript, TTsentence *sentence);

	void checkItems(TTroomScript *roomScript, TTsentence *sentence);

	/**
	 * Adds a random conversation response
	 */
	bool addRandomResponse(bool flag);

	/**
	 * Updates the current dial with the given delta
	 */
	void updateCurrentDial(bool changeDial);

	bool fn10(bool flag);

	static bool sentence2C(TTsentence *sentence);

	/**
	 * Gets the True Talk state value
	 */
	bool getStateValue() const;
public:
	static void init();
	static void deinit();
public:
	TTnpcScript(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4,
		int v5, int v6, int v7);

	virtual void addResponse(int id);

	/**
	 * Chooses and adds a conversation response based on a specified tag Id.
	 * This default implementation does a lookup into a list of known tags,
	 * and chooses a random dialogue Id from the available ones for that tag
	 */
	virtual int chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag);

	/**
	 * Does NPC specific processing of the parsed sentence
	 */
	virtual int process(TTroomScript *roomScript, TTsentence *sentence);

	virtual int proc8() const;
	virtual int proc9() const;

	/**
	 * Called when the script/id changes
	 */
	virtual ScriptChangedResult scriptChanged(TTscriptBase *roomScript, uint id) {
		return SCR_2;
	}

	virtual int proc11() const;
	virtual int proc12() const;

	/**
	 * Translate a passed Id to a dialogue Id if necessary,
	 * and adds it to the response
	 */
	virtual void selectResponse(int id);
	
	virtual int proc15(int id) const;

	virtual bool handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const;

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

	virtual int proc21(int v1, int v2, int v3);
	virtual int proc22(int id) const;
	virtual int proc23() const;
	virtual const TTscriptMapping *getMapping(int index);
	virtual int proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const;
	virtual void proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence);
	virtual void save(SimpleFile *file);
	virtual void load(SimpleFile *file);
	virtual void saveBody(SimpleFile *file);
	virtual void loadBody(SimpleFile *file);
	virtual int proc31() const;

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

	virtual int proc36(int val) const;
	virtual uint translateId(uint id) const;

	void preLoad();

	/**
	 * Called with the script and id changes
	 */
	ScriptChangedResult notifyScript(TTscriptBase *npcScript, int id) {
		return scriptChanged(npcScript, id);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_TT_NPC_SCRIPT_H */
