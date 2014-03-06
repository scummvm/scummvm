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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_SCENE_DATA_H
#define MADS_SCENE_DATA_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/rect.h"
#include "mads/action.h"
#include "mads/assets.h"
#include "mads/events.h"
#include "mads/game_data.h"
#include "mads/messages.h"

namespace MADS {

class MADSEngine;
class Scene;
class SpriteSlot;

#define MADS_INTERFACE_HEIGHT 44
#define MADS_SCENE_HEIGHT 156

#define DEPTH_BANDS_SIZE 15
#define MAX_ROUTE_NODES 22

#define DIRTY_AREAS_SIZE 90
#define DIRTY_AREAS_TEXT_DISPLAY_IDX 50

enum ScrCategory {
	CAT_NONE = 0, CAT_ACTION = 1, CAT_INV_LIST = 2, CAT_INV_VOCAB = 3,
	CAT_HOTSPOT = 4, CAT_INV_ANIM = 5, CAT_6 = 6, CAT_INV_SCROLLER = 7,
	CAT_12 = 12
};

enum Layer {
	LAYER_GUI = 19
};

class VerbInit {
public:
	int _id;
	int _action1;
	int _action2;

	VerbInit() {}
	VerbInit(int id, int action1, int action2): _id(id), _action1(action1), _action2(action2) {}
};

class ScreenObject {
public:
	Common::Rect _bounds;
	ScrCategory _category;
	int _descId;
	int _layer;

	ScreenObject();
};

class ScreenObjects: public Common::Array<ScreenObject> {
private:
	MADSEngine *_vm;

	int scanBackwards(const Common::Point &pt, int layer);

	void proc1();
public:
	int _v832EC;
	int _v7FECA;
	int _v7FED6;
	int _v8332A;
	int _yp;
	int _v8333C;
	int _selectedObject;
	ScrCategory _category;
	int _objectIndex;
	bool _released;

	/*
	 * Constructor
	 */
	ScreenObjects(MADSEngine *vm);

	/**
	 * Add a new item to the list
	 */
	void add(const Common::Rect &bounds, ScrCategory category, int descId);

	/**
	 */
	void check(bool scanFlag);
};

class DirtyArea {
private:
	static MADSEngine *_vm;
	friend class DirtyAreas;
public:
	Common::Rect _bounds;
	Common::Rect _bounds2;
	bool _textActive;
	bool _active;

	DirtyArea() { _active = false; }

	void setArea(int width, int height, int maxWidth, int maxHeight);

	void setSpriteSlot(const SpriteSlot *spriteSlot);

	/**
	* Set up a dirty area for a text display
	*/
	void setTextDisplay(const TextDisplay *textDisplay);
};

class DirtyAreas: public Common::Array<DirtyArea> {
private:
	MADSEngine *_vm;
public:
	DirtyAreas(MADSEngine *vm);
	
	/**
	* Merge together any designated dirty areas that overlap
	* @param startIndex	1-based starting dirty area starting index
	* @param count			Number of entries to process
	*/
	void merge(int startIndex, int count);

	bool intersects(int idx1, int idx2);
	void mergeAreas(int idx1, int idx2);
	void copy(MSurface *dest, MSurface *src, const Common::Point &posAdjust);
	void reset();
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
	virtual void step() = 0;

	/**
	 * Called before an action is started
	 */
	virtual void preActions() = 0;

	/**
	 * Handles scene actions
	 */
	virtual void actions() = 0;

	/**
	 * Post-action handling
	 */
	virtual void postActions() = 0;
};

struct ARTHeader {
	int _width;
	int _height;
	Common::Array<RGB6> _palette;
	Common::Array<RGB4> _palAnimData;

	void load(Common::SeekableReadStream *f);
};

class SceneNode {
public:
	Common::Point _walkPos;
	int _indexes[MAX_ROUTE_NODES];
	bool _active;

	/**
	 * Constructor
	 */
	SceneNode() : _active(false) {}

	/**
	 * Loads the scene node
	 */
	void load(Common::SeekableReadStream *f);
};
typedef Common::Array<SceneNode> SceneNodeList;


/**
 * Handles general data for a given scene
 */
class SceneInfo {
	class SpriteInfo {
	public:
		int _spriteSetIndex;
		Common::Point _position;
		int _depth;
		int _scale;

		void load(Common::SeekableReadStream *f);
	};
protected:
	MADSEngine *_vm;

	/**
	 * Constructor
	 */
	SceneInfo(MADSEngine *vm) : _vm(vm) {}
	
	/**
	 * Loads the given surface with depth information of a given scene
	 */
	virtual void loadCodes(MSurface &depthSurface);
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
	int _field4A;

	int _field4C;
	Common::Array<RGB4> _palAnimData;
	SceneNodeList _nodes;
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
	 loads the data
	 */
	void load(int sceneId, int flags, const Common::String &resName, int v3, 
		MSurface &depthSurface, MSurface &bgSurface);
};

class SceneInfoNebular : public SceneInfo {
	friend class SceneInfo;
protected:
	virtual void loadCodes(MSurface &depthSurface);

	/**
	* Constructor
	*/
	SceneInfoNebular(MADSEngine *vm) : SceneInfo(vm) {}
};

} // End of namespace MADS

#endif /* MADS_SCENE_DATA_H */
