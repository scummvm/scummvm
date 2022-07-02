//
//  Token.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef FREESCAPE_TOKEN_H
#define FREESCAPE_TOKEN_H

#include "common/str.h"

#include "freescape/gamestate.h"

namespace Freescape {

struct Token {
public:
	enum Type {
		ACTIVATEDQ,
		ADDVAR,
		AGAIN,
		AND,
		ANDV,
		COLLIDEDQ,
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
		OR,
		ORV,
		GETXPOS,
		GETYPOS,
		GETZPOS,
		PRINT,
		RESTART,
		REDRAW,
		REMOVE,
		SOUND,
		SETVAR,
		SHOTQ,
		START,
		STARTANIM,
		STOPANIM,
		SUBVAR,
		SYNCSND,
		THEN,
		TIMERQ,
		TOGVIS,
		TRIGANIM,
		UPDATEI,
		VAREQ,
		VARGQ,
		VARLQ,
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

	int32_t getValue(CGameState &, int32 suggestedValue = 0);
	Type getType();

	Token() {
		type = UNKNOWN;
		value = 0;
	}
	Token(Type _type) {
		type = _type;
		value = 0;
	}

	//Token(Common::String &string);
	//Token(Type type, int32_t value);
	//Token(const Token &other);
	//Token &operator=(const Token &rhs);

private:
	Type type;

	int32 value;
	Common::String string;
};

} // End of namespace Freescape

#endif /* defined(__Phantasma__Token__) */
