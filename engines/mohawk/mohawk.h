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

#ifndef MOHAWK_MOHAWK_H
#define MOHAWK_MOHAWK_H

#include "common/scummsys.h"
#include "common/array.h"

#include "engines/engine.h"

#include "mohawk/detection.h"

class OSystem;

namespace Common {
class SeekableReadStream;
}

/**
 * This is the namespace of the Mohawk engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Myst
 * - Riven: The Sequel to Myst
 */
namespace Mohawk {

class Sound;
class PauseDialog;
class Archive;
class CursorManager;

class MohawkEngine : public ::Engine {
protected:
	Common::Error run() override;

public:
	MohawkEngine(OSystem *syst, const MohawkGameDescription *gamedesc);
	~MohawkEngine() override;

	// Detection related functions
	const MohawkGameDescription *_gameDescription;
	const char *getGameId() const;
	uint32 getFeatures() const;
	bool isGameVariant(MohawkGameFeatures feature) const;
	const char *getAppName() const;
	Common::Platform getPlatform() const;
	uint8 getGameType() const;
	virtual Common::Language getLanguage() const;

	bool hasFeature(EngineFeature f) const override;

	CursorManager *_cursor;

	virtual Common::SeekableReadStream *getResource(uint32 tag, uint16 id);
	bool hasResource(uint32 tag, uint16 id);
	bool hasResource(uint32 tag, const Common::String &resName);
	uint32 getResourceOffset(uint32 tag, uint16 id);
	uint16 findResourceID(uint32 type, const Common::String &resName);
	Common::String getResourceName(uint32 tag, uint16 id);
	void closeAllArchives();

	void pauseGame();

private:
	PauseDialog *_pauseDialog;

protected:
	// An array holding the main Mohawk archives require by the games
	Common::Array<Archive *> _mhk;
};

} // End of namespace Mohawk

#endif
