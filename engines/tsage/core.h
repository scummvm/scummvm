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

#ifndef TSAGE_CORE_H
#define TSAGE_CORE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/error.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "tsage/resources.h"
#include "tsage/saveload.h"

namespace TsAGE {

#define MAX_FLAGS 256

class EventHandler;
class SceneObject;
class SceneObjectList;
class ObjectMover;
class Action;
class Serializer;

class InvObject : public SavedObject {
public:
	int _sceneNumber;
	int _displayResNum;
	int _rlbNum;
	int _cursorNum;
	Rect _bounds;
	CursorType _cursorId;
	Common::String _description;
	int _iconResNum;

	int _visage;
	int _strip;
	int _frame;
public:
	InvObject(int sceneNumber, int rlbNum, int cursorNum, CursorType cursorId, const Common::String description);
	InvObject(int visage, int strip, int frame);
	InvObject(int visage, int strip);

	bool inInventory() const;
	void setCursor();

	Common::String getClassName() override { return "InvObject"; }
	void synchronize(Serializer &s) override {
		s.syncAsUint16LE(_sceneNumber);
	}
};

class InvObjectList : public SavedObject {
public:
	SynchronizedList<InvObject *> _itemList;
	InvObject *_selectedItem;

	InvObjectList();
	int indexOf(InvObject *obj) const;
	InvObject *getItem(int objectNum);
	int getObjectScene(int objectNum);

	Common::String getClassName() override { return "InvObjectList"; }
	void synchronize(Serializer &s) override;
};

/*--------------------------------------------------------------------------*/

/**
 * Basic reference counter class
 */
class RefCounter : public Serialisable {
private:
	int _ctr;
public:
	RefCounter() { clear(); }
	~RefCounter() override {}

	RefCounter(int v) { _ctr = v; }

	void clear() { _ctr = 0; }
	void setCtr(int v) { _ctr = v; }
	int decCtr() {
		if (_ctr > 0) --_ctr;
		return _ctr;
	}
	int incCtr() { return ++_ctr; }
	int getCtr() const { return _ctr; }

	void synchronize(Serializer &s) override { s.syncAsSint16LE(_ctr); }
};

class EventHandler : public SavedObject {
public:
	Action *_action;

	EventHandler() : SavedObject() { _action = NULL; }
	~EventHandler() override { destroy(); }

	void synchronize(Serializer &s) override { SYNC_POINTER(_action); }
	Common::String getClassName() override { return "EventHandler"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL) {}
	virtual void remove() {}
	virtual void signal() {}
	virtual void process(Event &event) {}
	virtual void dispatch();
	virtual void setAction(Action *action) { setAction(action, NULL); }
	virtual void setAction(Action *action, EventHandler *endHandler, ...);
	virtual void destroy() {}
};

class Action : public EventHandler {
public:
	EventHandler *_owner;
	int _actionIndex;
	int _delayFrames;
	uint32 _startFrame;
	bool _attached;
	EventHandler *_endHandler;

	Action();

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "Action"; }
	void remove() override;
	void process(Event &event) override;
	void dispatch() override;
	virtual void attached(EventHandler *newOwner, EventHandler *endHandler, va_list va);

	void attach(EventHandler *newOwner, EventHandler *endHandler, ...) {
		va_list va;
		va_start(va, endHandler);
		attached(newOwner, endHandler, va);
		va_end(va);
	}
	int getActionIndex() const { return _actionIndex; }
	void setActionIndex(int index) { _actionIndex = index; }
	void setDelay(int numFrames);
};

class ActionExt : public Action {
public:
	int _state;
};

#define ADD_PLAYER_MOVER(X, Y) { Common::Point pt(X, Y); PlayerMover *mover = new PlayerMover(); \
	g_globals->_player.addMover(mover, &pt, this); }
#define ADD_PLAYER_MOVER_NULL(OBJ, X, Y) { Common::Point pt(X, Y); PlayerMover *mover = new PlayerMover(); \
	OBJ.addMover(mover, &pt, NULL); }
