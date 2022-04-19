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

#ifndef MTROPOLIS_DEBUG_H
#define MTROPOLIS_DEBUG_H

#include "common/str.h"
#include "common/ptr.h"

#define MTROPOLIS_DEBUG_VTHREAD_STACKS
#define MTROPOLIS_DEBUG_ENABLE

namespace MTropolis {

#ifdef MTROPOLIS_DEBUG_ENABLE
struct IDebuggable;

enum SupportStatus {
	kSupportStatusNone,
	kSupportStatusPartial,
	kSupportStatusDone,
};

class DebugInspector {
public:
	explicit DebugInspector(IDebuggable *debuggable);
	virtual ~DebugInspector();

	virtual void onDestroyed();

private:
	IDebuggable *_debuggable;
};

class Debugger {
public:
	Debugger();
	~Debugger();

	void setPaused(bool paused);
	bool isPaused() const;

private:
	bool _paused;
};

#endif

struct IDebuggable {
#ifdef MTROPOLIS_DEBUG_ENABLE
	virtual SupportStatus debugGetSupportStatus() const = 0;
	virtual const char *debugGetTypeName() const = 0;
	virtual const Common::String &debugGetName() const = 0;
	virtual Common::SharedPtr<DebugInspector> debugGetInspector() const = 0;
#endif
};

} // End of namespace MTropolis

#endif /* MTROPOLIS_DEBUG_H */



