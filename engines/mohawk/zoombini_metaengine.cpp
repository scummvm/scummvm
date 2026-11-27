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

#include "mohawk/zoombini_metaengine.h"
#include "mohawk/detection.h"
#include "mohawk/zoombini_resource.h"

#include "common/config-manager.h"
#include "common/keyboard.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/ustr.h"

Mohawk::ZmbReleaseInfo::ZmbReleaseInfo(const MohawkGameDescription &gameDesc)
	: _features(gameDesc.features), _lang(gameDesc.desc.language), _platform(gameDesc.desc.platform), _isDemo((gameDesc.desc.flags & ADGF_DEMO) != 0),
	  _hasMultipleLangResources(!_isDemo && (isJapaneseRelease() || (_lang == Common::UNK_LANG && (_features & (GF_ZMB_10_EU | GF_ZMB_11_EU)) != 0))),
	  _shortcutLayout(getShortcutLayout(gameDesc.desc.extra ? gameDesc.desc.extra : "")) {
	if (_lang == Common::UNK_LANG && !_hasMultipleLangResources)
		error("ZmbReleaseInfo: single-language release cannot have a value of UNK_LANG");
}

bool Mohawk::ZmbReleaseInfo::hasLangSwitch() const {
	// Japanese demos and French 2002 retain the commands, but does not have other language archives.
	return isJapaneseRelease() || (!_isDemo && (_hasMultipleLangResources || (_features & GF_ZMB_11_FR_2002) != 0));
}

bool Mohawk::ZmbReleaseInfo::isJapaneseRelease() const {
	return (_features & GF_ZMB_10_JP) != 0;
}

bool Mohawk::ZmbReleaseInfo::isMultilingualReleaseLang(Common::Language lang) const {
	switch (lang) {
	case Common::EN_GRB:
	case Common::FR_FRA:
	case Common::DE_DEU:
		return true;
	case Common::JA_JPN:
		return isJapaneseRelease();
	default:
		return false;
	}
}

bool Mohawk::ZmbReleaseInfo::setLanguage(Common::Language lang) {
	if (!hasMultipleLangResources() || !isMultilingualReleaseLang(lang))
		return false;
	_lang = lang;
	return true;
}

const char *Mohawk::ZmbReleaseInfo::getArchiveRoot(Common::Language lang) const {
	if (!isFamily(Family::kEuropeV1x))
		return ZMB_MHK_ROOT_GENERIC;
	switch (lang) {
	case Common::EN_GRB:
		return ZMB_MHK_ROOT_LANG_GB;
	case Common::FR_FRA:
		return ZMB_MHK_ROOT_LANG_FR;
	case Common::DE_DEU:
		return ZMB_MHK_ROOT_LANG_GE;
	case Common::IT_ITA:
		return ZMB_MHK_ROOT_LANG_IT;
	case Common::JA_JPN:
		return ZMB_MHK_ROOT_LANG_JP;
	default:
		error("ZmbReleaseInfo: unsupported language for v1.x Europe variant: %s", Common::getLanguageLocale(lang));
	}
}

Common::CodePage Mohawk::ZmbReleaseInfo::getExeCodePage(Common::Language lang, Common::Platform platform) {
	switch (platform) {
	case Common::kPlatformMacintosh:
		switch (lang) {
		case Common::JA_JPN:
			// FIXME: Implement proper MacJapanese support.
			// Fortunately, the v1.0JP demo's Japanese text decodes correctly with CP932,
			// as CP932 and MacJapanese are both based on Shift-JIS, but with minor different extensions.
			return Common::kWindows932;
		default:
			return Common::kMacRoman;
		}
	case Common::kPlatformWindows:
		switch (lang) {
		case Common::JA_JPN:
			return Common::kWindows932;
		case Common::KO_KOR:
			return Common::kWindows949;
		case Common::PL_POL:
			return Common::kWindows1250;
		default:
			return Common::kWindows1252;
		}
	default:
		error("ZmbReleaseInfo::getExeCodePage: unsupported platform %d", static_cast<int>(platform));
		break;
	}
	return Common::kCodePageInvalid;
}

