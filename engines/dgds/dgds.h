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
#include "common/platform.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "gui/debugger.h"

#include "dgds/resource.h"

namespace Dgds {

class Console;
class ResourceManager;
class Decompressor;
class Image;
class FontManager;
class SDSScene;
class GDSScene;
class Sound;
class REQFileData;
struct DgdsADS;

enum DgdsGameId {
	GID_DRAGON,
	GID_CHINA,
	GID_BEAMISH
};

class DgdsEngine : public Engine {
public:
	Common::Platform _platform;
	Sound *_soundPlayer;
	Image *_image;
	Graphics::ManagedSurface _resData;

private:
	Console *_console;

	ResourceManager *_resource;
	Decompressor *_decompressor;

	DgdsGameId _gameId;
	Graphics::Surface _bottomBuffer;
	Graphics::Surface _topBuffer;
	SDSScene *_scene;
	GDSScene *_gdsScene;

	FontManager *_fontManager;
	Common::Array<Common::SharedPtr<Image>> _corners;

public:
	DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~DgdsEngine();

	virtual Common::Error run() override;

	DgdsGameId getGameId() { return _gameId; }

	Graphics::Surface &getTopBuffer() { return _topBuffer; }
	Graphics::Surface &getBottomBuffer() { return _bottomBuffer; }
	Common::SeekableReadStream *getResource(const Common::String &name, bool ignorePatches);
	ResourceManager *getResourceManager() { return _resource; }
	Decompressor *getDecompressor() { return _decompressor; }
	const SDSScene *getScene() const { return _scene; }
	SDSScene *getScene() { return _scene; }
	const FontManager *getFontMan() const { return _fontManager; }
	void drawVCR(REQFileData &vcrRequestData);
	const Common::Array<Common::SharedPtr<Image>> &getUICorners() { return _corners; }

private:
	void loadCorners(const Common::String &filename, int numImgs);
};

} // End of namespace Dgds

#endif // DGDS_DGDS_H
