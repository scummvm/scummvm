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

#ifndef WAYNESWORLD_WAYNESWORLD_H
#define WAYNESWORLD_WAYNESWORLD_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"
#include "engines/engine.h"

struct ADGameDescription;

namespace WaynesWorld {

#define WAYNESWORLD_SAVEGAME_VERSION 0

enum {
	GF_GUILANGSWITCH =    (1 << 0) // If GUI language switch is required for menus
};

class WaynesWorldEngine : public Engine {
protected:
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:
	WaynesWorldEngine(OSystem *syst, const ADGameDescription *gd);
	~WaynesWorldEngine() override;
	const Common::String getTargetName() { return _targetName; }
	const ADGameDescription *_gameDescription;

private:
	Graphics::PixelFormat _pixelFormat;

#ifdef USE_TRANSLATION
	Common::String _oldGUILanguage;
#endif

public:
	Common::RandomSource *_random;

	int _mouseX, _mouseY;
	uint _mouseButtons;
	Common::KeyCode _keyCode;

	void updateEvents();
	int getRandom(int max);

	// Savegame API

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;
	/* TODO
	bool canLoadGameStateCurrently() override { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() override { return _isSaveAllowed; }
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &description, bool isAutosave = false) override;
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	bool existsSavegame(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	WARN_UNUSED_RESULT static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);
	*/

};

} // End of namespace WaynesWorld

#endif // WAYNESWORLD_WAYNESWORLD_H
