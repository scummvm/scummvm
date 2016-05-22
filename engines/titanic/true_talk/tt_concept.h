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

#ifndef TITANIC_TT_CONCEPT_H
#define TITANIC_TT_CONCEPT_H

#include "titanic/true_talk/tt_string.h"

namespace Titanic {

enum ScriptType { ST_UNKNOWN_SCRIPT = 0, ST_ROOM_SCRIPT = 1, ST_NPC_SCRIPT = 2 };

class TTscriptBase;
class TTword;

class TTconcept {
private:
	TTscriptBase *_scriptP;
	TTword *_wordP;
	TTstring _string1;
	int _field14;
	ScriptType _scriptType;
	int _field1C;
	int _field20;
	TTstring _string2;
	TTword *_word2P;
	int _field30;
	int _field34;
	bool _flag;
	int _status;
private:
	/**
	 * Sets the status of the concept
	 */
	bool setStatus();

	/**
	 * Sets the script type and resets other fields
	 */
	void setScriptType(ScriptType scriptType);

	/**
	 * Sets up the concept for a word reference
	 */
	int initializeWordRef(TTword *word);

	/**
	 * Resets the concept
	 */
	void reset();

	/**
	 * Initialize inner data for the concept from a given source concept
	 */
	void initialize(TTconcept &src);
public:
	TTconcept *_nextP;
public:
	TTconcept();
	TTconcept(TTscriptBase *script, ScriptType scriptType);
	TTconcept(TTword *word, ScriptType scriptType);
	TTconcept(TTconcept &src);
	~TTconcept();

	/**
	 * Destroys any attached sibling concepts to the given concept
	 */
	void deleteSiblings();

	/**
	 * Copies data from a source concept
	 */
	void copyFrom(TTconcept *src);

	/**
	 * Compares the name of the associated word, if any,
	 * to the passed string
	 */
	bool compareTo(const char *str) const;

	/**
	 * Return the status of the concept
	 */
	int getStatus() const { return _status; }

	void setFlag(bool val) { _flag = val; }
};

} // End of namespace Titanic

#endif /* TITANIC_TT_CONCEPT_H */