#define ADD_PLAYER_MOVER_THIS(OBJ, X, Y) { Common::Point pt(X, Y); PlayerMover *mover = new PlayerMover(); \
	OBJ.addMover(mover, &pt, this); }

#define ADD_MOVER(OBJ, X, Y) { Common::Point pt(X, Y); NpcMover *mover = new NpcMover(); \
	OBJ.addMover(mover, &pt, this); }
#define ADD_MOVER_NULL(OBJ, X, Y) { Common::Point pt(X, Y); NpcMover *mover = new NpcMover(); \
	OBJ.addMover(mover, &pt, NULL); }

class ObjectMover : public EventHandler {
public:
	Common::Point _destPosition;
	Common::Point _moveDelta;
	Common::Point _moveSign;
	int _minorDiff;
	int _majorDiff;
	int _changeCtr;
	Action *_action;
	SceneObject *_sceneObject;
public:
	ObjectMover() { _action = NULL; _sceneObject = NULL; _minorDiff = 0; _majorDiff = 0; _changeCtr = 0;}
	~ObjectMover() override;

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "ObjectMover"; }
	void remove() override;
	void dispatch() override;
	virtual void startMove(SceneObject *sceneObj, va_list va) {}
	virtual void setup(const Common::Point &destPos);
	virtual bool dontMove() const;
	virtual void endMove();
};

class ObjectMover2 : public ObjectMover {
public:
	SceneObject *_destObject;
	int _minArea;
	int _maxArea;
public:
	ObjectMover2();
	~ObjectMover2() override {}

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "ObjectMover2"; }
	void dispatch() override;
	void startMove(SceneObject *sceneObj, va_list va) override;
	void endMove() override;
};

class ObjectMover3 : public ObjectMover2 {
public:
	Common::String getClassName() override { return "ObjectMover3"; }
	void dispatch() override;
	void startMove(SceneObject *sceneObj, va_list va) override;
	void endMove() override;
};

class NpcMover : public ObjectMover {
public:
	Common::String getClassName() override { return "NpcMover"; }
	void startMove(SceneObject *sceneObj, va_list va) override;
};

#define MAX_ROUTE_SIZE 20
#define ROUTE_END_VAL -20000

class RouteEnds {
public:
	Common::Point moveSrc;
	Common::Point moveDest;
};

class PlayerMover : public NpcMover {
protected:
	void setDest(const Common::Point &destPos);
	void pathfind(Common::Point *routeList, Common::Point srcPos, Common::Point destPos, RouteEnds routeEnds);
	int regionIndexOf(const Common::Point &pt);
	int regionIndexOf(int xp, int yp) { return regionIndexOf(Common::Point(xp, yp)); }
	int findClosestRegion(Common::Point &pt, const Common::List<int> &indexList);
	int checkMover(Common::Point &srcPos, const Common::Point &destPos);
	void doStepsOfNpcMovement(const Common::Point &pt1, const Common::Point &pt2, int numSteps, Common::Point &ptOut);
	int calculateRestOfRoute(int *routeList, int srcRegion, int destRegion, bool &foundRoute);

	static Common::Point *findLinePoint(RouteEnds *routeEnds, Common::Point *objPos, int length, Common::Point *outPos);
	static int findDistance(const Common::Point &pt1, const Common::Point &pt2);
	static bool sub_F8E5_calculatePoint(const Common::Point &pt1, const Common::Point &pt2, const Common::Point &pt3,
		const Common::Point &pt4, Common::Point *ptOut = NULL);
public:
	Common::Point _finalDest;
	Common::Point _routeList[MAX_ROUTE_SIZE];
	int _routeIndex;

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "PlayerMover"; }
	void startMove(SceneObject *sceneObj, va_list va) override;
	void endMove() override;
};

class PlayerMover2 : public PlayerMover {
public:
	SceneObject *_destObject;
	int _maxArea;
	int _minArea;
	PlayerMover2() : PlayerMover() { _destObject = NULL; _minArea = _maxArea = 0;}

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "PlayerMover2"; }
	void dispatch() override;
	void startMove(SceneObject *sceneObj, va_list va) override;
	void endMove() override;
};