Common::CodePage Mohawk::ZmbReleaseInfo::getResCodePage(Common::Language lang, Common::Platform platform) {
	switch (platform) {
	case Common::kPlatformMacintosh:
		return getExeCodePage(lang, platform);
	case Common::kPlatformWindows:
		switch (lang) {
		case Common::EN_GRB:
		case Common::FR_FRA:
		case Common::DE_DEU:
		case Common::IT_ITA:
			// European TEXTSTR.MHK strings are encoded as MacRoman.
			return Common::kMacRoman;
		case Common::JA_JPN:
			return Common::kWindows932;
		case Common::KO_KOR:
			return Common::kWindows949;
		case Common::PL_POL:
			return Common::kWindows1250;
		case Common::ES_ESP:
		case Common::PT_PRT:
		case Common::EN_USA:
		default:
			return Common::kWindows1252;
		}
	default:
		error("ZmbReleaseInfo::getResCodePage: unsupported platform %d", static_cast<int>(platform));
		break;
	}
	return Common::kCodePageInvalid;
}

const char *Mohawk::ZmbReleaseInfo::getCodePageName(Common::CodePage codePage) {
	// FIXME: Move it to the ScummVM core?
	switch (codePage) {
	case Common::kWindows932:
		return "CP932";
	case Common::kWindows949:
		return "CP949";
	case Common::kWindows1250:
		return "CP1250";
	case Common::kWindows1252:
		return "CP1252";
	case Common::kMacRoman:
		return "MacRoman";
	default:
		return "Unknown";
	}
}

Mohawk::MohawkMetaEngine_Zoombini::MidiSoundtrack Mohawk::MohawkMetaEngine_Zoombini::getDefaultMidiSoundtrack(Common::Platform platform) {
	switch (platform) {
	case Common::kPlatformMacintosh:
		return MidiSoundtrack::kMacintosh;
	case Common::kPlatformWindows:
	default:
		return MidiSoundtrack::kWindowsMPC;
	}
}

Mohawk::MohawkMetaEngine_Zoombini::MidiSoundtrack Mohawk::MohawkMetaEngine_Zoombini::normalizeMidiSoundtrack(int soundtrack, Common::Platform platform) {
	switch (soundtrack) {
	case static_cast<int>(MidiSoundtrack::kWindowsMPC):
		return MidiSoundtrack::kWindowsMPC;
	case static_cast<int>(MidiSoundtrack::kMacintosh):
		return MidiSoundtrack::kMacintosh;
	default:
		return getDefaultMidiSoundtrack(platform);
	}
}

const char *Mohawk::MohawkMetaEngine_Zoombini::getMidiArchiveName(MidiSoundtrack soundtrack) {
	switch (soundtrack) {
	case MidiSoundtrack::kWindowsMPC:
		return ZMB_MHK_MIDIMPC;
	case MidiSoundtrack::kMacintosh:
		return ZMB_MHK_MIDIMAC;
	default:
		error("Invalid MIDI Soundtrack kind: %d", static_cast<int>(soundtrack));
	}
}

