/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_CONTAIN_H
#define SAGA2_CONTAIN_H

#include "saga2/floating.h"

namespace Saga2 {

//	gPanel
//		gControl
class ContainerView;
class ScrollingContainerView;
class ActorContainerView;
class ReadyContainerView;
class SmallContainerView;
class EnchantContainerView;
//		gWindow
//			DecoratedWindow
//				FloatingWindow
class ContainerWindow;
class ContainerNode;
class ContainerManager;
struct ContainerAppearanceDef;

class CMassWeightIndicator;
class ProtoObj;

class GfxCompButton;
class GfxCompImage;
class GfxMultCompButton;
struct TilePoint;

/* ===================================================================== *
   Class definitions
 * ===================================================================== */

//  The base class for all container panels

class ContainerView : public gControl {

	friend class ContainerWindow;
	friend class TangibleContainerWindow;
	friend class IntangibleContainerWindow;

protected:

	enum imageData {
		kSelectorX = 10,
		kSelectorY = 25
	};

public:

	ContainerNode   &_node;

	Point16         _iconOrigin;     //  of the top left icon.
	Point16         _iconSpacing;    //  The spacing between icons (in both X and Y)

	//  The number of rows and columns of icons that can be seen
	int16           _visibleRows,
	                _visibleCols;

	//  The total number of rows, and the scroll position of the control
	int16           _totalRows,
	                _scrollPosition;

	//  Pointer to the object that this control is showing the
	//  contents of.
	GameObject      *_containerObject;

	//  Mass and bulk indicators
	int16           _totalMass,
	                _totalBulk;

	//  Number of visible objects currently in the container
	int16           _numObjects;

	//  Constructor
	ContainerView(
	    gPanelList &,
	    const Rect16 &,
	    ContainerNode &nd,
	    const ContainerAppearanceDef &app,
	    AppFunc *cmd = NULL);

	//  Destructor
	~ContainerView();

	// redraw the panel offscreen
	virtual void drawClipped(gPort &port, const Point16 &offset, const Rect16 &clip);

	// draws the mereged object multi-item selector
	void drawSelector(gPort &port, Point16 &pos);

	//  Draw the quantity indicator below the object if quantity > 1
	void drawQuantity(gPort &port, GameObject *item, ProtoObj *objProto, int16 x, int16 y);

	//  returns true if the object is visible for this type of
	//  container.
	virtual bool isVisible(GameObject *obj);

	//  total the mass, bulk, and number of all objects in container.
	void totalObjects();

	//  Get the Nth visible object from this container.
	ObjectID getObject(int16 slotNum);

	void setContainer(GameObject *container);

	// get the slot the point is over
	TilePoint pickObjectSlot(const Point16 &pickPos);

	//  Get the object in a slot (u/v)
	GameObject *getObject(const TilePoint &slot);

	// Get the object that the point is over
	GameObject *pickObject(const Point16 &pickPos);

	//  Get the object ID that the point is over
	ObjectID  pickObjectID(const Point16 &pickPos);

protected: // actions within a container

	// These are the actions when there is no Item in the mouse
	virtual void clickOn(gPanelMessage &msg, GameObject *mObj,  GameObject *cObj);
	virtual void dblClickOn(gPanelMessage &msg, GameObject *mObj,  GameObject *cObj);

	// this gets a merged item
	void getMerged(GameObject *obj);

	// drop Physical Object into container
	virtual void dropPhysical(gPanelMessage &msg, GameObject *mObj,  GameObject *cObj, int16 num = 1);

	// Use Physical Object on other object in container
	virtual void usePhysical(gPanelMessage &msg, GameObject *mObj,  GameObject *cObj);

	// Use Concept or Psycological object in container
	// note: only valid container use is to drop into
	//       center characters's ready container.
	virtual void useConcept(gPanelMessage &msg, GameObject *mObj,  GameObject *cObj);

	// Use Spell or Skill on other object in container or
	// drop into center characters's ready container.
	virtual void useSpell(gPanelMessage &msg, GameObject *mObj,  GameObject *cObj);

	//  Event-handling functions

	bool activate(gEventType why);       // activate the control
	void deactivate();

	virtual void pointerMove(gPanelMessage &msg);
	virtual bool pointerHit(gPanelMessage &msg);
	virtual void pointerRelease(gPanelMessage &msg);
	virtual void timerTick(gPanelMessage &msg);

	void dblClick(GameObject *mouseObject, GameObject *slotObject, gPanelMessage &msg);

private:
	//  Container manipulation functions
	void add(ObjectID newObj);
	void remove(ObjectID obj);
	void swap(ObjectID newObj, ObjectID oldObj);

	//  Determine if the mouse is pointing a new object, and if so,
	//  adjust the mouse text
	void updateMouseText(Point16 &pickPos);
	void setCursorText(GameObject *obj);
	void setDelayedCursorText(GameObject *obj);
};

// sub class for ready inventory items

class ReadyContainerView : public ContainerView {
private:
	void            **_backImages;       // pointers to background imagery
	int16           _numIm;

public:

	ReadyContainerView(gPanelList &,
	                   const Rect16 &,
	                   ContainerNode &,
	                   void **backgrounds,
	                   int16 numRes,
	                   int16 numRows,
	                   int16 numCols,
	                   int16 totRows,
	                   AppFunc *cmd);

	// redraw the panel offscreen
	virtual void drawClipped(gPort &port, const Point16 &offset, const Rect16 &clip);

	void setScrollOffset(int8 num);
	void timerTick(gPanelMessage &msg);
};

//  sub class for enchantment container panels

class EnchantmentContainerView : public ContainerView {
public:
	EnchantmentContainerView(gPanelList &list,
	                         ContainerNode &nd,
	                         const ContainerAppearanceDef &app,
	                         AppFunc *cmd = NULL);

	virtual void pointerMove(gPanelMessage &msg);
	virtual bool pointerHit(gPanelMessage &msg);
};


//  The container window is simply a floating window with an embedded
//  container view panel and a close button control

class ContainerWindow : public FloatingWindow {
protected:
	GfxCompButton   *_closeCompButton;       //  the close button object
	ContainerView   *_view;          //  the container view object

public:
	ContainerWindow(ContainerNode &nd,
	                const ContainerAppearanceDef &app,
	                const char saveas[]);

	virtual ~ContainerWindow();

	ContainerView &getView();
	GameObject *containerObject() {
		return getView()._containerObject;
	}

	virtual void massBulkUpdate() {}
};

//  Base class for all container windows with scroll control
class ScrollableContainerWindow : public ContainerWindow {
protected:
	GfxCompButton     *_scrollCompButton;

public:
	ScrollableContainerWindow(ContainerNode &nd,
	                          const ContainerAppearanceDef &app,
	                          const char saveas[]);

	void scrollUp() {
		if (_view->_scrollPosition > 0) _view->_scrollPosition--;
	}

	void scrollDown() {
		if (_view->_scrollPosition + _view->_visibleRows < _view->_totalRows)
			_view->_scrollPosition++;
	}
};

//  A container window for tangible containers
class TangibleContainerWindow : public ScrollableContainerWindow {
private:
	GfxCompImage      *_containerSpriteImg;
	CMassWeightIndicator *_massWeightIndicator;

	Rect16          _objRect;
	bool            _deathFlag;

private:
	void setContainerSprite();

public:

	TangibleContainerWindow(ContainerNode &nd,
	                        const ContainerAppearanceDef &app);
	~TangibleContainerWindow();

	void drawClipped(gPort &port, const Point16 &offset, const Rect16 &clip);

	// this sets the mass and bulk gauges for physical containers
	void massBulkUpdate();
};

class IntangibleContainerWindow : public ScrollableContainerWindow {
protected:
	friend  void setMindContainer(int index, IntangibleContainerWindow &cw);
private:
	GfxMultCompButton *_mindSelectorCompButton;

public:

	IntangibleContainerWindow(ContainerNode &nd, const ContainerAppearanceDef &app);
};

class EnchantmentContainerWindow : public ContainerWindow {
protected:
	GfxCompButton     *_scrollCompButton;

public:
	EnchantmentContainerWindow(ContainerNode &nd,
	                           const ContainerAppearanceDef &app);
};

/* ===================================================================== *
    ContainerAppearanceDef: A record listing container appearance info
 * ===================================================================== */

struct ContainerAppearanceDef {
	StaticRect defaultWindowPos;       //  default position of window
	StaticRect viewRect;               //  position of view within window
	StaticRect closeRect,              //  position of close button
	           scrollRect,             //  position of scrolling button
	           iconRect,               //  position of container icon
	           massRect;               //  position of mass & bulk indicator

	hResID closeResID[2],              //  resource ID's for close box
	       scrollResID[2];             //  resource ID's for scroll indicator

	StaticPoint16 iconOrigin,
	              iconSpacing;
	uint16 rows,
	       cols,
	       totRows;
};

/* ===================================================================== *
    ContainerNode: records the fact that a container was open for a
    specific player
 * ===================================================================== */

//  ContainerNode records the fact that a container was opened for a
//  specific player

//  REM: What about the ordering of windows?

class ContainerNode {

	friend class    ContainerManager;
	friend class    ContainerView;
	friend class    ContainerWindow;

public:
	enum ContainerNodeOwnerType {
		kReadyType   = 0,                    //  This is a player ready container
		kDeadType,                           //  The "dead" container
		kMentalType,                         //  A player's mental container
		kPhysicalType,                       //  Physical container
		kEnchantType                         //  Enchantment container
	};

	enum ContainerNodeOwners {
		kNobody = 255                        //  owner = 255 means it's on the ground
	};