/*--------------------------------------------------------------------------*/

class ScenePalette;

class PaletteModifier : public SavedObject {
public:
	ScenePalette *_scenePalette;
	Action *_action;
public:
	PaletteModifier();

	void synchronize(Serializer &s) override {
		SYNC_POINTER(_scenePalette);
		SYNC_POINTER(_action);
	}
	virtual void signal() = 0;
	virtual void remove() = 0;
};

class PaletteModifierCached: public PaletteModifier {
public:
	byte _palette[256 * 3];
	int _step;
	int _percent;

	PaletteModifierCached();

	virtual void setPalette(ScenePalette *palette, int step);
	Common::String getClassName() override { return "PaletteModifierCached"; }
	void synchronize(Serializer &s) override;
};

class PaletteRotation: public PaletteModifierCached {
public:
	int _delayCtr;
	uint32 _frameNumber;
	int _currIndex;
	int _start;
	int _end;
	int _rotationMode;
	int _duration;
	int _idxChange;
	int _countdown;
public:
	PaletteRotation();

	Common::String getClassName() override { return "PaletteRotation"; }
	void synchronize(Serializer &s) override;
	void signal() override;
	void remove() override;

	void setStep(int step) { _step = step; }
	void set(ScenePalette *palette, int start, int end, int rotationMode, int duration, Action *action);
	bool decDuration();
	void setDelay(int amount);
};

class PaletteFader: public PaletteModifierCached {
public:
	byte _palette[256 * 3];
public:
	Common::String getClassName() override { return "PaletteFader"; }
	void synchronize(Serializer &s) override;
	void signal() override;
	void remove() override;
	void setPalette(ScenePalette *palette, int step) override;
};

/*--------------------------------------------------------------------------*/

enum FadeMode {FADEMODE_NONE = 0, FADEMODE_GRADUAL = 1, FADEMODE_IMMEDIATE = 2};

class ScenePalette : public SavedObject {
public:
	byte _palette[256 * 3];
	GfxColors _colors;
	SynchronizedList<PaletteModifier *> _listeners;

	uint8 _redColor;
	uint8 _greenColor;
	uint8 _blueColor;
	uint8 _aquaColor;
	uint8 _purpleColor;
	uint8 _limeColor;
public:
	ScenePalette();
	ScenePalette(int paletteNum);
	~ScenePalette() override;

	bool loadPalette(int paletteNum);
	void loadPalette(const byte *pSrc, int start, int count);
	void replace(const ScenePalette *src) { loadPalette(src->_palette, 0, 256); }
	void refresh();
	void setPalette(int index, int count);
	void getEntry(int index, uint *r, uint *g, uint *b);
	void setEntry(int index, uint r, uint g, uint b);
	uint8 indexOf(uint r, uint g, uint b, int threshold = 0xffff, int start = 0, int count = 256);
	void getPalette(int start = 0, int count = 256);
	void signalListeners();
	void clearListeners();
	void fade(const byte *adjustData, bool fullAdjust, int percent);
	PaletteRotation *addRotation(int start, int end, int rotationMode, int duration = 0, Action *action = NULL);
	PaletteFader *addFader(const byte *arrBufferRGB, int palSize, int step, Action *action);

	static void changeBackground(const Rect &bounds, FadeMode fadeMode);

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "ScenePalette"; }
};

// DisplayParamType constant set. This must not be an enum
const int SET_WIDTH = 0;
const int SET_X = 1;
const int SET_Y = 2;
const int SET_FONT = 3;
const int SET_BG_COLOR = 4;
const int SET_FG_COLOR = 5;
const int SET_KEEP_ONSCREEN = 6;
const int SET_EXT_BGCOLOR = 7;
const int SET_EXT_FGCOLOR = 8;
const int SET_POS_MODE = 9;
const int SET_TEXT_MODE = 10;
const int LIST_END = -999;

