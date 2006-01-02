/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef KYRA_DEBUGGER_H
#define KYRA_DEBUGGER_H

#include "common/debugger.h"

namespace Kyra {

class KyraEngine;

class Debugger : public Common::Debugger<Debugger> {
public:
	Debugger(KyraEngine *vm);
	virtual ~Debugger() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

protected:
	KyraEngine *_vm;

	virtual void preEnter();
	virtual void postEnter();

	bool cmd_exit(int argc, const char **argv);
	bool cmd_help(int argc, const char **argv);
	bool cmd_enterRoom(int argc, const char **argv);
	bool cmd_listRooms(int argc, const char **argv);
	bool cmd_listFlags(int argc, const char **argv);
	bool cmd_toggleFlag(int argc, const char **argv);
	bool cmd_queryFlag(int argc, const char **argv);
	bool cmd_listTimers(int argc, const char **argv);
	bool cmd_setTimerCountdown(int argc, const char **argv);
	bool cmd_giveItem(int argc, const char **argv);
};

} // End of namespace Kyra

#endif
