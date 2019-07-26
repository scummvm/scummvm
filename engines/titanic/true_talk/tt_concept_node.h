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

#ifndef TITANIC_TT_CONCEPT_NODE_H
#define TITANIC_TT_CONCEPT_NODE_H

#include "titanic/true_talk/tt_concept.h"

namespace Titanic {

class TTconceptNode {
public:
	TTconcept *_concepts[6];
	TTconcept *&_concept0P;
	TTconcept *&_concept1P;
	TTconcept *&_concept2P;
	TTconcept *&_concept3P;
	TTconcept *&_concept4P;
	TTconcept *&_concept5P;
	int _field18;
	int _field1C;
	TTconceptNode *_nextP;
	int _status;
public:
	TTconceptNode();
	TTconceptNode(const TTconceptNode &src);
	virtual ~TTconceptNode() {}

	/**
	 * Delete any sibling chain attached to this node
	 */
	void deleteSiblings();

	void set18(int val) { _field18 = val; }
	int get18() const { return _field18; }

	/**
	 * Returns true if the node is valid
	 */
	bool isValid() const { return _status == SS_VALID; }

	TTconcept **setConcept(int conceptIndex, TTconcept *src);
	int replaceConcept(int mode, int conceptIndex, TTconcept *concept);
	int changeConcept(int mode, TTconcept **conceptPP, int conceptIndex);
	bool createConcept(int mode, int conceptIndex, TTword *word);

	int concept1WordId() const {
		return getWordId(_concept1P);
	}
	int concept5WordId() const {
		return getWordId(_concept5P);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_TT_CONCEPT_NODE_H */
