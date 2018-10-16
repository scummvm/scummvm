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

#ifndef GARGOYLE_GARGOLE_H
#define GARGOYLE_GARGOLE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/serializer.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "graphics/screen.h"
#include "gargoyle/events.h"
#include "gargoyle/glk.h"
#include "gargoyle/scott/scott.h"

namespace Gargoyle {

enum GargoyleDebugChannels {
	kDebugCore      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2,
	kDebugSound     = 1 << 3
};

#define GARGOYLE_SAVEGAME_VERSION 1

struct GargoyleGameDescription;

struct GargoyleSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class GargoyleEngine : public Engine {
private:
	/**
	 * Handles basic initialization
	 */
	void initialize();
private:
	const GargoyleGameDescription *_gameDescription;
	int _loadSaveSlot;
	Graphics::Screen _screen;
	Events _events;
	Glk _glk;
	Scott::Scott _scott;

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	GargoyleEngine(OSystem *syst, const GargoyleGameDescription *gameDesc);
	virtual ~GargoyleEngine();

	uint32 getFeatures() const;
	bool isDemo() const;
	Common::Language getLanguage() const;
};

} // End of namespace Gargoyle

#endif
