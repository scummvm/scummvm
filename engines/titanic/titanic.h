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

#ifndef TITANIC_TITANIC_H
#define TITANIC_TITANIC_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/serializer.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"

/**
 * This is the namespace of the Titanic engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Starship Titanic
 */
namespace Titanic {

enum TitanicDebugChannels {
	kDebugCore      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2,
	kDebugSound     = 1 << 3
};

#define TITANIC_SAVEGAME_VERSION 1

struct TitanicGameDescription;

struct TitanicSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class TitanicEngine : public Engine {
private:
	/**
	 * Handles basic initialization
	 */
	void initialize();
protected:
	const TitanicGameDescription *_gameDescription;
	int _loadSaveSlot;

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	TitanicEngine(OSystem *syst, const TitanicGameDescription *gameDesc);
	virtual ~TitanicEngine();

	uint32 getFeatures() const;
	bool isDemo() const;
	Common::Language getLanguage() const;
};

} // End of namespace Titanic

#endif /* TITANIC_TITANIC_H */
