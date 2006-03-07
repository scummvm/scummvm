/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 */

#include "gui/theme.h"
#include "gui/eval.h"

namespace GUI {

const char *Theme::_defaultConfigINI = "\n\
# Define our classic greenish theme here\n\
[XxY]\n\
def_buttonHeight=kBigButtonHeight\n\
def_kLineHeight=16\n\
chooser_headline=10 6 (w - 2 * 16) (kLineHeight)\n\
chooser_list=10 (6 + kLineHeight + 2) (w - 2 * 16) (h - self.y - buttonHeight - 12)\n\
";

using Common::String;

void Theme::processSingleLine(const String &section, const String name, const String str) {
	int level = 0;
	int start = 0;
	uint i;
	int value;
	const char *postfixes[] = {"x", "y", "w", "h"};
	int npostfix = 0;

	// Make self.BLAH work
	for (i = 0; i < ARRAYSIZE(postfixes); i++) {
		String from, to;

		from = String("self.") + postfixes[i];
		to = name + "." + postfixes[i];

		_evaluator->setAlias(from, to);
		_evaluator->setVariable(to, EVAL_UNDEF_VAR);
	}

	for (i = 0; i < str.size(); i++) {
		if (isspace(str[i]) && level == 0) {
			value = _evaluator->eval(String(&(str.c_str()[start]), i - start), section, name, start);
			_evaluator->setVariable(name + "." + postfixes[npostfix++], value);
			start = i + 1;
		}
		if (str[i] == '(')
			level++;

		if (str[i] == ')') {
			if (level == 0) {
				error("Extra ')' in section: [%s] expression: \"%s\" start is at: %d",
					  section.c_str(), name.c_str(), start);
			}
			level--;
		}
	}

	if (level > 0)
		error("Missing ')' in section: [%s] expression: \"%s\" start is at: %d",
			  section.c_str(), name.c_str(), start);

	value = _evaluator->eval(String(&(str.c_str()[start]), i - start), section, name, start);

	// process VAR=VALUE construct
	if (npostfix == 0)
		_evaluator->setVariable(name, value);
	else
		_evaluator->setVariable(name + "." + postfixes[npostfix], value);

	// If we have all 4 parameters, set .x2 and .y2
	if (npostfix == 4) {
		_evaluator->setVariable(name + ".x2", _evaluator->lookupVar(name + ".x") + 
								_evaluator->lookupVar(name + ".w"));
		_evaluator->setVariable(name + ".y2", _evaluator->lookupVar(name + ".y") + 
								_evaluator->lookupVar(name + ".h"));
	}
}


void Theme::processResSection(Common::ConfigFile &config, String name, bool skipDefs) {
	debug(3, "Reading section: [%s]", name.c_str());

	const Common::ConfigFile::SectionKeyList &keys = config.getKeys(name);

	Common::ConfigFile::SectionKeyList::const_iterator iterk;
	for (iterk = keys.begin(); iterk != keys.end(); ++iterk) {
		if (iterk->key == "set_parent") {
			setParent(iterk->value);
			continue;
		}
		if (iterk->key.hasPrefix("set_")) {
			_evaluator->setAlias(name, iterk->key, iterk->value);
			continue;
		}
		if (iterk->key.hasPrefix("def_")) {
			if (!skipDefs)
				_evaluator->setVariable(name, iterk->key, iterk->value);
			continue;
		}
		if (iterk->key == "use") {
			if (iterk->value == name)
				error("Theme section [%s]: cannot use itself", name.c_str());
			if (!config.hasSection(name))
				error("Undefined use of section [%s]", name.c_str());
			processResSection(config, iterk->value, true);
			continue;
		}
		processSingleLine(name, iterk->key, iterk->value);
	}
}

void Theme::setParent(const String &name) {
	const char *postfixes[] = {"x", "y", "w", "h", "x2", "y2"};
	int i;

	for (i = 0; i < ARRAYSIZE(postfixes); i++) {
		String from, to;

		from = String("parent.") + postfixes[i];
		to = name + "." + postfixes[i];

		_evaluator->setAlias(from, to);
	}
}

bool Theme::isThemeLoadingRequired() {
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (_loadedThemeX == x && _loadedThemeY == y)
		return false;

	_loadedThemeX = x;
	_loadedThemeY = y;

	return true;
}

void Theme::loadTheme(Common::ConfigFile &config, bool reset) {
	char name[80];
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (reset)
		_evaluator->reset();

	strcpy(name, "XxY");
	if (config.hasSection(name))
		processResSection(config, name);

	sprintf(name, "%dxY", x);
	if (config.hasSection(name))
		processResSection(config, name);

	sprintf(name, "%dx%d", x, y);
	if (config.hasSection(name))
		processResSection(config, name);
}

} // End of namespace GUI