bool Mohawk::MohawkMetaEngine_Zoombini::getFixedMidiSoundtrack(Common::Platform platform, Common::Language lang, bool isDemo, bool isUsV10, MidiSoundtrack &soundtrack) {
	switch (platform) {
	case Common::kPlatformMacintosh:
		// Macintosh v1.0US retail, v1.0US Demo and v1.0JP Demo contain only MIDIMAC.MHK.
		// Macintosh v1.1US retail, v1.xEU and v1.0JP retail contain both MIDIMAC.MHK and MIDIMPC.MHK.
		if (isUsV10 || (isDemo && (lang == Common::EN_USA || lang == Common::JA_JPN))) {
			soundtrack = MidiSoundtrack::kMacintosh;
			return true;
		}
		break;
	case Common::kPlatformWindows:
		// Windows v1.0US retail, v1.0US Demo and v1.0JP Demo contain only MIDIMPC.MHK.
		if (isUsV10 || (isDemo && (lang == Common::EN_USA || lang == Common::JA_JPN))) {
			soundtrack = MidiSoundtrack::kWindowsMPC;
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

bool Mohawk::ZmbReleaseInfo::isFamily(Family family) const {
	switch (family) {
	case Family::kUsV10:
		return (_features & (GF_ZMB_10_US | GF_ZMB_10_ESPT)) != 0;
	case Family::kEuropeV1x:
		return (_features & (GF_ZMB_10_EU | GF_ZMB_11_EU | GF_ZMB_11_FR_2002 | GF_ZMB_10_JP)) != 0;
	case Family::kUsV11:
		return (_features & GF_ZMB_11_US) != 0;
	case Family::kTlcV20:
		return (_features & GF_ZMB_20_US) != 0;
	}
	return false;
}

bool Mohawk::ZmbReleaseInfo::isFamily(Family family, Common::Language lang) const {
	return isFamily(family) && _lang == lang;
}

bool Mohawk::ZmbReleaseInfo::isExtraFamily(const Common::String &extra, Family family) {
	switch (family) {
	case Family::kUsV10:
		return extra.hasPrefix("v1.0US") || extra.hasPrefix("v1.0ES") || extra.hasPrefix("v1.0PT");
	case Family::kEuropeV1x:
		return extra.hasPrefix("v1.0EU") || extra.hasPrefix("v1.1EU") ||
			   extra.hasPrefix("v1.0BR Demo") || extra.hasPrefix("v1.1FR_2002") ||
			   extra.hasPrefix("v1.0IT") || extra.hasPrefix("v1.0JP");
	case Family::kUsV11:
		return extra.hasPrefix("v1.1US") || extra.hasPrefix("v1.11KR");
	case Family::kTlcV20:
		return extra.hasPrefix("v2.0");
	}
	return false;
}

bool Mohawk::ZmbReleaseInfo::isV1() const {
	return isFamily(Family::kUsV10) || isFamily(Family::kEuropeV1x) || isFamily(Family::kUsV11);
}

Mohawk::ZmbReleaseInfo::ShortcutLayout Mohawk::ZmbReleaseInfo::getShortcutLayout(const Common::String &extra) {
	if (!isExtraFamily(extra, Family::kEuropeV1x))
		return ShortcutLayout::kUs;
	if (extra.hasPrefix("v1.0JP"))
		return ShortcutLayout::kJapanese;
	if (extra.hasPrefix("v1.0IT"))
		return ShortcutLayout::kItalian;
	return ShortcutLayout::kEurope;
}

const Common::Array<Common::KeyCode> &Mohawk::ZmbReleaseInfo::getShortcutKeys(ShortcutKind kind, ShortcutLayout layout, bool fixSwappedSoundMusic) {
	const bool isV1xEurope = layout != ShortcutLayout::kUs;
	static const Common::Array<Common::KeyCode> kNone;
	static const Common::Array<Common::KeyCode> kB = {Common::KEYCODE_b};
	static const Common::Array<Common::KeyCode> kBAndV = {Common::KEYCODE_b, Common::KEYCODE_v};
	static const Common::Array<Common::KeyCode> kD = {Common::KEYCODE_d};
	static const Common::Array<Common::KeyCode> kDAndEAndF = {Common::KEYCODE_d, Common::KEYCODE_e, Common::KEYCODE_f};
	static const Common::Array<Common::KeyCode> kDAndEAndFAndJ = {Common::KEYCODE_d, Common::KEYCODE_e, Common::KEYCODE_f, Common::KEYCODE_j};
	static const Common::Array<Common::KeyCode> kDAndEAndFAndK = {Common::KEYCODE_d, Common::KEYCODE_e, Common::KEYCODE_f, Common::KEYCODE_k};
	static const Common::Array<Common::KeyCode> kI = {Common::KEYCODE_i};
	static const Common::Array<Common::KeyCode> kJ = {Common::KEYCODE_j};
	static const Common::Array<Common::KeyCode> kK = {Common::KEYCODE_k};
	static const Common::Array<Common::KeyCode> kL = {Common::KEYCODE_l};
	static const Common::Array<Common::KeyCode> kLAndC = {Common::KEYCODE_l, Common::KEYCODE_c};
	static const Common::Array<Common::KeyCode> kS = {Common::KEYCODE_s};
	static const Common::Array<Common::KeyCode> kSAndR = {Common::KEYCODE_s, Common::KEYCODE_r};
	static const Common::Array<Common::KeyCode> kV = {Common::KEYCODE_v};
	static const Common::Array<Common::KeyCode> kVAndB = {Common::KEYCODE_v, Common::KEYCODE_b};
	static const Common::Array<Common::KeyCode> kY = {Common::KEYCODE_y};

	switch (kind) {
	case ShortcutKind::kVersionDialog:
		return isV1xEurope ? kI : kV;
	case ShortcutKind::kOptionsSound:
		if (!isV1xEurope)
			return kD;
		if (layout == ShortcutLayout::kItalian) {
			return kVAndB;
		} else if (layout == ShortcutLayout::kJapanese) {
			return kY;
		} else {
			return kBAndV;
		}
	case ShortcutKind::kOptionsMusic:
		if (isV1xEurope) {
			if (layout == ShortcutLayout::kJapanese)
				return kBAndV;
			return kY;
		}
		return kB;
	case ShortcutKind::kGameplaySound:
		// Italian gameplay uses Ctrl+V; its default keymap also maps Ctrl+B to this key.
		if (layout == ShortcutLayout::kItalian)
			return kV;
		if (layout == ShortcutLayout::kEurope && !fixSwappedSoundMusic)
			return kY;
		return getShortcutKeys(ShortcutKind::kOptionsSound, layout);
	case ShortcutKind::kGameplayMusic:
		if (layout == ShortcutLayout::kEurope && !fixSwappedSoundMusic)
			return kBAndV;
		return getShortcutKeys(ShortcutKind::kOptionsMusic, layout);
	case ShortcutKind::kStickyMouse:
		if (layout == ShortcutLayout::kJapanese)
			return kK;
		return kJ;
	case ShortcutKind::kLangSwitch:
		if (!isV1xEurope)
			return kNone;
		if (layout == ShortcutLayout::kJapanese)
			return kDAndEAndFAndJ;
		else if (layout == ShortcutLayout::kItalian)
			return kDAndEAndFAndK;
		return kDAndEAndF;
	case ShortcutKind::kLoadGame:
		return isV1xEurope ? kLAndC : kL;
	case ShortcutKind::kSaveGame:
		return isV1xEurope ? kSAndR : kS;
	}
	return kNone;
}

bool Mohawk::ZmbReleaseInfo::isShortcut(ShortcutKind kind, const Common::KeyState &kbd, bool fixSwappedSoundMusic) const {
	const ShortcutLayout layout = getShortcutLayout();

	// On some releases, version dialog can be spawned by Tab.
	if (kind == ShortcutKind::kVersionDialog && layout != ShortcutLayout::kUs && kbd.hasFlags(0) && kbd.keycode == Common::KEYCODE_TAB)
		return true;

	// The other Keycodes requires Ctrl.
	if (!kbd.hasFlags(Common::KBD_CTRL))
		return false;

	const Common::Array<Common::KeyCode> &keys = getShortcutKeys(kind, layout, fixSwappedSoundMusic);
	for (Common::KeyCode key : keys) {
		if (kbd.keycode == key)
			return true;
	}
	return false;
}

bool Mohawk::ZmbReleaseInfo::isDemo(Family family) const {
	return isDemo() && isFamily(family);
}

bool Mohawk::ZmbReleaseInfo::isDemo(Family family, Common::Language lang) const {
	return isDemo() && isFamily(family, lang);
}

bool Mohawk::ZmbReleaseInfo::hasDemoPracticeMap() const {
	return isDemo(Family::kUsV10) || isDemo(Family::kEuropeV1x, Common::JA_JPN);
}

Mohawk::ZmbReleaseInfo::MapArchiveKind Mohawk::ZmbReleaseInfo::getMapArchiveKind() const {
	if (isFamily(Family::kUsV10) || isFamily(Family::kEuropeV1x))
		return MapArchiveKind::kMap;
	return MapArchiveKind::kRodMap;
}

const char *Mohawk::ZmbReleaseInfo::getMacApplicationName() const {
	if (isFamily(Family::kTlcV20))
		return "Zoombinis I V2";
	if (isDemo(Family::kUsV10))
		return "Zoom";
	if (isFamily(Family::kUsV11))
		return "Zoombinis";
	return "Zoombini";
}

bool Mohawk::ZmbReleaseInfo::supportsLocalizedNames() const {
	return isFamily(Family::kEuropeV1x, Common::JA_JPN) || isFamily(Family::kUsV11, Common::KO_KOR);
}

Common::U32String Mohawk::ZmbReleaseInfo::formatMemorialDate(uint16 year, const Common::U32String &monthText, byte day) const {
	if (isFamily(Family::kEuropeV1x) && _lang != Common::JA_JPN) {
		// v1.0BR/FR/GE, v1.1BR/FR/GE, v1.1FR_2002 and v1.0IT: Day month year
		return Common::U32String::format("%d %S %d", day, monthText.c_str(), year);
	} else if (isFamily(Family::kEuropeV1x, Common::JA_JPN)) {
		// v1.0JP: Year-NEN month day-NICHI (with the Japenese year/day suffix.)
		return Common::U32String::format(Common::U32String(U"%d年 %S %d日"), year, monthText.c_str(), day);
	} else if (_lang == Common::KO_KOR) {
		// v1.1KR: month day-IL, year (with the Korean day suffix.)
		return Common::U32String::format(Common::U32String(U"%S %d일, %d"), monthText.c_str(), day, year);
	} else {
		// v1.0ES/PT, v1.1US, and v2.0US/PL: month day, year
		return Common::U32String::format("%S %d, %d", monthText.c_str(), day, year);
	}
}

Common::Rect Mohawk::ZmbReleaseInfo::LayoutAdjustment::apply(const Common::Rect &rect) const {
	return Common::Rect(rect.left + leftOffset, rect.top + topOffset, rect.right + rightOffset, rect.bottom + bottomOffset);
}

Mohawk::ZmbReleaseInfo::LayoutAdjustment Mohawk::ZmbReleaseInfo::getLayoutAdjustment(LayoutKind kind) const {
	LayoutAdjustment adjustment;
	switch (kind) {
	case LayoutKind::kCreditsText:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.leftOffset = 0x009B - 0x00BE;
			adjustment.rightOffset = 0x01E5 - 0x01C2;
		} else if (isFamily(Family::kTlcV20)) {
			adjustment.leftOffset = 0x0064 - 0x00BE;
			adjustment.rightOffset = 0x021C - 0x01C2;
		}
		if (_platform == Common::kPlatformMacintosh) {
			adjustment.leftOffset += 8;
			adjustment.rightOffset -= 8;
			if ((_features & GF_ZMB_11_EU) != 0) {
				adjustment.leftOffset -= 5;
				adjustment.rightOffset += 5;
			}
		}
		break;
	case LayoutKind::kCreditsScroll:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.leftOffset = 0x00AA - 0x00BE;
			adjustment.rightOffset = 0x01D6 - 0x01C2;
		} else if (isFamily(Family::kTlcV20)) {
			adjustment.leftOffset = 0x0064 - 0x00BE;
			adjustment.rightOffset = 0x021C - 0x01C2;
		}
		if (_platform == Common::kPlatformMacintosh) {
			adjustment.leftOffset += 8;
			adjustment.rightOffset -= 8;
		}
		break;
	case LayoutKind::kCreditsV20MilesLogo:
		if (_platform == Common::kPlatformMacintosh)
			adjustment.leftOffset = 1;
		break;
	case LayoutKind::kMsgBoxBody:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.topOffset = 0x0082 - 0x0091;
			adjustment.bottomOffset = 0x00EB - 0x00DC;
		}
		break;
	case LayoutKind::kOptionsTitle:
		if (isFamily(Family::kTlcV20)) {
			adjustment.leftOffset = 0x00B9 - 0x00FA;
			adjustment.rightOffset = 0x01BD - 0x017C;
		}
		break;
	case LayoutKind::kSaveLoadTitle:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.leftOffset = 0x00C8 - 0x00FA;
			adjustment.rightOffset = 0x01C7 - 0x0198;
		}
		break;
	case LayoutKind::kSaveNameInput:
		if (isFamily(Family::kEuropeV1x, Common::JA_JPN)) {
			if (_platform == Common::kPlatformMacintosh)
				adjustment.topOffset = -1;
			else
				adjustment.topOffset = 0x012C - 0x012A;
			adjustment.bottomOffset = 0x013E - 0x013C;
		}
		break;
	case LayoutKind::kMapLegendClick:
		if (isFamily(Family::kEuropeV1x))
			adjustment.leftOffset = 444 - 464;
		break;
	case LayoutKind::kMapLegendTitle:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.topOffset = -3 - 3;
			adjustment.bottomOffset = (-3 + 38) - (3 + 18);
		} else if (isFamily(Family::kUsV10, Common::PT_PRT)) {
			adjustment.topOffset = 2 - 3;
			adjustment.bottomOffset = (2 + 23) - (3 + 18);
		}
		break;
	case LayoutKind::kMapLegendRow:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.topOffset = 33 - 22;
			adjustment.bottomOffset = adjustment.topOffset;
			adjustment.rightOffset = 25;
		}
		break;
	case LayoutKind::kMapStatsTitle:
		if (isFamily(Family::kUsV10, Common::PT_PRT)) {
			adjustment.topOffset = 2 - 3;
			adjustment.bottomOffset = (2 + 23) - (3 + 18);
		}
		break;
	case LayoutKind::kMapStatsRow:
		if (isFamily(Family::kUsV10, Common::PT_PRT)) {
			adjustment.topOffset = 20 - 26;
			adjustment.bottomOffset = (20 + 23) - (26 + 18);
		}
		break;
	case LayoutKind::kMapStatsLabel:
		if (isFamily(Family::kUsV10, Common::PT_PRT))
			adjustment.rightOffset = 120 - 115;
		break;
	case LayoutKind::kMapPracticeRowStep:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.bottomOffset = 38 - 18;
		} else if (isFamily(Family::kUsV10, Common::PT_PRT)) {
			adjustment.topOffset = 23 - 18;
			adjustment.bottomOffset = adjustment.topOffset;
		}
		break;
	case LayoutKind::kMapTooltip:
		if (isFamily(Family::kUsV11, Common::KO_KOR)) {
			adjustment.topOffset = 2;
			adjustment.bottomOffset = adjustment.topOffset;
		}
		break;
	case LayoutKind::kMapOptionsLabel:
		if (isFamily(Family::kTlcV20) && _platform == Common::kPlatformMacintosh)
			adjustment.leftOffset = 3;
		break;
	case LayoutKind::kPickerName:
		if (!isFamily(Family::kUsV11, Common::KO_KOR))
			adjustment.topOffset = 1 - 4;
		break;
	case LayoutKind::kPickerGenerateButton:
		if (isFamily(Family::kTlcV20))
			adjustment.bottomOffset = 1;
		break;
	case LayoutKind::kPickerDiceButton:
		if (isFamily(Family::kTlcV20))
			adjustment.topOffset = -1;
		break;
	case LayoutKind::kTownMemorialBody:
		if (_platform == Common::kPlatformMacintosh && isFamily(Family::kEuropeV1x, Common::JA_JPN))
			adjustment.bottomOffset = 46 - 36;
		break;
	case LayoutKind::kBasecamp2ScrollLeftMaxButton:
		if (isFamily(Family::kTlcV20))
			adjustment.bottomOffset = 211 - 208;
		break;
	case LayoutKind::kBasecamp2ScrollLeftOneButton:
		if (isFamily(Family::kTlcV20))
			adjustment.bottomOffset = 207 - 205;
		break;
	case LayoutKind::kXferTownPopulationText:
		if (isFamily(Family::kEuropeV1x)) {
			adjustment.leftOffset = 10;
			adjustment.topOffset = 4;
		} else {
			adjustment.leftOffset = 16;
			adjustment.topOffset = 8;
		}
		break;
	case LayoutKind::kNotiBoxText:
		if (_lang == Common::KO_KOR)
			adjustment.topOffset = 3;
		break;
	case LayoutKind::kHelpHeading:
		if (isFamily(Family::kTlcV20)) {
			adjustment.topOffset = 0x0035 - 0x003E;
			adjustment.bottomOffset = adjustment.topOffset;
		}
		break;
	case LayoutKind::kHelpBody:
		if (isFamily(Family::kTlcV20))
			adjustment.topOffset = 0x0070 - 0x0079;
		break;
	}
	return adjustment;
}