class SceneItem : public EventHandler {
public:
	Rect _bounds;
	Common::String _msg;
	Common::Point _position;
	int _yDiff;
	int _sceneRegionId;
public:
	SceneItem() : EventHandler() { _msg = "Feature"; _action = NULL; _sceneRegionId = 0; _yDiff = 0;}

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "SceneItem"; }
	void remove() override;
	void destroy() override {}
	virtual bool startAction(CursorType action, Event &event);
	virtual void doAction(int action);

	bool contains(const Common::Point &pt);
	void setBounds(const Rect &newBounds) { _bounds = newBounds; }
	void setBounds(const int ys, const int xe, const int ye, const int xs) { _bounds = Rect(MIN(xs, xe), MIN(ys, ye), MAX(xs, xe), MAX(ys, ye)); }
	static void display(int resNum, int lineNum, ...);
	static void display2(int resNum, int lineNum);
	static void display(const Common::String &msg);
};

class SceneItemExt : public SceneItem {
public:
	int _state;

	Common::String getClassName() override { return "SceneItemExt"; }
	void synchronize(Serializer &s) override {
		SceneItem::synchronize(s);
		s.syncAsSint16LE(_state);
	}
};

class SceneHotspot : public SceneItem {
public:
	int _resNum, _lookLineNum, _useLineNum, _talkLineNum;
public:
	SceneHotspot();
	void synchronize(Serializer &s) override;
	bool startAction(CursorType action, Event &event) override;
	Common::String getClassName() override { return "SceneHotspot"; }
	void doAction(int action) override;

	void setDetails(int ys, int xs, int ye, int xe, const int resnum, const int lookLineNum, const int useLineNum);
	void setDetails(const Rect &bounds, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item);
	void setDetails(int sceneRegionId, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode = 0);
	void setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item);
	void setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
};

enum AnimateMode {ANIM_MODE_NONE = 0, ANIM_MODE_1 = 1, ANIM_MODE_2 = 2, ANIM_MODE_3 = 3,
		ANIM_MODE_4 = 4, ANIM_MODE_5 = 5, ANIM_MODE_6 = 6, ANIM_MODE_7 = 7, ANIM_MODE_8 = 8,
		// Introduced in Blue Force
		ANIM_MODE_9 = 9
};

// Actor effect enumeration used in Return to Ringworld 2
enum Effect { EFFECT_NONE = 0, EFFECT_SHADED = 1, EFFECT_SMOKE = 3,
	EFFECT_SHADOW_MAP = 5, EFFECT_SHADED2 = 6 };

class SceneObject;

class Visage {
private:
	byte *_data;

	void flipHorizontal(GfxSurface &s);
	void flipVertical(GfxSurface &s);
public:
	int _resNum;
	int _rlbNum;
	bool _flipHoriz;
	bool _flipVert;
public:
	Visage();
	Visage(const Visage &v);
	~Visage();

	void setVisage(int resNum, int rlbNum = 9999);
	GfxSurface getFrame(int frameNum);
	int getFrameCount() const;
	Visage &operator=(const Visage &gfxSurface);
};

class SceneObjectWrapper : public EventHandler {
private:
	Visage _visageImages;
public:
	SceneObject *_sceneObject;
public:
	SceneObjectWrapper() { _sceneObject = NULL; }
	~SceneObjectWrapper() override {}

	void setSceneObject(SceneObject *so);
	void check();

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "SceneObjectWrapper"; }
	void remove() override;
	void dispatch() override;
};

enum ObjectFlags {OBJFLAG_FIXED_PRIORITY = 1, OBJFLAG_NO_UPDATES = 2, OBJFLAG_ZOOMED = 4,
	OBJFLAG_SUPPRESS_DISPATCH = 8, OBJFLAG_HIDE = 0x100, OBJFLAG_HIDING = 0x200, OBJFLAG_REMOVE = 0x400,
	OBJFLAG_CLONED = 0x800, OBJFLAG_CHECK_REGION = 0x1000, OBJFLAG_PANE_0 = 0x4000, OBJFLAG_PANE_1 = 0x8000,
	OBJFLAG_PANES = OBJFLAG_PANE_0 | OBJFLAG_PANE_1,
	OBJFLAG_FLIP_CENTROID_X = 0x10000, OBJFLAG_FLIP_CENTROID_Y = 0x20000
};

