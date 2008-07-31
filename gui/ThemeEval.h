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
 * $URL$
 * $Id$
 *
 */

#ifndef GUI_THEME_EVAL
#define GUI_THEME_EVAL

#include "common/util.h"
#include "common/system.h"
#include "common/events.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/xmlparser.h"

#include "gui/ThemeRenderer.h"
#include "gui/ThemeParser.h"
#include "gui/ThemeEval.h"

namespace GUI {
	
class ThemeEval {

	typedef Common::HashMap<Common::String, int> VariablesMap;
	
public:
	ThemeEval() {}
	~ThemeEval() {}
	
	int getVar(const Common::String &s) {
		if (!_vars.contains(s)) {
			warning("Missing variable: '%s'", s.c_str());
			return -1;
		} 
		
		return _vars[s];
	}
	
	int getVar(const Common::String &s, int def) {
		return (_vars.contains(s)) ? _vars[s] : def;
	}
	
	void setVar(const String &name, int val) { _vars[name] = val; }
	
	void debugPrint() {
		printf("Debug variable list:\n");
		
		VariablesMap::const_iterator i;
		for (i = _vars.begin(); i != _vars.end(); ++i) {
			printf("  '%s' = %d\n", i->_key.c_str(), i->_value);
		}
	}
	
private:
	VariablesMap _vars;
};


}

#endif
