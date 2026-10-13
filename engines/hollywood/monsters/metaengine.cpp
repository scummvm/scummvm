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

#include "common/config-manager.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"

#include "hollywood/monsters/detection.h"
#include "hollywood/monsters/hollywood.h"
#include "hollywood/monsters/metaengine.h"

namespace Hollywood {
namespace {

const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_RESTORED_CONTENT,
		{
			_s("Enable restored content"),
			_s("Play optional content present in the game data but unused by the original release"),
			"restored_content",
			true,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of anonymous namespace

const ADExtraGuiOptionsMap *getMonstersExtraGuiOptions() {
	return optionsList;
}

void registerMonstersDefaultSettings() {
	ConfMan.registerDefault("restored_content", true);
}

void getMonstersSavegameThumbnail(Graphics::Surface &thumb) {
	if (g_engine && static_cast<HollywoodEngine *>(g_engine)->copyLastGameplayThumbnail(thumb))
		return;
	Graphics::createThumbnail(thumb);
}

} // End of namespace Hollywood