class SceneObject : public SceneHotspot {
private:
	Visage _visageImages;

	int getNewFrame();
	void animEnded();
public:
	int changeFrame();
	uint32 _updateStartFrame;
	uint32 _walkStartFrame;
	Common::Point _oldPosition;
	int _percent;
	int _priority;
	int _angle;
	uint32 _flags;
	int _xs, _xe;
	Rect _paneRects[2];
	int _visage;
	SceneObjectWrapper *_objectWrapper;
	int _strip;
	AnimateMode  _animateMode;
	int _frame;
	int _endFrame;
	int _loopCount;
	int _frameChange;
	int _numFrames;
	int _regionIndex;
	EventHandler *_mover;
	Common::Point _moveDiff;
	int _moveRate;
	Common::Point _actorDestPos;
	Action *_endAction;
	uint32 _regionBitList;

	// Ringworld 2 specific fields
	byte *_shadowMap;
	int _shade, _oldShade;
	int _effect;
	SceneObject *_linkedActor;
public:
	SceneObject();
	SceneObject(const SceneObject &so);
	~SceneObject() override;

	void setPosition(const Common::Point &p, int yDiff = 0);
	void setStrip(int frameNum);
	void setStrip2(int frameNum);
	void setZoom(int percent);
	void updateZoom();
	void changeZoom(int percent);
	void setFrame(int frameNum);
	void setFrame2(int frameNum);
	void setPriority(int priority);
	void fixPriority(int priority);
	void setVisage(int visage);
	void setObjectWrapper(SceneObjectWrapper *objWrapper);
	void addMover(ObjectMover *mover, ...);
	void getHorizBounds();
	int getRegionIndex();
	int checkRegion(const Common::Point &pt);
	void animate(int animMode, ...);
	void checkAngle(const SceneObject *obj);
	void checkAngle(const Common::Point &pt);
	void hide();
	void show();
	int getSpliceArea(const SceneObject *obj);
	int getFrameCount();
	bool isNoMover() const { return !_mover || (_regionIndex > 0); }

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "SceneObject"; }
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void process(Event &event) override { event.handled = true; }
	void dispatch() override;
	virtual void calcAngle(const Common::Point &pt);
	virtual void removeObject();
	virtual GfxSurface getFrame();
	virtual void reposition();
	virtual void draw();
	virtual void proc19() {}
	virtual void updateScreen();
	// New methods introduced by Blue Force
	virtual void updateAngle(const Common::Point &pt);
	virtual void changeAngle(int angle);
	// New methods introduced by Ringworld 2
	virtual void copy(SceneObject *src);
	virtual SceneObject *clone() const;

	void setup(int visage, int stripFrameNum, int frameNum, int posX, int posY, int priority);
	void setup(int visage, int stripFrameNum, int frameNum);
};

class BackgroundSceneObject: public SceneObject {
public:
	Common::String getClassName() override { return "BackgroundSceneObject"; }
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void draw() override;
	SceneObject *clone() const override;

	void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY, int priority, int effect);
	static void copySceneToBackground();
};

class SceneText : public SceneObject {
public:
	int _fontNumber;
	int _width;
	TextAlign _textMode;
	int _color1;
	int _color2;
	int _color3;
	GfxSurface _textSurface;
public:
	SceneText();
	~SceneText() override;

	void setup(const Common::String &msg);

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "SceneText"; }
	GfxSurface getFrame() override { return _textSurface; }
	void updateScreen() override;
};

#define MAX_CHARACTERS 4
enum R2RCharacter { R2_NONE = 0, R2_QUINN = 1, R2_SEEKER = 2, R2_MIRANDA = 3 };

class Player : public SceneObject {
public:
	bool _canWalk;
	bool _uiEnabled;
	bool _enabled;

