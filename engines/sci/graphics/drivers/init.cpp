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


#include "sci/graphics/drivers/gfxdriver.h"
#include "sci/sci.h"
#include "common/config-manager.h"
#include "common/language.h"
#include "common/platform.h"

namespace Sci {

#define SCI_GFXDRV_DCL1(name) \
	extern GfxDriver *name##Driver_create(int, ...)

#define SCI_GFXDRV_DCL2(name) \
	extern bool name##Driver_validateMode(Common::Platform); \
	extern GfxDriver *name##Driver_create(int, ...)

SCI_GFXDRV_DCL1(GfxDefault);
SCI_GFXDRV_DCL1(UpscaledGfx);
SCI_GFXDRV_DCL1(PC98Gfx16Colors);
SCI_GFXDRV_DCL1(WindowsGfx256Colors);
SCI_GFXDRV_DCL2(SCI1_EGA);
SCI_GFXDRV_DCL2(SCI1_VGAGreyScale);
SCI_GFXDRV_DCL2(SCI0_CGA);
SCI_GFXDRV_DCL2(SCI0_CGABW);
SCI_GFXDRV_DCL2(SCI0_Hercules);
SCI_GFXDRV_DCL2(SCI0_PC98Gfx8Colors);
SCI_GFXDRV_DCL2(SCI1_PC98Gfx8Colors);
SCI_GFXDRV_DCL2(WindowsGfx16Colors);

#undef SCI_GFXDRV_DCL1
#undef SCI_GFXDRV_DCL2

enum HiresSetting {
	kUnused = -1,
	kDisable = 0,
	kEnable = 1
};

struct GfxDriverInfo {
	Common::RenderMode renderMode;
	Common::Platform platform;
	SciVersion versionMin;
	SciVersion versionMax;
	SciGameId gameId;
	Common::Language language;
	HiresSetting hires;
	bool (*validateMode)(Common::Platform);
	GfxDriver *(*createDriver)(int, ...);
	int config;
};

#define INITPROCS1(x) nullptr, x##Driver_create
#define INITPROCS2(x) x##Driver_validateMode, x##Driver_create

static const GfxDriverInfo _gfxDriverInfos[] = {
	// Selected modes
	{ Common::kRenderEGA, Common::kPlatformUnknown, SCI_VERSION_1_EARLY, SCI_VERSION_1_1, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI1_EGA), 0 },
	{ Common::kRenderVGAGrey, Common::kPlatformUnknown, SCI_VERSION_1_EARLY, SCI_VERSION_1_1, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI1_VGAGreyScale), 0 },
	{ Common::kRenderCGA, Common::kPlatformUnknown, SCI_VERSION_0_EARLY, SCI_VERSION_1_EGA_ONLY, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI0_CGA), 0 },
	{ Common::kRenderCGA_BW, Common::kPlatformUnknown, SCI_VERSION_0_EARLY, SCI_VERSION_1_EGA_ONLY, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI0_CGABW), 2 },
	{ Common::kRenderHercA, Common::kPlatformUnknown, SCI_VERSION_0_EARLY, SCI_VERSION_1_EGA_ONLY, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI0_Hercules), 0 },
	{ Common::kRenderHercG, Common::kPlatformUnknown, SCI_VERSION_0_EARLY, SCI_VERSION_1_EGA_ONLY, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI0_Hercules), 1 },
	{ Common::kRenderPC98_8c, Common::kPlatformUnknown, SCI_VERSION_0_LATE, SCI_VERSION_0_LATE, GID_PQ2, Common::UNK_LANG, kUnused, INITPROCS2(SCI0_PC98Gfx8Colors), 1 },
	{ Common::kRenderPC98_8c, Common::kPlatformUnknown, SCI_VERSION_01, SCI_VERSION_01, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI0_PC98Gfx8Colors), 0 },
	{ Common::kRenderPC98_8c, Common::kPlatformUnknown, SCI_VERSION_1_LATE, SCI_VERSION_1_LATE, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(SCI1_PC98Gfx8Colors), 0 },
	{ Common::kRenderWin16c, Common::kPlatformUnknown, SCI_VERSION_1_1, SCI_VERSION_1_1, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS2(WindowsGfx16Colors), 0 },
	// Default modes
	{ Common::kRenderDefault, Common::kPlatformPC98, SCI_VERSION_0_LATE, SCI_VERSION_0_LATE, GID_PQ2, Common::UNK_LANG, kUnused, INITPROCS1(PC98Gfx16Colors), 2 },
	{ Common::kRenderDefault, Common::kPlatformPC98, SCI_VERSION_01, SCI_VERSION_01, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS1(PC98Gfx16Colors), 0 },
	{ Common::kRenderDefault, Common::kPlatformPC98, SCI_VERSION_1_EARLY, SCI_VERSION_1_LATE, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS1(PC98Gfx16Colors), 1 },
	{ Common::kRenderDefault, Common::kPlatformWindows, SCI_VERSION_1_1, SCI_VERSION_1_1, GID_ALL, Common::UNK_LANG, kDisable, INITPROCS1(WindowsGfx256Colors), 0 },
	{ Common::kRenderDefault, Common::kPlatformWindows, SCI_VERSION_1_1, SCI_VERSION_1_1, GID_KQ6, Common::UNK_LANG, kEnable, INITPROCS1(WindowsGfx256Colors), 1 },
	{ Common::kRenderDefault, Common::kPlatformDOS, SCI_VERSION_1_1, SCI_VERSION_1_1, GID_KQ6, Common::UNK_LANG, kEnable, INITPROCS1(WindowsGfx256Colors), 1 },
	{ Common::kRenderDefault, Common::kPlatformUnknown, SCI_VERSION_0_EARLY, SCI_VERSION_1_1, GID_ALL, Common::KO_KOR, kUnused, INITPROCS1(UpscaledGfx), 0 },
	{ Common::kRenderDefault, Common::kPlatformUnknown, SCI_VERSION_0_EARLY, SCI_VERSION_0_LATE, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS1(GfxDefault), 0 },
	{ Common::kRenderDefault, Common::kPlatformUnknown, SCI_VERSION_01, SCI_VERSION_1_1, GID_ALL, Common::UNK_LANG, kUnused, INITPROCS1(GfxDefault), 1 }
};

