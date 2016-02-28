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

#ifndef DEBUGGABLE_SCRIPT_ENGINE_H_
#define DEBUGGABLE_SCRIPT_ENGINE_H_

#include "engines/wintermute/base/scriptables/debuggable/debuggable_script.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/coll_templ.h"
#include "common/algorithm.h"

namespace Wintermute {

class Breakpoint;
class DebuggableScript;
class DebuggableScEngine;
class ScriptMonitor;

class DebuggableScEngine : public ScEngine {
	Common::Array<Breakpoint *> _breakpoints;
	ScriptMonitor *_monitor;
public:
	DebuggableScEngine(BaseGame *inGame);
	void attachMonitor(ScriptMonitor *);

	friend class DebuggerController;
	friend class DebuggableScript;
	friend class ScScript;
};

} // End of namespace Wintermute

#endif /* DEBUGGABLE_SCRIPT_ENGINE_H_ */
