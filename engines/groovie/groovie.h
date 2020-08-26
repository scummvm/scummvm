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

#ifndef GROOVIE_GROOVIE_H
#define GROOVIE_GROOVIE_H

#include "groovie/debug.h"
#include "groovie/font.h"

#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "groovie/detection.h"

namespace Common {
class MacResManager;
}

/**
 * This is the namespace of the Groovie engine.
 *
 * Status of this engine: This engine supports both versions of the Groovie
 * game engine.  The 7th Guest uses the first revision of Groovie, and is
 * now fully completable.  All remaining Groovie games use V2 of the engine,
 * which is under slow development.
 *
 * Games using this engine:
 * - The 7th Guest (completable)
 * - The 11th Hour
 * - Clandestiny
 * - Uncle Henry's Playhouse
 * - Tender Loving Care
 */
namespace Groovie {

class GraphicsMan;
class GrvCursorMan;
class MusicPlayer;
class ResMan;
class Script;
class VideoPlayer;

enum DebugLevels {
	kDebugVideo = 1 << 0,
	kDebugResource = 1 << 1,
	kDebugScript = 1 << 2,
	kDebugUnknown = 1 << 3,
	kDebugHotspots = 1 << 4,
	kDebugCursor = 1 << 5,
	kDebugMIDI = 1 << 6,
	kDebugScriptvars = 1 << 7,
	kDebugCell = 1 << 8,
	kDebugFast = 1 << 9
	// the current limitation is 32 debug levels (1 << 31 is the last one)
	// but some are used by system, so avoid high values.
};

/**
 * This enum reflects the available movie speed settings:
 * - Normal:  play videos at a normal speed
 * - Fast:    play videos with audio at a fast speed. Videos without audio,
 *            like teeth animations, are played at their regular speed to avoid
 *            audio sync issues
 */
enum GameSpeed {
	kGroovieSpeedNormal,
	kGroovieSpeedFast
};

#define MAX_SAVES 25

struct GroovieGameDescription;

class GroovieEngine : public Engine {
public:
	GroovieEngine(OSystem *syst, const GroovieGameDescription *gd);
	~GroovieEngine() override;

	Common::Platform getPlatform() const;

protected:

	// Engine APIs
	Common::Error run() override;
	void pauseEngineIntern(bool pause) override;

	bool hasFeature(EngineFeature f) const override;

	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	void syncSoundSettings() override;

public:
	void waitForInput();
	bool isWaitingForInput() { return _waitingForInput; }

	Graphics::PixelFormat _pixelFormat;
	bool _spookyMode;
	Script *_script;
	ResMan *_resMan;
	GrvCursorMan *_grvCursorMan;
	VideoPlayer *_videoPlayer;
	MusicPlayer *_musicPlayer;
	GraphicsMan *_graphicsMan;
	const Graphics::Font *_font;

	Common::MacResManager *_macResFork;

	GameSpeed _modeSpeed;

private:
	const GroovieGameDescription *_gameDescription;
	bool _waitingForInput;
	T7GFont _sphinxFont;
};

} // End of namespace Groovie

#endif // GROOVIE_GROOVIE_H
