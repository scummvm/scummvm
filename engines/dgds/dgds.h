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

#ifndef DGDS_DGDS_H
#define DGDS_DGDS_H

#include "common/error.h"
#include "common/events.h"
#include "common/platform.h"
#include "common/random.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "gui/debugger.h"

#include "dgds/resource.h"
#include "dgds/clock.h"

namespace Dgds {

class Console;
class ResourceManager;
class Decompressor;
class Image;
class GamePalettes;
class FontManager;
class SDSScene;
class GDSScene;
class Sound;
class REQFileData;
class Menu;
struct DgdsADS;
class ADSInterpreter;
class Globals;

enum DgdsGameId {
	GID_DRAGON,
	GID_CHINA,
	GID_BEAMISH
};

enum DgdsDetailLevel {
	kDgdsDetailLow = 0,
	kDgdsDetailHigh = 1
};

class DgdsEngine : public Engine {
public:
	Common::Platform _platform;
	Sound *_soundPlayer;
	Graphics::ManagedSurface _resData;

private:
	Console *_console;

	ResourceManager *_resource;
	Decompressor *_decompressor;

	DgdsGameId _gameId;
	Graphics::ManagedSurface _bottomBuffer;
	Graphics::ManagedSurface _topBuffer;
	SDSScene *_scene;
	GDSScene *_gdsScene;
	Menu *_menu;

	ADSInterpreter *_adsInterp;
	GamePalettes *_gamePals;
	Globals *_gameGlobals;

	FontManager *_fontManager;
	Common::SharedPtr<Image> _corners;
	Common::SharedPtr<Image> _icons;

	// Settings which we should integrate with ScummVM settings UI
	DgdsDetailLevel _detailLevel;
	int _textSpeed;

	bool _justChangedScene1;
	bool _justChangedScene2;

	Common::RandomSource _random;
	Common::Point _lastMouse; // originals start mouse at 0,0.

	Clock _clock;

public:
	DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~DgdsEngine();

	virtual Common::Error run() override;

	DgdsGameId getGameId() { return _gameId; }

	Graphics::ManagedSurface &getTopBuffer() { return _topBuffer; }
	Graphics::ManagedSurface &getBottomBuffer() { return _bottomBuffer; }
	Common::SeekableReadStream *getResource(const Common::String &name, bool ignorePatches);
	ResourceManager *getResourceManager() { return _resource; }
	Decompressor *getDecompressor() { return _decompressor; }
	const SDSScene *getScene() const { return _scene; }
	SDSScene *getScene() { return _scene; }
	GDSScene *getGDSScene() { return _gdsScene; }
	const FontManager *getFontMan() const { return _fontManager; }
	const Common::SharedPtr<Image> &getUICorners() { return _corners; }
	const Common::SharedPtr<Image> &getIcons() { return _icons; }
	bool changeScene(int sceneNum, bool runChangeOps);
	GamePalettes *getGamePals() { return _gamePals; }
	Globals *getGameGlobals() { return _gameGlobals; }
	void setMouseCursor(uint num);

	DgdsDetailLevel getDetailLevel() const { return _detailLevel; }
	int getTextSpeed() const { return _textSpeed; }
	void setShowClock(bool val);
	ADSInterpreter *adsInterpreter() { return _adsInterp; }
	bool justChangedScene1() const { return _justChangedScene1; }
	bool justChangedScene2() const { return _justChangedScene2; }
	Common::RandomSource &getRandom() { return _random; }
	Common::Point getLastMouse() const { return _lastMouse; }

	Clock &getClock() { return _clock; }

private:
	void loadCorners(const Common::String &filename);
	void loadIcons();
	void checkDrawInventoryButton();

};

} // End of namespace Dgds

#endif // DGDS_DGDS_H
