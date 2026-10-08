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

#ifndef GLK_OPTIONS_H
#define GLK_OPTIONS_H

#include "common/config-manager.h"
#include "glk/conf.h"

namespace Glk {

enum StyleProperty {
	kStyleFont,
	kStyleForeground,
	kStyleBackground
};

/**
 * Target-owned options state.
 *
 * Stored preferences retain exact configuration strings and key absence.
 * The resolved configuration overlays those preferences on interpreter
 * defaults and application preferences. Dialog widgets edit only drafts;
 * apply() writes their deltas after the containing dialog accepts them.
 */
class GlkOptionsState {
private:
	Common::String _domain;
	Common::ConfigManager::Domain _storedPreferences;
	Common::ConfigManager::Domain _draftPreferences;
	Common::ConfigManager::Domain _applicationPreferences;
	Common::ConfigManager::Domain _editedPreferences;
	Conf _defaults;
	Conf _resolved;

	GlkPreferences preferences(bool inherited = false) const;
	Conf inherited() const;
	Common::String getStoredStyleColor(bool grid, int style, bool foreground) const;

public:
	GlkOptionsState(InterpreterType interpreterType, const Common::String &domain);

	const Conf &resolved();
	Common::String inheritedValue(const Common::String &key) const;
	GlkPreferenceSource source(const Common::String &key) const;
	GlkPreferenceSource styleDefaultSource(bool grid, int style, StyleProperty property) const;
	const Common::String &domain() const { return _domain; }
	void setDomain(const Common::String &domain) { _domain = domain; }

	bool hasDraftPreference(const Common::String &key) const;
	Common::String getDraftPreference(const Common::String &key) const;
	Common::String getString(const Common::String &key,
		const Common::String &fallback) const;
	int getInt(const Common::String &key, int fallback) const;
	bool getBool(const Common::String &key, bool fallback) const;
	FACES getFont(const Common::String &key, FACES fallback) const;
	bool apply();

	Common::String getStyleForeground(bool grid, int style) const;
	Common::String getStyleBackground(bool grid, int style) const;
	FACES getStyleFont(bool grid, int style) const;
	bool hasLowContrast(bool grid, int style) const;
	bool isStylePropertyOverridden(bool grid, int style,
		StyleProperty property) const;
	Common::String getColor(const Common::String &key, uint fallback) const;

	bool commitString(const Common::String &key, const Common::String &draft);
	bool commitStyleColor(bool grid, int style, bool foreground,
		const Common::String &draft);
	bool commitStyleFont(bool grid, int style, FACES draft);
	bool resetStyleProperty(bool grid, int style, StyleProperty property);
	bool removePreference(const Common::String &key);
	bool resetAllPreferences(bool interpreterDefaults = false);

	static bool parseColor(const Common::String &text, Common::String &normalized);
	static bool parseInteger(const Common::String &text, int minimum,
		int maximum, int &value);
	static bool parseFloat(const Common::String &text, double minimum,
		double maximum, double &value);
};

} // End of namespace Glk

#endif