	enum containerAction {
		kActionUpdate    = (1 << 0),         //  Refresh this window
		kActionDelete    = (1 << 1),         //  Delete this window
		kActionHide      = (1 << 2),         //  Refresh this window
		kActionShow      = (1 << 3)          //  Refresh this window
	};

private:
	ObjectID        _object;                 //  Object being viewed
	uint8           _type;                   //  type of container
	uint8           _owner;                  //  which brother owns this container
	Rect16          _position;               //  position of window
	ContainerWindow *_window;                //  window, which may be NULL if hidden.
	uint8           _action;                 //  What action to take on container
public:
	uint8           _mindType;               //  mindContainer type

private:
	//  Nested structure used to archive ContainerNodes
	struct Archive {
		ObjectID        object;
		uint8           type;
		uint8           owner;
		Rect16          position;
		uint8           mindType;
		bool            shown;
	};

public:
	ContainerNode() {
		_object = 0;
		_type = 0;
		_owner = 0;
		_window = nullptr;
		_action = 0;
		_mindType = 0;
	}
	ContainerNode(ContainerManager &cl, ObjectID id, int type);
	~ContainerNode();

	static int32 archiveSize() {
		return sizeof(Archive);
	}

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);

	//  Hide or show this container window.
	void hide();
	void show();
	void update();                   //  Update container associated with this node

	//  Set for lazy deletion
	void markForDelete()   {
		_action |= kActionDelete;
	}
	void markForShow() {
		_action |= kActionShow;
		_action &= ~kActionHide;
	}
	void markForHide() {
		_action |= kActionHide;
		_action &= ~kActionShow;
	}
	void markForUpdate()   {
		_action |= kActionUpdate;
	}

	//  Find the address of the window and/or view
	ContainerWindow *getWindow();
	ContainerView   *getView();

	//  Access functions
	uint8 getType() {
		return _type;
	}
	uint8 getOwnerIndex() {
		return _owner;
	}
	ObjectID getObject() {
		return _object;
	}
	Rect16 &getPosition() {
		return _position;
	}
	void setObject(ObjectID id) {
		_object = id;
	}

	//  returns true if the object represented by the container can be
	//  accessed by the player.
	bool isAccessable(ObjectID enactor);

	void changeOwner(int16 newOwner);
};

//  A list of container nodes

class ContainerManager {
public:
	Common::List<ContainerNode *> _list;

	enum {
		kBufSize     = 60,
	};

	// used to ignore doubleClick when doubleClick == singleClick
	bool _alreadyDone;

	// this will be used to determine if the cursor has been
	// held over an object long enough to qualify for the hint to be displayed
	bool _objTextAlarm;

	// determines if the cursor is in *A* container view
	bool _mouseInView;

	ObjectID _lastPickedObjectID;

	// number of items to move for merged objects
	uint16 _numPicked;

	int16 _amountIndY;

	// this will be used to hold a value of uint16 plus a -1 as a flag
	int32 _lastPickedObjectQuantity;

	int32 _amountAccumulator;

	// merged object currently being gotten
	GameObject *_objToGet;

	// selector image pointer
	void *_selImage;

	// buffer for the mouse text
	char _mouseText[kBufSize];

	ContainerManager() {
		_alreadyDone = _objTextAlarm = _mouseInView = false;
		_lastPickedObjectID = Nothing;
		_numPicked = 1;
		_amountIndY = -1;
		_lastPickedObjectQuantity = - 1;
		_amountAccumulator = 0;
		_objToGet = nullptr;
		_selImage = nullptr;
		memset(_mouseText, 0, sizeof(_mouseText));
	}

	void add(ContainerNode *cn) {
		_list.push_front(cn);
	}

	void remove(ContainerNode *cn) {
		_list.remove(cn);
	}

	void moveToFront(ContainerNode *cn) {
		remove(cn);
		add(cn);
	}

	ContainerNode *find(ObjectID id);
	ContainerNode *find(ObjectID id, int16 type);

	//  Set which player is viewing the container windows.
	void setPlayerNum(PlayerActorID playerNum);
	void doDeferredActions();
	void setUpdate(ObjectID id);
};

ContainerNode *CreateContainerNode(ObjectID id, bool open = true, int16 mindType = 0);
ContainerNode *CreateReadyContainerNode(PlayerActorID player);
ContainerNode *OpenMindContainer(PlayerActorID player, int16 open, int16 type);

/* ===================================================================== *
   Exports
 * ===================================================================== */

void initContainers();
void cleanupContainers();

void initContainerNodes();
void saveContainerNodes(Common::OutSaveFile *outS);
void loadContainerNodes(Common::InSaveFile *in);
void cleanupContainerNodes();

extern void updateContainerWindows();

extern APPFUNC(cmdCloseButtonFunc);
extern APPFUNC(cmdMindContainerFunc);
extern APPFUNC(cmdScrollFunc);

} // end of namespace Saga2

#endif