	// Return to Ringworld specific fields
	R2RCharacter _characterIndex;
	int _characterScene[MAX_CHARACTERS];
	int _oldCharacterScene[MAX_CHARACTERS];
	Common::Point _characterPos[MAX_CHARACTERS];
	int _characterStrip[MAX_CHARACTERS];
	int _characterFrame[MAX_CHARACTERS];
public:
	Player();

	Common::String getClassName() override { return "Player"; }
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void process(Event &event) override;

	void disableControl();
	void enableControl();
	void disableControl(CursorType cursorId, CursorType objectId = CURSOR_NONE);
	void enableControl(CursorType cursorId, CursorType objectId = CURSOR_NONE);
};

/*--------------------------------------------------------------------------*/

class LineSliceSet {
public:
	Common::Array<LineSlice> items;

	void load(int size, const byte *srcP) {
		for (int i = 0; i < size; ++i, srcP += 4)
			items.push_back(LineSlice(READ_LE_UINT16(srcP), READ_LE_UINT16(srcP + 2)));
	}
	void load2(int size, ...) {
		va_list va;
		va_start(va, size);

		while (size-- > 0) {
			int xs = va_arg(va, int);
			int xe = va_arg(va, int);
			items.push_back(LineSlice(xs, xe));
		}
		va_end(va);
	}

	void add(LineSlice &slice) { items.push_back(slice); }
	void add(int xs, int xe) { items.push_back(LineSlice(xs, xe)); }
	static LineSliceSet mergeSlices(const LineSliceSet &set1, LineSliceSet &set2);
};

class Region {
public:
	int _regionSize;
	int _regionId;
	Rect _bounds;
	Common::Array<LineSliceSet> _ySlices;
public:
	Region() { _regionSize = 0; _regionId = 0; }
	Region(int resNum, int rlbNum, ResourceType ctlType = RES_CONTROL);
	Region(int regionId, const byte *regionData);

	void load(const byte *regionData);
	bool contains(const Common::Point &pt);
	bool empty() const;
	void clear();
	void setRect(const Rect &r);
	void setRect(int xs, int ys, int xe, int ye);
	const LineSliceSet &getLineSlices(int yp);
	LineSliceSet sectPoints(int yp, const LineSliceSet &sliceSet);
	void draw();
	void uniteLine(int yp, LineSliceSet &sliceSet);
	void uniteRect(const Rect &rect);

	static LineSliceSet mergeSlices(const LineSliceSet &set1, const LineSliceSet &set2);
};

class SceneRegions : public Common::List<Region> {
public:
	void load(int sceneNum);

	int indexOf(const Common::Point &pt);
};

class SceneObjectList : public SavedObject {
private:
	void checkIntersection(Common::Array<SceneObject *> &ObjList, uint ObjIndex, int PaneNum);

	SynchronizedList<SceneObject *> _objList;
	bool _listAltered;
public:
	SceneObjectList() { _listAltered = false; }
	void sortList(Common::Array<SceneObject *> &ObjList);

	Common::String getClassName() override { return "SceneObjectList"; }
	void synchronize(Serializer &s) override;

	void draw();
	void activate();
	static void deactivate();

	typedef void (*EventHandlerFn)(EventHandler *fn);
	void recurse(EventHandlerFn Fn) {
		// Loop through each object
		_listAltered = false;
		for (SynchronizedList<SceneObject *>::iterator i = _objList.begin(); i != _objList.end() && !_listAltered; ) {
			SceneObject *o = *i;
			++i;
			Fn(o);
		}
	}
	SynchronizedList<SceneObject *>::iterator begin() { return _objList.begin(); }
	SynchronizedList<SceneObject *>::iterator end() { return _objList.end(); }
	int size() const { return _objList.size(); }
	bool contains(SceneObject *sceneObj) { return TsAGE::contains(_objList, sceneObj); }
	void push_back(SceneObject *sceneObj) { _objList.push_back(sceneObj); }
	void push_front(SceneObject *sceneObj) { _objList.push_front(sceneObj); }
	void remove(SceneObject *sceneObj) {
		_objList.remove(sceneObj);
		_listAltered = true;
	}
	void clear() { _objList.clear(); }
};