#undef INITPROCS1
#undef INITPROCS2

} // End of namespace Sci

namespace SciGfxDriver {
using namespace Sci;

Common::RenderMode getRenderMode() {
	// Check if the selected render mode is available for the game. This is quite specific for each game. Sometime it
	// is only EGA, sometimes only CGA b/w without CGA 4 colors, etc. Also set default mode if undithering is enabled.
	bool undither = ConfMan.getBool("disable_dithering");
	Common::RenderMode selectedMode = ConfMan.hasKey("render_mode") ? Common::parseRenderMode(ConfMan.get("render_mode")) : Common::kRenderDefault;
	Common::RenderMode result = selectedMode;
	Common::Language lang = g_sci->getLanguage();
	Common::Platform platform = g_sci->getPlatform();
	SciVersion version = getSciVersion();

	// No extra modes supported for the Korean fan-patched games.
	// Also set default mode if undithering is enabled for a SCI0 game and the render mode is either set to kRenderEGA
	// or kRenderPC98_16c (probably redundant nowadays, but why not make sure everything works as intended).
	if (lang == Common::KO_KOR || (undither && ((selectedMode == Common::kRenderEGA && (version <= SCI_VERSION_0_LATE || version == SCI_VERSION_1_EGA_ONLY)) || selectedMode == Common::kRenderPC98_16c)))
		result = Common::kRenderDefault;

	if (result == Common::kRenderDefault)
		return result;

	// Now we just go over the first part of the table and if we find a config for the selected render mode and check
	// if the mode is usable. Otherwise we return the default mode.
	for (const GfxDriverInfo *info = _gfxDriverInfos; info->renderMode != Common::kRenderDefault; ++info) {
		if (info->renderMode == selectedMode && info->versionMin <= version && version <= info->versionMax && (info->gameId == GID_ALL || info->gameId == g_sci->getGameId())
			&& (info->language == Common::UNK_LANG || info->language == lang) && (info->hires == kUnused || (info->hires == kEnable) == g_sci->useHiresGraphics())) {
				result = (info->validateMode == nullptr || info->validateMode(platform)) ? selectedMode : Common::kRenderDefault;
				break;
		}
	}

	return result;
}

GfxDriver *create(Common::RenderMode renderMode, int width, int height) {
	GfxDriver *result = nullptr;

	int requestRGB = (int)(ConfMan.hasKey("palette_mods") && ConfMan.getBool("palette_mods")) || (ConfMan.hasKey("rgb_rendering") && ConfMan.getBool("rgb_rendering"));
	SciVersion version = getSciVersion();
	SciGameId gameId = g_sci->getGameId();
	Common::Language lang = g_sci->getLanguage();
	Common::Platform platform = g_sci->getPlatform();
	bool hires = g_sci->useHiresGraphics();

	bool undither = ConfMan.hasKey("disable_dithering") ? ConfMan.getBool("disable_dithering") : false;
	bool winCursors = ConfMan.hasKey("windows_cursors") ? ConfMan.getBool("windows_cursors") : false;

	// If a specific render mode is requested, we try to find a driver that supports it. Otherwise, we try to find a
	// driver that supports the current platform, game and version.
	for (const GfxDriverInfo *info = _gfxDriverInfos; info < &_gfxDriverInfos[ARRAYSIZE(_gfxDriverInfos)]; ++info)  {
		if (((info->renderMode == Common::kRenderDefault && (info->platform == Common::kPlatformUnknown || info->platform == platform)) || (renderMode != Common::kRenderDefault && info->renderMode == renderMode))
			&& version >= info->versionMin && version <= info->versionMax && (info->gameId == GID_ALL || info->gameId == gameId) && (info->language == Common::UNK_LANG || info->language == lang) && (info->hires == kUnused || (info->hires == kEnable) == hires)) {
				result = info->createDriver(requestRGB, info->config, width, height, undither, winCursors, hires);
				break;
		}
	}

	return result;
}

} // End of namespace SciGfxDriver

