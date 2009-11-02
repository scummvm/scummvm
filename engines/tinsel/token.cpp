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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * To ensure exclusive use of resources and exclusive control responsibilities.
 */

#include "common/util.h"

#include "tinsel/sched.h"
#include "tinsel/token.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

struct Token {
	PROCESS		*proc;
};

static Token tokens[NUMTOKENS];


/**
 * Release all tokens held by this process, and kill the process.
 */
static void TerminateProcess(PROCESS *tProc) {

	// Release tokens held by the process
	for (int i = 0; i < NUMTOKENS; i++) {
		if (tokens[i].proc == tProc) {
			tokens[i].proc = NULL;
		}
	}

	// Kill the process
	g_scheduler->killProcess(tProc);
}

/**
 * Gain control of the CONTROL token if it is free.
 */
void GetControlToken() {
	const int which = TOKEN_CONTROL;

	if (tokens[which].proc == NULL) {
		tokens[which].proc = g_scheduler->getCurrentProcess();
	}
}

/**
 * Release control of the CONTROL token.
 */
void FreeControlToken() {
	// Allow anyone to free TOKEN_CONTROL
	tokens[TOKEN_CONTROL].proc = NULL;
}


/**
 * Gain control of a token. If the requested token is out of range, or
 * is already held by the calling process, then the calling process
 * will be killed off.
 *
 * Otherwise, the calling process will gain the token. If the token was
 * held by another process, then the previous holder is killed off.
 */
void GetToken(int which) {
	assert(TOKEN_LEAD <= which && which < NUMTOKENS);

	if (tokens[which].proc != NULL) {
		assert(tokens[which].proc != g_scheduler->getCurrentProcess());
		TerminateProcess(tokens[which].proc);
	}

	tokens[which].proc = g_scheduler->getCurrentProcess();
}

/**
 * Release control of a token. If the requested token is not owned by
 * the calling process, then the calling process will be killed off.
 */
void FreeToken(int which) {
	assert(TOKEN_LEAD <= which && which < NUMTOKENS);

	assert(tokens[which].proc == g_scheduler->getCurrentProcess());	// we'd have been killed if some other proc had taken this token

	tokens[which].proc = NULL;
}

/**
 * If it's a valid token and it's free, returns true.
 */
bool TestToken(int which) {
	if (which < 0 || which >= NUMTOKENS)
		return false;

	return (tokens[which].proc == NULL);
}

/**
 * Call at the start of each scene.
 */
void FreeAllTokens() {
	for (int i = 0; i < NUMTOKENS; i++) {
		tokens[i].proc = NULL;
	}
}

} // End of namespace Tinsel