class ScenePriorities : public Common::List<Region> {
public:
	int _resNum;
	Region _defaultPriorityRegion;
public:
	void load(int resNum);

	Region *find(int priority);
};

/*--------------------------------------------------------------------------*/

class SceneItemList : public SynchronizedList<SceneItem *> {
public:
	void addItems(SceneItem *first, ...);
};

/*--------------------------------------------------------------------------*/

class RegionSupportRec {
public:
	int _yp;
	int _xp;
	int _xDiff;
	int _yDiff;
	int _xDirection;
	int _halfDiff;
	int _yDiff2;

	void process();
};

#define PROCESS_LIST_SIZE 100

class WalkRegion : public Region {
private:
	static RegionSupportRec _processList[PROCESS_LIST_SIZE];
	void loadProcessList(byte *dataP, int dataSize, int &dataIndex, int &regionHeight);
	int process1(int idx, byte *dataP, int dataSize);
	void process2(int dataIndex, int x1, int y1, int x2, int y2);
	void process3(int yp, int dataCount, int &idx1, int &idx2);
	void process4(int yp, int idx1, int idx2, int &count);
	void process5(int idx1, int idx2);
	void loadRecords(int yp, int size, int processIndex);
	void process6(RegionSupportRec &rec);
public:
	Common::Point _pt;
	int _idxListIndex;
	int _idxList2Index;
public:
	void loadRegion(byte *dataP, int size);
};

class WRField18 {
public:
	Common::Point _pt1, _pt2;
	int _v;
public:
	void load(byte *data);
};

class WalkRegions {
private:
	void loadOriginal();
	void loadRevised();
public:
	int _resNum;
	RouteEnds _routeEnds;
	Common::Array<WalkRegion> _regionList;
	Common::Array<WRField18> _field18;
	Common::Array<int> _idxList;
	Common::Array<int> _idxList2;
	Common::List<int> _disabledRegions;
public:
	WalkRegions() { _resNum = -1; }
	virtual ~WalkRegions() {}
	virtual void synchronize(Serializer &s);

	void clear();
	void load(int sceneNum);
	int indexOf(const Common::Point &pt, const Common::List<int> *indexList = NULL);
	WalkRegion &operator[](int idx) {
		assert((idx >= 1) && (idx <= (int)_regionList.size()));
		return _regionList[idx - 1];
	}
	void disableRegion(int regionId);
	void enableRegion(int regionId);
};

/*--------------------------------------------------------------------------*/

class FloatSet {
public:
	double _float1, _float2, _float3, _float4;

	FloatSet() { _float1 = _float2 = _float3 = _float4 = 0; }
	void add(double v1, double v2, double v3);
	void proc1(double v);
	double sqrt(FloatSet &floatSet);
};

/*--------------------------------------------------------------------------*/

class GameHandler : public EventHandler {
public:
	RefCounter _lockCtr;
	RefCounter _waitCtr;
	int _nextWaitCtr;
public:
	GameHandler();
	~GameHandler() override;
	void execute();

	void synchronize(Serializer &s) override;
	Common::String getClassName() override { return "GameHandler"; }
	void postInit(SceneObjectList *OwnerList = NULL) override {}
	void dispatch() override {}
};

class SceneHandler : public GameHandler {
public:
	int _saveGameSlot;
	int _loadGameSlot;
	int _delayTicks;
	Common::String _saveName;
	uint32 _prevFrameNumber;
protected:
	virtual void playerAction(Event &event) {}
	virtual void processEnd(Event &event) {}
	virtual void postLoad(int priorSceneBeforeLoad, int currentSceneBeforeLoad) {}
public:
	SceneHandler();
	void registerHandler();
	uint32 getFrameDifference();

	Common::String getClassName() override { return "SceneHandler"; }
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void process(Event &event) override;
	void dispatch() override;

	static void dispatchObject(EventHandler *obj);
	static void saveListener(Serializer &ser);
};

} // End of namespace TsAGE

#endif
