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

#include "glk/preferences.h"
#include "glk/screen.h"
#include "common/util.h"

#include <errno.h>
#include <limits.h>

namespace Glk {

static const GlkOptionContract GLK_OPTION_CONTRACTS[] = {
	{ "tfont_*", kGlkOptionFont, MONOR, PROPZ },
	{ "gfont_*", kGlkOptionFont, MONOR, MONOZ },
	{ "tcolor_*", kGlkOptionColor, 0, 0xffffff },
	{ "gcolor_*", kGlkOptionColor, 0, 0xffffff },
	{ "windowcolor", kGlkOptionColor, 0, 0xffffff },
	{ "windowcolor_override", kGlkOptionBoolean, 0, 1 },
	{ "bordercolor", kGlkOptionColor, 0, 0xffffff },
	{ "bordercolor_override", kGlkOptionBoolean, 0, 1 },
	{ "caretcolor", kGlkOptionColor, 0, 0xffffff },
	{ "linkcolor", kGlkOptionColor, 0, 0xffffff },
	{ "morecolor", kGlkOptionColor, 0, 0xffffff },
	{ "monosize", kGlkOptionFloat, 10, 25 },
	{ "propsize", kGlkOptionFloat, 10, 25 },
	{ "leading", kGlkOptionInteger, 0, 200 },
	{ "baseline", kGlkOptionInteger, 0, 200 },
	{ "wmarginx", kGlkOptionInteger, 0, 200 },
	{ "wmarginy", kGlkOptionInteger, 0, 200 },
	{ "wpaddingx", kGlkOptionInteger, 0, 200 },
	{ "wpaddingy", kGlkOptionInteger, 0, 200 },
	{ "wborderx", kGlkOptionInteger, 0, 200 },
	{ "wbordery", kGlkOptionInteger, 0, 200 },
	{ "tmarginx", kGlkOptionInteger, 0, 200 },
	{ "tmarginy", kGlkOptionInteger, 0, 200 },
	{ "cols", kGlkOptionInteger, 0, 999 },
	{ "rows", kGlkOptionInteger, 0, 999 },
	{ "lockcols", kGlkOptionBoolean, 0, 1 },
	{ "lockrows", kGlkOptionBoolean, 0, 1 },
	{ "justify", kGlkOptionBoolean, 0, 1 },
	{ "caps", kGlkOptionBoolean, 0, 1 },
	{ "graphics", kGlkOptionBoolean, 0, 1 },
	{ "stylehint", kGlkOptionBoolean, 0, 1 },
	{ "safeclicks", kGlkOptionBoolean, 0, 1 },
	{ "quotes", kGlkOptionInteger, 0, 2 },
	{ "dashes", kGlkOptionInteger, 0, 2 },
	{ "spaces", kGlkOptionInteger, 0, 2 },
	{ "caretshape", kGlkOptionInteger, 0, 4 },
	{ "linkstyle", kGlkOptionInteger, 0, 2 },
	{ "moreprompt", kGlkOptionString, 0, 0 },
	{ "morealign", kGlkOptionInteger, 0, 2 },
	{ "morefont", kGlkOptionFont, MONOR, PROPZ }
};

const GlkOptionContract *findGlkOptionContract(const Common::String &key) {
	const uint count = ARRAYSIZE(GLK_OPTION_CONTRACTS);
	const GlkOptionContract *contracts = GLK_OPTION_CONTRACTS;
	for (uint index = 0; index < count; ++index) {
		const Common::String contractKey(contracts[index].key);
		bool matches = key == contractKey;
		if (contractKey.hasSuffix("*")) {
			const Common::String prefix = contractKey.substr(0, contractKey.size() - 1);
			for (int style = 0; style < style_NUMSTYLES; ++style) {
				const Common::String styleKey = prefix + Common::String::format("%d", style);
				if (key == styleKey || (contracts[index].type == kGlkOptionColor &&
						(key == styleKey + "_fg_override" || key == styleKey + "_bg_override")))
					matches = true;
			}
		}
		if (matches)
			return &contracts[index];
	}
	return nullptr;
}


GlkPreferences::GlkPreferences(const Common::ConfigManager::Domain *target, bool runtime,
		const Common::ConfigManager::Domain *application, bool inherited)
		: _ignoreGlobalAppearance(false), _inheritAppearance(inherited) {
	if (target && target->contains("glk_ignore_global_appearance"))
		parseBool(target->getVal("glk_ignore_global_appearance"), _ignoreGlobalAppearance);
	_layers[0] = runtime ? ConfMan.getDomain(Common::ConfigManager::kTransientDomain) : nullptr;
	_layers[1] = runtime ? ConfMan.getDomain(Common::ConfigManager::kSessionDomain) : nullptr;
	_layers[2] = target;
	_layers[3] = application ? application : ConfMan.getDomain(Common::ConfigManager::kApplicationDomain);
}

bool GlkPreferences::useLayer(int layer, const Common::String &key) const {
	// Configuration-only target constraints still apply to inherited values.
	return _layers[layer] && !(findGlkOptionContract(key) &&
		((layer == 2 && _inheritAppearance) || (layer == 3 && _ignoreGlobalAppearance)));
}

static bool parsePreferenceBool(const Common::String &key, const Common::String &text, bool &value) {
	if (GlkPreferences::parseBool(text, value))
		return true;
	// These historical integer switches treat every nonzero value as enabled.
	if (key == "stylehint" || key == "lockcols" || key == "lockrows" ||
			key == "justify" || key == "caps") {
		int integer;
		if (GlkPreferences::parseInteger(text, integer)) {
			value = integer != 0;
			return true;
		}
	}
	return false;
}

bool GlkPreferences::isValidStoredValue(const Common::String &key, const Common::String &value) {
	const GlkOptionContract *contract = findGlkOptionContract(key);
	if (!contract)
		return false;
	int integer;
	double number;
	bool enabled;
	FACES font;
	Common::String color;
	switch (contract->type) {
	case kGlkOptionInteger:
		return parseInteger(value, integer) && integer >= contract->minimum && integer <= contract->maximum;
	case kGlkOptionFloat:
		// Promotion retains the legacy loading range, not edited-control limits.
		return parseFloat(value, 0, 32767, number);
	case kGlkOptionBoolean:
		return parsePreferenceBool(key, value, enabled);
	case kGlkOptionFont:
		return parseFont(value, font);
	case kGlkOptionColor:
		return parseColor(value, color);
	default:
		return true;
	}
}

bool GlkPreferences::equal(const Common::String &key, const Common::String &left,
		const Common::String &right) {
	const GlkOptionContract *contract = findGlkOptionContract(key);
	if (!contract)
		return left == right;
	int li, ri;
	double lf, rf;
	bool lb, rb;
	FACES lfont, rfont;
	Common::String lc, rc;
	switch (contract->type) {
	case kGlkOptionInteger:
		return parseInteger(left, li) && parseInteger(right, ri) && li == ri;
	case kGlkOptionFloat:
		return parseFloat(left, 0, 32767, lf) && parseFloat(right, 0, 32767, rf) && lf == rf;
	case kGlkOptionBoolean:
		return parsePreferenceBool(key, left, lb) && parsePreferenceBool(key, right, rb) && lb == rb;
	case kGlkOptionFont:
		return parseFont(left, lfont) && parseFont(right, rfont) && lfont == rfont;
	case kGlkOptionColor:
		return parseColor(left, lc) && parseColor(right, rc) && lc == rc;
	default:
		return left == right;
	}
}

GlkPreferenceSource GlkPreferences::source(const Common::String &key) const {
	for (int i = 0; i < 4; ++i) {
		if (useLayer(i, key) && _layers[i]->contains(key))
			return i < 2 ? kGlkTemporaryPreference : (i == 2 ? kGlkTargetPreference : kGlkApplicationPreference);
	}
	return kGlkInterpreterDefault;
}

Common::String GlkPreferences::get(const Common::String &key, const Common::String &fallback) const {
	for (int i = 0; i < 4; ++i) {
		if (useLayer(i, key) && _layers[i]->contains(key))
			return _layers[i]->getVal(key);
	}
	return fallback;
}

GlkPreferenceSource GlkPreferences::styleColor(bool grid, int style, bool foreground, Common::String &color) const {
	if (style < 0 || style >= style_NUMSTYLES)
		return kGlkInterpreterDefault;
	const Common::String key = Common::String::format("%ccolor_%d", grid ? 'g' : 't', style);
	const Common::String marker = key + (foreground ? "_fg_override" : "_bg_override");
	for (int i = 0; i < 4; ++i) {
		if (!useLayer(i, key) || !_layers[i]->contains(key))
			continue;
		bool enabled = true;
		if (_layers[i]->contains(marker))
			parseBool(_layers[i]->getVal(marker), enabled);
		const Common::String pair = _layers[i]->getVal(key);
		if (enabled && pair.size() == 13 && pair[6] == ',' &&
				parseColor(pair.substr(foreground ? 0 : 7, 6), color))
			return i < 2 ? kGlkTemporaryPreference : (i == 2 ? kGlkTargetPreference : kGlkApplicationPreference);
	}
	return kGlkInterpreterDefault;
}

bool GlkPreferences::parseColor(const Common::String &text, Common::String &normalized) {
	const uint offset = text.hasPrefix("#") ? 1 : 0;
	if (text.size() != offset + 6)
		return false;
	for (uint i = offset; i < text.size(); ++i) {
		if (!Common::isXDigit(text[i]))
			return false;
	}
	normalized = text.substr(offset);
	normalized.toLowercase();
	return true;
}

bool GlkPreferences::parseInteger(const Common::String &text, int &value, int base) {
	if (text.empty())
		return false;
	char *end = nullptr;
	errno = 0;
	const long parsed = strtol(text.c_str(), &end, base);
	if (errno == ERANGE || end == text.c_str() || *end || parsed < INT_MIN || parsed > INT_MAX)
		return false;
	value = (int)parsed;
	return true;
}

bool GlkPreferences::parseBool(const Common::String &text, bool &value) {
	if (text.equalsIgnoreCase("true") || text.equalsIgnoreCase("yes") || text == "1")
		value = true;
	else if (text.equalsIgnoreCase("false") || text.equalsIgnoreCase("no") || text == "0")
		value = false;
	else
		return false;
	return true;
}

bool GlkPreferences::parseFloat(const Common::String &text, double minimum, double maximum, double &value) {
	if (text.empty())
		return false;
	char *end = nullptr;
	errno = 0;
	const double parsed = strtod(text.c_str(), &end);
	// Ordered comparisons also reject NaN and either infinity.
	if (errno == ERANGE || end == text.c_str() || *end || !(parsed >= minimum && parsed <= maximum))
		return false;
	value = parsed;
	return true;
}

bool GlkPreferences::parseFont(const Common::String &text, FACES &value) {
	for (int i = MONOR; i <= PROPZ; ++i) {
		if (text.equalsIgnoreCase(Screen::getFontName((FACES)i))) {
			value = (FACES)i;
			return true;
		}
	}
	return false;
}

} // End of namespace Glk
