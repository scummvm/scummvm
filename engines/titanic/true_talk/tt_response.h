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

#ifndef TITANIC_TT_RESPONSE_H
#define TITANIC_TT_RESPONSE_H

#include "titanic/true_talk/tt_string.h"
namespace Titanic {

class TTsentence;

class TTresponse {
private:
	int _field0;
	TTstring _text;
	int _dialogueId;
	TTresponse *_nextP;
	TTresponse *_linkP;
public:
	TTresponse(const TTstring &src);
	TTresponse(int val1, int val2);
	TTresponse(const TTresponse *src);
	virtual ~TTresponse();

	/**
	 * Creates a new response and adds it as the current
	 * response's next response
	 */
	TTresponse *appendResponse(int id);

	/**
	 * Makes a copy of the chain of responses
	 */
	TTresponse *copyChain() const;

	TTresponse *getLink() const { return _linkP; }

	void addLink(TTresponse *item);

	/**
	 * Get the dialogue Id for the response
	 */
	int getDialogueId() const { return _dialogueId; }

	/**
	 * Return the next response item, if present
	 */
	TTresponse *getNext() const { return _nextP; }
};

} // End of namespace Titanic

#endif /* TITANIC_TT_RESPONSE_H */
