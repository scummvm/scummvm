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

#ifndef TITANIC_TT_SCRIPT_BASE_H
#define TITANIC_TT_SCRIPT_BASE_H

#include "titanic/true_talk/tt_string.h"
#include "titanic/true_talk/tt_hist.h"
#include "titanic/true_talk/tt_node.h"
#include "titanic/true_talk/tt_response.h"

namespace Titanic {

enum ScriptChangedResult {
	SCR_0 = 0, SCR_1 = 1, SCR_2 = 2, SCR_3 = 3, SCR_4 = 4, SCR_5 = 5
};

class TTsentence;

class TTscriptBase {
private:
	void reset();
protected:
	TTnode *_nodesP;
	TThist *_hist1P;
	TTstring _charName, _charClass;
	int _field20;
	int _field24;
	int _field28;
	int _field2C;
	int _field30;
	int _state;
	TThist *_hist2P;
	int _field3C;
	TTresponse *_respTailP;
	TTresponse *_respHeadP;
	TTresponse *_oldResponseP;
	int _status;
protected:
	/**
	 * Delete any responses set up for the script
	 */
	void deleteResponses();

	/**
	 * Creates and appends a new response fragment to the script specified by
	 * the given conversation Id
	 */
	void appendResponse(int index, int *maxP, int id);

	/**
	 * Creates and appends a new response fragment string to the script
	 */
	void appendResponse(int index, int *maxP, const TTstring &str);

	/**
	 * Creates and appends a new response fragment string to the script
	 */
	void appendResponse2(int index, int *maxP, const TTstring &str) {
		appendResponse(index, maxP, str);
	}

	/**
	 * Set the script state
	 */
	void setState(int state) { _state = state; }

	/**
	 * Get the current state
	 */
	int getState() const { return _state; }
public:
	int _id;
public:
	TTscriptBase(int scriptId, const char *charClass, int v2, const char *charName,
		int v3, int v4, int v5, int v6, int v7);
	virtual ~TTscriptBase();

	virtual void addResponse(const TTstring &str);

	virtual void addResponse(int id);

	/**
	 * Passes on the list of dialogue Ids stored in the response(s)
	 * to the title engine for later display in the PET
	 */
	virtual void applyResponse();

	/**
	 * Returns true if the script is in a valid state
	 */
	bool isValid();

	/**
	 * Return the Id of the script
	 */
	int getId() const { return _id; }

	/**
	 * Return the status
	 */
	int getStatus() const { return _status; }

	/**
	 * Return the script text
	 */
	const TTstring getText() { return _charClass; }

	/**
	 * Gets passed a newly created input wrapper during conversation text processing
	 */
	int scriptPreprocess(TTsentence *sentence);

};


} // End of namespace Titanic

#endif /* TITANIC_TT_SCRIPT_BASE_H */
