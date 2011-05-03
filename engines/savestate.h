/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ENGINES_SAVESTATE_H
#define ENGINES_SAVESTATE_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/ptr.h"


namespace Graphics {
struct Surface;
}

/**
 * A hashmap describing details about a given save state.
 * TODO
 * Guaranteed to contain save_slot and description values.
 * Additional ideas: Playtime, creation date, thumbnail, ...
 */
class SaveStateDescriptor : public Common::StringMap {
protected:
	Common::SharedPtr<Graphics::Surface> _thumbnail; // can be 0

public:
	SaveStateDescriptor() : _thumbnail() {
		setVal("save_slot", "-1");	// FIXME: default to 0 (first slot) or to -1 (invalid slot) ?
		setVal("description", "");
	}

	SaveStateDescriptor(int s, const Common::String &d) : _thumbnail() {
		char buf[16];
		sprintf(buf, "%d", s);
		setVal("save_slot", buf);
		setVal("description", d);
	}

	SaveStateDescriptor(const Common::String &s, const Common::String &d) : _thumbnail() {
		setVal("save_slot", s);
		setVal("description", d);
	}

	/** The saveslot id, as it would be passed to the "-x" command line switch. */
	Common::String &save_slot() { return getVal("save_slot"); }

	/** The saveslot id, as it would be passed to the "-x" command line switch (read-only variant). */
	const Common::String &save_slot() const { return getVal("save_slot"); }

	/** A human readable description of the save state. */
	Common::String &description() { return getVal("description"); }

	/** A human readable description of the save state (read-only variant). */
	const Common::String &description() const { return getVal("description"); }

	/** Optional entries only included when querying via MetaEngine::querySaveMetaInfo */

	/**
	 * Returns the value of a given key as boolean.
	 * It accepts 'true', 'yes' and '1' for true and
	 * 'false', 'no' and '0' for false.
	 * (FIXME:) On unknown value it errors out ScummVM.
	 * On unknown key it returns false as default.
	 */
	bool getBool(const Common::String &key) const;

	/**
	 * Sets the 'is_deletable' key, which indicates if the
	 * given savestate is safe for deletion.
	 */
	void setDeletableFlag(bool state);

	/**
	 * Sets the 'is_write_protected' key, which indicates if the
	 * given savestate can be overwritten or not
	 */
	void setWriteProtectedFlag(bool state);

	/**
	 * Return a thumbnail graphics surface representing the savestate visually.
	 * This is usually a scaled down version of the game graphics. The size
	 * should be either 160x100 or 160x120 pixels, depending on the aspect
	 * ratio of the game. If another ratio is required, contact the core team.
	 */
	const Graphics::Surface *getThumbnail() const { return _thumbnail.get(); }

	/**
	 * Set a thumbnail graphics surface representing the savestate visually.
	 * Ownership of the surface is transferred to the SaveStateDescriptor.
	 * Hence the caller must not delete the surface.
	 */
	void setThumbnail(Graphics::Surface *t);

	/**
	 * Sets the 'save_date' key properly, based on the given values.
	 */
	void setSaveDate(int year, int month, int day);

	/**
	 * Sets the 'save_time' key properly, based on the given values.
	 */
	void setSaveTime(int hour, int min);

	/**
	 * Sets the 'play_time' key properly, based on the given values.
	 */
	void setPlayTime(int hours, int minutes);

	/**
	 * Sets the 'play_time' key properly, based on the given value.
	 */
	void setPlayTime(uint32 msecs);
};

/** List of savestates. */
typedef Common::Array<SaveStateDescriptor> SaveStateList;


#endif
