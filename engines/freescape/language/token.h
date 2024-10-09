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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#ifndef FREESCAPE_TOKEN_H
#define FREESCAPE_TOKEN_H

namespace Freescape {

struct Token {
public:
	enum Type {
		ADDVAR,
		AGAIN,
		AND,
		ANDV,
		CONDITIONAL,
		DELAY,
		DESTROY,
		DESTROYEDQ,
		ELSE,
		END,
		ENDGAME,
		ENDIF,
		EXECUTE,
		GOTO,
		IF,
		INVIS,
		INVISQ,
		INCLUDE,
		LOOP,
		MODE,
		MOVE,
		MOVETO,
		NOTV,
		NOP,
		OR,
		ORV,
		GETXPOS,
		GETYPOS,
		GETZPOS,
		PRINT,
		RESTART,
		REDRAW,
		REMOVE,
		SCREEN,
		SOUND,
		SETVAR,
		SETFLAGS,
		START,
		STARTANIM,
		STOPANIM,
		SPFX,
		SUBVAR,
		SYNCSND,
		THEN,
		TOGVIS,
		TRIGANIM,
		UPDATEI,
		VAREQ,
		IFGTEQ,
		IFLTEQ,
		VISQ,
		VIS,
		WAIT,
		WAITTRIG,
		COMMA,
		OPENBRACKET,
		CLOSEBRACKET,
		CONSTANT,
		VARIABLE,
		STRINGLITERAL,
		UNKNOWN,
		ENDOFFILE,
		SETBIT,
		CLEARBIT,
		TOGGLEBIT,
		SWAPJET,
		BITNOTEQ,
		VARNOTEQ
	};

	Type getType();

	Token() {
		_type = UNKNOWN;
		_value = 0;
	}
	Token(Type type_) {
		_type = type_;
		_value = 0;
	}

private:
	Type _type;
	int32 _value;
};

} // End of namespace Freescape

#endif // FREESCAPE_TOKEN_H
