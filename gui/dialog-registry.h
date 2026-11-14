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

#ifndef GUI_DIALOG_REGISTRY_H
#define GUI_DIALOG_REGISTRY_H

#include "common/array.h"
#include "common/str.h"

namespace GUI {

class Dialog;
class DialogDumper;

/** Creates a dialog which needs nothing else. */
typedef Dialog *(*GuiDialogFactory)();

/**
 * Creates a dialog which needs a game target or a file from the dialog dumper.
 * engineId is the engine the dialog is created for.
 */
typedef Dialog *(*DumperDialogFactory)(DialogDumper &dumper, const Common::String &engineId);

/** A dialog registered with the dialog dumper registry. */
struct RegisteredDialog {
	Common::String name;              // e.g. "GlobalOptionsDialog"
	Common::String engineId;          // Empty for GUI dialogs, the engine ID otherwise
	Common::String factoryEngineId;   // Passed to dumperFactory
	GuiDialogFactory guiFactory = nullptr;
	DumperDialogFactory dumperFactory = nullptr;
};

/** Central registry of the dialogs the dialog dumper can generate snapshots for. */
class DialogDumperRegistry {
public:
	static void registerDialog(const Common::String &name, GuiDialogFactory factory);
	/** A GUI dialog created by the dialog dumper for the given engine. */
	static void registerDialog(const Common::String &name, DumperDialogFactory factory,
			const Common::String &engineId);
	static void registerEngineDialog(const Common::String &engineId, const Common::String &name,
			DumperDialogFactory factory);
	static const Common::Array<RegisteredDialog> &getAllDialogs();
	static void clear();
};

} // End of namespace GUI

#endif // GUI_DIALOG_REGISTRY_H
