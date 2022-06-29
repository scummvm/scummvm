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

#ifndef BLADERUNNER_DETECTION_TABLES_H
#define BLADERUNNER_DETECTION_TABLES_H

#include "common/translation.h"
#include "engines/advancedDetector.h"

#define GAMEOPTION_SITCOM                     GUIO_GAMEOPTIONS1
#define GAMEOPTION_SHORTY                     GUIO_GAMEOPTIONS2
#define GAMEOPTION_FRAMELIMITER_NODELAYMILLIS GUIO_GAMEOPTIONS3
#define GAMEOPTION_FRAMELIMITER_FPS           GUIO_GAMEOPTIONS4
#define GAMEOPTION_DISABLE_STAMINA_DRAIN      GUIO_GAMEOPTIONS5
#define GAMEOPTION_SHOW_SUBS_IN_CRAWL         GUIO_GAMEOPTIONS6

namespace BladeRunner {

static const ADGameDescription gameDescriptions[] = {
	// BladeRunner (English) - ENG
	{
		"bladerunner",
		"",
		AD_ENTRY1s("STARTUP.MIX", "5643b53306ca7764cf1ec7b79c9630a3", 2312374),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (German) - DEU
	{
		"bladerunner",
		"",
		AD_ENTRY1s("STARTUP.MIX", "57d674ed860148a530b7f4957cbe65ec", 2314301),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (French) - FRA - Bug #9722
	{
		"bladerunner",
		"",
		AD_ENTRY1s("STARTUP.MIX", "39d1901df50935d58aee252707134952", 2314526),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Italian) - ITA
	{
		"bladerunner",
		"",
		AD_ENTRY1s("STARTUP.MIX", "c7ceb9c691223d25e78516aa519ff504", 2314461),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Russian - Fargus Multimedia + Home Systems, Inc.) - RUS
	{
		"bladerunner",
		"",
		AD_ENTRY1s("STARTUP.MIX", "bf42af841d9f4b643665013a348c81e0", 2483111),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Russian - Fargus Multimedia + Home Systems, Inc. + Siberian Studio, R3) - RUS
	{
		"bladerunner",
		_s("The fan translator does not wish his translation to be incorporated into ScummVM."),
		AD_ENTRY1s("STARTUP.MIX", "c198b54a5366b88b1734bbca21d3b192", 2678672),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Russian - Fargus Multimedia + Home Systems, Inc. + Siberian Studio, R4) - RUS
	{
		"bladerunner",
		_s("The fan translator does not wish his translation to be incorporated into ScummVM."),
		AD_ENTRY1s("STARTUP.MIX", "d62498a7415682bb3ff86a894303c836", 2810053),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Spanish) - ESP
	{
		"bladerunner",
		"",
		AD_ENTRY1s("STARTUP.MIX", "54cad53da9e4ae03a85648834ac6765d", 2312976),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Chinese)
	{
		"bladerunner",
		_s("Missing game code"), // Reason for being unsupported
		AD_ENTRY2s("STARTUP.MIX", "c198b54a5366b88b1734bbca21d3b192", 2678672,
				   "95blade.dll", "187f257c3183d6b0a0aee69e5cde4c76", 307200),
		Common::ZH_CHN,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner - Enhanced edition
	{
		"bladerunner-ee",
		_s("Enhanced edition not supported, use the original files which are part of the release."),
		AD_ENTRY1s("BladeRunner.kpf", "c42766a9337c828f0d98383f72636fb3", 1163268364),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO6(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GUIO_NOMIDI)
	},

	// Versions with restored content

	// BladeRunner (English) - ENG
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "5643b53306ca7764cf1ec7b79c9630a3", 2312374),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (German) - DEU
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "57d674ed860148a530b7f4957cbe65ec", 2314301),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (French) - FRA
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "39d1901df50935d58aee252707134952", 2314526),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Italian) - ITA
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "c7ceb9c691223d25e78516aa519ff504", 2314461),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Russian - Fargus Multimedia + Home Systems, Inc.) - RUS
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "bf42af841d9f4b643665013a348c81e0", 2483111),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Russian - Fargus Multimedia + Home Systems, Inc. + Siberian Studio, R3) - RUS
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "c198b54a5366b88b1734bbca21d3b192", 2678672),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Russian - Fargus Multimedia + Home Systems, Inc. + Siberian Studio, R4) - RUS
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "d62498a7415682bb3ff86a894303c836", 2810053),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// BladeRunner (Spanish) - ESP
	{
		"bladerunner-final",
		"",
		AD_ENTRY1s("STARTUP.MIX", "54cad53da9e4ae03a85648834ac6765d", 2312976),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO7(GAMEOPTION_SITCOM, GAMEOPTION_SHORTY, GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GAMEOPTION_DISABLE_STAMINA_DRAIN, GAMEOPTION_SHOW_SUBS_IN_CRAWL, GUIO_NOMIDI)
	},

	// Demo Version(s)

	// BladeRunner (VQA/VQP Teaser) - Non-interactive demo
	{
		"bladerunner",
		"Non-Interactive Demo",
		AD_ENTRY1s("SIZZLE2.VQA", "2979892996f9f6fafb06c0cd72fc1025", 14386668),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GAMEOPTION_FRAMELIMITER_NODELAYMILLIS, GAMEOPTION_FRAMELIMITER_FPS, GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace BladeRunner

#endif
