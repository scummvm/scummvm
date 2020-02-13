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

#ifndef TITANIC_TT_ROOM_SCRIPT_H
#define TITANIC_TT_ROOM_SCRIPT_H

#include "titanic/true_talk/tt_script_base.h"

namespace Titanic {

class TTnpcScript;
class TTsentence;

class TTroomScriptBase : public TTscriptBase {
public:
	uint _scriptId;
public:
	TTroomScriptBase(int scriptId, const char *charClass, const char *charName,
		int v3, int v4, int v5, int v6, int v2, int v7);

	/**
	 * Returns true if a response can be made
	 */
	virtual bool canRespond(TTnpcScript *npcScript, TTsentence *sentence, int val) const = 0;

	/**
	 * Returns true if further sentence processing is allowed
	 */
	virtual bool canProcess(TTnpcScript *npcScript, TTsentence *sentence) const = 0;

	virtual bool proc8() const = 0;
	virtual void proc9(int v) = 0;

	/**
	 * Called when the script changes
	 */
	virtual ScriptChangedResult scriptChanged(TTscriptBase *npcScript, int id) = 0;

	virtual bool proc11() const = 0;
};


class TTroomScript : public TTroomScriptBase {
public:
	int _field54;
public:
	TTroomScript(int scriptId);

	/**
	 * Returns true if a response can be made
	 */
	bool canRespond(TTnpcScript *npcScript, TTsentence *sentence, int val) const override {
		return true;
	}

	/**
	 * Returns true if further sentence processing is allowed
	 */
	bool canProcess(TTnpcScript *npcScript, TTsentence *sentence) const override {
		return true;
	}

	bool  proc8() const override;

	void proc9(int v) override;

	/**
	 * Called when the script changes
	 */
	ScriptChangedResult scriptChanged(TTscriptBase *npcScript, int id) override;

	bool proc11() const override;

	/**
	 * Called with the new script and id
	 */
	ScriptChangedResult notifyScript(TTscriptBase *npcScript, int id) {
		return scriptChanged(npcScript, id);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_TT_ROOM_SCRIPT_H */
