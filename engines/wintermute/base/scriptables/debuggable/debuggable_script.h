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

#ifndef DEBUGGABLE_SCRIPT_H_
#define DEBUGGABLE_SCRIPT_H_
#include "engines/wintermute/base/scriptables/script.h"

namespace Wintermute {
class ScriptMonitor;
class Watch;
class WatchInstance;
class DebuggableScEngine;

class DebuggableScript : public ScScript {
	static const int kDefaultStepDepth = -2;
	int32 _stepDepth;
	DebuggableScEngine *_engine;
	BaseArray<WatchInstance *> _watchInstances;
	void preInstHook(uint32 inst) override;
	void postInstHook(uint32 inst) override;
	void setStepDepth(int depth);
public:
	DebuggableScript(BaseGame *inGame, DebuggableScEngine *engine);
	~DebuggableScript() override;
	ScValue *resolveName(const Common::String &name);
	/**
	 * Return argument to last II_DBG_LINE encountered
	 */
	virtual uint dbgGetLine() const;
	virtual Common::String dbgGetFilename() const;
	/**
	 * Execute one more instruction
	 */
	void step();
	/**
	 * Continue execution
	 */
	void stepContinue();
	/**
	 * Continue execution until the activation record on top of the stack is popped
	 */
	void stepFinish();
	void updateWatches();
};

} // End of namespace Wintermute

#endif /* DEBUGGABLE_SCRIPT_H_ */
