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

#ifndef WATCH_H_
#define WATCH_H_

#include "common/str.h"

namespace Wintermute {

class ScValue;
class ScScript;
class WatchInstance;
class ScriptMonitor;

class Watch {
	const Common::String _filename;
	const Common::String _symbol;
	int _enabled;
	ScriptMonitor *_monitor;
public:
	Watch(const Common::String &filename, const Common::String &symbol, ScriptMonitor*);
	Common::String getFilename() const;
	Common::String getSymbol() const;
	bool isEnabled() const;
	void enable();
	void disable();
	void trigger(WatchInstance*);
	virtual ~Watch();
};
}
#endif /* WATCH_H_ */
