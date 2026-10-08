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

#include "common/array.h"
#include "common/util.h"
#include "glk/options.h"
#include "glk/screen.h"

namespace Glk {

GlkOptionsState::GlkOptionsState(InterpreterType interpreterType,
		const Common::String &domain) : _domain(domain),
		_defaults(interpreterType, false, Graphics::PixelFormat(4, 8, 8, 8, 0, 16, 8, 0, 0)),
		_resolved(_defaults) {
	const Common::ConfigManager::Domain *stored = ConfMan.getDomain(domain);
	if (stored)
		_storedPreferences = *stored;
	_draftPreferences = _storedPreferences;
	_applicationPreferences = *ConfMan.getDomain(Common::ConfigManager::kApplicationDomain);
	_storedApplication = _applicationPreferences;
}

GlkPreferences GlkOptionsState::preferences(bool inherited) const {
	return GlkPreferences(&_draftPreferences, false, &_applicationPreferences, inherited);
}

Conf GlkOptionsState::inherited() const {
	Conf defaults(_defaults);
	defaults.load(preferences(true));
	return defaults;
}

Common::String GlkOptionsState::inheritedValue(const Common::String &key) const {
	const Conf defaults = inherited();
	const struct { const char *key; int value; } integers[] = {
		{ "wmarginx", defaults._wMarginX },
		{ "wmarginy", defaults._wMarginY },
		{ "wpaddingx", defaults._wPaddingX },
		{ "wpaddingy", defaults._wPaddingY },
		{ "wborderx", defaults._wBorderX },
		{ "wbordery", defaults._wBorderY },
		{ "tmarginx", defaults._tMarginX },
		{ "tmarginy", defaults._tMarginY },
		{ "leading", defaults._monoInfo._leading },
		{ "baseline", defaults._propInfo._baseLine },
		{ "cols", defaults._cols },
		{ "rows", defaults._rows },
		{ "lockcols", defaults._lockCols },
		{ "lockrows", defaults._lockRows },
		{ "justify", defaults._propInfo._justify },
		{ "caps", defaults._propInfo._caps },
		{ "graphics", defaults._graphics },
		{ "stylehint", defaults._styleHint },
		{ "safeclicks", defaults._safeClicks },
		{ "quotes", defaults._propInfo._quotes },
		{ "dashes", defaults._propInfo._dashes },
		{ "spaces", defaults._propInfo._spaces },
		{ "caretshape", defaults._propInfo._caretShape },
		{ "linkstyle", defaults._propInfo._linkStyle },
		{ "morealign", defaults._propInfo._moreAlign }
	};
	for (uint i = 0; i < ARRAYSIZE(integers); ++i)
		if (key == integers[i].key)
			return Common::String::format("%d", integers[i].value);
	if (key == "propsize" || key == "monosize")
		return Common::String::format("%.17g", key == "propsize" ? defaults._propInfo._size : defaults._monoInfo._size);
	if (key == "moreprompt")
		return defaults._propInfo._morePrompt;
	if (key == "morefont")
		return Screen::getFontName(defaults._propInfo._moreFont);
	const struct { const char *key; uint value; } colors[] = {
		{ "windowcolor", defaults._windowColor }, { "bordercolor", defaults._borderColor },
		{ "caretcolor", defaults._propInfo._caretColor }, { "linkcolor", defaults._propInfo._linkColor },
		{ "morecolor", defaults._propInfo._moreColor }
	};
	for (uint i = 0; i < ARRAYSIZE(colors); ++i)
		if (key == colors[i].key)
			return defaults.encodeColor(colors[i].value);
	for (int grid = 0; grid < 2; ++grid) {
		for (int style = 0; style < style_NUMSTYLES; ++style) {
			if (key == Common::String::format("%cfont_%d", grid ? 'g' : 't', style))
				return Screen::getFontName((grid ? defaults._gStyles : defaults._tStyles)[style].font);
		}
	}
	return Common::String();
}

bool GlkOptionsState::hasDraftPreference(const Common::String &key) const {
	return _draftPreferences.contains(key);
}

Common::String GlkOptionsState::getDraftPreference(const Common::String &key) const {
	return _draftPreferences.getValOrDefault(key);
}

const Conf &GlkOptionsState::resolved() {
	_resolved = _defaults;
	_resolved.load(preferences());
	return _resolved;
}

GlkPreferenceSource GlkOptionsState::source(const Common::String &key) const {
	return preferences().source(key);
}

GlkPreferenceSource GlkOptionsState::styleDefaultSource(bool grid, int style, StyleProperty property) const {
	const GlkPreferences defaults = preferences(true);
	if (property == kStyleFont) {
		const Common::String key = Common::String::format("%cfont_%d", grid ? 'g' : 't', style);
		FACES font;
		return GlkPreferences::parseFont(defaults.get(key), font) ?
			defaults.source(key) : kGlkInterpreterDefault;
	}
	Common::String color;
	return defaults.styleColor(grid, style, property == kStyleForeground, color);
}

Common::String GlkOptionsState::getString(const Common::String &key, const Common::String &fallback) const {
	return preferences().get(key, fallback);
}

int GlkOptionsState::getInt(const Common::String &key, int fallback) const {
	int value = fallback;
	GlkPreferences::parseInteger(getString(key, ""), value);
	return value;
}

bool GlkOptionsState::getBool(const Common::String &key, bool fallback) const {
	bool value = fallback;
	GlkPreferences::parseBool(getString(key, ""), value);
	return value;
}

FACES GlkOptionsState::getFont(const Common::String &key, FACES fallback) const {
	FACES value = fallback;
	GlkPreferences::parseFont(getString(key, ""), value);
	return value;
}

static bool applyPreferenceDelta(const Common::ConfigManager::Domain &stored,
		const Common::ConfigManager::Domain &draft, Common::ConfigManager::Domain *destination) {
	bool changed = false;
	for (Common::ConfigManager::Domain::const_iterator it = stored.begin(); it != stored.end(); ++it) {
		if (!draft.contains(it->_key)) {
			if (destination)
				destination->erase(it->_key);
			changed = true;
		}
	}
	for (Common::ConfigManager::Domain::const_iterator it = draft.begin(); it != draft.end(); ++it) {
		if (!stored.contains(it->_key) || stored.getVal(it->_key) != it->_value) {
			if (destination)
				destination->setVal(it->_key, it->_value);
			changed = true;
		}
	}
	return changed;
}

bool GlkOptionsState::hasGlobalChanges() const {
	return applyPreferenceDelta(_storedApplication, _applicationPreferences, nullptr);
}

bool GlkOptionsState::apply() {
	Common::ConfigManager::Domain *target = ConfMan.getDomain(_domain);
	if (!target || !ConfMan.hasGameDomain(_domain))
		return false;
	bool changed = applyPreferenceDelta(_storedPreferences, _draftPreferences, target);
	changed = applyPreferenceDelta(_storedApplication, _applicationPreferences,
		ConfMan.getDomain(Common::ConfigManager::kApplicationDomain)) || changed;
	_storedPreferences = _draftPreferences;
	_storedApplication = _applicationPreferences;
	_editedPreferences.clear();
	return changed;
}

// Preserve the other half's spelling; a new inactive half contains no default.
static bool setStyleColorPreference(Common::ConfigManager::Domain &domain,
		const Common::String &key, bool foreground, const Common::String &color) {
	const Common::String marker = key + (foreground ? "_fg_override" : "_bg_override");
	const Common::String otherMarker = key + (foreground ? "_bg_override" : "_fg_override");
	const Common::String stored = domain.getValOrDefault(key);
	const bool pairValid = stored.size() == 13 && stored[6] == ',';
	bool otherEnabled = pairValid;
	if (domain.contains(otherMarker))
		GlkPreferences::parseBool(domain.getVal(otherMarker), otherEnabled);
	const Common::String other = pairValid ? stored.substr(foreground ? 7 : 0, 6) : Common::String("000000");
	const Common::String pair = foreground ? color + "," + other : other + "," + color;
	bool enabled = false;
	if (domain.contains(marker))
		GlkPreferences::parseBool(domain.getVal(marker), enabled);
	if (stored == pair && enabled)
		return false;
	domain.setVal(key, pair);
	domain.setVal(marker, "true");
	if (!domain.contains(otherMarker))
		domain.setVal(otherMarker, otherEnabled ? "true" : "false");
	return true;
}

bool GlkOptionsState::stageGlobalPreferences(Common::String &invalidKey) {
	Common::ConfigManager::Domain application(_applicationPreferences);
	invalidKey.clear();
	for (Common::ConfigManager::Domain::const_iterator it = _draftPreferences.begin();
			it != _draftPreferences.end(); ++it) {
		const GlkOptionContract *contract = findGlkOptionContract(it->_key);
		if (!contract || Common::String(contract->key).hasSuffix("*") ||
				it->_key == "windowcolor_override" || it->_key == "bordercolor_override")
			continue;
		if (!GlkPreferences::isValidStoredValue(it->_key, it->_value)) {
			invalidKey = it->_key;
			return false;
		}
		if (!application.contains(it->_key) ||
				!GlkPreferences::equal(it->_key, it->_value, application.getVal(it->_key)))
			application.setVal(it->_key, it->_value);
	}
	for (int grid = 0; grid < 2; ++grid) {
		for (int style = 0; style < style_NUMSTYLES; ++style) {
			const Common::String fontKey = Common::String::format("%cfont_%d", grid ? 'g' : 't', style);
			FACES font;
			if (_draftPreferences.contains(fontKey)) {
				const Common::String value = _draftPreferences.getVal(fontKey);
				if (!GlkPreferences::parseFont(value, font) || (grid && font >= PROPR)) {
					invalidKey = fontKey;
					return false;
				}
				if (!application.contains(fontKey) || !GlkPreferences::equal(fontKey, value, application.getVal(fontKey)))
					application.setVal(fontKey, value);
			}
			const Common::String key = Common::String::format("%ccolor_%d", grid ? 'g' : 't', style);
			for (int property = 0; property < 2; ++property) {
				const bool foreground = property == 0;
				const Common::String marker = key + (foreground ? "_fg_override" : "_bg_override");
				bool enabled = true;
				if (_draftPreferences.contains(marker) &&
						!GlkPreferences::parseBool(_draftPreferences.getVal(marker), enabled)) {
					invalidKey = marker;
					return false;
				}
				if (!_draftPreferences.contains(key) || !enabled)
					continue;
				const Common::String pair = _draftPreferences.getVal(key);
				Common::String color;
				if (pair.size() != 13 || pair[6] != ',' ||
						!parseColor(pair.substr(foreground ? 0 : 7, 6), color)) {
					invalidKey = key;
					return false;
				}
				const GlkPreferences global(nullptr, false, &application);
				Common::String current;
				if (global.styleColor(grid, style, foreground, current) == kGlkInterpreterDefault || current != color)
					setStyleColorPreference(application, key, foreground, pair.substr(foreground ? 0 : 7, 6));
			}
		}
	}
	_applicationPreferences = application;
	// Only edited properties may stop pinning this target to their old values.
	const Common::ConfigManager::Domain edits(_editedPreferences);
	for (Common::ConfigManager::Domain::const_iterator it = edits.begin(); it != edits.end(); ++it) {
		if (!it->_key.hasSuffix("_override"))
			commitString(it->_key, it->_value);
	}
	for (int grid = 0; grid < 2; ++grid) {
		for (int style = 0; style < style_NUMSTYLES; ++style) {
			for (int property = 0; property < 2; ++property) {
				const Common::String marker = Common::String::format("%ccolor_%d_%s_override",
					grid ? 'g' : 't', style, property == 0 ? "fg" : "bg");
				if (edits.contains(marker))
					commitStyleColor(grid, style, property == 0, edits.getVal(marker));
			}
		}
	}
	return true;
}

bool GlkOptionsState::parseColor(const Common::String &text, Common::String &normalized) {
	return GlkPreferences::parseColor(text, normalized);
}

bool GlkOptionsState::parseInteger(const Common::String &text, int minimum, int maximum, int &value) {
	int parsed;
	if (!GlkPreferences::parseInteger(text, parsed, 10) || parsed < minimum || parsed > maximum)
		return false;
	value = parsed;
	return true;
}

bool GlkOptionsState::parseFloat(const Common::String &text, double minimum, double maximum, double &value) {
	// Edited values use decimal syntax; legacy loading remains more permissive.
	bool digit = false;
	bool point = false;
	for (uint i = 0; i < text.size(); ++i) {
		if (Common::isDigit(text[i]))
			digit = true;
		else if (text[i] == '.' && !point)
			point = true;
		else
			return false;
	}
	return digit && GlkPreferences::parseFloat(text, minimum, maximum, value);
}

Common::String GlkOptionsState::getStoredStyleColor(bool grid, int style, bool foreground) const {
	Common::String color;
	if (preferences().styleColor(grid, style, foreground, color) != kGlkInterpreterDefault)
		return color;
	const WindowStyle *styles = grid ? _defaults._gStyles : _defaults._tStyles;
	return _defaults.encodeColor(foreground ? styles[style].fg : styles[style].bg);
}

Common::String GlkOptionsState::getStyleForeground(bool grid, int style) const {
	return getStoredStyleColor(grid, style, true);
}

Common::String GlkOptionsState::getStyleBackground(bool grid, int style) const {
	return getStoredStyleColor(grid, style, false);
}

FACES GlkOptionsState::getStyleFont(bool grid, int style) const {
	const Common::String key = Common::String::format("%cfont_%d",
		grid ? 'g' : 't', style);
	const WindowStyle *styles = grid ? _defaults._gStyles : _defaults._tStyles;
	return getFont(key, styles[style].font);
}

bool GlkOptionsState::hasLowContrast(bool grid, int style) const {
	const uint foreground = strtol(
		getStyleForeground(grid, style).c_str(), nullptr, 16);
	const uint background = strtol(
		getStyleBackground(grid, style).c_str(), nullptr, 16);
	const int foregroundLuma = (((foreground >> 16) & 0xff) * 299 +
		((foreground >> 8) & 0xff) * 587 + (foreground & 0xff) * 114) /
		1000;
	const int backgroundLuma = (((background >> 16) & 0xff) * 299 +
		((background >> 8) & 0xff) * 587 + (background & 0xff) * 114) /
		1000;
	return ABS(foregroundLuma - backgroundLuma) < 64;
}

bool GlkOptionsState::isStylePropertyOverridden(bool grid, int style,
		StyleProperty property) const {
	const char prefix = grid ? 'g' : 't';
	if (property == kStyleFont)
		return _draftPreferences.contains(Common::String::format(
			"%cfont_%d", prefix, style));

	const Common::String key = Common::String::format("%ccolor_%d",
		prefix, style);
	if (!_draftPreferences.contains(key))
		return false;
	const Common::String marker = key +
		(property == kStyleForeground ? "_fg_override" : "_bg_override");
	return !_draftPreferences.contains(marker) || getBool(marker, true);
}

Common::String GlkOptionsState::getColor(const Common::String &key, uint fallback) const {
	Common::String normalized;
	return parseColor(getString(key, ""), normalized) ? normalized : _defaults.encodeColor(fallback);
}

bool GlkOptionsState::commitString(const Common::String &key, const Common::String &draft) {
	_editedPreferences.setVal(key, draft);
	if (GlkPreferences::equal(key, draft, inheritedValue(key))) {
		const bool changed = _draftPreferences.contains(key);
		_draftPreferences.erase(key);
		return changed;
	}
	if (_draftPreferences.contains(key) && _draftPreferences.getVal(key) == draft)
		return false;
	_draftPreferences.setVal(key, draft);
	return true;
}

bool GlkOptionsState::commitStyleColor(bool grid, int style,
		bool foreground, const Common::String &draft) {
	if (style < 0 || style >= style_NUMSTYLES)
		return false;
	Common::String normalized;
	if (!parseColor(draft, normalized))
		return false;

	const Conf defaults = inherited();
	const WindowStyle &defaultStyle = grid ? defaults._gStyles[style] : defaults._tStyles[style];
	const Common::String key = Common::String::format("%ccolor_%d", grid ? 'g' : 't', style);
	const Common::String marker = key + (foreground ? "_fg_override" : "_bg_override");
	bool changed;
	if (normalized == defaults.encodeColor(foreground ? defaultStyle.fg : defaultStyle.bg))
		changed = resetStyleProperty(grid, style, foreground ? kStyleForeground : kStyleBackground);
	else
		changed = setStyleColorPreference(_draftPreferences, key, foreground, normalized);
	_editedPreferences.setVal(marker, normalized);
	return changed;
}

bool GlkOptionsState::commitStyleFont(bool grid, int style, FACES draft) {
	if (style < 0 || style >= style_NUMSTYLES || draft < MONOR ||
			draft > PROPZ || (grid && draft >= PROPR))
		return false;
	const Common::String key = Common::String::format("%cfont_%d",
		grid ? 'g' : 't', style);
	return commitString(key, Screen::getFontName(draft));
}

bool GlkOptionsState::resetStyleProperty(bool grid, int style,
		StyleProperty property) {
	if (style < 0 || style >= style_NUMSTYLES)
		return false;
	const char prefix = grid ? 'g' : 't';
	if (property == kStyleFont)
		return removePreference(Common::String::format("%cfont_%d",
			prefix, style));
	if (property != kStyleForeground && property != kStyleBackground)
		return false;

	const Common::String key = Common::String::format("%ccolor_%d",
		prefix, style);
	const Common::String marker = key +
		(property == kStyleForeground ? "_fg_override" : "_bg_override");
	_editedPreferences.erase(marker);
	const StyleProperty otherProperty = property == kStyleForeground ?
		kStyleBackground : kStyleForeground;
	if (!isStylePropertyOverridden(grid, style, property))
		return false;

	if (isStylePropertyOverridden(grid, style, otherProperty)) {
		_draftPreferences.setVal(marker, "false");
	} else {
		_draftPreferences.erase(key);
		_draftPreferences.erase(key + "_fg_override");
		_draftPreferences.erase(key + "_bg_override");
	}
	return true;
}

bool GlkOptionsState::removePreference(const Common::String &key) {
	_editedPreferences.erase(key);
	if (!_draftPreferences.contains(key))
		return false;
	_draftPreferences.erase(key);
	return true;
}

bool GlkOptionsState::resetAllPreferences(bool interpreterDefaults) {
	_editedPreferences.clear();
	Common::Array<Common::String> keys;
	for (Common::ConfigManager::Domain::const_iterator it =
			_draftPreferences.begin(); it != _draftPreferences.end(); ++it) {
		if (findGlkOptionContract(it->_key) || it->_key == "glk_ignore_global_appearance")
			keys.push_back(it->_key);
	}

	for (uint index = 0; index < keys.size(); ++index)
		_draftPreferences.erase(keys[index]);
	if (interpreterDefaults)
		_draftPreferences.setVal("glk_ignore_global_appearance", "true");
	return interpreterDefaults || !keys.empty();
}

} // End of namespace Glk
