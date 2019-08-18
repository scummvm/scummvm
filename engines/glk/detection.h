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

#ifndef GLK_DETECTION_H
#define GLK_DETECTION_H

#include "engines/advancedDetector.h"
#include "engines/game.h"

#define MAX_SAVES 99

/**
 * ScummVM Meta Engine interface
 */
class GlkMetaEngine : public MetaEngine {
private:
	Common::String findFileByGameId(const Common::String &gameId) const;
public:
	GlkMetaEngine() : MetaEngine() {}

	virtual const char *getName() const {
		return "ScummGlk";
	}

	virtual const char *getOriginalCopyright() const {
		return "Infocom games (C) Infocom\nScott Adams games (C) Scott Adams";
	}

	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const override;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	/**
	 * Returns a list of games supported by this engine.
	 */
	virtual PlainGameList getSupportedGames() const override;

	/**
	 * Runs the engine's game detector on the given list of files, and returns a
	 * (possibly empty) list of games supported by the engine which it was able
	 * to detect amongst the given files.
	 */
	virtual DetectedGames detectGames(const Common::FSList &fslist) const override;

	/**
	 * Query the engine for a PlainGameDescriptor for the specified gameid, if any.
	 */
	virtual PlainGameDescriptor findGame(const char *gameId) const override;

	/**
	 * Calls each sub-engine in turn to ensure no game Id accidentally shares the same Id
	 */
	void detectClashes() const;
};

namespace Glk {

/**
 * Holds the name of a recognised game
 */
struct GameDescriptor {
	const char *_gameId;
	const char *_description;
	uint _options;

	GameDescriptor(const char *gameId, const char *description, uint options) :
		_gameId(gameId), _description(description), _options(options) {}
	GameDescriptor(const PlainGameDescriptor &gd) : _gameId(gd.gameId), _description(gd.description),
		_options(0) {}

	static PlainGameDescriptor empty() {
		return GameDescriptor(nullptr, nullptr, 0);
	}

	operator PlainGameDescriptor() const {
		PlainGameDescriptor pd;
		pd.gameId = _gameId;
		pd.description = _description;
		return pd;
	}
};

/**
 * Derived game descriptor class to simplifying setting up needed properties
 */
class GlkDetectedGame : public DetectedGame {
public:
	GlkDetectedGame(const char *id, const char *desc, const Common::String &filename);
	GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		Common::Language lang);
	GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		const Common::String &md5, size_t filesize);
};

/**
 * Game detection entry
 */
struct GlkDetectionEntry {
	const char *const _gameId;
	const char *const _extra;
	const char *const _md5;
	size_t _filesize;
	Common::Language _language;
};

#define DT_ENTRY0(ID, MD5, FILESIZE) { ID, "", MD5, FILESIZE, Common::EN_ANY }
#define DT_ENTRY1(ID, EXTRA, MD5, FILESIZE) { ID, EXTRA, MD5, FILESIZE, Common::EN_ANY }
#define DT_ENTRYL0(ID, LANG, MD5, FILESIZE) { ID, "", MD5, FILESIZE, LANG }
#define DT_ENTRYL1(ID, LANG, EXTRA, MD5, FILESIZE) { ID, EXTRA, MD5, FILESIZE, LANG }

#define DT_END_MARKER { nullptr, nullptr, nullptr, 0, Common::EN_ANY }

} // End of namespace Glk

#endif