Mohawk::ZmbReleaseInfo::LayoutAdjustment Mohawk::ZmbReleaseInfo::getMapRouteLayoutAdjustment(uint32 routeIndex) const {
	LayoutAdjustment adjustment;
	if (isFamily(Family::kEuropeV1x)) {
		// v1.0BR/FR/GE/IT/JP, v1.1BR/FR/GE, v1.1FR_2002 and v1.0BR Demo use the wider route rectangles.
		switch (routeIndex) {
		case 0: // Route 1: The big, the bad and the hungry
			adjustment.leftOffset = 10 - 15;
			adjustment.topOffset = 150 - 247;
			adjustment.rightOffset = 170 - 140;
			break;
		case 2: // Route 3: Deep, Dark Forest
			adjustment.topOffset = 390 - 414;
			adjustment.bottomOffset = 480 - 444;
			break;
		case 3: // Route 4: Mountains of Despair
			adjustment.leftOffset = 490 - 507;
			adjustment.topOffset = 195 - 193;
			adjustment.rightOffset = 610 - 587;
			adjustment.bottomOffset = 253 - 223;
			break;
		default:
			break;
		}
	}
	return adjustment;
}

Mohawk::ZmbReleaseInfo::LayoutAdjustment Mohawk::ZmbReleaseInfo::getOptionsLabelLayoutAdjustment(uint32 buttonIndex) const {
	LayoutAdjustment adjustment;
	if (!isFamily(Family::kTlcV20))
		return adjustment;

	// v2.0TLC family needs adjustments.
	static constexpr int16 kTopOffsets[] = {
		0x005A - 0x0066,
		0x006F - 0x007C,
		0x0084 - 0x0091,
		0x0099 - 0x00A6,
		0x00BE - 0x00DD,
		0x00D3 - 0x00F3,
		0x00E8 - 0x0108,
		0x00FD - 0x011D,
	};
	if (buttonIndex < ARRAYSIZE(kTopOffsets)) {
		adjustment.topOffset = kTopOffsets[buttonIndex];
		adjustment.bottomOffset = adjustment.topOffset;
	}
	return adjustment;
}

