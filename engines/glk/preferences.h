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

#ifndef GLK_PREFERENCES_H
#define GLK_PREFERENCES_H

#include "common/config-manager.h"
#include "glk/fonts.h"

namespace Glk {

enum GlkOptionValueType {
	kGlkOptionBoolean,
	kGlkOptionInteger,
	kGlkOptionFloat,
	kGlkOptionString,
	kGlkOptionColor,
	kGlkOptionFont
};

struct GlkOptionContract {
	const char *key;
	GlkOptionValueType type;
	int minimum;
	int maximum;

};

const GlkOptionContract *findGlkOptionContract(const Common::String &key);

enum GlkPreferenceSource {
	kGlkInterpreterDefault,
	kGlkApplicationPreference,
	kGlkTargetPreference,
	kGlkTemporaryPreference
};

/** Read-only configuration layers. Temporary values belong to runtime only. */
class GlkPreferences {
	const Common::ConfigManager::Domain *_layers[4];
	bool _ignoreGlobalAppearance;
	bool _inheritAppearance;
	bool useLayer(int layer, const Common::String &key) const;
public:
	GlkPreferences(const Common::ConfigManager::Domain *target, bool runtime = false,
		const Common::ConfigManager::Domain *application = nullptr, bool inherited = false);
	// Scalar promotion validation; style pairs and grid restrictions are checked per property.
	static bool isValidStoredValue(const Common::String &key, const Common::String &value);
	static bool equal(const Common::String &key, const Common::String &left, const Common::String &right);
	GlkPreferenceSource source(const Common::String &key) const;
	Common::String get(const Common::String &key, const Common::String &fallback = Common::String()) const;
	GlkPreferenceSource styleColor(bool grid, int style, bool foreground, Common::String &color) const;
	static bool parseColor(const Common::String &text, Common::String &normalized);
	static bool parseInteger(const Common::String &text, int &value, int base = 0);
	static bool parseBool(const Common::String &text, bool &value);
	static bool parseFloat(const Common::String &text, double minimum, double maximum, double &value);
	static bool parseFont(const Common::String &text, FACES &value);
};

} // End of namespace Glk

#endif
