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

#ifndef GUI_DIALOG_DUMPER_H
#define GUI_DIALOG_DUMPER_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"
#include "common/system.h"

#include "gui/dialog-registry.h"

namespace GUI {

class Dialog;
class Widget;

struct DumpConfig {
	Common::String outputDir = "snapshots";
	Common::Array<Common::String> languages;    // Empty = all
	Common::Array<int> resolutionsW;            // Empty = defaults
	Common::Array<int> resolutionsH;
	Common::Array<Common::String> dialogFilter; // Empty = all
	Common::Array<Common::String> engineFilter; // Empty = all of the engines, if includeEngines
	bool includeEngines = false;                // Dump the engine specific dialogs
	bool listOnly = false;
};

/**
 * Main orchestration class for dumping dialogs.
 * Uses DialogDumperRegistry to discover dialogs and iterate their states.
 */
class DialogDumper {
public:
	explicit DialogDumper(const DumpConfig &config);
	~DialogDumper();

	void dumpAll();

private:
	void layoutParentDialogs(const Common::Array<RegisteredDialog> &allDialogs);
	void dumpDialog(const RegisteredDialog &reg, const Common::String &lang, int w, int h);
	void dumpDialogStates(Dialog &dialog, const RegisteredDialog &reg, const Common::String &baseName,
		const Common::String &lang, int w, int h);
	void discoverEngineDialogs();
	void generateIndexHtml();
	bool matchesFilters(const RegisteredDialog &reg) const;
	Common::String createTempDomain(const Common::String &engineId, const Common::Path &path = Common::Path("."));
	void removeTempDomains();

	static Dialog *createTextViewerDialog(DialogDumper &dumper, const Common::String &engineId);
	static Dialog *createEditGameDialog(DialogDumper &dumper, const Common::String &engineId);
	static Dialog *createIntegrityDialog(DialogDumper &dumper, const Common::String &engineId);

	DumpConfig _config;
	bool _engineDialogsDiscovered = false;
	Common::Array<Common::String> _tempDomains;
};

} // End of namespace GUI

#endif // GUI_DIALOG_DUMPER_H