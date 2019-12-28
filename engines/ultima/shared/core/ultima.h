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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA_SHARED_ULTIMA_H
#define ULTIMA_SHARED_ULTIMA_H

#include "ultima/detection.h"
#include "Ultima/shared/core/debugger.h"
#include "Ultima/shared/core/events.h"
#include "common/archive.h"
#include "common/random.h"
#include "engines/engine.h"

namespace Ultima {
namespace Shared {

class UltimaEngine : public Engine {
    friend class EventsManager;
private:
    Common::RandomSource _randomSource;
protected:
	const Ultima::UltimaGameDescription *_gameDescription;
	Common::Archive *_dataArchive;
    Debugger *_debugger;
    EventsManager *_events;
protected:
	/**
	 * Initializes needed data for the engine
	 */
	virtual bool initialize();

    /**
     * Returns the data archive folder and version that's required
     */
    virtual bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
        return false;
    }

    /**
     * Checks if an auto save should be done, and if so, takes care of it
     */
    bool autoSaveCheck(int lastSaveTime);
public:
	UltimaEngine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~UltimaEngine();

    /**
     * Show a messae in a GUI dialog
     */
	void GUIError(const Common::String &msg);

	/**
	 * Get a random number
	 */
	uint getRandomNumber(uint maxVal) { return _randomSource.getRandomNumber(maxVal); }
};

extern UltimaEngine *g_ultima;

} // End of namespace Shared
} // End of namespace Ultima

#endif
