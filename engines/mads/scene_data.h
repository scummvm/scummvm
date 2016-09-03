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

#ifndef MADS_SCENE_DATA_H
#define MADS_SCENE_DATA_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/rect.h"
#include "mads/action.h"
#include "mads/assets.h"
#include "mads/events.h"
#include "mads/game_data.h"
#include "mads/hotspots.h"
#include "mads/messages.h"
#include "mads/rails.h"
#include "mads/user_interface.h"

namespace MADS {

class MADSEngine;
class Scene;
class SpriteSlot;

#define MADS_INTERFACE_HEIGHT 44
#define MADS_SCENE_HEIGHT 156

#define DEPTH_BANDS_SIZE 15

#define SPRITE_SLOTS_MAX_SIZE 50
#define TEXT_DISPLAY_MAX_SIZE 40
#define DIRTY_AREAS_SIZE (SPRITE_SLOTS_MAX_SIZE + TEXT_DISPLAY_MAX_SIZE)

enum {
	SCENEFLAG_DITHER		= 0x01,     // Dither to 16 colors
	SCENEFLAG_LOAD_SHADOW	= 0x10,		// Load hard shadows
	SCENEFLAG_TRANSLATE		= 0x10000	// Translate palette of loaded background
};

class VerbInit {
public:
	int _id;
	VerbType _verbType;
	PrepType _prepType;

	VerbInit() {}
	VerbInit(int id, VerbType verbType, PrepType prepType)
		: _id(id), _verbType(verbType), _prepType(prepType) {
	}
};

class SceneLogic {
protected:
	MADSEngine *_vm;
	Scene *_scene;
public:
	/**
	 * Constructor
	 */
	SceneLogic(MADSEngine *vm);

	/**
	 * Destructor
	 */
	virtual ~SceneLogic() {}

	/**
	 * Called to initially setup a scene
	 */
	virtual void setup() = 0;

	/**
	 * Called as the scene is entered (made active)
	 */
	virtual void enter() = 0;

	/**
	 * Called one per frame
	 */
	virtual void step() {}

	/**
	 * Called before an action is started
	 */
	virtual void preActions() {}

	/**
	 * Handles scene actions
	 */
	virtual void actions() = 0;

	/**
	 * Post-action handling
	 */
	virtual void postActions() {}

	/**
	 * Unhandled action handling
	 */
	virtual void unhandledAction() {}

	/**
	 * Synchronize any local data for the scene
	 */
	virtual void synchronize(Common::Serializer &s) {}
};

struct ARTHeader {
	int _width;
	int _height;
	Common::Array<RGB6> _palette;
	Common::Array<PaletteCycle> _paletteCycles;

	void load(Common::SeekableReadStream *f, bool isV2);
};

/**
 * Handles general data for a given scene
 */
class SceneInfo {
	class SpriteInfo {
	public:
		int _spriteSetIndex;
		Common::Point _position;
		int _frameNumber;
		int _depth;
		int _scale;

		void load(Common::SeekableReadStream *f);
	};
protected:
	MADSEngine *_vm;

	/**
	 * Constructor
	 */
	SceneInfo(MADSEngine *vm);
public:
	int _sceneId;
	int _artFileNum;
	int _depthStyle;
	int _width;
	int _height;

	int _yBandsEnd;
	int _yBandsStart;
	int _maxScale;
	int _minScale;
	int _depthList[DEPTH_BANDS_SIZE];
	int _field4A; // Useless field ?

	int _usageIndex;
	Common::Array<PaletteCycle> _paletteCycles;
	WalkNodeList _nodes;
public:
	/**
	 * Destructor
	 */
	virtual ~SceneInfo() {}

	/**
	 * Instantiates the class
	 */
	static SceneInfo *init(MADSEngine *vm);

	/**
	 * loads the data
	 */
	void load(int sceneId, int variant, const Common::String &resName, int flags,
		DepthSurface &depthSurface, BaseSurface &bgSurface);

	/**
	 * Loads the palette for a scene
	 */
	void loadPalette(int sceneId, int artFileNum, const Common::String &resName, int flags, BaseSurface &bgSurface);

	/**
	 * Loads a V1 game background
	 */
	void loadMadsV1Background(int sceneId, const Common::String &resName, int flags, BaseSurface &bgSurface);

	/**
	 * Loads a V2 game background
	 */
	void loadMadsV2Background(int sceneId, const Common::String &resName, int flags, BaseSurface &bgSurface);

	/**
	 * Loads the given surface with depth information of a given scene
	 * @param depthSurface	Depth/walk surface
	 * @param variant		Variant number to load
	 */
	virtual void loadCodes(BaseSurface &depthSurface, int variant) = 0;

	/**
	 * Loads the given surface with depth information of a given scene
	 * @param depthSurface	Depth/walk surface
	 * @param stream		Stream to load the data from
	 */
	virtual void loadCodes(BaseSurface &depthSurface, Common::SeekableReadStream *stream) = 0;
};

} // End of namespace MADS

#endif /* MADS_SCENE_DATA_H */
