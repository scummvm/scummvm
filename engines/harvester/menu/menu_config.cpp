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

#include "harvester/menu/menu.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"

namespace Harvester {

static const char *const kMenuPath = "MENU.INI";
static const char *const kMenuSectionName = "menu";
static const int kOptionsItemCount = 7;

struct MenuWeekdayEntry {
	const char *key;
	const char *englishFallback;
};

static const MenuWeekdayEntry kMenuWeekdays[] = {
	{ "monday", "Monday" },
	{ "tuesday", "Tuesday" },
	{ "wednesday", "Wednesday" },
	{ "thursday", "Thursday" },
	{ "friday", "Friday" },
	{ "saturday", "Saturday" }
};

// The first three strings retain the padding from the European executable's
// fixed-width table so their centered text sits underneath the volume bars.
static const char *const kEuropeanBuiltInOptionItems[kOptionsItemCount] = {
	" SOUND FX              ",
	" MUSIC               ",
	" GAMMA               ",
	"TEXT",
	"GORE",
	"QUICK TIPS",
	"PASSWORD"
};

static bool loadRawMenuValue(const Common::Array<byte> &data, const char *key, Common::String &value) {
	value.clear();
	if (!key)
		return false;

	Common::String line;
	for (uint i = 0; i <= data.size(); ++i) {
		const bool atEnd = i == data.size();
		const byte chr = atEnd ? '\n' : data[i];
		if (chr == '\r')
			continue;

		if (chr != '\n') {
			line += static_cast<char>(chr);
			continue;
		}

		const char *separator = strchr(line.c_str(), '=');
		if (separator) {
			const Common::String currentKey(line.c_str(), separator);
			if (currentKey.equalsIgnoreCase(key)) {
				value = Common::String(separator + 1);
				return true;
			}
		}

		line.clear();
	}

	return false;
}

static void loadMenuDisplayValue(Common::INIFile &menu, const char *key, Common::String &dest) {
	Common::String value;
	if (!menu.getKey(key, kMenuSectionName, value) || value.empty())
		return;

	for (uint i = 0; i < value.size(); ++i) {
		if (value[i] == '_')
			value.setChar(' ', i);
	}
	dest = Common::move(value);
}

static void applyEuropeanBuiltInMenuText(MenuTextConfig &config) {
	config.yesLabel = "Yes";
	config.noLabel = "No";
	config.clickLabel = "Click";
	config.newGamePrompt = "START A NEW GAME?";
	config.quitGamePrompt = " QUIT HARVESTER?";
	for (uint i = 0; i < config.optionItems.size(); ++i)
		config.optionItems[i] = kEuropeanBuiltInOptionItems[i];
}

bool loadMenuTextConfig(HarvesterEngine &engine, MenuTextConfig &config) {
	config = MenuTextConfig();
	if (engine.isDemo()) {
		config.clickLabel = "Click";
		config.newGamePrompt = "START A NEW GAME?";
		config.quitGamePrompt = " QUIT HARVESTER?";
	}
	config.optionItems.resize(kOptionsItemCount);
	config.optionItems[0] = "SOUND FX";
	config.optionItems[1] = "MUSIC";
	config.optionItems[2] = "GAMMA";
	config.optionItems[3] = "TEXT";
	config.optionItems[4] = "GORE";
	config.optionItems[5] = "QUICK TIPS";
	config.optionItems[6] = "PASSWORD";
	config.weekdayLabels.resize(ARRAYSIZE(kMenuWeekdays));
	for (uint i = 0; i < config.weekdayLabels.size(); ++i)
		config.weekdayLabels[i] = kMenuWeekdays[i].englishFallback;

	ResourceManager *resources = engine.getResources();
	if (!resources)
		return false;

	Common::Array<byte> data;
	if (!resources->loadFile(kMenuPath, data)) {
		if (!engine.isDemo())
			applyEuropeanBuiltInMenuText(config);
		debugC(2, kDebugGeneral,
			"Harvester: using built-in menu text because '%s' is unavailable", kMenuPath);
		return true;
	}

	Common::MemoryReadStream stream(data.data(), data.size());
	Common::INIFile menu;
	menu.setDefaultSectionName(kMenuSectionName);
	menu.requireKeyValueDelimiter();
	menu.suppressValuelessLineWarning();
	if (!menu.loadFromStream(stream))
		return false;

	for (uint i = 0; i < config.optionItems.size(); ++i) {
		const Common::String key = Common::String::format("options_menu_%u", i + 1);
		Common::String value;
		if (loadRawMenuValue(data, key.c_str(), value) && !value.empty())
			config.optionItems[i] = Common::move(value);
		else if (menu.getKey(key, kMenuSectionName, value) && !value.empty())
			config.optionItems[i] = Common::move(value);
	}

	Common::String value;
	if (menu.getKey("yes", kMenuSectionName, value) && !value.empty())
		config.yesLabel = Common::move(value);
	if (menu.getKey("no", kMenuSectionName, value) && !value.empty())
		config.noLabel = Common::move(value);
	if (menu.getKey("click", kMenuSectionName, value) && !value.empty())
		config.clickLabel = Common::move(value);
	loadMenuDisplayValue(menu, "on", config.onLabel);
	loadMenuDisplayValue(menu, "off", config.offLabel);
	loadMenuDisplayValue(menu, "enter_password", config.enterPasswordLabel);
	if (loadRawMenuValue(data, "newgame", value) && !value.empty())
		config.newGamePrompt = Common::move(value);
	if (loadRawMenuValue(data, "quitgame", value) && !value.empty())
		config.quitGamePrompt = Common::move(value);
	loadMenuDisplayValue(menu, "talk_to", config.talkToVerb);
	loadMenuDisplayValue(menu, "examine", config.examineVerb);
	loadMenuDisplayValue(menu, "examine_the", config.examineTheVerb);
	loadMenuDisplayValue(menu, "operate", config.operateVerb);
	loadMenuDisplayValue(menu, "pick_up", config.pickUpVerb);
	loadMenuDisplayValue(menu, "use", config.useVerb);
	loadMenuDisplayValue(menu, "use_on", config.useOnPreposition);
	loadMenuDisplayValue(menu, "other", config.dialogueOtherLabel);
	loadMenuDisplayValue(menu, "responses", config.dialogueResponsesLabel);
	loadMenuDisplayValue(menu, "keyword", config.dialogueKeywordLabel);
	for (uint i = 0; i < config.weekdayLabels.size(); ++i)
		loadMenuDisplayValue(menu, kMenuWeekdays[i].key, config.weekdayLabels[i]);
	loadMenuDisplayValue(menu, "Exit", config.quickTipsExitLabel);
	loadMenuDisplayValue(menu, "next", config.quickTipsNextLabel);
	loadMenuDisplayValue(menu, "show_tips_on", config.quickTipsOnLabel);
	loadMenuDisplayValue(menu, "show_tips_off", config.quickTipsOffLabel);
	loadMenuDisplayValue(menu, "quick_tips_header", config.quickTipsHeader);

	return true;
}

Common::String buildUseItemPrompt(const MenuTextConfig &config,
		const Common::String &itemLabel, const Common::String &targetLabel) {
	if (itemLabel.empty())
		return Common::String();
	if (targetLabel.empty())
		return Common::String::format("%s %s %s",
			config.useVerb.c_str(), itemLabel.c_str(), config.useOnPreposition.c_str());

	return Common::String::format("%s %s %s %s", config.useVerb.c_str(), itemLabel.c_str(),
		config.useOnPreposition.c_str(), targetLabel.c_str());
}

} // End of namespace Harvester
