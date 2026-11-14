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

#include "gui/dialog-registry.h"

namespace GUI {

static Common::Array<RegisteredDialog> s_registeredDialogs;

void DialogDumperRegistry::registerDialog(const Common::String &name, GuiDialogFactory factory) {
	RegisteredDialog dialog;
	dialog.name = name;
	dialog.guiFactory = factory;
	s_registeredDialogs.push_back(dialog);
}

void DialogDumperRegistry::registerDialog(const Common::String &name, DumperDialogFactory factory,
		const Common::String &engineId) {
	RegisteredDialog dialog;
	dialog.name = name;
	dialog.factoryEngineId = engineId;
	dialog.dumperFactory = factory;
	s_registeredDialogs.push_back(dialog);
}

void DialogDumperRegistry::registerEngineDialog(const Common::String &engineId, const Common::String &name,
		DumperDialogFactory factory) {
	RegisteredDialog dialog;
	dialog.name = name;
	dialog.engineId = engineId;
	dialog.factoryEngineId = engineId;
	dialog.dumperFactory = factory;
	s_registeredDialogs.push_back(dialog);
}

const Common::Array<RegisteredDialog> &DialogDumperRegistry::getAllDialogs() {
	return s_registeredDialogs;
}

void DialogDumperRegistry::clear() {
	s_registeredDialogs.clear();
}

} // End of namespace GUI
