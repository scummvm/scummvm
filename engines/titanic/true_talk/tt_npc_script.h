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

class CGameManager;
class CPetControl;
class TTroomScript;
class TTsentence;

struct TTnpcScriptResponse {
	uint _tag;
	uint _values[4];

	/**
	 * Returns the size of the values list plus 1
	 */
	int size() const;
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

	virtual void proc7(int v1, int v2) = 0;
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
	Common::Array<TTnpcScriptResponse> _responses;
	byte *_subPtr;
	int _field60;
	int _field64;
	int _field68;
	int _field6C;
	int _field70;
	int _field74;
	int _field78;
	int _field7C;
	int _field80;
	int _dialValues[DIALS_ARRAY_COUNT];
	int _array[136];
	bool _field2CC;
protected:
	/**
	 * Loads response data for the NPC from the given resource
	 */
	void load(const char *name, int valuesPerTag = 1);

	/**
	 * Reset script flags
	 */
	void resetFlags();

	void randomizeFlags();

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
public:
	TTnpcScript(int charId, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4,
		int v5, int v6, int v7);

	virtual void proc4(int v);

	/**
	 * Chooses and adds a conversation response based on a specified tag Id.
	 * This default implementation does a lookup into a list of known tags,
	 * and chooses a random dialogue Id from the available ones for that tag
	 */
	virtual int chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag);

	virtual void proc7(int v1, int v2);
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
	virtual bool proc13() const;
	virtual void proc14(int v);
	virtual int proc15() const;
	virtual bool proc16() const;
	virtual bool proc17() const;
	virtual bool proc18() const;
	virtual uint proc19(uint v);
	virtual void proc20(int v);
	virtual int proc21(int v1, int v2, int v3);
	virtual int proc22() const;
	virtual int proc23() const;
	virtual const int *getTablePtr(int id) = 0;
	virtual int proc25() const;
	virtual void proc26();
	virtual void save(SimpleFile *file);
	virtual void load(SimpleFile *file);
	virtual void saveBody(SimpleFile *file);
	virtual void loadBody(SimpleFile *file);
	virtual int proc31();

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
	virtual int getDialRegion(int dialNum);

	/**
	 * Get the NPC's dial level
	 */
	virtual int getDialLevel(uint dialNum, bool flag = true);

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
