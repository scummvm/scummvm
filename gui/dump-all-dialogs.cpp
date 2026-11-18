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

#include "gui/dump-all-dialogs.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/tokenizer.h"
#include "common/translation.h"

#include "gui/dialog-dumper.h"

namespace GUI {

static Common::Array<Common::String> splitList(const Common::String &list) {
	Common::Array<Common::String> result;
	Common::StringTokenizer tok(list, ",");
	while (!tok.empty()) {
		Common::String item = tok.nextToken();
		item.trim();
		if (!item.empty())
			result.push_back(item);
	}
	return result;
}

static DumpConfig buildDumpConfig() {
	DumpConfig config;

	if (ConfMan.hasKey("dump_output_dir"))
		config.outputDir = ConfMan.get("dump_output_dir");
	if (ConfMan.hasKey("dump_lang"))
		config.languages = splitList(ConfMan.get("dump_lang"));
	if (ConfMan.hasKey("dump_dialog"))
		config.dialogFilter = splitList(ConfMan.get("dump_dialog"));
	if (ConfMan.hasKey("dump_list_only"))
		config.listOnly = ConfMan.getBool("dump_list_only");

	// Engine dialogs are only dumped on request: --dump-engine=all or a list of engine IDs
	if (ConfMan.hasKey("dump_engine")) {
		config.includeEngines = true;
		for (const Common::String &engine : splitList(ConfMan.get("dump_engine"))) {
			if (engine.equalsIgnoreCase("all")) {
				config.engineFilter.clear();
				break;
			}
			config.engineFilter.push_back(engine);
		}
	}

	if (ConfMan.hasKey("dump_res")) {
		for (const Common::String &res : splitList(ConfMan.get("dump_res"))) {
			int w = 0, h = 0;
			if (sscanf(res.c_str(), "%dx%d", &w, &h) != 2 || w <= 0 || h <= 0) {
				warning("Ignoring invalid resolution '%s', expected WIDTHxHEIGHT", res.c_str());
				continue;
			}
			config.resolutionsW.push_back(w);
			config.resolutionsH.push_back(h);
		}
	}

	return config;
}

void dumpAllDialogs() {
#ifdef USE_TRANSLATION
	auto originalLang = TransMan.getCurrentLanguage();
#endif
	int original_window_width = ConfMan.getInt("last_window_width", Common::ConfigManager::kApplicationDomain);
	int original_window_height = ConfMan.getInt("last_window_height", Common::ConfigManager::kApplicationDomain);
	// The window size may have never been saved to the configuration
	if (original_window_width <= 0 || original_window_height <= 0) {
		original_window_width = g_system->getOverlayWidth();
		original_window_height = g_system->getOverlayHeight();
	}

	// HACK: Pass info to backend to force window resize
	ConfMan.setBool("dumper_force_resize", true, Common::ConfigManager::kApplicationDomain);

	{
		DialogDumper dumper(buildDumpConfig());
		dumper.dumpAll();
	}

#ifdef USE_TRANSLATION
	TransMan.setLanguage(originalLang);
#endif
	ConfMan.setInt("last_window_width", original_window_width, Common::ConfigManager::kApplicationDomain);
	ConfMan.setInt("last_window_height", original_window_height, Common::ConfigManager::kApplicationDomain);

	g_system->beginGFXTransaction();
		g_system->initSize(original_window_width, original_window_height);
	g_system->endGFXTransaction();

	// Clean up the temporary flag.
	// Since we are still within the same method where we added,
	// there is no need to flush config to the disk
	ConfMan.removeKey("dumper_force_resize", Common::ConfigManager::kApplicationDomain);
	ConfMan.flushToDisk();

	warning("ALL DIALOGS DUMPED");
}

} // End of namespace GUI
