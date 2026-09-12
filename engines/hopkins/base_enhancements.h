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

#ifndef HOPKINS_BASE_ENHANCEMENTS_H
#define HOPKINS_BASE_ENHANCEMENTS_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class BaseData;
class BaseEngine;

static const char *const kWBASEEnhancementsConfigKey = "wbase_enhancements";
static const char *const kWBASEForcedAutoplayConfigKey = "wbase_forced_autoplay";
static const char *const kWBASEEnhancementsKeymapId = "hopkins-wbase-enhancements";

enum WBASEEnhancementPanel {
	kWBASEEnhancementPanelNone,
	kWBASEEnhancementPanelNavigationMap,
	kWBASEEnhancementPanelAutoplay
};

enum WBASEEnhancementControl {
	kWBASEEnhancementControlNone,
	kWBASEEnhancementControlNavigationMap,
	kWBASEEnhancementControlAutoplay
};

class WBASEEnhancements {
public:
	explicit WBASEEnhancements(const Common::String &targetName);

	bool enabled() const { return _enabled; }
	bool forcedAutoplayEnabled() const { return _forcedAutoplay; }
	bool controlsEnabled() const { return _enabled || _forcedAutoplay; }
	bool navigationMapEnabled() const { return controlsEnabled(); }
	void renderNavigationMap(const BaseData &data, const BaseEngine &engine, byte *framebuffer) const;
	WBASEEnhancementControl controlAtPoint(int x, int y) const;
	void renderControls(const BaseData &data, WBASEEnhancementPanel panel,
			WBASEEnhancementControl hoveredControl, bool autoplayActive, byte *framebuffer) const;

private:
	bool _enabled;
	bool _forcedAutoplay;
};

} // End of namespace Hopkins

#endif // HOPKINS_BASE_ENHANCEMENTS_H
