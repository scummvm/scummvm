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

#ifndef MADS_USER_INTERFACE_H
#define MADS_USER_INTERFACE_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"
#include "mads/msurface.h"
#include "mads/screen.h"

namespace MADS {

enum { IMG_SPINNING_OBJECT = 200, IMG_TEXT_UPDATE = 201 };

enum ScrollbarActive {
	SCROLLBAR_NONE = 0,		// No state
	SCROLLBAR_UP = 1,		// Up butotn
	SCROLLBAR_DOWN = 2,		// Down button
	SCROLLBAR_ELEVATOR = 3,	// Elevator bar
	SCROLLBAR_THUMB = 4		// Scrollbar thumb
};

class AnimFrameEntry;
class MADSEngine;

class UISlot {
public:
	int _flags;
	int _segmentId;
	int _spritesIndex;
	int _frameNumber;
	Common::Point _position;

	// Only used for IMG_OVERPRINT
	int _width;
	int _height;

	UISlot();
};

/**
 * Sprite list for the user interface
 */
class UISlots : public Common::Array<UISlot> {
private:
	MADSEngine *_vm;
public:
	/**
	 * Constructor
	 */
	UISlots(MADSEngine *vm) : _vm(vm) {}

	/**
	 * Add an overprint (text) entry to the list
	 */
	void add(const Common::Rect &bounds);

	/**
	 * Loads the data from an aimation frame entry
	 */
	void add(const AnimFrameEntry &frameEntry);

	/**
	 * Adds a special entry for full refresh of the user interface
	 */
	void fullRefresh();

	/**
	 * Draw all the sprites in the list on the user interface.
	 * @param updateFlag	Flag drawn areas to be updated on physical screen
	 * @param delFlag		Controls how used slots are deleted after drawing
	 */
	void draw(bool updateFlag, bool delFlag);
};

class Conversation {
private:
	static MADSEngine *_vm;
public:
	static void init(MADSEngine *vm);
public:
	int _globalId;
	Common::Array<int> _quotes;

	/**
	 * Set up a conversation sequence
	 */
	void setup(int globalId, ...);

	/**
	 * Activates the passed set of quotes in the given conversation node
	 */
	void set(int quoteId, ...);

	/**
	 * Returns the bit for a given quote to indicate whether it's active or not or,
	 * if 0 is passed, returns the number of currently active quotes
	 */
	int read(int quoteId);

	/**
	 * Activates or deactivates the specified quote in the given conversation node
	 */
	void write(int quoteId, bool flag);

	/**
	 * Starts the conversation
	 */
	void start();
};

class UserInterface : public MSurface {
	friend class UISlots;
private:
	MADSEngine *_vm;
	int _invSpritesIndex;
	int _invFrameNumber;
	uint32 _scrollMilli;
	bool _scrollFlag;
	int _noSegmentsActive;
	int _someSegmentsActive;

	/**
	 * Loads the elements of the user interface
	 */
	void loadElements();

	/**
	 * Returns the area within the user interface a given element falls
	 */
	bool getBounds(ScrCategory category, int invIndex, Common::Rect &bounds);

	/**
	 * Reposition a bounding rectangle to physical co-ordinates
	 */
	void moveRect(Common::Rect &bounds);

	/**
	 * Draw options during a conversation.
	 */
	void drawConversationList();

	/**
	 * Draw the action list
	 */
	void drawActions();

	/**
	 * Draw the inventory list
	 */
	void drawInventoryList();

	/**
	 * Draw the inventory item vocab list
	 */
	void drawItemVocabList();

	/**
	 * Draw the inventory scroller
	 */
	void drawScroller();

	/**
	 * Called when the inventory scrollbar has changed
	 */
	void scrollbarChanged();

	/**
	 * Draw a UI textual element
	 */
	void writeVocab(ScrCategory category, int id);
public:
	MSurface _surface;
	UISlots _uiSlots;
	DirtyAreas _dirtyAreas;
	ScrCategory _category;
	Common::Rect *_rectP;
	int _inventoryTopIndex;
	int _selectedInvIndex;
	int _selectedActionIndex;
	int _selectedItemVocabIdx;
	ScrollbarActive _scrollbarActive, _scrollbarOldActive;
	int _highlightedCommandIndex;
	int _highlightedInvIndex;
	int _highlightedItemVocabIndex;
	bool _inventoryChanged;
	int _categoryIndexes[8];
	Common::StringArray _talkStrings;
	Common::Array<int> _talkIds;
	bool _scrollbarQuickly;
	uint32 _scrollbarMilliTime;
	int _scrollbarElevator, _scrollbarOldElevator;
	ScrollbarActive _scrollbarStrokeType;
public:
	/**
	* Constructor
	*/
	UserInterface(MADSEngine *vm);

	/**
	* Loads an interface from a specified resource
	*/
	void load(const Common::String &resName) override;

	/**
	* Set up the interface
	*/
	void setup(InputMode inputMode);

	void drawTextElements();

	/**
	* Merges a sub-section of another surface into the user interface without
	* destroying any on-screen text
	* @param src			Source surface
	* @param srcBounds		Area to copy/merge from
	* @param destPos		Destination position to draw in current surface
	* @param transparencyIndex	Transparency color
	*/
	void mergeFrom(BaseSurface *src, const Common::Rect &srcBounds, const Common::Point &destPos,
		int transparencyIndex = -1);

	/**
	 * Loads the animation sprite data for a given inventory object
	 */
	void loadInventoryAnim(int objectId);

	/**
	 * Resets the inventory animation when no inventory item is selected
	 */
	void noInventoryAnim();

	/**
	 * Handles any animation that occurs in the background of the user interface
	 */
	void doBackgroundAnimation();

	/**
	* Handles queuing a new frame of an inventory animation for drawing
	*/
	void inventoryAnim();

	void categoryChanged();

	/**
	 * Select an item from the inventory list
	 * @param invIndex	Index in the inventory list of the item to select
	 */
	void selectObject(int invIndex);

	void updateSelection(ScrCategory category, int newIndex, int *idx);

	/**
	* Updates the current top visible item of the scrollbar
	*/
	void changeScrollBar();

	void scrollerChanged();

	void scrollInventory();

	/**
	* Checks for the mouse being on the user interface inventory scroller,
	* and update the scroller highlight and selected inventory object as necessary
	*/
	void updateInventoryScroller();

	/**
	 * Empties the current conversation talk list
	 */
	void emptyConversationList();

	/**
	 * Add a msesage to the list of conversation items to select from
	 */
	void addConversationMessage(int vocabId, const Common::String &msg);

	/**
	 * Synchronize the data
	 */
	void synchronize(Common::Serializer &s);

	void refresh();
};

} // End of namespace MADS

#endif /* MADS_USER_INTERFACE_H */