const char *Mohawk::ZmbReleaseInfo::getFamilyName() const {
	if (isFamily(Family::kUsV10))
		return "USv1.0";
	if (isFamily(Family::kEuropeV1x))
		return "EUv1.x";
	if (isFamily(Family::kUsV11))
		return "USv1.1";
	if (isFamily(Family::kTlcV20))
		return "TLCv2.0";
	return "unknown";
}

constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionConfirm;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleDialogAndSfx;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleMusic;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleStickyMouse;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleAutoStickyMouse;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleTransitions;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleLessMoreAction;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleCursor;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleJapaneseNames;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionReplayPuzzleInstructions;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleHelpDialogNarration;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionToggleTouchSense;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionPracticeMode;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionNewGame;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionRemoveSave;
constexpr const char *Mohawk::MohawkMetaEngine_Zoombini::kActionQuit;

Mohawk::MohawkMetaEngine_Zoombini::TargetFrameRate Mohawk::MohawkMetaEngine_Zoombini::normalizeTargetFrameRate(int rate) {
	switch (rate) {
	case static_cast<int>(TargetFrameRate::k60FPS):
	case static_cast<int>(TargetFrameRate::k120FPS):
	case static_cast<int>(TargetFrameRate::k180FPS):
	case static_cast<int>(TargetFrameRate::k240FPS):
		return static_cast<TargetFrameRate>(rate);
	default:
		return TargetFrameRate::k60FPS;
	}
}

