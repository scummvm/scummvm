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

#ifndef MUTATIONOFJB_CONVERSATIONLINELIST_H
#define MUTATIONOFJB_CONVERSATIONLINELIST_H

#include "common/str.h"
#include "common/array.h"

namespace MutationOfJB {

class ConversationLineList {
public:
	struct Speech {
		Common::String _text;
		Common::String _voiceFile;

		bool isRepeating() const {
			return _text.firstChar() == '*';
		}
		bool isFirstSpeaker() const {
			return _text.firstChar() == '~';
		}
		bool isSecondSpeaker() const {
			return _text.firstChar() == '`';
		}
	};

	typedef Common::Array<Speech> Speeches;
	struct Line {
		Speeches _speeches;
		Common::String _extra;
	};

	ConversationLineList(const Common::String &fileName);
	const Line *getLine(uint index) const;

private:
	bool parseFile(const Common::String &fileName);

	Common::Array<Line> _lines;
};

}

#endif
