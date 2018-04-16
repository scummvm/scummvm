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

#include "breakpoint.h"
#include "engines/wintermute/base/scriptables/debuggable/debuggable_script.h"
#include "script_monitor.h"

namespace Wintermute {

Breakpoint::Breakpoint(const Common::String &filename, uint line, ScriptMonitor *monitor) :
	_filename(filename), _line(line), _monitor(monitor), _enabled(0), _hits(0) {}

void Breakpoint::hit(DebuggableScript *script) {
	_hits++;
	_monitor->onBreakpoint(this, script);
}

Common::String Breakpoint::getFilename() const {
	return _filename;
}
int Breakpoint::getLine() const {
	return _line;
}
int Breakpoint::getHits() const {
	return _hits;
}
bool Breakpoint::isEnabled() const {
	return _enabled;
}
void Breakpoint::enable() {
	_enabled = true;
}
void Breakpoint::disable() {
	_enabled = false;
}

void Breakpoint::evaluate(DebuggableScript *script) {
	if (isEnabled() &&
			getLine() == script->_currentLine &&
	        !getFilename().compareTo(script->_filename)) {
		hit(script);
	}
}

Breakpoint::~Breakpoint() {
	// Nothing to take care of in here
}

} // End of namespace Wintermute