Mohawk::MohawkMetaEngine_Zoombini::MacFontSmoothing Mohawk::MohawkMetaEngine_Zoombini::normalizeFontSmoothing(int smoothing) {
	switch (static_cast<MacFontSmoothing>(smoothing)) {
	case MacFontSmoothing::kEnabled:
		return MacFontSmoothing::kEnabled;
	case MacFontSmoothing::kDisabled:
	default:
		return MacFontSmoothing::kDisabled;
	}
}

void Mohawk::MohawkMetaEngine_Zoombini::registerDefaultSettings(Common::Platform platform) {
	// Bug fixes.
	ConfMan.registerDefault(kOptionFixAudioPops, true);
	ConfMan.registerDefault(kOptionFixSwappedSoundMusicShortcuts, true);
	ConfMan.registerDefault(kOptionFixFleensTreeDescendFeetBug, true);
	ConfMan.registerDefault(kOptionFixHotelMidiHaltBug, true);
	ConfMan.registerDefault(kOptionFixCavesL4MidiSilentBug, true);
	// Gameplay improvements.
	ConfMan.registerDefault(kOptionTickRate, static_cast<int>(TickRate::kAccurate60FPS));
	ConfMan.registerDefault(kOptionPositionInterpolation, false);
	ConfMan.registerDefault(kOptionTargetFrameRate, static_cast<int>(TargetFrameRate::k60FPS));
	ConfMan.registerDefault(kOptionEnhancedKbdShortcuts, true);
	ConfMan.registerDefault(kOptionShowRemappedOptionDialogShortcuts, true);
	// Gameplay tuning.
	ConfMan.registerDefault(kOptionPaletteFilter, static_cast<int>(PaletteFilter::kBrightenPalette));
	ConfMan.registerDefault(kOptionMacFontSmoothing, static_cast<int>(MacFontSmoothing::kDisabled));
	ConfMan.registerDefault(kOptionPrngAlgorithm, static_cast<int>(PrngAlgorithm::kOriginalPrng));
	ConfMan.registerDefault(kOptionMidiSoundtrack, static_cast<int>(getDefaultMidiSoundtrack(platform)));
	ConfMan.registerDefault(kOptionColorBlindMode, false);
	ConfMan.registerDefault(kOptionMazeAlwaysPlayCelebrationSfx, false);
	ConfMan.registerDefault(kOptionTownAlwaysPlayMemorialSfx, true);
	ConfMan.registerDefault(kOptionMazeRestoreUnusedL4Layout, false);
	ConfMan.registerDefault(kOptionMazeRandomizeInitialLayout, false);
	ConfMan.registerDefault(kOptionFerryHighlightTraitMatch, false);
}
