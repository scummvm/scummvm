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

#ifndef GUI_SAVELOAD_H
#define GUI_SAVELOAD_H

#include "common/str.h"
#include "engines/metaengine.h"

namespace GUI {

class SaveLoadChooserDialog;

class SaveLoadChooser {
protected:
	SaveLoadChooserDialog *_impl;

	const Common::U32String _title;
	const Common::U32String _buttonLabel;
	const bool _saveMode;

	void selectChooser(const MetaEngine *engine);
public:
	SaveLoadChooser(const Common::U32String &title, const Common::U32String &buttonLabel, bool saveMode);
	~SaveLoadChooser();

	/**
	 * Runs the save/load chooser with the currently active config manager
	 * domain as target.
	 *
	 * @return The selcted save slot. -1 in case none is selected.
	 */
	int runModalWithCurrentTarget();
	int runModalWithMetaEngineAndTarget(const MetaEngine *engine, const Common::String &target);

	const Common::U32String &getResultString() const;

	/**
	 * Creates a default save description for the specified slot. Depending
	 * on the ScummVM configuration this might be a simple "Slot #" description
	 * or the current date and time.
	 *
	 * TODO: This might not be the best place to put this, since engines not
	 * using this class might want to mimic the same behavior. Check whether
	 * moving this to a better place makes sense and find what this place would
	 * be.
	 *
	 * @param slot The slot number (must be >= 0).
	 * @return The slot description.
	 */
	Common::String createDefaultSaveDescription(const int slot) const;
};

} // End of namespace GUI

#endif
