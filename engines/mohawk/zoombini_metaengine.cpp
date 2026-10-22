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

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/ustr.h"

Mohawk::ZmbReleaseInfo::ZmbReleaseInfo(const MohawkGameDescription &gameDesc)
	: _features(gameDesc.features),
	  _lang(gameDesc.desc.language), _platform(gameDesc.desc.platform),
	  _demo((gameDesc.desc.flags & ADGF_DEMO) != 0) {
	if (_lang == Common::UNK_LANG)
		_lang = Common::EN_USA;
}

Common::CodePage Mohawk::ZmbReleaseInfo::getExeCodePage(Common::Language language, Common::Platform platform) {
	switch (platform) {
	case Common::kPlatformMacintosh:
		switch (language) {
		case Common::JA_JPN:
			// FIXME: Implement proper MacJapanese support.
			// Fortunately, the v1.0JP demo's Japanese text decodes correctly with CP932,
			// as CP932 and MacJapanese are both based on Shift-JIS, but with minor different extensions.
			return Common::kWindows932;
		default:
			return Common::kMacRoman;
		}
	case Common::kPlatformWindows:
		switch (language) {
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

Common::CodePage Mohawk::ZmbReleaseInfo::getResCodePage(Common::Language language, Common::Platform platform) {
	switch (platform) {
	case Common::kPlatformMacintosh:
		return getExeCodePage(language, platform);
	case Common::kPlatformWindows:
		switch (language) {
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

bool Mohawk::MohawkMetaEngine_Zoombini::getFixedDemoMidiSoundtrack(Common::Platform platform, Common::Language language, MidiSoundtrack &soundtrack) {
	switch (platform) {
	case Common::kPlatformMacintosh:
		if (language == Common::EN_USA || language == Common::JA_JPN) { // v1.1US Demo, v1.0JP Demo
			soundtrack = MidiSoundtrack::kMacintosh;
			return true;
		}
		break;
	case Common::kPlatformWindows:
		if (language == Common::EN_USA) { // v1.1US Demo
			soundtrack = MidiSoundtrack::kWindowsMPC;
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

bool Mohawk::ZmbReleaseInfo::isVariant(Variant variant) const {
	switch (variant) {
	case kEuropeV10:
		return (_features & GF_ZMB_10_EU) != 0;
	case kEuropeV11:
		return (_features & GF_ZMB_11_EU) != 0;
	case kFranceV11_2002:
		return (_features & GF_ZMB_11_FR_2002) != 0;
	case kEsPtV10:
		return (_features & GF_ZMB_10_ESPT) != 0;
	case kJapanV10:
		return (_features & GF_ZMB_10_JP) != 0;
	case kUsV11:
		return (_features & GF_ZMB_11_US) != 0;
	case kTlcV20:
		return (_features & GF_ZMB_20_US) != 0;
	}
	return false;
}

bool Mohawk::ZmbReleaseInfo::isVariant(Variant variant, Common::Language lang) const {
	return isVariant(variant) && _lang == lang;
}

bool Mohawk::ZmbReleaseInfo::isFamily(Family family) const {
	switch (family) {
	case kEuropeV1:
		return isVariant(kEuropeV10) || isVariant(kEuropeV11) || isVariant(kFranceV11_2002) || isVariant(kEsPtV10) || isVariant(kJapanV10);
	case kUsV1:
		return isVariant(kUsV11);
	case kTlcV2:
		return isVariant(kTlcV20);
	}
	return false;
}

bool Mohawk::ZmbReleaseInfo::isV1() const {
	return isFamily(kEuropeV1) || isFamily(kUsV1);
}

bool Mohawk::ZmbReleaseInfo::isDemo(Variant variant) const {
	return isDemo() && isVariant(variant);
}

bool Mohawk::ZmbReleaseInfo::hasDemoPracticeMap() const {
	return isDemo(kUsV11) || isDemo(kJapanV10);
}

bool Mohawk::ZmbReleaseInfo::supportsLocalizedNames() const {
	return isVariant(kJapanV10) || isVariant(kUsV11, Common::KO_KOR);
}

Common::U32String Mohawk::ZmbReleaseInfo::formatMemorialDate(uint16 year, const Common::U32String &monthText, byte day) const {
	if (isVariant(kEuropeV10) || isVariant(kEuropeV11) || isVariant(kFranceV11_2002)) {
		// v1.0BR/FR/GE, v1.1BR/FR/GE, v1.1FR_2002 and v1.0IT: Day month year
		return Common::U32String::format("%d %S %d", day, monthText.c_str(), year);
	} else if (isVariant(kJapanV10)) {
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

Mohawk::ZmbReleaseInfo::LayoutAdjustment Mohawk::ZmbReleaseInfo::getLayoutAdjustment(LayoutElement element) const {
	LayoutAdjustment adjustment;
	switch (element) {
	case kCreditsTextLayout:
		if (isFamily(kEuropeV1) && !isVariant(kEsPtV10)) {
			adjustment.leftOffset = 0x009B - 0x00BE;
			adjustment.rightOffset = 0x01E5 - 0x01C2;
		} else if (isFamily(kTlcV2)) {
			adjustment.leftOffset = 0x0064 - 0x00BE;
			adjustment.rightOffset = 0x021C - 0x01C2;
		}
		break;
	case kCreditsScrollLayout:
		if (isFamily(kEuropeV1) && !isVariant(kEsPtV10)) {
			adjustment.leftOffset = 0x00AA - 0x00BE;
			adjustment.rightOffset = 0x01D6 - 0x01C2;
		} else if (isFamily(kTlcV2)) {
			adjustment.leftOffset = 0x0064 - 0x00BE;
			adjustment.rightOffset = 0x021C - 0x01C2;
		}
		break;
	case kMsgBoxBodyLayout:
		if (isVariant(kEuropeV10, Common::IT_ITA) || isVariant(kJapanV10)) {
			adjustment.topOffset = 0x0082 - 0x0091;
			adjustment.bottomOffset = 0x00EB - 0x00DC;
		}
		break;
	case kOptionsTitleLayout:
		if (isFamily(kTlcV2)) {
			adjustment.leftOffset = 0x00B9 - 0x00FA;
			adjustment.rightOffset = 0x01BD - 0x017C;
		}
		break;
	case kSaveLoadTitleLayout:
		if (isVariant(kJapanV10)) {
			adjustment.leftOffset = 0x00C8 - 0x00FA;
			adjustment.rightOffset = 0x01C7 - 0x0198;
		}
		break;
	case kSaveNameInputLayout:
		if (isVariant(kJapanV10)) {
			adjustment.topOffset = 0x012C - 0x012A;
			adjustment.bottomOffset = 0x013E - 0x013C;
		}
		break;
	case kMapLegendClickLayout:
		if (isFamily(kEuropeV1) || isDemo(kUsV11)) {
			adjustment.leftOffset = 0x01C1 - 0x01D0;
			adjustment.rightOffset = 0x0255 - 0x0264;
		}
		break;
	case kMapLegendTitleLayout:
		if (isFamily(kEuropeV1) && !isVariant(kEsPtV10)) {
			adjustment.topOffset = -3 - 3;
			adjustment.bottomOffset = (-3 + 38) - (3 + 15);
		} else if (isVariant(kEsPtV10, Common::ES_ESP)) {
			adjustment.bottomOffset = 18 - 15;
		} else if (isVariant(kEsPtV10, Common::PT_PRT)) {
			adjustment.topOffset = 2 - 3;
			adjustment.bottomOffset = (2 + 23) - (3 + 15);
		} else if (isDemo(kUsV11) || isVariant(kTlcV20, Common::PL_POL)) {
			adjustment.bottomOffset = 18 - 15;
		}
		break;
	case kMapLegendRowLayout:
		if (isFamily(kEuropeV1) && !isVariant(kEsPtV10)) {
			adjustment.topOffset = 33 - 22;
			adjustment.bottomOffset = adjustment.topOffset;
			adjustment.rightOffset = 25;
		}
		break;
	case kMapStatsTitleLayout:
		if (isFamily(kEuropeV1)) {
			adjustment.bottomOffset = 18 - 15;
			if (isVariant(kEsPtV10, Common::PT_PRT)) {
				adjustment.topOffset = 2 - 3;
				adjustment.bottomOffset = (2 + 23) - (3 + 15);
			}
		} else if (isDemo(kUsV11) || isVariant(kTlcV20, Common::PL_POL)) {
			adjustment.bottomOffset = 18 - 15;
		}
		break;
	case kMapStatsRowLayout:
		if (isVariant(kEsPtV10, Common::PT_PRT)) {
			adjustment.topOffset = 20 - 26;
			adjustment.bottomOffset = (20 + 23) - (26 + 18);
		}
		break;
	case kMapStatsLabelLayout:
		if (isVariant(kEsPtV10, Common::PT_PRT))
			adjustment.rightOffset = 120 - 115;
		break;
	case kMapPracticeRowStepLayout:
		if (isFamily(kEuropeV1) && !isVariant(kEsPtV10)) {
			adjustment.bottomOffset = 38 - 18;
		} else if (isVariant(kEsPtV10, Common::PT_PRT)) {
			adjustment.topOffset = 23 - 18;
			adjustment.bottomOffset = adjustment.topOffset;
		}
		break;
	case kMapTooltipLayout:
		if (isVariant(kUsV11, Common::KO_KOR)) {
			adjustment.topOffset = 2;
			adjustment.bottomOffset = adjustment.topOffset;
		}
		break;
	case kPickerNameLayout:
		if (isVariant(kEsPtV10, Common::PT_PRT) || isVariant(kJapanV10) || isVariant(kTlcV20, Common::PL_POL))
			adjustment.topOffset = 1 - 4;
		break;
	case kNotiBoxLongLayout:
		if (isVariant(kJapanV10)) {
			adjustment.leftOffset = 0x00ED - 0x0101;
			adjustment.rightOffset = 0x01B5 - 0x0183;
		}
		break;
	case kNotiBoxTextLayout:
		if (_lang == Common::KO_KOR)
			adjustment.topOffset = 3;
		break;
	case kHelpHeadingLayout:
		if (isFamily(kTlcV2)) {
			adjustment.topOffset = 0x0035 - 0x003E;
			adjustment.bottomOffset = adjustment.topOffset;
		}
		break;
	case kHelpBodyLayout:
		if (isFamily(kTlcV2))
			adjustment.topOffset = 0x0070 - 0x0079;
		break;
	case kHelpButtonTextLayout:
		if (!isFamily(kTlcV2)) {
			adjustment.topOffset = 3;
			adjustment.bottomOffset = adjustment.topOffset;
		}
		break;
	}
	return adjustment;
}

Mohawk::ZmbReleaseInfo::LayoutAdjustment Mohawk::ZmbReleaseInfo::getMapRouteLayoutAdjustment(uint32 routeIndex) const {
	LayoutAdjustment adjustment;
	if (isFamily(kEuropeV1) && !isVariant(kEsPtV10)) {
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
	if (!isFamily(kTlcV2))
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
	if (isFamily(kEuropeV1))
		return "EUv1";
	if (isFamily(kUsV1))
		return "USv1";
	if (isFamily(kTlcV2))
		return "TLCv2";
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

void Mohawk::MohawkMetaEngine_Zoombini::registerDefaultSettings() {
	// Bug fixes.
	ConfMan.registerDefault(kOptionFixAudioPops, true);
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
	ConfMan.registerDefault(kOptionPrngAlgorithm, static_cast<int>(PrngAlgorithm::kOriginalPrng));
	ConfMan.registerDefault(kOptionMidiSoundtrack, static_cast<int>(MidiSoundtrack::kWindowsMPC));
	ConfMan.registerDefault(kOptionColorBlindMode, false);
	ConfMan.registerDefault(kOptionMazeAlwaysPlayCelebrationSfx, false);
	ConfMan.registerDefault(kOptionTownAlwaysPlayMemorialSfx, true);
	ConfMan.registerDefault(kOptionMazeRestoreUnusedL4Layout, false);
	ConfMan.registerDefault(kOptionMazeRandomizeInitialLayout, false);
	ConfMan.registerDefault(kOptionFerryHighlightTraitMatch, false);
}
